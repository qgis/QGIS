// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ParticleSystem.h>

namespace gte
{
    template <int N, typename Real>
    class MassSpringVolume : public ParticleSystem<N, Real>
    {
    public:
        // Construction and destruction.  This class represents an SxRxC array
        // of masses lying on in a volume and connected by an array of
        // springs.  The masses are indexed by mass[s][r][c] for 0 <= s < S,
        // 0 <= r < R and 0 <= c < C. The mass at interior position X[s][r][c]
        // is connected by springs to the masses at positions X[s][r-1][c],
        // X[s][r+1][c], X[s][r][c-1], X[s][r][c+1], X[s-1][r][c] and
        // X[s+1][r][c].  Boundary masses have springs connecting them to the
        // obvious neighbors ("face" mass has 5 neighbors, "edge" mass has 4
        // neighbors, "corner" mass has 3 neighbors).  The masses are arranged
        // in lexicographical order:  position[c+C*(r+R*s)] = X[s][r][c] for
        // 0 <= s < S, 0 <= r < R, and 0 <= c < C.  The other arrays are
        // stored similarly.
        virtual ~MassSpringVolume() = default;

        MassSpringVolume(int numSlices, int numRows, int numCols, Real step)
            :
            ParticleSystem<N, Real>(numSlices* numRows* numCols, step),
            mNumSlices(numSlices),
            mNumRows(numRows),
            mNumCols(numCols),
            mConstantS(numSlices* numRows* numCols),
            mLengthS(numSlices* numRows* numCols),
            mConstantR(numSlices* numRows* numCols),
            mLengthR(numSlices* numRows* numCols),
            mConstantC(numSlices* numRows* numCols),
            mLengthC(numSlices* numRows* numCols)
        {
            std::fill(mConstantS.begin(), mConstantS.end(), (Real)0);
            std::fill(mLengthS.begin(), mLengthS.end(), (Real)0);
            std::fill(mConstantR.begin(), mConstantR.end(), (Real)0);
            std::fill(mLengthR.begin(), mLengthR.end(), (Real)0);
            std::fill(mConstantC.begin(), mConstantC.end(), (Real)0);
            std::fill(mLengthC.begin(), mLengthC.end(), (Real)0);
        }

        // Member access.
        inline int GetNumSlices() const
        {
            return mNumSlices;
        }

        inline int GetNumRows() const
        {
            return mNumRows;
        }

        inline int GetNumCols() const
        {
            return mNumCols;
        }

        inline void SetMass(int s, int r, int c, Real mass)
        {
            ParticleSystem<N, Real>::SetMass(GetIndex(s, r, c), mass);
        }

        inline void SetPosition(int s, int r, int c, Vector<N, Real> const& position)
        {
            ParticleSystem<N, Real>::SetPosition(GetIndex(s, r, c), position);
        }

        inline void SetVelocity(int s, int r, int c, Vector<N, Real> const& velocity)
        {
            ParticleSystem<N, Real>::SetVelocity(GetIndex(s, r, c), velocity);
        }

        Real const& GetMass(int s, int r, int c) const
        {
            return ParticleSystem<N, Real>::GetMass(GetIndex(s, r, c));
        }

        inline Vector<N, Real> const& GetPosition(int s, int r, int c) const
        {
            return ParticleSystem<N, Real>::GetPosition(GetIndex(s, r, c));
        }

        inline Vector<N, Real> const& GetVelocity(int s, int r, int c) const
        {
            return ParticleSystem<N, Real>::GetVelocity(GetIndex(s, r, c));
        }

        // Each interior mass at (s,r,c) has 6 adjacent springs.  Face masses
        // have only 5 neighbors, edge masses have only 4 neighbors, and corner
        // masses have only 3 neighbors.  Each mass provides access to 3 adjacent
        // springs at (s,r,c+1), (s,r+1,c), and (s+1,r,c).  The face, edge, and
        // corner masses provide access to only an appropriate subset of these.
        // The caller is responsible for ensuring the validity of the (s,r,c)
        // inputs.

        // to (s+1,r,c)
        inline void SetConstantS(int s, int r, int c, Real constant)
        {
            mConstantS[GetIndex(s, r, c)] = constant;
        }

        // to (s+1,r,c)
        inline void SetLengthS(int s, int r, int c, Real length)
        {
            mLengthS[GetIndex(s, r, c)] = length;
        }

        // to (s,r+1,c)
        inline void SetConstantR(int s, int r, int c, Real constant)
        {
            mConstantR[GetIndex(s, r, c)] = constant;
        }

        // to (s,r+1,c)
        inline void SetLengthR(int s, int r, int c, Real length)
        {
            mLengthR[GetIndex(s, r, c)] = length;
        }

        // to (s,r,c+1)
        inline void SetConstantC(int s, int r, int c, Real constant)
        {
            mConstantC[GetIndex(s, r, c)] = constant;
        }

        // spring to (s,r,c+1)
        inline void SetLengthC(int s, int r, int c, Real length)
        {
            mLengthC[GetIndex(s, r, c)] = length;
        }

        inline Real const& GetConstantS(int s, int r, int c) const
        {
            return mConstantS[GetIndex(s, r, c)];
        }

        inline Real const& GetLengthS(int s, int r, int c) const
        {
            return mLengthS[GetIndex(s, r, c)];
        }

        inline Real const& GetConstantR(int s, int r, int c) const
        {
            return mConstantR[GetIndex(s, r, c)];
        }

        inline Real const& GetLengthR(int s, int r, int c) const
        {
            return mLengthR[GetIndex(s, r, c)];
        }

        inline Real const& GetConstantC(int s, int r, int c) const
        {
            return mConstantC[GetIndex(s, r, c)];
        }

        inline Real const& GetLengthC(int s, int r, int c) const
        {
            return mLengthC[GetIndex(s, r, c)];
        }

        // The default external force is zero.  Derive a class from this one
        // to provide nonzero external forces such as gravity, wind,
        // friction and so on.  This function is called by Acceleration(...)
        // to compute the impulse F/m generated by the external force F.
        virtual Vector<N, Real> ExternalAcceleration(int, Real,
            std::vector<Vector<N, Real>> const&,
            std::vector<Vector<N, Real>> const&)
        {
            return Vector<N, Real>::Zero();
        }

    protected:
        // Callback for acceleration (ODE solver uses x" = F/m) applied to
        // particle i.  The positions and velocities are not necessarily
        // mPosition and mVelocity, because the ODE solver evaluates the
        // impulse function at intermediate positions.
        virtual Vector<N, Real> Acceleration(int i, Real time,
            std::vector<Vector<N, Real>> const& position,
            std::vector<Vector<N, Real>> const& velocity)
        {
            // Compute spring forces on position X[i].  The positions are not
            // necessarily mPosition, because the RK4 solver in ParticleSystem
            // evaluates the acceleration function at intermediate positions.
            // The face, edge, and corner points of the volume of masses must
            // be handled separately, because each has fewer than eight
            // springs attached to it.

            Vector<N, Real> acceleration = ExternalAcceleration(i, time, position, velocity);
            Vector<N, Real> diff, force;
            Real ratio;

            int s, r, c, prev, next;
            GetCoordinates(i, s, r, c);

            if (s > 0)
            {
                prev = i - mNumRows * mNumCols;  // index to previous s-neighbor
                diff = position[prev] - position[i];
                ratio = GetLengthS(s - 1, r, c) / Length(diff);
                force = GetConstantS(s - 1, r, c) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            if (s < mNumSlices - 1)
            {
                next = i + mNumRows * mNumCols;  // index to next s-neighbor
                diff = position[next] - position[i];
                ratio = GetLengthS(s, r, c) / Length(diff);
                force = GetConstantS(s, r, c) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            if (r > 0)
            {
                prev = i - mNumCols;  // index to previous r-neighbor
                diff = position[prev] - position[i];
                ratio = GetLengthR(s, r - 1, c) / Length(diff);
                force = GetConstantR(s, r - 1, c) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            if (r < mNumRows - 1)
            {
                next = i + mNumCols;  // index to next r-neighbor
                diff = position[next] - position[i];
                ratio = GetLengthR(s, r, c) / Length(diff);
                force = GetConstantR(s, r, c) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            if (c > 0)
            {
                prev = i - 1;  // index to previous c-neighbor
                diff = position[prev] - position[i];
                ratio = GetLengthC(s, r, c - 1) / Length(diff);
                force = GetConstantC(s, r, c - 1) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            if (c < mNumCols - 1)
            {
                next = i + 1;  // index to next c-neighbor
                diff = position[next] - position[i];
                ratio = GetLengthC(s, r, c) / Length(diff);
                force = GetConstantC(s, r, c) * ((Real)1 - ratio) * diff;
                acceleration += this->mInvMass[i] * force;
            }

            return acceleration;
        }

        inline int GetIndex(int s, int r, int c) const
        {
            return c + mNumCols * (r + mNumRows * s);
        }

        void GetCoordinates(int i, int& s, int& r, int& c) const
        {
            c = i % mNumCols;
            i = (i - c) / mNumCols;
            r = i % mNumRows;
            s = i / mNumRows;
        }

        int mNumSlices, mNumRows, mNumCols;
        std::vector<Real> mConstantS, mLengthS;
        std::vector<Real> mConstantR, mLengthR;
        std::vector<Real> mConstantC, mLengthC;
    };
}
