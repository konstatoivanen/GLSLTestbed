#include "PrecompiledHeader.h"
#include "FilterBloom.h"
#include "Utilities/HashCache.h"
#include "Rendering/GraphicsAPI.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    static const uint HISTOGRAM_THREAD_COUNT = 16;
    static const uint HISTOGRAM_NUM_BINS = 256;

    struct PassParams
    {
        ulong source = 0;
        float2 offset = CG_FLOAT2_ZERO;
        uint2 readwrite = CG_UINT2_ZERO;
    };

    FilterBloom::FilterBloom(AssetDatabase* assetDatabase, const ApplicationConfig* config) : FilterBase(assetDatabase->Find<Shader>("SH_VS_FilterBloom"))
    {
        auto lensDirtTexture = assetDatabase->Load<TextureXD>(config->FileBloomDirt.value.c_str());
        m_computeHistogram = assetDatabase->Find<Shader>("CS_LuminanceHistogram");
        m_computeFilmgrain = assetDatabase->Find<Shader>("SH_VS_FilmGrain");

        m_passKeywords[0] = StringHashID::StringToID("PASS_COMPOSITE");
        m_passKeywords[1] = StringHashID::StringToID("PASS_DOWNSAMPLE");
        m_passKeywords[2] = StringHashID::StringToID("PASS_BLUR");
        m_passKeywords[3] = StringHashID::StringToID("PASS_HISTOGRAM");
        m_passKeywords[4] = StringHashID::StringToID("PASS_AVG");

        m_computeHistogram = assetDatabase->Find<Shader>("CS_LuminanceHistogram");
        m_histogram = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::UINT, "COUNT"} }), HISTOGRAM_NUM_BINS + 1, true, GL_NONE);
        m_paramatersBuffer = CreateRef<ConstantBuffer>(BufferLayout(
        {
            {CG_TYPE::FLOAT, "pk_MinLogLuminance"},
            {CG_TYPE::FLOAT, "pk_InvLogLuminanceRange"},
            {CG_TYPE::FLOAT, "pk_LogLuminanceRange"},
            {CG_TYPE::FLOAT, "pk_TargetExposure"},
            {CG_TYPE::FLOAT, "pk_AutoExposureSpeed"},
            {CG_TYPE::FLOAT, "pk_BloomIntensity"},
            {CG_TYPE::FLOAT, "pk_BloomDirtIntensity"},
            {CG_TYPE::FLOAT, "pk_Vibrance"},
            {CG_TYPE::FLOAT4, "pk_VignetteGrain"},
            {CG_TYPE::FLOAT4, "pk_WhiteBalance"},
            {CG_TYPE::FLOAT4, "pk_Lift"},
            {CG_TYPE::FLOAT4, "pk_Gamma"},
            {CG_TYPE::FLOAT4, "pk_Gain"},
            {CG_TYPE::FLOAT4, "pk_ContrastGainGammaContribution"},
            {CG_TYPE::FLOAT4, "pk_HSV"},
            {CG_TYPE::FLOAT4, "pk_ChannelMixerRed"},
            {CG_TYPE::FLOAT4, "pk_ChannelMixerGreen"},
            {CG_TYPE::FLOAT4, "pk_ChannelMixerBlue"},
            {CG_TYPE::HANDLE, "pk_BloomLensDirtTex"},
            {CG_TYPE::HANDLE, "pk_HDRScreenTex"},
            {CG_TYPE::HANDLE, "pk_FilmGrainTex"},
        }));
        
        m_passBuffer = CreateRef<ComputeBuffer>(BufferLayout(
        {
            { CG_TYPE::HANDLE, "SOURCE"},
            { CG_TYPE::FLOAT2, "OFFSET" },
            { CG_TYPE::UINT2, "READWRITE" }
        }), 31, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
        
        auto descriptor = RenderTextureDescriptor();
        descriptor.dimension = GL_TEXTURE_2D_ARRAY;
        descriptor.resolution = { config->InitialWidth, config->InitialHeight, 2 };
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RGBA16F };
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;

        for (int i = 0; i < 6; ++i)
        {
            descriptor.resolution.x /= 2;
            descriptor.resolution.y /= 2;
            m_renderTargets[i] = CreateRef<RenderTexture>(descriptor);
        }

        descriptor.dimension = GL_TEXTURE_2D;
        descriptor.wrapmodex = GL_REPEAT;
        descriptor.wrapmodey = GL_REPEAT;
        descriptor.wrapmodez = GL_REPEAT;
        descriptor.resolution.x = 256;
        descriptor.resolution.y = 256;
        descriptor.resolution.z = 0;
        descriptor.colorFormats = { GL_RGB8 };
        m_filmGrainTexture = CreateRef<RenderTexture>(descriptor);

        m_paramatersBuffer->SetResourceHandle(HashCache::Get()->pk_FilmGrainTex, m_filmGrainTexture->GetColorBuffer(0)->GetBindlessHandleResident());
        m_paramatersBuffer->SetResourceHandle(HashCache::Get()->pk_BloomLensDirtTex, lensDirtTexture->GetBindlessHandleResident());
        m_paramatersBuffer->SetResourceHandle(HashCache::Get()->pk_HDRScreenTex, m_renderTargets[0]->GetColorBuffer(0)->GetBindlessHandleResident());

        OnUpdateParameters(config);

        m_properties.SetConstantBuffer(HashCache::Get()->pk_TonemappingParams, m_paramatersBuffer->GetGraphicsID());
        m_properties.SetComputeBuffer(HashCache::Get()->pk_Histogram, m_histogram->GetGraphicsID());

        m_updateParameters = true;
    }

    void FilterBloom::OnPreRender(const RenderTexture* source)
    {
        auto resolution = source->GetResolution3D();
        resolution.z = 2;

        for (int i = 0; i < 6; ++i)
        {
            resolution.x /= 2;
            resolution.y /= 2;
            m_updateParameters |= m_renderTargets[i]->ValidateResolution(resolution);
        }

        if (!m_updateParameters)
        {
            return;
        }

        m_updateParameters = false;

        // Kinda volatile not to do this every frame but whatever.
        m_paramatersBuffer->SetResourceHandle(HashCache::Get()->pk_HDRScreenTex, m_renderTargets[0]->GetColorBuffer(0)->GetBindlessHandleResident());
        m_paramatersBuffer->FlushBuffer();

        GLuint64 handles[7] =
        {
            m_renderTargets[0]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[1]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[2]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[3]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[4]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[5]->GetColorBuffer(0)->GetBindlessHandleResident(),
            source->GetColorBuffer(0)->GetBindlessHandleResident(), // Same here
        };

        const float blurSize = 4.0f;
        auto bufferview = m_passBuffer->BeginMapBuffer<PassParams>();
        auto passIdx = 0;

        for (int i = 0; i < 6; ++i)
        {
            bufferview[passIdx++] = { i == 0 ? handles[6] : handles[i - 1], CG_FLOAT2_ZERO, { 0, 0 } };
            auto spread = i == 2 ? 0.75f : (i > 1 ? 1.0f : 0.5f);

            for (int j = 0; j < 2; ++j)
            {
                bufferview[passIdx++] = { handles[i], { (blurSize * 0.5f + j) * spread, 0 }, { 0, 1 } };
                bufferview[passIdx++] = { handles[i], { 0, (blurSize * 0.5f + j) * spread }, { 1, 0 } };
            }
        }

        bufferview[passIdx++] = { handles[6], CG_FLOAT2_ZERO, CG_UINT2_ZERO };

        m_passBuffer->EndMapBuffer();
    }
    
    void FilterBloom::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        m_properties.SetComputeBuffer(HashCache::Get()->_BloomPassParams, m_passBuffer->GetGraphicsID());

        GraphicsAPI::Blit(m_filmGrainTexture.get(), m_computeFilmgrain, GL_TEXTURE_FETCH_BARRIER_BIT);

        float4 viewports[7] =
        {
            {0,0, m_renderTargets[0]->GetResolution2D()},
            {0,0, m_renderTargets[1]->GetResolution2D()},
            {0,0, m_renderTargets[2]->GetResolution2D()},
            {0,0, m_renderTargets[3]->GetResolution2D()},
            {0,0, m_renderTargets[4]->GetResolution2D()},
            {0,0, m_renderTargets[5]->GetResolution2D()},
            {0,0, source->GetResolution2D()},
        };

        GraphicsAPI::SetViewPorts(0, viewports, 7);

        for (int i = 0; i < 6; ++i)
        {
            GraphicsAPI::SetRenderTarget(m_renderTargets[i].get(), false);
            m_properties.SetKeywords({ m_passKeywords[1] });                            // downsample
            GraphicsAPI::BlitInstanced(i * 5 + 0, 1, m_shader, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT);
            m_properties.SetKeywords({ m_passKeywords[2] });
            GraphicsAPI::BlitInstanced(i * 5 + 1, 1, m_shader, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT); // vertical blur
            GraphicsAPI::BlitInstanced(i * 5 + 2, 1, m_shader, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT); // horizontal blur
            GraphicsAPI::BlitInstanced(i * 5 + 3, 1, m_shader, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT); // vertical blur
            GraphicsAPI::BlitInstanced(i * 5 + 4, 1, m_shader, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT); // horizontal blur
        }

        uint3 histogramGroupCount =
        {
            (uint)std::ceilf(m_renderTargets[0]->GetWidth() / (float)HISTOGRAM_THREAD_COUNT),
            (uint)std::ceilf(m_renderTargets[0]->GetHeight() / (float)HISTOGRAM_THREAD_COUNT),
            1
        };

        // Auto exposure histogram
        m_properties.SetKeywords({ m_passKeywords[3] });
        GraphicsAPI::DispatchCompute(m_computeHistogram, histogramGroupCount, m_properties);
        m_properties.SetKeywords({ m_passKeywords[4] });
        GraphicsAPI::DispatchCompute(m_computeHistogram, { 1,1,1 }, m_properties);

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::SetRenderTarget(destination, false);
        GraphicsAPI::BlitInstanced(30, 1, m_shader, m_properties);
    }

    void FilterBloom::OnUpdateParameters(const ApplicationConfig* config)
    {
        m_paramatersBuffer->SetFloat(HashCache::Get()->pk_MinLogLuminance, config->AutoExposureLuminanceMin);
        m_paramatersBuffer->SetFloat(HashCache::Get()->pk_InvLogLuminanceRange, 1.0f / config->AutoExposureLuminanceRange);
        m_paramatersBuffer->SetFloat(HashCache::Get()->pk_LogLuminanceRange, config->AutoExposureLuminanceRange);
        m_paramatersBuffer->SetFloat(HashCache::Get()->pk_TargetExposure, config->TonemapExposure);
        m_paramatersBuffer->SetFloat(HashCache::Get()->pk_AutoExposureSpeed, config->AutoExposureSpeed);
        m_paramatersBuffer->SetFloat(HashCache::Get()->pk_BloomIntensity, glm::exp(config->BloomIntensity) - 1.0f);
        m_paramatersBuffer->SetFloat(HashCache::Get()->pk_BloomDirtIntensity, glm::exp(config->BloomLensDirtIntensity) - 1.0f);

        color lift, gamma, gain;
        Functions::GenerateLiftGammaGain(Functions::HexToRGB(config->CC_Shadows), Functions::HexToRGB(config->CC_Midtones), Functions::HexToRGB(config->CC_Highlights), &lift, &gamma, &gain);
        m_paramatersBuffer->SetFloat(HashCache::Get()->pk_Vibrance, config->CC_Vibrance);
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_VignetteGrain, { config->VignetteIntensity, config->VignettePower, config->FilmGrainLuminance, config->FilmGrainIntensity });
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_WhiteBalance, Functions::GetWhiteBalance(config->CC_TemperatureShift, config->CC_Tint));
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_Lift, lift);
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_Gamma, gamma);
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_Gain, gain);
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_ContrastGainGammaContribution, float4(config->CC_Contrast, config->CC_Gain, 1.0f / config->CC_Gamma, config->CC_Contribution));
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_HSV, float4(config->CC_Hue, config->CC_Saturation, config->CC_Value, 1.0f));
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_ChannelMixerRed, Functions::HexToRGB(config->CC_ChannelMixerRed));
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_ChannelMixerGreen, Functions::HexToRGB(config->CC_ChannelMixerGreen));
        m_paramatersBuffer->SetFloat4(HashCache::Get()->pk_ChannelMixerBlue, Functions::HexToRGB(config->CC_ChannelMixerBlue));
        m_paramatersBuffer->FlushBuffer();
    }
}