// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/AlignedBox.h>
#include <Mathematics/Cone.h>
#include <Mathematics/IntrRay3AlignedBox3.h>
#include <Mathematics/IntrSegment3AlignedBox3.h>

// Test for intersection of a box and a cone.  The cone can be infinite
//   0 <= minHeight < maxHeight = std::numeric_limits<Real>::max()
// or finite (cone frustum)
//   0 <= minHeight < maxHeight < std::numeric_limits<Real>::max().
// The algorithm is described in
//   https://www.geometrictools.com/Documentation/IntersectionBoxCone.pdf
// and reports an intersection only when the intersection set has positive
// volume.  For example, let the box be outside the cone.  If the box is
// below the minHeight plane at the cone vertex and just touches the cone
// vertex, no intersection is reported.  If the box is above the maxHeight
// plane and just touches the disk capping the cone, either at a single
// point, a line segment of points or a polygon of points, no intersection
// is reported.

// TODO: These queries were designed when an infinite cone was defined
// by choosing maxHeight of std::numeric_limits<Real>::max(). The Cone<N,Real>
// class has been redesigned not to use std::numeric_limits to allow for
// arithmetic systems that do not have representations for infinities
// (such as BSNumber and BSRational).  The intersection queries need to be
// rewritten for the new class design.  FOR NOW, the queries will work with
// float/double when you create a cone using the cone-frustum constructor
// Cone(ray, angle, minHeight, std::numeric_limits<Real>::max()).

namespace gte
{
    template <typename Real>
    class TIQuery<Real, AlignedBox<3, Real>, Cone<3, Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        TIQuery()
            :
            mNumCandidateEdges(0)
        {
            // An edge is { v0, v1 }, where v0 and v1 are relative to mVertices
            // with v0 < v1.
            mEdges[0] = { 0, 1 };
            mEdges[1] = { 1, 3 };
            mEdges[2] = { 2, 3 };
            mEdges[3] = { 0, 2 };
            mEdges[4] = { 4, 5 };
            mEdges[5] = { 5, 7 };
            mEdges[6] = { 6, 7 };
            mEdges[7] = { 4, 6 };
            mEdges[8] = { 0, 4 };
            mEdges[9] = { 1, 5 };
            mEdges[10] = { 3, 7 };
            mEdges[11] = { 2, 6 };

            // A face is { { v0, v1, v2, v3 }, { e0, e1, e2, e3 } }, where
            // { v0, v1, v2, v3 } are relative to mVertices with
            // v0 = min(v0,v1,v2,v3) and where { e0, e1, e2, e3 } are relative
            // to mEdges.  For example, mFaces[0] has vertices { 0, 4, 6, 2 }.
            // The edge { 0, 4 } is mEdges[8], the edge { 4, 6 } is mEdges[7],
            // the edge { 6, 2 } is mEdges[11] and the edge { 2, 0 } is
            // mEdges[3]; thus, the edge indices are { 8, 7, 11, 3 }.
            mFaces[0] = { { 0, 4, 6, 2 }, {  8,  7, 11,  3 } };
            mFaces[1] = { { 1, 3, 7, 5 }, {  1, 10,  5,  9 } };
            mFaces[2] = { { 0, 1, 5, 4 }, {  0,  9,  4,  8 } };
            mFaces[3] = { { 2, 6, 7, 3 }, { 11,  6, 10,  2 } };
            mFaces[4] = { { 0, 2, 3, 1 }, {  3,  2,  1,  0 } };
            mFaces[5] = { { 4, 5, 7, 6 }, {  4,  5,  6,  7 } };

            // Clear the edges.
            std::array<size_t, 2> ezero = { 0, 0 };
            mCandidateEdges.fill(ezero);
            for (size_t r = 0; r < MAX_VERTICES; ++r)
            {
                mAdjacencyMatrix[r].fill(0);
            }

            mConfiguration[0] = &TIQuery::NNNN_0;
            mConfiguration[1] = &TIQuery::NNNZ_1;
            mConfiguration[2] = &TIQuery::NNNP_2;
            mConfiguration[3] = &TIQuery::NNZN_3;
            mConfiguration[4] = &TIQuery::NNZZ_4;
            mConfiguration[5] = &TIQuery::NNZP_5;
            mConfiguration[6] = &TIQuery::NNPN_6;
            mConfiguration[7] = &TIQuery::NNPZ_7;
            mConfiguration[8] = &TIQuery::NNPP_8;
            mConfiguration[9] = &TIQuery::NZNN_9;
            mConfiguration[10] = &TIQuery::NZNZ_10;
            mConfiguration[11] = &TIQuery::NZNP_11;
            mConfiguration[12] = &TIQuery::NZZN_12;
            mConfiguration[13] = &TIQuery::NZZZ_13;
            mConfiguration[14] = &TIQuery::NZZP_14;
            mConfiguration[15] = &TIQuery::NZPN_15;
            mConfiguration[16] = &TIQuery::NZPZ_16;
            mConfiguration[17] = &TIQuery::NZPP_17;
            mConfiguration[18] = &TIQuery::NPNN_18;
            mConfiguration[19] = &TIQuery::NPNZ_19;
            mConfiguration[20] = &TIQuery::NPNP_20;
            mConfiguration[21] = &TIQuery::NPZN_21;
            mConfiguration[22] = &TIQuery::NPZZ_22;
            mConfiguration[23] = &TIQuery::NPZP_23;
            mConfiguration[24] = &TIQuery::NPPN_24;
            mConfiguration[25] = &TIQuery::NPPZ_25;
            mConfiguration[26] = &TIQuery::NPPP_26;
            mConfiguration[27] = &TIQuery::ZNNN_27;
            mConfiguration[28] = &TIQuery::ZNNZ_28;
            mConfiguration[29] = &TIQuery::ZNNP_29;
            mConfiguration[30] = &TIQuery::ZNZN_30;
            mConfiguration[31] = &TIQuery::ZNZZ_31;
            mConfiguration[32] = &TIQuery::ZNZP_32;
            mConfiguration[33] = &TIQuery::ZNPN_33;
            mConfiguration[34] = &TIQuery::ZNPZ_34;
            mConfiguration[35] = &TIQuery::ZNPP_35;
            mConfiguration[36] = &TIQuery::ZZNN_36;
            mConfiguration[37] = &TIQuery::ZZNZ_37;
            mConfiguration[38] = &TIQuery::ZZNP_38;
            mConfiguration[39] = &TIQuery::ZZZN_39;
            mConfiguration[40] = &TIQuery::ZZZZ_40;
            mConfiguration[41] = &TIQuery::ZZZP_41;
            mConfiguration[42] = &TIQuery::ZZPN_42;
            mConfiguration[43] = &TIQuery::ZZPZ_43;
            mConfiguration[44] = &TIQuery::ZZPP_44;
            mConfiguration[45] = &TIQuery::ZPNN_45;
            mConfiguration[46] = &TIQuery::ZPNZ_46;
            mConfiguration[47] = &TIQuery::ZPNP_47;
            mConfiguration[48] = &TIQuery::ZPZN_48;
            mConfiguration[49] = &TIQuery::ZPZZ_49;
            mConfiguration[50] = &TIQuery::ZPZP_50;
            mConfiguration[51] = &TIQuery::ZPPN_51;
            mConfiguration[52] = &TIQuery::ZPPZ_52;
            mConfiguration[53] = &TIQuery::ZPPP_53;
            mConfiguration[54] = &TIQuery::PNNN_54;
            mConfiguration[55] = &TIQuery::PNNZ_55;
            mConfiguration[56] = &TIQuery::PNNP_56;
            mConfiguration[57] = &TIQuery::PNZN_57;
            mConfiguration[58] = &TIQuery::PNZZ_58;
            mConfiguration[59] = &TIQuery::PNZP_59;
            mConfiguration[60] = &TIQuery::PNPN_60;
            mConfiguration[61] = &TIQuery::PNPZ_61;
            mConfiguration[62] = &TIQuery::PNPP_62;
            mConfiguration[63] = &TIQuery::PZNN_63;
            mConfiguration[64] = &TIQuery::PZNZ_64;
            mConfiguration[65] = &TIQuery::PZNP_65;
            mConfiguration[66] = &TIQuery::PZZN_66;
            mConfiguration[67] = &TIQuery::PZZZ_67;
            mConfiguration[68] = &TIQuery::PZZP_68;
            mConfiguration[69] = &TIQuery::PZPN_69;
            mConfiguration[70] = &TIQuery::PZPZ_70;
            mConfiguration[71] = &TIQuery::PZPP_71;
            mConfiguration[72] = &TIQuery::PPNN_72;
            mConfiguration[73] = &TIQuery::PPNZ_73;
            mConfiguration[74] = &TIQuery::PPNP_74;
            mConfiguration[75] = &TIQuery::PPZN_75;
            mConfiguration[76] = &TIQuery::PPZZ_76;
            mConfiguration[77] = &TIQuery::PPZP_77;
            mConfiguration[78] = &TIQuery::PPPN_78;
            mConfiguration[79] = &TIQuery::PPPZ_79;
            mConfiguration[80] = &TIQuery::PPPP_80;
        }

        Result operator()(AlignedBox<3, Real> const& box, Cone<3, Real> const& cone)
        {
            Result result;

            // Quick-rejectance test.  Determine whether the box is outside
            // the slab bounded by the minimum and maximum height planes.
            // When outside the slab, the box vertices are not required by the
            // cone-box intersection query, so the vertices are not yet
            // computed.
            Real boxMinHeight(0), boxMaxHeight(0);
            ComputeBoxHeightInterval(box, cone, boxMinHeight, boxMaxHeight);
            // TODO: See the comments at the beginning of this file.
            Real coneMaxHeight = (cone.IsFinite() ? cone.GetMaxHeight() : std::numeric_limits<Real>::max());
            if (boxMaxHeight <= cone.GetMinHeight() || boxMinHeight >= coneMaxHeight)
            {
                // There is no volumetric overlap of the box and the cone. The
                // box is clipped entirely.
                result.intersect = false;
                return result;
            }

            // Quick-acceptance test.  Determine whether the cone axis
            // intersects the box.
            if (ConeAxisIntersectsBox(box, cone))
            {
                result.intersect = true;
                return result;
            }

            // Test for box fully inside the slab.  When inside the slab, the
            // box vertices are required by the cone-box intersection query,
            // so they are computed here; they are also required in the
            // remaining cases.  Also when inside the slab, the box edges are
            // the candidates, so they are copied to mCandidateEdges.
            if (BoxFullyInConeSlab(box, boxMinHeight, boxMaxHeight, cone))
            {
                result.intersect = CandidatesHavePointInsideCone(cone);
                return result;
            }

            // Clear the candidates array and adjacency matrix.
            ClearCandidates();

            // The box intersects at least one plane.  Compute the box-plane
            // edge-interior intersection points.  Insert the box subedges into
            // the array of candidate edges.
            ComputeCandidatesOnBoxEdges(cone);

            // Insert any relevant box face-interior clipped edges into the array
            // of candidate edges.
            ComputeCandidatesOnBoxFaces();

            result.intersect = CandidatesHavePointInsideCone(cone);
            return result;
        }

    protected:
        // The constants here are described in the comments below.
        enum
        {
            NUM_BOX_VERTICES = 8,
            NUM_BOX_EDGES = 12,
            NUM_BOX_FACES = 6,
            MAX_VERTICES = 32,
            VERTEX_MIN_BASE = 8,
            VERTEX_MAX_BASE = 20,
            MAX_CANDIDATE_EDGES = 496,
            NUM_CONFIGURATIONS = 81
        };

        // The box topology is that of a cube whose vertices have components
        // in {0,1}.  The cube vertices are indexed by
        //   0: (0,0,0), 1: (1,0,0), 2: (1,1,0), 3: (0,1,0)
        //   4: (0,0,1), 5: (1,0,1), 6: (1,1,1), 7: (0,1,1)

        // The first 8 vertices are the box corners, the next 12 vertices are
        // reserved for hmin-edge points and the final 12 vertices are reserved
        // for the hmax-edge points.  The conservative upper bound of the number
        // of vertices is 8 + 12 + 12 = 32.
        std::array<Vector3<Real>, MAX_VERTICES> mVertices;

        // The box has 12 edges stored in mEdges.  An edge is mEdges[i] =
        // { v0, v1 }, where the indices v0 and v1 are relative to mVertices
        // with v0 < v1.
        std::array<std::array<size_t, 2>, NUM_BOX_EDGES> mEdges;

        // The box has 6 faces stored in mFaces.  A face is mFaces[i] =
        // { { v0, v1, v2, v3 }, { e0, e1, e2, e3 } }, where the face corner
        // vertices are { v0, v1, v2, v3 }.  These indices are relative to
        // mVertices.  The indices { e0, e1, e2, e3 } are relative to mEdges.
        // The index e0 refers to edge { v0, v1 }, the index e1 refers to edge
        // { v1, v2 }, the index e2 refers to edge { v2, v3 } and the index e3
        // refers to edge { v3, v0 }.  The ordering of vertices for the faces
        // is/ counterclockwise when viewed from outside the box.  The choice
        // of initial vertex affects how you implement the graph data
        // structure.  In this implementation, the initial vertex has minimum
        // index for all vertices of that face.  The faces themselves are
        // listed as -x face, +x face, -y face, +y face, -z face and +z face.
        struct Face
        {
            std::array<size_t, 4> v, e;
        };
        std::array<Face, NUM_BOX_FACES> mFaces;

        // Store the signed distances from the minimum and maximum height
        // planes for the cone to the projection of the box vertices onto the
        // cone axis.
        std::array<Real, NUM_BOX_VERTICES> mProjectionMin, mProjectionMax;

        // The mCandidateEdges array stores the edges of the clipped box that
        // are candidates for containing the optimizing point.  The maximum
        // number of candidate edges is 1 + 2 + ... + 31 = 496, which is a
        // conservative bound because not all pairs of vertices form edges on
        // box faces.  The candidate edges are stored as (v0,v1) with v0 < v1.
        // The implementation is designed so that during a single query, the
        // number of candidate edges can only grow.
        size_t mNumCandidateEdges;
        std::array<std::array<size_t, 2>, MAX_CANDIDATE_EDGES> mCandidateEdges;

        // The mAdjancencyMatrix is a simple representation of edges in the
        // graph G = (V,E) that represents the (wireframe) clipped box.  An
        // edge (r,c) does not exist when mAdjancencyMatrix[r][c] = 0.  If an
        // edge (r,c) does exist, it is appended to mCandidateEdges at index k
        // and the adjacency matrix is set to mAdjacencyMatrix[r][c] = 1.
        // This allows for a fast edge-in-graph test and a fast and efficient
        // clear of mCandidateEdges and mAdjacencyMatrix.
        std::array<std::array<size_t, MAX_VERTICES>, MAX_VERTICES> mAdjacencyMatrix;

        typedef void (TIQuery::* ConfigurationFunction)(size_t, Face const&);
        std::array<ConfigurationFunction, NUM_CONFIGURATIONS> mConfiguration;

        static void ComputeBoxHeightInterval(AlignedBox<3, Real> const& box, Cone<3, Real> const& cone,
            Real& boxMinHeight, Real& boxMaxHeight)
        {
            Vector<3, Real> C, e;
            box.GetCenteredForm(C, e);
            Vector<3, Real> const& V = cone.ray.origin;
            Vector<3, Real> const& U = cone.ray.direction;
            Vector<3, Real> CmV = C - V;
            Real DdCmV = Dot(U, CmV);
            Real radius = e[0] * std::abs(U[0]) + e[1] * std::abs(U[1]) + e[2] * std::abs(U[2]);
            boxMinHeight = DdCmV - radius;
            boxMaxHeight = DdCmV + radius;
        }

        static bool ConeAxisIntersectsBox(AlignedBox<3, Real> const& box, Cone<3, Real> const& cone)
        {
            if (cone.IsFinite())
            {
                Segment<3, Real> segment;
                segment.p[0] = cone.ray.origin + cone.GetMinHeight() * cone.ray.direction;
                segment.p[1] = cone.ray.origin + cone.GetMaxHeight() * cone.ray.direction;
                auto sbResult = TIQuery<Real, Segment<3, Real>, AlignedBox<3, Real>>()(segment, box);
                if (sbResult.intersect)
                {
                    return true;
                }
            }
            else
            {
                Ray<3, Real> ray;
                ray.origin = cone.ray.origin + cone.GetMinHeight() * cone.ray.direction;
                ray.direction = cone.ray.direction;
                auto rbResult = TIQuery<Real, Ray<3, Real>, AlignedBox<3, Real>>()(ray, box);
                if (rbResult.intersect)
                {
                    return true;
                }
            }
            return false;
        }

        bool BoxFullyInConeSlab(AlignedBox<3, Real> const& box, Real boxMinHeight, Real boxMaxHeight, Cone<3, Real> const& cone)
        {
            // Compute the box vertices relative to cone vertex as origin.
            mVertices[0] = { box.min[0], box.min[1], box.min[2] };
            mVertices[1] = { box.max[0], box.min[1], box.min[2] };
            mVertices[2] = { box.min[0], box.max[1], box.min[2] };
            mVertices[3] = { box.max[0], box.max[1], box.min[2] };
            mVertices[4] = { box.min[0], box.min[1], box.max[2] };
            mVertices[5] = { box.max[0], box.min[1], box.max[2] };
            mVertices[6] = { box.min[0], box.max[1], box.max[2] };
            mVertices[7] = { box.max[0], box.max[1], box.max[2] };
            for (int i = 0; i < NUM_BOX_VERTICES; ++i)
            {
                mVertices[i] -= cone.ray.origin;
            }

            Real coneMaxHeight = (cone.IsFinite() ? cone.GetMaxHeight() : std::numeric_limits<Real>::max());
            if (cone.GetMinHeight() <= boxMinHeight && boxMaxHeight <= coneMaxHeight)
            {
                // The box is fully inside, so no clipping is necessary.
                std::copy(mEdges.begin(), mEdges.end(), mCandidateEdges.begin());
                mNumCandidateEdges = 12;
                return true;
            }
            return false;
        }

        static bool HasPointInsideCone(Vector<3, Real> const& P0, Vector<3, Real> const& P1,
            Cone<3, Real> const& cone)
        {
            // Define F(X) = Dot(U,X - V)/|X - V|, where U is the unit-length
            // cone axis direction and V is the cone vertex.  The incoming
            // points P0 and P1 are relative to V; that is, the original
            // points are X0 = P0 + V and X1 = P1 + V.  The segment <P0,P1>
            // and cone intersect when a segment point X is inside the cone;
            // that is, when F(X) > cosAngle.  The comparison is converted to
            // an equivalent one that does not involve divisions in order to
            // avoid a division by zero if a vertex or edge contain (0,0,0).
            // The function is G(X) = Dot(U,X-V) - cosAngle*Length(X-V).
            Vector<3, Real> const& U = cone.ray.direction;

            // Test whether P0 or P1 is inside the cone.
            Real g = Dot(U, P0) - cone.cosAngle * Length(P0);
            if (g > (Real)0)
            {
                // X0 = P0 + V is inside the cone.
                return true;
            }

            g = Dot(U, P1) - cone.cosAngle * Length(P1);
            if (g > (Real)0)
            {
                // X1 = P1 + V is inside the cone.
                return true;
            }

            // Test whether an interior segment point is inside the cone.
            Vector<3, Real> E = P1 - P0;
            Vector<3, Real> crossP0U = Cross(P0, U);
            Vector<3, Real> crossP0E = Cross(P0, E);
            Real dphi0 = Dot(crossP0E, crossP0U);
            if (dphi0 > (Real)0)
            {
                Vector3<Real> crossP1U = Cross(P1, U);
                Real dphi1 = Dot(crossP0E, crossP1U);
                if (dphi1 < (Real)0)
                {
                    Real t = dphi0 / (dphi0 - dphi1);
                    Vector<3, Real> PMax = P0 + t * E;
                    g = Dot(U, PMax) - cone.cosAngle * Length(PMax);
                    if (g > (Real)0)
                    {
                        // The edge point XMax = Pmax + V is inside the cone.
                        return true;
                    }
                }
            }

            return false;
        }

        bool CandidatesHavePointInsideCone(Cone<3, Real> const& cone) const
        {
            for (size_t i = 0; i < mNumCandidateEdges; ++i)
            {
                auto const& edge = mCandidateEdges[i];
                Vector<3, Real> const& P0 = mVertices[edge[0]];
                Vector<3, Real> const& P1 = mVertices[edge[1]];
                if (HasPointInsideCone(P0, P1, cone))
                {
                    return true;
                }
            }
            return false;
        }

        void ComputeCandidatesOnBoxEdges(Cone<3, Real> const& cone)
        {
            for (size_t i = 0; i < NUM_BOX_VERTICES; ++i)
            {
                Real h = Dot(cone.ray.direction, mVertices[i]);
                Real coneMaxHeight = (cone.IsFinite() ? cone.GetMaxHeight() : std::numeric_limits<Real>::max());
                mProjectionMin[i] = cone.GetMinHeight() - h;
                mProjectionMax[i] = h - coneMaxHeight;
            }

            size_t v0 = VERTEX_MIN_BASE, v1 = VERTEX_MAX_BASE;
            for (size_t i = 0; i < NUM_BOX_EDGES; ++i, ++v0, ++v1)
            {
                auto const& edge = mEdges[i];

                // In the next blocks, the sign comparisons can be expressed
                // instead as "s0 * s1 < 0". The multiplication could lead to
                // floating-point underflow where the product becomes 0, so I
                // avoid that approach.

                // Process the hmin-plane.
                Real p0Min = mProjectionMin[edge[0]];
                Real p1Min = mProjectionMin[edge[1]];
                bool clipMin = (p0Min < (Real)0 && p1Min >(Real)0) || (p0Min > (Real)0 && p1Min < (Real)0);
                if (clipMin)
                {
                    mVertices[v0] = (p1Min * mVertices[edge[0]] - p0Min * mVertices[edge[1]]) / (p1Min - p0Min);
                }

                // Process the hmax-plane.
                Real p0Max = mProjectionMax[edge[0]];
                Real p1Max = mProjectionMax[edge[1]];
                bool clipMax = (p0Max < (Real)0 && p1Max >(Real)0) || (p0Max > (Real)0 && p1Max < (Real)0);
                if (clipMax)
                {
                    mVertices[v1] = (p1Max * mVertices[edge[0]] - p0Max * mVertices[edge[1]]) / (p1Max - p0Max);
                }

                // Get the candidate edges that are contained by the box edges.
                if (clipMin)
                {
                    if (clipMax)
                    {
                        InsertEdge(v0, v1);
                    }
                    else
                    {
                        if (p0Min < (Real)0)
                        {
                            InsertEdge(edge[0], v0);
                        }
                        else  // p1Min < 0
                        {
                            InsertEdge(edge[1], v0);
                        }
                    }
                }
                else if (clipMax)
                {
                    if (p0Max < (Real)0)
                    {
                        InsertEdge(edge[0], v1);
                    }
                    else  // p1Max < 0
                    {
                        InsertEdge(edge[1], v1);
                    }
                }
                else
                {
                    // No clipping has occurred.  If the edge is inside the box,
                    // it is a candidate edge.  To be inside the box, the p*min
                    // and p*max values must be nonpositive.
                    if (p0Min <= (Real)0 && p1Min <= (Real)0 && p0Max <= (Real)0 && p1Max <= (Real)0)
                    {
                        InsertEdge(edge[0], edge[1]);
                    }
                }
            }
        }

        void ComputeCandidatesOnBoxFaces()
        {
            Real p0, p1, p2, p3;
            size_t b0, b1, b2, b3, index;
            for (size_t i = 0; i < NUM_BOX_FACES; ++i)
            {
                auto const& face = mFaces[i];

                // Process the hmin-plane.
                p0 = mProjectionMin[face.v[0]];
                p1 = mProjectionMin[face.v[1]];
                p2 = mProjectionMin[face.v[2]];
                p3 = mProjectionMin[face.v[3]];
                b0 = (p0 < (Real)0 ? 0 : (p0 > (Real)0 ? 2 : 1));
                b1 = (p1 < (Real)0 ? 0 : (p1 > (Real)0 ? 2 : 1));
                b2 = (p2 < (Real)0 ? 0 : (p2 > (Real)0 ? 2 : 1));
                b3 = (p3 < (Real)0 ? 0 : (p3 > (Real)0 ? 2 : 1));
                index = b3 + 3 * (b2 + 3 * (b1 + 3 * b0));
                (this->*mConfiguration[index])(VERTEX_MIN_BASE, face);

                // Process the hmax-plane.
                p0 = mProjectionMax[face.v[0]];
                p1 = mProjectionMax[face.v[1]];
                p2 = mProjectionMax[face.v[2]];
                p3 = mProjectionMax[face.v[3]];
                b0 = (p0 < (Real)0 ? 0 : (p0 > (Real)0 ? 2 : 1));
                b1 = (p1 < (Real)0 ? 0 : (p1 > (Real)0 ? 2 : 1));
                b2 = (p2 < (Real)0 ? 0 : (p2 > (Real)0 ? 2 : 1));
                b3 = (p3 < (Real)0 ? 0 : (p3 > (Real)0 ? 2 : 1));
                index = b3 + 3 * (b2 + 3 * (b1 + 3 * b0));
                (this->*mConfiguration[index])(VERTEX_MAX_BASE, face);
            }
        }

        void ClearCandidates()
        {
            for (size_t i = 0; i < mNumCandidateEdges; ++i)
            {
                auto const& edge = mCandidateEdges[i];
                mAdjacencyMatrix[edge[0]][edge[1]] = 0;
                mAdjacencyMatrix[edge[1]][edge[0]] = 0;
            }
            mNumCandidateEdges = 0;
        }

        void InsertEdge(size_t v0, size_t v1)
        {
            if (mAdjacencyMatrix[v0][v1] == 0)
            {
                mAdjacencyMatrix[v0][v1] = 1;
                mAdjacencyMatrix[v1][v0] = 1;
                mCandidateEdges[mNumCandidateEdges] = { v0, v1 };
                ++mNumCandidateEdges;
            }
        }

        // The 81 possible configurations for a box face.  The N stands for a
        // '-', the Z stands for '0' and the P stands for '+'.  These are
        // listed in the order that maps to the array mConfiguration.  Thus,
        // NNNN maps to mConfiguration[0], NNNZ maps to mConfiguration[1], and
        // so on.
        void NNNN_0(size_t, Face const&)
        {
        }

        void NNNZ_1(size_t, Face const&)
        {
        }

        void NNNP_2(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[2], base + face.e[3]);
        }

        void NNZN_3(size_t, Face const&)
        {
        }

        void NNZZ_4(size_t, Face const&)
        {
        }

        void NNZP_5(size_t base, Face const& face)
        {
            InsertEdge(face.v[2], base + face.e[3]);
        }

        void NNPN_6(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[1], base + face.e[2]);
        }

        void NNPZ_7(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[1], face.v[3]);
        }

        void NNPP_8(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[1], base + face.e[3]);
        }

        void NZNN_9(size_t, Face const&)
        {
        }

        void NZNZ_10(size_t, Face const&)
        {
        }

        void NZNP_11(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[2], face.v[3]);
            InsertEdge(base + face.e[3], face.v[3]);
        }

        void NZZN_12(size_t, Face const&)
        {
        }

        void NZZZ_13(size_t, Face const&)
        {
        }

        void NZZP_14(size_t base, Face const& face)
        {
            InsertEdge(face.v[2], face.v[3]);
            InsertEdge(base + face.e[3], face.v[3]);
        }

        void NZPN_15(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[2], face.v[1]);
        }

        void NZPZ_16(size_t, Face const& face)
        {
            InsertEdge(face.v[1], face.v[3]);
        }

        void NZPP_17(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], face.v[1]);
        }

        void NPNN_18(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], base + face.e[1]);
        }

        void NPNZ_19(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], face.v[1]);
            InsertEdge(base + face.e[1], face.v[1]);
        }

        void NPNP_20(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], face.v[1]);
            InsertEdge(base + face.e[1], face.v[1]);
            InsertEdge(base + face.e[2], face.v[3]);
            InsertEdge(base + face.e[3], face.v[3]);
        }

        void NPZN_21(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], face.v[2]);
        }

        void NPZZ_22(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], face.v[1]);
            InsertEdge(face.v[1], face.v[2]);
        }

        void NPZP_23(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], face.v[1]);
            InsertEdge(face.v[1], face.v[2]);
            InsertEdge(base + face.e[3], face.v[2]);
            InsertEdge(face.v[2], face.v[3]);
        }

        void NPPN_24(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], base + face.e[2]);
        }

        void NPPZ_25(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], face.v[3]);
        }

        void NPPP_26(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], base + face.e[3]);
        }

        void ZNNN_27(size_t, Face const&)
        {
        }

        void ZNNZ_28(size_t, Face const&)
        {
        }

        void ZNNP_29(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[2], face.v[0]);
        }

        void ZNZN_30(size_t, Face const&)
        {
        }

        void ZNZZ_31(size_t, Face const&)
        {
        }

        void ZNZP_32(size_t, Face const& face)
        {
            InsertEdge(face.v[0], face.v[2]);
        }

        void ZNPN_33(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[1], face.v[2]);
            InsertEdge(base + face.e[2], face.v[2]);
        }

        void ZNPZ_34(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[1], face.v[2]);
            InsertEdge(face.v[2], face.v[3]);
        }

        void ZNPP_35(size_t base, Face const& face)
        {
            InsertEdge(face.v[0], base + face.e[1]);
        }

        void ZZNN_36(size_t, Face const&)
        {
        }

        void ZZNZ_37(size_t, Face const&)
        {
        }

        void ZZNP_38(size_t base, Face const& face)
        {
            InsertEdge(face.v[0], face.v[3]);
            InsertEdge(face.v[3], base + face.e[2]);
        }

        void ZZZN_39(size_t, Face const&)
        {
        }

        void ZZZZ_40(size_t, Face const&)
        {
        }

        void ZZZP_41(size_t, Face const& face)
        {
            InsertEdge(face.v[0], face.v[3]);
            InsertEdge(face.v[3], face.v[2]);
        }

        void ZZPN_42(size_t base, Face const& face)
        {
            InsertEdge(face.v[1], face.v[2]);
            InsertEdge(face.v[2], base + face.e[2]);
        }

        void ZZPZ_43(size_t, Face const& face)
        {
            InsertEdge(face.v[1], face.v[2]);
            InsertEdge(face.v[2], face.v[3]);
        }

        void ZZPP_44(size_t, Face const&)
        {
        }

        void ZPNN_45(size_t base, Face const& face)
        {
            InsertEdge(face.v[0], base + face.e[1]);
        }

        void ZPNZ_46(size_t base, Face const& face)
        {
            InsertEdge(face.v[0], face.v[1]);
            InsertEdge(face.v[1], base + face.e[1]);
        }

        void ZPNP_47(size_t base, Face const& face)
        {
            InsertEdge(face.v[0], face.v[1]);
            InsertEdge(face.v[1], base + face.e[1]);
            InsertEdge(base + face.e[2], face.v[3]);
            InsertEdge(face.v[3], face.v[0]);
        }

        void ZPZN_48(size_t, Face const& face)
        {
            InsertEdge(face.v[0], face.v[2]);
        }

        void ZPZZ_49(size_t, Face const& face)
        {
            InsertEdge(face.v[0], face.v[1]);
            InsertEdge(face.v[1], face.v[2]);
        }

        void ZPZP_50(size_t, Face const&)
        {
        }

        void ZPPN_51(size_t base, Face const& face)
        {
            InsertEdge(face.v[0], base + face.e[2]);
        }

        void ZPPZ_52(size_t, Face const&)
        {
        }

        void ZPPP_53(size_t, Face const&)
        {
        }

        void PNNN_54(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], base + face.e[0]);
        }

        void PNNZ_55(size_t base, Face const& face)
        {
            InsertEdge(face.v[3], base + face.e[0]);
        }

        void PNNP_56(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[2], base + face.e[0]);
        }

        void PNZN_57(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], face.v[0]);
            InsertEdge(face.v[0], base + face.e[0]);
        }

        void PNZZ_58(size_t base, Face const& face)
        {
            InsertEdge(face.v[3], face.v[0]);
            InsertEdge(face.v[0], base + face.e[0]);
        }

        void PNZP_59(size_t base, Face const& face)
        {
            InsertEdge(face.v[2], base + face.e[0]);
        }

        void PNPN_60(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], face.v[0]);
            InsertEdge(face.v[0], base + face.e[0]);
            InsertEdge(base + face.e[1], face.v[2]);
            InsertEdge(face.v[2], base + face.e[2]);
        }

        void PNPZ_61(size_t base, Face const& face)
        {
            InsertEdge(face.v[3], face.v[0]);
            InsertEdge(face.v[0], base + face.e[0]);
            InsertEdge(base + face.e[1], face.v[2]);
            InsertEdge(face.v[2], face.v[3]);
        }

        void PNPP_62(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[0], base + face.e[1]);
        }

        void PZNN_63(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], face.v[1]);
        }

        void PZNZ_64(size_t, Face const& face)
        {
            InsertEdge(face.v[3], face.v[1]);
        }

        void PZNP_65(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[2], face.v[1]);
        }

        void PZZN_66(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], face.v[0]);
            InsertEdge(face.v[0], face.v[1]);
        }

        void PZZZ_67(size_t, Face const&)
        {
        }

        void PZZP_68(size_t, Face const&)
        {
        }

        void PZPN_69(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], face.v[0]);
            InsertEdge(face.v[0], face.v[1]);
            InsertEdge(face.v[1], face.v[2]);
            InsertEdge(face.v[2], base + face.e[2]);
        }

        void PZPZ_70(size_t, Face const&)
        {
        }

        void PZPP_71(size_t, Face const&)
        {
        }

        void PPNN_72(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], base + face.e[1]);
        }

        void PPNZ_73(size_t base, Face const& face)
        {
            InsertEdge(face.v[3], base + face.e[1]);
        }

        void PPNP_74(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[2], base + face.e[1]);
        }

        void PPZN_75(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[2], face.v[2]);
        }

        void PPZZ_76(size_t, Face const&)
        {
        }

        void PPZP_77(size_t, Face const&)
        {
        }

        void PPPN_78(size_t base, Face const& face)
        {
            InsertEdge(base + face.e[3], base + face.e[2]);
        }

        void PPPZ_79(size_t, Face const&)
        {
        }

        void PPPP_80(size_t, Face const&)
        {
        }
    };

    // Template alias for convenience.
    template <typename Real>
    using TIAlignedBox3Cone3 = TIQuery<Real, AlignedBox<3, Real>, Cone<3, Real>>;
}
