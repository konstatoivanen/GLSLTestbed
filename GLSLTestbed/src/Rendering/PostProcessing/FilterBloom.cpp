#include "PrecompiledHeader.h"
#include "FilterBloom.h"
#include "Utilities/HashCache.h"
#include "Rendering/Graphics.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    FilterBloom::FilterBloom(Weak<Shader> shader, Weak<TextureXD> lensDirt, float exposure, float intensity, float lensDirtIntensity) : FilterBase(shader)
    {
        m_exposure = exposure;
        m_intensity = intensity;
        m_lensDirtIntensity = lensDirtIntensity;
        m_lensDirtTexture = lensDirt;
    
        m_passKeywords[0] = StringHashID::StringToID("BLOOM_PASS0");
        m_passKeywords[1] = StringHashID::StringToID("BLOOM_PASS1");
        m_passKeywords[2] = StringHashID::StringToID("BLOOM_PASS2");
        m_passKeywords[3] = StringHashID::StringToID("BLOOM_PASS3");
    }
    
    static void ValidateRenderTextures(RenderTextureDescriptor& descriptor, Ref<RenderTexture>* renderTextures)
    {
        for (int i = 0; i < 6; ++i)
        {
            descriptor.resolution.x /= 2;
            descriptor.resolution.y /= 2;
    
    
            if (renderTextures[i] == nullptr)
            {
                renderTextures[i] = CreateRef<RenderTexture>(descriptor);
            }
            else
            {
                renderTextures[i]->ValidateResolution(descriptor.resolution);
            }
        }
    }
    
    void FilterBloom::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        auto descriptor = source->GetCompoundDescriptor();
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RGBA16F, GL_RGBA16F, GL_RGBA16F };
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;
    
        ValidateRenderTextures(descriptor, m_blurTextures);
    
        auto hashCache = HashCache::Get();
    
        m_properties.SetFloat(hashCache->_Bloom_Intensity, glm::exp(m_intensity) - 1.0f);
        m_properties.SetFloat(hashCache->_Bloom_LensDirtIntensity, glm::exp(m_lensDirtIntensity) - 1.0f);
        m_properties.SetFloat(hashCache->_Tonemap_Exposure, m_exposure);
        m_properties.SetTexture(hashCache->_Bloom_DirtTexture, m_lensDirtTexture.lock()->GetGraphicsID());
    
        auto shader = m_shader.lock().get();
    
        const float blurSize = 4.0f;
    
        auto textureIdx = 0;
    
        auto downsampled = source;
        auto downsampledIdx = 0;
    
        for (int i = 0; i < 6; ++i)
        {
            auto fbo = m_blurTextures[textureIdx++].get();
    
            m_properties.SetKeywords({ m_passKeywords[1] });
            fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT0 });
            GraphicsAPI::Blit(downsampled->GetColorBufferPtr(downsampledIdx), fbo, shader, m_properties);
    
            auto spread = i == 2 ? 0.75f : (i > 1 ? 1.0f : 0.5f);
    
            for (int j = 0; j < 2; ++j)
            {
                m_properties.SetFloat(hashCache->_Bloom_BlurSize, (blurSize * 0.5f + j) * spread);
    
                // vertical blur
                m_properties.SetKeywords({ m_passKeywords[2] });
                fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT2 });
                GraphicsAPI::Blit(fbo->GetColorBufferPtr(j == 0 ? 0 : 1), fbo, shader, m_properties);
    
                // horizontal blur
                m_properties.SetKeywords({ m_passKeywords[3] });
                fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT1 });
                GraphicsAPI::Blit(fbo->GetColorBufferPtr(2), fbo, shader, m_properties);
            }
    
            downsampled = fbo;
            downsampledIdx = 1;
            m_bloomLayers[i] = fbo->GetColorBufferPtr(1)->GetGraphicsID();
        }
    
        m_properties.SetTexture(hashCache->_Bloom_Textures, m_bloomLayers, 6);

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::Blit(source->GetColorBufferPtr(0), destination, shader, m_properties);
    }
}