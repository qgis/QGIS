// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <iterator>
#include <type_traits>

// For information on range-based for-loops, see
// http://en.cppreference.com/w/cpp/language/range-for

namespace gte
{
    // The function gte::reverse supports reverse iteration in range-based
    // for-loops using the auto keyword.  For example,
    //
    //   std::vector<int> numbers(4);
    //   int i = 0;
    //   for (auto& number : numbers)
    //   {
    //       number = i++;
    //       std::cout << number << ' ';
    //   }
    //   // Output:  0 1 2 3
    //
    //   for (auto& number : gte::reverse(numbers))
    //   {
    //       std::cout << number << ' ';
    //   }
    //   // Output:  3 2 1 0

    template <typename Iterator>
    class ReversalObject
    {
    public:
        ReversalObject(Iterator begin, Iterator end)
            :
            mBegin(begin),
            mEnd(end)
        {
        }

        Iterator begin() const { return mBegin; }
        Iterator end() const { return mEnd; }

    private:
        Iterator mBegin, mEnd;
    };

    template
        <
        typename Iterable,
        typename Iterator = decltype(std::begin(std::declval<Iterable>())),
        typename ReverseIterator = std::reverse_iterator<Iterator>
        >
        ReversalObject<ReverseIterator> reverse(Iterable && range)
    {
        return ReversalObject<ReverseIterator>(
            ReverseIterator(std::end(range)),
            ReverseIterator(std::begin(range)));
    }
}
