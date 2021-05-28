#include "PrecompiledHeader.h"
#include "FilterVolumetricFog.h"
#include "Rendering/GraphicsAPI.h"

namespace PK::Rendering::PostProcessing
{
    static const uint3 InjectThreadCount = { 16u, 2u, 16u };
    static const uint3 ScatterThreadCount = { 32u,2u,1u };
    static const uint3 VolumeResolution = { 160u, 90u, 128u };

    FilterVolumetricFog::FilterVolumetricFog(AssetDatabase* assetDatabase, const ApplicationConfig& config) : FilterBase(assetDatabase->Find<Shader>("SH_VS_VolumeFogComposite"))
    {
        TextureDescriptor descriptor;
        descriptor.dimension = GL_TEXTURE_3D;
        descriptor.colorFormat = GL_RGBA16F;
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
        descriptor.resolution = VolumeResolution;

        m_volumeLightDensity = CreateRef<RenderBuffer>(descriptor);
        m_volumeScatter = CreateRef<RenderBuffer>(descriptor);

        m_depthTiles = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::UINT, "DEPTHMAX"} }), VolumeResolution.x * VolumeResolution.y, true, GL_NONE);

        m_computeInject = assetDatabase->Find<Shader>("CS_VolumeFogLightDensity");
        m_computeScatter = assetDatabase->Find<Shader>("CS_VolumeFogScatter");
        m_computeDepthTiles = assetDatabase->Find<Shader>("CS_VolumeFogDepthMax");

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
            {CG_TYPE::HANDLE, "pk_Volume_ScatterRead"},
        }));

        m_volumeResources->SetFloat4(StringHashID::StringToID("pk_Volume_WindDir"), float4(1.0f, 0.0f, 0.0f, 0.0f));
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_ConstantFog"), config.VolumeConstantFog);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_HeightFogExponent"), config.VolumeHeightFogExponent);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_HeightFogOffset"), config.VolumeHeightFogOffset);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_HeightFogAmount"), config.VolumeHeightFogAmount);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_Density"), config.VolumeDensity);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_Intensity"), config.VolumeIntensity);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_Anisotropy"), config.VolumeAnisotropy);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_NoiseFogAmount"), config.VolumeNoiseFogAmount);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_NoiseFogScale"), config.VolumeNoiseFogScale);
        m_volumeResources->SetFloat(StringHashID::StringToID("pk_Volume_WindSpeed"), config.VolumeWindSpeed);
        m_volumeResources->SetResourceHandle(StringHashID::StringToID("pk_Volume_ScatterRead"), m_volumeScatter->GetBindlessHandleResident());
        m_volumeResources->FlushBuffer();

        m_properties.SetImage(StringHashID::StringToID("pk_Volume_Inject"), m_volumeLightDensity->GetImageBindDescriptor(GL_READ_WRITE, 0, 0, true));
        m_properties.SetImage(StringHashID::StringToID("pk_Volume_Scatter"), m_volumeScatter->GetImageBindDescriptor(GL_READ_WRITE, 0, 0, true));
        m_properties.SetConstantBuffer(StringHashID::StringToID("pk_VolumeResources"), m_volumeResources->GetGraphicsID());
        m_properties.SetComputeBuffer(StringHashID::StringToID("pk_VolumeMaxDepths"), m_depthTiles->GetGraphicsID());
    }
    
    void FilterVolumetricFog::OnPreRender(const RenderTexture* source)
    {
    }
    
    void FilterVolumetricFog::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        auto depthCountX = (uint)std::ceilf(source->GetWidth() / 32.0f);
        auto depthCountY = (uint)std::ceilf(source->GetHeight() / 32.0f);
        auto groupsInject = uint3(VolumeResolution.x / InjectThreadCount.x, VolumeResolution.y / InjectThreadCount.y, VolumeResolution.z / InjectThreadCount.z);
        auto groupsScatter = uint3(VolumeResolution.x / ScatterThreadCount.x, VolumeResolution.y / ScatterThreadCount.y, 1);

        m_depthTiles->Clear();
        GraphicsAPI::DispatchCompute(m_computeDepthTiles, { depthCountX, depthCountY, 1 }, m_properties, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        GraphicsAPI::DispatchCompute(m_computeInject, groupsInject, m_properties, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        GraphicsAPI::DispatchCompute(m_computeScatter, groupsScatter, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT);
        GraphicsAPI::Blit(destination, m_shader, m_properties);
    }
}