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

	reflectionMaps[0] = assetDatabase->Find<Texture2D>("T_OEM_01");
	reflectionMaps[1] = assetDatabase->Find<Texture2D>("T_OEM_02");
	reflectionMaps[2] = assetDatabase->Find<Texture2D>("T_OEM_03");

	reflectionMaps[0].lock()->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	reflectionMaps[1].lock()->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	reflectionMaps[2].lock()->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
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
		iblShader.lock()->ListProperties();
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
	LightingUtility::SetOEMTextures(&reflectionMaps[0], 3, 1);
	Graphics::Blit(iblShader.lock());

	Graphics::DrawMesh(cubeMesh, cubeShader.lock(), CG_FLOAT4X4_IDENTITY);
	Graphics::DrawMesh(cubeMesh, cubeShader.lock(), CGMath::GetMatrixTRS({ 0.5f, 0.5f, 0.5f }, CG_QUATERNION_IDENTITY, CG_FLOAT3_ONE));
}
