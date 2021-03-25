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
#include "Rendering/GizmoRenderer.h"
#include "Rendering/RenderQueueInfo.h"
#include "ECS/Contextual/Engines/EngineEditorCamera.h"
#include "ECS/Contextual/Engines/DebugEngine.h"
#include "ECS/Contextual/Engines/EngineUpdateTransforms.h"
#include "Rendering/Graphics.h"
#include <math.h>

namespace PK::Core
{
	using namespace Utilities;
	using namespace Rendering::Objects;
	using namespace Rendering;

	Application* Application::s_Instance = nullptr;
	
	Application::Application(const std::string& name)
	{
		PK_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
	
		auto config = ApplicationConfig("res/ApplicationConfig.cfg");
	
		::ShowWindow(::GetConsoleWindow(), config.EnableConsole ? SW_SHOW : SW_HIDE);
	
		PK::Rendering::GraphicsAPI::Initialize();
		
		m_window = CreateScope<Window>(WindowProperties(name, config.WindowWidth, config.WindowHeight, config.EnableVsync));
		m_services = CreateScope<ServiceRegister>();
		
		m_services->Create<StringHashID>();
		m_services->Create<HashCache>();
		m_services->Create<RenderQueueInfo>();
		
		auto entityDb = m_services->Create<PK::ECS::EntityDatabase>();
		auto sequencer = m_services->Create<PK::ECS::Sequencer>();
		auto time = m_services->Create<Time>(sequencer, config.TimeScale);
		auto assetDatabase = m_services->Create<AssetDatabase>();
		auto input = m_services->Create<Input>(sequencer);
		
		m_window->OnKeyInput = PK_BIND_MEMBER_FUNCTION(input, OnKeyInput);
		m_window->OnScrollInput = PK_BIND_MEMBER_FUNCTION(input, OnScrollInput);
		m_window->OnMouseButtonInput = PK_BIND_MEMBER_FUNCTION(input, OnMouseButtonInput);
		m_window->OnClose = PK_BIND_FUNCTION(Application::Close);
		
		assetDatabase->LoadDirectory<Shader>("res/shaders/", { ".shader" });
		assetDatabase->LoadDirectory<TextureXD>("res/textures/", { ".png", ".ktx" });
		assetDatabase->LoadDirectory<Mesh>("res/models/", { ".obj" });
		assetDatabase->LoadDirectory<Material>("res/materials/", { ".material" });
	
		auto renderPipeline = m_services->Create<RenderPipeline>(assetDatabase, entityDb, config);
		auto engineEditorCamera = m_services->Create<ECS::Engines::EngineEditorCamera>(time, config);
		auto engineUpdateTransforms = m_services->Create<ECS::Engines::EngineUpdateTransforms>(entityDb);
		auto engineDebug = m_services->Create<ECS::Engines::DebugEngine>(assetDatabase, time, entityDb, config);
		auto gizmoRenderer = m_services->Create<GizmoRenderer>(sequencer, assetDatabase, config.EnableGizmos);
		
		sequencer->SetSteps(
		{
			{
				sequencer->GetRoot(),
				{
					{ (int)UpdateStep::OpenFrame,		{ PK_STEP_S(renderPipeline), time }},
					{ (int)UpdateStep::UpdateInput,		{ input } },
					{ (int)UpdateStep::UpdateEngines,	{ PK_STEP_S(engineDebug), PK_STEP_S(engineUpdateTransforms) }},
					{ (int)UpdateStep::PreRender,		{ PK_STEP_S(renderPipeline) }},
					{ (int)UpdateStep::Render,			{ PK_STEP_S(renderPipeline) }},
					{ (int)UpdateStep::PostRender,		{ PK_STEP_S(gizmoRenderer), PK_STEP_S(renderPipeline)}},
					{ (int)UpdateStep::CloseFrame,		{ PK_STEP_S(renderPipeline) }},
				}
			},
			{ input, { PK_STEP_T(engineDebug, Input), PK_STEP_T(engineEditorCamera, Input) } },
			{ time, { PK_STEP_T(renderPipeline, Time) } },
			{ gizmoRenderer, { PK_STEP_T(engineDebug, GizmoRenderer) }}
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
		GetService<ECS::Sequencer>()->Release();
		GetService<AssetDatabase>()->Unload();
		GraphicsAPI::Terminate();
		m_services->Clear();
	}
	
	void Application::Run()
	{
		auto sequencer = GetService<ECS::Sequencer>();
	
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
}