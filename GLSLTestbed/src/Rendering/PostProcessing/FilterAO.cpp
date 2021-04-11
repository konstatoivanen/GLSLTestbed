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

    void FilterAO::OnPreRender(const RenderTexture* source)
    {
        auto descriptor = source->GetCompoundDescriptor();
        auto resolution = descriptor.resolution;
        auto divisor = m_downsample ? 2 : 1;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_R8 };
        descriptor.resolution.x = resolution.x / divisor;
        descriptor.resolution.y = resolution.y / divisor;

        auto hasDelta = false;

        if (m_renderTargets[0] == nullptr)
        {
            m_renderTargets[0] = CreateRef<RenderTexture>(descriptor);
            hasDelta = true;
        }
        else
        {
            hasDelta |= m_renderTargets[0]->ValidateResolution(descriptor.resolution);
        }

        descriptor.resolution = resolution;
        descriptor.resolution.z = 2;
        descriptor.dimension = GL_TEXTURE_2D_ARRAY;
        descriptor.colorFormats = { GL_R8 };

        if (m_renderTargets[1] == nullptr)
        {
            m_renderTargets[1] = CreateRef<RenderTexture>(descriptor);
            hasDelta = true;
        }
        else
        {
            hasDelta |= m_renderTargets[1]->ValidateResolution(descriptor.resolution);
        }

        if (m_passBuffer == nullptr)
        {
            m_passBuffer = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::HANDLE, "SOURCE"}, { CG_TYPE::FLOAT2, "OFFSET" }, { CG_TYPE::UINT2, "READWRITE" } }), 5, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
        }

        if (!hasDelta)
        {
            return;
        }

        auto bufferview = m_passBuffer->BeginMapBuffer<PassParams>();

        bufferview[0] = { m_renderTargets[0]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_ZERO, {0, 0} };
        bufferview[1] = { m_renderTargets[0]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_RIGHT * 2.0f, {0, 1} };
        bufferview[2] = { m_renderTargets[1]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_UP * (2.0f * divisor), {1, 0} };
        bufferview[3] = { m_renderTargets[1]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_RIGHT * (1.0f * divisor), {0, 1} };
        bufferview[4] = { m_renderTargets[1]->GetColorBufferPtr(0)->GetBindlessHandleResident(), CG_FLOAT2_UP * (1.0f * divisor), {1, 0} };

        m_passBuffer->EndMapBuffer();
    
        GraphicsAPI::SetGlobalResourceHandle(HashCache::Get()->pk_ScreenOcclusion, m_renderTargets[1]->GetColorBufferPtr(0)->GetBindlessHandleResident());
    }

    void FilterAO::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        auto shader = m_shader.lock().get();
    
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
    }
}