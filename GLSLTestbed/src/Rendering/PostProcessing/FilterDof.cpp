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
        descriptor.dimension = GL_TEXTURE_2D_ARRAY;
        descriptor.resolution = { config->InitialWidth, config->InitialHeight, 2 };
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RGBA16F };
        descriptor.miplevels = 0;
        descriptor.filtermag = GL_LINEAR;
        descriptor.filtermin = GL_LINEAR;
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
        m_renderTarget = CreateRef<RenderTexture>(descriptor);

        m_autoFocusBuffer = CreateRef<ComputeBuffer>(BufferLayout(
        {
            {CG_TYPE::FLOAT2, "PARAMS"}
        }), 1, true, GL_NONE);

        m_paramsBuffer = CreateRef<ConstantBuffer>(BufferLayout(
        {
            {CG_TYPE::FLOAT, "pk_FocalLength"},
            {CG_TYPE::FLOAT, "pk_FNumber"},
            {CG_TYPE::FLOAT, "pk_FilmHeight"},
            {CG_TYPE::FLOAT, "pk_FocusSpeed"},
            {CG_TYPE::FLOAT, "pk_MaximumCoC"},
        }));

        m_paramsBuffer->SetFloat(HashCache::Get()->pk_FocalLength, config->CameraFocalLength);
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_FNumber, config->CameraFNumber);
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_FilmHeight, config->CameraFilmHeight);
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_FocusSpeed, config->CameraFocusSpeed);
        m_paramsBuffer->SetFloat(HashCache::Get()->pk_MaximumCoC, std::min(0.05f, 10.0f / config->InitialHeight));
        m_paramsBuffer->FlushBuffer();
        m_properties.SetConstantBuffer(HashCache::Get()->pk_DofParams, m_paramsBuffer->GetGraphicsID());
        m_properties.SetComputeBuffer(HashCache::Get()->pk_AutoFocusParams, m_autoFocusBuffer->GetGraphicsID());
    }
    
    void FilterDof::OnPreRender(const RenderTexture* source)
    {
        auto resolution = source->GetResolution3D();
        resolution.x /= 2;
        resolution.y /= 2;
        resolution.z  = 2;

        if (m_renderTarget->ValidateResolution(resolution))
        {
            m_paramsBuffer->SetFloat(HashCache::Get()->pk_MaximumCoC, std::min(0.05f, 10.0f / source->GetHeight()));
            m_paramsBuffer->FlushBuffer();
        }
    }
    
    void FilterDof::Execute(const RenderTexture* source, const RenderTexture* destination)
    {
        GraphicsAPI::DispatchCompute(m_shaderAutoFocus, { 1,1,1 }, m_properties);
        m_properties.SetKeywords({ m_passKeywords[0] });
        GraphicsAPI::Blit(source->GetColorBuffer(0), m_renderTarget.get(), m_shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        m_properties.SetKeywords({ m_passKeywords[1] });
        GraphicsAPI::Blit(m_renderTarget->GetColorBuffer(0), m_renderTarget.get(), m_shader, m_properties);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        GraphicsAPI::Blit(m_renderTarget->GetColorBuffer(0), destination, m_shaderComposite, m_properties);
    }
}