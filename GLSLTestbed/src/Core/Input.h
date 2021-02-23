#pragma once
#include "PrecompiledHeader.h"
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <hlslmath.h>

struct InputState
{
	int scancode;
	int action;
	int mods;
};

class Input
{
	public:
		bool GetKeyDown(GLint key);
		bool GetKeyUp(GLint key);
		bool GetKey(GLint key);

		float2 GetAxis2D(GLenum front, GLenum back, GLenum right, GLenum left);
		float3 GetAxis3D(GLenum up, GLenum down, GLenum front, GLenum back, GLenum right, GLenum left);
		float2 GetMouseDelta();
		float2 GetMousePosition();
		float2 GetMouseScroll();
		float GetMouseDeltaX();
		float GetMouseDeltaY();
		float GetMouseX();
		float GetMouseY();
		float GetMouseScrollX();
		float GetMouseScrollY();

		void PollInput();
		void OnKeyInput(int key, int scancode, int action, int mods);
		void OnScrollInput(double scrollX, double scrollY);
		void OnMouseButtonInput(int button, int action, int mods);

	public:
		std::unordered_map<int, InputState> m_inputStateCurrent;
		std::unordered_map<int, InputState> m_inputStatePrevious;
		float2 m_mousePrev = CG_FLOAT2_ZERO;
		float2 m_mouseDelta = CG_FLOAT2_ZERO;
		float2 m_mouseScrollRaw = CG_FLOAT2_ZERO;
		float2 m_mouseScroll = CG_FLOAT2_ZERO;
};
