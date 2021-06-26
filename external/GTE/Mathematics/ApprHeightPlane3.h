// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.07.20

#pragma once

#include <Mathematics/ApprQuery.h>
#include <Mathematics/Vector3.h>

// Least-squares fit of a plane to height data (x,y,f(x,y)). The plane is of
// the form (z - zAvr) = a*(x - xAvr) + b*(y - yAvr), where (xAvr,yAvr,zAvr)
// is the average of the sample points. The return value is 'true' if and
// only the if fit is successful (the input points are noncollinear). The
// mParameters values are ((xAvr,yAvr,zAvr),(a,b,-1)) on success and
// ((0,0,0),(0,0,0)) on failure. The error for (x0,y0,z0) is
// [a*(x0-xAvr)+b*(y0-yAvr)-(z0-zAvr)]^2.

namespace gte
{
    template <typename Real>
    class ApprHeightPlane3 : public ApprQuery<Real, Vector3<Real>>
    {
    public:
        // Initialize the model parameters to zero.
        ApprHeightPlane3()
        {
            mParameters.first = Vector3<Real>::Zero();
            mParameters.second = Vector3<Real>::Zero();
        }

        // Basic fitting algorithm. See ApprQuery.h for the various Fit(...)
        // functions that you can call.
        virtual bool FitIndexed(
            size_t numPoints, Vector3<Real> const* points,
            size_t numIndices, int const* indices) override
        {
            if (this->ValidIndices(numPoints, points, numIndices, indices))
            {
                // Compute the mean of the points.
                Vector3<Real> mean = Vector3<Real>::Zero();
                int const* currentIndex = indices;
                for (size_t i = 0; i < numIndices; ++i)
                {
                    mean += points[*currentIndex++];
                }
                mean /= (Real)numIndices;

                if (std::isfinite(mean[0]) && std::isfinite(mean[1]))
                {
                    // Compute the covariance matrix of the points.
                    Real covar00 = (Real)0, covar01 = (Real)0, covar02 = (Real)0;
                    Real covar11 = (Real)0, covar12 = (Real)0;
                    currentIndex = indices;
                    for (size_t i = 0; i < numIndices; ++i)
                    {
                        Vector3<Real> diff = points[*currentIndex++] - mean;
                        covar00 += diff[0] * diff[0];
                        covar01 += diff[0] * diff[1];
                        covar02 += diff[0] * diff[2];
                        covar11 += diff[1] * diff[1];
                        covar12 += diff[1] * diff[2];
                    }

                    // Decompose the covariance matrix.
                    Real det = covar00 * covar11 - covar01 * covar01;
                    if (det != (Real)0)
                    {
                        Real invDet = (Real)1 / det;
                        mParameters.first = mean;
                        mParameters.second[0] = (covar11 * covar02 - covar01 * covar12) * invDet;
                        mParameters.second[1] = (covar00 * covar12 - covar01 * covar02) * invDet;
                        mParameters.second[2] = (Real)-1;
                        return true;
                    }
                }
            }

            mParameters.first = Vector3<Real>::Zero();
            mParameters.second = Vector3<Real>::Zero();
            return false;
        }

        // Get the parameters for the best fit.
        std::pair<Vector3<Real>, Vector3<Real>> const& GetParameters() const
        {
            return mParameters;
        }

        virtual size_t GetMinimumRequired() const override
        {
            return 3;
        }

        virtual Real Error(Vector3<Real> const& point) const override
        {
            Real d = Dot(point - mParameters.first, mParameters.second);
            Real error = d * d;
            return error;
        }

        virtual void CopyParameters(ApprQuery<Real, Vector3<Real>> const* input) override
        {
            auto source = dynamic_cast<ApprHeightPlane3<Real> const*>(input);
            if (source)
            {
                *this = *source;
            }
        }

    private:
        std::pair<Vector3<Real>, Vector3<Real>> mParameters;
    };
}
