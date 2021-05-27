// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprQuery.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/SymmetricEigensolver3x3.h>
#include <Mathematics/Vector3.h>

// Fit points with a Gaussian distribution. The center is the mean of the
// points, the axes are the eigenvectors of the covariance matrix and the
// extents are the eigenvalues of the covariance matrix and are returned in
// increasing order. An oriented box is used to store the mean, axes and
// extents.

namespace gte
{
    template <typename Real>
    class ApprGaussian3 : public ApprQuery<Real, Vector3<Real>>
    {
    public:
        // Initialize the model parameters to zero.
        ApprGaussian3()
        {
            mParameters.center = Vector3<Real>::Zero();
            mParameters.axis[0] = Vector3<Real>::Zero();
            mParameters.axis[1] = Vector3<Real>::Zero();
            mParameters.axis[2] = Vector3<Real>::Zero();
            mParameters.extent = Vector3<Real>::Zero();
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
                Real invSize = (Real)1 / (Real)numIndices;
                mean *= invSize;

                if (std::isfinite(mean[0]) && std::isfinite(mean[1]))
                {
                    // Compute the covariance matrix of the points.
                    Real covar00 = (Real)0, covar01 = (Real)0, covar02 = (Real)0;
                    Real covar11 = (Real)0, covar12 = (Real)0, covar22 = (Real)0;
                    currentIndex = indices;
                    for (size_t i = 0; i < numIndices; ++i)
                    {
                        Vector3<Real> diff = points[*currentIndex++] - mean;
                        covar00 += diff[0] * diff[0];
                        covar01 += diff[0] * diff[1];
                        covar02 += diff[0] * diff[2];
                        covar11 += diff[1] * diff[1];
                        covar12 += diff[1] * diff[2];
                        covar22 += diff[2] * diff[2];
                    }
                    covar00 *= invSize;
                    covar01 *= invSize;
                    covar02 *= invSize;
                    covar11 *= invSize;
                    covar12 *= invSize;
                    covar22 *= invSize;

                    // Solve the eigensystem.
                    SymmetricEigensolver3x3<Real> es;
                    std::array<Real, 3> eval;
                    std::array<std::array<Real, 3>, 3> evec;
                    es(covar00, covar01, covar02, covar11, covar12, covar22,
                        false, +1, eval, evec);
                    mParameters.center = mean;
                    mParameters.axis[0] = evec[0];
                    mParameters.axis[1] = evec[1];
                    mParameters.axis[2] = evec[2];
                    mParameters.extent = eval;
                    return true;
                }
            }

            mParameters.center = Vector3<Real>::Zero();
            mParameters.axis[0] = Vector3<Real>::Zero();
            mParameters.axis[1] = Vector3<Real>::Zero();
            mParameters.axis[2] = Vector3<Real>::Zero();
            mParameters.extent = Vector3<Real>::Zero();
            return false;
        }

        // Get the parameters for the best fit.
        OrientedBox3<Real> const& GetParameters() const
        {
            return mParameters;
        }

        virtual size_t GetMinimumRequired() const override
        {
            return 2;
        }

        virtual Real Error(Vector3<Real> const& point) const override
        {
            Vector3<Real> diff = point - mParameters.center;
            Real error = (Real)0;
            for (int i = 0; i < 3; ++i)
            {
                if (mParameters.extent[i] > (Real)0)
                {
                    Real ratio = Dot(diff, mParameters.axis[i]) / mParameters.extent[i];
                    error += ratio * ratio;
                }
            }
            return error;
        }

        virtual void CopyParameters(ApprQuery<Real, Vector3<Real>> const* input) override
        {
            auto source = dynamic_cast<ApprGaussian3 const*>(input);
            if (source)
            {
                *this = *source;
            }
        }

    private:
        OrientedBox3<Real> mParameters;
    };
}
