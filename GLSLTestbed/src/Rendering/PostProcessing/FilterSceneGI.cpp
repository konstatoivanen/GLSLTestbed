#include "PrecompiledHeader.h"
#include "FilterSceneGI.h"
#include "Rendering/GraphicsAPI.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"

namespace PK::Rendering::PostProcessing
{
    FilterSceneGI::FilterSceneGI(AssetDatabase* assetDatabase, ECS::EntityDatabase* entityDb, const ApplicationConfig* config) : FilterBase(assetDatabase->Find<Shader>("CS_SceneGI_Bake_Checkerboard"))
    {
        m_shaderVoxelize = assetDatabase->Find<Shader>("SH_WS_SceneGI_Meta_White");
        m_computeMipmap = assetDatabase->Find<Shader>("CS_SceneGIMipmap");

        //auto scaleTransform = float4(-76.8f, -6.0f, -76.8f, 0.6f);

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

       // m_properties.SetFloat4(StringHashID::StringToID("pk_SceneGI_ST"), scaleTransform);
        m_properties.SetImage(StringHashID::StringToID("pk_SceneGI_VolumeWrite"), m_voxelsDiffuse->GetImageBindDescriptor(GL_WRITE_ONLY, 0, 0, true));
       // m_properties.SetTexture(StringHashID::StringToID("pk_SceneGI_VolumeRead"), m_voxelsDiffuse->GetGraphicsID());
    }

    void FilterSceneGI::OnPreRender(const RenderTexture* source)
    {
        auto res = source->GetResolution3D();

        if (m_screenSpaceGI->ValidateResolution(res))
        {
            GraphicsAPI::SetGlobalResourceHandle(StringHashID::StringToID("pk_ScreenGI_Diffuse"), m_screenSpaceGI->GetColorBuffer(0)->GetBindlessHandleResident());
            GraphicsAPI::SetGlobalResourceHandle(StringHashID::StringToID("pk_ScreenGI_Specular"), m_screenSpaceGI->GetColorBuffer(1)->GetBindlessHandleResident());

            m_properties.SetImage(StringHashID::StringToID("pk_SceneGI_DiffuseWrite"), m_screenSpaceGI->GetColorBuffer(0)->GetImageBindDescriptor(GL_WRITE_ONLY, 0, 0, false));
            m_properties.SetImage(StringHashID::StringToID("pk_SceneGI_SpecularWrite"), m_screenSpaceGI->GetColorBuffer(1)->GetImageBindDescriptor(GL_WRITE_ONLY, 0, 0, false));
        }

        GraphicsAPI::SetGlobalFloat4(StringHashID::StringToID("pk_SceneGI_ST"), float4(-76.8f, -6.0f, -76.8f, 0.6f));
        GraphicsAPI::SetGlobalTexture(StringHashID::StringToID("pk_SceneGI_VolumeRead"), m_voxelsDiffuse->GetGraphicsID());
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
        m_checkerboardIndex = (m_checkerboardIndex + 1) % 4;

        GraphicsAPI::SetViewPort(viewports[m_rasterAxis].x, viewports[m_rasterAxis].y, viewports[m_rasterAxis].z, viewports[m_rasterAxis].w);
        GraphicsAPI::SetGlobalUInt3(StringHashID::StringToID("pk_GIVoxelAxisSwizzle"), swizzles[m_rasterAxis]);
        GraphicsAPI::SetGlobalUInt(StringHashID::StringToID("pk_SceneGI_Checkerboard_Flip"), m_checkerboardIndex);
        Batching::DrawBatchesPredicated(visibleBatches, StringHashID::StringToID("PK_META_GI_VOXELIZE"), m_shaderVoxelize, m_properties, voxelizeAttributes);

        auto resolution = m_voxelsDiffuse->GetResolution3D();

        m_properties.SetTexture(StringHashID::StringToID("pk_MipSource"), m_voxelsDiffuse->GetGraphicsID());

        for (auto i = 1u; i < m_voxelsDiffuse->GetMipCount(); ++i)
        {
            m_properties.SetImage(StringHashID::StringToID("pk_MipTarget"), m_voxelsDiffuse->GetImageBindDescriptor(GL_WRITE_ONLY, i, 0, true));
            GraphicsAPI::DispatchCompute(m_computeMipmap, { (resolution.x >> i) / 4u, (resolution.y >> i) / 4u, (resolution.z >> i) / 4u }, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        auto res = m_screenSpaceGI->GetResolution3D();
        uint3 groupCount = { (uint)ceil((res.x / 4.0f) / 16.0f), (uint)ceil(res.y / 16.0f), 1u };

        GraphicsAPI::DispatchCompute(m_shader, groupCount, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
}