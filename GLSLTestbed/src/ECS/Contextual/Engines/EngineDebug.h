#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Ref.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/UpdateStep.h"
#include "Core/AssetDataBase.h"
#include "ECS/EntityDatabase.h"
#include "Rendering/GizmoRenderer.h"
#include "Core/ApplicationConfig.h"

namespace PK::ECS::Engines
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Objects;

	class EngineDebug : public IService, public ISimpleStep, public IStep<Rendering::GizmoRenderer>
	{
		public:
			EngineDebug(AssetDatabase* assetDatabase, EntityDatabase* entityDb, const ApplicationConfig* config);
			void Step(int condition) override;
			void Step(Rendering::GizmoRenderer* gizmos) override;
	
		private:
			EntityDatabase* m_entityDb;
			AssetDatabase* m_assetDatabase;
	};
}