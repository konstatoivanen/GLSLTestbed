#include "PrecompiledHeader.h"
#include "FilterAO.h"
#include "Rendering/GraphicsAPI.h"
#include "Utilities/HashCache.h"
#include "FilterFlowGraph.h"

namespace PK::Rendering::PostProcessing
{
    FilterFlowGraph::FilterFlowGraph(AssetDatabase* assetDatabase, const ApplicationConfig* config) : FilterBase(assetDatabase->Find<Shader>("SH_VS_FlowRender"))
    {
        m_shaderPaint = assetDatabase->Find<Shader>("CS_FlowPaint");
        m_shaderFlowIterate = assetDatabase->Find<Shader>("CS_FlowIterate");
        m_shaderBoidUpdate = assetDatabase->Find<Shader>("CS_FlowBoidUpdate");
        m_shaderDrawBoids = assetDatabase->Find<Shader>("SH_VS_FlowBoids");
        
        auto descriptor = RenderTextureDescriptor();
        descriptor.dimension = GL_TEXTURE_2D;
        descriptor.depthFormat = GL_NONE;
        descriptor.colorFormats = { GL_RG32UI };
        descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
        descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
        descriptor.resolution.x = 256;
        descriptor.resolution.y = 128;
        descriptor.resolution.z = 0;

        m_renderTarget = CreateRef<RenderTexture>(descriptor);
        m_boidsBuffer = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::FLOAT4, "POSITION"} }), 1024, true, GL_NONE);
    }

    void FilterFlowGraph::Execute(const RenderTexture* destination)
    {
        m_properties.SetUInt(StringHashID::StringToID("pk_Command"), m_command);
        m_properties.SetImage(StringHashID::StringToID("pk_FlowTexture"), m_renderTarget->GetColorBuffer(0)->GetImageBindDescriptor(GL_READ_WRITE, 0, 0, false));
        m_properties.SetComputeBuffer(StringHashID::StringToID("pk_Boids"), m_boidsBuffer->GetGraphicsID());

        if (m_command > 0)
        {
            GraphicsAPI::DispatchCompute(m_shaderPaint, { 1, 1, 1 }, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        GraphicsAPI::DispatchCompute(m_shaderFlowIterate, { 8, 4, 1 }, m_properties, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        GraphicsAPI::DispatchCompute(m_shaderBoidUpdate, { 32, 1, 1 }, m_properties, GL_SHADER_STORAGE_BARRIER_BIT);
        GraphicsAPI::Blit(destination, m_shader, m_properties);
        GraphicsAPI::BlitInstanced(0 ,1024, m_shaderDrawBoids, m_properties);
    }

    void FilterFlowGraph::OnInput(Input* input)
    {
        m_command = 0;

        if (input->GetKey(PK::Core::KeyCode::MOUSE1))
        {
            m_command = 1;
        }

        if (input->GetKey(PK::Core::KeyCode::MOUSE2))
        {
            m_command = 2;
        }
    }
}