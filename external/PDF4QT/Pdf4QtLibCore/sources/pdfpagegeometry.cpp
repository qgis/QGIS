// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pdfpagegeometry.h"

#include "pdfdocumentbuilder.h"

#include <cmath>
#include <optional>
#include <set>
#include <vector>
#include <QTransform>

#include "pdfdbgheap.h"

namespace pdf
{

class PDFPageGeometryHelper
{
public:
    static QRectF getReferenceRect(const PDFPage* page, PDFPageGeometrySettings::ReferenceBox referenceBox)
    {
        Q_ASSERT(page);

        switch (referenceBox)
        {
            case PDFPageGeometrySettings::ReferenceBox::MediaBox:
                return page->getMediaBox().normalized();

            case PDFPageGeometrySettings::ReferenceBox::CropBox:
                return page->getCropBox().normalized();

            case PDFPageGeometrySettings::ReferenceBox::BleedBox:
                return page->getBleedBox().normalized();

            case PDFPageGeometrySettings::ReferenceBox::TrimBox:
                return page->getTrimBox().normalized();

            case PDFPageGeometrySettings::ReferenceBox::ArtBox:
                return page->getArtBox().normalized();

            default:
                Q_ASSERT(false);
                break;
        }

        return page->getMediaBox().normalized();
    }

    static QRectF getRotatedReferenceRect(const PDFPage* page, PDFPageGeometrySettings::ReferenceBox referenceBox)
    {
        Q_ASSERT(page);
        return PDFPage::getRotatedBox(getReferenceRect(page, referenceBox), page->getPageRotation()).normalized();
    }

    static QPointF getAnchorFactors(PDFPageGeometrySettings::Anchor anchor)
    {
        switch (anchor)
        {
            case PDFPageGeometrySettings::Anchor::TopLeft: return QPointF(0.0, 1.0);
            case PDFPageGeometrySettings::Anchor::TopCenter: return QPointF(0.5, 1.0);
            case PDFPageGeometrySettings::Anchor::TopRight: return QPointF(1.0, 1.0);
            case PDFPageGeometrySettings::Anchor::MiddleLeft: return QPointF(0.0, 0.5);
            case PDFPageGeometrySettings::Anchor::MiddleCenter: return QPointF(0.5, 0.5);
            case PDFPageGeometrySettings::Anchor::MiddleRight: return QPointF(1.0, 0.5);
            case PDFPageGeometrySettings::Anchor::BottomLeft: return QPointF(0.0, 0.0);
            case PDFPageGeometrySettings::Anchor::BottomCenter: return QPointF(0.5, 0.0);
            case PDFPageGeometrySettings::Anchor::BottomRight: return QPointF(1.0, 0.0);
            default:
                Q_ASSERT(false);
                break;
        }

        return QPointF(0.5, 0.5);
    }

    static QByteArray formatPDFNumber(PDFReal value)
    {
        QByteArray text = QByteArray::number(value, 'f', 6);

        while (text.endsWith('0'))
        {
            text.chop(1);
        }

        if (text.endsWith('.'))
        {
            text.chop(1);
        }

        if (text.isEmpty() || text == "-0")
        {
            text = "0";
        }

        return text;
    }

    static PDFObjectReference createContentStream(PDFDocumentBuilder* builder, const QByteArray& content)
    {
        Q_ASSERT(builder);

        PDFDictionary dictionary;
        dictionary.addEntry(PDFInplaceOrMemoryString("Length"), PDFObject::createInteger(content.size()));
        PDFObject streamObject = PDFObject::createStream(std::make_shared<PDFStream>(std::move(dictionary), QByteArray(content)));
        return builder->addObject(std::move(streamObject));
    }

    static void appendContentReference(PDFDocumentBuilder* builder,
                                       std::vector<PDFObjectReference>& contentReferences,
                                       PDFObject contentObject)
    {
        Q_ASSERT(builder);

        const PDFObject contentObjectDereferenced = builder->getObject(contentObject);
        if (contentObjectDereferenced.isNull())
        {
            return;
        }

        if (contentObjectDereferenced.isStream())
        {
            if (contentObject.isReference())
            {
                contentReferences.push_back(contentObject.getReference());
            }
            else
            {
                contentReferences.push_back(builder->addObject(contentObjectDereferenced));
            }
        }
        else if (contentObjectDereferenced.isArray())
        {
            const PDFArray* array = contentObjectDereferenced.getArray();
            for (const PDFObject& object : *array)
            {
                appendContentReference(builder, contentReferences, object);
            }
        }
    }

    static void prependContentTransform(PDFDocumentBuilder* builder,
                                        PDFObjectReference pageReference,
                                        PDFReal sx,
                                        PDFReal sy,
                                        PDFReal tx,
                                        PDFReal ty)
    {
        Q_ASSERT(builder);

        const PDFObjectStorage* storage = builder->getStorage();
        const PDFDictionary* pageDictionary = storage->getDictionaryFromObject(storage->getObjectByReference(pageReference));
        if (!pageDictionary)
        {
            return;
        }

        std::vector<PDFObjectReference> contentReferences;
        appendContentReference(builder, contentReferences, pageDictionary->get("Contents"));
        if (contentReferences.empty())
        {
            return;
        }

        QByteArray prefix;
        prefix.reserve(128);
        prefix.append("q ");
        prefix.append(formatPDFNumber(sx));
        prefix.append(' ');
        prefix.append("0 0 ");
        prefix.append(formatPDFNumber(sy));
        prefix.append(' ');
        prefix.append(formatPDFNumber(tx));
        prefix.append(' ');
        prefix.append(formatPDFNumber(ty));
        prefix.append(" cm\n");

        const PDFObjectReference prefixReference = createContentStream(builder, prefix);
        const PDFObjectReference suffixReference = createContentStream(builder, QByteArray("Q\n"));

        contentReferences.insert(contentReferences.begin(), prefixReference);
        contentReferences.push_back(suffixReference);

        PDFObjectFactory pageUpdateFactory;
        pageUpdateFactory.beginDictionary();
        pageUpdateFactory.beginDictionaryItem("Contents");
        pageUpdateFactory << contentReferences;
        pageUpdateFactory.endDictionaryItem();
        pageUpdateFactory.endDictionary();
        builder->mergeTo(pageReference, pageUpdateFactory.takeObject());
    }

    static std::optional<PDFObject> transformCoordinateArray(PDFDocumentBuilder* builder,
                                                             PDFObject object,
                                                             const QTransform& transform)
    {
        Q_ASSERT(builder);

        object = builder->getObject(object);
        if (!object.isArray())
        {
            return std::nullopt;
        }

        std::vector<PDFReal> coordinates;
        const PDFArray* array = object.getArray();
        coordinates.reserve(array->getCount());
        for (const PDFObject& item : *array)
        {
            if (item.isReal())
            {
                coordinates.push_back(item.getReal());
            }
            else if (item.isInt())
            {
                coordinates.push_back(item.getInteger());
            }
            else
            {
                return std::nullopt;
            }
        }

        if (coordinates.size() % 2 != 0)
        {
            return std::nullopt;
        }

        PDFObjectFactory factory;
        factory.beginArray();
        for (size_t i = 0; i < coordinates.size(); i += 2)
        {
            const QPointF transformedPoint = transform.map(QPointF(coordinates[i], coordinates[i + 1]));
            factory << transformedPoint.x();
            factory << transformedPoint.y();
        }
        factory.endArray();
        return factory.takeObject();
    }

    static std::optional<PDFObject> transformInkListArray(PDFDocumentBuilder* builder,
                                                          PDFObject object,
                                                          const QTransform& transform)
    {
        Q_ASSERT(builder);

        object = builder->getObject(object);
        if (!object.isArray())
        {
            return std::nullopt;
        }

        PDFObjectFactory factory;
        factory.beginArray();
        for (const PDFObject& item : *object.getArray())
        {
            const std::optional<PDFObject> transformedArray = transformCoordinateArray(builder, item, transform);
            if (!transformedArray.has_value())
            {
                return std::nullopt;
            }

            factory << transformedArray.value();
        }
        factory.endArray();
        return factory.takeObject();
    }

    static void transformAnnotationGeometry(PDFDocumentBuilder* builder,
                                            PDFObjectReference annotationReference,
                                            const QTransform& transform)
    {
        Q_ASSERT(builder);

        const PDFObject& annotationObject = builder->getObjectByReference(annotationReference);
        const PDFDictionary* annotationDictionary = builder->getDictionaryFromObject(annotationObject);
        if (!annotationDictionary)
        {
            return;
        }

        const PDFObjectStorage* storage = builder->getStorage();
        PDFDocumentDataLoaderDecorator loader(storage);

        PDFObjectFactory updateFactory;
        updateFactory.beginDictionary();
        bool changed = false;

        const QRectF rect = loader.readRectangle(annotationDictionary->get("Rect"), QRectF());
        if (rect.isValid())
        {
            updateFactory.beginDictionaryItem("Rect");
            updateFactory << transform.mapRect(rect).normalized();
            updateFactory.endDictionaryItem();
            changed = true;
        }

        auto transformAndStore = [&](const QByteArray& key)
        {
            const std::optional<PDFObject> transformed = transformCoordinateArray(builder, annotationDictionary->get(key), transform);
            if (transformed.has_value())
            {
                updateFactory.beginDictionaryItem(key);
                updateFactory << transformed.value();
                updateFactory.endDictionaryItem();
                changed = true;
            }
        };

        transformAndStore("L");
        transformAndStore("CL");
        transformAndStore("Vertices");
        transformAndStore("QuadPoints");

        const std::optional<PDFObject> transformedInkList = transformInkListArray(builder, annotationDictionary->get("InkList"), transform);
        if (transformedInkList.has_value())
        {
            updateFactory.beginDictionaryItem("InkList");
            updateFactory << transformedInkList.value();
            updateFactory.endDictionaryItem();
            changed = true;
        }

        updateFactory.endDictionary();

        if (changed)
        {
            builder->mergeTo(annotationReference, updateFactory.takeObject());
            builder->updateAnnotationAppearanceStreams(annotationReference);
        }
    }

    static void transformPageAnnotations(PDFDocumentBuilder* builder,
                                         PDFObjectReference pageReference,
                                         const QTransform& transform)
    {
        Q_ASSERT(builder);

        const PDFObjectStorage* storage = builder->getStorage();
        const PDFObject& pageObject = storage->getObjectByReference(pageReference);
        const PDFDictionary* pageDictionary = storage->getDictionaryFromObject(pageObject);
        if (!pageDictionary)
        {
            return;
        }

        PDFDocumentDataLoaderDecorator loader(storage);
        const std::vector<PDFObjectReference> annotationReferences = loader.readReferenceArrayFromDictionary(pageDictionary, "Annots");
        for (const PDFObjectReference& annotationReference : annotationReferences)
        {
            transformAnnotationGeometry(builder, annotationReference, transform);
        }
    }

    static std::vector<PDFInteger> selectPageIndices(const PDFDocument* document,
                                                     const PDFPageGeometrySettings& settings,
                                                     QString* errorMessage)
    {
        std::vector<PDFInteger> result;

        if (!document)
        {
            if (errorMessage)
            {
                *errorMessage = PDFTranslationContext::tr("Invalid document.");
            }
            return result;
        }

        const PDFCatalog* catalog = document->getCatalog();
        const PDFInteger pageCount = catalog->getPageCount();
        if (pageCount <= 0)
        {
            return result;
        }

        std::set<PDFInteger> rangeSelection;
        const QString rangeText = settings.pageRange.simplified();
        if (!rangeText.isEmpty())
        {
            QString parseError;
            const PDFClosedIntervalSet closedIntervalSet = PDFClosedIntervalSet::parse(1, pageCount, rangeText, &parseError);
            if (!parseError.isEmpty())
            {
                if (errorMessage)
                {
                    *errorMessage = parseError;
                }
                return { };
            }

            const std::vector<PDFInteger> unfolded = closedIntervalSet.unfold();
            for (const PDFInteger pageNumber : unfolded)
            {
                rangeSelection.insert(pageNumber);
            }
        }

        result.reserve(pageCount);
        for (PDFInteger pageIndex = 0; pageIndex < pageCount; ++pageIndex)
        {
            const PDFInteger pageNumber = pageIndex + 1;
            if (!rangeSelection.empty() && !rangeSelection.count(pageNumber))
            {
                continue;
            }

            const PDFPage* page = catalog->getPage(pageIndex);
            if (!page)
            {
                continue;
            }

            switch (settings.pageSubset)
            {
                case PDFPageGeometrySettings::PageSubset::AllPages:
                    break;

                case PDFPageGeometrySettings::PageSubset::OddPages:
                    if ((pageNumber % 2) == 0)
                    {
                        continue;
                    }
                    break;

                case PDFPageGeometrySettings::PageSubset::EvenPages:
                    if ((pageNumber % 2) != 0)
                    {
                        continue;
                    }
                    break;

                case PDFPageGeometrySettings::PageSubset::PortraitPages:
                {
                    const QSizeF rotatedSize = getRotatedReferenceRect(page, settings.referenceBox).size();
                    if (rotatedSize.width() > rotatedSize.height())
                    {
                        continue;
                    }
                    break;
                }

                case PDFPageGeometrySettings::PageSubset::LandscapePages:
                {
                    const QSizeF rotatedSize = getRotatedReferenceRect(page, settings.referenceBox).size();
                    if (rotatedSize.width() < rotatedSize.height())
                    {
                        continue;
                    }
                    break;
                }

                default:
                    Q_ASSERT(false);
                    break;
            }

            result.push_back(pageIndex);
        }

        return result;
    }
};

PDFOperationResult PDFPageGeometry::apply(PDFDocument* document,
                                          const PDFPageGeometrySettings& settings,
                                          PDFModifiedDocument::ModificationFlags* modificationFlags)
{
    if (modificationFlags)
    {
        *modificationFlags = PDFModifiedDocument::ModificationFlags();
    }

    if (!document)
    {
        return PDFTranslationContext::tr("Invalid document.");
    }

    if (!settings.hasAnyTargetBoxSelected())
    {
        return PDFTranslationContext::tr("No target page box selected.");
    }

    QString pageSelectionError;
    const std::vector<PDFInteger> pageIndices = PDFPageGeometryHelper::selectPageIndices(document, settings, &pageSelectionError);
    if (!pageSelectionError.isEmpty())
    {
        return pageSelectionError;
    }

    if (pageIndices.empty())
    {
        return true;
    }

    PDFDocumentModifier modifier(document);
    PDFDocumentBuilder* builder = modifier.getBuilder();
    Q_ASSERT(builder);

    PDFModifiedDocument::ModificationFlags flags = PDFModifiedDocument::ModificationFlags(PDFModifiedDocument::Reset | PDFModifiedDocument::PreserveUndoRedo);
    bool isPageContentChanged = false;
    bool areAnnotationsChanged = false;

    for (const PDFInteger pageIndex : pageIndices)
    {
        const PDFPage* page = document->getCatalog()->getPage(pageIndex);
        if (!page)
        {
            continue;
        }

        const PDFObjectReference pageReference = page->getPageReference();
        const QRectF sourceRect = PDFPageGeometryHelper::getReferenceRect(page, settings.referenceBox);
        if (!sourceRect.isValid())
        {
            return PDFTranslationContext::tr("Reference box on page %1 is invalid.").arg(pageIndex + 1);
        }

        const PDFReal leftMarginPt = settings.marginsMM.left() * PDF_MM_TO_POINT;
        const PDFReal rightMarginPt = settings.marginsMM.right() * PDF_MM_TO_POINT;
        const PDFReal topMarginPt = settings.marginsMM.top() * PDF_MM_TO_POINT;
        const PDFReal bottomMarginPt = settings.marginsMM.bottom() * PDF_MM_TO_POINT;

        PDFReal targetWidthPt = sourceRect.width() + leftMarginPt + rightMarginPt;
        PDFReal targetHeightPt = sourceRect.height() + topMarginPt + bottomMarginPt;

        if (settings.useTargetPageSize)
        {
            targetWidthPt = settings.targetPageSizeMM.width() * PDF_MM_TO_POINT;
            targetHeightPt = settings.targetPageSizeMM.height() * PDF_MM_TO_POINT;
        }

        if (!(targetWidthPt > 0.0 && targetHeightPt > 0.0))
        {
            return PDFTranslationContext::tr("Target page size for page %1 is invalid.").arg(pageIndex + 1);
        }

        const QRectF innerRect(leftMarginPt,
                               bottomMarginPt,
                               targetWidthPt - leftMarginPt - rightMarginPt,
                               targetHeightPt - topMarginPt - bottomMarginPt);

        if (!(innerRect.width() > 0.0 && innerRect.height() > 0.0))
        {
            return PDFTranslationContext::tr("Margins for page %1 exceed target page size.").arg(pageIndex + 1);
        }

        PDFReal scaleX = 1.0;
        PDFReal scaleY = 1.0;
        if (settings.scaleContent)
        {
            scaleX = innerRect.width() / sourceRect.width();
            scaleY = innerRect.height() / sourceRect.height();

            if (settings.preserveAspectRatio)
            {
                const PDFReal scale = qMin(scaleX, scaleY);
                scaleX = scale;
                scaleY = scale;
            }
        }

        if (!(scaleX > 0.0 && scaleY > 0.0))
        {
            return PDFTranslationContext::tr("Content scale for page %1 is invalid.").arg(pageIndex + 1);
        }

        const PDFReal transformedWidth = sourceRect.width() * scaleX;
        const PDFReal transformedHeight = sourceRect.height() * scaleY;
        const QPointF anchorFactors = PDFPageGeometryHelper::getAnchorFactors(settings.anchor);
        const PDFReal offsetXPt = settings.offsetMM.x() * PDF_MM_TO_POINT;
        const PDFReal offsetYPt = settings.offsetMM.y() * PDF_MM_TO_POINT;

        const PDFReal destinationLeft = innerRect.left() + (innerRect.width() - transformedWidth) * anchorFactors.x() + offsetXPt;
        const PDFReal destinationBottom = innerRect.top() + (innerRect.height() - transformedHeight) * anchorFactors.y() + offsetYPt;

        QRectF pageBoxRect;
        if (settings.scaleContent)
        {
            // For content-scaling mode we normalize output page to [0, 0, targetWidth, targetHeight].
            pageBoxRect = QRectF(0.0, 0.0, targetWidthPt, targetHeightPt);

            const PDFReal tx = destinationLeft - sourceRect.left() * scaleX;
            const PDFReal ty = destinationBottom - sourceRect.top() * scaleY;
            PDFPageGeometryHelper::prependContentTransform(builder, pageReference, scaleX, scaleY, tx, ty);
            isPageContentChanged = true;

            if (settings.scaleAnnotationsAndFormFields)
            {
                QTransform transform(scaleX, 0.0, 0.0, scaleY, tx, ty);
                PDFPageGeometryHelper::transformPageAnnotations(builder, pageReference, transform);
                areAnnotationsChanged = true;
            }
        }
        else
        {
            // Keep content in-place and shift page coordinate origin to place the source box.
            const PDFReal dx = destinationLeft - sourceRect.left();
            const PDFReal dy = destinationBottom - sourceRect.top();
            pageBoxRect = QRectF(-dx, -dy, targetWidthPt, targetHeightPt);
        }

        if (settings.applyMediaBox)
        {
            builder->setPageMediaBox(pageReference, pageBoxRect);
        }
        if (settings.applyCropBox)
        {
            builder->setPageCropBox(pageReference, pageBoxRect);
        }
        if (settings.applyBleedBox)
        {
            builder->setPageBleedBox(pageReference, pageBoxRect);
        }
        if (settings.applyTrimBox)
        {
            builder->setPageTrimBox(pageReference, pageBoxRect);
        }
        if (settings.applyArtBox)
        {
            builder->setPageArtBox(pageReference, pageBoxRect);
        }
    }

    modifier.markReset();
    if (isPageContentChanged)
    {
        modifier.markPageContentsChanged();
        flags |= PDFModifiedDocument::PageContents;
    }
    if (areAnnotationsChanged)
    {
        modifier.markAnnotationsChanged();
        flags |= PDFModifiedDocument::Annotation;
    }

    if (modifier.finalize())
    {
        *document = *modifier.getDocument();
    }

    if (modificationFlags)
    {
        *modificationFlags = flags;
    }

    return true;
}

}   // namespace pdf
