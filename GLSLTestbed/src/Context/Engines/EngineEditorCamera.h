#pragma once
#include "Core/IService.h"
#include "Core/Sequencer.h"
#include "Core/Input.h"
#include "Core/Time.h"

class EngineEditorCamera : public IService, public PKECS::IStep<Input>
{
    public:
		EngineEditorCamera(Time* time);
		void Step(Input* input) override;

    private:
		Time* m_time = nullptr;
		float3 m_position = { 0, 0, -2 };
		float3 m_eulerAngles = CG_FLOAT3_ZERO;
		quaternion m_rotation = CG_QUATERNION_IDENTITY;
		float m_fieldOfView = 60.0f;
		float m_nearClip = 0.1f;
		float m_farClip = 250.0f;
};