#include "PrecompiledHeader.h"
#include "FilterAO.h"
#include "Rendering/Graphics.h"
#include "Utilities/HashCache.h"

namespace PK::Rendering::PostProcessing
{
    FilterAO::FilterAO(Weak<Shader> shader, float intensity, float radius, bool downsample) : FilterBase(shader)
    {
        m_radius = radius;
        m_intensity = intensity;
        m_downsample = downsample;
        m_passKeywords[0] = StringHashID::StringToID("AO_PASS0");
        m_passKeywords[1] = StringHashID::StringToID("AO_PASS1");
        m_passKeywords[2] = StringHashID::StringToID("AO_PASS2");
    }

    static void ValidateRenderTextures(RenderTextureDescriptor& descriptor, Ref<RenderTexture>* renderTextures, int downSample)
    {
        auto resolution = descriptor.resolution;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_R8 };
        descriptor.resolution.x = resolution.x / downSample;
        descriptor.resolution.y = resolution.y / downSample;

        if (renderTextures[0] == nullptr)
        {
            renderTextures[0] = CreateRef<RenderTexture>(descriptor);
        }
        else
        {
            renderTextures[0]->ValidateResolution(descriptor.resolution);
        }

        descriptor.resolution = resolution;
        descriptor.colorFormats = { GL_R8, GL_R8 };

        if (renderTextures[1] == nullptr)
        {
            renderTextures[1] = CreateRef<RenderTexture>(descriptor);
        }
        else
        {
            renderTextures[1]->ValidateResolution(descriptor.resolution);
        }
    }

    void FilterAO::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        auto hashCache = HashCache::Get();

        m_properties.SetFloat(HashCache::Get()->_Intensity, m_intensity);
        m_properties.SetFloat(HashCache::Get()->_Radius, m_radius);
        m_properties.SetFloat(HashCache::Get()->_TargetScale, (m_downsample ? 0.5f : 1));
    
        auto shader = m_shader.lock().get();

        auto descriptor = source->GetCompoundDescriptor();
        auto divisor = m_downsample ? 2 : 1;
    
        ValidateRenderTextures(descriptor, m_renderTargets, divisor);

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::Blit(m_renderTargets[0].get(), shader, m_properties);
    
        m_properties.SetFloat2(HashCache::Get()->_BlurVector, CG_FLOAT2_RIGHT * 2.0f);
        m_properties.SetKeywords({ m_passKeywords[1] });
        m_renderTargets[1]->SetDrawTargets({ GL_COLOR_ATTACHMENT0 });
        GraphicsAPI::Blit(m_renderTargets[0]->GetColorBufferPtr(0), m_renderTargets[1].get(), shader, m_properties);
    
        m_properties.SetFloat2(HashCache::Get()->_BlurVector, CG_FLOAT2_UP * (2.0f * divisor));
        m_renderTargets[1]->SetDrawTargets({ GL_COLOR_ATTACHMENT1 });
        GraphicsAPI::Blit(m_renderTargets[1]->GetColorBufferPtr(0), m_renderTargets[1].get(), shader, m_properties);
    
        m_properties.SetFloat2(HashCache::Get()->_BlurVector, CG_FLOAT2_RIGHT * (1.0f * divisor));
        m_properties.SetKeywords({ m_passKeywords[2] });
        m_renderTargets[1]->SetDrawTargets({ GL_COLOR_ATTACHMENT0 });
        GraphicsAPI::Blit(m_renderTargets[1]->GetColorBufferPtr(1), m_renderTargets[1].get(), shader, m_properties);
    
        m_properties.SetFloat2(HashCache::Get()->_BlurVector, CG_FLOAT2_UP * (1.0f * divisor));
        m_renderTargets[1]->SetDrawTargets({ GL_COLOR_ATTACHMENT1 });
        GraphicsAPI::Blit(m_renderTargets[1]->GetColorBufferPtr(0), m_renderTargets[1].get(), shader, m_properties);
    
        GraphicsAPI::SetGlobalTexture(HashCache::Get()->pk_ScreenOcclusion, m_renderTargets[1]->GetColorBuffer(1).lock()->GetGraphicsID());
    }
}