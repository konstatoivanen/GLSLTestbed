#include "PrecompiledHeader.h"
#include "EngineEditorCamera.h"
#include "Core/Application.h"
#include "Rendering/Graphics.h"

EngineEditorCamera::EngineEditorCamera(Time* time)
{
	m_time = time;
}

void EngineEditorCamera::Step(Input* input)
{
	auto deltaTime = m_time->GetDeltaTime();

	if (input->GetKey(KeyCode::MOUSE1))
	{
		m_eulerAngles.x -= input->GetMouseDeltaY() * deltaTime * 2.5f;
		m_eulerAngles.y -= input->GetMouseDeltaX() * deltaTime * 2.5f;
	}

	auto speed = input->GetKey(KeyCode::LEFT_SHIFT) ? 40.0f : 10.0f;
	auto offset = input->GetAxis3D(KeyCode::Q, KeyCode::E, KeyCode::W, KeyCode::S, KeyCode::D, KeyCode::A) * deltaTime * speed;

	auto fdelta = input->GetMouseScrollY() * deltaTime * 1000.0f;

	if (input->GetKey(KeyCode::LEFT_SHIFT))
	{
		auto fov0 = m_fieldOfView;
		auto fov1 = m_fieldOfView - fdelta;
		auto fd0 = CGMath::Cot(fov0 * CG_FLOAT_DEG2RAD * 0.5f);
		auto fd1 = CGMath::Cot(fov1 * CG_FLOAT_DEG2RAD * 0.5f);
		offset.z += (fd0 - fd1);
	}

	m_rotation = glm::quat(m_eulerAngles);
	m_position += m_rotation * offset;

	m_fieldOfView -= fdelta;

	auto proj = CGMath::GetPerspective(m_fieldOfView, Application::GetWindow().GetAspect(), 0.1f, 250.0f);
	auto view = CGMath::GetMatrixInvTRS(m_position, m_rotation, CG_FLOAT3_ONE);
	Graphics::SetViewProjectionMatrices(view, proj);
}
