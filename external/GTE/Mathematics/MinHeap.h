// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.03.26

#pragma once

#include <vector>

// A min-heap is a binary tree whose nodes have weights and with the
// constraint that the weight of a parent node is less than or equal to the
// weights of its children.  This data structure may be used as a priority
// queue.  If the std::priority_queue interface suffices for your needs, use
// that instead.  However, for some geometric algorithms, that interface is
// insufficient for optimal performance.  For example, if you have a polyline
// vertices that you want to decimate, each vertex's weight depends on its
// neighbors' locations.  If the minimum-weight vertex is removed from the
// min-heap, the neighboring vertex weights must be updated--something that
// is O(1) time when you store the vertices as a doubly linked list.  The
// neighbors are already in the min-heap, so modifying their weights without
// removing then from--and then reinserting into--the min-heap requires they
// must be moved to their proper places to restore the invariant of the
// min-heap.  With std::priority_queue, you have no direct access to the
// modified vertices, forcing you to search for those vertices, remove them,
// update their weights, and re-insert them.  The min-heap implementation here
// does support the update without removal and reinsertion.
//
// The ValueType represents the weight and it must support comparisons
// "<" and "<=".  Additional information can be stored in the min-heap for
// convenient access; this is stored as the KeyType.  In the (open) polyline
// decimation example, the KeyType is a structure that stores indices to
// a vertex and its neighbors.  The following code illustrates the creation
// and use of the min-heap.  The Weight() function is whatever you choose to
// guide which vertices are removed first from the polyline.
//
//    struct Vertex { int previous, current, next; };
//    int numVertices = <number of polyline vertices>;
//    std::vector<Vector<N, Real>> positions(numVertices);
//    <assign all positions[*]>;
//    MinHeap<Vertex, Real> minHeap(numVertices);
//    std::vector<MinHeap<Vertex, Real>::Record*> records(numVertices);
//    for (int i = 0; i < numVertices; ++i)
//    {
//        Vertex vertex;
//        vertex.previous = (i + numVertices - 1) % numVertices;
//        vertex.current = i;
//        vertex.next = (i + 1) % numVertices;
//        records[i] = minHeap.Insert(vertex, Weight(positions, vertex));
//    }
//
//    while (minHeap.GetNumElements() >= 2)
//    {
//        Vertex vertex;
//        Real weight;
//        minHeap.Remove(vertex, weight);
//        <consume the 'vertex' according to your application's needs>;
//
//        // Remove 'vertex' from the doubly linked list.
//        Vertex& vp = records[vertex.previous]->key;
//        Vertex& vc = records[vertex.current]->key;
//        Vertex& vn = records[vertex.next]->key;
//        vp.next = vc.next;
//        vn.previous = vc.previous;
//
//        // Update the neighbors' weights in the min-heap.
//        minHeap.Update(records[vertex.previous], Weight(positions, vp));
//        minHeap.Update(records[vertex.next], Weight(positions, vn));
//    }

namespace gte
{
    template <typename KeyType, typename ValueType>
    class MinHeap
    {
    public:
        struct Record
        {
            Record()
                :
                key{},
                value{},
                index(-1)
            {
            }

            KeyType key;
            ValueType value;
            int index;
        };

        // Construction.  The record 'value' members are uninitialized for
        // native types chosen for ValueType.  If ValueType is of class type,
        // then the default constructor is used to set the 'value' members.
        MinHeap(int maxElements = 0)
        {
            Reset(maxElements);
        }

        MinHeap(MinHeap const& minHeap)
        {
            *this = minHeap;
        }

        // Assignment.
        MinHeap& operator=(MinHeap const& minHeap)
        {
            mNumElements = minHeap.mNumElements;
            mRecords = minHeap.mRecords;
            mPointers.resize(minHeap.mPointers.size());
            for (auto& record : mRecords)
            {
                mPointers[record.index] = &record;
            }
            return *this;
        }

        // Clear the min-heap so that it has the specified max elements,
        // mNumElements is zero, and mPointers are set to the natural ordering
        // of mRecords.
        void Reset(int maxElements)
        {
            mNumElements = 0;
            if (maxElements > 0)
            {
                mRecords.resize(maxElements);
                mPointers.resize(maxElements);
                for (int i = 0; i < maxElements; ++i)
                {
                    mPointers[i] = &mRecords[i];
                    mPointers[i]->index = i;
                }
            }
            else
            {
                mRecords.clear();
                mPointers.clear();
            }
        }

        // Get the remaining number of elements in the min-heap.  This number
        // is in the range {0..maxElements}.
        inline int GetNumElements() const
        {
            return mNumElements;
        }

        // Get the root of the min-heap.  The return value is 'true' whenever
        // the min-heap is not empty.  This function reads the root but does
        // not remove the element from the min-heap.
        bool GetMinimum(KeyType& key, ValueType& value) const
        {
            if (mNumElements > 0)
            {
                key = mPointers[0]->key;
                value = mPointers[0]->value;
                return true;
            }
            else
            {
                return false;
            }
        }

        // Insert into the min-heap the 'value' that corresponds to the 'key'.
        // The return value is a pointer to the heap record that stores a copy
        // of 'value', and the pointer value is constant for the life of the
        // min-heap.  If you must update a member of the min-heap, say, as
        // illustrated in the polyline decimation example, pass the pointer to
        // Update:
        //    auto* valueRecord = minHeap.Insert(key, value);
        //    <do whatever>;
        //    minHeap.Update(valueRecord, newValue).
        Record* Insert(KeyType const& key, ValueType const& value)
        {
            // Return immediately when the heap is full.
            if (mNumElements == static_cast<int>(mRecords.size()))
            {
                return nullptr;
            }

            // Store the input information in the last heap record, which is
            // the last leaf in the tree.
            int child = mNumElements++;
            Record* record = mPointers[child];
            record->key = key;
            record->value = value;

            // Propagate the information toward the root of the tree until it
            // reaches its correct position, thus restoring the tree to a
            // valid heap.
            while (child > 0)
            {
                int parent = (child - 1) / 2;
                if (mPointers[parent]->value <= value)
                {
                    // The parent has a value smaller than or equal to the
                    // child's value, so we now have a valid heap.
                    break;
                }

                // The parent has a larger value than the child's value.  Swap
                // the parent and child:

                // Move the parent into the child's slot.
                mPointers[child] = mPointers[parent];
                mPointers[child]->index = child;

                // Move the child into the parent's slot.
                mPointers[parent] = record;
                mPointers[parent]->index = parent;

                child = parent;
            }

            return mPointers[child];
        }

        // Remove the root of the heap and return its 'key' and 'value
        // members.  The root contains the minimum value of all heap elements.
        // The return value is 'true' whenever the min-heap was not empty
        // before the Remove call.
        bool Remove(KeyType& key, ValueType& value)
        {
            // Return immediately when the heap is empty.
            if (mNumElements == 0)
            {
                return false;
            }

            // Get the information from the root of the heap.
            Record* root = mPointers[0];
            key = root->key;
            value = root->value;

            // Restore the tree to a heap.  Abstractly, record is the new root
            // of the heap.  It is moved down the tree via parent-child swaps
            // until it is in a location that restores the tree to a heap.
            int last = --mNumElements;
            Record* record = mPointers[last];
            int parent = 0, child = 1;
            while (child <= last)
            {
                if (child < last)
                {
                    // Select the child with smallest value to be the one that
                    // is swapped with the parent, if necessary.
                    int childP1 = child + 1;
                    if (mPointers[childP1]->value < mPointers[child]->value)
                    {
                        child = childP1;
                    }
                }

                if (record->value <= mPointers[child]->value)
                {
                    // The tree is now a heap.
                    break;
                }

                // Move the child into the parent's slot.
                mPointers[parent] = mPointers[child];
                mPointers[parent]->index = parent;

                parent = child;
                child = 2 * child + 1;
            }

            // The previous 'last' record was moved to the root and propagated
            // down the tree to its final resting place, restoring the tree to
            // a heap.  The slot mPointers[parent] is that resting place.
            mPointers[parent] = record;
            mPointers[parent]->index = parent;

            // The old root record must not be lost.  Attach it to the slot
            // that contained the old last record.
            mPointers[last] = root;
            mPointers[last]->index = last;
            return true;
        }

        // The value of a heap record must be modified through this function
        // call.  The side effect is that the heap is updated accordingly to
        // restore the data structure to a min-heap.  The input 'record'
        // should be a pointer returned by Insert(value); see the comments for
        // the Insert() function.
        void Update(Record* record, ValueType const& value)
        {
            // Return immediately on invalid record.
            if (!record)
            {
                return;
            }

            int parent, child, childP1, maxChild;

            if (record->value < value)
            {
                record->value = value;

                // The new value is larger than the old value.  Propagate it
                // toward the leaves.
                parent = record->index;
                child = 2 * parent + 1;
                while (child < mNumElements)
                {
                    // At least one child exists.  Locate the one of maximum
                    // value.
                    childP1 = child + 1;
                    if (childP1 < mNumElements)
                    {
                        // Two children exist.
                        if (mPointers[child]->value <= mPointers[childP1]->value)
                        {
                            maxChild = child;
                        }
                        else
                        {
                            maxChild = childP1;
                        }
                    }
                    else
                    {
                        // One child exists.
                        maxChild = child;
                    }

                    if (value <= mPointers[maxChild]->value)
                    {
                        // The new value is in the correct place to restore
                        // the tree to a heap.
                        break;
                    }

                    // The child has a larger value than the parent's value.
                    // Swap the parent and child:

                    // Move the child into the parent's slot.
                    mPointers[parent] = mPointers[maxChild];
                    mPointers[parent]->index = parent;

                    // Move the parent into the child's slot.
                    mPointers[maxChild] = record;
                    mPointers[maxChild]->index = maxChild;

                    parent = maxChild;
                    child = 2 * parent + 1;
                }
            }
            else if (value < record->value)
            {
                record->value = value;

                // The new weight is smaller than the old weight.  Propagate
                // it toward the root.
                child = record->index;
                while (child > 0)
                {
                    // A parent exists.
                    parent = (child - 1) / 2;

                    if (mPointers[parent]->value <= value)
                    {
                        // The new value is in the correct place to restore
                        // the tree to a heap.
                        break;
                    }

                    // The parent has a smaller value than the child's value.
                    // Swap the child and parent.

                    // Move the parent into the child's slot.
                    mPointers[child] = mPointers[parent];
                    mPointers[child]->index = child;

                    // Move the child into the parent's slot.
                    mPointers[parent] = record;
                    mPointers[parent]->index = parent;

                    child = parent;
                }
            }
        }

        // Support for debugging.  The functions test whether the data
        // structure is a valid min-heap.
        bool IsValid() const
        {
            for (int child = 0; child < mNumElements; ++child)
            {
                int parent = (child - 1) / 2;
                if (parent > 0)
                {
                    if (mPointers[child]->value < mPointers[parent]->value)
                    {
                        return false;
                    }

                    if (mPointers[parent]->index != parent)
                    {
                        return false;
                    }
                }
            }

            return true;
        }

    private:
        // A 2-level storage system is used.  The pointers have two roles.
        // Firstly, they are unique to each inserted value in order to support
        // the Update() capability of the min-heap.  Secondly, they avoid
        // potentially expensive copying of Record objects as sorting occurs
        // in the heap.
        int mNumElements;
        std::vector<Record> mRecords;
        std::vector<Record*> mPointers;
    };
}
