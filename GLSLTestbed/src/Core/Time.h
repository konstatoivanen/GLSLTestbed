#pragma once

class Time
{
    public:
        const float GetTimeScale() { return (float)m_timeScale; }
        void SetTimeScale(const float timeScale) { m_timeScale = (double)timeScale; }

        const float GetTime() { return (float)m_time; }
        const float GetUnscaledTime() { return (float)m_unscaledTime; }
        const float GetDeltaTime() { return (float)m_deltaTime; }
        const float GetUnscaledDeltaTime() { return (float)m_unscaledDeltaTime; }
        const float GetSmoothDeltaTime() { return (float)m_smoothDeltaTime; }

        void Reset();
        void UpdateTime();

    private:
        double m_previousSeconds;
        double m_timeScale;
        double m_time;
        double m_unscaledTime;
        double m_deltaTime;
        double m_unscaledDeltaTime;
        double m_smoothDeltaTime;
};

