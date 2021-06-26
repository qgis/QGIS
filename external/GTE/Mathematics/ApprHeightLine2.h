// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprQuery.h>
#include <Mathematics/Vector2.h>

// Least-squares fit of a line to height data (x,f(x)). The line is of the
// form: (y - yAvr) = a*(x - xAvr), where (xAvr,yAvr) is the average of the
// sample points. The return value of Fit is 'true' if and only if the fit is
// successful (the input points are not degenerate to a single point). The
// mParameters values are ((xAvr,yAvr),(a,-1)) on success and ((0,0),(0,0)) on
// failure. The error for (x0,y0) is [a*(x0-xAvr)-(y0-yAvr)]^2.

namespace gte
{
    template <typename Real>
    class ApprHeightLine2 : public ApprQuery<Real, Vector2<Real>>
    {
    public:
        // Initialize the model parameters to zero.
        ApprHeightLine2()
        {
            mParameters.first = Vector2<Real>::Zero();
            mParameters.second = Vector2<Real>::Zero();
        }

        // Basic fitting algorithm. See ApprQuery.h for the various Fit(...)
        // functions that you can call.
        virtual bool FitIndexed(
            size_t numPoints, Vector2<Real> const* points,
            size_t numIndices, int const* indices) override
        {
            if (this->ValidIndices(numPoints, points, numIndices, indices))
            {
                // Compute the mean of the points.
                Vector2<Real> mean = Vector2<Real>::Zero();
                int const* currentIndex = indices;
                for (size_t i = 0; i < numIndices; ++i)
                {
                    mean += points[*currentIndex++];
                }
                mean /= (Real)numIndices;

                if (std::isfinite(mean[0]) && std::isfinite(mean[1]))
                {
                    // Compute the covariance matrix of the points.
                    Real covar00 = (Real)0, covar01 = (Real)0;
                    currentIndex = indices;
                    for (size_t i = 0; i < numIndices; ++i)
                    {
                        Vector2<Real> diff = points[*currentIndex++] - mean;
                        covar00 += diff[0] * diff[0];
                        covar01 += diff[0] * diff[1];
                    }

                    // Decompose the covariance matrix.
                    if (covar00 > (Real)0)
                    {
                        mParameters.first = mean;
                        mParameters.second[0] = covar01 / covar00;
                        mParameters.second[1] = (Real)-1;
                        return true;
                    }
                }
            }

            mParameters.first = Vector2<Real>::Zero();
            mParameters.second = Vector2<Real>::Zero();
            return false;
        }

        // Get the parameters for the best fit.
        std::pair<Vector2<Real>, Vector2<Real>> const& GetParameters() const
        {
            return mParameters;
        }

        virtual size_t GetMinimumRequired() const override
        {
            return 2;
        }

        virtual Real Error(Vector2<Real> const& point) const override
        {
            Real d = Dot(point - mParameters.first, mParameters.second);
            Real error = d * d;
            return error;
        }

        virtual void CopyParameters(ApprQuery<Real, Vector2<Real>> const* input) override
        {
            auto source = dynamic_cast<ApprHeightLine2<Real> const*>(input);
            if (source)
            {
                *this = *source;
            }
        }

    private:
        std::pair<Vector2<Real>, Vector2<Real>> mParameters;
    };
}
