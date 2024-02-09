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

#ifndef PDFTEXTLAYOUT_H
#define PDFTEXTLAYOUT_H

#include "pdfglobal.h"
#include "pdfutils.h"

#include <QColor>
#include <QDataStream>
#include <QPainterPath>

#include <set>
#include <compare>

class QMutex;

namespace pdf
{
class PDFTextLayout;
class PDFTextLayoutStorage;
struct PDFCharacterPointer;

struct PDFTextCharacterInfo
{
    /// Character
    QChar character;

    /// Character path
    QPainterPath outline;

    /// Do we use a vertical writing system?
    bool isVerticalWritingSystem = false;

    /// Advance (in character space, it must be translated
    /// into device space), for both vertical/horizontal modes.
    PDFReal advance = 0.0;

    /// Font size (in character space, it must be translated
    /// into device space)
    PDFReal fontSize = 0.0;

    /// Transformation matrix from character space to device space
    QTransform matrix;
};

struct PDFTextLayoutSettings
{
    /// Number of samples for 'docstrum' algorithm, i.e. number of
    /// nearest characters. By default, 5 characters should fit.
    size_t samples = 5;

    /// Distance sensitivity to determine, if characters are close enough.
    /// Maximal distance is computed as current character advance multiplied
    /// by this constant.
    PDFReal distanceSensitivity = 4.0;

    /// Maximal vertical distance, in portion of font size, of two characters
    /// to be considered they lie on same line.
    PDFReal charactersOnLineSensitivity = 0.25;

    /// Maximal ratio between font size of characters to be considered
    /// that they lie on same line.
    PDFReal fontSensitivity = 2.0;

    /// Maximal space ratio between two lines of block. Default coefficient
    /// means, that height ratio limit is (height1 + height2)
    PDFReal blockVerticalSensitivity = 1.5;

    /// Minimal horizontal overlap for two lines considered to be in one block
    PDFReal blockOverlapSensitivity = 0.3;

    friend QDataStream& operator<<(QDataStream& stream, const PDFTextLayoutSettings& settings);
    friend QDataStream& operator>>(QDataStream& stream, PDFTextLayoutSettings& settings);
};

/// Represents character in device space coordinates. All values (dimensions,
/// bounding box, etc. are in device space coordinates).
struct TextCharacter
{
    QChar character;
    QPointF position;
    PDFReal angle = 0.0;
    PDFReal fontSize = 0.0;
    PDFReal advance = 0.0;
    QPainterPath boundingBox;

    size_t index = 0; // Just temporary index, it is not serialized, just for text layout algorithm

    void applyTransform(const QTransform& matrix);

    friend QDataStream& operator<<(QDataStream& stream, const TextCharacter& character);
    friend QDataStream& operator>>(QDataStream& stream, TextCharacter& character);
};

using TextCharacters = std::vector<TextCharacter>;

/// Represents text line consisting of set of characters and line bounding box.
class PDFTextLine
{
public:
    explicit inline PDFTextLine() = default;

    /// Construct new line from characters. Characters are sorted in x-coordinate
    /// and bounding box is computed.
    /// \param characters
    explicit PDFTextLine(TextCharacters characters);

    const TextCharacters& getCharacters() const { return m_characters; }
    const QPainterPath& getBoundingBox() const { return m_boundingBox; }
    const QPointF& getTopLeft() const { return m_topLeft; }

    /// Get angle inclination of block
    PDFReal getAngle() const;

    void applyTransform(const QTransform& matrix);

    friend QDataStream& operator<<(QDataStream& stream, const PDFTextLine& line);
    friend QDataStream& operator>>(QDataStream& stream, PDFTextLine& line);

private:
    TextCharacters m_characters;
    QPainterPath m_boundingBox;
    QPointF m_topLeft;
};

using PDFTextLines = std::vector<PDFTextLine>;

/// Represents text block consisting of set of lines and block bounding box.
class PDFTextBlock
{
public:
    explicit inline PDFTextBlock() = default;
    explicit inline PDFTextBlock(PDFTextLines textLines);

    const PDFTextLines& getLines() const { return m_lines; }
    const QPainterPath& getBoundingBox() const { return m_boundingBox; }
    const QPointF& getTopLeft() const { return m_topLeft; }

    /// Get angle inclination of block
    PDFReal getAngle() const;

    void applyTransform(const QTransform& matrix);

    /// Retrieves the bounding QPainterPath between two specified character positions within this block.
    /// The provided character pointers must point to characters within the current block.
    /// \param start A reference to the PDFCharacterPointer indicating the start character.
    /// \param end A reference to the PDFCharacterPointer indicating the end character.
    /// \param matrix Transformation applied to the path
    /// \param heightIncreaseFactor Height increase factor for characters
    /// \return QPainterPath representing the bounding path between the start and end characters.
    QPainterPath getCharacterRangeBoundingPath(const PDFCharacterPointer& start,
                                               const PDFCharacterPointer& end,
                                               const QTransform& matrix,
                                               PDFReal heightIncreaseFactor) const;

    friend QDataStream& operator<<(QDataStream& stream, const PDFTextBlock& block);
    friend QDataStream& operator>>(QDataStream& stream, PDFTextBlock& block);

private:
    PDFTextLines m_lines;
    QPainterPath m_boundingBox;
    QPointF m_topLeft;
};

using PDFTextBlocks = std::vector<PDFTextBlock>;

/// Character pointer points to some character in text layout.
/// It also has page index to decide, which page the pointer points to.
struct PDFCharacterPointer
{
    auto operator<=>(const PDFCharacterPointer&) const = default;

    /// Returns true, if character pointer is valid and points to the correct location
    bool isValid() const { return pageIndex > -1; }

    /// Returns true, if character belongs to same block
    bool hasSameBlock(const PDFCharacterPointer& other) const;

    /// Returns true, if character belongs to same line
    bool hasSameLine(const PDFCharacterPointer& other) const;

    PDFInteger pageIndex = -1;
    size_t blockIndex = 0;
    size_t lineIndex = 0;
    size_t characterIndex = 0;
};

using PDFTextSelectionItem = std::pair<PDFCharacterPointer, PDFCharacterPointer>;
using PDFTextSelectionItems = std::vector<PDFTextSelectionItem>;

struct PDFTextSelectionColoredItem
{
    explicit inline PDFTextSelectionColoredItem() = default;
    explicit inline PDFTextSelectionColoredItem(PDFCharacterPointer start, PDFCharacterPointer end, QColor color) :
        start(start),
        end(end),
        color(color)
    {

    }

    bool operator==(const PDFTextSelectionColoredItem&) const = default;
    bool operator!=(const PDFTextSelectionColoredItem&) const = default;

    inline bool operator<(const PDFTextSelectionColoredItem& other) const { return std::tie(start, end) < std::tie(other.start, other.end); }

    PDFCharacterPointer start;
    PDFCharacterPointer end;
    QColor color;
};
using PDFTextSelectionColoredItems = std::vector<PDFTextSelectionColoredItem>;

/// Text selection, can be used across multiple pages. Also defines color
/// for each text selection.
class PDF4QTLIBCORESHARED_EXPORT PDFTextSelection
{
public:
    explicit PDFTextSelection() = default;

    using iterator = PDFTextSelectionColoredItems::const_iterator;

    bool operator==(const PDFTextSelection&) const = default;
    bool operator!=(const PDFTextSelection&) const = default;

    /// Adds text selection items to selection
    /// \param items Items
    /// \param color Color for items (must include alpha channel)
    void addItems(const PDFTextSelectionItems& items, QColor color);

    /// Builds text selection, so it is prepared for rendering. Text selection,
    /// which is not build, can't be used for rendering.
    void build();

    /// Returns iterator to start of page range
    iterator begin(PDFInteger pageIndex) const;

    /// Returns iterator to end of page range
    iterator end(PDFInteger pageIndex) const;

    /// Returns iterator to next page range
    iterator nextPageRange(iterator currentPageRange) const;

    /// Returns true, if text selection is empty
    bool isEmpty() const { return m_items.empty(); }

    iterator begin() const { return m_items.cbegin(); }
    iterator end() const { return m_items.cend(); }

private:
    PDFTextSelectionColoredItems m_items;
};

struct PDF4QTLIBCORESHARED_EXPORT PDFFindResult
{
    bool operator<(const PDFFindResult& other) const;

    /// Matched string during search
    QString matched;

    /// Context (text before and after match)
    QString context;

    /// Matched selection (can be multiple items, if selection
    /// is spanned between multiple blocks)
    PDFTextSelectionItems textSelectionItems;
};
using PDFFindResults = std::vector<PDFFindResult>;

class PDFTextFlow;
using PDFTextFlows = std::vector<PDFTextFlow>;

/// This class represents a portion of continuous text on the page. It can
/// consists of multiple blocks (which follow reading order).
class PDF4QTLIBCORESHARED_EXPORT PDFTextFlow
{
public:

    enum FlowFlag
    {
        None                = 0x0000,
        SeparateBlocks      = 0x0001, ///< Create flow for each block
        RemoveSoftHyphen    = 0x0002, ///< Removes 'soft hyphen' unicode character from end-of-line (character 0x00AD)
        AddLineBreaks       = 0x0004, ///< Add line break characters to the end of line
    };
    Q_DECLARE_FLAGS(FlowFlags, FlowFlag)

    /// Finds simple text in current text flow. All text occurences are returned.
    /// \param text Text to be found
    /// \param caseSensitivity Case sensitivity
    PDFFindResults find(const QString& text, Qt::CaseSensitivity caseSensitivity) const;

    /// Finds regular expression matches in current text flow. All text occurences are returned.
    /// \param expression Regular expression to be matched
    PDFFindResults find(const QRegularExpression& expression) const;

    /// Returns whole text for this text flow
    QString getText() const { return m_text; }

    /// Returns character bounding boxes
    std::vector<QRectF> getBoundingBoxes() const { return m_characterBoundingBoxes; }

    /// Returns text form character pointers
    /// \param begin Begin character
    /// \param end End character
    QString getText(const PDFCharacterPointer& begin, const PDFCharacterPointer& end) const;

    /// Merge data from \p next flow (i.e. connect two consecutive flows)
    void merge(const PDFTextFlow& next);

    /// Returns bounding box of a text flow on the page
    QRectF getBoundingBox() const { return m_boundingBox; }

    /// Creates text flows from text layout, according to creation flags.
    /// \param layout Layout, from which is text flow created
    /// \param flags Flow creation flags
    /// \param pageIndex Page index
    static PDFTextFlows createTextFlows(const PDFTextLayout& layout, FlowFlags flags, PDFInteger pageIndex);

private:
    /// Returns text selection from index and length. Returned text selection can also
    /// be empty (for example, if only single space character is selected, which has
    /// no counterpart in real text)
    /// \param index Index of text selection subrange
    /// \param length Length of text selection
    PDFTextSelectionItems getTextSelectionItems(size_t index, size_t length) const;

    /// Returns context for text selection (or empty string, if text selection is empty)
    /// \param index Index of text selection subrange
    /// \param length Length of text selection
    QString getContext(size_t index, size_t length) const;

    QString m_text;
    QRectF m_boundingBox;
    std::vector<PDFCharacterPointer> m_characterPointers;
    std::vector<QRectF> m_characterBoundingBoxes;
};

/// Text layout of single page. Can handle various fonts, various angles of lines
/// and vertically oriented text. It performs the "docstrum" algorithm.
class PDF4QTLIBCORESHARED_EXPORT PDFTextLayout
{
public:
    explicit PDFTextLayout();

    /// Adds character to the layout
    void addCharacter(const PDFTextCharacterInfo& info);

    /// Performs text layout algorithm
    void perform();

    /// Optimizes layout memory allocation to contain less space
    void optimize();

    /// Returns estimate of number of bytes, which this mesh occupies in memory
    qint64 getMemoryConsumptionEstimate() const;

    /// Returns recognized text blocks
    const PDFTextBlocks& getTextBlocks() const { return m_blocks; }

    /// Returns true, if given point is pointing to some text block
    bool isHoveringOverTextBlock(const QPointF& point) const;

    /// Creates text selection. This function needs to modify the layout contents,
    /// so do not use this function from multiple threads (it is not thread-safe).
    /// Text selection is created from rectangle using two points.
    /// \param pageIndex Page index
    /// \param point1 First point
    /// \param point2 Second point
    /// \param selectionColor Selection color
    /// \param strictSelection If true, does not adjust horizontal range when above/below text block
    PDFTextSelection createTextSelection(PDFInteger pageIndex,
                                         const QPointF& point1,
                                         const QPointF& point2,
                                         QColor selectionColor = Qt::yellow,
                                         bool strictSelection = false);

    /// Returns string from text selection
    /// \param itBegin Iterator (begin range)
    /// \param itEnd Iterator (end range)
    /// \param pageIndex Index of the page
    QString getTextFromSelection(PDFTextSelection::iterator itBegin, PDFTextSelection::iterator itEnd, PDFInteger pageIndex) const;

    /// Returns string from text selection
    /// \param selection Text selection
    /// \param pageIndex Index of the page
    QString getTextFromSelection(const PDFTextSelection& selection, PDFInteger pageIndex) const;

    /// Creates text selection for whole block
    /// \param blockIndex Text block index
    /// \param pageIndex pageIndex
    /// \param color Selection color
    PDFTextSelection selectBlock(const size_t blockIndex, PDFInteger pageIndex, QColor color) const;

    /// Creates text selection for signle line of text block
    /// \param blockIndex Text block index
    /// \param lineIndex Line index
    /// \param pageIndex pageIndex
    /// \param color Selection color
    PDFTextSelection selectLineInBlock(const size_t blockIndex, const size_t lineIndex, PDFInteger pageIndex, QColor color) const;

    friend QDataStream& operator<<(QDataStream& stream, const PDFTextLayout& layout);
    friend QDataStream& operator>>(QDataStream& stream, PDFTextLayout& layout);

private:
    /// Makes layout for particular angle
    void performDoLayout(PDFReal angle);

    /// Returns a list of characters for particular angle. Exact match is used
    /// for angle, even if angle is floating point number.
    /// \param angle Angle
    TextCharacters getCharactersForAngle(PDFReal angle) const;

    /// Applies transform to text characters (positions and bounding boxes)
    /// \param characters Characters
    /// \param matrix Transform matrix
    static void applyTransform(TextCharacters& characters, const QTransform& matrix);

    TextCharacters m_characters;
    std::set<PDFReal> m_angles;
    PDFTextLayoutSettings m_settings;
    PDFTextBlocks m_blocks;
};

/// Cache for storing single text layout
class PDF4QTLIBCORESHARED_EXPORT PDFTextLayoutCache
{
public:
    explicit PDFTextLayoutCache(std::function<PDFTextLayout(PDFInteger)> textLayoutGetter);

    /// Clears the cache
    void clear();

    /// Returns text layout. This function always succeeds. If compiler is not active,
    /// then empty layout is returned.
    /// \param compiler Text layout compiler
    /// \param pageIndex Page index
    const PDFTextLayout& getTextLayout(PDFInteger pageIndex);

private:
    std::function<PDFTextLayout(PDFInteger)> m_textLayoutGetter;
    PDFInteger m_pageIndex;
    PDFTextLayout m_layout;
};

class PDF4QTLIBCORESHARED_EXPORT PDFTextLayoutGetter
{
public:
    explicit inline PDFTextLayoutGetter(PDFTextLayoutCache* cache, PDFInteger pageIndex) :
        m_cache(cache),
        m_pageIndex(pageIndex)
    {

    }

    /// Cast operator, casts to constant reference to PDFTextLayout
    operator const PDFTextLayout&()
    {
        return m_cache->getTextLayout(m_pageIndex);
    }

private:
    PDFTextLayoutCache* m_cache;
    PDFInteger m_pageIndex;
};

/// Lazy getter for text layouts from storage. This is used, when we do not want to
/// get text layout each time, because it is time expensive. If text layout is not needed,
/// then nothing happens. Text layout is returned only, if conversion operator is used.
class PDFTextLayoutStorageGetter
{
public:
    explicit PDFTextLayoutStorageGetter(const PDFTextLayoutStorage* storage, PDFInteger pageIndex) :
        m_storage(storage),
        m_pageIndex(pageIndex)
    {

    }

    /// Cast operator, casts to constant reference to PDFTextLayout
    operator const PDFTextLayout&()
    {
        return m_textLayout.get(this, &PDFTextLayoutStorageGetter::getTextLayoutImpl);
    }

private:
    PDFTextLayout getTextLayoutImpl() const;

    const PDFTextLayoutStorage* m_storage;
    PDFInteger m_pageIndex;
    PDFCachedItem<PDFTextLayout> m_textLayout;
};

/// Paints text selection on various pages using page to device point matrix
class PDF4QTLIBCORESHARED_EXPORT PDFTextSelectionPainter
{
public:
    explicit inline PDFTextSelectionPainter(const PDFTextSelection* selection) :
        m_selection(selection)
    {

    }

    /// Draws text selection on the painter, using text layout and matrix. If current text selection
    /// doesn't contain items from active page, then text layout is not accessed.
    /// \param painter Painter
    /// \param pageIndex Page index
    /// \param textLayoutGetter Text layout getter
    /// \param matrix Matrix which translates from page space to device space
    void draw(QPainter* painter, PDFInteger pageIndex, PDFTextLayoutGetter& textLayoutGetter, const QTransform& matrix);

    /// Prepares geometry for text selection drawing, using text layout and matrix.  If current text selection
    /// doesn't contain items from active page, then text layout is not accessed.
    /// \param pageIndex Page index
    /// \param textLayoutGetter Text layout getter
    /// \param matrix Matrix which translates from page space to device space
    QPainterPath prepareGeometry(PDFInteger pageIndex, PDFTextLayoutGetter& textLayoutGetter, const QTransform& matrix, QPolygonF* quadrilaterals);

private:
    static constexpr const PDFReal HEIGHT_INCREASE_FACTOR = 0.40;
    static constexpr const PDFReal SELECTION_ALPHA = 0.25;

    const PDFTextSelection* m_selection;
};

/// Storage for text layouts. For reading and writing, this object is thread safe.
/// For writing, mutex is used to synchronize asynchronous writes, for reading
/// no mutex is used at all. For this reason, both reading/writing at the same time
/// is prohibited, it is not thread safe.
class PDF4QTLIBCORESHARED_EXPORT PDFTextLayoutStorage
{
public:
    explicit inline PDFTextLayoutStorage() = default;
    explicit inline PDFTextLayoutStorage(PDFInteger pageCount) :
        m_offsets(pageCount, 0)
    {

    }

    /// Returns text layout for particular page. If page index is invalid,
    /// then empty text layout is returned. Function is not thread safe, if
    /// function \p setTextLayout is called from another thread.
    /// \param pageIndex Page index
    PDFTextLayout getTextLayout(PDFInteger pageIndex) const;

    /// Returns text layout for particular page. If page index is invalid,
    /// then empty text layout is returned. Function is not thread safe, if
    /// function \p setTextLayout is called from another thread.
    /// \param pageIndex Page index
    PDFTextLayoutStorageGetter getTextLayoutLazy(PDFInteger pageIndex) const { return PDFTextLayoutStorageGetter(this, pageIndex); }

    /// Sets text layout to the particular index. Index must be valid and from
    /// range 0 to \p pageCount - 1. Function is not thread safe.
    /// \param pageIndex Page index
    /// \param layout Text layout
    /// \param mutex Mutex for locking (calls of setTextLayout from multiple threads)
    void setTextLayout(PDFInteger pageIndex, const PDFTextLayout& layout, QMutex* mutex);

    /// Finds simple text in all pages. All text occurences are returned.
    /// \param text Text to be found
    /// \param caseSensitivity Case sensitivity
    /// \param flowFlags Text flow flags
    PDFFindResults find(const QString& text, Qt::CaseSensitivity caseSensitivity, PDFTextFlow::FlowFlags flowFlags) const;

    /// Finds regular expression matches in current text flow. All text occurences are returned.
    /// \param expression Regular expression to be matched
    /// \param flowFlags Text flow flags
    PDFFindResults find(const QRegularExpression& expression, PDFTextFlow::FlowFlags flowFlags) const;

    /// Returns number of pages
    size_t getCount() const { return m_offsets.size(); }

private:
    std::vector<int> m_offsets;
    QByteArray m_textLayouts;
};

}   // namespace pdf

Q_DECLARE_OPERATORS_FOR_FLAGS(pdf::PDFTextFlow::FlowFlags)

#endif // PDFTEXTLAYOUT_H
