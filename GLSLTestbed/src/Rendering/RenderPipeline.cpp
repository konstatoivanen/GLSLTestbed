#include "PrecompiledHeader.h"
#include "Core/UpdateStep.h"
#include "Core/Application.h"
#include "Utilities/HashCache.h"
#include "Utilities/Utilities.h"
#include "Rendering/RenderPipeline.h"
#include "Rendering/Graphics.h"
#include "Rendering/MeshUtility.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"

namespace PK::Rendering
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Objects;
	using namespace PK::Math;

	static void SetOEMTextures(const GraphicsID texture, int probeSize, float exposure)
	{
		float OEMRoughnessLevels[] = { 0.0f, 0.33f, 0.66f, 1.0f };

		auto* hashCache = HashCache::Get();

		GraphicsAPI::SetGlobalTexture(hashCache->pk_SceneOEM_HDR, texture);
		GraphicsAPI::SetGlobalFloat(hashCache->pk_SceneOEM_RVS, &OEMRoughnessLevels[0], 4);
		GraphicsAPI::SetGlobalFloat4(hashCache->pk_SceneOEM_ST, { 0, 0, 1, 1 });
		GraphicsAPI::SetGlobalFloat(hashCache->pk_SceneOEM_Exposure, exposure);
	}

	static void UpdateLightsBuffer(ECS::EntityDatabase* entityDb, FrustumCuller& culler, Ref<ComputeBuffer>& lightsBuffer)
	{
		auto cullingResults = culler.GetCullingResults((int)ECS::Components::RenderHandleType::PointLight);
	
		if (cullingResults.count > 0)
		{
			lightsBuffer->ValidateSize((uint)cullingResults.count);
	
			auto buffer = lightsBuffer->BeginMapBuffer<Structs::PKPointLight>();
	
			for (size_t i = 0; i < cullingResults.count; ++i)
			{
				auto* view = entityDb->Query<ECS::EntityViews::PointLightRenderable>(ECS::EGID(cullingResults[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
				buffer[i] = { view->pointLight->color, float4(view->transform->position, view->pointLight->radius) };
			}
	
			lightsBuffer->EndMapBuffer();
			GraphicsAPI::SetGlobalComputeBuffer(HashCache::Get()->pk_Lights, lightsBuffer->GetGraphicsID());
			GraphicsAPI::SetGlobalInt(HashCache::Get()->pk_LightCount, (int)cullingResults.count);
		}
	}
	
	static void UpdateDynamicBatches(ECS::EntityDatabase* entityDb, FrustumCuller& culler, DynamicBatcher& batcher)
	{
		batcher.Reset();
	
		auto cullingResults = culler.GetCullingResults((int)ECS::Components::RenderHandleType::MeshRenderer);
	
		for (uint i = 0; i < cullingResults.count; ++i)
		{
			auto* view = entityDb->Query<ECS::EntityViews::MeshRenderable>(ECS::EGID(cullingResults[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
			auto* materials = &view->materials->sharedMaterials;
			auto mesh = view->mesh->sharedMesh;
	
			for (auto i = 0; i < materials->size(); ++i)
			{
				batcher.QueueDraw(mesh, i, materials->at(i), &view->transform->localToWorld, &view->transform->worldToLocal);
			}
		}
	
		batcher.UpdateBuffers();
	}
	
	RenderPipeline::RenderPipeline(AssetDatabase* assetDatabase, ECS::EntityDatabase* entityDb, const ApplicationConfig& config) :
		m_filterBloom(
			assetDatabase->Find<Shader>("SH_VS_FilterBloom"), 
			assetDatabase->Find<TextureXD>(config.FileBloomDirt.c_str()), 
			config.TonemapExposure, 
			config.BloomIntensity, 
			config.BloomLensDirtIntensity),
		m_filterAO(
			assetDatabase->Find<Shader>("SH_VS_FilterAO"),
			config.AmbientOcclusionIntensity, 
			config.AmbientOcclusionRadius, 
			config.AmbientOcclusionDownsample)
	{
		m_entityDb = entityDb;
		m_context.BlitQuad = MeshUtility::GetQuad2D({ -1.0f,-1.0f }, { 1.0f, 1.0f });
		m_context.BlitShader = assetDatabase->Find<Shader>("SH_VS_Internal_Blit");
	
		m_depthNormalsShader = assetDatabase->Find<Shader>("SH_WS_DepthNormals");
		m_OEMBackgroundShader = assetDatabase->Find<Shader>("SH_VS_IBLBackground");
		m_OEMTexture = assetDatabase->Find<TextureXD>(config.FileBackgroundTexture.c_str());
		m_OEMExposure = config.BackgroundExposure;
	
		auto renderTargetDescriptor = RenderTextureDescriptor();
		renderTargetDescriptor.colorFormats = { GL_RGBA16F };
		renderTargetDescriptor.depthFormat = GL_DEPTH24_STENCIL8;
		renderTargetDescriptor.resolution = { Application::GetWindow().GetWidth(), Application::GetWindow().GetHeight(), 0 };
		renderTargetDescriptor.filtermin = GL_NEAREST;
		renderTargetDescriptor.filtermag = GL_LINEAR;
		renderTargetDescriptor.wrapmodex = GL_CLAMP_TO_EDGE;
		renderTargetDescriptor.wrapmodey = GL_CLAMP_TO_EDGE;
		renderTargetDescriptor.dimension = GL_TEXTURE_2D;
	
		m_HDRRenderTarget = CreateRef<RenderTexture>(renderTargetDescriptor);

		renderTargetDescriptor.colorFormats = { GL_RGB16F };

		m_PreZRenderTarget = CreateRef<RenderTexture>(renderTargetDescriptor);
	
		m_constantsPerFrame = CreateRef<ConstantBuffer>(BufferLayout(
		{
			{CG_TYPE::FLOAT4, "pk_Time"},
			{CG_TYPE::FLOAT4, "pk_SinTime"},
			{CG_TYPE::FLOAT4, "pk_CosTime"},
			{CG_TYPE::FLOAT4, "pk_DeltaTime"},
			{CG_TYPE::FLOAT4, "pk_WorldSpaceCameraPos"},
			{CG_TYPE::FLOAT4, "pk_ProjectionParams"},
			{CG_TYPE::FLOAT4, "pk_ScreenParams"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_V"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_I_V"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_P"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_I_P"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_VP"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_I_VP"},
		}));
	
		m_lightsBuffer = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::FLOAT4, "COLOR"},
			{CG_TYPE::FLOAT4, "DIRECTION"}
		}), 32);
	}
	
	void RenderPipeline::Step(Time* timeRef)
	{
		auto* hashCache = HashCache::Get();
		auto time = timeRef->GetTime();
		auto deltatime = timeRef->GetDeltaTime();
		auto smoothdeltatime = timeRef->GetSmoothDeltaTime();
		m_constantsPerFrame->SetFloat4(hashCache->pk_Time, { time / 20, time, time * 2, time * 3 });
		m_constantsPerFrame->SetFloat4(hashCache->pk_SinTime, { sinf(time / 8), sinf(time / 4), sinf(time / 2), sinf(time) });
		m_constantsPerFrame->SetFloat4(hashCache->pk_CosTime, { cosf(time / 8), cosf(time / 4), cosf(time / 2), cosf(time) });
		m_constantsPerFrame->SetFloat4(hashCache->pk_DeltaTime, { deltatime, 1.0f / deltatime, smoothdeltatime, 1.0f / smoothdeltatime });
	}
	
	void RenderPipeline::Step(int condition)
	{
		auto step = (UpdateStep)condition;
	
		switch (step)
		{
			case UpdateStep::OpenFrame: GraphicsAPI::OpenContext(&m_context); break;
			case UpdateStep::PreRender: OnPreRender(); break;
			case UpdateStep::Render: OnRender(); break;
			case UpdateStep::PostRender: OnPostRender(); break;
			case UpdateStep::CloseFrame: GraphicsAPI::CloseContext(); break;
		}
	}
	
	void RenderPipeline::OnPreRender()
	{
		GraphicsAPI::StartWindow();
		GraphicsAPI::ResetResourceBindings();
	
		m_constantsPerFrame->CopyFrom(m_context.ShaderProperties);
		m_constantsPerFrame->FlushBufer();
		GraphicsAPI::SetGlobalConstantBuffer(HashCache::Get()->pk_PerFrameConstants, m_constantsPerFrame->GetGraphicsID());
		SetOEMTextures(m_OEMTexture.lock()->GetGraphicsID(), 1, m_OEMExposure);
	
		m_frustrumCuller.Update(m_entityDb, GraphicsAPI::GetActiveViewProjectionMatrix());
	
		UpdateLightsBuffer(m_entityDb, m_frustrumCuller, m_lightsBuffer);
		UpdateDynamicBatches(m_entityDb, m_frustrumCuller, m_dynamicBatcher);
	
		m_HDRRenderTarget->ValidateResolution(uint3(GraphicsAPI::GetActiveWindowResolution(), 0));
		m_PreZRenderTarget->ValidateResolution(uint3(GraphicsAPI::GetActiveWindowResolution(), 0));
	}
	
	void RenderPipeline::OnRender()
	{
		GraphicsAPI::SetRenderTarget(m_PreZRenderTarget);
		GraphicsAPI::Clear(CG_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto depthNormals = m_depthNormalsShader.lock();
		m_dynamicBatcher.Execute(0, depthNormals);

		m_filterAO.Execute(m_PreZRenderTarget, nullptr);

		GraphicsAPI::SetRenderTarget(m_HDRRenderTarget);
		GraphicsAPI::Clear(CG_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT);

		GraphicsAPI::CopyRenderTexture(m_PreZRenderTarget, m_HDRRenderTarget, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		GraphicsAPI::Blit(m_OEMBackgroundShader.lock());

		// @Todo Implement depth sorted index queue
		// @Todo Implement render passes
		m_dynamicBatcher.Execute(0);

		m_filterBloom.Execute(m_HDRRenderTarget, GraphicsAPI::GetBackBuffer());

		// Required for gizmos depth testing
		GraphicsAPI::CopyRenderTexture(m_HDRRenderTarget, nullptr, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}
	
	void RenderPipeline::OnPostRender()
	{
		GraphicsAPI::EndWindow();
	}
}