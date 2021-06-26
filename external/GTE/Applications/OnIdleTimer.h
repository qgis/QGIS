// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace gte
{
    class OnIdleTimer
    {
    public:
        // The 'frequency' is how many calls of OnIdle() occur before the
        // high-frequency clock is called.
        OnIdleTimer(int64_t frequency = 30);

        // Restart the timer from zero.  This is useful to call after the
        // application has "warmed up."
        void Reset();

        // Call this function at the beginning of OnIdle().
        void Measure();

        // Call this function at the end of OnIdle();
        inline void UpdateFrameCount()
        {
            ++mFrameCount;
        }

        // Report the rates.
        double GetFramesPerSecond() const;
        double GetSecondsPerFrame() const;
        int64_t GetMillisecondsPerFrame() const;

        // For convenience in displaying a formatted frames-per-second string.
        std::string GetFPS() const;

    private:
        // The accumulated time is measured in milliseconds.
        int64_t mFrequency, mCallCount;
        int64_t mFrameCount, mAccumulatedFrameCount, mAccumulatedTime;
        std::chrono::high_resolution_clock::time_point mTime0, mTime1;
    };
}
