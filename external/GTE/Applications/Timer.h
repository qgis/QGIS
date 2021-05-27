// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <cstdint>
#include <chrono>

namespace gte
{
    class Timer
    {
    public:
        // Construction of a high-resolution timer (64-bit).
        Timer();

        // Get the current time relative to the initial time.
        int64_t GetNanoseconds() const;
        int64_t GetMicroseconds() const;
        int64_t GetMilliseconds() const;
        double GetSeconds() const;

        // Reset so that the current time is the initial time.
        void Reset();

    private:
        std::chrono::high_resolution_clock::time_point mInitialTime;
    };
}
