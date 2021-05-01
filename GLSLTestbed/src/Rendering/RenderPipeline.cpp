#include "PrecompiledHeader.h"
#include "Core/UpdateStep.h"
#include "Core/Application.h"
#include "Utilities/HashCache.h"
#include "Utilities/Utilities.h"
#include "Rendering/RenderPipeline.h"
#include "Rendering/GraphicsAPI.h"
#include "Rendering/MeshUtility.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"

namespace PK::Rendering
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Objects;
	using namespace PK::Math;

	static void SetOEMTextures(const TextureXD* texture, Ref<ConstantBuffer>& properties, int probeSize, float exposure)
	{
		float OEMRoughnessLevels[] = { 0.0f, 0.33f, 0.66f, 1.0f };

		auto* hashCache = HashCache::Get();

		properties->SetResourceHandle(hashCache->pk_SceneOEM_HDR, texture->GetBindlessHandleResident());
		//GraphicsAPI::SetGlobalFloat(hashCache->pk_SceneOEM_RVS, &OEMRoughnessLevels[0], 4);
		//GraphicsAPI::SetGlobalFloat4(hashCache->pk_SceneOEM_ST, { 0, 0, 1, 1 });
		properties->SetFloat(hashCache->pk_SceneOEM_Exposure, exposure);
	}
	
	static void UpdateDynamicBatches(ECS::EntityDatabase* entityDb, Culling::VisibilityCache& viscache, Batching::DynamicBatchCollection& batches)
	{
		Batching::ResetCollection(&batches);
	
		auto cullingResults = viscache.GetList(Culling::CullingGroup::CameraFrustum, (int)ECS::Components::RenderHandleFlags::Renderer);
	
		for (uint i = 0; i < cullingResults.count; ++i)
		{
			auto* view = entityDb->Query<ECS::EntityViews::MeshRenderable>(ECS::EGID(cullingResults[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
			auto* materials = &view->materials->sharedMaterials;
			auto mesh = view->mesh->sharedMesh;
	
			for (auto i = 0; i < materials->size(); ++i)
			{
				Batching::QueueDraw(&batches, mesh, i, materials->at(i), { &view->transform->localToWorld, 0.0f });
			}
		}
	
		Batching::UpdateBuffers(&batches);
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
			config.AmbientOcclusionDownsample),
		m_filterFog(assetDatabase, config),
		m_lightsManager(assetDatabase, config.CascadeLinearity)
	{
		m_entityDb = entityDb;
		m_context.BlitQuad = MeshUtility::GetQuad2D({ -1.0f,-1.0f }, { 1.0f, 1.0f });
		m_context.BlitShader = assetDatabase->Find<Shader>("SH_VS_Internal_Blit");

		m_depthNormalsShader = assetDatabase->Find<Shader>("SH_WS_DepthNormals");
		m_OEMBackgroundShader = assetDatabase->Find<Shader>("SH_VS_IBLBackground");
		m_OEMTexture = assetDatabase->Find<TextureXD>(config.FileBackgroundTexture.c_str());
		m_OEMExposure = config.BackgroundExposure;

		m_enableLightingDebug = config.EnableLightingDebug;
	
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
			{CG_TYPE::FLOAT4, "pk_ExpProjectionParams"},
			{CG_TYPE::FLOAT4, "pk_ScreenParams"},
			{CG_TYPE::FLOAT4, "pk_ShadowCascadeZSplits"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_V"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_I_V"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_P"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_I_P"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_VP"},
			{CG_TYPE::FLOAT4X4, "pk_MATRIX_I_VP"},
			{CG_TYPE::HANDLE, "pk_SceneOEM_HDR"},
			{CG_TYPE::HANDLE, "pk_ScreenNormals"},
			{CG_TYPE::HANDLE, "pk_ScreenDepth"},
			{CG_TYPE::HANDLE, "pk_ShadowmapAtlas"},
			{CG_TYPE::HANDLE, "pk_ScreenOcclusion"},
			{CG_TYPE::HANDLE, "pk_LightCookies"},
			{CG_TYPE::HANDLE, "pk_Bluenoise256"},
			{CG_TYPE::FLOAT, "pk_SceneOEM_Exposure"},
		}));

		auto bluenoiseTex = assetDatabase->Find<TextureXD>("T_Bluenoise256_repeat");
		auto cookies = assetDatabase->Find<TextureXD>("T_LightCookies");

		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_Bluenoise256, bluenoiseTex->GetBindlessHandleResident());
		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_LightCookies, cookies->GetBindlessHandleResident());
		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ScreenDepth, m_PreZRenderTarget->GetDepthBuffer()->GetBindlessHandleResident());
		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ScreenNormals, m_PreZRenderTarget->GetColorBuffer(0)->GetBindlessHandleResident());
		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ShadowmapAtlas, m_lightsManager.GetShadowmapAtlas()->GetColorBuffer(0)->GetBindlessHandleResident());
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
			case UpdateStep::PostRender: GraphicsAPI::EndWindow(); break;
			case UpdateStep::CloseFrame: GraphicsAPI::CloseContext(); break;
		}
	}
	
	void RenderPipeline::OnPreRender()
	{
		GraphicsAPI::StartWindow();
		GraphicsAPI::ResetResourceBindings();
		auto resolution = GraphicsAPI::GetActiveWindowResolution();
		const float4x4& inverseViewProjection = *m_context.ShaderProperties.GetPropertyPtr<float4x4>(HashCache::Get()->pk_MATRIX_I_VP);
		const float4 projParams = *m_context.ShaderProperties.GetPropertyPtr<float4>(HashCache::Get()->pk_ProjectionParams);

		SetOEMTextures(m_OEMTexture, m_constantsPerFrame, 1, m_OEMExposure);

		m_HDRRenderTarget->ValidateResolution(uint3(resolution, 0));

		if (m_PreZRenderTarget->ValidateResolution(uint3(resolution, 0)))
		{
			m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ScreenDepth, m_PreZRenderTarget->GetDepthBuffer()->GetBindlessHandleResident());
			m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ScreenNormals, m_PreZRenderTarget->GetColorBuffer(0)->GetBindlessHandleResident());
		}

		m_constantsPerFrame->SetFloat4(HashCache::Get()->pk_ShadowCascadeZSplits, m_lightsManager.GetCascadeZSplits(projParams.x, projParams.y));

		m_filterAO.OnPreRender(m_PreZRenderTarget.get());
		m_filterBloom.OnPreRender(m_HDRRenderTarget.get());

		m_constantsPerFrame->CopyFrom(m_context.ShaderProperties);
		m_constantsPerFrame->FlushBufer();
		GraphicsAPI::SetGlobalConstantBuffer(HashCache::Get()->pk_PerFrameConstants, m_constantsPerFrame->GetGraphicsID());
	
		Culling::ResetEntityVisibilities(m_entityDb);
		m_visibilityCache.Reset();
		
		Culling::BuildVisibilityCacheFrustum(m_entityDb, 
			&m_visibilityCache, 
			GraphicsAPI::GetActiveViewProjectionMatrix(), 
			Culling::CullingGroup::CameraFrustum, 
			(ushort)(ECS::Components::RenderHandleFlags::Renderer | ECS::Components::RenderHandleFlags::Light));
	
		UpdateDynamicBatches(m_entityDb, m_visibilityCache, m_dynamicBatches);

		m_lightsManager.Preprocess(
			m_entityDb, 
			m_visibilityCache.GetList(Culling::CullingGroup::CameraFrustum, (int)ECS::Components::RenderHandleFlags::Light), 
			resolution, 
			inverseViewProjection, 
			projParams.x, 
			projParams.y);
	}
	
	void RenderPipeline::OnRender()
	{
		GraphicsAPI::SetRenderTarget(m_PreZRenderTarget.get());
		GraphicsAPI::Clear(CG_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Batching::DrawBatches(&m_dynamicBatches, 0, m_depthNormalsShader);
		
		m_lightsManager.UpdateLightTiles(m_PreZRenderTarget->GetResolution2D());

		m_filterAO.Execute(m_PreZRenderTarget.get(), nullptr);

		GraphicsAPI::SetRenderTarget(m_HDRRenderTarget.get());
		GraphicsAPI::Clear(CG_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT);

		GraphicsAPI::CopyRenderTexture(m_PreZRenderTarget.get(), m_HDRRenderTarget.get(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		GraphicsAPI::Blit(m_OEMBackgroundShader);

		// @Todo Implement render passes
		Batching::DrawBatches(&m_dynamicBatches, 0);

		m_filterFog.Execute(m_HDRRenderTarget.get(), m_HDRRenderTarget.get());
		m_filterBloom.Execute(m_HDRRenderTarget.get(), GraphicsAPI::GetBackBuffer());

		// Required for gizmos depth testing
		GraphicsAPI::CopyRenderTexture(m_HDRRenderTarget.get(), nullptr, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		GraphicsAPI::ResetViewPort();

		if (m_enableLightingDebug)
		{
			m_lightsManager.DrawDebug();
		}
	}
}