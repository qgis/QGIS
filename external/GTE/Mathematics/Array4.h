// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <cstddef>
#include <vector>

// The Array4 class represents a 4-dimensional array that minimizes the number
// of new and delete calls.  The T objects are stored in a contiguous array.

namespace gte
{
    template <typename T>
    class Array4
    {
    public:
        // Construction.  The first constructor generates an array of objects
        // that are owned by Array4.  The second constructor is given an array
        // of objects that are owned by the caller.  The array has bound0
        // columns, bound1 rows, bound2 slices, and bound3 cuboids.
        Array4(size_t bound0, size_t bound1, size_t bound2, size_t bound3)
            :
            mBound0(bound0),
            mBound1(bound1),
            mBound2(bound2),
            mBound3(bound3),
            mObjects(bound0 * bound1 * bound2 * bound3),
            mIndirect1(bound1 * bound2 * bound3),
            mIndirect2(bound2 * bound3),
            mIndirect3(bound3)
        {
            SetPointers(mObjects.data());
        }

        Array4(size_t bound0, size_t bound1, size_t bound2, size_t bound3, T* objects)
            :
            mBound0(bound0),
            mBound1(bound1),
            mBound2(bound2),
            mBound3(bound3),
            mIndirect1(bound1 * bound2 * bound3),
            mIndirect2(bound2 * bound3),
            mIndirect3(bound3)
        {
            SetPointers(objects);
        }

        // Support for dynamic resizing, copying, or moving.  If 'other' does
        // not own the original 'objects', they are not copied by the
        // assignment operator.
        Array4()
            :
            mBound0(0),
            mBound1(0),
            mBound2(0),
            mBound3(0)
        {
        }

        Array4(Array4 const& other)
            :
            mBound0(other.mBound0),
            mBound1(other.mBound1),
            mBound2(other.mBound2),
            mBound3(other.mBound3)
        {
            *this = other;
        }

        Array4& operator=(Array4 const& other)
        {
            // The copy is valid whether or not other.mObjects has elements.
            mObjects = other.mObjects;
            SetPointers(other);
            return *this;
        }

        Array4(Array4&& other) noexcept
            :
            mBound0(other.mBound0),
            mBound1(other.mBound1),
            mBound2(other.mBound2),
            mBound3(other.mBound3)
        {
            *this = std::move(other);
        }

        Array4& operator=(Array4&& other) noexcept
        {
            // The move is valid whether or not other.mObjects has elements.
            mObjects = std::move(other.mObjects);
            SetPointers(other);
            return *this;
        }

        // Access to the array.  Sample usage is
        //   Array4<T> myArray(5, 4, 3, 2);
        //   T*** cuboid1 = myArray[1];
        //   T** cuboid1Slice2 = myArray[1][2];
        //   T* cuboid1Slice2Row3 = myArray[1][2][3];
        //   T cuboid1Slice2Row3Col4 = myArray[1][2][3][4];
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

        inline size_t GetBound3() const
        {
            return mBound3;
        }

        inline T** const* operator[](int cuboid) const
        {
            return mIndirect3[cuboid];
        }

        inline T*** operator[](int cuboid)
        {
            return mIndirect3[cuboid];
        }

    private:
        void SetPointers(T* objects)
        {
            for (size_t i3 = 0; i3 < mBound3; ++i3)
            {
                size_t j2 = mBound2 * i3;  // = bound2*(i3 + j3) where j3 = 0
                mIndirect3[i3] = &mIndirect2[j2];
                for (size_t i2 = 0; i2 < mBound2; ++i2)
                {
                    size_t j1 = mBound1 * (i2 + j2);
                    mIndirect3[i3][i2] = &mIndirect1[j1];
                    for (size_t i1 = 0; i1 < mBound1; ++i1)
                    {
                        size_t j0 = mBound0 * (i1 + j1);
                        mIndirect3[i3][i2][i1] = &objects[j0];
                    }
                }
            }
        }

        void SetPointers(Array4 const& other)
        {
            mBound0 = other.mBound0;
            mBound1 = other.mBound1;
            mBound2 = other.mBound2;
            mBound3 = other.mBound3;
            mIndirect1.resize(mBound1 * mBound2 * mBound3);
            mIndirect2.resize(mBound2 * mBound3);
            mIndirect3.resize(mBound3);

            if (mBound0 > 0)
            {
                // The objects are owned.
                SetPointers(mObjects.data());
            }
            else if (mIndirect1.size() > 0)
            {
                // The objects are not owned.
                SetPointers(other.mIndirect3[0][0][0]);
            }
            // else 'other' is an empty Array3.
        }

        size_t mBound0, mBound1, mBound2, mBound3;
        std::vector<T> mObjects;
        std::vector<T*> mIndirect1;
        std::vector<T**> mIndirect2;
        std::vector<T***> mIndirect3;
    };
}
