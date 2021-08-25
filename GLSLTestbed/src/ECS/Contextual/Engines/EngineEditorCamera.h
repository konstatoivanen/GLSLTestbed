#pragma once
#include "Core/IService.h"
#include "ECS/Sequencer.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/ConsoleCommandBinding.h"
#include "Core/ApplicationConfig.h"

namespace PK::ECS::Engines
{
	using namespace PK::Math;
	using namespace PK::Core;

	class EngineEditorCamera : public IService, public IStep<Input>, public IStep<ConsoleCommandToken>
	{
	    public:
			EngineEditorCamera(Time* time, const ApplicationConfig* config);
			void Step(Input* input) override;
			void Step(ConsoleCommandToken* token) override;
	
	    private:
			Time* m_time = nullptr;
			float3 m_position = CG_FLOAT3_ZERO;
			float3 m_eulerAngles = CG_FLOAT3_ZERO;
			quaternion m_rotation = CG_QUATERNION_IDENTITY;
			float3 m_smoothPosition = CG_FLOAT3_ZERO;
			quaternion m_smoothRotation = CG_QUATERNION_IDENTITY;
			float m_fieldOfView = 60.0f;
			float m_zNear = 0.1f;
			float m_zFar = 250.0f;
			float m_moveSpeed = 5.0f;
			float m_moveSmoothing = 0.0f;
			float m_rotationSmoothing = 0.0f;
			float m_sensitivity = 1.0f;
	};
}