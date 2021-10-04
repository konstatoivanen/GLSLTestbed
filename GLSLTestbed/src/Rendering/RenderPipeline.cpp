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
	
	RenderPipeline::RenderPipeline(AssetDatabase* assetDatabase, ECS::EntityDatabase* entityDb, const ApplicationConfig* config) :
		m_filterBloom(assetDatabase, config),
		m_filterDof(assetDatabase, config),
		m_filterAO(assetDatabase, config),
		m_filterFog(assetDatabase, config),
		m_filterSceneGi(assetDatabase, entityDb, config),
		m_lightsManager(assetDatabase, config)
	{
		m_entityDb = entityDb;
		m_context.BlitQuad = MeshUtility::GetQuad2D({ -1.0f,-1.0f }, { 1.0f, 1.0f });
		m_context.BlitShader = assetDatabase->Find<Shader>("SH_VS_Internal_Blit");

		m_depthNormalsShader = assetDatabase->Find<Shader>("SH_WS_DepthNormals");
		m_OEMBackgroundShader = assetDatabase->Find<Shader>("SH_VS_IBLBackground");
		m_OEMTexture = assetDatabase->Load<TextureXD>(config->FileBackgroundTexture.value.c_str());
		m_OEMExposure = config->BackgroundExposure;
	
		m_enableLightingDebug = config->EnableLightingDebug;
		m_logframerate = config->EnableFrameRateLog;

		auto renderTargetDescriptor = RenderTextureDescriptor();
		renderTargetDescriptor.colorFormats = { GL_RGBA16F };
		renderTargetDescriptor.depthFormat = GL_DEPTH24_STENCIL8;
		renderTargetDescriptor.resolution = { Application::GetWindow()->GetWidth(), Application::GetWindow()->GetHeight(), 0 };
		renderTargetDescriptor.filtermin = GL_NEAREST;
		renderTargetDescriptor.filtermag = GL_LINEAR;
		renderTargetDescriptor.wrapmodex = GL_CLAMP_TO_EDGE;
		renderTargetDescriptor.wrapmodey = GL_CLAMP_TO_EDGE;
		renderTargetDescriptor.dimension = GL_TEXTURE_2D;
	
		m_HDRRenderTarget = CreateRef<RenderTexture>(renderTargetDescriptor);

		renderTargetDescriptor.colorFormats = { GL_RGBA16F };

		m_GeometryBufferTarget = CreateRef<RenderTexture>(renderTargetDescriptor);
	
		m_constantsPerFrame = CreateRef<ConstantBuffer>(BufferLayout(
		{
			{PK_TYPE::FLOAT4, "pk_Time"},
			{PK_TYPE::FLOAT4, "pk_SinTime"},
			{PK_TYPE::FLOAT4, "pk_CosTime"},
			{PK_TYPE::FLOAT4, "pk_DeltaTime"},
			{PK_TYPE::FLOAT4, "pk_CursorParams"},
			{PK_TYPE::FLOAT4, "pk_WorldSpaceCameraPos"},
			{PK_TYPE::FLOAT4, "pk_ProjectionParams"},
			{PK_TYPE::FLOAT4, "pk_ExpProjectionParams"},
			{PK_TYPE::FLOAT4, "pk_ScreenParams"},
			{PK_TYPE::FLOAT4, "pk_ShadowCascadeZSplits"},
			{PK_TYPE::FLOAT4X4, "pk_MATRIX_V"},
			{PK_TYPE::FLOAT4X4, "pk_MATRIX_I_V"},
			{PK_TYPE::FLOAT4X4, "pk_MATRIX_P"},
			{PK_TYPE::FLOAT4X4, "pk_MATRIX_I_P"},
			{PK_TYPE::FLOAT4X4, "pk_MATRIX_VP"},
			{PK_TYPE::FLOAT4X4, "pk_MATRIX_I_VP"},
			{PK_TYPE::FLOAT4X4, "pk_MATRIX_L_VP"},
			{PK_TYPE::HANDLE, "pk_SceneOEM_HDR"},
			{PK_TYPE::HANDLE, "pk_ScreenNormals"},
			{PK_TYPE::HANDLE, "pk_ScreenDepth"},
			{PK_TYPE::HANDLE, "pk_ScreenGI_Diffuse"},
			{PK_TYPE::HANDLE, "pk_ScreenGI_Specular"},
			{PK_TYPE::HANDLE, "pk_ScreenOcclusion"},
			{PK_TYPE::HANDLE, "pk_ShadowmapAtlas"},
			{PK_TYPE::HANDLE, "pk_LightCookies"},
			{PK_TYPE::HANDLE, "pk_Bluenoise256"},
			{PK_TYPE::FLOAT, "pk_SceneOEM_Exposure"},
		}));

		auto bluenoiseTex = assetDatabase->Load<TextureXD>("res/textures/T_Bluenoise256_repeat.ktx");
		auto cookies = assetDatabase->Load<TextureXD>("res/textures/T_LightCookies.ktx");

		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_Bluenoise256, bluenoiseTex->GetBindlessHandleResident());
		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_LightCookies, cookies->GetBindlessHandleResident());
		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ScreenDepth, m_GeometryBufferTarget->GetDepthBuffer()->GetBindlessHandleResident());
		m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ScreenNormals, m_GeometryBufferTarget->GetColorBuffer(0)->GetBindlessHandleResident());
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

		if (m_logframerate)
		{
			timeRef->LogFrameRate();
		}
	}

	void RenderPipeline::Step(Input* inputRef)
	{
		m_constantsPerFrame->SetFloat4(HashCache::Get()->pk_CursorParams, { inputRef->GetMouseX(), inputRef->GetMouseY(), inputRef->GetMouseDeltaX(), inputRef->GetMouseDeltaY() });
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

	void RenderPipeline::Step(AssetImportToken<ApplicationConfig>* token)
	{
		m_enableLightingDebug = token->asset->EnableLightingDebug;
		m_logframerate = token->asset->EnableFrameRateLog;

		m_OEMTexture = token->assetDatabase->Load<TextureXD>(token->asset->FileBackgroundTexture.value.c_str());
		m_OEMExposure = token->asset->BackgroundExposure.value;
		m_filterFog.OnUpdateParameters(token->asset);
		m_filterBloom.OnUpdateParameters(token->asset);
		m_filterDof.OnUpdateParameters(token->asset);
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

		if (m_GeometryBufferTarget->ValidateResolution(uint3(resolution, 0)))
		{
			m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ScreenDepth, m_GeometryBufferTarget->GetDepthBuffer()->GetBindlessHandleResident());
			m_constantsPerFrame->SetResourceHandle(HashCache::Get()->pk_ScreenNormals, m_GeometryBufferTarget->GetColorBuffer(0)->GetBindlessHandleResident());
		}

		auto cascadeZSplits = m_lightsManager.GetCascadeZSplits(projParams.x, projParams.y);
		m_constantsPerFrame->SetFloat4(HashCache::Get()->pk_ShadowCascadeZSplits, reinterpret_cast<float4*>(cascadeZSplits.planes));

		m_filterAO.OnPreRender(m_GeometryBufferTarget.get());
		m_filterBloom.OnPreRender(m_HDRRenderTarget.get());
		m_filterDof.OnPreRender(m_HDRRenderTarget.get());
		m_filterSceneGi.OnPreRender(m_HDRRenderTarget.get());

		m_constantsPerFrame->CopyFrom(m_context.ShaderProperties);
		m_constantsPerFrame->FlushBuffer();
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
		GraphicsAPI::SetRenderTarget(m_GeometryBufferTarget.get());
		GraphicsAPI::Clear(PK_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		FixedStateAttributes depthNormalsAttributes;
		depthNormalsAttributes.BlendEnabled = false;
		depthNormalsAttributes.ColorMask = 255;
		depthNormalsAttributes.CullEnabled = true;
		depthNormalsAttributes.CullMode = GL_BACK;
		depthNormalsAttributes.ZTest = GL_LEQUAL;
		depthNormalsAttributes.ZTestEnabled = true;
		depthNormalsAttributes.ZWriteEnabled = true;

		Batching::DrawBatchesPredicated(&m_dynamicBatches, StringHashID::StringToID("PK_META_DEPTH_NORMALS"), m_depthNormalsShader, depthNormalsAttributes);
		
		m_lightsManager.UpdateLightTiles(m_GeometryBufferTarget->GetResolution2D());

		m_filterAO.Execute();
		m_filterSceneGi.Execute(&m_dynamicBatches);

		GraphicsAPI::SetRenderTarget(m_HDRRenderTarget.get());
		GraphicsAPI::Clear(PK_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT);

		GraphicsAPI::CopyRenderTexture(m_GeometryBufferTarget.get(), m_HDRRenderTarget.get(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		GraphicsAPI::Blit(m_OEMBackgroundShader);

		// @Todo Implement render passes
		Batching::DrawBatches(&m_dynamicBatches);

		m_filterFog.Execute(m_HDRRenderTarget.get(), m_HDRRenderTarget.get());
		m_filterDof.Execute(m_HDRRenderTarget.get(), m_HDRRenderTarget.get());
		m_filterBloom.Execute(m_HDRRenderTarget.get(), GraphicsAPI::GetBackBuffer());

		// Required for gizmos depth testing
		GraphicsAPI::CopyRenderTexture(m_HDRRenderTarget.get(), GraphicsAPI::GetBackBuffer(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		GraphicsAPI::ResetViewPort();

		if (m_enableLightingDebug)
		{
			m_lightsManager.DrawDebug();
		}
	}
}