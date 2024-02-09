//    Copyright (C) 2021-2022 Jakub Melka
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

#include "pdfalgorithmlcs.h"
#include "pdfdbgheap.h"

namespace pdf
{

void PDFAlgorithmLongestCommonSubsequenceBase::markSequence(Sequence& sequence,
                                                            const std::vector<size_t>& movedItemsLeft,
                                                            const std::vector<size_t>& movedItemsRight)
{
    Sequence updatedSequence;

    Q_ASSERT(std::is_sorted(movedItemsLeft.cbegin(), movedItemsLeft.cend()));
    Q_ASSERT(std::is_sorted(movedItemsRight.cbegin(), movedItemsRight.cend()));

    for (auto it = sequence.cbegin(); it != sequence.cend();)
    {
        if (it->isMatch())
        {
            updatedSequence.push_back(*it);
            ++it;
            continue;
        }

        Sequence leftItems;
        Sequence rightItems;

        for (; it != sequence.cend() && !it->isMatch(); ++it)
        {
            const SequenceItem& currentItem = *it;
            Q_ASSERT(currentItem.isLeft() || currentItem.isRight());

            if (currentItem.isLeft())
            {
                if (std::binary_search(movedItemsLeft.cbegin(), movedItemsLeft.cend(), currentItem.index1))
                {
                    SequenceItem item = *it;
                    item.markMovedLeft();
                    updatedSequence.push_back(item);
                }
                else
                {
                    leftItems.push_back(currentItem);
                }
            }

            if (currentItem.isRight())
            {
                if (std::binary_search(movedItemsRight.cbegin(), movedItemsRight.cend(), currentItem.index2))
                {
                    SequenceItem item = *it;
                    item.markMovedRight();
                    updatedSequence.push_back(item);
                }
                else
                {
                    rightItems.push_back(currentItem);
                }
            }
        }

        std::reverse(leftItems.begin(), leftItems.end());
        std::reverse(rightItems.begin(), rightItems.end());

        bool isReplaced = !leftItems.empty() && !rightItems.empty();

        while (!leftItems.empty() && !rightItems.empty())
        {
            SequenceItem item;
            item.index1 = leftItems.back().index1;
            item.index2 = rightItems.back().index2;
            item.markReplaced();
            updatedSequence.push_back(item);

            leftItems.pop_back();
            rightItems.pop_back();
        }

        while (!leftItems.empty())
        {
            SequenceItem item = leftItems.back();
            item.markRemoved();

            if (isReplaced)
            {
                item.markReplaced();
            }

            updatedSequence.push_back(item);
            leftItems.pop_back();
        }

        while (!rightItems.empty())
        {
            SequenceItem item = rightItems.back();
            item.markAdded();

            if (isReplaced)
            {
                item.markReplaced();
            }

            updatedSequence.push_back(item);
            rightItems.pop_back();
        }
    }

    for (SequenceItem& item : updatedSequence)
    {
        if (item.isMatch() && !item.isRemoved() && !item.isReplaced() && !item.isAdded() && item.index1 != item.index2)
        {
            item.markMoved();
        }
    }

    sequence = qMove(updatedSequence);
}

PDFAlgorithmLongestCommonSubsequenceBase::SequenceItemRanges PDFAlgorithmLongestCommonSubsequenceBase::getModifiedRanges(Sequence& sequence)
{
    SequenceItemRanges result;

    for (auto it = sequence.begin(); it != sequence.end();)
    {
        const SequenceItem& item = *it;
        if (!item.isModified())
        {
            ++it;
            continue;
        }

        // Jakub Melka: now, we have iterator pointing on item,
        // which has been modified. We will search for modification
        // range.

        auto itEnd = it;
        while (itEnd != sequence.end() && itEnd->isModified())
        {
            ++itEnd;
        }

        result.emplace_back(it, itEnd);
        it = itEnd;
    }

    return result;
}

PDFAlgorithmLongestCommonSubsequenceBase::SequenceItemFlags PDFAlgorithmLongestCommonSubsequenceBase::collectFlags(const SequenceItemRange& range)
{
    SequenceItemFlags flags = SequenceItemFlags();

    for (auto it = range.first; it != range.second; ++it)
    {
        flags |= it->flags;
    }

    return flags;
}

}   // namespace pdf
