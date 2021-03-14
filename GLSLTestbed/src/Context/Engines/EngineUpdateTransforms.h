#pragma once
#include "Core/IService.h"
#include "Core/Sequencer.h"
#include "Core/EntityDatabase.h"

class EngineUpdateTransforms : public IService, public PKECS::ISimpleStep
{
	public:
		EngineUpdateTransforms(PKECS::EntityDatabase* entityDb);
		void Step(int condition) override;
	
	private:
		PKECS::EntityDatabase* m_entityDb = nullptr;
};