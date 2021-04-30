#include "PrecompiledHeader.h"
#include "DebugEngine.h"
#include "ECS/Contextual/Implementers/Implementers.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"
#include "ECS/Contextual/Builders/Builders.h"
#include "Rendering/MeshUtility.h"
#include "Core/Application.h"

namespace PK::ECS::Engines
{
	using namespace PK::Rendering::Objects;
	using namespace PK::Rendering::Structs;
	using namespace PK::Math;

	static EGID CreateMeshRenderable(EntityDatabase* entityDb, const float3& position, const float3& rotation, float size, Mesh* mesh, Material* material)
	{
		auto egid = EGID(entityDb->ReserveEntityId(), (uint)ENTITY_GROUPS::ACTIVE);
		auto implementer = entityDb->ResereveImplementer<Implementers::MeshRenderableImplementer>();
		auto transformView = entityDb->ReserveEntityView<EntityViews::TransformView>(egid);
		auto baseView = entityDb->ReserveEntityView<EntityViews::BaseRenderable>(egid);
		auto meshView = entityDb->ReserveEntityView<EntityViews::MeshRenderable>(egid);
	
		transformView->bounds = static_cast<Components::Bounds*>(implementer);
		transformView->transform = static_cast<Components::Transform*>(implementer);
		baseView->bounds = static_cast<Components::Bounds*>(implementer);
		baseView->handle = static_cast<Components::RenderableHandle*>(implementer);
		meshView->materials = static_cast<Components::Materials*>(implementer);
		meshView->mesh = static_cast<Components::MeshReference*>(implementer);
		meshView->transform = static_cast<Components::Transform*>(implementer);
	
		implementer->localAABB = Functions::CreateBoundsCenterExtents(CG_FLOAT3_ZERO, CG_FLOAT3_ONE);
		implementer->isCullable = true;
		implementer->isVisible = false;
		implementer->position = position;
		implementer->rotation = glm::quat(rotation * CG_FLOAT_DEG2RAD);
		implementer->scale = CG_FLOAT3_ONE * size;
		implementer->sharedMaterials.push_back(material);
		implementer->sharedMesh = mesh;
		implementer->flags = Components::RenderHandleFlags::Renderer | Components::RenderHandleFlags::ShadowCaster;
		return egid;
	}
	
	static void CreateLight(EntityDatabase* entityDb, PK::Core::AssetDatabase* assetDatabase, const float3& position, const color& color, bool castShadows, LightType type, LightCookie cookie)
	{
		auto egid = EGID(entityDb->ReserveEntityId(), (uint)ENTITY_GROUPS::ACTIVE);
		auto implementer = entityDb->ResereveImplementer<Implementers::LightImplementer>();
		auto transformView = entityDb->ReserveEntityView<EntityViews::TransformView>(egid);
		auto baseView = entityDb->ReserveEntityView<EntityViews::BaseRenderable>(egid);
		auto lightView = entityDb->ReserveEntityView<EntityViews::LightRenderable>(egid);
		auto lightSphereView = entityDb->ReserveEntityView<EntityViews::LightSphere>(egid);
	
		transformView->bounds = static_cast<Components::Bounds*>(implementer);
		transformView->transform = static_cast<Components::Transform*>(implementer);
		baseView->bounds = static_cast<Components::Bounds*>(implementer);
		baseView->handle = static_cast<Components::RenderableHandle*>(implementer);
		lightView->light = static_cast<Components::Light*>(implementer);
		lightView->transform = static_cast<Components::Transform*>(implementer);
		lightSphereView->transformLight = static_cast<Components::Transform*>(implementer);
	
		implementer->position = position;
		ECS::Builders::InitializeLightValues(implementer, color, type, cookie, castShadows, 90.0f);

		const auto intensityThreshold = 0.2f;
		const auto sphereRadius = 0.2f;
		const auto sphereTranslucency = 0.1f;
		auto hdrColor = implementer->color * sphereTranslucency * (1.0f / (sphereRadius * sphereRadius));
	
		auto mesh = assetDatabase->Find<Mesh>("Primitive_Sphere");
		auto shader = assetDatabase->Find<Shader>("SH_WS_Unlit_Color");
		auto material = assetDatabase->RegisterProcedural("M_Point_Light_" + std::to_string(egid.entityID()), CreateRef<Material>(shader));
		material->SetFloat4(StringHashID::StringToID("_Color"), hdrColor);
		
		auto meshEgid = CreateMeshRenderable(entityDb, position, CG_FLOAT3_ZERO, sphereRadius, mesh, material);
		auto meshTransform = entityDb->Query<EntityViews::TransformView>(meshEgid);
		entityDb->Query<EntityViews::BaseRenderable>(meshEgid)->handle->flags = Components::RenderHandleFlags::Renderer;
		lightSphereView->transformMesh = meshTransform->transform;
	}
	
	static void CreateDirectionalLight(EntityDatabase* entityDb, PK::Core::AssetDatabase* assetDatabase, const float3& rotation, const color& color, bool castShadows)
	{
		auto egid = EGID(entityDb->ReserveEntityId(), (uint)ENTITY_GROUPS::ACTIVE);
		auto implementer = entityDb->ResereveImplementer<Implementers::LightImplementer>();
		auto transformView = entityDb->ReserveEntityView<EntityViews::TransformView>(egid);
		auto baseView = entityDb->ReserveEntityView<EntityViews::BaseRenderable>(egid);
		auto lightView = entityDb->ReserveEntityView<EntityViews::LightRenderable>(egid);

		transformView->bounds = static_cast<Components::Bounds*>(implementer);
		transformView->transform = static_cast<Components::Transform*>(implementer);
		baseView->bounds = static_cast<Components::Bounds*>(implementer);
		baseView->handle = static_cast<Components::RenderableHandle*>(implementer);
		lightView->light = static_cast<Components::Light*>(implementer);
		lightView->transform = static_cast<Components::Transform*>(implementer);
		implementer->position = CG_FLOAT3_ZERO;
		implementer->rotation = glm::quat(rotation * CG_FLOAT_DEG2RAD);
		
		ECS::Builders::InitializeLightValues(implementer, color, LightType::Directional, LightCookie::NoCookie, castShadows, 90.0f, 50.0f);

		implementer->color = color;
	}

	DebugEngine::DebugEngine(AssetDatabase* assetDatabase, Time* time, EntityDatabase* entityDb, const ApplicationConfig& config)
	{
		m_entityDb = entityDb;
		m_assetDatabase = assetDatabase;
		m_time = time;
	
		//meshCube = MeshUtilities::GetBox(CG_FLOAT3_ZERO, { 10.0f, 0.5f, 10.0f });
	
		auto sphereMesh = assetDatabase->RegisterProcedural<Mesh>("Primitive_Sphere", Rendering::MeshUtility::GetSphere(CG_FLOAT3_ZERO, 1.0f));
		auto planeMesh = assetDatabase->RegisterProcedural<Mesh>("Primitive_Plane16x16", Rendering::MeshUtility::GetPlane(CG_FLOAT2_ZERO, CG_FLOAT2_ONE, { 16, 16 }));
	
		auto materialMetal = assetDatabase->Find<Material>("M_Metal_Panel");
		auto materialGravel = assetDatabase->Find<Material>("M_Gravel");
		auto materialWood = assetDatabase->Find<Material>("M_Wood_Floor");
		cornellBoxMaterial = assetDatabase->Find<Material>("M_Metal_Panel");
		cornellBox = assetDatabase->Find<Mesh>("cornell_box");
	
		auto minpos = float3(0, -5, -40);
		auto maxpos = float3(40, 5, 40);

		srand(config.RandomSeed);

		CreateMeshRenderable(entityDb, float3(0,-5,0), { 90, 0, 0 }, 40.0f, planeMesh, materialWood);
		
		for (auto i = 0; i < 256; ++i)
		{
			CreateMeshRenderable(entityDb, Functions::RandomRangeFloat3(minpos, maxpos), Functions::RandomEuler(), 1.0f, sphereMesh, materialMetal);
		}
	
		for (auto i = 0; i < 256; ++i)
		{
			CreateMeshRenderable(entityDb, Functions::RandomRangeFloat3(minpos, maxpos), Functions::RandomEuler(), 1.0f, sphereMesh, materialGravel);
		}
	
		bool flipperinotyperino = false;

		for (uint i = 0; i < config.LightCount; ++i)
		{
			auto type = flipperinotyperino ? LightType::Point : LightType::Spot;
			auto cookie = flipperinotyperino ? LightCookie::NoCookie : LightCookie::Circle1;
			CreateLight(entityDb, assetDatabase, Functions::RandomRangeFloat3(minpos, maxpos), Functions::HueToRGB(Functions::RandomRangeFloat(0.0f, 1.0f)) * Functions::RandomRangeFloat(2.0f, 6.0f), true, type, cookie);
			flipperinotyperino ^= true;
		}

		CreateDirectionalLight(entityDb, assetDatabase, { 35, -35, 0 }, color(109.0f / 255.0f, 86.0f / 255.0f, 61.0f / 255.0f, 1.0f) * 2.5f, true);
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
			m_assetDatabase->Find<Shader>("SH_WS_PBR_Forward")->ListProperties();
		}
	
		if (input->GetKeyDown(KeyCode::R))
		{
			m_assetDatabase->ReloadDirectory<Shader>("res/shaders/", { ".shader" });
			PK_CORE_LOG("Reimported shaders!                                        ");
		}
	
		if (input->GetKey(KeyCode::C))
		{
			m_time->Reset();
		}
	
		m_time->LogFrameRate();
	}
	
	void DebugEngine::Step(int condition)
	{
		auto lights = m_entityDb->Query<EntityViews::LightSphere>((int)ENTITY_GROUPS::ACTIVE);
		auto time = Application::GetService<Time>()->GetTime();
	
		for (auto i = 0; i < lights.count; ++i)
		{
			// auto ypos = sin(time + ((float)i * 4 / lights.count)) * 4;
			auto rotation = glm::quat(float3(0, time + float(i), 0));
			lights[i].transformLight->rotation = rotation;
			lights[i].transformMesh->rotation = rotation;
		}

		return;
	
		auto meshes = m_entityDb->Query<EntityViews::MeshRenderable>((int)ENTITY_GROUPS::ACTIVE);
	
		for (auto i = 0; i < meshes.count; ++i)
		{
			meshes[i].transform->position.y = sin(time + (10 * (float)i / meshes.count)) * 10;
		}
	}
	
	void DebugEngine::Step(Rendering::GizmoRenderer* gizmos)
	{
		auto time = Application::GetService<Time>()->GetTime();
		auto aspect = Application::GetWindow().GetAspect();
		auto proj = Functions::GetPerspective(50.0f, aspect, 0.2f, 100.0f);
		auto view = Functions::GetMatrixInvTRS(CG_FLOAT3_ZERO, { 0, 0, 0 }, CG_FLOAT3_ONE);
		auto vp = proj * view;
	

		float4x4 localToWorld = Functions::GetMatrixTRS(CG_FLOAT3_ZERO, float3(35, -35, 0) * CG_FLOAT_DEG2RAD, CG_FLOAT3_ONE);
		float4x4 worldToLocal = glm::inverse(localToWorld);
		float4x4 invvp = glm::inverse(vp);
		float4x4 cascades[4];

		float minNear, maxFar;
		Functions::GetShadowCascadeMatrices(worldToLocal, invvp, 0.2f, 100.0f, 0.5f, -15.0f, 4, cascades, &minNear, &maxFar);

		for (auto i = 0; i < 4; ++i)
		{
			gizmos->SetColor(CG_COLOR_GREEN);
			gizmos->DrawFrustrum(cascades[i]);
		}

		gizmos->SetColor(CG_COLOR_RED);
		gizmos->DrawFrustrum(vp);

		auto znear = 0.2f;
		auto zfar = 100.0f;

		for (int i = 0; i < 4; ++i)
		{
			float n = Functions::CascadeDepth(znear, zfar, 0.5f, (float)i / 4);
			float f = Functions::CascadeDepth(znear, zfar, 0.5f, (float)(i + 1) / 4);

			auto proj = Functions::GetOffsetPerspective(-1, 1, -1, 1, 50.0f, aspect, n, f);
			vp = proj * view;
			gizmos->DrawFrustrum(vp);
		}
		return;

		auto cullables = m_entityDb->Query<EntityViews::BaseRenderable>((int)ENTITY_GROUPS::ACTIVE);
	
		gizmos->SetColor(CG_COLOR_GREEN);

		for (auto i = 0; i < cullables.count; ++i)
		{
			if (cullables[i].handle->flags != Components::RenderHandleFlags::Light)
			{
				continue;
			}

			const auto& bounds = cullables[i].bounds->worldAABB;
			gizmos->DrawWireBounds(bounds);
		}
	}
}