#include "PrecompiledHeader.h"
#include "DebugEngine.h"

DebugEngine::DebugEngine(AssetDatabase* assetDatabase, Time* time)
{
	m_assetDatabase = assetDatabase;
	m_time = time;

	auto desc = RenderTextureDescriptor();

	desc.colorFormat = GL_RGBA8;
	desc.depthFormat = GL_DEPTH24_STENCIL8;
	desc.width = 512;
	desc.height = 512;
	desc.filtermin = GL_NEAREST;
	desc.filtermag = GL_LINEAR;
	desc.wrapmodex = GL_CLAMP_TO_EDGE;
	desc.wrapmodey = GL_CLAMP_TO_EDGE;
	desc.dimension = GL_TEXTURE_2D;

	renderTarget = CreateRef<RenderTexture>(desc);
	cubeMesh = MeshUtilities::GetBox(CG_FLOAT3_ZERO, { 0.5f, 0.5f, 0.5f });
	cubeShader = assetDatabase->Find<Shader>("SH_WS_Default_Unlit");
	iblShader = assetDatabase->Find<Shader>("SH_VS_IBLBackground");

	reflectionMap = assetDatabase->Find<Texture2D>("T_OEM_01");

	instanceMatrices = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE_FLOAT4X4, "Matrix" } }), 2);

	float4x4 matrices[] = { CG_FLOAT4X4_IDENTITY , CGMath::GetMatrixTRS({ 0.5f, 0.5f, 0.5f }, CG_QUATERNION_IDENTITY, CG_FLOAT3_ONE) };
	instanceMatrices->SetData(matrices, CG_TYPE_SIZE_FLOAT4X4 * 2);
}

DebugEngine::~DebugEngine()
{
	renderTarget = nullptr;
	cubeMesh = nullptr;
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
		cubeShader.lock()->ListProperties();
	}

	if (input->GetKeyDown(KeyCode::R))
	{
		m_assetDatabase->Reload<Shader>(cubeShader.lock()->GetAssetID());
		PK_CORE_LOG("Reimported shader: %s", cubeShader.lock()->GetFileName().c_str());
	}

	if (input->GetKey(KeyCode::C))
	{
		m_time->Reset();
	}

	m_time->LogFrameRate();

	Graphics::SetGlobalKeyword(StringHashID::StringToID("Test3"), input->GetKey(KeyCode::G));
	Graphics::SetGlobalKeyword(StringHashID::StringToID("Test6"), input->GetKey(KeyCode::F));
}

void DebugEngine::Step(int condition)
{
	LightingUtility::SetOEMTextures(reflectionMap.lock()->GetGraphicsID(), 1);
	
	Graphics::SetGlobalComputeBuffer(StringHashID::StringToID("pk_InstancingData"), instanceMatrices->GetGraphicsID());

	Graphics::Blit(iblShader.lock());

	Graphics::DrawMeshInstanced(cubeMesh, cubeShader.lock(), 2);
}
