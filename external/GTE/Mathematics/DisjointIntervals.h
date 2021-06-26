// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <vector>

namespace gte
{
    // Compute Boolean operations of disjoint sets of half-open intervals of
    // the form [xmin,xmax) with xmin < xmax.
    template <typename Scalar>
    class DisjointIntervals
    {
    public:
        // Construction and destruction.  The non-default constructor requires
        // that xmin < xmax.
        DisjointIntervals()
        {
        }

        DisjointIntervals(Scalar const& xmin, Scalar const& xmax)
        {
            if (xmin < xmax)
            {
                mEndpoints = { xmin, xmax };
                mEndpoints[0] = xmin;
                mEndpoints[1] = xmax;
            }
        }

        ~DisjointIntervals()
        {
        }

        // Copy operations.
        DisjointIntervals(DisjointIntervals const& other)
            :
            mEndpoints(other.mEndpoints)
        {
        }

        DisjointIntervals& operator=(DisjointIntervals const& other)
        {
            mEndpoints = other.mEndpoints;
            return *this;
        }

        // Move operations.
        DisjointIntervals(DisjointIntervals&& other)
            :
            mEndpoints(std::move(other.mEndpoints))
        {
        }

        DisjointIntervals& operator=(DisjointIntervals&& other)
        {
            mEndpoints = std::move(other.mEndpoints);
            return *this;
        }

        // The number of intervals in the set.
        inline int GetNumIntervals() const
        {
            return static_cast<int>(mEndpoints.size() / 2);
        }

        // The i-th interval is [xmin,xmax).  The values xmin and xmax are
        // valid only when 0 <= i < GetNumIntervals().
        bool GetInterval(int i, Scalar& xmin, Scalar& xmax) const
        {
            int index = 2 * i;
            if (0 <= index && index < static_cast<int>(mEndpoints.size()))
            {
                xmin = mEndpoints[index];
                xmax = mEndpoints[++index];
                return true;
            }
            xmin = (Scalar)0;
            xmax = (Scalar)0;
            return false;
        }

        // Make this set empty.
        inline void Clear()
        {
            mEndpoints.clear();
        }

        // Insert [xmin,xmax) into the set.  This is a Boolean 'union'
        // operation.  The operation is successful only when xmin < xmax.
        bool Insert(Scalar const& xmin, Scalar const& xmax)
        {
            if (xmin < xmax)
            {
                DisjointIntervals input(xmin, xmax);
                DisjointIntervals output = *this | input;
                mEndpoints = std::move(output.mEndpoints);
                return true;
            }
            return false;
        }

        // Remove [xmin,xmax) from the set.  This is a Boolean 'difference'
        // operation.  The operation is successful only when xmin < xmax.
        bool Remove(Scalar const& xmin, Scalar const& xmax)
        {
            if (xmin < xmax)
            {
                DisjointIntervals input(xmin, xmax);
                DisjointIntervals output = std::move(*this - input);
                mEndpoints = std::move(output.mEndpoints);
                return true;
            }
            return false;
        }

        // Get the union of the interval sets, input0 union input1.
        friend DisjointIntervals operator|(DisjointIntervals const& input0, DisjointIntervals const& input1)
        {
            DisjointIntervals output;

            size_t const numEndpoints0 = input0.mEndpoints.size();
            size_t const numEndpoints1 = input1.mEndpoints.size();
            size_t i0 = 0, i1 = 0;
            int parity0 = 0, parity1 = 0;
            while (i0 < numEndpoints0 && i1 < numEndpoints1)
            {
                Scalar const& value0 = input0.mEndpoints[i0];
                Scalar const& value1 = input1.mEndpoints[i1];

                if (value0 < value1)
                {
                    if (parity0 == 0)
                    {
                        parity0 = 1;
                        if (parity1 == 0)
                        {
                            output.mEndpoints.push_back(value0);
                        }
                    }
                    else
                    {
                        if (parity1 == 0)
                        {
                            output.mEndpoints.push_back(value0);
                        }
                        parity0 = 0;
                    }
                    ++i0;
                }
                else if (value1 < value0)
                {
                    if (parity1 == 0)
                    {
                        parity1 = 1;
                        if (parity0 == 0)
                        {
                            output.mEndpoints.push_back(value1);
                        }
                    }
                    else
                    {
                        if (parity0 == 0)
                        {
                            output.mEndpoints.push_back(value1);
                        }
                        parity1 = 0;
                    }
                    ++i1;
                }
                else  // value0 == value1
                {
                    if (parity0 == parity1)
                    {
                        output.mEndpoints.push_back(value0);
                    }
                    parity0 ^= 1;
                    parity1 ^= 1;
                    ++i0;
                    ++i1;
                }
            }

            while (i0 < numEndpoints0)
            {
                output.mEndpoints.push_back(input0.mEndpoints[i0]);
                ++i0;
            }

            while (i1 < numEndpoints1)
            {
                output.mEndpoints.push_back(input1.mEndpoints[i1]);
                ++i1;
            }

            return output;
        }

        // Get the intersection of the interval sets, input0 intersect is1.
        friend DisjointIntervals operator&(DisjointIntervals const& input0, DisjointIntervals const& input1)
        {
            DisjointIntervals output;

            size_t const numEndpoints0 = input0.mEndpoints.size();
            size_t const numEndpoints1 = input1.mEndpoints.size();
            size_t i0 = 0, i1 = 0;
            int parity0 = 0, parity1 = 0;
            while (i0 < numEndpoints0 && i1 < numEndpoints1)
            {
                Scalar const& value0 = input0.mEndpoints[i0];
                Scalar const& value1 = input1.mEndpoints[i1];

                if (value0 < value1)
                {
                    if (parity0 == 0)
                    {
                        parity0 = 1;
                        if (parity1 == 1)
                        {
                            output.mEndpoints.push_back(value0);
                        }
                    }
                    else
                    {
                        if (parity1 == 1)
                        {
                            output.mEndpoints.push_back(value0);
                        }
                        parity0 = 0;
                    }
                    ++i0;
                }
                else if (value1 < value0)
                {
                    if (parity1 == 0)
                    {
                        parity1 = 1;
                        if (parity0 == 1)
                        {
                            output.mEndpoints.push_back(value1);
                        }
                    }
                    else
                    {
                        if (parity0 == 1)
                        {
                            output.mEndpoints.push_back(value1);
                        }
                        parity1 = 0;
                    }
                    ++i1;
                }
                else  // value0 == value1
                {
                    if (parity0 == parity1)
                    {
                        output.mEndpoints.push_back(value0);
                    }
                    parity0 ^= 1;
                    parity1 ^= 1;
                    ++i0;
                    ++i1;
                }
            }

            return output;
        }

        // Get the differences of the interval sets, input0 minus input1.
        friend DisjointIntervals operator-(DisjointIntervals const& input0, DisjointIntervals const& input1)
        {
            DisjointIntervals output;

            size_t const numEndpoints0 = input0.mEndpoints.size();
            size_t const numEndpoints1 = input1.mEndpoints.size();
            size_t i0 = 0, i1 = 0;
            int parity0 = 0, parity1 = 1;
            while (i0 < numEndpoints0 && i1 < numEndpoints1)
            {
                Scalar const& value0 = input0.mEndpoints[i0];
                Scalar const& value1 = input1.mEndpoints[i1];

                if (value0 < value1)
                {
                    if (parity0 == 0)
                    {
                        parity0 = 1;
                        if (parity1 == 1)
                        {
                            output.mEndpoints.push_back(value0);
                        }
                    }
                    else
                    {
                        if (parity1 == 1)
                        {
                            output.mEndpoints.push_back(value0);
                        }
                        parity0 = 0;
                    }
                    ++i0;
                }
                else if (value1 < value0)
                {
                    if (parity1 == 0)
                    {
                        parity1 = 1;
                        if (parity0 == 1)
                        {
                            output.mEndpoints.push_back(value1);
                        }
                    }
                    else
                    {
                        if (parity0 == 1)
                        {
                            output.mEndpoints.push_back(value1);
                        }
                        parity1 = 0;
                    }
                    ++i1;
                }
                else  // value0 == value1
                {
                    if (parity0 == parity1)
                    {
                        output.mEndpoints.push_back(value0);
                    }
                    parity0 ^= 1;
                    parity1 ^= 1;
                    ++i0;
                    ++i1;
                }
            }

            while (i0 < numEndpoints0)
            {
                output.mEndpoints.push_back(input0.mEndpoints[i0]);
                ++i0;
            }

            return output;
        }

        // Get the exclusive or of the interval sets, input0 xor input1 =
        // (input0 minus input1) or (input1 minus input0).
        friend DisjointIntervals operator^(DisjointIntervals const& input0, DisjointIntervals const& input1)
        {
            DisjointIntervals output;

            size_t const numEndpoints0 = input0.mEndpoints.size();
            size_t const numEndpoints1 = input1.mEndpoints.size();
            size_t i0 = 0, i1 = 0;
            while (i0 < numEndpoints0 && i1 < numEndpoints1)
            {
                Scalar const& value0 = input0.mEndpoints[i0];
                Scalar const& value1 = input1.mEndpoints[i1];

                if (value0 < value1)
                {
                    output.mEndpoints.push_back(value0);
                    ++i0;
                }
                else if (value1 < value0)
                {
                    output.mEndpoints.push_back(value1);
                    ++i1;
                }
                else  // value0 == value1
                {
                    ++i0;
                    ++i1;
                }
            }

            while (i0 < numEndpoints0)
            {
                output.mEndpoints.push_back(input0.mEndpoints[i0]);
                ++i0;
            }

            while (i1 < numEndpoints1)
            {
                output.mEndpoints.push_back(input1.mEndpoints[i1]);
                ++i1;
            }

            return output;
        }

    private:
        // The array of endpoints has an even number of elements.  The i-th
        // interval is [mEndPoints[2*i],mEndPoints[2*i+1]).
        std::vector<Scalar> mEndpoints;
    };
}
