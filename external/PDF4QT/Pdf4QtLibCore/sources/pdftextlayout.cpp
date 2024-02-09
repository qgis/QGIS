//    Copyright (C) 2019-2022 Jakub Melka
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

#include "pdftextlayout.h"
#include "pdfutils.h"
#include "pdfexecutionpolicy.h"

#include <QtMath>
#include <QMutex>
#include <QPainter>
#include <QIODevice>
#include <QMutexLocker>
#include <QRegularExpression>

#include "pdfdbgheap.h"

#include <execution>

namespace pdf
{

/// Spatial 2D index for indexing of text characters. It is a R-tree like structure,
/// build over an array of text characters. Array is modified (structure is build over
/// array).
class PDFTextCharacterSpatialIndex
{
public:
    explicit PDFTextCharacterSpatialIndex(TextCharacters* characters, size_t leafSize) :
        m_characters(characters),
        m_leafSize(leafSize),
        m_epsilon(0.0)
    {
        m_nodes.reserve(2 * characters->size() / leafSize);

        // Calculate epsilon from the bounding box. We must use epsilon to avoid empty
        // rectangles, which can occur, if text is on a single line.
        QRectF boundingBox = getBoundingBox(characters->begin(), characters->end());
        if (boundingBox.isValid())
        {
            qreal edge = qMax(boundingBox.width(), boundingBox.height());
            m_epsilon = edge * 0.001;
        }
        else
        {
            m_epsilon = 0.01;
        }

        build(characters->begin(), characters->end());
    }

    using Iterator = TextCharacters::iterator;

    /// Builds structure over range of iterators. Array is build in O(n * log^2 (n)) time.
    /// Index to internal nodes array is returned.
    /// \param it1 Start iterator
    /// \param it2 End iterator
    size_t build(Iterator it1, Iterator it2);

    /// Returns bounding box of character positions over given iterator range.
    /// If iterator range is empty, then empty bounding box is returned.
    /// \param it1 Start iterator
    /// \param it2 End iterator
    QRectF getBoundingBox(Iterator it1, Iterator it2) const;

    /// Performs query on structure - finds all characters, which are in given
    /// rectangle, and returns intersection size. If \p result parameter is set
    /// to valid pointer, all intersected  characters are inserted into the result
    /// array.
    /// \param rect Query rectangle
    /// \param result Result of query (can be nullptr)
    /// \returns Size of intersection
    size_t query(const QRectF& rect, TextCharacters* result) const;

    /// Finds character array, which contains at least \p minimalSize characters,
    /// with some extra characters, which must be filtered out.
    /// \param minimalSize Minimal size
    /// \param sample Sample character
    /// \param result Result
    void queryNearestEstimate(size_t minimalSize, const TextCharacter& sample, TextCharacters* result) const;

private:
    size_t queryImpl(size_t nodeIndex, const QRectF& rect, TextCharacters* result) const;

    struct Node
    {
        bool isLeaf = false;
        size_t index1 = 0;
        size_t index2 = 0;
        QRectF boundingBox;
    };

    using Nodes = std::vector<Node>;

    TextCharacters* m_characters;
    Nodes m_nodes;
    size_t m_leafSize;
    qreal m_epsilon;
};

size_t PDFTextCharacterSpatialIndex::build(Iterator it1, Iterator it2)
{
    size_t nodeIndex = m_nodes.size();

    if (size_t(std::distance(it1, it2)) < m_leafSize)
    {
        // Create leaf node
        Node node;
        node.isLeaf = true;
        node.index1 = std::distance(m_characters->begin(), it1);
        node.index2 = std::distance(m_characters->begin(), it2);
        node.boundingBox = getBoundingBox(it1, it2);
        m_nodes.push_back(qMove(node));
    }
    else
    {
        // Jakub Melka: split array of nodes into half, using larger side.
        // It is like in R-tree structure.
        m_nodes.push_back(Node());

        QRectF boundingBox = getBoundingBox(it1, it2);
        if (boundingBox.width() > boundingBox.height())
        {
            // Split using x-axis
            std::sort(it1, it2, [](const TextCharacter& l, const TextCharacter& r) { return l.position.x() < r.position.x(); });
        }
        else
        {
            // Split using y-axis
            std::sort(it1, it2, [](const TextCharacter& l, const TextCharacter& r) { return l.position.y() < r.position.y(); });
        }

        const size_t distance = std::distance(it1, it2);
        Iterator itMid = std::next(it1, distance / 2);

        const size_t index1 = build(it1, itMid);
        const size_t index2 = build(itMid, it2);

        Node& node = m_nodes[nodeIndex];
        node.isLeaf = false;
        node.index1 = index1;
        node.index2 = index2;
        node.boundingBox = boundingBox;
    }

    return nodeIndex;
}

QRectF PDFTextCharacterSpatialIndex::getBoundingBox(Iterator it1, Iterator it2) const
{
    if (it1 != it2)
    {
        qreal x_min = qInf();
        qreal x_max = -qInf();
        qreal y_min = qInf();
        qreal y_max = -qInf();

        for (Iterator it = it1; it != it2; ++it)
        {
            const TextCharacter& character = *it;
            x_min = qMin(x_min, character.position.x() - m_epsilon);
            x_max = qMax(x_max, character.position.x() + m_epsilon);
            y_min = qMin(y_min, character.position.y() - m_epsilon);
            y_max = qMax(y_max, character.position.y() + m_epsilon);
        }

        return QRectF(x_min, y_min, x_max - x_min, y_max - y_min);
    }

    return QRectF();
}

size_t PDFTextCharacterSpatialIndex::query(const QRectF& rect, TextCharacters* result) const
{
    if (!m_nodes.empty())
    {
        return queryImpl(0, rect, result);
    }

    return 0;
}

void PDFTextCharacterSpatialIndex::queryNearestEstimate(size_t minimalSize, const TextCharacter& sample, TextCharacters* result) const
{
    if (m_characters->size() <= minimalSize)
    {
        *result = *m_characters;
    }
    else
    {
        // Query result
        qreal querySizeEstimate = qMax(qMax(m_nodes[0].boundingBox.width(), m_nodes[0].boundingBox.height()) * 0.01, sample.advance * minimalSize * 0.5);

        QRectF rect(sample.position, QSizeF(querySizeEstimate, querySizeEstimate));
        rect.translate(-querySizeEstimate * 0.5, -querySizeEstimate * 0.5);

        while (query(rect, nullptr) < minimalSize)
        {
            qreal expansion = rect.width() * 0.5;
            rect.adjust(-expansion, -expansion, expansion, expansion);
        }

        qreal expansion = rect.width() * (qSqrt(2.0) - 1.0);
        rect.adjust(-expansion, -expansion, expansion, expansion);
        query(rect, result);
    }
}

size_t PDFTextCharacterSpatialIndex::queryImpl(size_t nodeIndex, const QRectF& rect, TextCharacters* result) const
{
    const Node& node = m_nodes[nodeIndex];

    if (!node.boundingBox.intersects(rect))
    {
        // Node is not intersected, just return
        return 0;
    }

    if (!node.isLeaf)
    {
        return queryImpl(node.index1, rect, result) + queryImpl(node.index2, rect, result);
    }
    else
    {
        // Jakub Melka: it is a leaf...
        auto isInside = [&rect](const TextCharacter& character)
        {
            return rect.contains(character.position);
        };

        auto itStart = std::next(m_characters->begin(), node.index1);
        auto itEnd = std::next(m_characters->begin(), node.index2);

        if (result)
        {
            const size_t oldSize = result->size();
            std::copy_if(itStart, itEnd, std::back_inserter(*result), isInside);
            return result->size() - oldSize;
        }

        return std::count_if(itStart, itEnd, isInside);
    }
}

PDFTextLayout::PDFTextLayout()
{

}

void PDFTextLayout::addCharacter(const PDFTextCharacterInfo& info)
{
    TextCharacter character;

    // Fill the basic info. For computing the angle, we must consider, if we are
    // in vertical writing system. If yes, take vertical edge of the character,
    // otherwise take horizontal edge.
    character.character = info.character;
    character.position = info.matrix.map(QPointF(0.0, 0.0));

    QLineF testLine(QPointF(0.0, 0.0), QPointF(info.isVerticalWritingSystem ? 0.0 : info.advance, !info.isVerticalWritingSystem ? 0.0 : info.advance));
    QLineF mappedLine = info.matrix.map(testLine);
    character.advance = mappedLine.length();
    character.angle = qRound(mappedLine.angle());

    QLineF fontTestLine(QPointF(0.0, 0.0), QPointF(0.0, info.fontSize));
    QLineF fontMappedLine = info.matrix.map(fontTestLine);
    character.fontSize = fontMappedLine.length();

    QRectF boundingBox = info.outline.boundingRect();
    character.boundingBox.addPolygon(info.matrix.map(boundingBox));

    m_characters.emplace_back(qMove(character));
    m_angles.insert(character.angle);
}

void PDFTextLayout::perform()
{
    for (PDFReal angle : m_angles)
    {
        performDoLayout(angle);
    }
}

void PDFTextLayout::optimize()
{
    m_characters.shrink_to_fit();
}

qint64 PDFTextLayout::getMemoryConsumptionEstimate() const
{
    qint64 estimate = sizeof(*this);
    estimate += sizeof(decltype(m_characters)::value_type) * m_characters.capacity();
    estimate += sizeof(decltype(m_angles)::value_type) * m_angles.size();
    return estimate;
}

bool PDFTextLayout::isHoveringOverTextBlock(const QPointF& point) const
{
    for (const PDFTextBlock& block : m_blocks)
    {
        if (block.getBoundingBox().contains(point))
        {
            return true;
        }
    }

    return false;
}

PDFTextSelection PDFTextLayout::createTextSelection(PDFInteger pageIndex, const QPointF& point1, const QPointF& point2, QColor selectionColor, bool strictSelection)
{
    PDFTextSelection selection;

    // Jakub Melka: We must treat each block in its own coordinate system. Because texts can
    // have different angles, we will treat each block separately.

    size_t blockId = 0;
    for (PDFTextBlock& block : m_blocks)
    {
        QTransform angleMatrix;
        angleMatrix.rotate(block.getAngle());
        block.applyTransform(angleMatrix);

        QPointF pointA = angleMatrix.map(point1);
        QPointF pointB = angleMatrix.map(point2);

        const qreal xMin = qMin(pointA.x(), pointB.x());
        const qreal yMin = qMin(pointA.y(), pointB.y());
        const qreal xMax = qMax(pointA.x(), pointB.x());
        const qreal yMax = qMax(pointA.y(), pointB.y());

        QRectF rect(xMin, yMin, xMax - xMin, yMax - yMin);
        QPainterPath rectPath;
        rectPath.addRect(rect);
        const QPainterPath& boundingBoxPath = block.getBoundingBox();
        QPainterPath intersectionPath = boundingBoxPath.intersected(rectPath);
        if (!intersectionPath.isEmpty())
        {
            QRectF intersectionRect = intersectionPath.boundingRect();
            Q_ASSERT(intersectionRect.isValid());

            bool isTopPointAboveText = false;
            bool isBottomPointBelowText = false;

            const PDFTextLines& lines = block.getLines();
            auto itLineA = std::find_if(lines.cbegin(), lines.cend(), [pointA](const PDFTextLine& line) { return line.getBoundingBox().contains(pointA); });
            auto itLineB = std::find_if(lines.cbegin(), lines.cend(), [pointB](const PDFTextLine& line) { return line.getBoundingBox().contains(pointB); });
            if (itLineA == itLineB && itLineA != lines.cend())
            {
                // Both points are in the same line. We consider point with lesser
                // horizontal coordinate as start selection point, and point with greater
                // horizontal coordinate as end selection point.
                if (pointA.x() > pointB.x())
                {
                    std::swap(pointA, pointB);
                }
            }
            else
            {
                // Otherwise points are not in the same line. Then start point will be
                // point top of the second point. Bottom point will mark end of selection.
                if (pointA.y() < pointB.y())
                {
                    std::swap(pointA, pointB);
                }

                QRectF boundingBoxPathBBRect = boundingBoxPath.controlPointRect();

                // If start point is above the text block, move start point to the left.
                if (!strictSelection && boundingBoxPathBBRect.bottom() < pointA.y())
                {
                    pointA.setX(boundingBoxPathBBRect.left());
                    isTopPointAboveText = true;
                }
                if (!strictSelection && boundingBoxPathBBRect.top() > pointB.y())
                {
                    pointB.setX(boundingBoxPathBBRect.right());
                    isBottomPointBelowText = true;
                }
            }

            // Now, we have pointA as start point and pointB as end point. We must found
            // nearest character to the right of point A, and nearest character to the
            // left of point B (with respect to point A/B).

            qreal maxDistanceA = std::numeric_limits<qreal>::infinity();
            qreal maxDistanceB = std::numeric_limits<qreal>::infinity();

            PDFCharacterPointer ptrA;
            PDFCharacterPointer ptrB;

            for (size_t lineId = 0, linesCount = lines.size(); lineId < linesCount; ++lineId)
            {
                const PDFTextLine& line = lines[lineId];
                QRectF lineBoundingRect = line.getBoundingBox().boundingRect();

                // We skip lines, which are not in the range (pointB.y(), pointA.y),
                // i.e. are above or below.
                if (lineBoundingRect.bottom() < pointB.y() ||
                    lineBoundingRect.top() > pointA.y())
                {
                    continue;
                }

                const TextCharacters& characters = line.getCharacters();
                for (size_t characterId = 0, characterCount = characters.size(); characterId < characterCount; ++characterId)
                {
                    const TextCharacter& character = characters[characterId];
                    QPointF characterCenter = character.boundingBox.boundingRect().center();

                    qreal distanceA = QLineF(pointA, characterCenter).length();
                    qreal distanceB = QLineF(pointB, characterCenter).length();

                    if (distanceA < maxDistanceA && characterCenter.x() > pointA.x())
                    {
                        maxDistanceA = distanceA;
                        ptrA.pageIndex = pageIndex;
                        ptrA.blockIndex = blockId;
                        ptrA.lineIndex = lineId;
                        ptrA.characterIndex = characterId;
                    }

                    if (distanceB < maxDistanceB && characterCenter.x() < pointB.x())
                    {
                        maxDistanceB = distanceB;
                        ptrB.pageIndex = pageIndex;
                        ptrB.blockIndex = blockId;
                        ptrB.lineIndex = lineId;
                        ptrB.characterIndex = characterId;
                    }
                }
            }

            if (isTopPointAboveText && !lines.empty())
            {
                ptrA.pageIndex = pageIndex;
                ptrA.blockIndex = blockId;
                ptrA.lineIndex = 0;
                ptrA.characterIndex = 0;
            }

            if (isBottomPointBelowText && !lines.empty())
            {
                ptrB.pageIndex = pageIndex;
                ptrB.blockIndex = blockId;
                ptrB.lineIndex = lines.size() - 1;
                ptrB.characterIndex = lines.back().getCharacters().size() - 1;
            }

            // If we have filled the pointers, add them to the selection
            if (ptrA.isValid() && ptrB.isValid())
            {
                if (ptrA < ptrB)
                {
                    selection.addItems({ PDFTextSelectionItem(ptrA, ptrB) }, selectionColor);
                }
                else
                {
                    selection.addItems({ PDFTextSelectionItem(ptrB, ptrA) }, selectionColor);
                }
            }
        }

        // Increment block index
        ++blockId;

        // Apply backward transformation to restore original coordinate system
        block.applyTransform(angleMatrix.inverted());
    }

    selection.build();
    return selection;
}

PDFTextSelection PDFTextLayout::selectBlock(const size_t blockIndex, PDFInteger pageIndex, QColor color) const
{
    PDFTextSelection selection;
    if (blockIndex >= m_blocks.size())
    {
        return selection;
    }

    const PDFTextBlock& textBlock = m_blocks[blockIndex];
    if (textBlock.getLines().empty())
    {
        return selection;
    }

    PDFCharacterPointer ptrA;
    PDFCharacterPointer ptrB;

    ptrA.pageIndex = pageIndex;
    ptrA.blockIndex = blockIndex;
    ptrA.lineIndex = 0;
    ptrA.characterIndex = 0;

    ptrB.pageIndex = pageIndex;
    ptrB.blockIndex = blockIndex;
    ptrB.lineIndex = textBlock.getLines().size() - 1;
    ptrB.characterIndex = textBlock.getLines().back().getCharacters().size() - 1;

    selection.addItems({ PDFTextSelectionItem(ptrA, ptrB) }, color);
    selection.build();
    return selection;
}

PDFTextSelection PDFTextLayout::selectLineInBlock(const size_t blockIndex, const size_t lineIndex, PDFInteger pageIndex, QColor color) const
{
    PDFTextSelection selection;
    if (blockIndex >= m_blocks.size())
    {
        return selection;
    }

    const PDFTextBlock& textBlock = m_blocks[blockIndex];
    if (lineIndex >= textBlock.getLines().size())
    {
        return selection;
    }

    PDFCharacterPointer ptrA;
    PDFCharacterPointer ptrB;

    ptrA.pageIndex = pageIndex;
    ptrA.blockIndex = blockIndex;
    ptrA.lineIndex = lineIndex;
    ptrA.characterIndex = 0;

    ptrB.pageIndex = pageIndex;
    ptrB.blockIndex = blockIndex;
    ptrB.lineIndex = lineIndex;
    ptrB.characterIndex = textBlock.getLines()[lineIndex].getCharacters().size() - 1;

    selection.addItems({ PDFTextSelectionItem(ptrA, ptrB) }, color);
    selection.build();
    return selection;
}

QString PDFTextLayout::getTextFromSelection(PDFTextSelection::iterator itBegin, PDFTextSelection::iterator itEnd, PDFInteger pageIndex) const
{
    QStringList text;

    if (itBegin != itEnd)
    {
        PDFTextFlows flows = PDFTextFlow::createTextFlows(*this, PDFTextFlow::RemoveSoftHyphen, pageIndex);
        Q_ASSERT(flows.size() < 2);

        if (!flows.empty())
        {
            const PDFTextFlow& textFlow = flows.front();
            for (auto it = itBegin; it != itEnd; ++it)
            {
                text << textFlow.getText(it->start, it->end);
            }
        }
    }

    return text.join("\n");
}

QString PDFTextLayout::getTextFromSelection(const PDFTextSelection& selection, PDFInteger pageIndex) const
{
    return getTextFromSelection(selection.begin(pageIndex), selection.end(pageIndex), pageIndex);
}

QDataStream& operator>>(QDataStream& stream, PDFTextLayout& layout)
{
    stream >> layout.m_characters;
    stream >> layout.m_angles;
    stream >> layout.m_settings;
    stream >> layout.m_blocks;
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const PDFTextLayout& layout)
{
    stream << layout.m_characters;
    stream << layout.m_angles;
    stream << layout.m_settings;
    stream << layout.m_blocks;
    return stream;
}

struct NearestCharacterInfo
{
    size_t index = std::numeric_limits<size_t>::max();
    PDFReal distance = std::numeric_limits<PDFReal>::infinity();

    inline bool operator<(const NearestCharacterInfo& other) const { return distance < other.distance; }
};

void PDFTextLayout::performDoLayout(PDFReal angle)
{
    // We will implement variation of 'docstrum' algorithm, we have divided characters by angles,
    // for each angle we get characters for that particular angle, and run 'docstrum' algorithm.
    // We will do following steps:
    //      1) Rotate the plane with characters so that they are all in horizontal line
    //      2) Find k-nearest characters for each character (so each character will have
    //         k pointers to the nearest characters)
    //      3) Find text lines. We will do that by creating transitive closure of characters, i.e.
    //         characters, which are close and are on horizontal line, are marked as in one text line.
    //         Consider also font size and empty space size between different characters.
    //      4) Merge text lines into text blocks using various criteria, such as overlap,
    //         distance between the lines, and also using again, transitive closure.
    //      5) Sort blocks using topological ordering
    TextCharacters characters = getCharactersForAngle(angle);

    // Step 1) - rotate blocks
    QTransform angleMatrix;
    angleMatrix.rotate(angle);
    applyTransform(characters, angleMatrix);

    // Create spatial index
    PDFTextCharacterSpatialIndex spatialIndex(&characters, 16);
    for (size_t i = 0, count = characters.size(); i < count; ++i)
    {
        characters[i].index = i;
    }

    // Step 2) - find k-nearest characters
    const size_t characterCount = characters.size();
    const size_t bucketSize = m_settings.samples + 1;
    std::vector<NearestCharacterInfo> nearestCharacters(bucketSize * characters.size(), NearestCharacterInfo());

    auto findNearestCharacters = [this, bucketSize, &characters, &spatialIndex, &nearestCharacters](size_t currentCharacterIndex)
    {
        // It will be iterator to the start of the nearest neighbour sequence
        auto it = std::next(nearestCharacters.begin(), currentCharacterIndex * bucketSize);
        auto itLast = std::next(it, m_settings.samples);
        NearestCharacterInfo& insertInfo = *itLast;
        QPointF currentPoint = characters[currentCharacterIndex].position;

        TextCharacters nearestPointSamples;
        spatialIndex.queryNearestEstimate(m_settings.samples, characters[currentCharacterIndex], &nearestPointSamples);
        for (size_t i = 0, count = nearestPointSamples.size(); i < count; ++i)
        {
            if (nearestPointSamples[i].index == currentCharacterIndex)
            {
                continue;
            }

            insertInfo.index = nearestPointSamples[i].index;
            insertInfo.distance = QLineF(currentPoint, nearestPointSamples[i].position).length();

            // Now, use insert sort to sort the array of samples + 1 elements (#samples elements
            // are sorted, we use only insert sort on the last element).
            auto itLeft = std::prev(itLast);
            auto itRight = itLast;
            while (true)
            {
                if (*itRight < *itLeft)
                {
                    std::swap(*itRight, *itLeft);
                    itRight = itLeft;

                    if (itLeft == it)
                    {
                        // We have reached the end
                        break;
                    }

                    --itLeft;
                }
                else
                {
                    // We have proper order, break the cycle
                    break;
                }
            }
        }
    };

    auto range = PDFIntegerRange<size_t>(0, characterCount);
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), findNearestCharacters);

    // Step 3) - detect lines
    PDFUnionFindAlgorithm<size_t> textLinesUF(characterCount);
    for (size_t i = 0; i < characterCount; ++i)
    {
        auto it = std::next(nearestCharacters.begin(), i * bucketSize);
        auto itEnd = std::next(it, m_settings.samples);

        for (; it != itEnd; ++it)
        {
            const NearestCharacterInfo& info = *it;
            if (info.index == std::numeric_limits<size_t>::max())
            {
                // We have reached the end - or we do not have enough characters
                break;
            }

            // Criteria:
            //   1) Distance of characters is not too large
            //   2) Characters are approximately at same line
            //   3) Font size of characters are approximately equal

            PDFReal fontSizeMax = qMax(characters[i].fontSize, characters[info.index].fontSize);
            PDFReal fontSizeMin = qMin(characters[i].fontSize, characters[info.index].fontSize);

            if (info.distance < m_settings.distanceSensitivity * characters[i].advance && // 1)
                std::fabs(characters[i].position.y() - characters[info.index].position.y()) < fontSizeMin * m_settings.charactersOnLineSensitivity && // 2)
                fontSizeMax / fontSizeMin < m_settings.fontSensitivity) // 3)
            {
                textLinesUF.unify(i, info.index);
            }
        }
    }

    std::map<size_t, TextCharacters> lineToCharactersMap;
    for (size_t i = 0; i < characterCount; ++i)
    {
        lineToCharactersMap[textLinesUF.find(i)].push_back(characters[i]);
    }

    PDFTextLines lines;
    lines.reserve(lineToCharactersMap.size());
    for (auto& item : lineToCharactersMap)
    {
        lines.emplace_back(qMove(item.second));
    }

    // Step 4) - detect text blocks
    const size_t lineCount = lines.size();
    PDFUnionFindAlgorithm<size_t> textBlocksUF(lineCount);
    for (size_t i = 0; i < lineCount; ++i)
    {
        for (size_t j = i + 1; j < lineCount; ++j)
        {
            QRectF bb1 = lines[i].getBoundingBox().boundingRect();
            QRectF bb2 = lines[j].getBoundingBox().boundingRect();

            // Jakub Melka: we will join two blocks, if these two conditions both holds:
            //     1) bounding boxes overlap horizontally by large portion
            //     2) vertical space between bounding boxes is not too large

            QRectF bbUnion = bb1.united(bb2);
            const PDFReal height = bbUnion.height();
            const PDFReal heightLimit = (bb1.height() + bb2.height()) * m_settings.blockVerticalSensitivity;
            const PDFReal overlap = qMax(0.0, bb1.width() + bb2.width() - bbUnion.width());
            const PDFReal minimalOverlap = qMin(bb1.width(), bb2.width()) * m_settings.blockOverlapSensitivity;
            if (height < heightLimit && overlap > minimalOverlap)
            {
                textBlocksUF.unify(i, j);
            }
        }
    }

    std::map<size_t, PDFTextLines> blockToLines;
    for (size_t i = 0; i < lineCount; ++i)
    {
        blockToLines[textBlocksUF.find(i)].push_back(qMove(lines[i]));
    }

    PDFTextBlocks blocks;
    blocks.reserve(blockToLines.size());
    for (auto& item : blockToLines)
    {
        blocks.emplace_back(qMove(item.second));
    }

    // 5) Sort block by topological ordering. We will use approache described in paper
    // "High Performance Document Layout Analysis", T.M. Breuel, 2003, where are described
    // two rules, which are used to determine block precedence.
    //
    // Rule 1: a<b, if:
    //    - blocks a,b have overlap in x-axis
    //    - block a is above block b
    //
    // Rule 2: a<b, if:
    //    - block a is entirely on left side of block b
    //    - there doesn't exist block c, which is between a,b in y-axis
    //      and moreover, overlaps both a and b in x-axis.

    auto isBeforeByRule1 = [&blocks](const size_t aIndex, const size_t bIndex)
    {
        QRectF aBB = blocks[aIndex].getBoundingBox().boundingRect();
        QRectF bBB = blocks[bIndex].getBoundingBox().boundingRect();

        const bool isOverlappedOnHorizontalAxis = isRectangleHorizontallyOverlapped(aBB, bBB);
        const bool isAoverB = aBB.bottom() > bBB.top();
        return isOverlappedOnHorizontalAxis && isAoverB;
    };
    auto isBeforeByRule2 = [&blocks](const size_t aIndex, const size_t bIndex)
    {
        QRectF aBB = blocks[aIndex].getBoundingBox().boundingRect();
        QRectF bBB = blocks[bIndex].getBoundingBox().boundingRect();
        QRectF abBB = aBB.united(bBB);

        if (aBB.right() < bBB.left())
        {
            // Check, if 'c' block doesn't exist
            for (size_t i = 0, count = blocks.size(); i < count; ++i)
            {
                if (i == aIndex || i == bIndex)
                {
                    continue;
                }

                QRectF cBB = blocks[i].getBoundingBox().boundingRect();
                if (cBB.top() >= abBB.top() && cBB.bottom() <= abBB.bottom())
                {
                    const bool isAOverlappedOnHorizontalAxis = isRectangleHorizontallyOverlapped(aBB, cBB);
                    const bool isBOverlappedOnHorizontalAxis = isRectangleHorizontallyOverlapped(bBB, cBB);
                    if (isAOverlappedOnHorizontalAxis && isBOverlappedOnHorizontalAxis)
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        return false;
    };

    // Order blocks using topological sort (https://en.wikipedia.org/wiki/Topological_sorting,
    // Kahn's algorithm is used)
    std::set<size_t> workBlocks;
    std::vector<size_t> ordering;
    std::vector<std::set<size_t>> orderingEdges(blocks.size(), std::set<size_t>());
    ordering.reserve(blocks.size());
    for (size_t i = 0; i < blocks.size(); ++i)
    {
        workBlocks.insert(workBlocks.end(), i);
        for (size_t j = 0; j < blocks.size(); ++j)
        {
            if (i != j && (isBeforeByRule1(j, i) || isBeforeByRule2(j, i)))
            {
                orderingEdges[i].insert(j);
            }
        }
    }

    // Topological sort
    QTransform invertedAngleMatrix = angleMatrix.inverted();
    while (!workBlocks.empty())
    {
        auto it = std::min_element(workBlocks.begin(), workBlocks.end(), [&orderingEdges](const size_t l, const size_t r) { return orderingEdges[l].size() < orderingEdges[r].size(); });
        ordering.push_back(*it);
        for (std::set<size_t>& edges : orderingEdges)
        {
            edges.erase(*it);
        }

        blocks[*it].applyTransform(invertedAngleMatrix);
        m_blocks.emplace_back(qMove(blocks[*it]));
        workBlocks.erase(it);
    }
}

TextCharacters PDFTextLayout::getCharactersForAngle(PDFReal angle) const
{
    TextCharacters result;
    std::copy_if(m_characters.cbegin(), m_characters.cend(), std::back_inserter(result), [angle](const TextCharacter& character) { return character.angle == angle; });
    return result;
}

void PDFTextLayout::applyTransform(TextCharacters& characters, const QTransform& matrix)
{
    for (TextCharacter& character : characters)
    {
        character.position = matrix.map(character.position);
        character.boundingBox = matrix.map(character.boundingBox);
    }
}

PDFTextLine::PDFTextLine(TextCharacters characters) :
    m_characters(qMove(characters))
{
    std::sort(m_characters.begin(), m_characters.end(), [](const TextCharacter& l, const TextCharacter& r) { return l.position.x() < r.position.x(); });

    QRectF boundingBox;
    for (const TextCharacter& character : m_characters)
    {
        boundingBox = boundingBox.united(character.boundingBox.boundingRect());
    }
    m_boundingBox.addRect(boundingBox);
    m_topLeft = boundingBox.topLeft();
}

PDFReal PDFTextLine::getAngle() const
{
    if (!m_characters.empty())
    {
        return m_characters.front().angle;
    }

    return 0.0;
}

void PDFTextLine::applyTransform(const QTransform& matrix)
{
    m_boundingBox = matrix.map(m_boundingBox);
    m_topLeft = matrix.map(m_topLeft);
    for (TextCharacter& character : m_characters)
    {
        character.applyTransform(matrix);
    }
}

QDataStream& operator>>(QDataStream& stream, PDFTextLine& line)
{
    stream >> line.m_characters;
    stream >> line.m_boundingBox;
    stream >> line.m_topLeft;
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const PDFTextLine& line)
{
    stream << line.m_characters;
    stream << line.m_boundingBox;
    stream << line.m_topLeft;
    return stream;
}

PDFTextBlock::PDFTextBlock(PDFTextLines textLines) :
    m_lines(qMove(textLines))
{
    auto sortFunction = [](const PDFTextLine& l, const PDFTextLine& r)
    {
        QRectF bl = l.getBoundingBox().boundingRect();
        QRectF br = r.getBoundingBox().boundingRect();
        const PDFReal xL = bl.x();
        const PDFReal xR = br.x();
        const PDFReal yL = qRound(bl.y() * 100.0);
        const PDFReal yR = qRound(br.y() * 100.0);
        return std::make_pair(-yL, xL) < std::make_pair(-yR, xR);
    };
    std::sort(m_lines.begin(), m_lines.end(), sortFunction);

    QRectF boundingBox;
    for (const PDFTextLine& line : m_lines)
    {
        boundingBox = boundingBox.united(line.getBoundingBox().boundingRect());
    }
    m_boundingBox.addRect(boundingBox);
    m_topLeft = boundingBox.topLeft();
}

PDFReal PDFTextBlock::getAngle() const
{
    if (!m_lines.empty())
    {
        return m_lines.front().getAngle();
    }

    return 0.0;
}

void PDFTextBlock::applyTransform(const QTransform& matrix)
{
    m_boundingBox = matrix.map(m_boundingBox);
    m_topLeft = matrix.map(m_topLeft);
    for (PDFTextLine& textLine : m_lines)
    {
        textLine.applyTransform(matrix);
    }
}

QPainterPath PDFTextBlock::getCharacterRangeBoundingPath(const PDFCharacterPointer& start,
                                                         const PDFCharacterPointer& end,
                                                         const QTransform& matrix,
                                                         PDFReal heightIncreaseFactor) const
{
    QPainterPath path;

    PDFTextBlock block = *this;

    // Fix angle of block, so we will get correct selection rectangles (parallel to lines)
    QTransform angleMatrix;
    angleMatrix.rotate(block.getAngle());
    block.applyTransform(angleMatrix);

    const size_t lineStart = start.lineIndex;
    const size_t lineEnd = end.lineIndex;
    Q_ASSERT(lineEnd >= lineStart);

    const PDFTextLines& lines = block.getLines();
    for (size_t lineIndex = lineStart; lineIndex <= lineEnd; ++lineIndex)
    {
        if (lineIndex >= lines.size())
        {
            // Selection is invalid, do nothing
            continue;
        }

        const PDFTextLine& line = lines[lineIndex];
        const TextCharacters& characters = line.getCharacters();

        if (characters.empty())
        {
            // Selection is invalid, do nothing
            continue;
        }

        // First determine, which characters will be selected
        size_t characterStart = 0;
        size_t characterEnd = characters.size() - 1;

        if (lineIndex == lineStart)
        {
            characterStart = start.characterIndex;
        }
        if (lineIndex == lineEnd)
        {
            characterEnd = end.characterIndex;
        }

        // Validate indices, then calculate bounding box
        if (!(characterStart <= characterEnd && characterEnd < characters.size()))
        {
            continue;
        }

        QRectF boundingBox;
        for (size_t i = characterStart; i <= characterEnd; ++i)
        {
            boundingBox = boundingBox.united(characters[i].boundingBox.boundingRect());
        }

        if (boundingBox.isValid())
        {
            // Enlarge height by some percent
            PDFReal heightAdvance = boundingBox.height() * heightIncreaseFactor * 0.5;
            boundingBox.adjust(0, -heightAdvance, 0, heightAdvance);
            path.addRect(boundingBox);
        }
    }

    QTransform transformMatrix = angleMatrix.inverted() * matrix;
    path = transformMatrix.map(path);

    return path;
}

QDataStream& operator>>(QDataStream& stream, PDFTextBlock& block)
{
    stream >> block.m_lines;
    stream >> block.m_boundingBox;
    stream >> block.m_topLeft;
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const PDFTextBlock& block)
{
    stream << block.m_lines;
    stream << block.m_boundingBox;
    stream << block.m_topLeft;
    return stream;
}

void TextCharacter::applyTransform(const QTransform& matrix)
{
    position = matrix.map(position);
    boundingBox = matrix.map(boundingBox);
}

QDataStream& operator<<(QDataStream& stream, const TextCharacter& character)
{
    stream << character.character;
    stream << character.position;
    stream << character.angle;
    stream << character.fontSize;
    stream << character.advance;
    stream << character.boundingBox;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, TextCharacter& character)
{
    stream >> character.character;
    stream >> character.position;
    stream >> character.angle;
    stream >> character.fontSize;
    stream >> character.advance;
    stream >> character.boundingBox;
    return stream;
}

PDFTextLayout PDFTextLayoutStorage::getTextLayout(PDFInteger pageIndex) const
{
    PDFTextLayout result;

    if (pageIndex >= 0 && pageIndex < static_cast<PDFInteger>(m_offsets.size()))
    {
        QDataStream layoutStream(const_cast<QByteArray*>(&m_textLayouts), QIODevice::ReadOnly);
        layoutStream.skipRawData(m_offsets[pageIndex]);

        QByteArray buffer;
        layoutStream >> buffer;
        buffer = qUncompress(buffer);

        QDataStream stream(&buffer, QIODevice::ReadOnly);
        stream >> result;
    }

    return result;
}

void PDFTextLayoutStorage::setTextLayout(PDFInteger pageIndex, const PDFTextLayout& layout, QMutex* mutex)
{
    QByteArray result;
    {
        QDataStream stream(&result, QIODevice::WriteOnly);
        stream << layout;
    }
    result = qCompress(result, 9);

    QMutexLocker lock(mutex);
    m_offsets[pageIndex] = m_textLayouts.size();

    QDataStream layoutStream(&m_textLayouts, QIODevice::Append | QIODevice::WriteOnly);
    layoutStream << result;
}

PDFFindResults PDFTextLayoutStorage::find(const QString& text, Qt::CaseSensitivity caseSensitivity, PDFTextFlow::FlowFlags flowFlags) const
{
    PDFFindResults results;

    QMutex resultsMutex;
    auto findImpl = [this, flowFlags, caseSensitivity, &results, &resultsMutex, &text](size_t pageIndex)
    {
        PDFTextLayout textLayout = getTextLayout(pageIndex);
        PDFTextFlows textFlows = PDFTextFlow::createTextFlows(textLayout, flowFlags, pageIndex);
        for (const PDFTextFlow& textFlow : textFlows)
        {
            PDFFindResults flowResults = textFlow.find(text, caseSensitivity);

            // Jakub Melka: Do not lock mutex, if we didn't find anything. In that case, just skip to next flow.
            if (!flowResults.empty())
            {
                QMutexLocker lock(&resultsMutex);
                results.insert(results.end(), flowResults.begin(), flowResults.end());
            }
        }
    };

    auto range = PDFIntegerRange<size_t>(0, m_offsets.size());
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, range.begin(), range.end(), findImpl);

    std::sort(results.begin(), results.end());
    return results;
}

PDFFindResults PDFTextLayoutStorage::find(const QRegularExpression& expression, PDFTextFlow::FlowFlags flowFlags) const
{
    PDFFindResults results;

    QMutex resultsMutex;
    auto findImpl = [this, flowFlags, &results, &resultsMutex, &expression](size_t pageIndex)
    {
        PDFTextLayout textLayout = getTextLayout(pageIndex);
        PDFTextFlows textFlows = PDFTextFlow::createTextFlows(textLayout, flowFlags, pageIndex);
        for (const PDFTextFlow& textFlow : textFlows)
        {
            PDFFindResults flowResults = textFlow.find(expression);

            // Jakub Melka: Do not lock mutex, if we didn't find anything. In that case, just skip to next flow.
            if (!flowResults.empty())
            {
                QMutexLocker lock(&resultsMutex);
                results.insert(results.end(), flowResults.begin(), flowResults.end());
            }
        }
    };

    auto range = PDFIntegerRange<size_t>(0, m_offsets.size());
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, range.begin(), range.end(), findImpl);

    std::sort(results.begin(), results.end());
    return results;
}

QDataStream& operator<<(QDataStream& stream, const PDFTextLayoutSettings& settings)
{
    stream << settings.samples;
    stream << settings.distanceSensitivity;
    stream << settings.charactersOnLineSensitivity;
    stream << settings.fontSensitivity;
    stream << settings.blockVerticalSensitivity;
    stream << settings.blockOverlapSensitivity;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, PDFTextLayoutSettings& settings)
{
    stream >> settings.samples;
    stream >> settings.distanceSensitivity;
    stream >> settings.charactersOnLineSensitivity;
    stream >> settings.fontSensitivity;
    stream >> settings.blockVerticalSensitivity;
    stream >> settings.blockOverlapSensitivity;
    return stream;
}

void PDFTextSelection::addItems(const PDFTextSelectionItems& items, QColor color)
{
    std::transform(items.cbegin(), items.cend(), std::back_inserter(m_items), [color] (const auto& item) { return PDFTextSelectionColoredItem(item.first, item.second, color); });
}

void PDFTextSelection::build()
{
    std::sort(m_items.begin(), m_items.end());
}

PDFTextSelection::iterator PDFTextSelection::begin(PDFInteger pageIndex) const
{
    Q_ASSERT(std::is_sorted(m_items.cbegin(), m_items.end()));

    PDFCharacterPointer pointer;
    pointer.pageIndex = pageIndex;
    pointer.blockIndex = 0;
    pointer.lineIndex = 0;
    pointer.characterIndex = 0;

    PDFTextSelectionColoredItem item;
    item.start = pointer;
    item.end = pointer;

    return std::lower_bound(m_items.cbegin(), m_items.end(), item);
}

PDFTextSelection::iterator PDFTextSelection::end(PDFInteger pageIndex) const
{
    Q_ASSERT(std::is_sorted(m_items.cbegin(), m_items.end()));

    PDFCharacterPointer pointer;
    pointer.pageIndex = pageIndex;
    pointer.blockIndex = std::numeric_limits<decltype(pointer.blockIndex)>::max();
    pointer.lineIndex = std::numeric_limits<decltype(pointer.lineIndex)>::max();
    pointer.characterIndex = std::numeric_limits<decltype(pointer.characterIndex)>::max();

    PDFTextSelectionColoredItem item;
    item.start = pointer;
    item.end = pointer;

    return std::upper_bound(m_items.cbegin(), m_items.end(), item);
}

PDFTextSelection::iterator PDFTextSelection::nextPageRange(iterator currentPageRange) const
{
    auto it = currentPageRange;
    while (it != m_items.cend() && it->start.pageIndex == currentPageRange->start.pageIndex)
    {
        ++it;
    }

    return it;
}

PDFFindResults PDFTextFlow::find(const QString& text, Qt::CaseSensitivity caseSensitivity) const
{
    PDFFindResults results;

    int index = m_text.indexOf(text, 0, caseSensitivity);
    while (index != -1)
    {
        PDFFindResult result;
        result.matched = text;
        result.textSelectionItems = getTextSelectionItems(index, text.length());
        result.context = getContext(index, text.length());

        if (!result.textSelectionItems.empty())
        {
            results.emplace_back(qMove(result));
        }

        index = m_text.indexOf(text, index + 1, caseSensitivity);
    }

    return results;
}

PDFFindResults PDFTextFlow::find(const QRegularExpression& expression) const
{
    PDFFindResults results;

    QRegularExpressionMatchIterator iterator = expression.globalMatch(m_text, 0, QRegularExpression::NormalMatch, QRegularExpression::NoMatchOption);
    while (iterator.hasNext())
    {
        QRegularExpressionMatch match = iterator.next();

        Q_ASSERT(match.hasMatch());
        const int index = match.capturedStart();
        const int length = match.capturedLength();

        PDFFindResult result;
        result.matched = match.captured();
        result.textSelectionItems = getTextSelectionItems(index, length);
        result.context = getContext(index, length);

        if (!result.textSelectionItems.empty())
        {
            results.emplace_back(qMove(result));
        }
    }

    return results;
}

QString PDFTextFlow::getText(const PDFCharacterPointer& begin, const PDFCharacterPointer& end) const
{
    auto it = std::find(m_characterPointers.cbegin(), m_characterPointers.cend(), begin);
    auto itEnd = std::find(m_characterPointers.cbegin(), m_characterPointers.cend(), end);

    const std::size_t startIndex = std::distance(m_characterPointers.cbegin(), it);
    const std::size_t endIndex = std::distance(m_characterPointers.cbegin(), itEnd);
    if (startIndex <= endIndex)
    {
        return m_text.mid(int(startIndex), int(endIndex - startIndex + 1));
    }

    return QString();
}

void PDFTextFlow::merge(const PDFTextFlow& next)
{
    m_text += next.m_text;
    m_boundingBox = m_boundingBox.united(next.m_boundingBox);
    m_characterPointers.insert(m_characterPointers.end(), next.m_characterPointers.cbegin(), next.m_characterPointers.cend());
    m_characterBoundingBoxes.insert(m_characterBoundingBoxes.end(), next.m_characterBoundingBoxes.cbegin(), next.m_characterBoundingBoxes.cend());
}

PDFTextFlows PDFTextFlow::createTextFlows(const PDFTextLayout& layout, FlowFlags flags, PDFInteger pageIndex)
{
    PDFTextFlows result;

    if (!flags.testFlag(SeparateBlocks))
    {
        result.emplace_back();
    }

    QString lineBreak(" ");
    if (flags.testFlag(AddLineBreaks))
    {
#if defined(Q_OS_WIN)
        lineBreak = QString("\r\n");
#elif defined(Q_OS_UNIX)
        lineBreak = QString("\n");
#elif defined(Q_OS_MAC)
        lineBreak = QString("\r");
#else
        static_assert(false, "Fix this code!");
#endif
    }

    size_t textBlockIndex = 0;
    for (const PDFTextBlock& textBlock : layout.getTextBlocks())
    {
        PDFTextFlow currentFlow;
        currentFlow.m_boundingBox = textBlock.getBoundingBox().controlPointRect();

        size_t textLineIndex = 0;
        for (const PDFTextLine& textLine : textBlock.getLines())
        {
            const TextCharacters& characters = textLine.getCharacters();
            for (size_t i = 0, characterCount = characters.size(); i < characterCount; ++i)
            {
                const TextCharacter& currentCharacter = characters[i];
                if (i > 0 && !currentCharacter.character.isSpace())
                {
                    // Jakub Melka: try to guess space between letters
                    const TextCharacter& previousCharacter = characters[i - 1];
                    if (!previousCharacter.character.isSpace() && QLineF(previousCharacter.position, currentCharacter.position).length() > previousCharacter.advance * 1.2)
                    {
                        currentFlow.m_text += QChar(' ');
                        currentFlow.m_characterPointers.emplace_back();
                        currentFlow.m_characterBoundingBoxes.emplace_back();
                    }
                }

                currentFlow.m_text += currentCharacter.character;

                PDFCharacterPointer pointer;
                pointer.pageIndex = pageIndex;
                pointer.blockIndex = textBlockIndex;
                pointer.lineIndex = textLineIndex;
                pointer.characterIndex = i;
                currentFlow.m_characterPointers.emplace_back(qMove(pointer));
                currentFlow.m_characterBoundingBoxes.emplace_back(currentCharacter.boundingBox.controlPointRect());
            }

            // Remove soft hyphen, if it is enabled
            if (flags.testFlag(RemoveSoftHyphen) && !characters.empty() && currentFlow.m_text.back() == QChar(QChar::SoftHyphen))
            {
                currentFlow.m_text.chop(1);
                currentFlow.m_characterPointers.pop_back();
                currentFlow.m_characterBoundingBoxes.pop_back();

                if (!flags.testFlag(AddLineBreaks))
                {
                    // Do not add single empty space - because soft hypen probably breaks a word
                    ++textLineIndex;
                    continue;
                }
            }

            // Add line break
            currentFlow.m_text += lineBreak;
            currentFlow.m_characterPointers.insert(currentFlow.m_characterPointers.end(), lineBreak.length(), PDFCharacterPointer());
            currentFlow.m_characterBoundingBoxes.insert(currentFlow.m_characterBoundingBoxes.end(), lineBreak.length(), QRectF());

            ++textLineIndex;
        }

        // If we are producing separate blocks, then make flow for each
        // text block, otherwise join flows.
        if (flags.testFlag(SeparateBlocks))
        {
            result.emplace_back(qMove(currentFlow));
        }
        else
        {
            result.back().merge(currentFlow);
        }

        ++textBlockIndex;
    }

    return result;
}

PDFTextSelectionItems PDFTextFlow::getTextSelectionItems(size_t index, size_t length) const
{
    PDFTextSelectionItems items;

    auto it = std::next(m_characterPointers.cbegin(), index);
    auto itEnd = std::next(m_characterPointers.cbegin(), index + length);
    while (it != itEnd)
    {
        // Skip invalid items, find first valid
        if (!it->isValid())
        {
            ++it;
            continue;
        }

        auto itSelectionStart = it;
        while (it != itEnd && it->isValid() && it->hasSameBlock(*itSelectionStart))
        {
            ++it;
        }
        auto itSelectionEnd = std::prev(it);
        items.emplace_back(*itSelectionStart, *itSelectionEnd);
    }

    std::sort(items.begin(), items.end());
    return items;
}

QString PDFTextFlow::getContext(size_t index, size_t length) const
{
    Q_ASSERT(length > 0);

    const PDFCharacterPointer& frontComparatorItem = m_characterPointers[index];
    while (index > 0 && (m_characterPointers[index - 1].hasSameLine(frontComparatorItem) || !m_characterPointers[index - 1].isValid()))
    {
        --index;
        ++length;
    }

    size_t currentEnd = index + length - 1;
    size_t last = m_characterPointers.size() - 1;
    const PDFCharacterPointer& backComparatorItem = m_characterPointers[currentEnd];
    while (currentEnd < last && (m_characterPointers[currentEnd + 1].hasSameLine(backComparatorItem) || !m_characterPointers[currentEnd + 1].isValid()))
    {
        ++currentEnd;
        ++length;
    }

    return m_text.mid(int(index), int(length)).trimmed();
}

bool PDFCharacterPointer::hasSameBlock(const PDFCharacterPointer& other) const
{
    return pageIndex == other.pageIndex && blockIndex == other.blockIndex;
}

bool PDFCharacterPointer::hasSameLine(const PDFCharacterPointer& other) const
{
    return hasSameBlock(other) && lineIndex == other.lineIndex;
}

bool PDFFindResult::operator<(const PDFFindResult& other) const
{
    Q_ASSERT(!textSelectionItems.empty());
    Q_ASSERT(!other.textSelectionItems.empty());

    return textSelectionItems.front() < other.textSelectionItems.front();
}

PDFTextLayout PDFTextLayoutStorageGetter::getTextLayoutImpl() const
{
    return m_storage ? m_storage->getTextLayout(m_pageIndex) : PDFTextLayout();
}

void PDFTextSelectionPainter::draw(QPainter* painter, PDFInteger pageIndex, PDFTextLayoutGetter& textLayoutGetter, const QTransform& matrix)
{
    Q_ASSERT(painter);

    auto it = m_selection->begin(pageIndex);
    auto itEnd = m_selection->end(pageIndex);

    if (it == itEnd)
    {
        // Jakub Melka: no text is selected on current page; do nothing
        return;
    }

    painter->save();

    const PDFTextLayout& layout = textLayoutGetter;
    const PDFTextBlocks& blocks = layout.getTextBlocks();
    for (; it != itEnd; ++it)
    {
        const PDFTextSelectionColoredItem& item = *it;
        const PDFCharacterPointer& start = item.start;
        const PDFCharacterPointer& end = item.end;

        Q_ASSERT(start.pageIndex == end.pageIndex);
        Q_ASSERT(start.blockIndex == end.blockIndex);

        if (start.blockIndex >= blocks.size())
        {
            // Selection is invalid, do nothing
            continue;
        }

        const PDFTextBlock& block = blocks[start.blockIndex];
        QPainterPath path = block.getCharacterRangeBoundingPath(start, end, matrix, HEIGHT_INCREASE_FACTOR);

        QColor penColor = item.color.darker();
        QColor brushColor = item.color;
        brushColor.setAlphaF(SELECTION_ALPHA);

        painter->setPen(penColor);
        painter->setBrush(QBrush(brushColor, Qt::SolidPattern));
        painter->drawPath(path);
    }

    painter->restore();
}

QPainterPath PDFTextSelectionPainter::prepareGeometry(PDFInteger pageIndex, PDFTextLayoutGetter& textLayoutGetter, const QTransform& matrix, QPolygonF* quadrilaterals)
{
    QPainterPath path;

    auto it = m_selection->begin(pageIndex);
    auto itEnd = m_selection->end(pageIndex);

    if (it == itEnd)
    {
        // Jakub Melka: no text is selected on current page; do nothing
        return path;
    }

    const PDFTextLayout& layout = textLayoutGetter;
    const PDFTextBlocks& blocks = layout.getTextBlocks();
    for (; it != itEnd; ++it)
    {
        const PDFTextSelectionColoredItem& item = *it;
        const PDFCharacterPointer& start = item.start;
        const PDFCharacterPointer& end = item.end;

        Q_ASSERT(start.pageIndex == end.pageIndex);
        Q_ASSERT(start.blockIndex == end.blockIndex);

        if (start.blockIndex >= blocks.size())
        {
            // Selection is invalid, do nothing
            continue;
        }

        PDFTextBlock block = blocks[start.blockIndex];

        // Fix angle of block, so we will get correct selection rectangles (parallel to lines)
        QTransform angleMatrix;
        angleMatrix.rotate(block.getAngle());
        block.applyTransform(angleMatrix);

        QPainterPath currentPath;
        QPolygonF currentPolygon;

        const size_t lineStart = start.lineIndex;
        const size_t lineEnd = end.lineIndex;
        Q_ASSERT(lineEnd >= lineStart);

        const PDFTextLines& lines = block.getLines();
        for (size_t lineIndex = lineStart; lineIndex <= lineEnd; ++lineIndex)
        {
            if (lineIndex >= lines.size())
            {
                // Selection is invalid, do nothing
                continue;
            }

            const PDFTextLine& line = lines[lineIndex];
            const TextCharacters& characters = line.getCharacters();

            if (characters.empty())
            {
                // Selection is invalid, do nothing
                continue;
            }

            // First determine, which characters will be selected
            size_t characterStart = 0;
            size_t characterEnd = characters.size() - 1;

            if (lineIndex == lineStart)
            {
                characterStart = start.characterIndex;
            }
            if (lineIndex == lineEnd)
            {
                characterEnd = end.characterIndex;
            }

            // Validate indices, then calculate bounding box
            if (!(characterStart <= characterEnd && characterEnd < characters.size()))
            {
                continue;
            }

            QRectF boundingBox;
            for (size_t i = characterStart; i <= characterEnd; ++i)
            {
                boundingBox = boundingBox.united(characters[i].boundingBox.boundingRect());
            }

            if (boundingBox.isValid())
            {
                // Enlarge height by some percent
                PDFReal heightAdvance = boundingBox.height() * HEIGHT_INCREASE_FACTOR * 0.5;
                boundingBox.adjust(0, -heightAdvance, 0, heightAdvance);
                currentPath.addRect(boundingBox);

                if (quadrilaterals)
                {
                    currentPolygon.append({ boundingBox.topLeft(), boundingBox.topRight(), boundingBox.bottomLeft(), boundingBox.bottomRight() });
                }
            }
        }

        QTransform transformMatrix = angleMatrix.inverted() * matrix;
        currentPath = transformMatrix.map(currentPath);

        if (quadrilaterals)
        {
            currentPolygon = transformMatrix.map(currentPolygon);
            quadrilaterals->append(currentPolygon);
        }

        path.addPath(currentPath);
    }

    return path;
}

PDFTextLayoutCache::PDFTextLayoutCache(std::function<PDFTextLayout (PDFInteger)> textLayoutGetter) :
    m_textLayoutGetter(qMove(textLayoutGetter)),
    m_pageIndex(-1),
    m_layout()
{

}

void PDFTextLayoutCache::clear()
{
    m_pageIndex = -1;
    m_layout = PDFTextLayout();
}

const PDFTextLayout& PDFTextLayoutCache::getTextLayout(PDFInteger pageIndex)
{
    if (m_pageIndex != pageIndex)
    {
        m_pageIndex = pageIndex;
        m_layout = m_textLayoutGetter(pageIndex);
    }

    return m_layout;
}

}   // namespace pdf
