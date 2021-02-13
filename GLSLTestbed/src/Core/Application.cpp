#include "PrecompiledHeader.h"
#include "Core/Application.h"
#include "Utilities/Ref.h"
#include "Utilities/StringHashID.h"
#include "Utilities/Log.h"
#include "Utilities/HashCache.h"
#include "Rendering/Texture2D.h"
#include "Rendering/MeshUtility.h"
#include "Rendering/LightingUtility.h"
#include <math.h>

Application* Application::s_Instance = nullptr;

class ShaderEngine
{
	public:
		ShaderEngine()
		{
			shaderHashIds = Application::GetShaderCollection().GetAllShaderHashIds();
			currentShaderIndex = 0;
			currentShader = Application::GetShaderCollection().Find(shaderHashIds.at(currentShaderIndex));

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

			cubeMesh = MeshUtilities::GetBox({ 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f});
			cubeShader = Application::GetShaderCollection().Find("SH_WS_Default_Unlit");
			iblShader = Application::GetShaderCollection().Find("SH_VS_IBLBackground");

			renderTarget = CreateRef<RenderTexture>(desc);

			fieldOfView = 60;
			position = { 0, 0, -2 };
			rotation = { 0, 0, 0 };

			reflectionMaps[0] = CreateRef<Texture2D>("res/T_OEM_01.png");
			reflectionMaps[1] = CreateRef<Texture2D>("res/T_OEM_02.png");
			reflectionMaps[2] = CreateRef<Texture2D>("res/T_OEM_03.png");
		}

		~ShaderEngine()
		{
			iblShader = nullptr;
			reflectionMaps[0] = nullptr;
			reflectionMaps[1] = nullptr;
			reflectionMaps[2] = nullptr;
			currentShader = nullptr;
			renderTarget = nullptr;
			cubeMesh = nullptr;
			cubeShader = nullptr;
			shaderHashIds.clear();
		}

		virtual void Update()
		{
			auto& input = Application::GetInput();

			if (input.GetKeyDown(Application::GetConfig().input_exit))
			{
				Application::Get().Close();
				return;
			}

			/*
			if (Input::GetKeyDown(Application::GetConfig().input_shader_list_uniforms))
			{
				currentShader->ListUniforms();
			}

			if (Input::GetKeyDown(Application::GetConfig().input_shader_reimport))
			{
				Application::GetShaderCollection().Reimport(currentShader);
				PK_CORE_LOG("Reimported shader: %s", currentShader->GetName().c_str());
			}

			if (Input::GetKeyDown(Application::GetConfig().input_shader_next))
			{
				currentShader = Application::GetShaderCollection().Find(shaderHashIds.at(currentShaderIndex));
				PK_CORE_LOG(currentShader->GetName().c_str());
				currentShaderIndex = (currentShaderIndex + 1u) % shaderHashIds.size();
			}

			if (Input::GetKeyDown(Application::GetConfig().input_timescale_increase))
			{
				Application::GetTime().SetTimeScale(Application::GetTime().GetTimeScale() + 0.2f);
				PK_CORE_LOG("Timescale is: %f", Application::GetTime().GetTimeScale());
			}

			if (Input::GetKeyDown(Application::GetConfig().input_timescale_decrease))
			{
				Application::GetTime().SetTimeScale(Application::GetTime().GetTimeScale() - 0.2f);
				PK_CORE_LOG("Timescale is: %f", Application::GetTime().GetTimeScale());
			}
			*/

			if (input.GetKeyDown(GLFW_KEY_T))
			{
				iblShader->ListUniforms();
			}

			if (input.GetKeyDown(GLFW_KEY_R))
			{
				Application::GetShaderCollection().Reimport(iblShader);
				PK_CORE_LOG("Reimported shader: %s", iblShader->GetName().c_str());
			}

			auto deltaTime = Application::GetTime().GetDeltaTime();

			if (input.GetKey(GLFW_MOUSE_BUTTON_1))
			{
				rotation.x -= input.GetMouseDeltaY() * deltaTime * 2.5f;
				rotation.y -= input.GetMouseDeltaX() * deltaTime * 2.5f;
			}

			auto speed = input.GetKey(GLFW_KEY_LEFT_SHIFT) ? 40.0f : 10.0f;
			auto offset = input.GetAxis3D(GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_A) * deltaTime * speed;
			
			auto fdelta = input.GetMouseScrollY() * deltaTime * 1000.0f;

			if (input.GetKey(GLFW_KEY_LEFT_SHIFT))
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

			GraphicsID atlases[] =
			{
				reflectionMaps[0]->GetGraphicsID(),
				reflectionMaps[1]->GetGraphicsID(),
				reflectionMaps[2]->GetGraphicsID()
			};

			LightingUtility::SetOEMTextures(atlases, 3, 1);
			Graphics::SetGlobalFloat4x4(StringHashID::StringToID("_IBLRotationMatrix"), glm::toMat4(qrot) * glm::scale(float3(Application::GetWindow().GetAspect(), 1, 2.0f)));
			Graphics::Blit(iblShader);

			Graphics::DrawMesh(cubeMesh, cubeShader, CG_FLOAT4X4_IDENTITY);
			Graphics::DrawMesh(cubeMesh, cubeShader, CGMath::GetMatrixTRS({ 0.5f, 0.5f, 0.5f }, CG_QUATERNION_IDENTITY, CG_FLOAT3_ONE));

			Graphics::SetRenderTarget(nullptr);
		}

	private:
		float3 position;
		float3 rotation;
		float fieldOfView;

		Ref<Texture2D> reflectionMaps[3];

		Ref<Mesh> cubeMesh;
		Ref<Shader> cubeShader;
		Ref<Shader> iblShader;
		Ref<RenderTexture> renderTarget;
		Ref<Shader> currentShader;
		std::vector<uint32_t> shaderHashIds;
		uint32_t currentShaderIndex = 0;
};

// Move somewhere approrpriate
static void SetShaderGlobals()
{
	auto time = Application::GetTime().GetTime();
	auto deltatime = Application::GetTime().GetDeltaTime();
	auto smoothdeltatime = Application::GetTime().GetSmoothDeltaTime();
	Graphics::SetGlobalFloat4(HashCache::pk_Time, { time / 20, time, time * 2, time * 3 });
	Graphics::SetGlobalFloat4(HashCache::pk_SinTime, { sinf(time / 8), sinf(time / 4), sinf(time / 2), sinf(time) });
	Graphics::SetGlobalFloat4(HashCache::pk_CosTime, { cosf(time / 8), cosf(time / 4), cosf(time / 2), cosf(time) });
	Graphics::SetGlobalFloat4(HashCache::pk_DeltaTime, { deltatime, 1.0f / deltatime, smoothdeltatime, 1.0f / smoothdeltatime });
}

Application::Application(const std::string& name) : m_applicationConfig("res/Config.txt")
{
	PK_CORE_ASSERT(!s_Instance, "Application already exists!");
	s_Instance = this;

	::ShowWindow(::GetConsoleWindow(), m_applicationConfig.enable_console ? SW_SHOW : SW_HIDE);
	HashCache::Intitialize();
	Graphics::Initialize();
	m_time.Reset();
	m_time.SetTimeScale(m_applicationConfig.time_scale);
	m_window = CreateScope<Window>(WindowProperties(name));
	m_window->OnKeyInput = PK_BIND_FUNCTION(m_input.OnKeyInput);
	m_window->OnScrollInput = PK_BIND_FUNCTION(m_input.OnScrollInput);
	m_window->OnMouseButtonInput = PK_BIND_FUNCTION(m_input.OnMouseButtonInput);
	m_window->OnClose = PK_BIND_FUNCTION(Application::Close);
	m_shaderCollection.ImportCollection("res/Shaders.txt");
	m_graphicsContext.BlitQuad = MeshUtilities::GetQuad2D({ -1.0f,-1.0f }, { 1.0f, 1.0f });
	m_graphicsContext.BlitShader = Application::GetShaderCollection().Find("SH_VS_Internal_Blit");
}

Application::~Application()
{
	m_window = nullptr;
	m_shaderCollection.Release();
	Graphics::Terminate();
}

void Application::Run()
{
	auto shaderEngine = CreateRef<ShaderEngine>();

	while (m_window->IsAlive() && m_Running)
	{
		if (m_window->IsMinimized())
		{
			continue;
		}

		m_time.UpdateTime();
		m_input.PollInput();
		Graphics::StartFrame(&m_graphicsContext);
		SetShaderGlobals();
		shaderEngine->Update();
		Graphics::EndFrame(m_window->GetNativeWindow());
	}
}

void Application::Close()
{
	m_Running = false;
}