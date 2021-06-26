// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <algorithm>
#include <vector>

namespace gte
{
    class Histogram
    {
    public:
        // In the constructor with input 'int const* samples', set noRescaling
        // to 'true' when you want the sample values mapped directly to the
        // buckets.  Typically, you know that the sample values are in the set
        // of numbers {0,1,...,numBuckets-1}, but in the event of out-of-range
        // values, the histogram stores a count for those numbers smaller than
        // 0 and those numbers larger or equal to numBuckets.
        Histogram(int numBuckets, int numSamples, int const* samples, bool noRescaling)
            :
            mBuckets(numBuckets),
            mExcessLess(0),
            mExcessGreater(0)
        {
            LogAssert(numBuckets > 0 && numSamples > 0 && samples != nullptr, "Invalid input.");

            std::fill(mBuckets.begin(), mBuckets.end(), 0);

            if (noRescaling)
            {
                // Map to the buckets, also counting out-of-range pixels.
                for (int i = 0; i < numSamples; ++i)
                {
                    int value = samples[i];
                    if (0 <= value)
                    {
                        if (value < numBuckets)
                        {
                            ++mBuckets[value];
                        }
                        else
                        {
                            ++mExcessGreater;
                        }
                    }
                    else
                    {
                        ++mExcessLess;
                    }
                }
            }
            else
            {
                // Compute the extremes.
                int minValue = samples[0], maxValue = minValue;
                for (int i = 1; i < numSamples; ++i)
                {
                    int value = samples[i];
                    if (value < minValue)
                    {
                        minValue = value;
                    }
                    else if (value > maxValue)
                    {
                        maxValue = value;
                    }
                }

                // Map to the buckets.
                if (minValue < maxValue)
                {
                    // The image is not constant.
                    double numer = static_cast<double>(numBuckets - 1);
                    double denom = static_cast<double>(maxValue - minValue);
                    double mult = numer / denom;
                    for (int i = 0; i < numSamples; ++i)
                    {
                        int index = static_cast<int>(mult * static_cast<double>(samples[i] - minValue));
                        ++mBuckets[index];
                    }
                }
                else
                {
                    // The image is constant.
                    mBuckets[0] = numSamples;
                }
            }
        }

        Histogram(int numBuckets, int numSamples, float const* samples)
            :
            mBuckets(numBuckets),
            mExcessLess(0),
            mExcessGreater(0)
        {
            LogAssert(numBuckets > 0 && numSamples > 0 && samples != nullptr, "Invalid input.");

            std::fill(mBuckets.begin(), mBuckets.end(), 0);

            // Compute the extremes.
            float minValue = samples[0], maxValue = minValue;
            for (int i = 1; i < numSamples; ++i)
            {
                float value = samples[i];
                if (value < minValue)
                {
                    minValue = value;
                }
                else if (value > maxValue)
                {
                    maxValue = value;
                }
            }

            // Map to the buckets.
            if (minValue < maxValue)
            {
                // The image is not constant.
                double numer = static_cast<double>(numBuckets - 1);
                double denom = static_cast<double>(maxValue - minValue);
                double mult = numer / denom;
                for (int i = 0; i < numSamples; ++i)
                {
                    int index = static_cast<int>(mult * static_cast<double>(samples[i] - minValue));
                    ++mBuckets[index];
                }
            }
            else
            {
                // The image is constant.
                mBuckets[0] = numSamples;
            }
        }

        Histogram(int numBuckets, int numSamples, double const* samples)
            :
            mBuckets(numBuckets),
            mExcessLess(0),
            mExcessGreater(0)
        {
            LogAssert(numBuckets > 0 && numSamples > 0 && samples != nullptr, "Invalid input.");

            std::fill(mBuckets.begin(), mBuckets.end(), 0);

            // Compute the extremes.
            double minValue = samples[0], maxValue = minValue;
            for (int i = 1; i < numSamples; ++i)
            {
                double value = samples[i];
                if (value < minValue)
                {
                    minValue = value;
                }
                else if (value > maxValue)
                {
                    maxValue = value;
                }
            }

            // Map to the buckets.
            if (minValue < maxValue)
            {
                // The image is not constant.
                double numer = static_cast<double>(numBuckets - 1);
                double denom = maxValue - minValue;
                double mult = numer / denom;
                for (int i = 0; i < numSamples; ++i)
                {
                    int index = static_cast<int>(mult * (samples[i] - minValue));
                    ++mBuckets[index];
                }
            }
            else
            {
                // The image is constant.
                mBuckets[0] = numSamples;
            }
        }

        // Construction when you plan on updating the histogram incrementally.
        // The incremental update is implemented only for integer samples and
        // no rescaling.
        Histogram(int numBuckets)
            :
            mBuckets(numBuckets),
            mExcessLess(0),
            mExcessGreater(0)
        {
            LogAssert(numBuckets > 0, "Invalid input.");

            std::fill(mBuckets.begin(), mBuckets.end(), 0);
        }

        // This function is called when you have used the Histogram(int)
        // constructor.  No bounds checking is used; you must ensure that the
        // input value is in {0,...,numBuckets-1}.
        inline void Insert(int value)
        {
            ++mBuckets[value];
        }

        // This function is called when you have used the Histogram(int)
        // constructor.  Bounds checking is used.
        void InsertCheck(int value)
        {
            if (0 <= value)
            {
                if (value < static_cast<int>(mBuckets.size()))
                {
                    ++mBuckets[value];
                }
                else
                {
                    ++mExcessGreater;
                }
            }
            else
            {
                ++mExcessLess;
            }
        }

        // Member access.
        inline std::vector<int> const& GetBuckets() const
        {
            return mBuckets;
        }

        inline int GetExcessLess() const
        {
            return mExcessLess;
        }

        inline int GetExcessGreater() const
        {
            return mExcessGreater;
        }

        // In the following, define cdf(V) = sum_{i=0}^{V} bucket[i], where
        // 0 <= V < B and B is the number of buckets.  Define N = cdf(B-1),
        // which must be the number of pixels in the image.

        // Get the lower tail of the histogram.  The returned index L has the
        // properties:  cdf(L-1)/N < tailAmount and cdf(L)/N >= tailAmount.
        int GetLowerTail(double tailAmount)
        {
            int const numBuckets = static_cast<int>(mBuckets.size());
            int hSum = 0;
            for (int i = 0; i < numBuckets; ++i)
            {
                hSum += mBuckets[i];
            }

            int hTailSum = static_cast<int>(tailAmount * hSum);
            int hLowerSum = 0;
            int lower;
            for (lower = 0; lower < numBuckets; ++lower)
            {
                hLowerSum += mBuckets[lower];
                if (hLowerSum >= hTailSum)
                {
                    break;
                }
            }
            return lower;
        }

        // Get the upper tail of the histogram.  The returned index U has the
        // properties:  cdf(U)/N >= 1-tailAmount and cdf(U+1) < 1-tailAmount.
        int GetUpperTail(double tailAmount)
        {
            int const numBuckets = static_cast<int>(mBuckets.size());
            int hSum = 0;
            for (int i = 0; i < numBuckets; ++i)
            {
                hSum += mBuckets[i];
            }

            int hTailSum = static_cast<int>(tailAmount * hSum);
            int hUpperSum = 0;
            int upper;
            for (upper = numBuckets - 1; upper >= 0; --upper)
            {
                hUpperSum += mBuckets[upper];
                if (hUpperSum >= hTailSum)
                {
                    break;
                }
            }
            return upper;
        }

        // Get the lower and upper tails of the histogram.  The returned
        // indices are L and U and have the properties:
        //   cdf(L-1)/N < tailAmount/2, cdf(L)/N >= tailAmount/2,
        //   cdf(U)/N >= 1-tailAmount/2, and cdf(U+1) < 1-tailAmount/2.
        void GetTails(double tailAmount, int& lower, int& upper)
        {
            int const numBuckets = static_cast<int>(mBuckets.size());
            int hSum = 0;
            for (int i = 0; i < numBuckets; ++i)
            {
                hSum += mBuckets[i];
            }

            int hTailSum = static_cast<int>(0.5 * tailAmount * hSum);
            int hLowerSum = 0;
            for (lower = 0; lower < numBuckets; ++lower)
            {
                hLowerSum += mBuckets[lower];
                if (hLowerSum >= hTailSum)
                {
                    break;
                }
            }

            int hUpperSum = 0;
            for (upper = numBuckets - 1; upper >= 0; --upper)
            {
                hUpperSum += mBuckets[upper];
                if (hUpperSum >= hTailSum)
                {
                    break;
                }
            }
        }

    private:
        std::vector<int> mBuckets;
        int mExcessLess, mExcessGreater;
    };
}
