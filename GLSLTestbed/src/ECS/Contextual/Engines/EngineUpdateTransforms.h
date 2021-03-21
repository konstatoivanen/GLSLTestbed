#pragma once
#include "Core/IService.h"
#include "ECS/Sequencer.h"
#include "ECS/EntityDatabase.h"

namespace PK::ECS::Engines
{
	class EngineUpdateTransforms : public IService, public ISimpleStep
	{
		public:
			EngineUpdateTransforms(EntityDatabase* entityDb);
			void Step(int condition) override;
		
		private:
			EntityDatabase* m_entityDb = nullptr;
	};
}