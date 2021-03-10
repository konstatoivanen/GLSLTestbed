#include "PrecompiledHeader.h"
#include "DebugEngine.h"
#include "Core/Application.h"

DebugEngine::DebugEngine(AssetDatabase* assetDatabase, Time* time)
{
	m_assetDatabase = assetDatabase;
	m_time = time;

	auto desc = RenderTextureDescriptor();

	desc.colorFormats = { GL_RGBA8 };
	desc.depthFormat = GL_DEPTH24_STENCIL8;
	desc.width = Application::GetWindow().GetWidth();
	desc.height = Application::GetWindow().GetHeight();
	desc.filtermin = GL_NEAREST;
	desc.filtermag = GL_LINEAR;
	desc.wrapmodex = GL_CLAMP_TO_EDGE;
	desc.wrapmodey = GL_CLAMP_TO_EDGE;
	desc.dimension = GL_TEXTURE_2D;

	renderTarget = CreateRef<RenderTexture>(desc);
	//meshCube = MeshUtilities::GetBox(CG_FLOAT3_ZERO, { 10.0f, 0.5f, 10.0f });
	meshSphere = MeshUtilities::GetSphere(CG_FLOAT3_ZERO, 1.0f);
	materialMetal = assetDatabase->Find<Material>("M_Metal_Panel");
	materialGravel = assetDatabase->Find<Material>("M_Gravel");
	iblShader = assetDatabase->Find<Shader>("SH_VS_IBLBackground");
	cornellBox = assetDatabase->Find<Mesh>("cornell_box");

	reflectionMap = assetDatabase->Find<Texture2D>("T_OEM_01");

	instanceMatrices = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE_FLOAT4X4, "Matrix" } }), 2);

	float4x4 matrices[] = { CG_FLOAT4X4_IDENTITY , CGMath::GetMatrixTRS({ 0.5f, 0.5f, 0.5f }, CG_QUATERNION_IDENTITY, CG_FLOAT3_ONE) };
	instanceMatrices->SetData(matrices, CG_TYPE_SIZE_FLOAT4X4 * 2);
}

DebugEngine::~DebugEngine()
{
	renderTarget = nullptr;
	meshSphere = nullptr;
}

void DebugEngine::Step(Input* input)
{
	if (input->GetKeyDown(KeyCode::ESCAPE))
	{
		Application::Get().Close();
		return;
	}

	if (input->GetKeyDown(KeyCode::T))
	{
		materialMetal.lock()->GetShader().lock()->ListProperties();
	}

	if (input->GetKeyDown(KeyCode::R))
	{
		auto shader = materialMetal.lock()->GetShader();
		m_assetDatabase->Reload<Shader>(shader);
		PK_CORE_LOG("Reimported shader: %s", shader.lock()->GetFileName().c_str());
	}

	if (input->GetKey(KeyCode::C))
	{
		m_time->Reset();
	}

	m_time->LogFrameRate();
}

void DebugEngine::Step(int condition)
{
	auto hashCache = HashCache::Get();

	Graphics::SetRenderTarget(renderTarget);
	Graphics::Clear(CG_COLOR_CLEAR, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	LightingUtility::SetOEMTextures(reflectionMap.lock()->GetGraphicsID(), 1);
	
	Graphics::SetGlobalComputeBuffer(hashCache->pk_InstancingData, instanceMatrices->GetGraphicsID());

	Graphics::Blit(iblShader.lock());

	Graphics::SetGlobalKeyword(hashCache->PK_ENABLE_INSTANCING, true);
	Graphics::DrawMeshInstanced(meshSphere, 0, 2, materialMetal.lock());
	Graphics::SetGlobalKeyword(hashCache->PK_ENABLE_INSTANCING, false);

	Graphics::DrawMesh(meshSphere, 0, materialGravel.lock(), CGMath::GetMatrixTRS({10.0f, 0.0f, 10.0f}, CG_QUATERNION_IDENTITY, CG_FLOAT3_ONE));

	auto count = cornellBox.lock()->GetSubmeshCount();
	auto matrix = CGMath::GetMatrixTRS(CG_FLOAT3_ZERO, CG_QUATERNION_IDENTITY, CG_FLOAT3_ONE * 0.01f);

	for (uint i = 0; i < count; ++i)
	{
		Graphics::DrawMesh(cornellBox.lock(), i, materialMetal.lock(), matrix);
	}

	Graphics::Blit(renderTarget->GetColorBuffer(0).lock(), Graphics::GetBackBuffer());
}
