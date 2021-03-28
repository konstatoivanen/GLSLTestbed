#pragma once
#include "Core/IService.h"
#include "ECS/Sequencer.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/ApplicationConfig.h"

namespace PK::ECS::Engines
{
	using namespace PK::Math;
	using namespace PK::Core;

	class EngineEditorCamera : public IService, public IStep<Input>
	{
	    public:
			EngineEditorCamera(Time* time, const ApplicationConfig& config);
			void Step(Input* input) override;
	
	    private:
			Time* m_time = nullptr;
			float3 m_position = { 0, 40, 0 };
			float3 m_eulerAngles = { 90 * CG_FLOAT_DEG2RAD, 0, 0 };
			quaternion m_rotation = CG_QUATERNION_IDENTITY;
			float m_fieldOfView = 60.0f;
			float m_zNear = 0.1f;
			float m_zFar = 250.0f;
			float m_moveSpeed = 5.0f;
	};
}