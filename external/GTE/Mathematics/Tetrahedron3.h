// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Hyperplane.h>
#include <Mathematics/Vector3.h>

// The tetrahedron is represented as an array of four vertices: V0, V1, V2,
// and V3.  The vertices are ordered so that the triangular faces are
// counterclockwise-ordered triangles when viewed by an observer outside the
// tetrahedron:
//   face 0 = <V[0],V[2],V[1]>
//   face 1 = <V[0],V[1],V[3]>
//   face 2 = <V[0],V[3],V[2]>
//   face 3 = <V[1],V[2],V[3]>

namespace gte
{
    template <typename Real>
    class Tetrahedron3
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // vertices to (0,0,0), (1,0,0), (0,1,0), and (0,0,1).
        Tetrahedron3()
            :
            v{ Vector3<Real>::Zero(), Vector3<Real>::Unit(0),
                Vector3<Real>::Unit(1), Vector3<Real>::Unit(2) }
        {
        }

        Tetrahedron3(Vector3<Real> const& v0, Vector3<Real> const& v1,
            Vector3<Real> const& v2, Vector3<Real> const& v3)
            :
            v{ v0, v1, v2, v3 }
        {
        }

        Tetrahedron3(std::array<Vector3<Real>, 4> const& inV)
            :
            v(inV)
        {
        }


        // Get the vertex indices for the specified face.  The input 'face'
        // must be in {0,1,2,3}.
        void GetFaceIndices(int face, int index[3]) const
        {
            if (face == 0)
            {
                index[0] = 0;
                index[1] = 2;
                index[2] = 1;
            }
            else if (face == 1)
            {
                index[0] = 0;
                index[1] = 1;
                index[2] = 3;
            }
            else if (face == 2)
            {
                index[0] = 0;
                index[1] = 3;
                index[2] = 2;
            }
            else  // face == 3 (no index validation is performed)
            {
                index[0] = 1;
                index[1] = 2;
                index[2] = 3;
            }
        }

        // Construct the planes of the faces.  The planes have outer pointing
        // normal vectors.  The plane indexing is the same as the face
        // indexing mentioned previously.
        void GetPlanes(Plane3<Real> plane[4]) const
        {
            Vector3<Real> edge10 = v[1] - v[0];
            Vector3<Real> edge20 = v[2] - v[0];
            Vector3<Real> edge30 = v[3] - v[0];
            Vector3<Real> edge21 = v[2] - v[1];
            Vector3<Real> edge31 = v[3] - v[1];

            plane[0].normal = UnitCross(edge20, edge10);  // <v0,v2,v1>
            plane[1].normal = UnitCross(edge10, edge30);  // <v0,v1,v3>
            plane[2].normal = UnitCross(edge30, edge20);  // <v0,v3,v2>
            plane[3].normal = UnitCross(edge21, edge31);  // <v1,v2,v3>

            Real det = Dot(edge10, plane[3].normal);
            if (det < (Real)0)
            {
                // The normals are inner pointing, reverse their directions.
                for (int i = 0; i < 4; ++i)
                {
                    plane[i].normal = -plane[i].normal;
                }
            }

            for (int i = 0; i < 4; ++i)
            {
                plane[i].constant = Dot(v[i], plane[i].normal);
            }
        }

        // Public member access.
        std::array<Vector3<Real>, 4> v;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Tetrahedron3 const& tetrahedron) const
        {
            return v == tetrahedron.v;
        }

        bool operator!=(Tetrahedron3 const& tetrahedron) const
        {
            return v != tetrahedron.v;
        }

        bool operator< (Tetrahedron3 const& tetrahedron) const
        {
            return v < tetrahedron.v;
        }

        bool operator<=(Tetrahedron3 const& tetrahedron) const
        {
            return v <= tetrahedron.v;
        }

        bool operator> (Tetrahedron3 const& tetrahedron) const
        {
            return v > tetrahedron.v;
        }

        bool operator>=(Tetrahedron3 const& tetrahedron) const
        {
            return v >= tetrahedron.v;
        }
    };
}
