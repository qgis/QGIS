// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/Timer.h>
using namespace gte;

Timer::Timer()
{
    Reset();
}

int64_t Timer::GetNanoseconds() const
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        currentTime - mInitialTime).count();
}

int64_t Timer::GetMicroseconds() const
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        currentTime - mInitialTime).count();
}

int64_t Timer::GetMilliseconds() const
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - mInitialTime).count();
}

double Timer::GetSeconds() const
{
    int64_t msecs = GetMilliseconds();
    return static_cast<double>(msecs) / 1000.0;
}

void Timer::Reset()
{
    mInitialTime = std::chrono::high_resolution_clock::now();
}
