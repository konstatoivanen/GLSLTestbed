#include "PrecompiledHeader.h"
#include "Core/UpdateStep.h"
#include "Core/Application.h"
#include "Utilities/HashCache.h"
#include "Rendering/RenderPipeline.h"
#include "Rendering/Graphics.h"
#include "Rendering/MeshUtility.h"
#include "Context/Structs/EntityViews.h"
#include "Rendering/LightingUtility.h"

static void UpdateLightsBuffer(PKECS::EntityDatabase* entityDb, FrustrumCuller& culler, Ref<ComputeBuffer>& lightsBuffer)
{
	auto cullingResults = culler.GetCullingResults((int)PKECS::Components::RenderHandleType::PointLight);

	if (cullingResults.count > 0)
	{
		lightsBuffer->ValidateSize((uint)cullingResults.count);

		auto buffer = lightsBuffer->BeginMapBuffer<PKStructs::PKPointLight>();

		for (size_t i = 0; i < cullingResults.count; ++i)
		{
			auto* view = entityDb->Query<PKECS::EntityViews::PointLightRenderable>(PKECS::EGID(cullingResults[i], (uint)PKECS::ENTITY_GROUPS::ACTIVE));
			buffer[i] = { view->pointLight->color, float4(view->transform->position, view->pointLight->radius) };
		}

		lightsBuffer->EndMapBuffer();
		Graphics::SetGlobalComputeBuffer(HashCache::Get()->pk_Lights, lightsBuffer->GetGraphicsID());
		Graphics::SetGlobalInt(HashCache::Get()->pk_LightCount, (int)cullingResults.count);
	}
}

static void UpdateDynamicBatches(PKECS::EntityDatabase* entityDb, FrustrumCuller& culler, DynamicBatcher& batcher)
{
	batcher.Reset();

	auto cullingResults = culler.GetCullingResults((int)PKECS::Components::RenderHandleType::MeshRenderer);

	for (uint i = 0; i < cullingResults.count; ++i)
	{
		auto* view = entityDb->Query<PKECS::EntityViews::MeshRenderable>(PKECS::EGID(cullingResults[i], (uint)PKECS::ENTITY_GROUPS::ACTIVE));
		auto* materials = &view->materials->sharedMaterials;
		auto mesh = view->mesh->sharedMesh;

		for (auto i = 0; i < materials->size(); ++i)
		{
			batcher.QueueDraw(mesh, i, materials->at(i), view->transform->localToWorld);
		}
	}

	batcher.UpdateBuffers();
}

RenderPipeline::RenderPipeline(AssetDatabase* assetDatabase, PKECS::EntityDatabase* entityDb)
{
	m_entityDb = entityDb;
	m_context.BlitQuad = MeshUtilities::GetQuad2D({ -1.0f,-1.0f }, { 1.0f, 1.0f });
	m_context.BlitShader = assetDatabase->Find<Shader>("SH_VS_Internal_Blit");

	m_OEMBackgroundShader = assetDatabase->Find<Shader>("SH_VS_IBLBackground");
	m_OEMTexture = assetDatabase->Find<TextureXD>("T_OEM_01");

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

	m_constantsPerFrame = CreateRef<ConstantBuffer>(BufferLayout(
	{
		{CG_TYPE_FLOAT4, "pk_Time"},
		{CG_TYPE_FLOAT4, "pk_SinTime"},
		{CG_TYPE_FLOAT4, "pk_CosTime"},
		{CG_TYPE_FLOAT4, "pk_DeltaTime"},
		{CG_TYPE_FLOAT4, "pk_WorldSpaceCameraPos"},
		{CG_TYPE_FLOAT4, "pk_ProjectionParams"},
		{CG_TYPE_FLOAT4, "pk_ScreenParams"},
		{CG_TYPE_FLOAT4, "pk_ZBufferParams"},
		{CG_TYPE_FLOAT4X4, "pk_MATRIX_V"},
		{CG_TYPE_FLOAT4X4, "pk_MATRIX_P"},
		{CG_TYPE_FLOAT4X4, "pk_MATRIX_VP"},
		{CG_TYPE_FLOAT4X4, "pk_MATRIX_I_VP"},
	}));

	m_lightsBuffer = CreateRef<ComputeBuffer>(BufferLayout(
	{
		{CG_TYPE_FLOAT4, "COLOR"},
		{CG_TYPE_FLOAT4, "DIRECTION"}
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
		case UpdateStep::OpenFrame: Graphics::OpenContext(&m_context); break;
		case UpdateStep::PreRender: OnPreRender(); break;
		case UpdateStep::Render: OnRender(); break;
		case UpdateStep::PostRender: OnPostRender(); break;
		case UpdateStep::CloseFrame: Graphics::CloseContext(); break;
	}
}

void RenderPipeline::OnPreRender()
{
	Graphics::StartWindow();
	m_constantsPerFrame->CopyFrom(m_context.ShaderProperties);
	m_constantsPerFrame->FlushBufer();
	Graphics::SetGlobalConstantBuffer(HashCache::Get()->pk_PerFrameConstants, m_constantsPerFrame->GetGraphicsID());
	LightingUtility::SetOEMTextures(m_OEMTexture.lock()->GetGraphicsID(), 1);

	m_frustrumCuller.Update(m_entityDb, Graphics::GetActiveViewProjectionMatrix());

	UpdateLightsBuffer(m_entityDb, m_frustrumCuller, m_lightsBuffer);
	UpdateDynamicBatches(m_entityDb, m_frustrumCuller, m_dynamicBatcher);

	m_HDRRenderTarget->ValidateResolution(uint3(Graphics::GetActiveWindowResolution(), 0));

	Graphics::SetRenderTarget(m_HDRRenderTarget);
	Graphics::Clear(CG_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Graphics::Blit(m_OEMBackgroundShader.lock());
}

void RenderPipeline::OnRender()
{
	// @Todo Implement depth sorted index queue
	// @Todo Implement render passes
	m_dynamicBatcher.Execute();
}

void RenderPipeline::OnPostRender()
{
	Graphics::Blit(m_HDRRenderTarget->GetColorBuffer(0).lock(), Graphics::GetBackBuffer());
	Graphics::EndWindow();
}

void FrustrumCuller::Update(PKECS::EntityDatabase* entityDb, const float4x4& matrix)
{
	FrustrumPlanes frustrum;
	CGMath::ExtractFrustrumPlanes(matrix, &frustrum, true);

	for (auto& kv : m_visibilityLists)
	{
		kv.second.count = 0;
	}

	auto cullables = entityDb->Query<PKECS::EntityViews::BaseRenderable>((int)PKECS::ENTITY_GROUPS::ACTIVE);

	for (auto i = 0; i < cullables.count; ++i)
	{
		auto cullable = &cullables[i];
		cullable->handle->isVisible = !cullable->handle->isCullable || CGMath::IntersectPlanesAABB(frustrum.planes, 6, cullable->bounds->worldAABB);

		if (cullable->handle->isVisible)
		{
			auto& visibilityList = m_visibilityLists[(uint)cullable->handle->type];
			auto& list = visibilityList.list;
			auto& count = visibilityList.count;
			CGMath::ValidateVectorSize(list, count + 1);
			list[count++] = cullable->GID.entityID();
		}
	}
}
