#include "PrecompiledHeader.h"
#include "FilterBloom.h"
#include "Utilities/HashCache.h"
#include "Rendering/Graphics.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    struct PassParams
    {
        ulong source;
        float2 offset;
        uint2 readwrite;
    };

    FilterBloom::FilterBloom(Shader* shader, TextureXD* lensDirt, float exposure, float intensity, float lensDirtIntensity) : FilterBase(shader)
    {
        m_exposure = exposure;
        m_intensity = intensity;
        m_lensDirtIntensity = lensDirtIntensity;
        m_lensDirtTexture = lensDirt;   
        m_passKeywords[0] = StringHashID::StringToID("BLOOM_PASS0");
        m_passKeywords[1] = StringHashID::StringToID("BLOOM_PASS1");
        m_passKeywords[2] = StringHashID::StringToID("BLOOM_PASS2");
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
                    { CG_TYPE::UINT2, "READWRITE" } }), 
                    32, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
        }

        if (!hasdelta)
        {
            return;
        }

        GLuint64 handles[8] =
        {
            m_renderTargets[0]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[1]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[2]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[3]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[4]->GetColorBuffer(0)->GetBindlessHandleResident(),
            m_renderTargets[5]->GetColorBuffer(0)->GetBindlessHandleResident(),
            source->GetColorBuffer(0)->GetBindlessHandleResident(),//Kinda volatile not to do this every frame but whatever.
            m_lensDirtTexture->GetBindlessHandleResident(),
        };

        const float saturation = 0.8f;
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

        bufferview[passIdx++] = { handles[6], { glm::exp(m_intensity) - 1.0f, glm::exp(m_lensDirtIntensity) - 1.0f }, { 0, 0} };
        bufferview[passIdx++] = { handles[7], { m_exposure, saturation }, { 0, 0 } };

        m_passBuffer->EndMapBuffer();
    }
    
    void FilterBloom::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        m_properties.SetComputeBuffer(HashCache::Get()->_BloomPassParams, m_passBuffer->GetGraphicsID());

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