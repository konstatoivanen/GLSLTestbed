#include "PrecompiledHeader.h"
#include "EngineEditorCamera.h"
#include "Core/Application.h"
#include "Rendering/GraphicsAPI.h"

namespace PK::ECS::Engines
{
	using namespace PK::Math;

	EngineEditorCamera::EngineEditorCamera(Time* time, const ApplicationConfig& config)
	{
		m_time = time;
		m_moveSpeed = config.CameraSpeed;
		m_fieldOfView = config.CameraFov;
		m_zNear = config.CameraZNear;
		m_zFar = config.CameraZFar;
		m_moveSmoothing = glm::clamp(config.CameraMoveSmoothing, 0.0f, 1.0f);
		m_rotationSmoothing = glm::clamp(config.CameraLookSmoothing, 0.0f, 1.0f);
		m_sensitivity = config.CameraLookSensitivity / 1000.0f;
	}
	
	void EngineEditorCamera::Step(Input* input)
	{
		auto deltaTime = m_time->GetDeltaTime();
		deltaTime = glm::max(0.001f, deltaTime);
	
		if (input->GetKey(KeyCode::MOUSE1))
		{
			m_eulerAngles.x -= input->GetMouseDeltaY() * m_sensitivity;
			m_eulerAngles.y -= input->GetMouseDeltaX() * m_sensitivity;
		}
	
		auto speed = input->GetKey(KeyCode::LEFT_CONTROL) ? (m_moveSpeed * 0.25f) : input->GetKey(KeyCode::LEFT_SHIFT) ? (m_moveSpeed * 5) : m_moveSpeed;
		auto offset = input->GetAxis3D(KeyCode::Q, KeyCode::E, KeyCode::W, KeyCode::S, KeyCode::D, KeyCode::A) * deltaTime * speed;
	
		auto fdelta = input->GetMouseScrollY() * deltaTime * 1000.0f;
	
		if (input->GetKey(KeyCode::LEFT_SHIFT))
		{
			auto fov0 = m_fieldOfView;
			auto fov1 = m_fieldOfView - fdelta;
			auto fd0 = Functions::Cot(fov0 * CG_FLOAT_DEG2RAD * 0.5f);
			auto fd1 = Functions::Cot(fov1 * CG_FLOAT_DEG2RAD * 0.5f);
			offset.z += (fd0 - fd1);
		}
	
		m_rotation = glm::quat(m_eulerAngles);
		m_position += m_rotation * offset;
	
		m_fieldOfView -= fdelta;
	
		m_smoothPosition = glm::mix(m_position, m_smoothPosition, m_moveSmoothing * (1.0f - deltaTime));
		m_smoothRotation = glm::slerp(m_rotation, m_smoothRotation, m_rotationSmoothing * (1.0f - deltaTime));

		auto proj = Functions::GetPerspective(m_fieldOfView, Application::GetWindow().GetAspect(), m_zNear, m_zFar);
		auto view = Functions::GetMatrixInvTRS(m_smoothPosition, m_smoothRotation, CG_FLOAT3_ONE);
		Rendering::GraphicsAPI::SetViewProjectionMatrices(view, proj);
	}
}