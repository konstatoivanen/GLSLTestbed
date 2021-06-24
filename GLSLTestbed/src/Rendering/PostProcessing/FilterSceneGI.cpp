#include "PrecompiledHeader.h"
#include "FilterSceneGI.h"
#include "Rendering/GraphicsAPI.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"

namespace PK::Rendering::PostProcessing
{
    FilterSceneGI::FilterSceneGI(AssetDatabase* assetDatabase, ECS::EntityDatabase* entityDb, const ApplicationConfig* config) : FilterBase(assetDatabase->Find<Shader>("SH_VS_SceneGI_Bake"))
    {
        m_shaderVoxelize = assetDatabase->Find<Shader>("SH_WS_SceneGI_Meta_White");
        m_computeMipmap = assetDatabase->Find<Shader>("CS_SceneGIMipmap");

        auto scaleTransform = float4(-40.0f, -6.0f, -55.0f, 0.6f);

        uint3 resolution = { 256u, 128u, 256u };

        TextureDescriptor volumeDescriptor;
        volumeDescriptor.dimension = GL_TEXTURE_3D;
        volumeDescriptor.colorFormat = GL_RGBA16;
        volumeDescriptor.miplevels = Functions::GetMaxMipLevel(resolution);
        volumeDescriptor.filtermag = GL_LINEAR;
        volumeDescriptor.filtermin = GL_LINEAR_MIPMAP_LINEAR;
        volumeDescriptor.wrapmodex = GL_CLAMP_TO_BORDER;
        volumeDescriptor.wrapmodey = GL_CLAMP_TO_BORDER;
        volumeDescriptor.wrapmodez = GL_CLAMP_TO_BORDER;
        volumeDescriptor.resolution = resolution;
        volumeDescriptor.miplevels = glm::min(6u, Functions::GetMaxMipLevel(resolution));
        volumeDescriptor.bordercolor = CG_COLOR_CLEAR;
        m_voxelsDiffuse = CreateRef<RenderBuffer>(volumeDescriptor);

        auto descriptor = RenderTextureDescriptor();
        descriptor.dimension = GL_TEXTURE_2D;
        descriptor.resolution = uint3(2, 2, 0);
        descriptor.colorFormats = { GL_RGBA16F, GL_RGBA16F };
        descriptor.depthFormat = GL_NONE;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
        descriptor.miplevels = 0;
        m_screenSpaceGI = CreateRef<RenderTexture>(descriptor);

        m_entityDb = entityDb;

        m_properties.SetFloat4(StringHashID::StringToID("pk_SceneGI_ST"), scaleTransform);
        m_properties.SetImage(StringHashID::StringToID("pk_SceneGI_VolumeWrite"), m_voxelsDiffuse->GetImageBindDescriptor(GL_WRITE_ONLY, 0, 0, true));
        m_properties.SetTexture(StringHashID::StringToID("pk_SceneGI_VolumeRead"), m_voxelsDiffuse->GetGraphicsID());
    }

    void FilterSceneGI::OnPreRender(const RenderTexture* source)
    {
        Batching::ResetCollection(&m_batches);

        auto views = m_entityDb->Query<PK::ECS::EntityViews::MeshRenderable>((uint)ECS::ENTITY_GROUPS::ACTIVE);

        for (uint i = 0; i < views.count; ++i)
        {
            const auto& view = views[i];
            auto* materials = &view.materials->sharedMaterials;
            auto* mesh = view.mesh->sharedMesh;

            for (auto i = 0; i < materials->size(); ++i)
            {
                Batching::QueueDraw(&m_batches, mesh, { &view.transform->localToWorld, 0.0f });
            }
        }

        Batching::UpdateBuffers(&m_batches);

        auto res = source->GetResolution3D();
        res.x /= 2;
        res.y /= 2;

        if (m_screenSpaceGI->ValidateResolution(res))
        {
            GraphicsAPI::SetGlobalResourceHandle(StringHashID::StringToID("pk_ScreenGI_Diffuse"), m_screenSpaceGI->GetColorBuffer(0)->GetBindlessHandleResident());
            GraphicsAPI::SetGlobalResourceHandle(StringHashID::StringToID("pk_ScreenGI_Specular"), m_screenSpaceGI->GetColorBuffer(1)->GetBindlessHandleResident());
        }
    }

    void FilterSceneGI::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        float4 viewports[4] = 
        { 
            {0, 0, m_screenSpaceGI->GetWidth(), m_screenSpaceGI->GetHeight() },
            {0, 0, m_voxelsDiffuse->GetWidth(), m_screenSpaceGI->GetHeight() },
            {0, 0, m_screenSpaceGI->GetHeight(), m_screenSpaceGI->GetDepth() },
            {0, 0, m_screenSpaceGI->GetWidth(), m_screenSpaceGI->GetDepth() },
        };

        GraphicsAPI::SetViewPorts(0, viewports, 4);
        GraphicsAPI::SetRenderTarget(m_screenSpaceGI.get(), false);
        Batching::DrawBatches(&m_batches, m_shaderVoxelize, m_properties);

        auto resolution = m_voxelsDiffuse->GetResolution3D();

        for (auto i = 1u; i < m_voxelsDiffuse->GetMipCount(); ++i)
        {
            m_properties.SetInt(StringHashID::StringToID("pk_SampleLevel"), i - 1);
            m_properties.SetTexture(StringHashID::StringToID("pk_MipSource"), m_voxelsDiffuse->GetGraphicsID());
            m_properties.SetImage(StringHashID::StringToID("pk_MipTarget"), m_voxelsDiffuse->GetImageBindDescriptor(GL_WRITE_ONLY, i, 0, true));
            GraphicsAPI::DispatchCompute(m_computeMipmap, { resolution.x >> i, resolution.y >> i, resolution.z >> i }, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        GraphicsAPI::Blit(m_shader, m_properties);
    }
}