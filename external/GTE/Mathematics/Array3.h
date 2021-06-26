// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <cstddef>
#include <vector>

// The Array3 class represents a 3-dimensional array that minimizes the number
// of new and delete calls.  The T objects are stored in a contiguous array.

namespace gte
{
    template <typename T>
    class Array3
    {
    public:
        // Construction.  The first constructor generates an array of objects
        // that are owned by Array3.  The second constructor is given an array
        // of objects that are owned by the caller.  The array has bound0
        // columns, bound1 rows, and bound2 slices.
        Array3(size_t bound0, size_t bound1, size_t bound2)
            :
            mBound0(bound0),
            mBound1(bound1),
            mBound2(bound2),
            mObjects(bound0 * bound1 * bound2),
            mIndirect1(bound1 * bound2),
            mIndirect2(bound2)
        {
            SetPointers(mObjects.data());
        }

        Array3(size_t bound0, size_t bound1, size_t bound2, T* objects)
            :
            mBound0(bound0),
            mBound1(bound1),
            mBound2(bound2),
            mIndirect1(bound1 * bound2),
            mIndirect2(bound2)
        {
            SetPointers(objects);
        }

        // Support for dynamic resizing, copying, or moving.  If 'other' does
        // not own the original 'objects', they are not copied by the
        // assignment operator.
        Array3()
            :
            mBound0(0),
            mBound1(0),
            mBound2(0)
        {
        }

        Array3(Array3 const& other)
            :
            mBound0(other.mBound0),
            mBound1(other.mBound1),
            mBound2(other.mBound2)
        {
            *this = other;
        }

        Array3& operator=(Array3 const& other)
        {
            // The copy is valid whether or not other.mObjects has elements.
            mObjects = other.mObjects;
            SetPointers(other);
            return *this;
        }

        Array3(Array3&& other) noexcept
            :
            mBound0(other.mBound0),
            mBound1(other.mBound1),
            mBound2(other.mBound2)
        {
            *this = std::move(other);
        }

        Array3& operator=(Array3&& other) noexcept
        {
            // The move is valid whether or not other.mObjects has elements.
            mObjects = std::move(other.mObjects);
            SetPointers(other);
            return *this;
        }

        // Access to the array.  Sample usage is
        //   Array3<T> myArray(4, 3, 2);
        //   T** slice1 = myArray[1];
        //   T* slice1row2 = myArray[1][2];
        //   T slice1Row2Col3 = myArray[1][2][3];
        inline size_t GetBound0() const
        {
            return mBound0;
        }

        inline size_t GetBound1() const
        {
            return mBound1;
        }

        inline size_t GetBound2() const
        {
            return mBound2;
        }

        inline T* const* operator[](int slice) const
        {
            return mIndirect2[slice];
        }

        inline T** operator[](int slice)
        {
            return mIndirect2[slice];
        }

    private:
        void SetPointers(T* objects)
        {
            for (size_t i2 = 0; i2 < mBound2; ++i2)
            {
                size_t j1 = mBound1 * i2;  // = bound1*(i2 + j2) where j2 = 0
                mIndirect2[i2] = &mIndirect1[j1];
                for (size_t i1 = 0; i1 < mBound1; ++i1)
                {
                    size_t j0 = mBound0 * (i1 + j1);
                    mIndirect2[i2][i1] = &objects[j0];
                }
            }
        }

        void SetPointers(Array3 const& other)
        {
            mBound0 = other.mBound0;
            mBound1 = other.mBound1;
            mBound2 = other.mBound2;
            mIndirect1.resize(mBound1 * mBound2);
            mIndirect2.resize(mBound2);

            if (mBound0 > 0)
            {
                // The objects are owned.
                SetPointers(mObjects.data());
            }
            else if (mIndirect1.size() > 0)
            {
                // The objects are not owned.
                SetPointers(other.mIndirect2[0][0]);
            }
            // else 'other' is an empty Array3.
        }

        size_t mBound0, mBound1, mBound2;
        std::vector<T> mObjects;
        std::vector<T*> mIndirect1;
        std::vector<T**> mIndirect2;
    };
}
