#include "PrecompiledHeader.h"
#include "DebugEngine.h"
#include "ECS/Contextual/Implementers/Implementers.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"
#include "Rendering/MeshUtility.h"
#include "Core/Application.h"

namespace PK::ECS::Engines
{
	using namespace PK::Rendering::Objects;
	using namespace PK::Rendering::Structs;
	using namespace PK::Math;

	static EGID CreateMeshRenderable(EntityDatabase* entityDb, const float3& position, const float3& rotation, float size, Weak<Mesh> mesh, Weak<Material> material)
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
		implementer->flags = Components::RenderHandleFlags::Renderer;
		implementer->viewSize = 1.0f;
	
		return egid;
	}
	
	static void CreatePointLight(EntityDatabase* entityDb, PK::Core::AssetDatabase* assetDatabase, const float3& position, const color& color)
	{
		auto egid = EGID(entityDb->ReserveEntityId(), (uint)ENTITY_GROUPS::ACTIVE);
		auto implementer = entityDb->ResereveImplementer<Implementers::PointLightImplementer>();
		auto transformView = entityDb->ReserveEntityView<EntityViews::TransformView>(egid);
		auto baseView = entityDb->ReserveEntityView<EntityViews::BaseRenderable>(egid);
		auto lightView = entityDb->ReserveEntityView<EntityViews::PointLightRenderable>(egid);
		auto lightSphereView = entityDb->ReserveEntityView<EntityViews::LightSphere>(egid);
	
		transformView->bounds = static_cast<Components::Bounds*>(implementer);
		transformView->transform = static_cast<Components::Transform*>(implementer);
		baseView->bounds = static_cast<Components::Bounds*>(implementer);
		baseView->handle = static_cast<Components::RenderableHandle*>(implementer);
		lightView->pointLight = static_cast<Components::PointLight*>(implementer);
		lightView->transform = static_cast<Components::Transform*>(implementer);
		lightSphereView->transformLight = static_cast<Components::Transform*>(implementer);
	
		const auto intensityThreshold = 0.2f;
		const auto sphereRadius = 0.2f;
		const auto sphereTranslucency = 0.1f;
		auto lightColor = glm::exp(color);
		auto hdrColor = lightColor * sphereTranslucency * (1.0f / (sphereRadius * sphereRadius));

		float3 gammaColor = glm::pow(float3(lightColor.rgb), float3(1.0f / 2.2f));
		float intensity = glm::compMax(gammaColor);
		auto radius = intensity / intensityThreshold;
	
		implementer->localAABB = Functions::CreateBoundsCenterExtents(CG_FLOAT3_ZERO, CG_FLOAT3_ONE * radius);
		implementer->isCullable = true;
		implementer->isVisible = false;
		implementer->position = position;
		implementer->color = lightColor;
		implementer->radius = radius;
		implementer->flags = Components::RenderHandleFlags::Light;
		implementer->viewSize = 1.0f;
	
		auto mesh = assetDatabase->Find<Mesh>("Primitive_Sphere").lock();
		auto shader = assetDatabase->Find<Shader>("SH_WS_Unlit_Color").lock();
		auto material = assetDatabase->RegisterProcedural("M_Point_Light_" + std::to_string(egid.entityID()), CreateRef<Material>(shader));
		material.lock()->SetFloat4(StringHashID::StringToID("_Color"), hdrColor);
		
		auto meshEgid = CreateMeshRenderable(entityDb, position, CG_FLOAT3_ZERO, sphereRadius, mesh, material);
		auto meshView = entityDb->Query<EntityViews::TransformView>(meshEgid);
		lightSphereView->transformMesh = meshView->transform;
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
	
		auto minpos = float3(-40, -5, -40);
		auto maxpos = float3(40, 0, 40);

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
	
		for (uint i = 0; i < config.LightCount; ++i)
		{
			CreatePointLight(entityDb, assetDatabase, Functions::RandomRangeFloat3(minpos, maxpos), Functions::HueToRGB(Functions::RandomRangeFloat(0.0f, 1.0f)) * Functions::RandomRangeFloat(5.0f, 6.0f));
		}
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
			m_assetDatabase->Find<Shader>("SH_WS_PBR_Forward").lock()->ListProperties();
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
			auto ypos = sin(time + ((float)i * 4 / lights.count)) * 4;
			lights[i].transformLight->position.y = ypos;
			lights[i].transformMesh->position.y = ypos;
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
		auto aspect = Application::GetWindow().GetAspect();
		auto proj = Functions::GetPerspective(50.0f, aspect, 0.1f, 4.0f);
		// auto view = Functions::GetMatrixInvTRS(CG_FLOAT3_ZERO, glm::quat(float3(0, Application::GetService<Time>()->GetTime(), 0)), CG_FLOAT3_ONE);
		auto view = Functions::GetMatrixInvTRS(CG_FLOAT3_ZERO, CG_QUATERNION_IDENTITY, CG_FLOAT3_ONE);
		auto vp = proj * view;
	
		gizmos->SetColor(CG_COLOR_RED);
		gizmos->DrawFrustrum(vp);
		
		auto znear = 4.0f;
		auto zfar = 200.0f;

		for (int i = 0; i < 24; ++i)
		{
			float n = znear * pow(zfar / znear, (float)i / 24);
			float f = znear * pow(zfar / znear, (float)(i + 1) / 24);

			auto proj = Functions::GetOffsetPerspective(-1, 1, -1, 1, 50.0f, aspect, n, f);
			vp = proj * view;
			gizmos->DrawFrustrum(vp);
		}

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