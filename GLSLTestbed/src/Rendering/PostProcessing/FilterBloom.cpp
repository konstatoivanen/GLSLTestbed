#include "PrecompiledHeader.h"
#include "FilterBloom.h"
#include "Utilities/HashCache.h"
#include "Rendering/Graphics.h"

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

void FilterBloom::Execute(Ref<RenderTexture> source, Ref<RenderTexture> destination)
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

    auto shader = m_shader.lock();

    const float blurSize = 4.0f;

    auto textureIdx = 0;

    auto downsampled = source;
    auto downsampledIdx = 0;

    m_properties.SetKeyword(m_passKeywords[0], false);

    for (int i = 0; i < 6; ++i)
    {
        auto fbo = m_blurTextures[textureIdx++];

        m_properties.SetKeyword(m_passKeywords[1], true);
        fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT0 });
        Graphics::Blit(downsampled->GetColorBuffer(downsampledIdx).lock(), fbo, shader, m_properties);

        auto spread = i == 2 ? 0.75f : (i > 1 ? 1.0f : 0.5f);

        for (int j = 0; j < 2; ++j)
        {
            m_properties.SetFloat(hashCache->_Bloom_BlurSize, (blurSize * 0.5f + j) * spread);

            // vertical blur
            m_properties.SetKeyword(m_passKeywords[1], false);
            m_properties.SetKeyword(m_passKeywords[2], true);
            fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT2 });
            Graphics::Blit(fbo->GetColorBuffer(j == 0 ? 0 : 1).lock(), fbo, shader, m_properties);

            // horizontal blur
            m_properties.SetKeyword(m_passKeywords[2], false);
            m_properties.SetKeyword(m_passKeywords[3], true);
            fbo->SetDrawTargets({ GL_COLOR_ATTACHMENT1 });
            Graphics::Blit(fbo->GetColorBuffer(2).lock(), fbo, shader, m_properties);

            m_properties.SetKeyword(m_passKeywords[3], false);
        }

        downsampled = fbo;
        downsampledIdx = 1;
        m_bloomLayers[i] = fbo->GetColorBuffer(1).lock()->GetGraphicsID();
    }

    m_properties.SetTexture(hashCache->_Bloom_Textures, m_bloomLayers, 6);

    //Pass 0
    m_properties.SetKeyword(m_passKeywords[0], true);
    Graphics::Blit(source->GetColorBuffer(0).lock(), destination, shader, m_properties);
}
