#include "PrecompiledHeader.h"
#include "FilterAO.h"
#include "Rendering/Graphics.h"
#include "Utilities/HashCache.h"

namespace PK::Rendering::PostProcessing
{
    struct PassParams
    {
        ulong source;
        float2 offset;
        uint2 readwrite;
    };

    FilterAO::FilterAO(Weak<Shader> shader, float intensity, float radius, bool downsample) : FilterBase(shader)
    {
        m_radius = radius;
        m_intensity = intensity;
        m_downsample = downsample;
        m_passKeywords[0] = StringHashID::StringToID("AO_PASS0");
        m_passKeywords[1] = StringHashID::StringToID("AO_PASS1");
        m_passKeywords[2] = StringHashID::StringToID("AO_PASS2");
    }

    static void ValidateBufferData(RenderTextureDescriptor& descriptor, Ref<RenderTexture>* renderTextures, Ref<ComputeBuffer>& passBuffer, int downSample)
    {
        auto resolution = descriptor.resolution;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_R8 };
        descriptor.resolution.x = resolution.x / downSample;
        descriptor.resolution.y = resolution.y / downSample;

        auto hasDelta = false;

        if (renderTextures[0] == nullptr)
        {
            renderTextures[0] = CreateRef<RenderTexture>(descriptor);
            hasDelta = true;
        }
        else
        {
            hasDelta |= renderTextures[0]->ValidateResolution(descriptor.resolution);
        }

        descriptor.resolution = resolution;
        descriptor.resolution.z = 2;
        descriptor.dimension = GL_TEXTURE_2D_ARRAY;
        descriptor.colorFormats = { GL_R8 };

        if (renderTextures[1] == nullptr)
        {
            renderTextures[1] = CreateRef<RenderTexture>(descriptor);
            hasDelta = true;
        }
        else
        {
            hasDelta |= renderTextures[1]->ValidateResolution(descriptor.resolution);
        }

        if (passBuffer == nullptr)
        {
            passBuffer = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::HANDLE, "SOURCE"}, { CG_TYPE::FLOAT2, "OFFSET" }, { CG_TYPE::UINT2, "READWRITE" } }), 5, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
        }

        if (!hasDelta)
        {
            return;
        }

        auto bufferview = passBuffer->BeginMapBuffer<PassParams>();

        bufferview[0] = { renderTextures[0]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_ZERO, {0, 0}};
        bufferview[1] = { renderTextures[0]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_RIGHT * 2.0f, {0, 1}};
        bufferview[2] = { renderTextures[1]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_UP * (2.0f * downSample), {1, 0}};
        bufferview[3] = { renderTextures[1]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_RIGHT * (1.0f * downSample), {0, 1}};
        bufferview[4] = { renderTextures[1]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_UP * (1.0f * downSample), {1, 0}};

        passBuffer->EndMapBuffer();
    }

    void FilterAO::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        auto hashCache = HashCache::Get();
        auto shader = m_shader.lock().get();
        auto descriptor = source->GetCompoundDescriptor();
        auto divisor = m_downsample ? 2 : 1;
    
        ValidateBufferData(descriptor, m_renderTargets, m_passBuffer, divisor);

        m_properties.SetFloat3(HashCache::Get()->_AOParams, { m_intensity, m_radius, m_downsample ? 0.5f : 1.0f });
        m_properties.SetComputeBuffer(HashCache::Get()->_AOPassParams, m_passBuffer->GetGraphicsID());

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::Blit(m_renderTargets[0].get(), shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    
        m_properties.SetKeywords({ m_passKeywords[1] });
        GraphicsAPI::BlitInstanced(1, 1, m_renderTargets[1].get(), shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        GraphicsAPI::BlitInstanced(2, 1, m_renderTargets[1].get(), shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    
        m_properties.SetKeywords({ m_passKeywords[2] });
        GraphicsAPI::BlitInstanced(3, 1, m_renderTargets[1].get(), shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        GraphicsAPI::BlitInstanced(4, 1, m_renderTargets[1].get(), shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    
        GraphicsAPI::SetGlobalTexture(HashCache::Get()->pk_ScreenOcclusion, m_renderTargets[1]->GetColorBufferPtr(0)->GetGraphicsID());
    }
}