// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DX11/DX11.h>

// Support for coarse-level GPU timing.

namespace gte
{
    class DX11PerformanceCounter
    {
    public:
        // Construction and destruction.
        ~DX11PerformanceCounter();
        DX11PerformanceCounter(ID3D11Device* device);

        // Performance measurements.
        int64_t GetTicks() const;
        double GetSeconds() const;

        // Get the time for the specified number of ticks.
        double GetSeconds(int64_t numTicks) const;

        // Get the number of ticks for the specified time.
        int64_t GetTicks(double seconds) const;

        // For average performance measurements.
        void ResetAccumulateTime();
        void AccumulateTime();
        double GetAverageSeconds() const;
        unsigned int GetNumMeasurements() const;

    private:
        // Allow the engine to access the members directly to avoid exposing
        // internals via the public interface.
        friend class DX11Engine;

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT mTimeStamp;
        ID3D11Query* mFrequencyQuery;
        ID3D11Query* mStartTimeQuery;
        ID3D11Query* mFinalTimeQuery;
        double mFrequency, mInvFrequency;
        int64_t mStartTime, mFinalTime;
        double mTotalSeconds;
        unsigned int mNumMeasurements;
    };
}
