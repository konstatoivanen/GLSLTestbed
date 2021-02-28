#pragma once
#include "Core/ServiceRegister.h"
#include "Core/Sequencer.h"

class Time : public IService, public PKECS::ISimpleStep
{
    public:
        Time(PKECS::Sequencer* sequencer, float timeScale);

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
        void LogFrameRate();

        void Step(int condition) override;

    private:
        PKECS::Sequencer* m_sequencer;

        uint64_t m_frameIndex = 0;
        uint64_t m_frameIndexFixed = 0;
        uint64_t m_framerate = 0;
        uint64_t m_framerateMin = (uint64_t)-1;
        uint64_t m_framerateMax = 0;
        uint64_t m_framerateAvg = 0;
        uint64_t m_framerateFixed = 0;
        uint64_t m_second = 0;
        double m_previousSeconds = 0.0;
        double m_timeScale = 0.0;
        double m_time = 0.0;
        double m_unscaledTime = 0.0;
        double m_deltaTime = 0.0;
        double m_unscaledDeltaTime = 0.0;
        double m_smoothDeltaTime = 0.0;
};

