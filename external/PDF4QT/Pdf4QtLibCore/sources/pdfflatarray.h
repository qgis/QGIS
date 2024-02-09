//    Copyright (C) 2019-2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.


#ifndef PDFFLATARRAY_H
#define PDFFLATARRAY_H

#include <QtGlobal>

#include <array>
#include <vector>
#include <algorithm>

namespace pdf
{

/// This represents a fast array, consisting of "fast" block of fixed size \p FlatSize,
/// and "slow" block of variable size. Usually, this array is used when vast majority
/// of usage size is below FlatSize, only minority is above FlatSize. Typical example
/// of use of this class:
///
/// We have colors in PDF, which can have usually 1, 3 or 4 color components. But in some
/// rare cases, we have much more components, for example for DeviceN color spaces.
/// For this reason, we will set FlatSize to 4 (so Gray, RGB and CMYK colors will not
/// use slow "variable" part).
template<typename T, size_t FlatSize>
class PDFFlatArray
{
public:
    PDFFlatArray() :
        m_flatBlock(),
        m_flatBlockItemCount(0),
        m_variableBlock()
    {

    }

    template<typename... Arguments, typename std::enable_if<sizeof...(Arguments) <= FlatSize, int>::type = 0>
    inline PDFFlatArray(Arguments... arguments) :
        m_flatBlock({ arguments... }),
        m_flatBlockItemCount(sizeof...(Arguments)),
        m_variableBlock()
    {

    }

    using value_type = T;

    /// Returns the size of the array
    size_t size() const { return getFlatBlockSize() + m_variableBlock.size(); }

    /// Returns true, if array is empty
    bool empty() const { return size() == 0; }

    template<size_t index>
    const T& get() const
    {
        if constexpr (index < FlatSize)
        {
            return m_flatBlock[size()];
        }
        else
        {
            return m_variableBlock[size() - FlatSize];
        }
    }

    template<size_t index>
    T& get()
    {
        if constexpr (index < FlatSize)
        {
            return m_flatBlock[size()];
        }
        else
        {
            return m_variableBlock[size() - FlatSize];
        }
    }

    const T& operator[] (size_t index) const
    {
        Q_ASSERT(index < size());

        if (index < FlatSize)
        {
            return m_flatBlock[index];
        }
        else
        {
            return m_variableBlock[index - FlatSize];
        }
    }

   T& operator[] (size_t index)
   {
       Q_ASSERT(index < size());

       if (index < FlatSize)
       {
           return m_flatBlock[index];
       }
       else
       {
           return m_variableBlock[index - FlatSize];
       }
   }

    void clear()
    {
        m_flatBlockItemCount = 0;
        m_variableBlock.clear();
    }

    void push_back(T object)
    {
        if (m_flatBlockItemCount < m_flatBlock.size())
        {
            m_flatBlock[m_flatBlockItemCount++] = std::move(object);
        }
        else
        {
            m_variableBlock.emplace_back(std::move(object));
        }
    }

    void resize(std::size_t size)
    {
        if (size <= FlatSize)
        {
            m_flatBlockItemCount = size;
            m_variableBlock.clear();
        }
        else
        {
            m_flatBlockItemCount = FlatSize;
            m_variableBlock.resize(size - FlatSize);
        }
    }

    /// Returns the last element of the array
    inline const T& back() const { return m_variableBlock.empty() ? m_flatBlock[m_flatBlockItemCount - 1] : m_variableBlock.back(); }

    /// Erases the last element from the array
    inline void pop_back() { resize(size() - 1); }

    bool operator==(const PDFFlatArray& other) const
    {
        const size_t size = this->size();
        if (size != other.size())
        {
            return false;
        }

        for (size_t i = 0; i < size; ++i)
        {
            if ((*this)[i] != other[i])
            {
                return false;
            }
        }

        return true;
    }

private:
    size_t getFlatBlockSize() const { return m_flatBlockItemCount; }

    std::array<T, FlatSize> m_flatBlock;
    size_t m_flatBlockItemCount; ///< Number of items in the flat block
    std::vector<T> m_variableBlock;
};

}   // namespace pdf

#endif // PDFFLATARRAY_H
