#include "PrecompiledHeader.h"
#include "FilterVolumetricFog.h"
#include "Rendering/Graphics.h"

namespace PK::Rendering::PostProcessing
{
    static const uint3 InjectThreadCount = { 16u, 2u, 16u };
    static const uint3 ScatterThreadCount = { 32u,2u,1u };
    static const uint3 VolumeResolution = { 160u, 90u, 128u };

    FilterVolumetricFog::FilterVolumetricFog(AssetDatabase* assetDatabase) : FilterBase(assetDatabase->Find<Shader>("SH_VS_VolumeFogComposite"))
    {
        RenderTextureDescriptor descriptor;
        descriptor.dimension = GL_TEXTURE_3D;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RGBA16F, GL_RGBA16F };
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
        descriptor.resolution = VolumeResolution;
        m_volumes = CreateRef<RenderTexture>(descriptor);

        m_computeInject = assetDatabase->Find<Shader>("CS_VolumeFogLightDensity");
        m_computeScatter = assetDatabase->Find<Shader>("CS_VolumeFogScatter");

        m_volumeResources = CreateRef<ConstantBuffer>(BufferLayout(
        {
            {CG_TYPE::FLOAT4, "pk_Volume_WindDir"},
            {CG_TYPE::FLOAT, "pk_Volume_ConstantFog"},
            {CG_TYPE::FLOAT, "pk_Volume_HeightFogExponent"},
            {CG_TYPE::FLOAT, "pk_Volume_HeightFogOffset"},
            {CG_TYPE::FLOAT, "pk_Volume_HeightFogAmount"},
            {CG_TYPE::FLOAT, "pk_Volume_Density"},
            {CG_TYPE::FLOAT, "pk_Volume_Intensity"},
            {CG_TYPE::FLOAT, "pk_Volume_Anisotropy"},
            {CG_TYPE::FLOAT, "pk_Volume_NoiseFogAmount"},
            {CG_TYPE::FLOAT, "pk_Volume_NoiseFogScale"},
            {CG_TYPE::FLOAT, "pk_Volume_WindSpeed"},
            //{CG_TYPE::HANDLE, "pk_Volume_Inject"},
            //{CG_TYPE::HANDLE, "pk_Volume_InjectRead"},
        }));

        auto bufferInject = m_volumes->GetColorBufferPtr(0);
        auto bufferScatter = m_volumes->GetColorBufferPtr(1);

        m_volumeResources->SetFloat4(StringHashID::StringToID("pk_Volume_WindDir"), float4(1.0f, 0.0f, 0.0f, 0.0f));
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_ConstantFog"), 0.0f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_HeightFogExponent"), 2.0f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_HeightFogOffset"), -1.0f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_HeightFogAmount"), 0.5f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_Density"), 1.0f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_Intensity"), 0.5f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_Anisotropy"), 0.5f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_NoiseFogAmount"), 1.5f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_NoiseFogScale"), 0.5f);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_WindSpeed"), 0.0f);
        m_volumeResources->FlushBufer();
        //m_volumeResources->SetResourceHandle(StringHashID::StringToID("pk_Volume_Inject"), bufferInject->GetImageHandleResident(GL_RGBA16F, GL_READ_WRITE, 0, 0, true));
        //m_volumeResources->SetResourceHandle(StringHashID::StringToID("pk_Volume_InjectRead"), bufferInject->GetTextureHandleResident());

        auto bluenoiseTex = assetDatabase->Find<TextureXD>("T_Bluenoise256_repeat").lock();
        m_properties.SetTexture(StringHashID::StringToID("pk_Bluenoise256"), bluenoiseTex->GetGraphicsID());

        m_properties.SetImage(StringHashID::StringToID("pk_Volume_Inject"), { bufferInject->GetGraphicsID(), GL_RGBA16F, GL_READ_WRITE, 0, 0, true });
        m_properties.SetImage(StringHashID::StringToID("pk_Volume_Scatter"), { bufferScatter->GetGraphicsID(), GL_RGBA16F, GL_READ_WRITE, 0, 0, true });
        m_properties.SetTexture(StringHashID::StringToID("pk_Volume_ScatterRead"), bufferScatter->GetGraphicsID());

        m_properties.SetConstantBuffer(StringHashID::StringToID("pk_VolumeResources"), m_volumeResources->GetGraphicsID());
    }
    
    void FilterVolumetricFog::OnPreRender(const RenderTexture* source)
    {
    }
    
    void FilterVolumetricFog::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        auto shaderComposite = m_shader.lock().get();
        auto computeInject = m_computeInject.lock().get();
        auto computeScatter = m_computeScatter.lock().get();
        auto groupsInject = uint3(VolumeResolution.x / InjectThreadCount.x, VolumeResolution.y / InjectThreadCount.y, VolumeResolution.z / InjectThreadCount.z);
        auto groupsScatter = uint3(VolumeResolution.x / ScatterThreadCount.x, VolumeResolution.y / ScatterThreadCount.y, 1);

        GraphicsAPI::DispatchCompute(computeInject, groupsInject, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        GraphicsAPI::DispatchCompute(computeScatter, groupsScatter, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        GraphicsAPI::Blit(destination, shaderComposite, m_properties);
    }
}