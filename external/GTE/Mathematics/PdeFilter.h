// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

namespace gte
{
    template <typename Real>
    class PdeFilter
    {
    public:
        enum ScaleType
        {
            // The data is processed as is.
            ST_NONE,

            // The data range is d in [min,max].  The scaled values are d'.

            // d' = (d-min)/(max-min) in [0,1]
            ST_UNIT,

            // d' = -1 + 2*(d-min)/(max-min) in [-1,1]
            ST_SYMMETRIC,

            // max > -min:  d' = d/max in [min/max,1]
            // max < -min:  d' = -d/min in [-1,-max/min]
            ST_PRESERVE_ZERO
        };

        // The abstract base class for all PDE-based filters.
        virtual ~PdeFilter()
        {
        }

        // Member access.
        inline int GetQuantity() const
        {
            return mQuantity;
        }

        inline Real GetBorderValue() const
        {
            return mBorderValue;
        }

        inline ScaleType GetScaleType() const
        {
            return mScaleType;
        }

        // Access to the time step for the PDE solver.
        inline void SetTimeStep(Real timeStep)
        {
            mTimeStep = timeStep;
        }

        inline Real GetTimeStep() const
        {
            return mTimeStep;
        }

        // This function executes one iteration of the filter.  It calls
        // OnPreUpdate, OnUpdate and OnPostUpdate, in that order.
        void Update()
        {
            OnPreUpdate();
            OnUpdate();
            OnPostUpdate();
        }

    protected:
        PdeFilter(int quantity, Real const* data, Real borderValue, ScaleType scaleType)
            :
            mQuantity(quantity),
            mBorderValue(borderValue),
            mScaleType(scaleType),
            mTimeStep(0)
        {
            Real maxValue = data[0];
            mMin = maxValue;
            for (int i = 1; i < mQuantity; i++)
            {
                Real value = data[i];
                if (value < mMin)
                {
                    mMin = value;
                }
                else if (value > maxValue)
                {
                    maxValue = value;
                }
            }

            if (mMin != maxValue)
            {
                switch (mScaleType)
                {
                case ST_NONE:
                    mOffset = (Real)0;
                    mScale = (Real)1;
                    break;
                case ST_UNIT:
                    mOffset = (Real)0;
                    mScale = (Real)1 / (maxValue - mMin);
                    break;
                case ST_SYMMETRIC:
                    mOffset = (Real)-1;
                    mScale = (Real)2 / (maxValue - mMin);
                    break;
                case ST_PRESERVE_ZERO:
                    mOffset = (Real)0;
                    mScale = (maxValue >= -mMin ? (Real)1 / maxValue : (Real)-1 / mMin);
                    mMin = (Real)0;
                    break;
                }
            }
            else
            {
                mOffset = (Real)0;
                mScale = (Real)1;
            }
        }

        // The derived classes for 2D and 3D implement this to recompute the
        // boundary values when Neumann conditions are used.  If derived
        // classes built on top of the 2D or 3D classes implement this also,
        // they must call the base-class OnPreUpdate first.
        virtual void OnPreUpdate() = 0;

        // The derived classes for 2D and 3D implement this to iterate over
        // the image elements, updating an element only if it is not masked
        // out.
        virtual void OnUpdate() = 0;

        // The derived classes for 2D and 3D implement this to swap the
        // buffers for the next pass.  If derived classes built on top of the
        // 2D or 3D classes implement this also, they must call the base-class
        // OnPostUpdate last. 
        virtual void OnPostUpdate() = 0;

        // The number of image elements.
        int mQuantity;

        // When set to std::numeric_limits<Real>::max(), Neumann conditions
        // are in use (zero-valued derivatives on the image border).
        // Dirichlet conditions are used, otherwise (image is constant on the
        // border).
        Real mBorderValue;

        // This member stores how the image data was transformed during the
        // constructor call.
        ScaleType mScaleType;
        Real mMin, mOffset, mScale;

        // The time step for the PDE solver.  The stability of an algorithm
        // depends on the magnitude of the time step, but the magnitude itself
        // depends on the algorithm.
        Real mTimeStep;
    };
}
