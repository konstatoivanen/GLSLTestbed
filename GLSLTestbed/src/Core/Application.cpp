#include "PrecompiledHeader.h"
#include "Utilities/Ref.h"
#include "Utilities/Log.h"
#include "Utilities/StringHashID.h"
#include "Utilities/HashCache.h"
#include "Core/Input.h"
#include "Core/UpdateStep.h"
#include "Core/Application.h"
#include "Core/ApplicationConfig.h"
#include "Rendering/RenderPipeline.h"
#include "Context/Engines/EngineEditorCamera.h"
#include "Context/Engines/DebugEngine.h"
#include <math.h>

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name)
{
	PK_CORE_ASSERT(!s_Instance, "Application already exists!");
	s_Instance = this;

	auto config = ApplicationConfig("res/Config.txt");

	::ShowWindow(::GetConsoleWindow(), config.enable_console ? SW_SHOW : SW_HIDE);

	Graphics::Initialize();
	
	m_window = CreateScope<Window>(WindowProperties(name, config.window_width, config.window_height, config.enable_vsync));
	m_services = CreateScope<ServiceRegister>();
	
	m_services->Create<StringHashID>();
	m_services->Create<HashCache>();
	
	auto sequencer = m_services->Create<PKECS::Sequencer>();
	auto time = m_services->Create<Time>(sequencer, config.time_scale);
	auto assetDatabase = m_services->Create<AssetDatabase>();
	auto input = m_services->Create<Input>(sequencer);
	
	m_window->OnKeyInput = PK_BIND_MEMBER_FUNCTION(input, OnKeyInput);
	m_window->OnScrollInput = PK_BIND_MEMBER_FUNCTION(input, OnScrollInput);
	m_window->OnMouseButtonInput = PK_BIND_MEMBER_FUNCTION(input, OnMouseButtonInput);
	m_window->OnClose = PK_BIND_FUNCTION(Application::Close);
	
	assetDatabase->LoadDirectory<Shader>("res/shaders/", { ".shader" });
	assetDatabase->LoadDirectory<Texture2D>("res/textures/", { ".png", ".ktx" });

	auto renderPipeline = m_services->Create<RenderPipeline>(assetDatabase);
	auto editorCameraEngine = m_services->Create<EngineEditorCamera>(time);
	auto debugEngine = m_services->Create<DebugEngine>(assetDatabase, time);
	
	sequencer->SetSteps(
	{
		{
			sequencer->GetRoot(),
			{
				{ (int)UpdateStep::OpenFrame, { PK_STEP_S(renderPipeline), time }},
				{ (int)UpdateStep::UpdateInput, { input } },
				{ (int)UpdateStep::PreRender, { PK_STEP_S(renderPipeline) }},
				{ (int)UpdateStep::Render, { PK_STEP_S(debugEngine) }},
				{ (int)UpdateStep::PostRender, { PK_STEP_S(renderPipeline) }},
				{ (int)UpdateStep::CloseFrame, { PK_STEP_S(renderPipeline) }},
			}
		},
		{ input, { PK_STEP_T(debugEngine, Input), PK_STEP_T(editorCameraEngine, Input) } },
		{ time, { PK_STEP_T(renderPipeline, Time) } }
	});

	sequencer->SetRootSequence(
	{ 
		(int)UpdateStep::OpenFrame,
		(int)UpdateStep::UpdateInput,
		(int)UpdateStep::UpdateEngines,
		(int)UpdateStep::PreRender,
		(int)UpdateStep::Render,
		(int)UpdateStep::PostRender,
		(int)UpdateStep::CloseFrame
	});
}

Application::~Application()
{
	m_window = nullptr;
	GetService<PKECS::Sequencer>()->Release();
	GetService<AssetDatabase>()->Unload();
	Graphics::Terminate();
	m_services->Clear();
}

void Application::Run()
{
	auto sequencer = GetService<PKECS::Sequencer>();

	while (m_window->IsAlive() && m_Running)
	{
		if (m_window->IsMinimized())
		{
			continue;
		}

		sequencer->ExecuteRootSequence();
	}
}

void Application::Close()
{
	m_Running = false;
}