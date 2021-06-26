// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/OnIdleTimer.h>
#include <chrono>
#include <sstream>
using namespace gte;

OnIdleTimer::OnIdleTimer(int64_t frequency)
    :
    mFrequency(frequency > 0 ? frequency : 1)
{
    Reset();
}

void OnIdleTimer::Reset()
{
    mCallCount = mFrequency;
    mFrameCount = 0;
    mAccumulatedFrameCount = 0;
    mAccumulatedTime = 0;
    mTime0 = std::chrono::high_resolution_clock::now();
    mTime1 = mTime0;
}

void OnIdleTimer::Measure()
{
    if (--mCallCount == 0)
    {
        mTime1 = std::chrono::high_resolution_clock::now();
        int64_t delta = std::chrono::duration_cast<std::chrono::milliseconds>(
            mTime1 - mTime0).count();
        mTime0 = mTime1;
        mAccumulatedFrameCount += mFrameCount;
        mAccumulatedTime += delta;
        mFrameCount = 0;
        mCallCount = mFrequency;
    }
}

double OnIdleTimer::GetFramesPerSecond() const
{
    if (mAccumulatedTime > 0)
    {
        double numFrames = static_cast<double>(mAccumulatedFrameCount);
        double numMilliseconds = static_cast<double>(mAccumulatedTime);
        return 1000.0 * numFrames / numMilliseconds;
    }
    else
    {
        return 0.0;
    }
}

double OnIdleTimer::GetSecondsPerFrame() const
{
    if (mAccumulatedFrameCount > 0)
    {
        double numFrames = static_cast<double>(mAccumulatedFrameCount);
        double numMilliseconds = static_cast<double>(mAccumulatedTime);
        return 1000.0 * numMilliseconds / numFrames;
    }
    else
    {
        return 0.0;
    }
}

int64_t OnIdleTimer::GetMillisecondsPerFrame() const
{
    if (mAccumulatedFrameCount > 0)
    {
        double numFrames = static_cast<double>(mAccumulatedFrameCount);
        double numMilliseconds = static_cast<double>(mAccumulatedTime);
        double rate = numMilliseconds / numFrames;
        return static_cast<int64_t>(rate + 0.5);
    }
    else
    {
        return 0;
    }
}

std::string OnIdleTimer::GetFPS() const
{
    std::ostringstream stream;
    stream.setf(std::ios::fixed | std::ios::showpoint);
    stream.precision(1);
    stream << "fps: " << GetFramesPerSecond();
    return stream.str();
}
