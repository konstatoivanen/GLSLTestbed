#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Core/Time.h"
#include <ctime>
#include <cstdlib>

static double GetClockSeconds() { return clock() / (double)CLOCKS_PER_SEC; } //glfwGetTime()

Time::Time(PKECS::Sequencer* sequencer, float timeScale) : m_sequencer(sequencer), m_timeScale(timeScale)
{
}

void Time::Reset()
{
    m_time = 0;
    m_unscaledTime = 0;

    m_frameIndex = 0;
    m_frameIndexFixed = 0;
    m_framerate = 0;
    m_framerateMin = (uint64_t)-1;
    m_framerateMax = 0;
    m_framerateAvg = 0;
    m_framerateFixed = 0;
    m_second = 0;
}

void Time::LogFrameRate()
{
    PK_CORE_LOG_OVERWRITE("FPS: %4.1i, FIXED: %i, MIN: %i, MAX: %i, AVG: %i     ", m_framerate, m_framerateFixed, m_framerateMin, m_framerateMax, m_framerateAvg);
}

void Time::Step(int condition)
{
    auto currentSeconds = GetClockSeconds();

    m_unscaledDeltaTime = currentSeconds - m_previousSeconds;
    m_deltaTime = m_unscaledDeltaTime * m_timeScale;

    m_unscaledTime += m_unscaledDeltaTime;
    m_time += m_deltaTime;

    m_smoothDeltaTime = m_smoothDeltaTime + 0.5 * (m_deltaTime - m_smoothDeltaTime);

    m_previousSeconds = currentSeconds;

    ++m_frameIndex;

    if (m_unscaledDeltaTime > 0)
    {
        m_framerate = (uint64_t)(1.0 / m_unscaledDeltaTime);
    }

    if ((uint64_t)m_unscaledTime != m_second)
    {
        m_framerateFixed = m_frameIndex - m_frameIndexFixed;
        m_frameIndexFixed = m_frameIndex;
        m_second = (uint64_t)m_unscaledTime;
    }

    if (m_framerate < m_framerateMin)
    {
        m_framerateMin = m_framerate;
    }

    if (m_framerate > m_framerateMax)
    {
        m_framerateMax = m_framerate;
    }

    m_framerateAvg = (uint64_t)(m_frameIndex / m_unscaledTime);

    m_sequencer->Next<Time>(this, this, 0);
}
