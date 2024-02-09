//    Copyright (C) 2021 Jakub Melka
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

#ifndef PDFALGORITHMLCS_H
#define PDFALGORITHMLCS_H

#include "pdfglobal.h"

namespace pdf
{

class PDFAlgorithmLongestCommonSubsequenceBase
{
public:

    enum SequenceItemFlag
    {
        None            = 0x0000,
        MovedLeft       = 0x0001,   ///< Item has been moved from this position (is present in a sequence no. 1)
        MovedRight      = 0x0002,   ///< Item has been moved to this position (is present in a sequence no. 2)
        Moved           = 0x0004,   ///< Index of item has been changed
        Added           = 0x0008,   ///< Item has been added to a sequence no. 2
        Removed         = 0x0010,   ///< Item has been removed from a sequence no. 1
        Replaced        = 0x0020,   ///< Item has been replaced (or sequence of items has been replaced)
    };
    Q_DECLARE_FLAGS(SequenceItemFlags, SequenceItemFlag)

    struct SequenceItem
    {
        size_t index1 = std::numeric_limits<size_t>::max();
        size_t index2 = std::numeric_limits<size_t>::max();
        SequenceItemFlags flags = None;

        bool isLeftValid() const { return index1 != std::numeric_limits<size_t>::max(); }
        bool isRightValid() const { return index2 != std::numeric_limits<size_t>::max(); }
        bool isLeft() const { return isLeftValid() && !isRightValid(); }
        bool isRight() const { return isRightValid() && !isLeftValid(); }
        bool isMatch() const { return isLeftValid() && isRightValid(); }
        bool isMovedLeft() const { return flags.testFlag(MovedLeft); }
        bool isMovedRight() const { return flags.testFlag(MovedRight); }
        bool isMoved() const { return flags.testFlag(Moved); }
        bool isAdded() const { return flags.testFlag(Added); }
        bool isRemoved() const { return flags.testFlag(Removed); }
        bool isReplaced() const { return flags.testFlag(Replaced); }
        bool isModified() const { return isAdded() || isRemoved() || isReplaced(); }

        void markMovedLeft() { flags.setFlag(MovedLeft); }
        void markMovedRight() { flags.setFlag(MovedRight); }
        void markMoved() { flags.setFlag(Moved); }
        void markAdded() { flags.setFlag(Added); }
        void markRemoved() { flags.setFlag(Removed); }
        void markReplaced() { flags.setFlag(Replaced); }
    };

    using Sequence = typename std::vector<SequenceItem>;
    using SequenceIterator = typename Sequence::iterator;
    using SequenceItemRange = typename std::pair<SequenceIterator, SequenceIterator>;
    using SequenceItemRanges = typename std::vector<SequenceItemRange>;

    /// Marks a sequence with set of flags representing added/removed/replaced/moved
    /// items. Moved items sequences must be sorted.
    /// \param sequence Sequence to be marked
    /// \param movedItemsLeft Sorted sequence of left indices, which have been moved
    /// \param movedItemsRight sorted sequence of right indices, which have been moved
    static void markSequence(Sequence& sequence,
                             const std::vector<size_t>& movedItemsLeft,
                             const std::vector<size_t>& movedItemsRight);

    /// Returns item ranges, which should be checked - for example,
    /// for text modification.
    /// \param sequence Sequence
    static SequenceItemRanges getModifiedRanges(Sequence& sequence);

    /// Collect flags from given item range
    /// \param range Range
    static SequenceItemFlags collectFlags(const SequenceItemRange& range);
};

/// Algorithm for computing longest common subsequence, on two sequences
/// of objects, which are implementing operator "==" (equal operator).
/// Constructor takes bidirectional iterators to the sequence. So, iterators
/// are requred to be bidirectional.
template<typename Iterator, typename Comparator>
class PDFAlgorithmLongestCommonSubsequence : public PDFAlgorithmLongestCommonSubsequenceBase
{
public:
    PDFAlgorithmLongestCommonSubsequence(Iterator it1,
                                         Iterator it1End,
                                         Iterator it2,
                                         Iterator it2End,
                                         Comparator comparator);


    void perform();

    const Sequence& getSequence() const { return m_sequence; }

private:
    Iterator m_it1;
    Iterator m_it1End;
    Iterator m_it2;
    Iterator m_it2End;

    size_t m_size1;
    size_t m_size2;
    size_t m_matrixSize;

    Comparator m_comparator;

    std::vector<bool> m_backtrackData;
    Sequence m_sequence;
};

template<typename Iterator, typename Comparator>
PDFAlgorithmLongestCommonSubsequence<Iterator, Comparator>::PDFAlgorithmLongestCommonSubsequence(Iterator it1,
                                                                                                 Iterator it1End,
                                                                                                 Iterator it2,
                                                                                                 Iterator it2End,
                                                                                                 Comparator comparator) :
    m_it1(std::move(it1)),
    m_it1End(std::move(it1End)),
    m_it2(std::move(it2)),
    m_it2End(std::move(it2End)),
    m_size1(0),
    m_size2(0),
    m_matrixSize(0),
    m_comparator(std::move(comparator))
{
    m_size1 = std::distance(m_it1, m_it1End) + 1;
    m_size2 = std::distance(m_it2, m_it2End) + 1;
    m_matrixSize = m_size1 * m_size2;
}

template<typename Iterator, typename Comparator>
void PDFAlgorithmLongestCommonSubsequence<Iterator, Comparator>::perform()
{
    m_backtrackData.resize(m_matrixSize);
    m_sequence.clear();

    std::vector<size_t> rowTop(m_size1, size_t());
    std::vector<size_t> rowBottom(m_size1, size_t());

    // Jakub Melka: we will have columns consisting of it1...it1End
    // and rows consisting of it2...it2End. We iterate trough rows,
    // and for each row, we update longest common subsequence data.

    auto it2 = m_it2;
    for (size_t i2 = 1; i2 < m_size2; ++i2, ++it2)
    {
        auto it1 = m_it1;
        for (size_t i1 = 1; i1 < m_size1; ++i1, ++it1)
        {
            if (m_comparator(*it1, *it2))
            {
                // We have match
                rowBottom[i1] = rowTop[i1 - 1] + 1;
            }
            else
            {
                const size_t leftCellValue = rowBottom[i1 - 1];
                const size_t upperCellValue = rowTop[i1];
                bool isLeftBigger = leftCellValue > upperCellValue;

                if (isLeftBigger)
                {
                    rowBottom[i1] = leftCellValue;
                    m_backtrackData[i2 * m_size1 + i1] = true;
                }
                else
                {
                    rowBottom[i1] = upperCellValue;
                    m_backtrackData[i2 * m_size1 + i1] = false;
                }
            }
        }

        // Bottom row will become top row
        std::swap(rowTop, rowBottom);
    }

    size_t i1 = m_size1 - 1;
    size_t i2 = m_size2 - 1;

    while (i1 > 0 && i2 > 0)
    {
        SequenceItem item;

        const size_t index1 = i1 - 1;
        const size_t index2 = i2 - 1;

        auto cit1 = std::next(m_it1, index1);
        auto cit2 = std::next(m_it2, index2);

        if (m_comparator(*cit1, *cit2))
        {
            item.index1 = index1;
            item.index2 = index2;

            --i1;
            --i2;
        }
        else
        {
            if (m_backtrackData[i2 * m_size1 + i1])
            {
                item.index1 = index1;
                --i1;
            }
            else
            {
                item.index2 = index2;
                --i2;
            }
        }

        m_sequence.push_back(item);
    }

    while (i1 > 0)
    {
        SequenceItem item;

        const size_t index1 = i1 - 1;
        item.index1 = index1;
        --i1;

        m_sequence.push_back(item);
    }

    while (i2 > 0)
    {
        SequenceItem item;

        const size_t index2 = i2 - 1;
        item.index2 = index2;
        --i2;

        m_sequence.push_back(item);
    }

    std::reverse(m_sequence.begin(), m_sequence.end());
}

}   // namespace pdf

#endif // PDFALGORITHMLCS_H
