// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.25

#pragma once

#include <Mathematics/DistPointAlignedBox.h>
#include <Mathematics/IntrRay3AlignedBox3.h>
#include <Mathematics/Hypersphere.h>

// The find-intersection query is based on the document
// https://www.geometrictools.com/Documentation/IntersectionMovingSphereBox.pdf
// and also uses the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf

namespace gte
{
    template <typename Real>
    class TIQuery<Real, AlignedBox3<Real>, Sphere3<Real>>
    {
    public:
        // The intersection query considers the box and sphere to be solids;
        // that is, the sphere object includes the region inside the spherical
        // boundary and the box object includes the region inside the cuboid
        // boundary.  If the sphere object and box object object overlap, the
        // objects intersect.
        struct Result
        {
            bool intersect;
        };

        Result operator()(AlignedBox3<Real> const& box, Sphere3<Real> const& sphere)
        {
            DCPQuery<Real, Vector3<Real>, AlignedBox3<Real>> pbQuery;
            auto pbResult = pbQuery(sphere.center, box);
            Result result;
            result.intersect = (pbResult.sqrDistance <= sphere.radius * sphere.radius);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, AlignedBox3<Real>, Sphere3<Real>>
    {
    public:
        // Currently, only a dynamic query is supported.  A static query will
        // need to compute the intersection set of (solid) box and sphere.
        struct Result
        {
            // The cases are
            // 1. Objects initially overlapping.  The contactPoint is only one
            //    of infinitely many points in the overlap.
            //      intersectionType = -1
            //      contactTime = 0
            //      contactPoint = sphere.center
            // 2. Objects initially separated but do not intersect later.  The
            //      contactTime and contactPoint are invalid.
            //      intersectionType = 0
            //      contactTime = 0
            //      contactPoint = (0,0,0)
            // 3. Objects initially separated but intersect later.
            //      intersectionType = +1
            //      contactTime = first time T > 0
            //      contactPoint = corresponding first contact
            int intersectionType;
            Real contactTime;
            Vector3<Real> contactPoint;

            // TODO: To support arbitrary precision for the contactTime,
            // return q0, q1 and q2 where contactTime = (q0 - sqrt(q1)) / q2.
            // The caller can compute contactTime to desired number of digits
            // of precision.  These are valid when intersectionType is +1 but
            // are set to zero (invalid) in the other cases.  Do the same for
            // the contactPoint.
        };

        Result operator()(AlignedBox3<Real> const& box, Vector3<Real> const& boxVelocity,
            Sphere3<Real> const& sphere, Vector3<Real> const& sphereVelocity)
        {
            Result result = { 0, (Real)0, { (Real)0, (Real)0, (Real)0 } };

            // Translate the sphere and box so that the box center becomes
            // the origin.  Compute the velocity of the sphere relative to
            // the box.
            Vector3<Real> boxCenter = (box.max + box.min) * (Real)0.5;
            Vector3<Real> extent = (box.max - box.min) * (Real)0.5;
            Vector3<Real> C = sphere.center - boxCenter;
            Vector3<Real> V = sphereVelocity - boxVelocity;

            // Test for no-intersection that leads to an early exit.  The test
            // is fast, using the method of separating axes.
            AlignedBox3<Real> superBox;
            for (int i = 0; i < 3; ++i)
            {
                superBox.max[i] = extent[i] + sphere.radius;
                superBox.min[i] = -superBox.max[i];
            }
            TIQuery<Real, Ray3<Real>, AlignedBox3<Real>> rbQuery;
            auto rbResult = rbQuery(Ray3<Real>(C, V), superBox);
            if (rbResult.intersect)
            {
                DoQuery(extent, C, sphere.radius, V, result);

                // Translate the contact point back to the coordinate system
                // of the original sphere and box.
                result.contactPoint += boxCenter;
            }
            return result;
        }

    protected:
        // The query assumes the box is axis-aligned with center at the
        // origin. Callers need to convert the results back to the original
        // coordinate system of the query.
        void DoQuery(Vector3<Real> const& K, Vector3<Real> const& inC,
            Real radius, Vector3<Real> const& inV, Result& result)
        {
            // Change signs on components, if necessary, to transform C to the
            // first quadrant. Adjust the velocity accordingly.
            Vector3<Real> C = inC, V = inV;
            Real sign[3];
            for (int i = 0; i < 3; ++i)
            {
                if (C[i] >= (Real)0)
                {
                    sign[i] = (Real)1;
                }
                else
                {
                    C[i] = -C[i];
                    V[i] = -V[i];
                    sign[i] = (Real)-1;
                }
            }

            Vector3<Real> delta = C - K;
            if (delta[2] <= radius)
            {
                if (delta[1] <= radius)
                {
                    if (delta[0] <= radius)
                    {
                        if (delta[2] <= (Real)0)
                        {
                            if (delta[1] <= (Real)0)
                            {
                                if (delta[0] <= (Real)0)
                                {
                                    InteriorOverlap(C, result);
                                }
                                else
                                {
                                    // x-face
                                    FaceOverlap(0, 1, 2, K, C, radius, delta, result);
                                }
                            }
                            else
                            {
                                if (delta[0] <= (Real)0)
                                {
                                    // y-face
                                    FaceOverlap(1, 2, 0, K, C, radius, delta, result);
                                }
                                else
                                {
                                    // xy-edge
                                    if (delta[0] * delta[0] + delta[1] * delta[1] <= radius * radius)
                                    {
                                        EdgeOverlap(0, 1, 2, K, C, radius, delta, result);
                                    }
                                    else
                                    {
                                        EdgeSeparated(0, 1, 2, K, C, radius, delta, V, result);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (delta[1] <= (Real)0)
                            {
                                if (delta[0] <= (Real)0)
                                {
                                    // z-face
                                    FaceOverlap(2, 0, 1, K, C, radius, delta, result);
                                }
                                else
                                {
                                    // xz-edge
                                    if (delta[0] * delta[0] + delta[2] * delta[2] <= radius * radius)
                                    {
                                        EdgeOverlap(2, 0, 1, K, C, radius, delta, result);
                                    }
                                    else
                                    {
                                        EdgeSeparated(2, 0, 1, K, C, radius, delta, V, result);
                                    }
                                }
                            }
                            else
                            {
                                if (delta[0] <= (Real)0)
                                {
                                    // yz-edge
                                    if (delta[1] * delta[1] + delta[2] * delta[2] <= radius * radius)
                                    {
                                        EdgeOverlap(1, 2, 0, K, C, radius, delta, result);
                                    }
                                    else
                                    {
                                        EdgeSeparated(1, 2, 0, K, C, radius, delta, V, result);
                                    }
                                }
                                else
                                {
                                    // xyz-vertex
                                    if (Dot(delta, delta) <= radius * radius)
                                    {
                                        VertexOverlap(K, radius, delta, result);
                                    }
                                    else
                                    {
                                        VertexSeparated(K, radius, delta, V, result);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // x-face
                        FaceUnbounded(0, 1, 2, K, C, radius, delta, V, result);
                    }
                }
                else
                {
                    if (delta[0] <= radius)
                    {
                        // y-face
                        FaceUnbounded(1, 2, 0, K, C, radius, delta, V, result);
                    }
                    else
                    {
                        // xy-edge
                        EdgeUnbounded(0, 1, 2, K, C, radius, delta, V, result);
                    }
                }
            }
            else
            {
                if (delta[1] <= radius)
                {
                    if (delta[0] <= radius)
                    {
                        // z-face
                        FaceUnbounded(2, 0, 1, K, C, radius, delta, V, result);
                    }
                    else
                    {
                        // xz-edge
                        EdgeUnbounded(2, 0, 1, K, C, radius, delta, V, result);
                    }
                }
                else
                {
                    if (delta[0] <= radius)
                    {
                        // yz-edge
                        EdgeUnbounded(1, 2, 0, K, C, radius, delta, V, result);
                    }
                    else
                    {
                        // xyz-vertex
                        VertexUnbounded(K, C, radius, delta, V, result);
                    }
                }
            }

            if (result.intersectionType != 0)
            {
                // Translate back to the coordinate system of the
                // tranlated box and sphere.
                for (int i = 0; i < 3; ++i)
                {
                    if (sign[i] < (Real)0)
                    {
                        result.contactPoint[i] = -result.contactPoint[i];
                    }
                }
            }
        }

    private:
        void InteriorOverlap(Vector3<Real> const& C, Result& result)
        {
            result.intersectionType = -1;
            result.contactTime = (Real)0;
            result.contactPoint = C;
        }

        void VertexOverlap(Vector3<Real> const& K, Real radius,
            Vector3<Real> const& delta, Result& result)
        {
            result.intersectionType = (Dot(delta, delta) < radius * radius ? -1 : 1);
            result.contactTime = (Real)0;
            result.contactPoint = K;
        }

        void EdgeOverlap(int i0, int i1, int i2, Vector3<Real> const& K,
            Vector3<Real> const& C, Real radius, Vector3<Real> const& delta,
            Result& result)
        {
            result.intersectionType = (delta[i0] * delta[i0] + delta[i1] * delta[i1] < radius * radius ? -1 : 1);
            result.contactTime = (Real)0;
            result.contactPoint[i0] = K[i0];
            result.contactPoint[i1] = K[i1];
            result.contactPoint[i2] = C[i2];
        }

        void FaceOverlap(int i0, int i1, int i2, Vector3<Real> const& K,
            Vector3<Real> const& C, Real radius, Vector3<Real> const& delta,
            Result& result)
        {
            result.intersectionType = (delta[i0] < radius ? -1 : 1);
            result.contactTime = (Real)0;
            result.contactPoint[i0] = K[i0];
            result.contactPoint[i1] = C[i1];
            result.contactPoint[i2] = C[i2];
        }

        void VertexSeparated(Vector3<Real> const& K, Real radius,
            Vector3<Real> const& delta, Vector3<Real> const& V, Result& result)
        {
            if (V[0] < (Real)0 || V[1] < (Real)0 || V[2] < (Real)0)
            {
                DoQueryRayRoundedVertex(K, radius, delta, V, result);
            }
        }

        void EdgeSeparated(int i0, int i1, int i2, Vector3<Real> const& K,
            Vector3<Real> const& C, Real radius, Vector3<Real> const& delta,
            Vector3<Real> const& V, Result& result)
        {
            if (V[i0] < (Real)0 || V[i1] < (Real)0)
            {
                DoQueryRayRoundedEdge(i0, i1, i2, K, C, radius, delta, V, result);
            }
        }

        void VertexUnbounded(Vector3<Real> const& K, Vector3<Real> const& C, Real radius,
            Vector3<Real> const& delta, Vector3<Real> const& V, Result& result)
        {
            if (V[0] < (Real)0 && V[1] < (Real)0 && V[2] < (Real)0)
            {
                // Determine the face of the rounded box that is intersected
                // by the ray C+T*V.
                Real T = (radius - delta[0]) / V[0];
                int j0 = 0;
                Real temp = (radius - delta[1]) / V[1];
                if (temp > T)
                {
                    T = temp;
                    j0 = 1;
                }
                temp = (radius - delta[2]) / V[2];
                if (temp > T)
                {
                    T = temp;
                    j0 = 2;
                }

                // The j0-rounded face is the candidate for intersection.
                int j1 = (j0 + 1) % 3;
                int j2 = (j1 + 1) % 3;
                DoQueryRayRoundedFace(j0, j1, j2, K, C, radius, delta, V, result);
            }
        }

        void EdgeUnbounded(int i0, int i1, int /* i2 */, Vector3<Real> const& K,
            Vector3<Real> const& C, Real radius, Vector3<Real> const& delta,
            Vector3<Real> const& V, Result& result)
        {
            if (V[i0] < (Real)0 && V[i1] < (Real)0)
            {
                // Determine the face of the rounded box that is intersected
                // by the ray C+T*V.
                Real T = (radius - delta[i0]) / V[i0];
                int j0 = i0;
                Real temp = (radius - delta[i1]) / V[i1];
                if (temp > T)
                {
                    T = temp;
                    j0 = i1;
                }

                // The j0-rounded face is the candidate for intersection.
                int j1 = (j0 + 1) % 3;
                int j2 = (j1 + 1) % 3;
                DoQueryRayRoundedFace(j0, j1, j2, K, C, radius, delta, V, result);
            }
        }

        void FaceUnbounded(int i0, int i1, int i2, Vector3<Real> const& K,
            Vector3<Real> const& C, Real radius, Vector3<Real> const& delta,
            Vector3<Real> const& V, Result& result)
        {
            if (V[i0] < (Real)0)
            {
                DoQueryRayRoundedFace(i0, i1, i2, K, C, radius, delta, V, result);
            }
        }

        void DoQueryRayRoundedVertex(Vector3<Real> const& K, Real radius,
            Vector3<Real> const& delta, Vector3<Real> const& V, Result& result)
        {
            Real a1 = Dot(V, delta);
            if (a1 < (Real)0)
            {
                // The caller must ensure that a0 > 0 and a2 > 0.
                Real a0 = Dot(delta, delta) - radius * radius;
                Real a2 = Dot(V, V);
                Real adiscr = a1 * a1 - a2 * a0;
                if (adiscr >= (Real)0)
                {
                    // The ray intersects the rounded vertex, so the sphere-box
                    // contact point is the vertex.
                    result.intersectionType = 1;
                    result.contactTime = -(a1 + std::sqrt(adiscr)) / a2;
                    result.contactPoint = K;
                }
            }
        }

        void DoQueryRayRoundedEdge(int i0, int i1, int i2, Vector3<Real> const& K,
            Vector3<Real> const& C, Real radius, Vector3<Real> const& delta,
            Vector3<Real> const& V, Result& result)
        {
            Real b1 = V[i0] * delta[i0] + V[i1] * delta[i1];
            if (b1 < (Real)0)
            {
                // The caller must ensure that b0 > 0 and b2 > 0.
                Real b0 = delta[i0] * delta[i0] + delta[i1] * delta[i1] - radius * radius;
                Real b2 = V[i0] * V[i0] + V[i1] * V[i1];
                Real bdiscr = b1 * b1 - b2 * b0;
                if (bdiscr >= (Real)0)
                {
                    Real T = -(b1 + std::sqrt(bdiscr)) / b2;
                    Real p2 = C[i2] + T * V[i2];
                    if (-K[i2] <= p2)
                    {
                        if (p2 <= K[i2])
                        {
                            // The ray intersects the finite cylinder of the
                            // rounded edge, so the sphere-box contact point
                            // is on the corresponding box edge.
                            result.intersectionType = 1;
                            result.contactTime = T;
                            result.contactPoint[i0] = K[i0];
                            result.contactPoint[i1] = K[i1];
                            result.contactPoint[i2] = p2;
                        }
                        else
                        {
                            // The ray intersects the infinite cylinder but
                            // not the finite cylinder of the rounded edge.
                            // It is possible the ray intersects the rounded
                            // vertex for K.
                            DoQueryRayRoundedVertex(K, radius, delta, V, result);
                        }
                    }
                    else
                    {
                        // The ray intersects the infinite cylinder but
                        // not the finite cylinder of the rounded edge.
                        // It is possible the ray intersects the rounded
                        // vertex for otherK.
                        Vector3<Real> otherK, otherDelta;
                        otherK[i0] = K[i0];
                        otherK[i1] = K[i1];
                        otherK[i2] = -K[i2];
                        otherDelta[i0] = C[i0] - otherK[i0];
                        otherDelta[i1] = C[i1] - otherK[i1];
                        otherDelta[i2] = C[i2] - otherK[i2];
                        DoQueryRayRoundedVertex(otherK, radius, otherDelta, V, result);
                    }
                }
            }
        }

        void DoQueryRayRoundedFace(int i0, int i1, int i2, Vector3<Real> const& K,
            Vector3<Real> const& C, Real radius, Vector3<Real> const& delta,
            Vector3<Real> const& V, Result& result)
        {
            Vector3<Real> otherK, otherDelta;

            Real T = (radius - delta[i0]) / V[i0];
            Real p1 = C[i1] + T * V[i1];
            Real p2 = C[i2] + T * V[i2];

            if (p1 < -K[i1])
            {
                // The ray potentially intersects the rounded (i0,i1)-edge
                // whose top-most vertex is otherK.
                otherK[i0] = K[i0];
                otherK[i1] = -K[i1];
                otherK[i2] = K[i2];
                otherDelta[i0] = C[i0] - otherK[i0];
                otherDelta[i1] = C[i1] - otherK[i1];
                otherDelta[i2] = C[i2] - otherK[i2];
                DoQueryRayRoundedEdge(i0, i1, i2, otherK, C, radius, otherDelta, V, result);
                if (result.intersectionType == 0)
                {
                    if (p2 < -K[i2])
                    {
                        // The ray potentially intersects the rounded
                        // (i2,i0)-edge whose right-most vertex is otherK.
                        otherK[i0] = K[i0];
                        otherK[i1] = K[i1];
                        otherK[i2] = -K[i2];
                        otherDelta[i0] = C[i0] - otherK[i0];
                        otherDelta[i1] = C[i1] - otherK[i1];
                        otherDelta[i2] = C[i2] - otherK[i2];
                        DoQueryRayRoundedEdge(i2, i0, i1, otherK, C, radius, otherDelta, V, result);
                    }
                    else if (p2 > K[i2])
                    {
                        // The ray potentially intersects the rounded
                        // (i2,i0)-edge whose right-most vertex is K.
                        DoQueryRayRoundedEdge(i2, i0, i1, K, C, radius, delta, V, result);
                    }
                }
            }
            else if (p1 <= K[i1])
            {
                if (p2 < -K[i2])
                {
                    // The ray potentially intersects the rounded
                    // (i2,i0)-edge whose right-most vertex is otherK.
                    otherK[i0] = K[i0];
                    otherK[i1] = K[i1];
                    otherK[i2] = -K[i2];
                    otherDelta[i0] = C[i0] - otherK[i0];
                    otherDelta[i1] = C[i1] - otherK[i1];
                    otherDelta[i2] = C[i2] - otherK[i2];
                    DoQueryRayRoundedEdge(i2, i0, i1, otherK, C, radius, otherDelta, V, result);
                }
                else if (p2 <= K[i2])
                {
                    // The ray intersects the i0-face of the rounded box, so
                    // the sphere-box contact point is on the corresponding
                    // box face.
                    result.intersectionType = 1;
                    result.contactTime = T;
                    result.contactPoint[i0] = K[i0];
                    result.contactPoint[i1] = p1;
                    result.contactPoint[i2] = p2;
                }
                else  // p2 > K[i2]
                {
                    // The ray potentially intersects the rounded
                    // (i2,i0)-edge whose right-most vertex is K.
                    DoQueryRayRoundedEdge(i2, i0, i1, K, C, radius, delta, V, result);
                }
            }
            else // p1 > K[i1]
            {
                // The ray potentially intersects the rounded (i0,i1)-edge
                // whose top-most vertex is K.
                DoQueryRayRoundedEdge(i0, i1, i2, K, C, radius, delta, V, result);
                if (result.intersectionType == 0)
                {
                    if (p2 < -K[i2])
                    {
                        // The ray potentially intersects the rounded
                        // (i2,i0)-edge whose right-most vertex is otherK.
                        otherK[i0] = K[i0];
                        otherK[i1] = K[i1];
                        otherK[i2] = -K[i2];
                        otherDelta[i0] = C[i0] - otherK[i0];
                        otherDelta[i1] = C[i1] - otherK[i1];
                        otherDelta[i2] = C[i2] - otherK[i2];
                        DoQueryRayRoundedEdge(i2, i0, i1, otherK, C, radius, otherDelta, V, result);
                    }
                    else if (p2 > K[i2])
                    {
                        // The ray potentially intersects the rounded
                        // (i2,i0)-edge whose right-most vertex is K.
                        DoQueryRayRoundedEdge(i2, i0, i1, K, C, radius, delta, V, result);
                    }
                }
            }
        }
    };
}
