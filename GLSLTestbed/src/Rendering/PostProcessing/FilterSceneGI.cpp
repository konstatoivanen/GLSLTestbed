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
        volumeDescriptor.filtermag = GL_LINEAR;
        volumeDescriptor.filtermin = GL_LINEAR_MIPMAP_LINEAR;
        volumeDescriptor.wrapmodex = GL_CLAMP_TO_BORDER;
        volumeDescriptor.wrapmodey = GL_CLAMP_TO_BORDER;
        volumeDescriptor.wrapmodez = GL_CLAMP_TO_BORDER;
        volumeDescriptor.resolution = resolution;
        volumeDescriptor.miplevels = glm::min(6u, Functions::GetMaxMipLevelPow2(resolution));
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
        auto res = source->GetResolution3D();
        res.x /= 2;
        res.y /= 2;

        if (m_screenSpaceGI->ValidateResolution(res))
        {
            GraphicsAPI::SetGlobalResourceHandle(StringHashID::StringToID("pk_ScreenGI_Diffuse"), m_screenSpaceGI->GetColorBuffer(0)->GetBindlessHandleResident());
            GraphicsAPI::SetGlobalResourceHandle(StringHashID::StringToID("pk_ScreenGI_Specular"), m_screenSpaceGI->GetColorBuffer(1)->GetBindlessHandleResident());
        }
    }

    void FilterSceneGI::Execute(Batching::DynamicBatchCollection* visibleBatches)
    {
        uint4 viewports[3] = 
        { 
            {0u, 0u, m_voxelsDiffuse->GetWidth(),  m_voxelsDiffuse->GetHeight() },
            {0u, 0u, m_voxelsDiffuse->GetHeight(), m_voxelsDiffuse->GetDepth() },
            {0u, 0u, m_voxelsDiffuse->GetWidth(),  m_voxelsDiffuse->GetDepth() },
        };

        uint3 swizzles[3] =
        {
             { 0u, 1u, 2u }, 
             { 1u, 2u, 0u },
             { 0u, 2u, 1u }
        };

        FixedStateAttributes voxelizeAttributes;
        voxelizeAttributes.BlendEnabled = false;
        voxelizeAttributes.ColorMask = 0;
        voxelizeAttributes.CullEnabled = false;
        voxelizeAttributes.CullMode = GL_BACK;
        voxelizeAttributes.ZTest = GL_LEQUAL;
        voxelizeAttributes.ZTestEnabled = false;
        voxelizeAttributes.ZWriteEnabled = false;

        m_rasterAxis = (m_rasterAxis + 1) % 3;

        GraphicsAPI::SetViewPort(viewports[m_rasterAxis].x, viewports[m_rasterAxis].y, viewports[m_rasterAxis].z, viewports[m_rasterAxis].w);
        GraphicsAPI::SetGlobalUInt3(StringHashID::StringToID("pk_GIVoxelAxisSwizzle"), swizzles[m_rasterAxis]);
        Batching::DrawBatchesPredicated(visibleBatches, StringHashID::StringToID("PK_META_GI_VOXELIZE"), m_shaderVoxelize, m_properties, voxelizeAttributes);

        auto resolution = m_voxelsDiffuse->GetResolution3D();

        m_properties.SetTexture(StringHashID::StringToID("pk_MipSource"), m_voxelsDiffuse->GetGraphicsID());

        for (auto i = 1u; i < m_voxelsDiffuse->GetMipCount(); ++i)
        {
            m_properties.SetImage(StringHashID::StringToID("pk_MipTarget"), m_voxelsDiffuse->GetImageBindDescriptor(GL_WRITE_ONLY, i, 0, true));
            GraphicsAPI::DispatchCompute(m_computeMipmap, { (resolution.x >> i) / 4u, (resolution.y >> i) / 4u, (resolution.z >> i) / 4u }, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        GraphicsAPI::SetRenderTarget(m_screenSpaceGI.get());
        GraphicsAPI::Blit(m_shader, m_properties);
    }
}