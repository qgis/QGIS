// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.01

#pragma once

// Compute the convex hull of 2D points using a divide-and-conquer algorithm.
// This is an O(N log N) algorithm for N input points. The only way to ensure
// a correct result for the input vertices is to use an exact predicate for
// computing signs of various expressions. The implementation uses interval
// arithmetic and rational arithmetic for the predicate.

#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/FPInterval.h>
#include <Mathematics/Line.h>
#include <Mathematics/Vector2.h>

// Uncomment this to assert when an infinite loop is encountered in
// ConvexHull2::GetTangent.
//#define GTE_THROW_ON_CONVEXHULL2_INFINITE_LOOP

namespace gte
{
    // The Real must be 'float' or 'double'.
    template <typename Real>
    class ConvexHull2
    {
    public:
        // Supporting constants and types for rational arithmetic used in
        // the exact predicate for sign computations.
        static int constexpr NumWords = std::is_same<Real, float>::value ? 18 : 132;
        using Rational = BSNumber<UIntegerFP32<NumWords>>;
        using Interval = FPInterval<Real>;

        // The class is a functor to support computing the convex hull of
        // multiple data sets using the same class object.
        ConvexHull2()
            :
            mEpsilon(static_cast<Real>(0)),
            mDimension(0),
            mLine(Vector2<Real>::Zero(), Vector2<Real>::Zero()),
            mRationalPoints{},
            mConverted{},
            mNumPoints(0),
            mNumUniquePoints(0),
            mPoints(nullptr)
        {
            static_assert(std::is_floating_point<Real>::value,
                "The input type must be 'float' or 'double'.");
        }

        // The input is the array of points whose convex hull is required. The
        // epsilon value is used to determine the intrinsic dimensionality of
        // the vertices (d = 0, 1, or 2).  When epsilon is positive, the
        // determination is fuzzy: points approximately the same point,
        // approximately on a line, or planar.  The return value is 'true' if
        // and only if the hull construction is successful.
        bool operator()(int numPoints, Vector2<Real> const* points, Real epsilon)
        {
            mEpsilon = std::max(epsilon, static_cast<Real>(0));
            mDimension = 0;
            mLine.origin = Vector2<Real>::Zero();
            mLine.direction = Vector2<Real>::Zero();
            mNumPoints = numPoints;
            mNumUniquePoints = 0;
            mPoints = points;
            mMerged.clear();
            mHull.clear();

            if (mNumPoints < 3)
            {
                // ConvexHull2 should be called with at least three points.
                return false;
            }

            IntrinsicsVector2<Real> info(mNumPoints, mPoints, mEpsilon);
            if (info.dimension == 0)
            {
                // mDimension is 0
                return false;
            }

            if (info.dimension == 1)
            {
                // The set is (nearly) collinear.
                mDimension = 1;
                mLine = Line2<Real>(info.origin, info.direction[0]);
                return false;
            }

            mDimension = 2;

            // Allocate storage for any rational points that must be
            // computed in the exact predicate.
            mRationalPoints.resize(mNumPoints);
            mConverted.resize(mNumPoints);
            std::fill(mConverted.begin(), mConverted.end(), 0u);

            // Sort the points.
            mHull.resize(mNumPoints);
            for (int i = 0; i < mNumPoints; ++i)
            {
                mHull[i] = i;
            }
            std::sort(mHull.begin(), mHull.end(),
                [points](int i0, int i1)
                {
                    if (points[i0][0] < points[i1][0])
                    {
                        return true;
                    }
                    if (points[i0][0] > points[i1][0])
                    {
                        return false;
                    }
                    return points[i0][1] < points[i1][1];
                }
            );

            // Remove duplicates.
            auto newEnd = std::unique(mHull.begin(), mHull.end(),
                [points](int i0, int i1)
                {
                    return points[i0] == points[i1];
                }
            );
            mHull.erase(newEnd, mHull.end());
            mNumUniquePoints = static_cast<int>(mHull.size());

            // Use a divide-and-conquer algorithm. The merge step computes
            // the convex hull of two convex polygons.
            mMerged.resize(mNumUniquePoints);
            int i0 = 0, i1 = mNumUniquePoints - 1;
            GetHull(i0, i1);
            mHull.resize(i1 - i0 + 1);
            return true;
        }

        // Dimensional information. If GetDimension() returns 1, the points
        // lie on a line P+t*D (fuzzy comparison when epsilon > 0). You can
        // sort these if you need a polyline output by projecting onto the
        // line each vertex X = P+t*D, where t = Dot(D,X-P).
        inline Real GetEpsilon() const
        {
            return mEpsilon;
        }

        inline int GetDimension() const
        {
            return mDimension;
        }

        inline Line2<Real> const& GetLine() const
        {
            return mLine;
        }

        // Member access.
        inline int GetNumPoints() const
        {
            return mNumPoints;
        }

        inline int GetNumUniquePoints() const
        {
            return mNumUniquePoints;
        }

        inline Vector2<Real> const* GetPoints() const
        {
            return mPoints;
        }

        // The convex hull is a convex polygon whose vertices are listed in
        // counterclockwise order.
        inline std::vector<int> const& GetHull() const
        {
            return mHull;
        }

    private:
        // Support for divide-and-conquer.
        void GetHull(int& i0, int& i1)
        {
            int numVertices = i1 - i0 + 1;
            if (numVertices > 1)
            {
                // Compute the middle index of input range.
                int mid = (i0 + i1) / 2;

                // Compute the hull of subsets (mid-i0+1 >= i1-mid).
                int j0 = i0, j1 = mid, j2 = mid + 1, j3 = i1;
                GetHull(j0, j1);
                GetHull(j2, j3);

                // Merge the convex hulls into a single convex hull.
                Merge(j0, j1, j2, j3, i0, i1);
            }
            // else: The convex hull is a single point.
        }

        void Merge(int j0, int j1, int j2, int j3, int& i0, int& i1)
        {
            // Subhull0 is to the left of subhull1 because of the initial
            // sorting of the points by x-components. We need to find two
            // mutually visible points, one on the left subhull and one on
            // the right subhull.
            int size0 = j1 - j0 + 1;
            int size1 = j3 - j2 + 1;

            int i;
            Vector2<Real> p;

            // Find the right-most point of the left subhull.
            Vector2<Real> pmax0 = mPoints[mHull[j0]];
            int imax0 = j0;
            for (i = j0 + 1; i <= j1; ++i)
            {
                p = mPoints[mHull[i]];
                if (pmax0 < p)
                {
                    pmax0 = p;
                    imax0 = i;
                }
            }

            // Find the left-most point of the right subhull.
            Vector2<Real> pmin1 = mPoints[mHull[j2]];
            int imin1 = j2;
            for (i = j2 + 1; i <= j3; ++i)
            {
                p = mPoints[mHull[i]];
                if (p < pmin1)
                {
                    pmin1 = p;
                    imin1 = i;
                }
            }

            // Get the lower tangent to hulls (LL = lower-left,
            // LR = lower-right).
            int iLL = imax0, iLR = imin1;
            GetTangent(j0, j1, j2, j3, iLL, iLR);

            // Get the upper tangent to hulls (UL = upper-left,
            // UR = upper-right).
            int iUL = imax0, iUR = imin1;
            GetTangent(j2, j3, j0, j1, iUR, iUL);

            // Construct the counterclockwise-ordered merged-hull vertices.
            int k;
            int numMerged = 0;

            i = iUL;
            for (k = 0; k < size0; ++k)
            {
                mMerged[numMerged++] = mHull[i];
                if (i == iLL)
                {
                    break;
                }
                i = (i < j1 ? i + 1 : j0);
            }
            LogAssert(k < size0, "Unexpected condition.");

            i = iLR;
            for (k = 0; k < size1; ++k)
            {
                mMerged[numMerged++] = mHull[i];
                if (i == iUR)
                {
                    break;
                }
                i = (i < j3 ? i + 1 : j2);
            }
            LogAssert(k < size1, "Unexpected condition.");

            int next = j0;
            for (k = 0; k < numMerged; ++k)
            {
                mHull[next] = mMerged[k];
                ++next;
            }

            i0 = j0;
            i1 = next - 1;
        }

        void GetTangent(int j0, int j1, int j2, int j3, int& i0, int& i1)
        {
            // In theory the loop terminates in a finite number of steps,
            // but the upper bound for the loop variable is used to trap
            // problems caused by floating-point roundoff errors that might
            // lead to an infinite loop.

            int size0 = j1 - j0 + 1;
            int size1 = j3 - j2 + 1;
            int const imax = size0 + size1;
            int i, iLm1, iRp1;
            int L0index, L1index, R0index, R1index;

            for (i = 0; i < imax; i++)
            {
                // Get the endpoints of the potential tangent.
                L1index = mHull[i0];
                R0index = mHull[i1];

                // Walk along the left hull to find the point of tangency.
                if (size0 > 1)
                {
                    iLm1 = (i0 > j0 ? i0 - 1 : j1);
                    L0index = mHull[iLm1];
                    auto order = ToLineExtended(R0index, L0index, L1index);
                    if (order == Order::NEGATIVE || order == Order::COLLINEAR_RIGHT)
                    {
                        i0 = iLm1;
                        continue;
                    }
                }

                // Walk along right hull to find the point of tangency.
                if (size1 > 1)
                {
                    iRp1 = (i1 < j3 ? i1 + 1 : j2);
                    R1index = mHull[iRp1];
                    auto order = ToLineExtended(L1index, R0index, R1index);
                    if (order == Order::NEGATIVE || order == Order::COLLINEAR_LEFT)
                    {
                        i1 = iRp1;
                        continue;
                    }
                }

                // The tangent segment has been found.
                break;
            }

            // Detect an "infinite loop" caused by floating point round-off
            // errors.
#if defined(GTE_THROW_ON_CONVEXHULL2_INFINITE_LOOP)
            LogAssert(i < imax, "Unexpected condition.");
#endif
        }

        // Memoized access to the rational representation of the points.
        Vector2<Rational> const& GetRationalPoint(int index) const
        {
            if (mConverted[index] == 0)
            {
                mConverted[index] = 1;
                for (int i = 0; i < 2; ++i)
                {
                    mRationalPoints[index][i] = mPoints[index][i];
                }
            }
            return mRationalPoints[index];
        }

        // An extended classification of the relationship of a point to a line
        // segment.  For noncollinear points, the return value is
        //   POSITIVE when <P,Q0,Q1> is a counterclockwise triangle
        //   NEGATIVE when <P,Q0,Q1> is a clockwise triangle
        // For collinear points, the line direction is Q1-Q0.  The return
        // value is
        //   COLLINEAR_LEFT when the line ordering is <P,Q0,Q1>
        //   COLLINEAR_RIGHT when the line ordering is <Q0,Q1,P>
        //   COLLINEAR_CONTAIN when the line ordering is <Q0,P,Q1>
        enum class Order
        {
            Q0_EQUALS_Q1,
            P_EQUALS_Q0,
            P_EQUALS_Q1,
            POSITIVE,
            NEGATIVE,
            COLLINEAR_LEFT,
            COLLINEAR_RIGHT,
            COLLINEAR_CONTAIN
        };

        Order ToLineExtended(int pIndex, int q0Index, int q1Index) const
        {
            Vector2<Real> const& P = mPoints[pIndex];
            Vector2<Real> const& Q0 = mPoints[q0Index];
            Vector2<Real> const& Q1 = mPoints[q1Index];

            if (Q1[0] == Q0[0] && Q1[1] == Q0[1])
            {
                return Order::Q0_EQUALS_Q1;
            }

            if (P[0] == Q0[0] && P[1] == Q0[1])
            {
                return Order::P_EQUALS_Q0;
            }

            if (P[0] == Q1[0] && P[1] == Q1[1])
            {
                return Order::P_EQUALS_Q1;
            }

            // The theoretical classification relies on computing exactly the
            // sign of the determinant.  Numerical roundoff errors can cause
            // misclassification.
            Real const zero(0);
            Interval ip0(P[0]), ip1(P[1]);
            Interval iq00(Q0[0]), iq01(Q0[1]), iq10(Q1[0]), iq11(Q1[1]);
            Interval ix0 = iq10 - iq00, iy0 = iq11 - iq01;
            Interval ix1 = ip0 - iq00, iy1 = ip1 - iq01;
            Interval ix0y1 = ix0 * iy1;
            Interval ix1y0 = ix1 * iy0;
            Interval iDet = ix0y1 - ix1y0;
            int32_t sign;

            Vector2<Rational> rDiff0, rDiff1;
            Rational rDot;
            bool rDiff0Computed = false;
            bool rDiff1Computed = false;
            bool rDotComputed = false;

            if (iDet[0] > zero)
            {
                sign = +1;
            }
            else if (iDet[1] < zero)
            {
                sign = -1;
            }
            else
            {
                // The exact sign of the determinant is not known, so compute
                // the determinant using rational arithmetic.
                auto const& rP = GetRationalPoint(pIndex);
                auto const& rQ0 = GetRationalPoint(q0Index);
                auto const& rQ1 = GetRationalPoint(q1Index);
                rDiff0 = rQ1 - rQ0;
                rDiff1 = rP - rQ0;
                auto rDet = DotPerp(rDiff0, rDiff1);
                rDiff0Computed = true;
                rDiff1Computed = true;
                sign = rDet.GetSign();
            }

            if (sign > 0)
            {
                // The points form a counterclockwise triangle <P,Q0,Q1>.
                return Order::POSITIVE;
            }
            else if (sign < 0)
            {
                // The points form a clockwise triangle <P,Q1,Q0>.
                return Order::NEGATIVE;
            }
            else
            {
                // The points are collinear. P is on the line through Q0
                // and Q1.
                Interval iDot = ix0 * ix1 + iy0 * iy1;
                if (iDot[0] > zero)
                {
                    sign = +1;
                }
                else if (iDot[1] < zero)
                {
                    sign = -1;
                }
                else
                {
                    // The exact sign of the dot product is not known, so
                    // compute the dot product using rational arithmetic.
                    auto const& rP = GetRationalPoint(pIndex);
                    auto const& rQ0 = GetRationalPoint(q0Index);
                    auto const& rQ1 = GetRationalPoint(q1Index);
                    if (!rDiff0Computed)
                    {
                        rDiff0 = rQ1 - rQ0;
                    }
                    if (!rDiff1Computed)
                    {
                        rDiff1 = rP - rQ0;
                    }
                    rDot = Dot(rDiff0, rDiff1);
                    rDotComputed = true;
                    sign = rDot.GetSign();
                }

                if (sign < zero)
                {
                    // The line ordering is <P,Q0,Q1>.
                    return Order::COLLINEAR_LEFT;
                }

                Interval iSqrLength = ix0 * ix0 + iy0 * iy0;
                Interval iTest = iDot - iSqrLength;
                if (iTest[0] > zero)
                {
                    sign = +1;
                }
                else if (iTest[1] < zero)
                {
                    sign = -1;
                }
                else
                {
                    // The exact sign of the test is not known, so
                    // compute the test using rational arithmetic.
                    auto const& rP = GetRationalPoint(pIndex);
                    auto const& rQ0 = GetRationalPoint(q0Index);
                    auto const& rQ1 = GetRationalPoint(q1Index);
                    if (!rDiff0Computed)
                    {
                        rDiff0 = rQ1 - rQ0;
                    }
                    if (!rDiff1Computed)
                    {
                        rDiff1 = rP - rQ0;
                    }
                    if (!rDotComputed)
                    {
                        rDot = Dot(rDiff0, rDiff1);
                    }
                    auto rSqrLength = Dot(rDiff0, rDiff0);
                    auto rTest = rDot - rSqrLength;
                    sign = rTest.GetSign();
                }

                if (sign > 0)
                {
                    // The line ordering is <Q0,Q1,P>.
                    return Order::COLLINEAR_RIGHT;
                }

                // The line ordering is <Q0,P,Q1> with P strictly between
                // Q0 and Q1.
                return Order::COLLINEAR_CONTAIN;
            }
        }

        // The epsilon value is used for fuzzy determination of intrinsic
        // dimensionality. If the dimension is 0 or 1, the constructor returns
        // early. The caller is responsible for retrieving the dimension and
        // taking an alternate path should the dimension be smaller than 2.
        // If the dimension is 0, the caller may as well treat all points[]
        // as a single point, say, points[0]. If the dimension is 1, the
        // caller can query for the approximating line and project points[]
        // onto it for further processing.
        Real mEpsilon;
        int mDimension;
        Line2<Real> mLine;

        // The array of rational points used for the exact predicate. The
        // mConverted array is used to store 0 or 1, where initially the
        // values are 0. The first time mComputePoints[i] is encountered,
        // mConverted[i] is 0. The floating-point vector is converted to
        // a rational number, after which mConverted[1] is set to 1 to
        // avoid converting again if the floating-point vector is
        // encountered in another predicate computation.
        mutable std::vector<Vector2<Rational>> mRationalPoints;
        mutable std::vector<uint32_t> mConverted;

        int mNumPoints;
        int mNumUniquePoints;
        Vector2<Real> const* mPoints;
        std::vector<int> mMerged, mHull;
    };
}
