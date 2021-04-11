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
    };

    FilterBloom::FilterBloom(Weak<Shader> shader, Weak<TextureXD> lensDirt, float exposure, float intensity, float lensDirtIntensity) : FilterBase(shader)
    {
        m_exposure = exposure;
        m_intensity = intensity;
        m_lensDirtIntensity = lensDirtIntensity;
        m_lensDirtTexture = lensDirt;
    
        m_passKeywords[0] = StringHashID::StringToID("BLOOM_PASS0");
        m_passKeywords[1] = StringHashID::StringToID("BLOOM_PASS1");
        m_passKeywords[2] = StringHashID::StringToID("BLOOM_PASS2");
    }
    
    static void UpdateBufferData(const RenderTexture* source, Ref<RenderTexture>* renderTextures, Ref<ComputeBuffer>& textureBuffer, Weak<TextureXD>& lensdirt, float bloomIntensity, float dirtIntensity, float exposure)
    {
        auto descriptor = source->GetCompoundDescriptor();
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RGBA16F, GL_RGBA16F };
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;

        auto hasdelta = false;

        for (int i = 0; i < 6; ++i)
        {
            descriptor.resolution.x /= 2;
            descriptor.resolution.y /= 2;
    
            if (renderTextures[i] == nullptr)
            {
                renderTextures[i] = CreateRef<RenderTexture>(descriptor);
                hasdelta = true;
            }
            else
            {
                hasdelta |= renderTextures[i]->ValidateResolution(descriptor.resolution);
            }
        }

        if (textureBuffer == nullptr)
        {
            textureBuffer = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::HANDLE, "SOURCE"}, { CG_TYPE::FLOAT2, "OFFSET" } }), 32, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
        }

        if (!hasdelta)
        {
            return;
        }

        const float blurSize = 4.0f;
        auto srcHandle = source->GetColorBufferPtr(0)->GetBindlessHandleResident();
        auto bufferview = textureBuffer->BeginMapBuffer<PassParams>();
        auto passIdx = 0;

        for (int i = 0; i < 6; ++i)
        {
            bufferview[passIdx++] = { i == 0 ? srcHandle : renderTextures[i - 1]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_ZERO };
            auto spread = i == 2 ? 0.75f : (i > 1 ? 1.0f : 0.5f);

            for (int j = 0; j < 2; ++j)
            {
                bufferview[passIdx++] = { renderTextures[i]->GetColorBufferPtr(0)->GetBindlessHandleResident(), { (blurSize * 0.5f + j) * spread, 0 } };
                bufferview[passIdx++] = { renderTextures[i]->GetColorBufferPtr(1)->GetBindlessHandleResident(), { 0, (blurSize * 0.5f + j) * spread } };
            }
        }

        bufferview[passIdx++] = { srcHandle, { glm::exp(bloomIntensity) - 1.0f, glm::exp(dirtIntensity) - 1.0f } };
        bufferview[passIdx++] = { lensdirt.lock()->GetBindlessHandleResident(), { exposure, 0 } };

        textureBuffer->EndMapBuffer();
    }
    
    void FilterBloom::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        UpdateBufferData(source, m_blurTextures, m_textureBuffer, m_lensDirtTexture, m_intensity, m_lensDirtIntensity, m_exposure);
    
        auto hashCache = HashCache::Get();
    
        m_properties.SetComputeBuffer(StringHashID::StringToID("_BloomPassParams"), m_textureBuffer->GetGraphicsID());

        const float blurSize = 4.0f;

        auto shader = m_shader.lock().get();
        auto passIdx = 0;

        for (int i = 0; i < 6; ++i)
        {
            auto fbo = m_blurTextures[i].get();
    
            m_properties.SetKeywords({ m_passKeywords[1] });
            fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT0 });
            GraphicsAPI::BlitInstanced(passIdx++, 1, fbo, shader, m_properties);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

            for (int j = 0; j < 2; ++j)
            {
                m_properties.SetKeywords({ m_passKeywords[2] });
    
                // vertical blur
                fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT1 });
                GraphicsAPI::BlitInstanced(passIdx++, 1, fbo, shader, m_properties);
                glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    
                // horizontal blur
                fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT0 });
                GraphicsAPI::BlitInstanced(passIdx++, 1, fbo, shader, m_properties);
                glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            }
        }

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::BlitInstanced(passIdx++, 1, destination, shader, m_properties);
    }
}