#pragma once

class Time
{
    public:
        const float GetTimeScale() const { return (float)m_timeScale; }
        void SetTimeScale(const float timeScale) { m_timeScale = (double)timeScale; }

        const float GetTime() const { return (float)m_time; }
        const float GetUnscaledTime() const { return (float)m_unscaledTime; }
        const float GetDeltaTime() const { return (float)m_deltaTime; }
        const float GetUnscaledDeltaTime() const { return (float)m_unscaledDeltaTime; }
        const float GetSmoothDeltaTime() const { return (float)m_smoothDeltaTime; }
        const uint64_t GetFrameIndex() const { return m_frameIndex; }
        const uint64_t GetFrameRate() const { return m_framerate; }
        const uint64_t GetFrameRateMin() const { return m_framerateMin; }
        const uint64_t GetFrameRateMax() const { return m_framerateMax; }
        const uint64_t GetFrameRateAvg() const { return m_framerateAvg; }
        const uint64_t GetFrameRateFixed() const { return m_framerateFixed; }

        void Reset();
        void UpdateTime();
        void LogFrameRate();

    private:
        uint64_t m_frameIndex;
        uint64_t m_frameIndexFixed;
        uint64_t m_framerate;
        uint64_t m_framerateMin;
        uint64_t m_framerateMax;
        uint64_t m_framerateAvg;
        uint64_t m_framerateFixed;
        uint64_t m_second;
        double m_previousSeconds;
        double m_timeScale;
        double m_time;
        double m_unscaledTime;
        double m_deltaTime;
        double m_unscaledDeltaTime;
        double m_smoothDeltaTime;
};

