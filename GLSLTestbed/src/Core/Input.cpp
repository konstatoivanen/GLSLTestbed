#include "PreCompiledHeader.h"
#include "Core/Input.h"
#include "Rendering/Graphics.h"

void Input::OnKeyInput(int key, int scancode, int action, int mods)
{
	auto& statecur = m_inputStateCurrent[key];
	statecur.scancode = scancode;
	statecur.action = action;
	statecur.mods = mods;
}

void Input::OnScrollInput(double scrollX, double scrollY)
{
	m_mouseScrollRaw.x = (float)scrollX;
	m_mouseScrollRaw.y = (float)scrollY;
}

void Input::OnMouseButtonInput(int button, int action, int mods)
{
	auto& statecur = m_inputStateCurrent[button];
	statecur.action = action;
	statecur.mods = mods;
}

bool Input::GetKeyDown(GLint key)
{
	auto statecur = m_inputStateCurrent[key];
	auto statepre = m_inputStatePrevious[key];
	return statecur.action == GLFW_PRESS && statepre.action == GLFW_RELEASE;
}

bool Input::GetKeyUp(GLint key)
{
	auto statecur = m_inputStateCurrent[key];
	auto statepre = m_inputStatePrevious[key];
	return statecur.action == GLFW_RELEASE && statepre.action == GLFW_PRESS;
}

bool Input::GetKey(GLint key)
{
	auto state = m_inputStateCurrent[key];
	return state.action == GLFW_PRESS || state.action == GLFW_REPEAT;
}

float2 Input::GetAxis2D(GLenum front, GLenum back, GLenum right, GLenum left)
{
	return
	{
		(GetKey(front) ? 1.0f : GetKey(back) ? -1.0f : 0.0f),
		(GetKey(right) ? 1.0f : GetKey(left) ? -1.0f : 0.0f)
	};
}

float3 Input::GetAxis3D(GLenum up, GLenum down, GLenum front, GLenum back, GLenum right, GLenum left)
{
	return
	{
		(GetKey(right) ? 1.0f : GetKey(left) ? -1.0f : 0.0f),
		(GetKey(up) ? 1.0f : GetKey(down) ? -1.0f : 0.0f),
		(GetKey(front) ? 1.0f : GetKey(back) ? -1.0f : 0.0f)
	};
}

float2 Input::GetMouseDelta()
{
	return m_mouseDelta;
}

float2 Input::GetMousePosition()
{
	double xpos, ypos;
	glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
	return { (float)xpos, (float)ypos };
}

float2 Input::GetMouseScroll()
{
	return m_mouseScroll;
}

float Input::GetMouseDeltaX()
{
	return GetMouseDelta().x;
}

float Input::GetMouseDeltaY()
{
	return GetMouseDelta().y;
}

float Input::GetMouseX()
{
	return GetMousePosition().x;
}

float Input::GetMouseY()
{
	return GetMousePosition().y;
}

float Input::GetMouseScrollX()
{
	return GetMouseScroll().x;
}

float Input::GetMouseScrollY()
{
	return GetMouseScroll().y;
}

void Input::PollInput()
{
	for (auto& statecur : m_inputStateCurrent)
	{
		auto& statepre = m_inputStatePrevious[statecur.first];
		statepre.scancode = statecur.second.scancode;
		statepre.action = statecur.second.action;
		statepre.mods = statecur.second.mods;
	}

	glfwPollEvents();

	auto mousecur = GetMousePosition();
	m_mouseDelta = mousecur - m_mousePrev;
	m_mousePrev = mousecur;

	m_mouseScroll = m_mouseScrollRaw;
	m_mouseScrollRaw = { 0, 0 };
}
