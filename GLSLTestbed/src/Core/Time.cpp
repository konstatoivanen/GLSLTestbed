#include "PrecompiledHeader.h"
#include "Core/Time.h"
#include <ctime>
#include <cstdlib>

static double GetClockSeconds() { return clock() / (double)CLOCKS_PER_SEC; } //glfwGetTime()

void Time::Reset()
{
    m_time = 0;
    m_unscaledTime = 0;
    m_previousSeconds = GetClockSeconds();
}

void Time::UpdateTime()
{
    auto currentSeconds = GetClockSeconds();

    m_unscaledDeltaTime = currentSeconds - m_previousSeconds;
    m_deltaTime = m_unscaledDeltaTime * m_timeScale;

    m_unscaledTime += m_unscaledDeltaTime;
    m_time += m_deltaTime;

    m_smoothDeltaTime = m_smoothDeltaTime + 0.5 * (m_deltaTime - m_smoothDeltaTime);

    m_previousSeconds = currentSeconds;
}