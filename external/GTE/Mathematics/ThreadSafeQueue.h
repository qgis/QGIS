// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <mutex>
#include <queue>

namespace gte
{
    template <typename Element>
    class ThreadSafeQueue
    {
    public:
        // Construction and destruction.
        ThreadSafeQueue(size_t maxNumElements = 0)
            :
            mMaxNumElements(maxNumElements)
        {
        }

        virtual ~ThreadSafeQueue() = default;

        // All the operations are thread-safe.
        size_t GetMaxNumElements() const
        {
            size_t maxNumElements;
            mMutex.lock();
            {
                maxNumElements = mMaxNumElements;
            }
            mMutex.unlock();
            return maxNumElements;
        }

        size_t GetNumElements() const
        {
            size_t numElements;
            mMutex.lock();
            {
                numElements = mQueue.size();
            }
            mMutex.unlock();
            return numElements;
        }

        bool Push(Element const& element)
        {
            bool pushed;
            mMutex.lock();
            {
                if (mQueue.size() < mMaxNumElements)
                {
                    mQueue.push(element);
                    pushed = true;
                }
                else
                {
                    pushed = false;
                }
            }
            mMutex.unlock();
            return pushed;
        }

        bool Pop(Element& element)
        {
            bool popped;
            mMutex.lock();
            {
                if (mQueue.size() > 0)
                {
                    element = mQueue.front();
                    mQueue.pop();
                    popped = true;
                }
                else
                {
                    popped = false;
                }
            }
            mMutex.unlock();
            return popped;
        }

    protected:
        size_t mMaxNumElements;
        std::queue<Element> mQueue;
        mutable std::mutex mMutex;
    };
}
