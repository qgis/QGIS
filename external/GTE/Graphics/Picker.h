// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/PickRecord.h>
#include <Graphics/Node.h>
#include <Graphics/Visual.h>
#include <Mathematics/Line.h>

namespace gte
{
    class Picker
    {
    public:
        // Construction and destruction. Set the numThreads parameter to a
        // value larger than 1 for multithreaded picking of triangle
        // primitives.
        ~Picker() = default;
        Picker(unsigned int numThreads = 1);

        // Set the maximum distance when the 'scene' contains point or segment
        // primitives.  Such primitives are selected when they are within the
        // specified distance of the pick line.  The default is 0.0f.
        void SetMaxDistance(float maxDistance);
        float GetMaxDistance() const;

        // The linear component is parameterized by P + t*D, where P is a
        // point on the component (the origin) and D is a unit-length
        // direction vector.  Both P and D must be in world coordinates.
        // The interval [tmin,tmax] is
        //   line:     tmin = -fmax, tmax = fmax
        //   ray:      tmin = 0, tmax = fmax
        //   segment:  tmin = 0, tmax > 0;
        // where fmax is std::numeric_limits<float>::max().  A call to this
        // function will automatically clear the 'records' array.  If you need
        // any information from this array obtained by a previous call to
        // Execute, you must save it first.
        void operator()(std::shared_ptr<Spatial> const& scene,
            Vector4<float> const& origin, Vector4<float> const& direction,
            float tmin, float tmax);

        // The following three functions return the record satisfying the
        // constraints.  They should be called only when records.size() > 0.

        // Locate the record whose t-value is smallest in absolute value.
        PickRecord const& GetClosestToZero() const;

        // Locate the record with nonnegative t-value closest to zero.
        PickRecord const& GetClosestNonnegative() const;

        // Locate the record with nonpositive t-value closest to zero.
        PickRecord const& GetClosestNonpositive() const;

        // Access to all the records for the pick operation.
        std::vector<PickRecord> records;

    private:
        // The picking occurs recursively by traversing the input scene.
        void ExecuteRecursive(std::shared_ptr<Spatial> const& object);

        void PickTriangles(std::shared_ptr<Visual> const& visual, char const* positions,
            unsigned int vstride, IndexBuffer* ibuffer, Line3<float> const& line);

        void PickTriangles(std::shared_ptr<Visual> const& visual, char const* positions,
            unsigned int vstride, IndexBuffer* ibuffer, Line3<float> const& line,
            unsigned int i0, unsigned int i1, std::vector<PickRecord>& output) const;

        void PickSegments(std::shared_ptr<Visual> const& visual, char const* positions,
            unsigned int vstride, IndexBuffer* ibuffer, Line3<float> const& line);

        void PickPoints(std::shared_ptr<Visual> const& visual, char const* positions,
            unsigned int vstride, IndexBuffer* ibuffer, Line3<float> const& line);

        // The maximum number of threads that may be used to perform picking
        // requests for triangle primitives.
        unsigned int mNumThreads;

        // The maximum distance from the pick line used to select point or
        // segment primitives.
        float mMaxDistance;

        // The parameters for the linear component used to pick.
        Vector4<float> mOrigin;
        Vector4<float> mDirection;
        float mTMin, mTMax;

        // The value returned if the Get* functions are called when 'records'
        // has no elements.
        static PickRecord const msInvalid;
    };
}
