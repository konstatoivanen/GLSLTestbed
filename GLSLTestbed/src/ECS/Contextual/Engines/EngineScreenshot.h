#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Ref.h"
#include "Core/IService.h"
#include "Core/ConsoleCommandBinding.h"
#include "ECS/Sequencer.h"

namespace PK::ECS::Engines
{
	using namespace PK::Utilities;
	using namespace PK::Core;

	class EngineScreenshot : public IService, public IStep<ConsoleCommandToken>, public ISimpleStep
	{
		public:
			void Step(PK::Core::ConsoleCommandToken* token) override;
			void Step(int condition) override;

		private:
			uint m_captureFramesRemaining;
			uint m_captureFrameCount;
			uint2 m_captureResolution;
			std::vector<ushort> m_accumulatedPixels;
	};
}