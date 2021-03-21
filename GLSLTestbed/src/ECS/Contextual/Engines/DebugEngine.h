#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Ref.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/UpdateStep.h"
#include "Core/AssetDataBase.h"
#include "ECS/EntityDatabase.h"
#include "Rendering/GizmoRenderer.h"

namespace PK::ECS::Engines
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Objects;

	class DebugEngine : public IService, public ISimpleStep, public IStep<Input>, public IStep<Rendering::GizmoRenderer>
	{
		public:
			DebugEngine(AssetDatabase* assetDatabase, Time* time, EntityDatabase* entityDb);
			~DebugEngine();
			void Step(Input* input) override;
			void Step(int condition) override;
			void Step(Rendering::GizmoRenderer* gizmos) override;
	
		private:
			EntityDatabase* m_entityDb;
			AssetDatabase* m_assetDatabase;
			Time* m_time;
		
			Weak<Mesh> cornellBox;
			Weak<Material> cornellBoxMaterial;
	};
}