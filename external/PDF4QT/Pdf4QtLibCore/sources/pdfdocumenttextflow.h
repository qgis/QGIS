//    Copyright (C) 2020-2021 Jakub Melka
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

#ifndef PDFDOCUMENTTEXTFLOW_H
#define PDFDOCUMENTTEXTFLOW_H

#include "pdfglobal.h"
#include "pdfexception.h"
#include "pdfutils.h"

namespace pdf
{
class PDFDocument;

/// Text flow extracted from document. Text flow can be created \p PDFDocumentTextFlowFactory.
/// Flow can contain various items, not just text ones. Also, some manipulation functions
/// are available, they can modify text flow by various content.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentTextFlow
{
public:

    enum Flag
    {
        None                                = 0x0000,   ///< No text flag
        Text                                = 0x0001,   ///< Ordinary text
        PageStart                           = 0x0002,   ///< Page start marker
        PageEnd                             = 0x0004,   ///< Page end marker
        StructureTitle                      = 0x0008,   ///< Structure tree item title
        StructureLanguage                   = 0x0010,   ///< Structure tree item language
        StructureAlternativeDescription     = 0x0020,   ///< Structure tree item alternative description
        StructureExpandedForm               = 0x0040,   ///< Structure tree item expanded form of text
        StructureActualText                 = 0x0080,   ///< Structure tree item actual text
        StructurePhoneme                    = 0x0100,   ///< Structure tree item  phoneme
        StructureItemStart                  = 0x0200,   ///< Start of structure tree item
        StructureItemEnd                    = 0x0400,   ///< End of structure tree item
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    struct Item
    {
        QRectF boundingRect; ///< Bounding rect in page coordinates
        PDFInteger pageIndex = 0;
        QString text;
        Flags flags = None;
        std::vector<QRectF> characterBoundingRects;

        bool isText() const { return flags.testFlag(Text); }
        bool isSpecial() const { return !isText(); }
        bool isTitle() const { return flags.testFlag(StructureTitle); }
        bool isLanguage() const { return flags.testFlag(StructureLanguage); }
    };
    using Items = std::vector<Item>;

    explicit PDFDocumentTextFlow() = default;
    explicit PDFDocumentTextFlow(Items&& items) :
        m_items(qMove(items))
    {

    }

    /// Add text item
    void addItem(Item item) { m_items.emplace_back(std::move(item)); }

    const Items& getItems() const { return m_items; }

    /// Returns item at a given index
    /// \param index Index
    const Item* getItem(size_t index) const { return &m_items.at(index); }

    /// Returns text flow item count
    size_t getSize() const { return m_items.size(); }

    /// Returns true, if text flow is empty
    bool isEmpty() const { return m_items.empty(); }

    /// Split text flow to pages using given mask. Items, which
    /// are masked out, are not added.
    /// \param mask Mask
    std::map<PDFInteger, PDFDocumentTextFlow> split(Flags mask) const;

    /// Appends document text flow to this one
    /// \param textFlow Text flow
    void append(const PDFDocumentTextFlow& textFlow);

    /// Returns text concantecated from all items
    QString getText() const;

private:
    Items m_items;
};

/// This factory creates text flow for whole document
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentTextFlowFactory
{
public:
    explicit PDFDocumentTextFlowFactory() = default;

    enum class Algorithm
    {
        Auto,       ///< Determine best text layout algorithm automatically
        Layout,     ///< Use text layout recognition using docstrum algorithm
        Content,    ///< Use content-stream text layout recognition (usually unreliable), but fast
        Structure,  ///< Use structure oriented text layout recognition (requires tagged document)
    };

    /// Performs document text flow analysis using given algorithm. Text flow
    /// can be performed only for given subset of pages, if required.
    /// \param document Document
    /// \param pageIndices Analyzed page indices
    /// \param algorithm Algorithm
    PDFDocumentTextFlow create(const PDFDocument* document,
                               const std::vector<PDFInteger>& pageIndices,
                               Algorithm algorithm);

    /// Performs document text flow analysis using given algorithm. Text flow
    /// is created for all pages.
    /// \param document Document
    /// \param algorithm Algorithm
    PDFDocumentTextFlow create(const PDFDocument* document, Algorithm algorithm);

    /// Has some error/warning occured during text layout creation?
    bool hasError() const { return !m_errors.isEmpty(); }

    /// Returns a list of errors/warnings
    const QList<PDFRenderError>& getErrors() const { return m_errors; }

    /// Sets if bounding boxes for text blocks should be calculated
    /// \param calculateBoundingBoxes Perform bounding box calculation?
    void setCalculateBoundingBoxes(bool calculateBoundingBoxes);

private:
    QList<PDFRenderError> m_errors;
    bool m_calculateBoundingBoxes = false;
};

/// Editor which can edit document text flow, modify user text,
/// change order of text items, restore original state of a text flow,
/// and many other features.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentTextFlowEditor
{
public:
    inline PDFDocumentTextFlowEditor() = default;

    /// Sets a text flow and initializes edited text flow
    /// \param textFlow Text flow
    void setTextFlow(PDFDocumentTextFlow textFlow);

    /// Marks selected item as active or inactive
    /// \param active Active
    void setSelectionActive(bool active);

    /// Selects or deselects item
    /// \param index Index
    /// \param select Select (true) or deselect (false)
    void select(size_t index, bool select);

    /// Deselects all selected items
    void deselect();

    void removeItem(size_t index);
    void addItem(size_t index);

    void clear();

    enum EditedItemFlag
    {
        None        = 0x0000,
        Removed     = 0x0001,
        Modified    = 0x0002,
        Selected    = 0x0004
    };
    Q_DECLARE_FLAGS(EditedItemFlags, EditedItemFlag)

    struct EditedItem : public PDFDocumentTextFlow::Item
    {
        size_t originalIndex = 0; ///< Index of original item
        EditedItemFlags editedItemFlags = None;
    };

    using EditedItems = std::vector<EditedItem>;
    using PageIndicesMapping = std::vector<std::pair<PDFInteger, size_t>>;
    using PageIndicesMappingIterator = PageIndicesMapping::const_iterator;
    using PageIndicesMappingRange = std::pair<PageIndicesMappingIterator, PageIndicesMappingIterator>;

    /// Returns true, if item is active
    /// \param index Index
    bool isActive(size_t index) const { return !getEditedItem(index)->editedItemFlags.testFlag(Removed); }

    /// Returns true, if item is removed
    /// \param index Index
    bool isRemoved(size_t index) const { return !isActive(index); }

    /// Returns true, if item is modified
    /// \param index Index
    bool isModified(size_t index) const { return getEditedItem(index)->editedItemFlags.testFlag(Modified); }

    /// Returns true, if item is selected
    /// \param index Index
    bool isSelected(size_t index) const { return getEditedItem(index)->editedItemFlags.testFlag(Selected); }

    /// Returns edited text (or original, if edited text is not modified)
    /// for a given index.
    /// \param index Index
    const QString& getText(size_t index) const { return getEditedItem(index)->text; }

    /// Sets edited text for a given index
    void setText(const QString& text, size_t index);

    /// Returns true, if text flow is empty
    bool isEmpty() const { return m_originalTextFlow.isEmpty(); }

    /// Returns true, if text selection is empty
    bool isSelectionEmpty() const;

    /// Returns true, if selection contains modified items
    bool isSelectionModified() const;

    /// Returns item count in edited text flow
    size_t getItemCount() const { return m_editedTextFlow.size(); }

    /// Returns page index for given item
    /// \param index Index
    PDFInteger getPageIndex(size_t index) const { return getEditedItem(index)->pageIndex; }

    bool isItemTypeText(size_t index) const { return getEditedItem(index)->isText(); }
    bool isItemTypeSpecial(size_t index) const { return getEditedItem(index)->isSpecial(); }
    bool isItemTypeTitle(size_t index) const { return getEditedItem(index)->isTitle(); }
    bool isItemTypeLanguage(size_t index) const { return getEditedItem(index)->isLanguage(); }

    /// Selects items contained in a rectangle
    /// \param rectangle Selection rectangle
    void selectByRectangle(QRectF rectangle);

    /// Select items which contains text
    /// \param text Text
    void selectByContainedText(QString text);

    /// Select items which matches regular expression
    /// \param expression Regular expression
    void selectByRegularExpression(const QRegularExpression& expression);

    /// Select all items on a given page indices
    /// \param indices Indices
    void selectByPageIndices(const PDFClosedIntervalSet& indices);

    /// Restores original texts in selected items
    void restoreOriginalTexts();

    /// Move all selected items one position up. If multiple non-consecutive
    /// items are selected, they are grouped into one group and move one item
    /// up above first selected item.
    void moveSelectionUp();

    /// Move all selected items one position down. If multiple non-consecutive
    /// items are selected, they are grouped into one group and move one item
    /// down above first selected item.
    void moveSelectionDown();

    /// Returns item indices for a given page index, i.e.
    /// index of items which are lying on a page.
    /// \param pageIndex Page index
    PageIndicesMappingRange getItemsForPageIndex(PDFInteger pageIndex) const;

    const EditedItem* getEditedItem(size_t index) const { return &m_editedTextFlow.at(index); }

    /// Creates text flow from active edited items. If item is removed,
    /// then it is not added into this text flow. User text modification
    /// is applied to a text flow.
    PDFDocumentTextFlow createEditedTextFlow() const;

private:
    void createPageMapping();
    void createEditedFromOriginalTextFlow();
    void updateModifiedFlag(size_t index);

    const PDFDocumentTextFlow::Item* getOriginalItem(size_t index) const { return m_originalTextFlow.getItem(index); }
    EditedItem* getEditedItem(size_t index) { return &m_editedTextFlow.at(index); }

    PDFDocumentTextFlow m_originalTextFlow;
    EditedItems m_editedTextFlow;
    PageIndicesMapping m_pageIndicesMapping;
};

}   // namespace pdf

#endif // PDFDOCUMENTTEXTFLOW_H
