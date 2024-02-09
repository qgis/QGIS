//    Copyright (C) 2018-2021 Jakub Melka
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


#ifndef PDFFLATMAP_H
#define PDFFLATMAP_H

#include <set>
#include <array>
#include <algorithm>

namespace pdf
{

/// This class behaves like std::set, but have "flat" part, and if size of the set
/// is small (smaller than \p FlatSize), then no memory allocation is needed. This
/// container supports inserting, deleting and searching for the object presence.
template<typename Key, int FlatSize>
class PDFFlatMap
{
public:
    constexpr inline PDFFlatMap();

    /// Inserts a key in the container. Checks, if key is already present
    /// in the container, in this case no insertion occurs.
    /// \param key Key to be inserted
    void insert(const Key& key);

    /// Erases a key in the container, if it is in the set
    /// \param key Key to be erased
    void erase(const Key& key);

    /// Searchs for a given key. If it is found, true is returned, false otherwise.
    /// \param key Key to be searched
    bool search(const Key& key) const;

    /// Returns size of  the container
    std::size_t size() const;

    /// Returns true, if container is empty
    bool empty() const;

private:
    /// Flat part of the set
    std::array<Key, FlatSize> m_flat;

    /// This iterator points to first empty position, or it is
    /// the last iterator (pointing to the end of the array).
    typename std::array<Key, FlatSize>::iterator m_flatEmptyPosition;

    std::set<Key> m_overflowContainer;
};

template<typename Key, int FlatSize>
constexpr PDFFlatMap<Key, FlatSize>::PDFFlatMap() :
    m_flat(),
    m_flatEmptyPosition(m_flat.begin()),
    m_overflowContainer()
{

}

template<typename Key, int FlatSize>
void PDFFlatMap<Key, FlatSize>::insert(const Key& key)
{
    if (!search(key))
    {
        // Try to insert key in the flat part, if possible (we are not at end of the array)
        if (m_flatEmptyPosition != m_flat.end())
        {
            *m_flatEmptyPosition++ = key;
        }
        else
        {
            m_overflowContainer.insert(key);
        }
    }
}

template<typename Key, int FlatSize>
void PDFFlatMap<Key, FlatSize>::erase(const Key& key)
{
    // First we must check, if the key is present in the flat part. If yes, then remove key
    // from the flat part and try to move one item from the overflow part to the flat part, if possible.
    // Otherwise check overflow part.
    m_flatEmptyPosition = std::remove_if(m_flat.begin(), m_flatEmptyPosition, [&key](const Key& otherKey) { return key == otherKey; });
    m_overflowContainer.erase(key);

    if (!m_overflowContainer.empty() && m_flatEmptyPosition != m_flat.end())
    {
        *m_flatEmptyPosition++ = *m_overflowContainer.begin();
        m_overflowContainer.erase(m_overflowContainer.begin());
    }
}

template<typename Key, int FlatSize>
bool PDFFlatMap<Key, FlatSize>::search(const Key& key) const
{
    return std::find<typename std::array<Key, FlatSize>::const_iterator, Key>(m_flat.begin(), m_flatEmptyPosition, key) != m_flatEmptyPosition || static_cast<bool>(m_overflowContainer.count(key));
}

template<typename Key, int FlatSize>
std::size_t PDFFlatMap<Key, FlatSize>::size() const
{
    return std::distance<typename std::array<Key, FlatSize>::const_iterator>(m_flat.begin(), m_flatEmptyPosition) + m_overflowContainer.size();
}

template<typename Key, int FlatSize>
bool PDFFlatMap<Key, FlatSize>::empty() const
{
    return size() == 0;
}

}   // namespace pdf

#endif // PDFFLATMAP_H
