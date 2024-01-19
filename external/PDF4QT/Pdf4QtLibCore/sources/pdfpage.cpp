//    Copyright (C) 2018-2023 Jakub Melka
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

#include "pdfpage.h"
#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfencoding.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFPageInheritableAttributes PDFPageInheritableAttributes::parse(const PDFPageInheritableAttributes& templateAttributes,
                                                                 const PDFObject& dictionary,
                                                                 const PDFObjectStorage* storage)
{
    PDFPageInheritableAttributes result(templateAttributes);

    const PDFObject& dereferencedDictionary = storage->getObject(dictionary);
    if (dereferencedDictionary.isDictionary())
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        const PDFDictionary* pageAttributesDictionary = dereferencedDictionary.getDictionary();
        if (pageAttributesDictionary->hasKey("MediaBox"))
        {
            result.m_mediaBox = loader.readRectangle(pageAttributesDictionary->get("MediaBox"), result.getMediaBox());
        }
        if (pageAttributesDictionary->hasKey("CropBox"))
        {
            result.m_cropBox = loader.readRectangle(pageAttributesDictionary->get("CropBox"), result.getCropBox());
        }
        if (pageAttributesDictionary->hasKey("Resources"))
        {
            result.m_resources = pageAttributesDictionary->get("Resources");
        }
        if (pageAttributesDictionary->hasKey("Rotate"))
        {
            PDFInteger rotation = loader.readInteger(pageAttributesDictionary->get("Rotate"), 0);

            // PDF specification says, that angle can be multiple of 90, so we can have here
            // for example, 450° (90° * 5), or even negative angles. We must get rid of them.
            PDFInteger fullCircles = rotation / 360;
            if (fullCircles != 0)
            {
                rotation = rotation - fullCircles * 360;
            }

            if (rotation < 0)
            {
                rotation += 360;
            }

            switch (rotation)
            {
                case 0:
                {
                    result.m_pageRotation = PageRotation::None;
                    break;
                }
                case 90:
                {
                    result.m_pageRotation = PageRotation::Rotate90;
                    break;
                }
                case 180:
                {
                    result.m_pageRotation = PageRotation::Rotate180;
                    break;
                }
                case 270:
                {
                    result.m_pageRotation = PageRotation::Rotate270;
                    break;
                }
                default:
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid page rotation."));
                }
            }
        }
    }

    return result;
}

PageRotation PDFPageInheritableAttributes::getPageRotation() const
{
    if (m_pageRotation)
    {
        return m_pageRotation.value();
    }
    return PageRotation::None;
}

std::vector<PDFPage> PDFPage::parse(const PDFObjectStorage* storage, const PDFObject& root)
{
    std::vector<PDFPage> result;
    std::set<PDFObjectReference> visited;
    parseImpl(result, visited, PDFPageInheritableAttributes(), root, storage);
    return result;
}

QRectF PDFPage::getRectMM(const QRectF& rect) const
{
    return QRectF(convertPDFPointToMM(rect.left()),
                  convertPDFPointToMM(rect.top()),
                  convertPDFPointToMM(rect.width()),
                  convertPDFPointToMM(rect.height()));
}

QRectF PDFPage::getRotatedMediaBox() const
{
    return getRotatedBox(getMediaBox(), getPageRotation());
}

QRectF PDFPage::getRotatedMediaBoxMM() const
{
    return getRotatedBox(getMediaBoxMM(), getPageRotation());
}

QRectF PDFPage::getRotatedCropBox() const
{
    return getRotatedBox(getCropBox(), getPageRotation());
}

PDFObject PDFPage::getObjectFromPageDictionary(const PDFObjectStorage* storage, const char* key) const
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(m_pageObject))
    {
        return dictionary->get(key);
    }

    return PDFObject();
}

PDFObject PDFPage::getBoxColorInfo(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "BoxColorInfo");
}

PDFObject PDFPage::getTransparencyGroup(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "Group");
}

PDFObject PDFPage::getThumbnail(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "Thumb");
}

PDFObject PDFPage::getTransition(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "Trans");
}

PDFObject PDFPage::getAdditionalActions(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "AA");
}

PDFObject PDFPage::getMetadata(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "Metadata");
}

PDFObject PDFPage::getPieceDictionary(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "PieceInfo");
}

PDFObject PDFPage::getColorSeparationInfo(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "SeparationInfo");
}

PDFObject PDFPage::getFirstSubpageNavigationNode(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "PresSteps");
}

PDFObject PDFPage::getViewports(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "VP");
}

PDFObject PDFPage::getAssociatedFiles(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "AF");
}

PDFObject PDFPage::getOutputIntents(const PDFObjectStorage* storage) const
{
    return getObjectFromPageDictionary(storage, "OutputIntents");
}

QSizeF PDFPage::getRotatedSize(const QSizeF& size, PageRotation rotation)
{
    switch (rotation)
    {
        case PageRotation::None:
        case PageRotation::Rotate180:
            // Preserve rotation
            break;

        case PageRotation::Rotate90:
        case PageRotation::Rotate270:
            return size.transposed();
    }

    return size;
}

QRectF PDFPage::getRotatedBox(const QRectF& rect, PageRotation rotation)
{
    switch (rotation)
    {
        case PageRotation::None:
        case PageRotation::Rotate180:
            // Preserve rotation
            break;

        case PageRotation::Rotate90:
        case PageRotation::Rotate270:
            return rect.transposed();
    }

    return rect;
}

void PDFPage::parseImpl(std::vector<PDFPage>& pages,
                        std::set<PDFObjectReference>& visitedReferences,
                        const PDFPageInheritableAttributes& templateAttributes,
                        const PDFObject& root,
                        const PDFObjectStorage* storage)
{
    // Are we in internal node, or leaf (page object)?
    PDFObjectReference objectReference = root.isReference() ? root.getReference() : PDFObjectReference();
    const PDFObject& dereferenced = storage->getObject(root);

    if (dereferenced.isDictionary())
    {
        const PDFDictionary* dictionary = dereferenced.getDictionary();
        const PDFObject& typeObject =  storage->getObject(dictionary->get("Type"));
        if (typeObject.isName())
        {
            PDFPageInheritableAttributes currentInheritableAttributes = PDFPageInheritableAttributes::parse(templateAttributes, root, storage);

            QByteArray typeString = typeObject.getString();
            if (typeString == "Pages")
            {
                const PDFObject& kids = storage->getObject(dictionary->get("Kids"));
                if (kids.isArray())
                {
                    const PDFArray* kidsArray = kids.getArray();
                    const size_t count = kidsArray->getCount();

                    for (size_t i = 0; i < count; ++i)
                    {
                        const PDFObject& kid = kidsArray->getItem(i);

                        // Check reference
                        if (!kid.isReference())
                        {
                            throw PDFException(PDFTranslationContext::tr("Expected valid kids in page tree."));
                        }

                        // Check cycles
                        if (visitedReferences.count(kid.getReference()))
                        {
                            throw PDFException(PDFTranslationContext::tr("Detected cycles in page tree."));
                        }

                        visitedReferences.insert(kid.getReference());
                        parseImpl(pages, visitedReferences, currentInheritableAttributes, kid, storage);
                    }
                }
                else
                {
                    throw PDFException(PDFTranslationContext::tr("Expected valid kids in page tree."));
                }
            }
            else if (typeString == "Page")
            {
                PDFPage page;

                page.m_pageObject = dereferenced;
                page.m_pageReference = objectReference;
                page.m_mediaBox = currentInheritableAttributes.getMediaBox();
                page.m_cropBox = currentInheritableAttributes.getCropBox();
                page.m_resources = storage->getObject(currentInheritableAttributes.getResources());
                page.m_pageRotation = currentInheritableAttributes.getPageRotation();

                if (!page.m_cropBox.isValid())
                {
                    page.m_cropBox = page.m_mediaBox;
                }

                PDFDocumentDataLoaderDecorator loader(storage);
                page.m_bleedBox = loader.readRectangle(dictionary->get("BleedBox"), page.getCropBox());
                page.m_trimBox = loader.readRectangle(dictionary->get("TrimBox"), page.getCropBox());
                page.m_artBox = loader.readRectangle(dictionary->get("ArtBox"), page.getCropBox());
                page.m_contents = storage->getObject(dictionary->get("Contents"));
                page.m_annots = loader.readReferenceArrayFromDictionary(dictionary, "Annots");
                page.m_lastModified = PDFEncoding::convertToDateTime(loader.readStringFromDictionary(dictionary, "LastModified"));
                page.m_thumbnailReference = loader.readReferenceFromDictionary(dictionary, "Thumb");
                page.m_beads = loader.readReferenceArrayFromDictionary(dictionary, "B");
                page.m_duration = loader.readIntegerFromDictionary(dictionary, "Dur", 0);
                page.m_structParent = loader.readIntegerFromDictionary(dictionary, "StructParents", 0);
                page.m_webCaptureContentSetId = loader.readStringFromDictionary(dictionary, "ID");
                page.m_preferredZoom = loader.readNumberFromDictionary(dictionary, "PZ", 0.0);

                constexpr const std::array<std::pair<const char*, PageTabOrder>, 5> tabStops =
                {
                    std::pair<const char*, PageTabOrder>{ "R", PageTabOrder::Row },
                    std::pair<const char*, PageTabOrder>{ "C", PageTabOrder::Column },
                    std::pair<const char*, PageTabOrder>{ "S", PageTabOrder::Structure },
                    std::pair<const char*, PageTabOrder>{ "A", PageTabOrder::Array },
                    std::pair<const char*, PageTabOrder>{ "W", PageTabOrder::Widget }
                };

                page.m_pageTabOrder = loader.readEnumByName(dictionary->get("Tabs"), tabStops.cbegin(), tabStops.cend(), PageTabOrder::Invalid);
                page.m_templateName = loader.readNameFromDictionary(dictionary, "TemplateInstantiated");
                page.m_userUnit = loader.readNumberFromDictionary(dictionary, "UserUnit", 1.0);
                page.m_documentPart = loader.readReferenceFromDictionary(dictionary, "DPart");

                pages.emplace_back(std::move(page));
            }
            else
            {
                throw PDFException(PDFTranslationContext::tr("Expected valid type item in page tree."));
            }
        }
        else
        {
            throw PDFException(PDFTranslationContext::tr("Expected valid type item in page tree."));
        }
    }
    else
    {
        throw PDFException(PDFTranslationContext::tr("Expected dictionary in page tree."));
    }
}

}   // namespace pdf
