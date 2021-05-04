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

    FilterBloom::FilterBloom(AssetDatabase* assetDatabase, const ApplicationConfig& config) : FilterBase(assetDatabase->Find<Shader>("SH_VS_FilterBloom"))
    {
        m_lensDirtTexture = assetDatabase->Find<TextureXD>(config.FileBloomDirt.c_str());
        m_computeHistogram = assetDatabase->Find<Shader>("CS_LuminanceHistogram");

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
            {CG_TYPE::FLOAT, "pk_Saturation"},
            {CG_TYPE::HANDLE, "pk_BloomLensDirtTex"},
            {CG_TYPE::HANDLE, "pk_HDRScreenTex"},
        }));
        
        m_paramatersBuffer->SetFloat(StringHashID::StringToID("pk_MinLogLuminance"), config.AutoExposureLuminanceMin);
        m_paramatersBuffer->SetFloat(StringHashID::StringToID("pk_InvLogLuminanceRange"), 1.0f / config.AutoExposureLuminanceRange);
        m_paramatersBuffer->SetFloat(StringHashID::StringToID("pk_LogLuminanceRange"), config.AutoExposureLuminanceRange);
        m_paramatersBuffer->SetFloat(StringHashID::StringToID("pk_TargetExposure"), config.TonemapExposure);
        m_paramatersBuffer->SetFloat(StringHashID::StringToID("pk_AutoExposureSpeed"), 5.0f);
        m_paramatersBuffer->SetFloat(StringHashID::StringToID("pk_BloomIntensity"), glm::exp(config.BloomIntensity) - 1.0f);
        m_paramatersBuffer->SetFloat(StringHashID::StringToID("pk_BloomDirtIntensity"), glm::exp(config.BloomLensDirtIntensity) - 1.0f);
        m_paramatersBuffer->SetFloat(StringHashID::StringToID("pk_Saturation"), config.TonemapSaturation);
        m_paramatersBuffer->SetResourceHandle(StringHashID::StringToID("pk_BloomLensDirtTex"), m_lensDirtTexture->GetBindlessHandleResident());
        m_paramatersBuffer->FlushBuffer();

        m_properties.SetConstantBuffer(StringHashID::StringToID("pk_TonemappingParams"), m_paramatersBuffer->GetGraphicsID());
        m_properties.SetComputeBuffer(StringHashID::StringToID("pk_Histogram"), m_histogram->GetGraphicsID());
    }

    void FilterBloom::OnPreRender(const RenderTexture* source)
    {
        auto descriptor = source->GetCompoundDescriptor();
        descriptor.dimension = GL_TEXTURE_2D_ARRAY;
        descriptor.resolution.z = 2;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RGBA16F };
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;

        auto hasdelta = false;

        for (int i = 0; i < 6; ++i)
        {
            descriptor.resolution.x /= 2;
            descriptor.resolution.y /= 2;

            if (m_renderTargets[i] == nullptr)
            {
                m_renderTargets[i] = CreateRef<RenderTexture>(descriptor);
                hasdelta = true;
            }
            else
            {
                hasdelta |= m_renderTargets[i]->ValidateResolution(descriptor.resolution);
            }
        }

        if (m_passBuffer == nullptr)
        {
            m_passBuffer = CreateRef<ComputeBuffer>(BufferLayout(
            { 
                { CG_TYPE::HANDLE, "SOURCE"}, 
                { CG_TYPE::FLOAT2, "OFFSET" }, 
                { CG_TYPE::UINT2, "READWRITE" } 
            }), 31, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
        }

        if (!hasdelta)
        {
            return;
        }

        // Kinda volatile not to do this every frame but whatever.
        m_paramatersBuffer->SetResourceHandle(StringHashID::StringToID("pk_HDRScreenTex"), source->GetColorBuffer(0)->GetBindlessHandleResident());
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

        uint3 histogramGroupCount =
        {
            (uint)std::ceilf(source->GetWidth() / (float)HISTOGRAM_THREAD_COUNT),
            (uint)std::ceilf(source->GetHeight() / (float)HISTOGRAM_THREAD_COUNT),
            1
        };

        // Auto exposure histogram
        m_properties.SetKeywords({ m_passKeywords[3] });
        GraphicsAPI::DispatchCompute(m_computeHistogram, histogramGroupCount, m_properties);
        m_properties.SetKeywords({ m_passKeywords[4] });
        GraphicsAPI::DispatchCompute(m_computeHistogram, { 1,1,1 }, m_properties);

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
            GraphicsAPI::BlitInstanced(i * 5 + 0, 1, m_shader, m_properties);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            m_properties.SetKeywords({ m_passKeywords[2] });
            GraphicsAPI::BlitInstanced(i * 5 + 1, 1, m_shader, m_properties);             // vertical blur
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            GraphicsAPI::BlitInstanced(i * 5 + 2, 1, m_shader, m_properties);             // horizontal blur
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            GraphicsAPI::BlitInstanced(i * 5 + 3, 1, m_shader, m_properties);             // vertical blur
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            GraphicsAPI::BlitInstanced(i * 5 + 4, 1, m_shader, m_properties);            // horizontal blur
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::SetRenderTarget(destination, false);
        GraphicsAPI::BlitInstanced(30, 1, m_shader, m_properties);
    }
}