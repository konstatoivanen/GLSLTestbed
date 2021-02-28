#include "PrecompiledHeader.h"
#include "Core/UpdateStep.h"
#include "Utilities/HashCache.h"
#include "Rendering/RenderPipeline.h"
#include "Rendering/Graphics.h"
#include "Rendering/MeshUtility.h"

RenderPipeline::RenderPipeline(AssetDatabase* assetDatabase)
{
	m_context.BlitQuad = MeshUtilities::GetQuad2D({ -1.0f,-1.0f }, { 1.0f, 1.0f });
	m_context.BlitShader = assetDatabase->Find<Shader>("SH_VS_Internal_Blit");

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
	Graphics::SetGlobalConstantBuffer(StringHashID::StringToID("pk_PerFrameConstants"), m_constantsPerFrame->GetGraphicsID());
}

void RenderPipeline::OnRender()
{
}

void RenderPipeline::OnPostRender()
{
	Graphics::EndWindow();
}
