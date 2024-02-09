//    Copyright (C) 2020-2022 Jakub Melka
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

#include "pdfdocumenttextflow.h"
#include "pdfdocument.h"
#include "pdfstructuretree.h"
#include "pdfexecutionpolicy.h"
#include "pdfconstants.h"
#include "pdfcms.h"
#include "pdftextlayoutgenerator.h"
#include "pdfpagecontentprocessor.h"
#include "pdfdbgheap.h"

namespace pdf
{

class PDFStructureTreeReferenceCollector : public PDFStructureTreeAbstractVisitor
{
public:
    explicit inline PDFStructureTreeReferenceCollector(std::map<PDFObjectReference, const PDFStructureItem*>* mapping) :
        m_mapping(mapping)
    {

    }

    virtual void visitStructureTree(const PDFStructureTree* structureTree) override;
    virtual void visitStructureElement(const PDFStructureElement* structureElement) override;
    virtual void visitStructureMarkedContentReference(const PDFStructureMarkedContentReference* structureMarkedContentReference) override;
    virtual void visitStructureObjectReference(const PDFStructureObjectReference* structureObjectReference) override;

private:
    void addReference(const PDFStructureItem* structureObjectReference);

    std::map<PDFObjectReference, const PDFStructureItem*>* m_mapping;
};

void PDFStructureTreeReferenceCollector::visitStructureTree(const PDFStructureTree* structureTree)
{
    addReference(structureTree);
    acceptChildren(structureTree);
}

void PDFStructureTreeReferenceCollector::visitStructureElement(const PDFStructureElement* structureElement)
{
    addReference(structureElement);
    acceptChildren(structureElement);
}

void PDFStructureTreeReferenceCollector::visitStructureMarkedContentReference(const PDFStructureMarkedContentReference* structureMarkedContentReference)
{
    addReference(structureMarkedContentReference);
    acceptChildren(structureMarkedContentReference);
}

void PDFStructureTreeReferenceCollector::visitStructureObjectReference(const PDFStructureObjectReference* structureObjectReference)
{
    addReference(structureObjectReference);
    acceptChildren(structureObjectReference);
}

void PDFStructureTreeReferenceCollector::addReference(const PDFStructureItem* structureItem)
{
    if (structureItem->getSelfReference().isValid())
    {
        (*m_mapping)[structureItem->getSelfReference()] = structureItem;
    }
}

struct PDFStructureTreeTextItem
{
    enum class Type
    {
        StartTag,
        EndTag,
        Text
    };

    PDFStructureTreeTextItem() = default;
    PDFStructureTreeTextItem(Type type, const PDFStructureItem* item, QString text, PDFInteger pageIndex, QRectF boundingRect, std::vector<QRectF> characterBoundingRects) :
        type(type), item(item), text(qMove(text)), pageIndex(pageIndex), boundingRect(boundingRect), characterBoundingRects(std::move(characterBoundingRects))
    {

    }

    static PDFStructureTreeTextItem createText(QString text, PDFInteger pageIndex, QRectF boundingRect, std::vector<QRectF> characterBoundingRects) { return PDFStructureTreeTextItem(Type::Text, nullptr, qMove(text), pageIndex, boundingRect, std::move(characterBoundingRects)); }
    static PDFStructureTreeTextItem createStartTag(const PDFStructureItem* item) { return PDFStructureTreeTextItem(Type::StartTag, item, QString(), -1, QRectF(), { }); }
    static PDFStructureTreeTextItem createEndTag(const PDFStructureItem* item) { return PDFStructureTreeTextItem(Type::EndTag, item, QString(), -1, QRectF(), { }); }

    Type type = Type::Text;
    const PDFStructureItem* item = nullptr;
    QString text;
    PDFInteger pageIndex = -1;
    QRectF boundingRect;
    std::vector<QRectF> characterBoundingRects;
};

using PDFStructureTreeTextSequence = std::vector<PDFStructureTreeTextItem>;

/// Text extractor for structure tree. Extracts sequences of structure items,
/// page sequences are stored in \p textSequences. They can be accessed using
/// getters.
class PDFStructureTreeTextExtractor
{
public:
    enum Option
    {
        None                = 0x0000,
        SkipArtifact        = 0x0001,   ///< Skip content marked as 'Artifact'
        AdjustReversedText  = 0x0002,   ///< Adjust reversed text
        CreateTreeMapping   = 0x0004,   ///< Create text mapping to structure tree item
        BoundingBoxes       = 0x0008,   ///< Compute bounding boxes of the texts
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit PDFStructureTreeTextExtractor(const PDFDocument* document, const PDFStructureTree* tree, Options options);

    /// Performs text extracting algorithm. Only \p pageIndices
    /// pages are processed for text extraction.
    /// \param pageIndices Page indices
    void perform(const std::vector<PDFInteger>& pageIndices);

    /// Returns a list of errors/warnings
    const QList<PDFRenderError>& getErrors() const { return m_errors; }

    /// Returns a list of unmatched text
    const QStringList& getUnmatchedText() const { return m_unmatchedText; }

    /// Returns text sequence for given page. If page number is invalid,
    /// then empty text sequence is returned.
    /// \param pageNumber Page number
    const PDFStructureTreeTextSequence& getTextSequence(PDFInteger pageNumber) const;

    struct TextItem
    {
        QRectF boundingRect;
        PDFInteger pageIndex = -1;
        QString text;
        std::vector<QRectF> characterBoundingRects;
    };

    using TextItems = std::vector<TextItem>;

    /// Returns text for given structure tree item. If structure tree item
    /// is not found, then empty list is returned. This functionality
    /// requires, that \p CreateTreeMapping flag is being set.
    /// \param item Item
    const TextItems& getText(const PDFStructureItem* item) const;

private:
    QList<PDFRenderError> m_errors;
    const PDFDocument* m_document;
    const PDFStructureTree* m_tree;
    QStringList m_unmatchedText;
    std::map<PDFInteger, PDFStructureTreeTextSequence> m_textSequences;
    std::map<const PDFStructureItem*, TextItems> m_textForItems;
    Options m_options;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PDFStructureTreeTextExtractor::Options)

class PDFStructureTreeTextContentProcessor : public PDFPageContentProcessor
{
    using BaseClass = PDFPageContentProcessor;

public:
    explicit PDFStructureTreeTextContentProcessor(PDFRenderer::Features features,
                                                  const PDFPage* page,
                                                  const PDFDocument* document,
                                                  const PDFFontCache* fontCache,
                                                  const PDFCMS* cms,
                                                  const PDFOptionalContentActivity* optionalContentActivity,
                                                  QTransform pagePointToDevicePointMatrix,
                                                  const PDFMeshQualitySettings& meshQualitySettings,
                                                  const PDFStructureTree* tree,
                                                  const std::map<PDFObjectReference, const PDFStructureItem*>* mapping,
                                                  PDFStructureTreeTextExtractor::Options extractorOptions) :
        BaseClass(page, document, fontCache, cms, optionalContentActivity, pagePointToDevicePointMatrix, meshQualitySettings),
        m_features(features),
        m_tree(tree),
        m_mapping(mapping),
        m_extractorOptions(extractorOptions),
        m_pageIndex(document->getCatalog()->getPageIndexFromPageReference(page->getPageReference()))
    {

    }

    PDFStructureTreeTextSequence& takeSequence() { return m_textSequence; }
    QStringList& takeUnmatchedTexts() { return m_unmatchedText; }

protected:
    virtual bool isContentSuppressedByOC(PDFObjectReference ocgOrOcmd) override;
    virtual bool isContentKindSuppressed(ContentKind kind) const override;
    virtual void performOutputCharacter(const PDFTextCharacterInfo& info) override;
    virtual void performMarkedContentBegin(const QByteArray& tag, const PDFObject& properties) override;
    virtual void performMarkedContentEnd() override;

private:
    const PDFStructureItem* getStructureTreeItemFromMCID(PDFInteger mcid) const;
    void finishText();

    bool isArtifact() const;
    bool isReversedText() const;

    struct MarkedContentInfo
    {
        QByteArray tag;
        PDFInteger mcid = -1;
        const PDFStructureItem* structureTreeItem = nullptr;
        bool isArtifact = false;
        bool isReversedText = false;
    };

    PDFRenderer::Features m_features;
    const PDFStructureTree* m_tree;
    const std::map<PDFObjectReference, const PDFStructureItem*>* m_mapping;
    std::vector<MarkedContentInfo> m_markedContentInfoStack;
    QString m_currentText;
    QRectF m_currentBoundingBox;
    PDFStructureTreeTextSequence m_textSequence;
    QStringList m_unmatchedText;
    PDFStructureTreeTextExtractor::Options m_extractorOptions;
    PDFInteger m_pageIndex;
    std::vector<QRectF> m_characterBoundingRects;
};

void PDFStructureTreeTextContentProcessor::finishText()
{
    QString trimmedText = m_currentText.trimmed();
    const int index = m_currentText.indexOf(trimmedText);
    Q_ASSERT(index != -1);
    if (trimmedText.size() < m_currentText.size())
    {
        // Fix character bounding boxes...
        if (m_characterBoundingRects.size() == static_cast<size_t>(m_currentText.size()))
        {
            std::vector<QRectF> boundingRects(std::next(m_characterBoundingRects.cbegin(), index), std::next(m_characterBoundingRects.cbegin(), index + trimmedText.length()));
            m_characterBoundingRects = std::move(boundingRects);
        }
        m_currentText = std::move(trimmedText);
    }

    if (!m_currentText.isEmpty() && (!m_extractorOptions.testFlag(PDFStructureTreeTextExtractor::SkipArtifact) || !isArtifact()))
    {
        if (m_extractorOptions.testFlag(PDFStructureTreeTextExtractor::AdjustReversedText) && isReversedText())
        {
            QString reversed;
            reversed.reserve(m_currentText.size());
            for (auto it = m_currentText.rbegin(); it != m_currentText.rend(); ++it)
            {
                reversed.push_back(*it);
            }
            m_currentText = qMove(reversed);
            std::reverse(m_characterBoundingRects.begin(), m_characterBoundingRects.end());
        }
        Q_ASSERT(static_cast<size_t>(m_currentText.size()) == m_characterBoundingRects.size() || m_characterBoundingRects.empty());
        m_textSequence.emplace_back(PDFStructureTreeTextItem::createText(std::move(m_currentText), m_pageIndex, m_currentBoundingBox, std::move(m_characterBoundingRects)));
    }
    m_currentText = QString();
    m_currentBoundingBox = QRectF();
    m_characterBoundingRects.clear();
}

bool PDFStructureTreeTextContentProcessor::isArtifact() const
{
    return std::any_of(m_markedContentInfoStack.cbegin(), m_markedContentInfoStack.cend(), [](const auto& item) { return item.isArtifact; });
}

bool PDFStructureTreeTextContentProcessor::isReversedText() const
{
    return std::any_of(m_markedContentInfoStack.cbegin(), m_markedContentInfoStack.cend(), [](const auto& item) { return item.isReversedText; });
}

void PDFStructureTreeTextContentProcessor::performMarkedContentBegin(const QByteArray& tag, const PDFObject& properties)
{
    MarkedContentInfo info;
    info.tag = tag;

    if (properties.isDictionary())
    {
        const PDFDictionary* dictionary = properties.getDictionary();
        PDFObject mcid = dictionary->get("MCID");
        if (mcid.isInt())
        {
            // We must finish text, because we can have a sequence of text,
            // then subitem, then text, and followed by another subitem. They
            // can be interleaved.
            finishText();

            info.mcid = mcid.getInteger();
            info.structureTreeItem = getStructureTreeItemFromMCID(info.mcid);
            info.isArtifact = tag == "Artifact";
            info.isReversedText = tag == "ReversedChars";

            if (!info.structureTreeItem)
            {
                reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Structure tree item for MCID %1 not found.").arg(info.mcid));
            }

            if (info.structureTreeItem)
            {
                m_textSequence.emplace_back(PDFStructureTreeTextItem::createStartTag(info.structureTreeItem));
            }
        }
    }

    m_markedContentInfoStack.emplace_back(qMove(info));
}

void PDFStructureTreeTextContentProcessor::performMarkedContentEnd()
{
    MarkedContentInfo info = qMove(m_markedContentInfoStack.back());
    m_markedContentInfoStack.pop_back();

    if (info.mcid != -1)
    {
        finishText();
        if (info.structureTreeItem)
        {
            m_textSequence.emplace_back(PDFStructureTreeTextItem::createEndTag(info.structureTreeItem));
        }
    }

    // Check for text, which doesn't belong to any structure tree item
    if (m_markedContentInfoStack.empty())
    {
        m_currentText = m_currentText.trimmed();
        if (!m_currentText.isEmpty())
        {
            m_unmatchedText << qMove(m_currentText);
        }
        m_currentBoundingBox = QRectF();
        m_characterBoundingRects.clear();
    }
}

const PDFStructureItem* PDFStructureTreeTextContentProcessor::getStructureTreeItemFromMCID(PDFInteger mcid) const
{
    auto it = m_mapping->find(m_tree->getParent(getStructuralParentKey(), mcid));
    if (it != m_mapping->cend())
    {
        return it->second;
    }
    return nullptr;
}

bool PDFStructureTreeTextContentProcessor::isContentSuppressedByOC(PDFObjectReference ocgOrOcmd)
{
    if (m_features.testFlag(PDFRenderer::IgnoreOptionalContent))
    {
        return false;
    }

    return PDFPageContentProcessor::isContentSuppressedByOC(ocgOrOcmd);
}

bool PDFStructureTreeTextContentProcessor::isContentKindSuppressed(ContentKind kind) const
{
    switch (kind)
    {
        case ContentKind::Text:
        case ContentKind::Shapes:
        case ContentKind::Images:
        case ContentKind::Shading:
            return true;

        case ContentKind::Tiling:
            return false; // Tiling can have text

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return false;
}

void PDFStructureTreeTextContentProcessor::performOutputCharacter(const PDFTextCharacterInfo& info)
{
    if (!isContentSuppressed())
    {
        if (!info.character.isNull() && info.character != QChar(QChar::SoftHyphen))
        {
            m_currentText.push_back(info.character);

            QPainterPath worldPath = info.matrix.map(info.outline);
            if (!worldPath.isEmpty())
            {
                QRectF boundingRect = worldPath.controlPointRect();
                m_currentBoundingBox = m_currentBoundingBox.united(boundingRect);
                m_characterBoundingRects.push_back(boundingRect);
            }
            else
            {
                m_characterBoundingRects.push_back(QRectF());
            }
        }
    }
}

PDFStructureTreeTextExtractor::PDFStructureTreeTextExtractor(const PDFDocument* document, const PDFStructureTree* tree, Options options) :
    m_document(document),
    m_tree(tree),
    m_options(options)
{

}

void PDFStructureTreeTextExtractor::perform(const std::vector<PDFInteger>& pageIndices)
{
    std::map<PDFObjectReference, const PDFStructureItem*> mapping;
    PDFStructureTreeReferenceCollector referenceCollector(&mapping);
    m_tree->accept(&referenceCollector);

    PDFFontCache fontCache(DEFAULT_FONT_CACHE_LIMIT, DEFAULT_REALIZED_FONT_CACHE_LIMIT);

    QMutex mutex;
    PDFCMSGeneric cms;
    PDFMeshQualitySettings mqs;
    PDFOptionalContentActivity oca(m_document, OCUsage::Export, nullptr);
    pdf::PDFModifiedDocument md(const_cast<PDFDocument*>(m_document), &oca);
    fontCache.setDocument(md);
    fontCache.setCacheShrinkEnabled(nullptr, false);

    auto generateTextLayout = [&, this](PDFInteger pageIndex)
    {
        const PDFCatalog* catalog = m_document->getCatalog();
        if (!catalog->getPage(pageIndex))
        {
            // Invalid page index
            return;
        }

        const PDFPage* page = catalog->getPage(pageIndex);
        Q_ASSERT(page);

        PDFStructureTreeTextContentProcessor processor(PDFRenderer::IgnoreOptionalContent, page, m_document, &fontCache, &cms, &oca, QTransform(), mqs, m_tree, &mapping, m_options);
        QList<PDFRenderError> errors = processor.processContents();

        QMutexLocker lock(&mutex);
        m_textSequences[pageIndex] = qMove(processor.takeSequence());
        m_unmatchedText << qMove(processor.takeUnmatchedTexts());
        m_errors.append(qMove(errors));
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, pageIndices.begin(), pageIndices.end(), generateTextLayout);

    fontCache.setCacheShrinkEnabled(nullptr, true);

    if (m_options.testFlag(CreateTreeMapping))
    {
        for (const auto& sequence : m_textSequences)
        {
            std::stack<const PDFStructureItem*> stack;
            for (const PDFStructureTreeTextItem& sequenceItem : sequence.second)
            {
                switch (sequenceItem.type)
                {
                    case PDFStructureTreeTextItem::Type::StartTag:
                    {
                        stack.push(sequenceItem.item);
                        break;
                    }
                    case PDFStructureTreeTextItem::Type::EndTag:
                    {
                        stack.pop();
                        break;
                    }
                    case PDFStructureTreeTextItem::Type::Text:
                    {
                        if (!stack.empty())
                        {
                            m_textForItems[stack.top()].emplace_back(TextItem{ sequenceItem.boundingRect, sequenceItem.pageIndex, sequenceItem.text, sequenceItem.characterBoundingRects });
                        }
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }
}

const PDFStructureTreeTextSequence& PDFStructureTreeTextExtractor::getTextSequence(PDFInteger pageIndex) const
{
    auto it = m_textSequences.find(pageIndex);
    if (it != m_textSequences.cend())
    {
        return it->second;
    }

    static PDFStructureTreeTextSequence dummy;
    return dummy;
}

const PDFStructureTreeTextExtractor::TextItems& PDFStructureTreeTextExtractor::getText(const PDFStructureItem* item) const
{
    auto it = m_textForItems.find(item);
    if (it != m_textForItems.cend())
    {
        return it->second;
    }

    static const TextItems dummy;
    return dummy;
}


class PDFStructureTreeTextFlowCollector : public PDFStructureTreeAbstractVisitor
{
public:
    explicit PDFStructureTreeTextFlowCollector(PDFDocumentTextFlow::Items* items, const PDFStructureTreeTextExtractor* extractor) :
        m_items(items),
        m_extractor(extractor)
    {

    }

    virtual void visitStructureTree(const PDFStructureTree* structureTree) override;
    virtual void visitStructureElement(const PDFStructureElement* structureElement) override;
    virtual void visitStructureMarkedContentReference(const PDFStructureMarkedContentReference* structureMarkedContentReference) override;
    virtual void visitStructureObjectReference(const PDFStructureObjectReference* structureObjectReference) override;

private:
    void markHasContent();

    PDFDocumentTextFlow::Items* m_items;
    const PDFStructureTreeTextExtractor* m_extractor;
    std::vector<bool> m_hasContentStack;
};

void PDFStructureTreeTextFlowCollector::visitStructureTree(const PDFStructureTree* structureTree)
{
    m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, QString(), PDFDocumentTextFlow::StructureItemStart, {} });
    acceptChildren(structureTree);
    m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, QString(), PDFDocumentTextFlow::StructureItemEnd, {} });
}

void PDFStructureTreeTextFlowCollector::markHasContent()
{
    for (size_t i = 0; i < m_hasContentStack.size(); ++i)
    {
        m_hasContentStack[i] = true;
    }
}

void PDFStructureTreeTextFlowCollector::visitStructureElement(const PDFStructureElement* structureElement)
{
    size_t index = m_items->size();
    m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, QString(), PDFDocumentTextFlow::StructureItemStart, {} });

    // Mark stack so we can delete unused items
    m_hasContentStack.push_back(false);

    QString title = structureElement->getText(PDFStructureElement::Title);
    QString language = structureElement->getText(PDFStructureElement::Language);
    QString alternativeDescription = structureElement->getText(PDFStructureElement::AlternativeDescription);
    QString expandedForm = structureElement->getText(PDFStructureElement::ExpandedForm);
    QString actualText = structureElement->getText(PDFStructureElement::ActualText);
    QString phoneme = structureElement->getText(PDFStructureElement::Phoneme);

    if (!title.isEmpty())
    {
        markHasContent();
        m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, QString(), PDFDocumentTextFlow::StructureTitle, {} });
    }

    if (!language.isEmpty())
    {
        markHasContent();
        m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, language, PDFDocumentTextFlow::StructureLanguage, {} });
    }

    if (!alternativeDescription.isEmpty())
    {
        markHasContent();
        m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, alternativeDescription, PDFDocumentTextFlow::StructureAlternativeDescription, {} });
    }

    if (!expandedForm.isEmpty())
    {
        markHasContent();
        m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, expandedForm, PDFDocumentTextFlow::StructureExpandedForm, {} });
    }

    if (!actualText.isEmpty())
    {
        markHasContent();
        m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, actualText, PDFDocumentTextFlow::StructureActualText, {} });
    }

    if (!phoneme.isEmpty())
    {
        markHasContent();
        m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, phoneme, PDFDocumentTextFlow::StructurePhoneme, {} });
    }

    for (const auto& textItem : m_extractor->getText(structureElement))
    {
        markHasContent();
        m_items->push_back(PDFDocumentTextFlow::Item{ textItem.boundingRect, textItem.pageIndex, textItem.text, PDFDocumentTextFlow::Text, textItem.characterBoundingRects });
    }

    acceptChildren(structureElement);

    const bool hasContent = m_hasContentStack.back();
    m_hasContentStack.pop_back();

    m_items->push_back(PDFDocumentTextFlow::Item{ QRectF(), -1, QString(), PDFDocumentTextFlow::StructureItemEnd, {} });

    if (!hasContent)
    {
        // Delete unused content
        m_items->erase(std::next(m_items->begin(), index), m_items->end());
    }
}

void PDFStructureTreeTextFlowCollector::visitStructureMarkedContentReference(const PDFStructureMarkedContentReference* structureMarkedContentReference)
{
    acceptChildren(structureMarkedContentReference);
}

void PDFStructureTreeTextFlowCollector::visitStructureObjectReference(const PDFStructureObjectReference* structureObjectReference)
{
    acceptChildren(structureObjectReference);
}

PDFDocumentTextFlow PDFDocumentTextFlowFactory::create(const PDFDocument* document, const std::vector<PDFInteger>& pageIndices, Algorithm algorithm)
{
    PDFDocumentTextFlow result;
    PDFStructureTree structureTree;

    const PDFCatalog* catalog = document->getCatalog();
    if (algorithm != Algorithm::Layout)
    {
        structureTree = PDFStructureTree::parse(&document->getStorage(), catalog->getStructureTreeRoot());
    }

    if (algorithm == Algorithm::Auto)
    {
        // Determine algorithm
        if (catalog->isLogicalStructureMarked() && structureTree.isValid())
        {
            algorithm = Algorithm::Structure;
        }
        else
        {
            algorithm = Algorithm::Layout;
        }
    }

    Q_ASSERT(algorithm != Algorithm::Auto);

    // Perform algorithm to retrieve document text
    switch (algorithm)
    {
        case Algorithm::Layout:
        {
            PDFFontCache fontCache(DEFAULT_FONT_CACHE_LIMIT, DEFAULT_REALIZED_FONT_CACHE_LIMIT);

            std::map<PDFInteger, PDFDocumentTextFlow::Items> items;

            QMutex mutex;
            PDFCMSGeneric cms;
            PDFMeshQualitySettings mqs;
            PDFOptionalContentActivity oca(document, OCUsage::Export, nullptr);
            pdf::PDFModifiedDocument md(const_cast<PDFDocument*>(document), &oca);
            fontCache.setDocument(md);
            fontCache.setCacheShrinkEnabled(nullptr, false);

            auto generateTextLayout = [this, &items, &mutex, &fontCache, &cms, &mqs, &oca, document, catalog](PDFInteger pageIndex)
            {
                if (!catalog->getPage(pageIndex))
                {
                    // Invalid page index
                    return;
                }

                const PDFPage* page = catalog->getPage(pageIndex);
                Q_ASSERT(page);

                PDFTextLayoutGenerator generator(PDFRenderer::IgnoreOptionalContent, page, document, &fontCache, &cms, &oca, QTransform(), mqs);
                QList<PDFRenderError> errors = generator.processContents();
                PDFTextLayout textLayout = generator.createTextLayout();
                PDFTextFlows textFlows = PDFTextFlow::createTextFlows(textLayout, PDFTextFlow::FlowFlags(PDFTextFlow::SeparateBlocks) | PDFTextFlow::RemoveSoftHyphen, pageIndex);

                PDFDocumentTextFlow::Items flowItems;
                flowItems.emplace_back(PDFDocumentTextFlow::Item{ QRectF(), pageIndex, PDFTranslationContext::tr("Page %1").arg(pageIndex + 1), PDFDocumentTextFlow::PageStart, {} });
                for (const PDFTextFlow& textFlow : textFlows)
                {
                    flowItems.emplace_back(PDFDocumentTextFlow::Item{ textFlow.getBoundingBox(), pageIndex, textFlow.getText(), PDFDocumentTextFlow::Text, textFlow.getBoundingBoxes() });
                }
                flowItems.emplace_back(PDFDocumentTextFlow::Item{ QRectF(), pageIndex, QString(), PDFDocumentTextFlow::PageEnd, {} });

                QMutexLocker lock(&mutex);
                items[pageIndex] = qMove(flowItems);
                m_errors.append(qMove(errors));
            };

            PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, pageIndices.begin(), pageIndices.end(), generateTextLayout);

            fontCache.setCacheShrinkEnabled(nullptr, true);

            PDFDocumentTextFlow::Items flowItems;
            for (const auto& item : items)
            {
                flowItems.insert(flowItems.end(), std::make_move_iterator(item.second.begin()), std::make_move_iterator(item.second.end()));
            }

            result = PDFDocumentTextFlow(qMove(flowItems));
            break;
        }

        case Algorithm::Structure:
        {
            if (!structureTree.isValid())
            {
                m_errors << PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Valid tagged document required."));
                break;
            }

            PDFStructureTreeTextExtractor::Options options = PDFStructureTreeTextExtractor::SkipArtifact | PDFStructureTreeTextExtractor::AdjustReversedText | PDFStructureTreeTextExtractor::CreateTreeMapping;
            options.setFlag(PDFStructureTreeTextExtractor::BoundingBoxes, m_calculateBoundingBoxes);
            PDFStructureTreeTextExtractor extractor(document, &structureTree, options);
            extractor.perform(pageIndices);

            PDFDocumentTextFlow::Items flowItems;
            PDFStructureTreeTextFlowCollector collector(&flowItems, &extractor);
            structureTree.accept(&collector);

            result = PDFDocumentTextFlow(qMove(flowItems));
            m_errors.append(extractor.getErrors());
            break;
        }

        case Algorithm::Content:
        {
            PDFStructureTreeTextExtractor::Options options = PDFStructureTreeTextExtractor::None;
            options.setFlag(PDFStructureTreeTextExtractor::BoundingBoxes, m_calculateBoundingBoxes);
            PDFStructureTreeTextExtractor extractor(document, &structureTree, options);
            extractor.perform(pageIndices);

            PDFDocumentTextFlow::Items flowItems;
            for (PDFInteger pageIndex : pageIndices)
            {
                flowItems.emplace_back(PDFDocumentTextFlow::Item{ QRectF(), pageIndex, PDFTranslationContext::tr("Page %1").arg(pageIndex + 1), PDFDocumentTextFlow::PageStart, {} });
                for (const PDFStructureTreeTextItem& sequenceItem : extractor.getTextSequence(pageIndex))
                {
                    if (sequenceItem.type == PDFStructureTreeTextItem::Type::Text)
                    {
                        flowItems.emplace_back(PDFDocumentTextFlow::Item{ sequenceItem.boundingRect, pageIndex, sequenceItem.text, PDFDocumentTextFlow::Text, sequenceItem.characterBoundingRects });
                    }
                }
                flowItems.emplace_back(PDFDocumentTextFlow::Item{ QRectF(), pageIndex, QString(), PDFDocumentTextFlow::PageEnd, {} });
            }

            result = PDFDocumentTextFlow(qMove(flowItems));
            m_errors.append(extractor.getErrors());
            break;
        }

        default:
            Q_ASSERT(false);
            break;
    }

    return result;
}

PDFDocumentTextFlow PDFDocumentTextFlowFactory::create(const PDFDocument* document, Algorithm algorithm)
{
    std::vector<pdf::PDFInteger> pageIndices;
    pageIndices.resize(document->getCatalog()->getPageCount(), 0);
    std::iota(pageIndices.begin(), pageIndices.end(), 0);

    return create(document, pageIndices, algorithm);
}

void PDFDocumentTextFlowFactory::setCalculateBoundingBoxes(bool calculateBoundingBoxes)
{
    m_calculateBoundingBoxes = calculateBoundingBoxes;
}

void PDFDocumentTextFlowEditor::setTextFlow(PDFDocumentTextFlow textFlow)
{
    m_originalTextFlow = std::move(textFlow);
    createEditedFromOriginalTextFlow();
}

void PDFDocumentTextFlowEditor::setSelectionActive(bool active)
{
    for (auto& item : m_editedTextFlow)
    {
        if (item.editedItemFlags.testFlag(Selected))
        {
            item.editedItemFlags.setFlag(Removed, !active);
        }
    }
}

void PDFDocumentTextFlowEditor::select(size_t index, bool select)
{
    getEditedItem(index)->editedItemFlags.setFlag(Selected, select);
}

void PDFDocumentTextFlowEditor::deselect()
{
    for (auto& item : m_editedTextFlow)
    {
        item.editedItemFlags.setFlag(Selected, false);
    }
}

void PDFDocumentTextFlowEditor::removeItem(size_t index)
{
    getEditedItem(index)->editedItemFlags.setFlag(Removed, true);
}

void PDFDocumentTextFlowEditor::addItem(size_t index)
{
    getEditedItem(index)->editedItemFlags.setFlag(Removed, false);
}

void PDFDocumentTextFlowEditor::clear()
{
    m_originalTextFlow = PDFDocumentTextFlow();
    m_editedTextFlow.clear();
    m_pageIndicesMapping.clear();
}

void PDFDocumentTextFlowEditor::setText(const QString& text, size_t index)
{
    EditedItem* item = getEditedItem(index);
    item->text = text;
    updateModifiedFlag(index);
}

bool PDFDocumentTextFlowEditor::isSelectionEmpty() const
{
    return std::all_of(m_editedTextFlow.cbegin(), m_editedTextFlow.cend(), [](const auto& item) { return !item.editedItemFlags.testFlag(Selected); });
}

bool PDFDocumentTextFlowEditor::isSelectionModified() const
{
    return std::any_of(m_editedTextFlow.cbegin(), m_editedTextFlow.cend(), [](const auto& item) { return item.editedItemFlags.testFlag(Selected) && item.editedItemFlags.testFlag(Modified); });
}

void PDFDocumentTextFlowEditor::selectByRectangle(QRectF rectangle)
{
    for (auto& item : m_editedTextFlow)
    {
        const QRectF& boundingRectangle = item.boundingRect;

        if (boundingRectangle.isEmpty())
        {
            item.editedItemFlags.setFlag(Selected, false);
            continue;
        }

        const bool isContained = rectangle.contains(boundingRectangle);
        item.editedItemFlags.setFlag(Selected, isContained);
    }
}

void PDFDocumentTextFlowEditor::selectByContainedText(QString text)
{
    for (auto& item : m_editedTextFlow)
    {
        const bool isContained = item.text.contains(text, Qt::CaseSensitive);
        item.editedItemFlags.setFlag(Selected, isContained);
    }
}

void PDFDocumentTextFlowEditor::selectByRegularExpression(const QRegularExpression& expression)
{
    for (auto& item : m_editedTextFlow)
    {
        QRegularExpressionMatch match = expression.match(item.text, 0, QRegularExpression::NormalMatch, QRegularExpression::NoMatchOption);
        const bool hasMatch = match.hasMatch();
        item.editedItemFlags.setFlag(Selected, hasMatch);
    }
}

void PDFDocumentTextFlowEditor::selectByPageIndices(const pdf::PDFClosedIntervalSet& indices)
{
    std::vector<PDFInteger> pageIndices = indices.unfold();
    std::sort(pageIndices.begin(), pageIndices.end());

    for (auto& item : m_editedTextFlow)
    {
        const bool isPageValid = std::binary_search(pageIndices.begin(), pageIndices.end(), item.pageIndex + 1);
        item.editedItemFlags.setFlag(Selected, isPageValid);
    }
}

void PDFDocumentTextFlowEditor::restoreOriginalTexts()
{
    for (auto& item : m_editedTextFlow)
    {
        if (item.editedItemFlags.testFlag(Selected))
        {
            item.text = m_originalTextFlow.getItem(item.originalIndex)->text;
            item.editedItemFlags.setFlag(Modified, false);
        }
    }
}

void PDFDocumentTextFlowEditor::moveSelectionUp()
{
    size_t originalSize = m_editedTextFlow.size();
    size_t firstSelectedIndex = originalSize;

    EditedItems selectedItems;
    for (auto it = m_editedTextFlow.begin(); it != m_editedTextFlow.end();)
    {
        if (it->editedItemFlags.testFlag(Selected))
        {
            if (firstSelectedIndex == originalSize)
            {
                firstSelectedIndex = std::distance(m_editedTextFlow.begin(), it);
            }

            selectedItems.emplace_back(std::move(*it));
            it = m_editedTextFlow.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (firstSelectedIndex > 0)
    {
        --firstSelectedIndex;
    }

    m_editedTextFlow.insert(std::next(m_editedTextFlow.begin(), firstSelectedIndex), std::make_move_iterator(selectedItems.begin()), std::make_move_iterator(selectedItems.end()));
}

void PDFDocumentTextFlowEditor::moveSelectionDown()
{
    size_t originalSize = m_editedTextFlow.size();
    size_t lastSelectedIndex = originalSize;

    EditedItems selectedItems;
    for (auto it = m_editedTextFlow.begin(); it != m_editedTextFlow.end();)
    {
        if (it->editedItemFlags.testFlag(Selected))
        {
            lastSelectedIndex = std::distance(m_editedTextFlow.begin(), it);
            selectedItems.emplace_back(std::move(*it));
            it = m_editedTextFlow.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (lastSelectedIndex < m_editedTextFlow.size())
    {
        ++lastSelectedIndex;
    }

    m_editedTextFlow.insert(std::next(m_editedTextFlow.begin(), lastSelectedIndex), std::make_move_iterator(selectedItems.begin()), std::make_move_iterator(selectedItems.end()));
}

PDFDocumentTextFlowEditor::PageIndicesMappingRange PDFDocumentTextFlowEditor::getItemsForPageIndex(PDFInteger pageIndex) const
{
    auto comparator = [](const auto& l, const auto& r)
    {
        return l.first < r.first;
    };
    return std::equal_range(m_pageIndicesMapping.cbegin(), m_pageIndicesMapping.cend(), std::make_pair(pageIndex, size_t(0)), comparator);
}

PDFDocumentTextFlow PDFDocumentTextFlowEditor::createEditedTextFlow() const
{
    PDFDocumentTextFlow::Items items;
    items.reserve(getItemCount());

    const size_t size = getItemCount();
    for (size_t i = 0; i < size; ++i)
    {
        if (isRemoved(i))
        {
            continue;
        }

        PDFDocumentTextFlow::Item item = *getOriginalItem(i);
        item.text = getText(i);
        items.emplace_back(std::move(item));
    }

    return PDFDocumentTextFlow(std::move(items));
}

void PDFDocumentTextFlowEditor::createPageMapping()
{
    m_pageIndicesMapping.clear();
    m_pageIndicesMapping.reserve(m_editedTextFlow.size());

    for (size_t i = 0; i < m_editedTextFlow.size(); ++i)
    {
        m_pageIndicesMapping.emplace_back(m_editedTextFlow[i].pageIndex, i);
    }

    std::sort(m_pageIndicesMapping.begin(), m_pageIndicesMapping.end());
}

void PDFDocumentTextFlowEditor::createEditedFromOriginalTextFlow()
{
    const size_t count = m_originalTextFlow.getSize();
    m_editedTextFlow.reserve(count);

    for (size_t i = 0; i < count; ++i)
    {
        const PDFDocumentTextFlow::Item* originalItem = getOriginalItem(i);

        if (originalItem->text.trimmed().isEmpty())
        {
            continue;
        }

        EditedItem editedItem;
        static_cast<PDFDocumentTextFlow::Item&>(editedItem) = *originalItem;
        editedItem.originalIndex = i;
        editedItem.editedItemFlags = originalItem->isText() ? None : Removed;
        m_editedTextFlow.emplace_back(std::move(editedItem));
    }

    createPageMapping();
}

void PDFDocumentTextFlowEditor::updateModifiedFlag(size_t index)
{
    const bool isModified = getText(index) != getOriginalItem(index)->text;

    EditedItem* item = getEditedItem(index);
    item->editedItemFlags.setFlag(Modified, isModified);
}

std::map<PDFInteger, PDFDocumentTextFlow> PDFDocumentTextFlow::split(Flags mask) const
{
    std::map<PDFInteger, PDFDocumentTextFlow> result;

    for (const Item& item : m_items)
    {
        if (item.flags & mask)
        {
            result[item.pageIndex].addItem(item);
        }
    }

    return result;
}

void PDFDocumentTextFlow::append(const PDFDocumentTextFlow& textFlow)
{
    m_items.insert(m_items.end(), textFlow.m_items.cbegin(), textFlow.m_items.cend());
}

QString PDFDocumentTextFlow::getText() const
{
    QStringList texts;

    for (const auto& item : m_items)
    {
        texts << item.text.trimmed();
    }

    return texts.join(" ");
}

}   // namespace pdf
