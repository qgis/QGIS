// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <cstddef>
#include <vector>

// The Array2 class represents a 2-dimensional array that minimizes the number
// of new and delete calls.  The T objects are stored in a contiguous array.

namespace gte
{
    template <typename T>
    class Array2
    {
    public:
        // Construction.  The first constructor generates an array of objects
        // that are owned by Array2.  The second constructor is given an array
        // of objects that are owned by the caller.  The array has bound0
        // columns and bound1 rows.
        Array2(size_t bound0, size_t bound1)
            :
            mBound0(bound0),
            mBound1(bound1),
            mObjects(bound0 * bound1),
            mIndirect1(bound1)
        {
            SetPointers(mObjects.data());
        }

        Array2(size_t bound0, size_t bound1, T* objects)
            :
            mBound0(bound0),
            mBound1(bound1),
            mIndirect1(bound1)
        {
            SetPointers(objects);
        }

        // Support for dynamic resizing, copying, or moving.  If 'other' does
        // not own the original 'objects', they are not copied by the
        // assignment operator.
        Array2()
            :
            mBound0(0),
            mBound1(0)
        {
        }

        Array2(Array2 const& other)
            :
            mBound0(other.mBound0),
            mBound1(other.mBound1)
        {
            *this = other;
        }

        Array2& operator=(Array2 const& other)
        {
            // The copy is valid whether or not other.mObjects has elements.
            mObjects = other.mObjects;
            SetPointers(other);
            return *this;
        }

        Array2(Array2&& other) noexcept
            :
            mBound0(other.mBound0),
            mBound1(other.mBound1)
        {
            *this = std::move(other);
        }

        Array2& operator=(Array2&& other) noexcept
        {
            // The move is valid whether or not other.mObjects has elements.
            mObjects = std::move(other.mObjects);
            SetPointers(other);
            return *this;
        }

        // Access to the array.  Sample usage is
        //   Array2<T> myArray(3, 2);
        //   T* row1 = myArray[1];
        //   T row1Col2 = myArray[1][2];
        inline size_t GetBound0() const
        {
            return mBound0;
        }

        inline size_t GetBound1() const
        {
            return mBound1;
        }

        inline T const* operator[](int row) const
        {
            return mIndirect1[row];
        }

        inline T* operator[](int row)
        {
            return mIndirect1[row];
        }

    private:
        void SetPointers(T* objects)
        {
            for (size_t i1 = 0; i1 < mBound1; ++i1)
            {
                size_t j0 = mBound0 * i1;  // = bound0*(i1 + j1) where j1 = 0
                mIndirect1[i1] = &objects[j0];
            }
        }

        void SetPointers(Array2 const& other)
        {
            mBound0 = other.mBound0;
            mBound1 = other.mBound1;
            mIndirect1.resize(mBound1);

            if (mBound0 > 0)
            {
                // The objects are owned.
                SetPointers(mObjects.data());
            }
            else if (mIndirect1.size() > 0)
            {
                // The objects are not owned.
                SetPointers(other.mIndirect1[0]);
            }
            // else 'other' is an empty Array2.
        }

        size_t mBound0, mBound1;
        std::vector<T> mObjects;
        std::vector<T*> mIndirect1;
    };
}
