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
}

void RenderPipeline::Step(Time* timeRef)
{
	auto* hashCache = HashCache::Get();
	auto time = timeRef->GetTime();
	auto deltatime = timeRef->GetDeltaTime();
	auto smoothdeltatime = timeRef->GetSmoothDeltaTime();
	Graphics::SetGlobalFloat4(hashCache->pk_Time, { time / 20, time, time * 2, time * 3 });
	Graphics::SetGlobalFloat4(hashCache->pk_SinTime, { sinf(time / 8), sinf(time / 4), sinf(time / 2), sinf(time) });
	Graphics::SetGlobalFloat4(hashCache->pk_CosTime, { cosf(time / 8), cosf(time / 4), cosf(time / 2), cosf(time) });
	Graphics::SetGlobalFloat4(hashCache->pk_DeltaTime, { deltatime, 1.0f / deltatime, smoothdeltatime, 1.0f / smoothdeltatime });
}

void RenderPipeline::Step(int condition)
{
	auto step = (UpdateStep)condition;

	switch (step)
	{
		case UpdateStep::OpenFrame: Graphics::OpenContext(&m_context); break;
		case UpdateStep::PreRender: Graphics::StartWindow(); break;
		case UpdateStep::PostRender: Graphics::EndWindow(); break;
		case UpdateStep::CloseFrame: Graphics::CloseContext(); break;
	}
}
