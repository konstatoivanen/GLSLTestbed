#include "PrecompiledHeader.h"
#include "FilterDof.h"
#include "Utilities/HashCache.h"
#include "Rendering/GraphicsAPI.h"

namespace PK::Rendering::PostProcessing
{
    FilterDof::FilterDof(AssetDatabase* assetDatabase, const ApplicationConfig* config) : FilterBase(assetDatabase->Find<Shader>("SH_VS_DOFBlur"))
    {
        m_passKeywords[0] = StringHashID::StringToID("PASS_PREFILTER");
        m_passKeywords[1] = StringHashID::StringToID("PASS_DISKBLUR");

        m_shaderComposite = assetDatabase->Find<Shader>("SH_VS_DOFComposite");
        m_shaderAutoFocus = assetDatabase->Find<Shader>("CS_AutoFocus");

        auto descriptor = RenderTextureDescriptor();
        descriptor.dimension = GL_TEXTURE_2D;
        descriptor.resolution = { config->InitialWidth, config->InitialHeight, 0 };
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RGBA16F };
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
        m_renderTarget0 = CreateRef<RenderTexture>(descriptor);


        descriptor.colorFormats = { GL_RGBA16F, GL_RGBA16F };
        m_renderTarget1 = CreateRef<RenderTexture>(descriptor);

        m_autoFocusBuffer = CreateRef<ComputeBuffer>(BufferLayout(
        {
            {PK_TYPE::FLOAT2, "PARAMS"}
        }), 1, true, GL_NONE);

        m_paramsBuffer = CreateRef<ConstantBuffer>(BufferLayout(
        {
            {PK_TYPE::HANDLE, "pk_Foreground"},
            {PK_TYPE::HANDLE, "pk_Background"},
            {PK_TYPE::FLOAT, "pk_FocalLength"},
            {PK_TYPE::FLOAT, "pk_FNumber"},
            {PK_TYPE::FLOAT, "pk_FilmHeight"},
            {PK_TYPE::FLOAT, "pk_FocusSpeed"},
            {PK_TYPE::FLOAT, "pk_MaximumCoC"},
        }));

        m_paramsBuffer->SetResourceHandle(StringHashID::StringToID("pk_Foreground"), m_renderTarget1->GetColorBuffer(0)->GetBindlessHandleResident());
        m_paramsBuffer->SetResourceHandle(StringHashID::StringToID("pk_Background"), m_renderTarget1->GetColorBuffer(1)->GetBindlessHandleResident());
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_MaximumCoC, std::min(0.05f, 10.0f / config->InitialHeight));
        OnUpdateParameters(config);
        m_properties.SetConstantBuffer(HashCache::Get()->pk_DofParams, m_paramsBuffer->GetGraphicsID());
        m_properties.SetComputeBuffer(HashCache::Get()->pk_AutoFocusParams, m_autoFocusBuffer->GetGraphicsID());
    }
    
    void FilterDof::OnPreRender(const RenderTexture* source)
    {
        auto resolution = source->GetResolution3D();
        resolution.x /= 2;
        resolution.y /= 2;
        resolution.z  = 0;

        m_renderTarget1->ValidateResolution(resolution);

        if (m_renderTarget0->ValidateResolution(resolution))
        {
            m_paramsBuffer->SetResourceHandle(StringHashID::StringToID("pk_Foreground"), m_renderTarget1->GetColorBuffer(0)->GetBindlessHandleResident());
            m_paramsBuffer->SetResourceHandle(StringHashID::StringToID("pk_Background"), m_renderTarget1->GetColorBuffer(1)->GetBindlessHandleResident());
            m_paramsBuffer->SetFloat(HashCache::Get()->pk_MaximumCoC, std::min(0.05f, 10.0f / source->GetHeight()));
            m_paramsBuffer->FlushBuffer();
        }
    }
    
    void FilterDof::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        GraphicsAPI::DispatchCompute(m_shaderAutoFocus, { 1,1,1 }, m_properties);

        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::Blit(source->GetColorBuffer(0), m_renderTarget0.get(), m_shader, m_properties);

        m_properties.SetKeywords({ m_passKeywords[1] });
        GraphicsAPI::Blit(m_renderTarget0->GetColorBuffer(0), m_renderTarget1.get(), m_shader, m_properties);
        GraphicsAPI::Blit(destination, m_shaderComposite, m_properties);
    }

    void FilterDof::OnUpdateParameters(const ApplicationConfig* config)
    {
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_FocalLength, config->CameraFocalLength);
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_FNumber, config->CameraFNumber);
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_FilmHeight, config->CameraFilmHeight);
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_FocusSpeed, config->CameraFocusSpeed);
        m_paramsBuffer->FlushBuffer();
    }
}