#include "PrecompiledHeader.h"
#include "Core/Application.h"
#include "Utilities/Ref.h"
#include "Utilities/StringHashID.h"
#include "Utilities/Log.h"
#include "Utilities/HashCache.h"
#include "Rendering/Objects/Texture2D.h"
#include "Rendering/MeshUtility.h"
#include "Rendering/LightingUtility.h"
#include <math.h>

Application* Application::s_Instance = nullptr;

enum class UpdateStep
{
	OpenFrame,
	UpdateInput,
	UpdateEngines,
	PreRender,
	Render,
	PostRender,
	CloseFrame,
};

class ShaderEngine : public PKECS::ISimpleStep
{
	public:
		ShaderEngine()
		{
			auto assetDatabase = Application::GetService<AssetDatabase>();

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
			cubeMesh = MeshUtilities::GetBox(CG_FLOAT3_ZERO, { 0.5f, 0.5f, 0.5f});
			cubeShader = assetDatabase->Find<Shader>("SH_WS_Default_Unlit");
			iblShader = assetDatabase->Find<Shader>("SH_VS_IBLBackground");

			fieldOfView = 60;
			position = { 0, 0, -2 };
			rotation = { 0, 0, 0 };

			reflectionMaps[0] = assetDatabase->Find<Texture2D>("T_OEM_01");
			reflectionMaps[1] = assetDatabase->Find<Texture2D>("T_OEM_02");
			reflectionMaps[2] = assetDatabase->Find<Texture2D>("T_OEM_03");

			reflectionMaps[0].lock()->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			reflectionMaps[1].lock()->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			reflectionMaps[2].lock()->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		}

		~ShaderEngine()
		{
			renderTarget = nullptr;
			cubeMesh = nullptr;
		}

		void Step() override
		{
			auto input = Application::GetService<Input>();
			auto assetDataBase = Application::GetService<AssetDatabase>();
			auto time = Application::GetService<Time>();

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
				assetDataBase->Reload<Shader>(cubeShader.lock()->GetAssetID());
				PK_CORE_LOG("Reimported shader: %s", cubeShader.lock()->GetFileName().c_str());
			}

			if (input->GetKey(KeyCode::C))
			{
				time->Reset();
			}

			time->LogFrameRate();

			Graphics::SetGlobalKeyword(StringHashID::StringToID("Test3"), input->GetKey(KeyCode::G));
			Graphics::SetGlobalKeyword(StringHashID::StringToID("Test6"), input->GetKey(KeyCode::F));

			auto deltaTime = time->GetDeltaTime();

			if (input->GetKey(KeyCode::MOUSE1))
			{
				rotation.x -= input->GetMouseDeltaY() * deltaTime * 2.5f;
				rotation.y -= input->GetMouseDeltaX() * deltaTime * 2.5f;
			}

			auto speed = input->GetKey(KeyCode::LEFT_SHIFT) ? 40.0f : 10.0f;
			auto offset = input->GetAxis3D(KeyCode::Q, KeyCode::E, KeyCode::W, KeyCode::S, KeyCode::D, KeyCode::A) * deltaTime * speed;
			
			auto fdelta = input->GetMouseScrollY() * deltaTime * 1000.0f;

			if (input->GetKey(KeyCode::LEFT_SHIFT))
			{
				auto fov0 = fieldOfView;
				auto fov1 = fieldOfView - fdelta;
				auto fd0 = CGMath::Cot(fov0 * CG_FLOAT_DEG2RAD * 0.5f);
				auto fd1 = CGMath::Cot(fov1 * CG_FLOAT_DEG2RAD * 0.5f);
				offset.z += (fd0 - fd1);
			}

			auto qrot = glm::quat(rotation);
			position += qrot * offset;

			fieldOfView -= fdelta;
			auto proj = CGMath::GetPerspective(fieldOfView, Application::GetWindow().GetAspect(), 0.1f, 250.0f);
			auto view = CGMath::GetMatrixInvTRS(position, qrot, CG_FLOAT3_ONE);

			Graphics::SetViewProjectionMatrices(view, proj);

			LightingUtility::SetOEMTextures(&reflectionMaps[0], 3, 1);
			Graphics::Blit(iblShader.lock());

			Graphics::DrawMesh(cubeMesh, cubeShader.lock(), CG_FLOAT4X4_IDENTITY);
			Graphics::DrawMesh(cubeMesh, cubeShader.lock(), CGMath::GetMatrixTRS({ 0.5f, 0.5f, 0.5f }, CG_QUATERNION_IDENTITY, CG_FLOAT3_ONE));
		}

	private:
		float3 position;
		float3 rotation;
		float fieldOfView;

		Weak<Shader> cubeShader;
		Weak<Shader> iblShader;
		Ref<Mesh> cubeMesh;
		Ref<RenderTexture> renderTarget;
		Weak<Texture2D> reflectionMaps[3];
};

// Move somewhere appropriate
static void SetShaderGlobals()
{
	auto timeService = Application::GetService<Time>();
	auto time = timeService->GetTime();
	auto deltatime = timeService->GetDeltaTime();
	auto smoothdeltatime = timeService->GetSmoothDeltaTime();
	Graphics::SetGlobalFloat4(HashCache::pk_Time, { time / 20, time, time * 2, time * 3 });
	Graphics::SetGlobalFloat4(HashCache::pk_SinTime, { sinf(time / 8), sinf(time / 4), sinf(time / 2), sinf(time) });
	Graphics::SetGlobalFloat4(HashCache::pk_CosTime, { cosf(time / 8), cosf(time / 4), cosf(time / 2), cosf(time) });
	Graphics::SetGlobalFloat4(HashCache::pk_DeltaTime, { deltatime, 1.0f / deltatime, smoothdeltatime, 1.0f / smoothdeltatime });
}

Application::Application(const std::string& name)
{
	PK_CORE_ASSERT(!s_Instance, "Application already exists!");
	s_Instance = this;

	auto config = ApplicationConfig("res/Config.txt");

	::ShowWindow(::GetConsoleWindow(), config.enable_console ? SW_SHOW : SW_HIDE);

	m_services = CreateScope<ServiceRegister>();

	StringHashID::SetCache(&m_hashCache);
	HashCache::Intitialize();
	Graphics::Initialize();
	
	auto time = m_services->CreateService<Time>();
	auto assetDatabase = m_services->CreateService<AssetDatabase>();
	auto input = m_services->CreateService<Input>();

	time->Reset();
	time->SetTimeScale(config.time_scale);
	
	m_window = CreateScope<Window>(WindowProperties(name, config.window_width, config.window_height, config.enable_vsync));
	m_window->OnKeyInput = PK_BIND_MEMBER_FUNCTION(input, OnKeyInput);
	m_window->OnScrollInput = PK_BIND_MEMBER_FUNCTION(input, OnScrollInput);
	m_window->OnMouseButtonInput = PK_BIND_MEMBER_FUNCTION(input, OnMouseButtonInput);
	m_window->OnClose = PK_BIND_FUNCTION(Application::Close);
	
	assetDatabase->LoadDirectory<Shader>("res/shaders/", ".shader");
	assetDatabase->LoadDirectory<Texture2D>("res/textures/", ".png");

	m_graphicsContext.BlitQuad = MeshUtilities::GetQuad2D({ -1.0f,-1.0f }, { 1.0f, 1.0f });
	m_graphicsContext.BlitShader = assetDatabase->Find<Shader>("SH_VS_Internal_Blit");

	auto shaderEngine = CreateRef<ShaderEngine>();

	m_sequencer.SetSteps(
		{
			{
				m_sequencer.GetRoot(),
				PKECS::To(
				{
					{ (int)UpdateStep::OpenFrame, { time }},
					{ (int)UpdateStep::UpdateInput, { input } },
					//{ (int)UpdateStep::UpdateEngines, { engine }},
					//{ (int)UpdateStep::PreRender, { engine }},
					{ (int)UpdateStep::Render, { shaderEngine }},
					//{ (int)UpdateStep::PostRender, { engine }},
					//{ (int)UpdateStep::CloseFrame, { engine }},
				})
			}
		}
	);
}

Application::~Application()
{
	m_window = nullptr;
	m_sequencer.Release();
	GetService<AssetDatabase>()->Unload();
	Graphics::Terminate();
	m_services->Clear();
}

void Application::Run()
{
	while (m_window->IsAlive() && m_Running)
	{
		if (m_window->IsMinimized())
		{
			continue;
		}

		m_sequencer.ExecuteCommand((int)UpdateStep::OpenFrame);
		m_sequencer.ExecuteCommand((int)UpdateStep::UpdateInput);
		Graphics::OpenContext(&m_graphicsContext);
		SetShaderGlobals();

		Graphics::StartWindow();
		m_sequencer.ExecuteCommand((int)UpdateStep::Render);
		Graphics::EndWindow();
	
		Graphics::CloseContext();
	}
}

void Application::Close()
{
	m_Running = false;
}