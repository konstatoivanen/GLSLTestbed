#include "PrecompiledHeader.h"
#include "FilterAO.h"
#include "Rendering/GraphicsAPI.h"
#include "Utilities/HashCache.h"

namespace PK::Rendering::PostProcessing
{
    struct PassParams
    {
        ulong source;
        float2 offset;
        uint2 readwrite;
    };

    FilterAO::FilterAO(AssetDatabase* assetDatabase, const ApplicationConfig* config) : FilterBase(assetDatabase->Find<Shader>("SH_VS_FilterAO"))
    {
        m_radius = config->AmbientOcclusionRadius;
        m_intensity = config->AmbientOcclusionIntensity;
        m_downsample = config->AmbientOcclusionDownsample;
        m_passKeywords[0] = StringHashID::StringToID("AO_PASS0");
        m_passKeywords[1] = StringHashID::StringToID("AO_PASS1");
        m_passBuffer = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::HANDLE, "SOURCE"}, { CG_TYPE::FLOAT2, "OFFSET" }, { CG_TYPE::UINT2, "READWRITE" } }), 3, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
   
        auto divisor = m_downsample ? 2 : 1;
        auto descriptor = RenderTextureDescriptor();
        descriptor.dimension = GL_TEXTURE_2D;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_R8 };
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
        descriptor.resolution.x = config->InitialWidth / divisor;
        descriptor.resolution.y = config->InitialHeight / divisor;
        descriptor.resolution.z = 0;
        m_renderTargets[0] = CreateRef<RenderTexture>(descriptor);

        descriptor.dimension = GL_TEXTURE_2D_ARRAY;
        descriptor.resolution.x = config->InitialWidth;
        descriptor.resolution.y = config->InitialHeight;
        descriptor.resolution.z = 2;
        m_renderTargets[1] = CreateRef<RenderTexture>(descriptor);

        m_updateParams = true;
    }

    void FilterAO::OnPreRender(const RenderTexture* source)
    {
        auto resolution = source->GetResolution3D();
        auto divisor = m_downsample ? 2 : 1;

        m_updateParams |= m_renderTargets[0]->ValidateResolution({ resolution.x / divisor, resolution.y / divisor, 0 });
        m_updateParams |= m_renderTargets[1]->ValidateResolution({ resolution.x, resolution.y, 2 });

        if (!m_updateParams)
        {
            return;
        }

        m_updateParams = false;

        auto bufferview = m_passBuffer->BeginMapBuffer<PassParams>();
        bufferview[0] = { m_renderTargets[0]->GetColorBuffer(0)->GetBindlessHandleResident(), CG_FLOAT2_ZERO, {0, 0} };
        bufferview[1] = { m_renderTargets[0]->GetColorBuffer(0)->GetBindlessHandleResident(), CG_FLOAT2_RIGHT * 2.0f, {0, 1} };
        bufferview[2] = { m_renderTargets[1]->GetColorBuffer(0)->GetBindlessHandleResident(), CG_FLOAT2_UP * (2.0f * divisor), {1, 0} };
        m_passBuffer->EndMapBuffer();

        GraphicsAPI::SetGlobalResourceHandle(HashCache::Get()->pk_ScreenOcclusion, m_renderTargets[1]->GetColorBuffer(0)->GetBindlessHandleResident());
    }

    void FilterAO::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        m_properties.SetFloat3(HashCache::Get()->_AOParams, { m_intensity, m_radius, m_downsample ? 0.5f : 1.0f });
        m_properties.SetComputeBuffer(HashCache::Get()->_AOPassParams, m_passBuffer->GetGraphicsID());

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::Blit(m_renderTargets[0].get(), m_shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    
        m_properties.SetKeywords({ m_passKeywords[1] });
        GraphicsAPI::BlitInstanced(1, 1, m_renderTargets[1].get(), m_shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        GraphicsAPI::BlitInstanced(2, 1, m_renderTargets[1].get(), m_shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }
}