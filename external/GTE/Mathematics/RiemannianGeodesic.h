// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/GMatrix.h>
#include <Mathematics/LinearSystem.h>
#include <functional>

// Computing geodesics on a surface is a differential geometric topic that
// involves Riemannian geometry.  The algorithm for constructing geodesics
// that is implemented here uses a multiresolution approach.  A description
// of the algorithm is in the document
// https://www.geometrictools.com/Documentation/RiemannianGeodesics.pdf

namespace gte
{
    template <typename Real>
    class RiemannianGeodesic
    {
    public:
        // Construction and destruction.  The input dimension must be two or
        // larger.
        RiemannianGeodesic(int dimension)
            :
            integralSamples(16),
            searchSamples(32),
            derivativeStep((Real)1e-04),
            subdivisions(7),
            refinements(8),
            searchRadius((Real)1),
            refineCallback([]() {}),
            mDimension(dimension >= 2 ? dimension : 2),
            mMetric(mDimension, mDimension),
            mMetricInverse(mDimension, mDimension),
            mChristoffel1(mDimension),
            mChristoffel2(mDimension),
            mMetricDerivative(mDimension),
            mMetricInverseExists(true),
            mSubdivide(0),
            mRefine(0),
            mCurrentQuantity(0),
            mIntegralStep((Real)1 / (Real)(integralSamples - 1)),
            mSearchStep((Real)1 / (Real)searchSamples),
            mDerivativeFactor((Real)0.5 / derivativeStep)
        {
            LogAssert(dimension >= 2, "Dimension must be at least 2.");
            for (int i = 0; i < mDimension; ++i)
            {
                mChristoffel1[i].SetSize(mDimension, mDimension);
                mChristoffel2[i].SetSize(mDimension, mDimension);
                mMetricDerivative[i].SetSize(mDimension, mDimension);
            }
        }

        virtual ~RiemannianGeodesic()
        {
        }

        // Tweakable parameters.
        // 1. The integral samples are the number of samples used in the
        //    Trapezoid Rule numerical integrator.
        // 2. The search samples are the number of samples taken along a ray
        //    for the steepest descent algorithm used to refine the vertices
        //    of the polyline approximation to the geodesic curve.
        // 3. The derivative step is the value of h used for centered
        //    difference approximations df/dx = (f(x+h)-f(x-h))/(2*h) in the
        //    steepest descent algorithm.
        // 4. The number of subdivisions indicates how many times the polyline
        //    segments should be subdivided.  The number of polyline vertices
        //    will be pow(2,subdivisions)+1.
        // 5. The number of refinements per subdivision.  Setting this to a
        //    positive value appears necessary when the geodesic curve has a
        //    large length.
        // 6. The search radius is the distance over which the steepest
        //    descent algorithm searches for a minimum on the line whose
        //    direction is the estimated gradient.  The default of 1 means the
        //    search interval is [-L,L], where L is the length of the gradient.
        //    If the search radius is r, then the interval is [-r*L,r*L].
        int integralSamples;  // default = 16
        int searchSamples;    // default = 32
        Real derivativeStep;  // default = 0.0001
        int subdivisions;     // default = 7
        int refinements;      // default = 8
        Real searchRadius;    // default = 1.0

        // The dimension of the manifold.
        inline int GetDimension() const
        {
            return mDimension;
        }

        // Returns the length of the line segment connecting the points.
        Real ComputeSegmentLength(GVector<Real> const& point0, GVector<Real> const& point1)
        {
            // The Trapezoid Rule is used for integration of the length
            // integral.  The ComputeMetric function internally modifies
            // mMetric, which means the qForm values are actually varying
            // even though diff does not.
            GVector<Real> diff = point1 - point0;
            GVector<Real> temp(mDimension);

            // Evaluate the integrand at point0.
            ComputeMetric(point0);
            Real qForm = Dot(diff, mMetric * diff);
            LogAssert(qForm > (Real)0, "Unexpected condition.");
            Real length = std::sqrt(qForm);

            // Evaluate the integrand at point1.
            ComputeMetric(point1);
            qForm = Dot(diff, mMetric * diff);
            LogAssert(qForm > (Real)0, "Unexpected condition.");
            length += std::sqrt(qForm);
            length *= (Real)0.5;

            int imax = integralSamples - 2;
            for (int i = 1; i <= imax; ++i)
            {
                // Evaluate the integrand at point0+t*(point1-point0).
                Real t = mIntegralStep * static_cast<Real>(i);
                temp = point0 + t * diff;
                ComputeMetric(temp);
                qForm = Dot(diff, mMetric * diff);
                LogAssert(qForm > (Real)0, "Unexpected condition.");
                length += std::sqrt(qForm);
            }
            length *= mIntegralStep;
            return length;
        }

        // Compute the total length of the polyline.  The lengths of the
        // segments are computed relative to the metric tensor.
        Real ComputeTotalLength(int quantity, std::vector<GVector<Real>> const& path)
        {
            LogAssert(quantity >= 2, "Path must have at least two points.");
            Real length = ComputeSegmentLength(path[0], path[1]);
            for (int i = 1; i <= quantity - 2; ++i)
            {
                length += ComputeSegmentLength(path[i], path[i + 1]);
            }
            return length;
        }

        // Returns a polyline approximation to a geodesic curve connecting the
        // points.
        void ComputeGeodesic(GVector<Real> const& end0, GVector<Real> const& end1,
            int& quantity, std::vector<GVector<Real>>& path)
        {
            LogAssert(subdivisions < 32, "Exceeds maximum iterations.");
            quantity = (1 << subdivisions) + 1;
            path.resize(quantity);
            for (int i = 0; i < quantity; ++i)
            {
                path[i].SetSize(mDimension);
            }

            mCurrentQuantity = 2;
            path[0] = end0;
            path[1] = end1;

            for (mSubdivide = 1; mSubdivide <= subdivisions; ++mSubdivide)
            {
                // A subdivision essentially doubles the number of points.
                int newQuantity = 2 * mCurrentQuantity - 1;
                LogAssert(newQuantity <= quantity, "Unexpected condition.");

                // Copy the old points so that there are slots for the
                // midpoints during the subdivision, the slots interleaved
                // between the old points.
                for (int i = mCurrentQuantity - 1; i > 0; --i)
                {
                    path[2 * i] = path[i];
                }

                // Subdivide the polyline.
                for (int i = 0; i <= mCurrentQuantity - 2; ++i)
                {
                    Subdivide(path[2 * i], path[2 * i + 1], path[2 * i + 2]);
                }

                mCurrentQuantity = newQuantity;

                // Refine the current polyline vertices.
                for (mRefine = 1; mRefine <= refinements; ++mRefine)
                {
                    for (int i = 1; i <= mCurrentQuantity - 2; ++i)
                    {
                        Refine(path[i - 1], path[i], path[i + 1]);
                    }
                }
            }

            LogAssert(mCurrentQuantity == quantity, "Unexpected condition.");
            mSubdivide = 0;
            mRefine = 0;
            mCurrentQuantity = 0;
        }

        // Start with the midpoint M of the line segment (E0,E1) and use a
        // steepest descent algorithm to move M so that
        //     Length(E0,M) + Length(M,E1) < Length(E0,E1)
        // This is essentially a relaxation scheme that inserts points into
        // the current polyline approximation to the geodesic curve.
        bool Subdivide(GVector<Real> const& end0, GVector<Real>& mid, GVector<Real> const& end1)
        {
            mid = (Real)0.5 * (end0 + end1);
            auto save = refineCallback;
            refineCallback = []() {};
            bool changed = Refine(end0, mid, end1);
            refineCallback = save;
            return changed;
        }

        // Apply the steepest descent algorithm to move the midpoint M of the
        // line segment (E0,E1) so that
        //     Length(E0,M) + Length(M,E1) < Length(E0,E1)
        // This is essentially a relaxation scheme that inserts points into
        // the current polyline approximation to the geodesic curve.
        bool Refine(GVector<Real> const& end0, GVector<Real>& mid, GVector<Real> const& end1)
        {
            // Estimate the gradient vector for the function
            // F(m) = Length(e0,m) + Length(m,e1).
            GVector<Real> temp = mid;
            GVector<Real> gradient(mDimension);
            for (int i = 0; i < mDimension; ++i)
            {
                temp[i] = mid[i] + derivativeStep;
                gradient[i] = ComputeSegmentLength(end0, temp);
                gradient[i] += ComputeSegmentLength(temp, end1);

                temp[i] = mid[i] - derivativeStep;
                gradient[i] -= ComputeSegmentLength(end0, temp);
                gradient[i] -= ComputeSegmentLength(temp, end1);

                temp[i] = mid[i];
                gradient[i] *= mDerivativeFactor;
            }

            // Compute the length sum for the current midpoint.
            Real length0 = ComputeSegmentLength(end0, mid);
            Real length1 = ComputeSegmentLength(mid, end1);
            Real oldLength = length0 + length1;

            Real tRay, newLength;
            GVector<Real> pRay(mDimension);

            Real multiplier = mSearchStep * searchRadius;
            Real minLength = oldLength;
            GVector<Real> minPoint = mid;
            for (int i = -searchSamples; i <= searchSamples; ++i)
            {
                tRay = multiplier * static_cast<Real>(i);
                pRay = mid - tRay * gradient;
                length0 = ComputeSegmentLength(end0, pRay);
                length1 = ComputeSegmentLength(end1, pRay);
                newLength = length0 + length1;
                if (newLength < minLength)
                {
                    minLength = newLength;
                    minPoint = pRay;
                }
            }

            mid = minPoint;
            refineCallback();
            return minLength < oldLength;
        }

        // A callback that is executed during each call of Refine.
        std::function<void(void)> refineCallback;

        // Information to be used during the callback.
        inline int GetSubdivisionStep() const
        {
            return mSubdivide;
        }

        inline int GetRefinementStep() const
        {
            return mRefine;
        }

        inline int GetCurrentQuantity() const
        {
            return mCurrentQuantity;
        }

        // Curvature computations to measure how close the approximating
        // polyline is to a geodesic.

        // Returns the total curvature of the line segment connecting the
        // points.
        Real ComputeSegmentCurvature(GVector<Real> const& point0, GVector<Real> const& point1)
        {
            // The Trapezoid Rule is used for integration of the curvature
            // integral.  The ComputeIntegrand function internally modifies
            // mMetric, which means the curvature values are actually varying
            // even though diff does not.
            GVector<Real> diff = point1 - point0;
            GVector<Real> temp(mDimension);

            // Evaluate the integrand at point0.
            Real curvature = ComputeIntegrand(point0, diff);

            // Evaluate the integrand at point1.
            curvature += ComputeIntegrand(point1, diff);
            curvature *= (Real)0.5;

            int imax = integralSamples - 2;
            for (int i = 1; i <= imax; ++i)
            {
                // Evaluate the integrand at point0+t*(point1-point0).
                Real t = mIntegralStep * static_cast<Real>(i);
                temp = point0 + t * diff;
                curvature += ComputeIntegrand(temp, diff);
            }
            curvature *= mIntegralStep;
            return curvature;
        }

        // Compute the total curvature of the polyline.  The curvatures of the
        // segments are computed relative to the metric tensor.
        Real ComputeTotalCurvature(int quantity, std::vector<GVector<Real>> const& path)
        {
            LogAssert(quantity >= 2, "Path must have at least two points.");
            Real curvature = ComputeSegmentCurvature(path[0], path[1]);
            for (int i = 1; i <= quantity - 2; ++i)
            {
                curvature += ComputeSegmentCurvature(path[i], path[i + 1]);
            }
            return curvature;
        }

    protected:
        // Support for ComputeSegmentCurvature.
        Real ComputeIntegrand(GVector<Real> const& pos, GVector<Real> const& der)
        {
            ComputeMetric(pos);
            ComputeChristoffel1(pos);
            ComputeMetricInverse();
            ComputeChristoffel2();

            // g_{ij}*der_{i}*der_{j}
            Real qForm0 = Dot(der, mMetric * der);
            LogAssert(qForm0 > (Real)0, "Unexpected condition.");

            // gamma_{kij}*der_{k}*der_{i}*der_{j}
            GMatrix<Real> mat(mDimension, mDimension);
            for (int k = 0; k < mDimension; ++k)
            {
                mat += der[k] * mChristoffel1[k];
            }
            // This product can be negative because mat is not guaranteed to
            // be positive semidefinite.  No assertion is added.
            Real qForm1 = Dot(der, mat * der);

            Real ratio = -qForm1 / qForm0;

            // Compute the acceleration.
            GVector<Real> acc = ratio * der;
            for (int k = 0; k < mDimension; ++k)
            {
                acc[k] += Dot(der, mChristoffel2[k] * der);
            }

            // Compute the curvature.
            Real curvature = std::sqrt(Dot(acc, mMetric * acc));
            return curvature;
        }

        // Compute the metric tensor for the specified point.  Derived classes
        // are responsible for implementing this function.
        virtual void ComputeMetric(GVector<Real> const& point) = 0;

        // Compute the Christoffel symbols of the first kind for the current
        // point.  Derived classes are responsible for implementing this
        // function.
        virtual void ComputeChristoffel1(GVector<Real> const& point) = 0;

        // Compute the inverse of the current metric tensor.  The function
        // returns 'true' iff the inverse exists.
        bool ComputeMetricInverse()
        {
            mMetricInverse = Inverse(mMetric, &mMetricInverseExists);
            return mMetricInverseExists;
        }

        // Compute the derivative of the metric tensor for the current state.
        // This is a triply indexed quantity, the values computed using the
        // Christoffel symbols of the first kind.
        void ComputeMetricDerivative()
        {
            for (int derivative = 0; derivative < mDimension; ++derivative)
            {
                for (int i0 = 0; i0 < mDimension; ++i0)
                {
                    for (int i1 = 0; i1 < mDimension; ++i1)
                    {
                        mMetricDerivative[derivative](i0, i1) =
                            mChristoffel1[derivative](i0, i1) +
                            mChristoffel1[derivative](i1, i0);
                    }
                }
            }
        }

        // Compute the Christoffel symbols of the second kind for the current
        // state.  The values depend on the inverse of the metric tensor, so
        // they may be computed only when the inverse exists.  The function
        // returns 'true' whenever the inverse metric tensor exists.
        bool ComputeChristoffel2()
        {
            for (int i2 = 0; i2 < mDimension; ++i2)
            {
                for (int i0 = 0; i0 < mDimension; ++i0)
                {
                    for (int i1 = 0; i1 < mDimension; ++i1)
                    {
                        Real fValue = (Real)0;
                        for (int j = 0; j < mDimension; ++j)
                        {
                            fValue += mMetricInverse(i2, j) * mChristoffel1[j](i0, i1);
                        }
                        mChristoffel2[i2](i0, i1) = fValue;
                    }
                }
            }
            return mMetricInverseExists;
        }

        int mDimension;
        GMatrix<Real> mMetric;
        GMatrix<Real> mMetricInverse;
        std::vector<GMatrix<Real>> mChristoffel1;
        std::vector<GMatrix<Real>> mChristoffel2;
        std::vector<GMatrix<Real>> mMetricDerivative;
        bool mMetricInverseExists;

        // Progress parameters that are useful to mRefineCallback.
        int mSubdivide, mRefine, mCurrentQuantity;

        // Derived tweaking parameters.
        Real mIntegralStep;      // = 1/(mIntegralQuantity-1)
        Real mSearchStep;        // = 1/mSearchQuantity
        Real mDerivativeFactor;  // = 1/(2*mDerivativeStep)
    };
}
