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
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#include "pdfdocumentbuilder.h"
#include "pdfencoding.h"
#include "pdfconstants.h"
#include "pdfdocumentreader.h"
#include "pdfobjectutils.h"
#include "pdfnametreeloader.h"
#include "pdfparser.h"
#include "pdfstreamfilters.h"

#include <QBuffer>
#include <QPainter>
#include <QPdfWriter>

#include "pdfdbgheap.h"

namespace pdf
{

void PDFObjectFactory::beginArray()
{
    m_items.emplace_back(ItemType::Array, PDFArray());
}

void PDFObjectFactory::endArray()
{
    Item topItem = qMove(m_items.back());
    Q_ASSERT(topItem.type == ItemType::Array);
    m_items.pop_back();
    addObject(PDFObject::createArray(std::make_shared<PDFArray>(qMove(std::get<PDFArray>(topItem.object)))));
}

void PDFObjectFactory::beginDictionary()
{
    m_items.emplace_back(ItemType::Dictionary, PDFDictionary());
}

void PDFObjectFactory::endDictionary()
{
    Item topItem = qMove(m_items.back());
    Q_ASSERT(topItem.type == ItemType::Dictionary);
    m_items.pop_back();
    addObject(PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(std::get<PDFDictionary>(topItem.object)))));
}

void PDFObjectFactory::beginDictionaryItem(const QByteArray& name)
{
    m_items.emplace_back(ItemType::DictionaryItem, name, PDFObject());
}

void PDFObjectFactory::endDictionaryItem()
{
    Item topItem = qMove(m_items.back());
    Q_ASSERT(topItem.type == ItemType::DictionaryItem);
    m_items.pop_back();

    Item& dictionaryItem = m_items.back();
    Q_ASSERT(dictionaryItem.type == ItemType::Dictionary);
    std::get<PDFDictionary>(dictionaryItem.object).addEntry(PDFInplaceOrMemoryString(qMove(topItem.itemName)), qMove(std::get<PDFObject>(topItem.object)));
}

PDFObjectFactory& PDFObjectFactory::operator<<(const PDFDestination& destination)
{
    if (!destination.isValid())
    {
        *this << PDFObject();
        return *this;
    }

    if (destination.isNamedDestination())
    {
        *this << WrapName(destination.getName());
    }
    else
    {
        beginArray();

        // Destination
        if (destination.getPageReference().isValid())
        {
            *this << destination.getPageReference();
        }
        else
        {
            *this << destination.getPageIndex();
        }

        // Type
        QByteArray type;

        switch (destination.getDestinationType())
        {
            case DestinationType::XYZ:
                type = "XYZ";
                break;

            case DestinationType::Fit:
                type = "Fit";
                break;

            case DestinationType::FitH:
                type = "FitH";
                break;

            case DestinationType::FitV:
                type = "FitV";
                break;

            case DestinationType::FitR:
                type = "FitR";
                break;

            case DestinationType::FitB:
                type = "FitB";
                break;

            case DestinationType::FitBH:
                type = "FitBH";
                break;

            case DestinationType::FitBV:
                type = "FitBV";
                break;

            default:
                Q_ASSERT(false);
                break;
        }

        *this << WrapName(type);

        endArray();
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(FileAttachmentIcon icon)
{
    switch (icon)
    {
        case FileAttachmentIcon::Graph:
            *this << WrapName("Graph");
            break;

        case FileAttachmentIcon::Paperclip:
            *this << WrapName("Paperclip");
            break;

        case FileAttachmentIcon::PushPin:
            *this << WrapName("PushPin");
            break;

        case FileAttachmentIcon::Tag:
            *this << WrapName("Tag");
            break;

        default:
            Q_ASSERT(false);
            break;
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(Stamp stamp)
{
    switch (stamp)
    {
        case Stamp::Approved:
            *this << WrapName("Approved");
            break;

        case Stamp::AsIs:
            *this << WrapName("AsIs");
            break;

        case Stamp::Confidential:
            *this << WrapName("Confidential");
            break;

        case Stamp::Departmental:
            *this << WrapName("Departmental");
            break;

        case Stamp::Draft:
            *this << WrapName("Draft");
            break;

        case Stamp::Experimental:
            *this << WrapName("Experimental");
            break;

        case Stamp::Expired:
            *this << WrapName("Expired");
            break;

        case Stamp::Final:
            *this << WrapName("Final");
            break;

        case Stamp::ForComment:
            *this << WrapName("ForComment");
            break;

        case Stamp::ForPublicRelease:
            *this << WrapName("ForPublicRelease");
            break;

        case Stamp::NotApproved:
            *this << WrapName("NotApproved");
            break;

        case Stamp::NotForPublicRelease:
            *this << WrapName("NotForPublicRelease");
            break;

        case Stamp::Sold:
            *this << WrapName("Sold");
            break;

        case Stamp::TopSecret:
            *this << WrapName("TopSecret");
            break;

        default:
            Q_ASSERT(false);
            break;
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(const PDFObject& object)
{
    addObject(object);
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(AnnotationBorderStyle style)
{
    switch (style)
    {
        case AnnotationBorderStyle::Solid:
            *this << WrapName("S");
            break;

        case AnnotationBorderStyle::Dashed:
            *this << WrapName("D");
            break;

        case AnnotationBorderStyle::Beveled:
            *this << WrapName("B");
            break;

        case AnnotationBorderStyle::Inset:
            *this << WrapName("I");
            break;

        case AnnotationBorderStyle::Underline:
            *this << WrapName("U");
            break;

        default:
            Q_ASSERT(false);
            break;
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(const QDateTime& dateTime)
{
    addObject(PDFObject::createString(PDFEncoding::convertDateTimeToString(dateTime)));
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(const QPointF& point)
{
    *this << point.x();
    *this << point.y();
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(AnnotationLineEnding lineEnding)
{
    switch (lineEnding)
    {
        case AnnotationLineEnding::Square:
            *this << WrapName("Square");
            break;

        case AnnotationLineEnding::Circle:
            *this << WrapName("Circle");
            break;

        case AnnotationLineEnding::Diamond:
            *this << WrapName("Diamond");
            break;

        case AnnotationLineEnding::OpenArrow:
            *this << WrapName("OpenArrow");
            break;

        case AnnotationLineEnding::ClosedArrow:
            *this << WrapName("ClosedArrow");
            break;

        case AnnotationLineEnding::Butt:
            *this << WrapName("Butt");
            break;

        case AnnotationLineEnding::ROpenArrow:
            *this << WrapName("ROpenArrow");
            break;

        case AnnotationLineEnding::RClosedArrow:
            *this << WrapName("RClosedArrow");
            break;

        case AnnotationLineEnding::Slash:
            *this << WrapName("Slash");
            break;

        case AnnotationLineEnding::None:
        default:
            *this << WrapName("None");
            break;
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(WrapString string)
{
    addObject(PDFObject::createString(qMove(string.string)));
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(WrapFreeTextAlignment alignment)
{
    if (alignment.alignment.testFlag(Qt::AlignLeft))
    {
        *this << 0;
    }
    else if (alignment.alignment.testFlag(Qt::AlignHCenter))
    {
        *this << 1;
    }
    else if (alignment.alignment.testFlag(Qt::AlignRight))
    {
        *this << 2;
    }
    else
    {
        // Default is left alignment
        *this << 0;
    }
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(LinkHighlightMode mode)
{
    switch (mode)
    {
        case LinkHighlightMode::None:
        {
            *this << WrapName("N");
            break;
        }

        case LinkHighlightMode::Invert:
        {
            *this << WrapName("I");
            break;
        }

        case LinkHighlightMode::Outline:
        {
            *this << WrapName("O");
            break;
        }

        case LinkHighlightMode::Push:
        {
            *this << WrapName("P");
            break;
        }

        default:
            Q_ASSERT(false);
            break;
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(TextAnnotationIcon icon)
{
    switch (icon)
    {
        case TextAnnotationIcon::Comment:
        {
            *this << WrapName("Comment");
            break;
        }

        case TextAnnotationIcon::Help:
        {
            *this << WrapName("Help");
            break;
        }

        case TextAnnotationIcon::Insert:
        {
            *this << WrapName("Insert");
            break;
        }

        case TextAnnotationIcon::Key:
        {
            *this << WrapName("Key");
            break;
        }

        case TextAnnotationIcon::NewParagraph:
        {
            *this << WrapName("NewParagraph");
            break;
        }

        case TextAnnotationIcon::Note:
        {
            *this << WrapName("Note");
            break;
        }

        case TextAnnotationIcon::Paragraph:
        {
            *this << WrapName("Paragraph");
            break;
        }

        default:
        {
            Q_ASSERT(false);
        }
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(PageRotation pageRotation)
{
    switch (pageRotation)
    {
        case pdf::PageRotation::None:
            *this << PDFInteger(0);
            break;
        case pdf::PageRotation::Rotate90:
            *this << PDFInteger(90);
            break;
        case pdf::PageRotation::Rotate180:
            *this << PDFInteger(180);
            break;
        case pdf::PageRotation::Rotate270:
            *this << PDFInteger(270);
            break;

        default:
            Q_ASSERT(false);
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(WrapEmptyArray)
{
    beginArray();
    endArray();
    return *this;
}

PDFObject PDFObjectFactory::createTextString(QString textString)
{
    if (!PDFEncoding::canConvertToEncoding(textString, PDFEncoding::Encoding::PDFDoc, nullptr))
    {
        // Use unicode encoding
        QByteArray ba;

        {
            QTextStream textStream(&ba, QIODevice::WriteOnly);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            textStream.setEncoding(QStringConverter::Utf16BE);
#else
            textStream.setCodec("UTF-16BE");
#endif
            textStream.setGenerateByteOrderMark(true);
            textStream << textString;
        }

        return PDFObject::createString(qMove(ba));
    }
    else
    {
        // Use PDF document encoding
        return PDFObject::createString(PDFEncoding::convertToEncoding(textString, PDFEncoding::Encoding::PDFDoc));
    }
}

PDFObjectFactory& PDFObjectFactory::operator<<(QString textString)
{
    addObject(createTextString(textString));
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(WrapAnnotationColor color)
{
    if (color.color.isValid())
    {
        // Jakub Melka: we will decide, if we have gray/rgb/cmyk color
        QColor value = color.color;
        if (value.spec() == QColor::Cmyk)
        {
            *this << std::initializer_list<PDFReal>{ value.cyanF(), value.magentaF(), value.yellowF(), value.blackF() };
        }
        else if (qIsGray(value.rgb()))
        {
            *this << std::initializer_list<PDFReal>{ value.redF() };
        }
        else
        {
            *this << std::initializer_list<PDFReal>{ value.redF(), value.greenF(), value.blueF() };
        }
    }
    else
    {
        addObject(PDFObject::createNull());
    }

    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(WrapCurrentDateTime)
{
    addObject(PDFObject::createString(PDFEncoding::convertDateTimeToString(QDateTime::currentDateTime())));
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(const QRectF& value)
{
    *this << std::initializer_list<PDFReal>{ value.left(), value.top(), value.right(), value.bottom() };
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(int value)
{
    *this << PDFInteger(value);
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(PDFFormSubmitFlags flags)
{
    *this << PDFInteger(flags);
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(WrapName wrapName)
{
    addObject(PDFObject::createName(qMove(wrapName.name)));
    return *this;
}

PDFObject PDFObjectFactory::takeObject()
{
    Q_ASSERT(m_items.size() == 1);
    Q_ASSERT(m_items.back().type == ItemType::Object);
    PDFObject result = qMove(std::get<PDFObject>(m_items.back().object));
    m_items.clear();
    return result;
}

void PDFObjectFactory::addObject(PDFObject object)
{
    if (m_items.empty())
    {
        m_items.emplace_back(ItemType::Object, qMove(object));
        return;
    }

    Item& topItem = m_items.back();
    switch (topItem.type)
    {
        case ItemType::Object:
            // Just override the object
            topItem.object = qMove(object);
            break;

        case ItemType::Dictionary:
            // Do not do anything - we are inside dictionary
            break;

        case ItemType::DictionaryItem:
            // Add item to dictionary item
            topItem.object = qMove(object);
            break;

        case ItemType::Array:
            std::get<PDFArray>(topItem.object).appendItem(qMove(object));
            break;

        default:
            Q_ASSERT(false);
            break;
    }
}

PDFObjectFactory& PDFObjectFactory::operator<<(std::nullptr_t)
{
    addObject(PDFObject::createNull());
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(bool value)
{
    addObject(PDFObject::createBool(value));
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(PDFReal value)
{
    addObject(PDFObject::createReal(value));
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(PDFInteger value)
{
    addObject(PDFObject::createInteger(value));
    return *this;
}

PDFObjectFactory& PDFObjectFactory::operator<<(PDFObjectReference value)
{
    addObject(PDFObject::createReference(value));
    return *this;
}

PDFDocumentBuilder::PDFDocumentBuilder() :
    m_version(1, 7)
{
    createDocument();
}

PDFDocumentBuilder::PDFDocumentBuilder(const PDFDocument* document) :
    m_storage(document->getStorage()),
    m_version(document->getInfo()->version)
{

}

PDFDocumentBuilder::PDFDocumentBuilder(const PDFObjectStorage& storage, PDFVersion version) :
    m_storage(storage),
    m_version(version)
{

}

void PDFDocumentBuilder::reset()
{
    *this = PDFDocumentBuilder();
}

void PDFDocumentBuilder::createDocument()
{
    if (!m_storage.getObjects().empty())
    {
        reset();
    }

    addObject(PDFObject::createNull());
    PDFObjectReference catalog = createCatalog();
    PDFObject trailerDictionary = createTrailerDictionary(catalog);
    m_storage.updateTrailerDictionary(trailerDictionary);
    m_storage.setSecurityHandler(PDFSecurityHandlerPointer(new PDFNoneSecurityHandler()));
}

void PDFDocumentBuilder::setDocument(const PDFDocument* document)
{
    m_storage = document->getStorage();
    m_version = document->getInfo()->version;
}

PDFDocument PDFDocumentBuilder::build()
{
    updateTrailerDictionary(m_storage.getObjects().size());
    return PDFDocument(PDFObjectStorage(m_storage), m_version, QByteArray());
}

QByteArray PDFDocumentBuilder::getDecodedStream(const PDFStream* stream) const
{
    return m_storage.getDecodedStream(stream);
}

std::array<PDFReal, 4> PDFDocumentBuilder::getAnnotationReductionRectangle(const QRectF& boundingRect, const QRectF& innerRect) const
{
    return { qAbs(innerRect.left() - boundingRect.left()), qAbs(boundingRect.bottom() - innerRect.bottom()), qAbs(boundingRect.right() - innerRect.right()), qAbs(boundingRect.top() - innerRect.top()) };
}

PDFPageContentStreamBuilder::PDFPageContentStreamBuilder(PDFDocumentBuilder* builder,
                                                         PDFContentStreamBuilder::CoordinateSystem coordinateSystem,
                                                         Mode mode) :
    m_documentBuilder(builder),
    m_contentStreamBuilder(nullptr),
    m_coordinateSystem(coordinateSystem),
    m_mode(mode)
{

}

QPainter* PDFPageContentStreamBuilder::begin(PDFObjectReference page)
{
    if (m_contentStreamBuilder)
    {
        // Invalid call to begin function
        return nullptr;
    }

    const PDFObjectStorage* storage = m_documentBuilder->getStorage();
    const PDFDictionary* pageDictionary = storage->getDictionaryFromObject(storage->getObject(page));
    if (!pageDictionary)
    {
        return nullptr;
    }

    PDFDocumentDataLoaderDecorator loader(storage);
    QRectF mediaBox = loader.readRectangle(pageDictionary->get("MediaBox"), QRectF());
    if (!mediaBox.isValid())
    {
        return nullptr;
    }

    m_pageReference = page;
    m_contentStreamBuilder = new PDFContentStreamBuilder(mediaBox.size(), m_coordinateSystem);
    return m_contentStreamBuilder->begin();
}

QPainter* PDFPageContentStreamBuilder::beginNewPage(QRectF mediaBox)
{
    return begin(m_documentBuilder->appendPage(mediaBox));
}

void PDFPageContentStreamBuilder::end(QPainter* painter)
{
    if (!m_contentStreamBuilder)
    {
        return;
    }

    PDFContentStreamBuilder::ContentStream contentStream = m_contentStreamBuilder->end(painter);

    delete m_contentStreamBuilder;
    m_contentStreamBuilder = nullptr;

    // Update page's content stream

    std::vector<PDFObject> copiedObjects = m_documentBuilder->copyFrom({ contentStream.resources, contentStream.contents }, contentStream.document.getStorage(), true);
    Q_ASSERT(copiedObjects.size() == 2);

    PDFObjectReference resourcesReference = copiedObjects[0].getReference();
    PDFObjectReference contentsReference = copiedObjects[1].getReference();

    if (m_mode == Mode::Replace)
    {
        PDFObjectFactory pageUpdateFactory;

        pageUpdateFactory.beginDictionary();

        pageUpdateFactory.beginDictionaryItem("Contents");
        pageUpdateFactory << contentsReference;
        pageUpdateFactory.endDictionaryItem();

        pageUpdateFactory.beginDictionaryItem("Resources");
        pageUpdateFactory << resourcesReference;
        pageUpdateFactory.endDictionaryItem();

        pageUpdateFactory.endDictionary();

        m_documentBuilder->mergeTo(m_pageReference, pageUpdateFactory.takeObject());
    }
    else
    {
        std::vector<PDFObjectReference> contentReferences;
        PDFObject pageObject = m_documentBuilder->getObjectByReference(m_pageReference);

        if (pageObject.isDictionary())
        {
            const PDFDictionary* pageDictionary = pageObject.getDictionary();
            const PDFObject& oldContents = pageDictionary->get("Contents");
            const PDFObject& oldContentsObject = m_documentBuilder->getObject(oldContents);

            if (oldContentsObject.isStream())
            {
                if (oldContents.isReference())
                {
                    contentReferences.push_back(oldContents.getReference());
                }
            }
            else if (oldContentsObject.isArray())
            {
                const PDFArray* contentsArray = oldContentsObject.getArray();
                for (const PDFObject& object : *contentsArray)
                {
                    if (object.isReference())
                    {
                        contentReferences.push_back(object.getReference());
                    }
                }
            }

            PDFObject oldResourcesObject = m_documentBuilder->getObject(pageDictionary->get("Resources"));
            oldResourcesObject = removeDictionaryReferencesFromResources(oldResourcesObject);
            replaceResources(contentsReference, resourcesReference, oldResourcesObject);
            m_documentBuilder->mergeTo(resourcesReference, oldResourcesObject);
        }

        switch (m_mode)
        {
            case Mode::PlaceBefore:
                contentReferences.insert(contentReferences.begin(), contentsReference);
                break;

            case Mode::PlaceAfter:
                contentReferences.push_back(contentsReference);
                break;

            default:
                Q_ASSERT(false);
                break;
        }

        PDFObjectFactory pageUpdateFactory;

        pageUpdateFactory.beginDictionary();

        pageUpdateFactory.beginDictionaryItem("Contents");
        pageUpdateFactory << contentReferences;
        pageUpdateFactory.endDictionaryItem();

        pageUpdateFactory.beginDictionaryItem("Resources");
        pageUpdateFactory << resourcesReference;
        pageUpdateFactory.endDictionaryItem();

        pageUpdateFactory.endDictionary();

        m_documentBuilder->mergeTo(m_pageReference, pageUpdateFactory.takeObject());
    }
}

PDFObject PDFPageContentStreamBuilder::removeDictionaryReferencesFromResources(PDFObject resources)
{
    PDFObjectFactory resourcesBuilder;

    resources = m_documentBuilder->getObject(resources);
    if (resources.isDictionary())
    {
        resourcesBuilder.beginDictionary();

        const PDFDictionary* resourcesDictionary = resources.getDictionary();
        const size_t count = resourcesDictionary->getCount();
        for (size_t i = 0; i < count; ++i)
        {
            PDFObject object = m_documentBuilder->getObject(resourcesDictionary->getValue(i));

            if (object.isNull())
            {
                continue;
            }

            resourcesBuilder.beginDictionaryItem(resourcesDictionary->getKey(i).getString());
            resourcesBuilder << object;
            resourcesBuilder.endDictionaryItem();
        }

        resourcesBuilder.endDictionary();
        resources = resourcesBuilder.takeObject();
    }

    return resources;
}

void PDFPageContentStreamBuilder::replaceResources(PDFObjectReference contentStreamReference,
                                                   PDFObjectReference resourcesReference,
                                                   PDFObject oldResources)
{
    PDFObject newResources = m_documentBuilder->getObjectByReference(resourcesReference);
    oldResources = m_documentBuilder->getObject(oldResources);
    PDFObject contentStreamObject = m_documentBuilder->getObjectByReference(contentStreamReference);

    std::vector<std::pair<QByteArray, QByteArray>> renamings;

    if (oldResources.isDictionary() && newResources.isDictionary())
    {
        PDFObjectFactory renamedResourcesDictionary;

        renamedResourcesDictionary.beginDictionary();

        const PDFDictionary* oldResourcesDictionary = oldResources.getDictionary();
        const PDFDictionary* newResourcesDictionary = newResources.getDictionary();
        const size_t count = newResourcesDictionary->getCount();
        for (size_t i = 0; i < count; ++i)
        {
            const PDFInplaceOrMemoryString& key = newResourcesDictionary->getKey(i);
            QByteArray keyString = key.getString();

            // Process current resource key
            renamedResourcesDictionary.beginDictionaryItem(keyString);

            if (oldResourcesDictionary->hasKey(keyString))
            {
                const PDFObject& newResourcesSubdictionaryObject = m_documentBuilder->getObject(newResourcesDictionary->getValue(i));
                const PDFObject& oldResourcesSubdictionaryObject = m_documentBuilder->getObject(oldResourcesDictionary->get(keyString));
                if (oldResourcesSubdictionaryObject.isDictionary() && newResourcesSubdictionaryObject.isDictionary())
                {
                    // Jakub Melka: Rename items, which are in both dictionaries
                    const PDFDictionary* oldSd = oldResourcesSubdictionaryObject.getDictionary();
                    const PDFDictionary* newSd = newResourcesSubdictionaryObject.getDictionary();

                    renamedResourcesDictionary.beginDictionary();

                    const size_t subcount = newSd->getCount();
                    for (size_t j = 0; j < subcount; ++j)
                    {
                        const PDFInplaceOrMemoryString& subkey = newSd->getKey(j);
                        QByteArray subkeyString = subkey.getString();
                        if (oldSd->hasKey(subkeyString))
                        {
                            // Jakub Melka: we must rename the item
                            QByteArray newSubkeyString = subkeyString;
                            int k = 0;
                            while (oldSd->hasKey(newSubkeyString))
                            {
                                newSubkeyString = subkeyString + "_" + QByteArray::number(++k);
                            }
                            renamings.emplace_back(std::make_pair(subkeyString, newSubkeyString));
                            subkeyString = newSubkeyString;
                        }

                        renamedResourcesDictionary.beginDictionaryItem(subkeyString);
                        renamedResourcesDictionary << newSd->getValue(j);
                        renamedResourcesDictionary.endDictionaryItem();
                    }

                    renamedResourcesDictionary.endDictionary();
                }
                else
                {
                    renamedResourcesDictionary << newResourcesDictionary->getValue(i);
                }
            }
            else
            {
                renamedResourcesDictionary << newResourcesDictionary->getValue(i);
            }

            renamedResourcesDictionary.endDictionaryItem();
        }

        renamedResourcesDictionary.endDictionary();
        m_documentBuilder->setObject(resourcesReference, renamedResourcesDictionary.takeObject());
    }

    if (contentStreamObject.isStream())
    {
        QByteArray decodedStream = m_documentBuilder->getDecodedStream(contentStreamObject.getStream());
        decodedStream = decodedStream.trimmed();

        // Append save/restore state
        if (!decodedStream.startsWith("q "))
        {
            decodedStream.prepend("q ");
            decodedStream.append(" Q");
        }

        // Replace all occurences in the stream
        for (const auto& item : renamings)
        {
            QByteArray oldName = item.first;
            QByteArray newName = item.second;

            oldName.prepend('/');
            newName.prepend('/');

            int currentPos = 0;
            int oldNameIndex = decodedStream.indexOf(oldName, currentPos);
            while (oldNameIndex != -1)
            {
                const int whiteSpacePosition = oldNameIndex + oldName.size();
                if (whiteSpacePosition < decodedStream.size() && !PDFLexicalAnalyzer::isWhitespace(decodedStream[whiteSpacePosition]))
                {
                    currentPos = oldNameIndex + 1;
                    oldNameIndex = decodedStream.indexOf(oldName, currentPos);
                    continue;
                }

                decodedStream.replace(oldNameIndex, oldName.length(), newName);
                currentPos = oldNameIndex + 1;
                oldNameIndex = decodedStream.indexOf(oldName, currentPos);
            }
        }

        PDFArray array;
        array.appendItem(PDFObject::createName("FlateDecode"));

        // Compress the content stream
        QByteArray compressedData = PDFFlateDecodeFilter::compress(decodedStream);
        PDFDictionary updatedDictionary = *contentStreamObject.getStream()->getDictionary();
        updatedDictionary.setEntry(PDFInplaceOrMemoryString("Length"), PDFObject::createInteger(compressedData.size()));
        updatedDictionary.setEntry(PDFInplaceOrMemoryString("Filter"), PDFObject::createArray(std::make_shared<PDFArray>(qMove(array))));
        PDFObject newContentStream = PDFObject::createStream(std::make_shared<PDFStream>(qMove(updatedDictionary), qMove(compressedData)));
        m_documentBuilder->setObject(contentStreamReference, std::move(newContentStream));
    }
}

void PDFDocumentBuilder::updateAnnotationAppearanceStreams(PDFObjectReference annotationReference)
{
    PDFAnnotationPtr annotation = PDFAnnotation::parse(&m_storage, annotationReference);
    if (!annotation)
    {
        return;
    }

    const PDFDictionary* pageDictionary = m_storage.getDictionaryFromObject(m_storage.getObject(annotation->getPageReference()));
    if (!pageDictionary)
    {
        return;
    }

    PDFDocumentDataLoaderDecorator loader(&m_storage);
    QRectF mediaBox = loader.readRectangle(pageDictionary->get("MediaBox"), QRectF());
    if (!mediaBox.isValid())
    {
        return;
    }

    std::vector<PDFAppeareanceStreams::Key> keys = annotation->getDrawKeys(m_formManager);
    std::map<PDFAppeareanceStreams::Key, PDFObjectReference> appearanceStreams;

    QRectF boundingRectangle;
    for (const PDFAppeareanceStreams::Key& key : keys)
    {
        PDFContentStreamBuilder builder(mediaBox.size(), PDFContentStreamBuilder::CoordinateSystem::PDF);

        AnnotationDrawParameters parameters;
        parameters.annotation = annotation.data();
        parameters.key = key;
        parameters.painter = builder.begin();
        parameters.formManager = m_formManager;
        annotation->draw(parameters);
        PDFContentStreamBuilder::ContentStream contentStream = builder.end(parameters.painter);

        if (!parameters.boundingRectangle.isValid() || !contentStream.pageObject.isValid())
        {
            // Invalid stream, do nothing
            continue;
        }

        boundingRectangle = boundingRectangle.united(parameters.boundingRectangle);

        std::vector<PDFObject> copiedObjects = copyFrom({ contentStream.resources, contentStream.contents }, contentStream.document.getStorage(), true);
        Q_ASSERT(copiedObjects.size() == 2);

        PDFObjectReference resourcesReference = copiedObjects[0].getReference();
        PDFObjectReference formReference = copiedObjects[1].getReference();

        // Create form object
        PDFObjectFactory formFactory;

        formFactory.beginDictionary();

        formFactory.beginDictionaryItem("Type");
        formFactory << WrapName("XObject");
        formFactory.endDictionaryItem();

        formFactory.beginDictionaryItem("Subtype");
        formFactory << WrapName("Form");
        formFactory.endDictionaryItem();

        formFactory.beginDictionaryItem("BBox");
        formFactory << parameters.boundingRectangle;
        formFactory.endDictionaryItem();

        formFactory.beginDictionaryItem("Resources");
        formFactory << resourcesReference;
        formFactory.endDictionaryItem();

        formFactory.endDictionary();

        mergeTo(formReference, formFactory.takeObject());
        appearanceStreams[key] = formReference;
    }

    if (!appearanceStreams.empty())
    {
        PDFObjectFactory asDictionaryFactory;
        asDictionaryFactory.beginDictionary();

        auto it = appearanceStreams.cbegin();
        while (it != appearanceStreams.cend())
        {
            const PDFAppeareanceStreams::Key& key = it->first;

            auto itEnd = std::next(it);
            while (itEnd != appearanceStreams.cend() && itEnd->first.first == key.first)
            {
                ++itEnd;
            }

            QByteArray name;
            switch (key.first)
            {
                case PDFAppeareanceStreams::Appearance::Normal:
                    name = "N";
                    break;
                case PDFAppeareanceStreams::Appearance::Rollover:
                    name = "R";
                    break;
                case PDFAppeareanceStreams::Appearance::Down:
                    name = "D";
                    break;

                default:
                    Q_ASSERT(false);
                    break;
            }

            asDictionaryFactory.beginDictionaryItem(name);
            const size_t streamCount = std::distance(it, itEnd);
            if (streamCount == 1)
            {
                asDictionaryFactory << it->second;
            }
            else
            {
                asDictionaryFactory.beginDictionary();
                for (; it != itEnd; ++it)
                {
                    asDictionaryFactory.beginDictionaryItem(it->first.second);
                    asDictionaryFactory << it->second;
                    asDictionaryFactory.endDictionaryItem();
                }
                asDictionaryFactory.endDictionary();
            }
            asDictionaryFactory.endDictionaryItem();

            it = itEnd;
        }

        asDictionaryFactory.endDictionary();

        PDFObjectFactory annotationFactory;
        annotationFactory.beginDictionary();

        annotationFactory.beginDictionaryItem("Rect");
        annotationFactory << boundingRectangle;
        annotationFactory.endDictionaryItem();

        annotationFactory.beginDictionaryItem("AP");
        annotationFactory << asDictionaryFactory.takeObject();
        annotationFactory.endDictionaryItem();

        annotationFactory.endDictionary();

        mergeTo(annotationReference, annotationFactory.takeObject());
    }
}

PDFObjectReference PDFDocumentBuilder::addObject(PDFObject object)
{
    return m_storage.addObject(PDFObjectManipulator::removeNullObjects(object));
}

void PDFDocumentBuilder::mergeTo(PDFObjectReference reference, PDFObject object)
{
    m_storage.setObject(reference, PDFObjectManipulator::merge(m_storage.getObject(reference), qMove(object), PDFObjectManipulator::RemoveNullObjects));
}

void PDFDocumentBuilder::setObject(PDFObjectReference reference, PDFObject object)
{
    m_storage.setObject(reference, qMove(object));
}

std::vector<PDFObjectReference> PDFDocumentBuilder::createDocumentParts(const std::vector<size_t>& parts)
{
    std::vector<PDFObjectReference> documentParts;

    PDFObjectReference root = createDocumentPartRoot();
    std::vector<PDFObjectReference> pages = getPages();

    PDFObjectFactory objectFactory;
    objectFactory.beginDictionary();
    objectFactory.beginDictionaryItem("DParts");
    objectFactory.beginArray();

    documentParts.reserve(parts.size());

    size_t start = 0;
    for (std::size_t count : parts)
    {
        if (count == 0)
        {
            continue;
        }

        auto itStart = std::next(pages.cbegin(), start);
        auto itEnd = std::next(itStart, count);

        PDFObjectReference item = createDocumentPartItem(*itStart, *std::prev(itEnd), root);
        for (auto it = itStart; it != itEnd; ++it)
        {
            setPageDocumentPart(*it, item);
        }

        documentParts.push_back(item);

        objectFactory.beginArray();
        objectFactory << item;
        objectFactory.endArray();

        start += count;
    }

    objectFactory.endArray();
    objectFactory.endDictionaryItem();
    objectFactory.endDictionary();

    mergeTo(root, objectFactory.takeObject());
    return documentParts;
}

void PDFDocumentBuilder::mergeNames(PDFObjectReference a, PDFObjectReference b)
{
    PDFObject aObject = getObjectByReference(a);

    // First object is null, do nothing
    if (aObject.isNull())
    {
        setObject(a, getObjectByReference(b));
        return;
    }

    // Jakub Melka: otherwise we must merge names tree
    PDFObject bObject = getObjectByReference(b);
    const PDFDictionary* aDict = getDictionaryFromObject(aObject);
    const PDFDictionary* bDict = getDictionaryFromObject(bObject);

    // Store keys
    std::set<QByteArray> keys;
    for (size_t i = 0; i < aDict->getCount(); ++i)
    {
        keys.insert(aDict->getKey(i).getString());
    }
    for (size_t i = 0; i < bDict->getCount(); ++i)
    {
        keys.insert(bDict->getKey(i).getString());
    }

    PDFObjectFactory factory;
    factory.beginDictionary();

    for (const QByteArray& key : keys)
    {
        auto getObject = [](const PDFObjectStorage*, const PDFObject& object) { return object; };
        auto aMap = PDFNameTreeLoader<PDFObject>::parse(&m_storage, aDict->get(key), getObject);
        auto bMap = PDFNameTreeLoader<PDFObject>::parse(&m_storage, bDict->get(key), getObject);
        aMap.merge(qMove(bMap));

        if (aMap.empty())
        {
            continue;
        }

        factory.beginDictionaryItem(key);
        factory.beginDictionary();

        factory.beginDictionaryItem("Names");
        factory.beginArray();

        for (const auto& item : aMap)
        {
            factory << WrapName(item.first);
            factory << item.second;
        }

        factory.endArray();
        factory.endDictionaryItem();

        factory.beginDictionaryItem("Limits");
        factory.beginArray();

        factory << WrapName((*aMap.begin()).first);
        factory << WrapName((*aMap.rbegin()).first);

        factory.endArray();
        factory.endDictionaryItem();

        factory.endDictionary();
        factory.endDictionaryItem();
    }

    factory.endDictionary();
    setObject(a, factory.takeObject());
}

void PDFDocumentBuilder::appendTo(PDFObjectReference reference, PDFObject object)
{
    m_storage.setObject(reference, PDFObjectManipulator::merge(m_storage.getObject(reference), qMove(object), PDFObjectManipulator::ConcatenateArrays));
}

QRectF PDFDocumentBuilder::getPopupWindowRect(const QRectF& rectangle) const
{
    QRectF rect = rectangle.translated(rectangle.width() * 1.25, 0);
    rect.setSize(QSizeF(100, 100));
    return rect;
}

QString PDFDocumentBuilder::getProducerString() const
{
    return PDF_LIBRARY_NAME;
}

PDFObjectReference PDFDocumentBuilder::getPageTreeRoot() const
{
    if (const PDFDictionary* trailerDictionary = getDictionaryFromObject(m_storage.getTrailerDictionary()))
    {
        if (const PDFDictionary* catalogDictionary = getDictionaryFromObject(trailerDictionary->get("Root")))
        {
            PDFObject pagesRoot = catalogDictionary->get("Pages");
            if (pagesRoot.isReference())
            {
                return pagesRoot.getReference();
            }
        }
    }

    return PDFObjectReference();
}

PDFInteger PDFDocumentBuilder::getPageTreeRootChildCount() const
{
    if (const PDFDictionary* pageTreeRootDictionary = getDictionaryFromObject(getObjectByReference(getPageTreeRoot())))
    {
        PDFObject childCountObject = getObject(pageTreeRootDictionary->get("Count"));
        if (childCountObject.isInt())
        {
            return childCountObject.getInteger();
        }
    }

    return 0;
}

PDFObjectReference PDFDocumentBuilder::getDocumentInfo() const
{
    if (const PDFDictionary* trailerDictionary = getDictionaryFromObject(m_storage.getTrailerDictionary()))
    {
        PDFObject object = trailerDictionary->get("Info");
        if (object.isReference())
        {
            return object.getReference();
        }
    }

    return PDFObjectReference();
}

void PDFDocumentBuilder::copyAnnotation(PDFObjectReference pageReference, PDFObjectReference annotationReference)
{
    PDFObjectReference copiedAnnotation = addObject(getObjectByReference(annotationReference));

    PDFObjectFactory factory;
    factory.beginDictionary();
    factory.beginDictionaryItem("P");
    factory << pageReference;
    factory.endDictionaryItem();
    factory.beginDictionaryItem("Popup");
    factory << PDFObject();
    factory.endDictionaryItem();
    factory.beginDictionaryItem("IRT");
    factory << PDFObject();
    factory.endDictionaryItem();
    factory.endDictionary();

    mergeTo(copiedAnnotation, factory.takeObject());

    factory.beginDictionary();
    factory.beginDictionaryItem("Annots");
    factory.beginArray();
    factory << copiedAnnotation;
    factory.endArray();
    factory.endDictionaryItem();
    factory.endDictionary();
    PDFObject pageAnnots = factory.takeObject();
    appendTo(pageReference, pageAnnots);
}

void PDFDocumentBuilder::setSecurityHandler(PDFSecurityHandlerPointer handler)
{
    if (!handler)
    {
        handler.reset(new PDFNoneSecurityHandler());
    }

    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Encrypt");

    PDFObject encryptionDictionaryObject = handler->createEncryptionDictionaryObject();
    Q_ASSERT(!encryptionDictionaryObject.isReference());

    if (!encryptionDictionaryObject.isNull())
    {
        encryptionDictionaryObject = PDFObject::createReference(addObject(encryptionDictionaryObject));
    }

    objectBuilder << encryptionDictionaryObject;

    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedTrailerDictionary = objectBuilder.takeObject();
    m_storage.updateTrailerDictionary(qMove(updatedTrailerDictionary));

    m_storage.setSecurityHandler(qMove(handler));
}

PDFObjectReference PDFDocumentBuilder::getCatalogReference() const
{
    if (const PDFDictionary* trailerDictionary = getDictionaryFromObject(m_storage.getTrailerDictionary()))
    {
        PDFObject object = trailerDictionary->get("Root");
        if (object.isReference())
        {
            return object.getReference();
        }
    }

    return PDFObjectReference();
}

void PDFDocumentBuilder::removeAnnotation(PDFObjectReference page, PDFObjectReference annotation)
{
    PDFDocumentDataLoaderDecorator loader(&m_storage);

    if (const PDFDictionary* pageDictionary = m_storage.getDictionaryFromObject(m_storage.getObjectByReference(page)))
    {
        std::vector<PDFObjectReference> annots = loader.readReferenceArrayFromDictionary(pageDictionary, "Annots");
        annots.erase(std::remove(annots.begin(), annots.end(), annotation), annots.end());

        PDFObjectFactory factory;
        factory.beginDictionary();
        factory.beginDictionaryItem("Annots");
        if (!annots.empty())
        {
            factory << annots;
        }
        else
        {
            factory << PDFObject();
        }
        factory.endDictionaryItem();
        factory.endDictionary();

        mergeTo(page, factory.takeObject());
    }

    setObject(annotation, PDFObject());
}

void PDFDocumentBuilder::updateDocumentInfo(PDFObject info)
{
    PDFObjectReference infoReference = getDocumentInfo();
    if (!infoReference.isValid())
    {
        PDFObjectFactory objectFactory;
        objectFactory.beginDictionary();
        objectFactory.endDictionary();
        infoReference = addObject(objectFactory.takeObject());

        // Update the trailer dictionary
        objectFactory.beginDictionary();
        objectFactory.beginDictionaryItem("Info");
        objectFactory << infoReference;
        objectFactory.endDictionaryItem();
        objectFactory.endDictionary();
        m_storage.updateTrailerDictionary(objectFactory.takeObject());
    }

    mergeTo(infoReference, info);
}

void PDFDocumentBuilder::setDocumentInfo(PDFObjectReference infoReference)
{
    // Update the trailer dictionary
    PDFObjectFactory objectFactory;
    objectFactory.beginDictionary();
    objectFactory.beginDictionaryItem("Info");
    objectFactory << infoReference;
    objectFactory.endDictionaryItem();
    objectFactory.endDictionary();
    m_storage.updateTrailerDictionary(objectFactory.takeObject());
}

QRectF PDFDocumentBuilder::getPolygonsBoundingRect(const Polygons& polygons) const
{
    QRectF rect;

    for (const QPolygonF& polygon : polygons)
    {
        rect = rect.united(polygon.boundingRect());
    }

    return rect;
}

PDFObjectReference PDFDocumentBuilder::createOutlineItem(const PDFOutlineItem* root, bool writeOutlineData)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();

    if (writeOutlineData)
    {
        // Title
        objectBuilder.beginDictionaryItem("Title");
        objectBuilder << root->getTitle();
        objectBuilder.endDictionaryItem();

        // Destination
        const PDFActionGoTo* actionGoTo = dynamic_cast<const PDFActionGoTo*>(root->getAction());
        if (actionGoTo)
        {
            objectBuilder.beginDictionaryItem("Dest");
            objectBuilder << actionGoTo->getDestination();
            objectBuilder.endDictionaryItem();
        }
        const PDFActionGoToDp* actionGoToDp = dynamic_cast<const PDFActionGoToDp*>(root->getAction());
        if (actionGoToDp)
        {
            objectBuilder.beginDictionaryItem("A");

            objectBuilder.beginDictionary();
            objectBuilder.beginDictionaryItem("S");
            objectBuilder << WrapName("GoToDp");
            objectBuilder.endDictionaryItem();
            objectBuilder.beginDictionaryItem("Dp");
            objectBuilder << actionGoToDp->getDocumentPart();
            objectBuilder.endDictionaryItem();
            objectBuilder.endDictionary();

            objectBuilder.endDictionaryItem();
        }

        // Color
        if (root->getTextColor().isValid() && root->getTextColor() != Qt::black)
        {
            objectBuilder.beginDictionaryItem("C");
            objectBuilder << root->getTextColor();
            objectBuilder.endDictionaryItem();
        }

        // Flags
        PDFInteger flags = 0;
        if (root->isFontItalics())
        {
            flags += 1;
        }
        if (root->isFontBold())
        {
            flags += 2;
        }

        if (flags > 0)
        {
            objectBuilder.beginDictionaryItem("F");
            objectBuilder << flags;
            objectBuilder.endDictionaryItem();
        }
    }

    // Create descendands
    std::vector<PDFObjectReference> children;
    children.reserve(root->getChildCount());
    for (size_t i = 0; i < root->getChildCount(); ++i)
    {
        children.push_back(createOutlineItem(root->getChild(i), true));
    }

    if (!children.empty())
    {
        // First/Last pointers
        objectBuilder.beginDictionaryItem("First");
        objectBuilder << children.front();
        objectBuilder.endDictionaryItem();
        objectBuilder.beginDictionaryItem("Last");
        objectBuilder << children.back();
        objectBuilder.endDictionaryItem();
    }

    size_t totalCount = root->getTotalCount();
    if (totalCount > 0)
    {
        objectBuilder.beginDictionaryItem("Count");
        objectBuilder << PDFInteger(totalCount);
        objectBuilder.endDictionaryItem();
    }

    objectBuilder.endDictionary();
    PDFObjectReference parentReference = addObject(objectBuilder.takeObject());

    for (size_t i = 0; i < children.size(); ++i)
    {
        PDFObjectFactory fixPointersObjectBuilder;
        fixPointersObjectBuilder.beginDictionary();

        fixPointersObjectBuilder.beginDictionaryItem("Parent");
        fixPointersObjectBuilder << parentReference;
        fixPointersObjectBuilder.endDictionaryItem();

        if (i > 0)
        {
            fixPointersObjectBuilder.beginDictionaryItem("Prev");
            fixPointersObjectBuilder << children[i - 1];
            fixPointersObjectBuilder.endDictionaryItem();
        }

        if (i + 1 < children.size())
        {
            fixPointersObjectBuilder.beginDictionaryItem("Next");
            fixPointersObjectBuilder << children[i + 1];
            fixPointersObjectBuilder.endDictionaryItem();
        }

        fixPointersObjectBuilder.endDictionary();
        mergeTo(children[i], fixPointersObjectBuilder.takeObject());
    }

    return parentReference;
}

void PDFDocumentBuilder::flattenPageTree()
{
    PDFObjectReference pageTreeRoot = getPageTreeRoot();
    PDFObject pageTree = PDFObject::createReference(pageTreeRoot);
    std::vector<PDFPage> pages = PDFPage::parse(&m_storage, pageTree);
    std::vector<PDFObjectReference> pageReferences;

    // First, fill inheritable attributes to pages and correct parent
    for (const PDFPage& page : pages)
    {
        PDFObjectFactory objectBuilder;

        objectBuilder.beginDictionary();

        objectBuilder.beginDictionaryItem("Parent");
        objectBuilder << pageTreeRoot;
        objectBuilder.endDictionaryItem();

        objectBuilder.beginDictionaryItem("MediaBox");
        objectBuilder << page.getMediaBox();
        objectBuilder.endDictionaryItem();

        if (page.getCropBox().isValid())
        {
            objectBuilder.beginDictionaryItem("CropBox");
            objectBuilder << page.getCropBox();
            objectBuilder.endDictionaryItem();
        }

        if (!page.getResources().isNull())
        {
            objectBuilder.beginDictionaryItem("Resources");
            objectBuilder << page.getResources();
            objectBuilder.endDictionaryItem();
        }

        if (page.getPageRotation() != PageRotation::None)
        {
            PDFInteger angle = 0;
            switch (page.getPageRotation())
            {
                case PageRotation::Rotate90:
                    angle = 90;
                    break;

                case PageRotation::Rotate180:
                    angle = 180;
                    break;

                case PageRotation::Rotate270:
                    angle = 270;
                    break;

                default:
                    break;
            }

            objectBuilder.beginDictionaryItem("Rotate");
            objectBuilder << angle;
            objectBuilder.endDictionaryItem();
        }

        objectBuilder.endDictionary();
        mergeTo(page.getPageReference(), objectBuilder.takeObject());
        pageReferences.push_back(page.getPageReference());
    }

    setPages(pageReferences);
}

void PDFDocumentBuilder::setPages(const std::vector<PDFObjectReference>& pageReferences)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();

    objectBuilder.beginDictionaryItem("Kids");
    objectBuilder.beginArray();
    for (const PDFObjectReference& pageReference : pageReferences)
    {
        objectBuilder << pageReference;
    }
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();

    objectBuilder.beginDictionaryItem("Count");
    objectBuilder << PDFInteger(pageReferences.size());
    objectBuilder.endDictionaryItem();

    objectBuilder.endDictionary();

    mergeTo(getPageTreeRoot(), objectBuilder.takeObject());
}

std::vector<PDFObjectReference> PDFDocumentBuilder::getPages() const
{
    std::vector<PDFObjectReference> result;

    if (const PDFDictionary* pageTreeRoot = m_storage.getDictionaryFromObject(m_storage.getObject(getPageTreeRoot())))
    {
        PDFDocumentDataLoaderDecorator loader(&m_storage);
        result = loader.readReferenceArrayFromDictionary(pageTreeRoot, "Kids");
    }

    return result;
}

void PDFDocumentBuilder::setOutline(const PDFOutlineItem* root)
{
    setOutline(createOutlineItem(root, false));
}

std::vector<PDFObject> PDFDocumentBuilder::copyFrom(const std::vector<PDFObject>& objects, const PDFObjectStorage& storage, bool createReferences)
{
    // 1) Collect all references, which we must copy. If object is referenced, then
    //    we must also collect references of referenced object.
    std::set<PDFObjectReference> references = PDFObjectUtils::getReferences(objects, storage);

    // 2) Make room for new objects, together with mapping
    std::map<PDFObjectReference, PDFObjectReference> referenceMapping;
    for (const PDFObjectReference& reference : references)
    {
        referenceMapping[reference] = addObject(PDFObject::createNull());
    }

    // 3) Copy objects from other object to this one
    for (const PDFObjectReference& sourceReference : references)
    {
        const PDFObjectReference targetReference = referenceMapping.at(sourceReference);
        m_storage.setObject(targetReference, PDFObjectUtils::replaceReferences(storage.getObject(sourceReference), referenceMapping));
    }

    std::vector<PDFObject> result;
    result.reserve(objects.size());

    for (const PDFObject& object : objects)
    {
        if (object.isReference())
        {
            result.push_back(PDFObject::createReference(referenceMapping.at(object.getReference())));
        }
        else
        {
            PDFObject replacedObject = PDFObjectUtils::replaceReferences(object, referenceMapping);

            if (createReferences)
            {
                result.push_back(PDFObject::createReference(addObject(qMove(replacedObject))));
            }
            else
            {
                result.emplace_back(qMove(replacedObject));
            }
        }
    }

    return result;
}

std::vector<PDFObject> PDFDocumentBuilder::createObjectsFromReferences(const std::vector<PDFObjectReference>& references)
{
    std::vector<PDFObject> result;
    std::transform(references.cbegin(), references.cend(), std::back_inserter(result), [](const PDFObjectReference& reference) { return PDFObject::createReference(reference); });
    return result;
}

std::vector<PDFObjectReference> PDFDocumentBuilder::createReferencesFromObjects(const std::vector<PDFObject>& objects)
{
    std::vector<PDFObjectReference> result;
    std::transform(objects.cbegin(), objects.cend(), std::back_inserter(result), [](const PDFObject& object) { Q_ASSERT(object.isReference()); return object.getReference(); });
    return result;
}

const PDFFormManager* PDFDocumentBuilder::getFormManager() const
{
    return m_formManager;
}

void PDFDocumentBuilder::setFormManager(const PDFFormManager* formManager)
{
    m_formManager = formManager;
}

PDFContentStreamBuilder::PDFContentStreamBuilder(QSizeF size, CoordinateSystem coordinateSystem) :
    m_size(size),
    m_coordinateSystem(coordinateSystem),
    m_buffer(nullptr),
    m_pdfWriter(nullptr),
    m_painter(nullptr)
{

}

PDFContentStreamBuilder::~PDFContentStreamBuilder()
{
    Q_ASSERT(!m_buffer);
    Q_ASSERT(!m_pdfWriter);
    Q_ASSERT(!m_painter);
}

QPainter* PDFContentStreamBuilder::begin()
{
    Q_ASSERT(!m_buffer);
    Q_ASSERT(!m_pdfWriter);
    Q_ASSERT(!m_painter);

    m_buffer = new QBuffer();
    m_buffer->open(QBuffer::ReadWrite);

    m_pdfWriter = new QPdfWriter(m_buffer);
    m_pdfWriter->setPdfVersion(QPdfWriter::PdfVersion_1_6);
    m_pdfWriter->setPageSize(QPageSize(m_size, QPageSize::Point));
    m_pdfWriter->setResolution(72);
    m_pdfWriter->setPageMargins(QMarginsF());

    m_painter = new QPainter(m_pdfWriter);

    if (m_coordinateSystem == CoordinateSystem::PDF)
    {
        m_painter->translate(0, m_size.height());
        m_painter->scale(1.0, -1.0);
    }

    return m_painter;
}

PDFContentStreamBuilder::ContentStream PDFContentStreamBuilder::end(QPainter* painter)
{
    ContentStream result;
    Q_ASSERT(m_painter == painter);

    m_painter->end();
    delete m_painter;
    m_painter = nullptr;

    delete m_pdfWriter;
    m_pdfWriter = nullptr;

    QByteArray bufferData = m_buffer->buffer();
    m_buffer->close();
    delete m_buffer;
    m_buffer = nullptr;

    PDFDocumentReader reader(nullptr, nullptr, false, false);
    result.document = reader.readFromBuffer(bufferData);

    if (result.document.getCatalog()->getPageCount() > 0)
    {
        const PDFPage* page = result.document.getCatalog()->getPage(0);
        result.pageObject = page->getPageReference();
        result.resources = page->getResources();
        result.contents = page->getContents();
    }

    return result;
}

PDFDocumentModifier::PDFDocumentModifier(const PDFDocument* originalDocument) :
    m_originalDocument(originalDocument),
    m_builder(originalDocument)
{

}

bool PDFDocumentModifier::finalize()
{
    PDFDocument document = m_builder.build();
    if (document != *m_originalDocument)
    {
        m_modifiedDocument.reset(new PDFDocument(qMove(document)));
        return true;
    }

    return false;
}

/* START GENERATED CODE */

PDFObjectReference PDFDocumentBuilder::appendPage(QRectF mediaBox)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Page");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Parent");
    objectBuilder << getPageTreeRoot();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionary();
    objectBuilder.endDictionary();
    objectBuilder.beginDictionaryItem("MediaBox");
    objectBuilder << mediaBox;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference pageReference = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Kids");
    objectBuilder << std::initializer_list<PDFObjectReference>{ pageReference };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Count");
    objectBuilder << getPageTreeRootChildCount() + 1;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedTreeRoot = objectBuilder.takeObject();
    appendTo(getPageTreeRoot(), updatedTreeRoot);
    return pageReference;
}


PDFObjectReference PDFDocumentBuilder::createAcroForm(PDFObjectReferenceVector fields)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Fields");
    objectBuilder << fields;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("NeedAppearances");
    objectBuilder << false;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("SigFlags");
    objectBuilder << 0;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("XFA");
    objectBuilder << PDFObject();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference acroForm = addObject(objectBuilder.takeObject());
    setCatalogAcroForm(acroForm);
    return acroForm;
}


PDFObjectReference PDFDocumentBuilder::createActionGoTo(PDFDestination destination)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("GoTo");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("D");
    objectBuilder << destination;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionGoToDocumentPart(PDFObjectReference documentPart)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("GoToDp");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Dp");
    objectBuilder << documentPart;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionGoToEmbedded(PDFObjectReference fileSpecification,
                                                                PDFDestination destination,
                                                                bool newWindow)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("GoToE");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << fileSpecification;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("D");
    objectBuilder << destination;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("NewWindow");
    objectBuilder << newWindow;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionGoToRemote(PDFObjectReference fileSpecification,
                                                              PDFDestination destination,
                                                              bool newWindow)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("GoToR");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << fileSpecification;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("D");
    objectBuilder << destination;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("NewWindow");
    objectBuilder << newWindow;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionHide(PDFObjectReference annotation,
                                                        bool hide)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Hide");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << annotation;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("H");
    objectBuilder << hide;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionHide(QString field,
                                                        bool hide)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Hide");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << field;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("H");
    objectBuilder << hide;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionImportData(PDFObjectReference fileSpecification)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("ImportData");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << fileSpecification;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionJavaScript(QString code)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("JavaScript");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("JS");
    objectBuilder << code;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionLaunch(PDFObjectReference fileSpecification,
                                                          bool newWindow)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Launch");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << fileSpecification;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("NewWindow");
    objectBuilder << newWindow;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionLaunchWin(QByteArray fileName,
                                                             QByteArray defaultDirectory,
                                                             QByteArray action,
                                                             QByteArray parameters,
                                                             bool newWindow)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Launch");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Win");
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << fileName;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("D");
    objectBuilder << defaultDirectory;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("O");
    objectBuilder << action;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << parameters;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("NewWindow");
    objectBuilder << newWindow;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionNamed(QByteArray name)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Named");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("N");
    objectBuilder << WrapName(name);
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionNavigateFirstPage()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Named");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("N");
    objectBuilder << WrapName("FirstPage");
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionNavigateLastPage()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Named");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("N");
    objectBuilder << WrapName("LastPage");
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionNavigateNextPage()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Named");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("N");
    objectBuilder << WrapName("NextPage");
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionNavigatePrevPage()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Named");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("N");
    objectBuilder << WrapName("PrevPage");
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionResetForm()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("ResetForm");
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionResetFormExcludedFields(PDFObjectReferenceVector fields)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("ResetForm");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Fields");
    objectBuilder << fields;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Flags");
    objectBuilder << 1;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionResetFormFields(PDFObjectReferenceVector fields)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("ResetForm");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Fields");
    objectBuilder << fields;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionSubmitForm(QString URL,
                                                              PDFFormSubmitFlags flags)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("SubmitForm");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("UF");
    objectBuilder << URL;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Flags");
    objectBuilder << flags;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionSubmitForm(QString URL,
                                                              PDFObjectReferenceVector fields,
                                                              PDFFormSubmitFlags flags)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("SubmitForm");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("UF");
    objectBuilder << URL;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Fields");
    objectBuilder << fields;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Flags");
    objectBuilder << flags;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionThread(PDFObjectReference thread,
                                                          PDFObjectReference bead)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Thread");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("D");
    objectBuilder << thread;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("B");
    objectBuilder << bead;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionThread(PDFObjectReference fileSpecification,
                                                          PDFInteger thread,
                                                          PDFInteger bead)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Thread");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << fileSpecification;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("D");
    objectBuilder << thread;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("B");
    objectBuilder << bead;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionThread(PDFObjectReference fileSpecification,
                                                          PDFInteger thread)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Thread");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << fileSpecification;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("D");
    objectBuilder << thread;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionThread(PDFObjectReference thread)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("Thread");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("D");
    objectBuilder << thread;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createActionURI(QString URL)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Action");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << WrapName("URI");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("URI");
    objectBuilder << URL;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference actionReference = addObject(objectBuilder.takeObject());
    return actionReference;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationCaret(PDFObjectReference page,
                                                             QRectF rectangle,
                                                             PDFReal borderWidth,
                                                             QColor color,
                                                             QString title,
                                                             QString subject,
                                                             QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Caret");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, borderWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(color);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationCircle(PDFObjectReference page,
                                                              QRectF rectangle,
                                                              PDFReal borderWidth,
                                                              QColor fillColor,
                                                              QColor strokeColor,
                                                              QString title,
                                                              QString subject,
                                                              QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Circle");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, borderWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(strokeColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("IC");
    objectBuilder << WrapAnnotationColor(fillColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationFileAttachment(PDFObjectReference page,
                                                                      QPointF position,
                                                                      PDFObjectReference fileSpecification,
                                                                      FileAttachmentIcon icon,
                                                                      QString title,
                                                                      QString description)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("FileAttachment");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << QRectF(position, QSizeF(32, 32));
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("FS");
    objectBuilder << fileSpecification;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Name");
    objectBuilder << icon;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << description;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationReference = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationReference;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationReference);
    return annotationReference;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationFreeText(PDFObjectReference page,
                                                                QRectF boundingRectangle,
                                                                QRectF textRectangle,
                                                                QString title,
                                                                QString subject,
                                                                QString contents,
                                                                TextAlignment textAlignment,
                                                                QPointF startPoint,
                                                                QPointF kneePoint,
                                                                QPointF endPoint,
                                                                AnnotationLineEnding startLineType,
                                                                AnnotationLineEnding endLineType)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("FreeText");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << boundingRectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Q");
    objectBuilder << WrapFreeTextAlignment(textAlignment);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("DA");
    objectBuilder << WrapString("/Arial 10 Tf");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("RD");
    objectBuilder << getAnnotationReductionRectangle(boundingRectangle, textRectangle);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CL");
    objectBuilder.beginArray();
    objectBuilder << startPoint;
    objectBuilder << kneePoint;
    objectBuilder << endPoint;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("LE");
    objectBuilder.beginArray();
    objectBuilder << startLineType;
    objectBuilder << endLineType;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationFreeText(PDFObjectReference page,
                                                                QRectF rectangle,
                                                                QString title,
                                                                QString subject,
                                                                QString contents,
                                                                TextAlignment textAlignment)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("FreeText");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Q");
    objectBuilder << WrapFreeTextAlignment(textAlignment);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("DA");
    objectBuilder << WrapString("/Arial 10 Tf");
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationFreeText(PDFObjectReference page,
                                                                QRectF boundingRectangle,
                                                                QRectF textRectangle,
                                                                QString title,
                                                                QString subject,
                                                                QString contents,
                                                                TextAlignment textAlignment,
                                                                QPointF startPoint,
                                                                QPointF endPoint,
                                                                AnnotationLineEnding startLineType,
                                                                AnnotationLineEnding endLineType)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("FreeText");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << boundingRectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Q");
    objectBuilder << WrapFreeTextAlignment(textAlignment);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("DA");
    objectBuilder << WrapString("/Arial 10 Tf");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("RD");
    objectBuilder << getAnnotationReductionRectangle(boundingRectangle, textRectangle);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CL");
    objectBuilder.beginArray();
    objectBuilder << startPoint;
    objectBuilder << endPoint;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("LE");
    objectBuilder.beginArray();
    objectBuilder << startLineType;
    objectBuilder << endLineType;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationHighlight(PDFObjectReference page,
                                                                 QRectF rectangle,
                                                                 QColor color,
                                                                 QString title,
                                                                 QString subject,
                                                                 QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Highlight");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationHighlight(PDFObjectReference page,
                                                                 QRectF rectangle,
                                                                 QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Highlight");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationHighlight(PDFObjectReference page,
                                                                 QPolygonF quadrilaterals,
                                                                 QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Highlight");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder << quadrilaterals;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationInk(PDFObjectReference page,
                                                           QPolygonF inkPoints,
                                                           PDFReal borderWidth,
                                                           QColor strokeColor,
                                                           QString title,
                                                           QString subject,
                                                           QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Ink");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << inkPoints.boundingRect();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("InkList");
    objectBuilder.beginArray();
    objectBuilder << inkPoints;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, borderWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(strokeColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationInk(PDFObjectReference page,
                                                           Polygons inkPoints,
                                                           PDFReal borderWidth,
                                                           QColor strokeColor,
                                                           QString title,
                                                           QString subject,
                                                           QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Ink");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << getPolygonsBoundingRect(inkPoints);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("InkList");
    objectBuilder << inkPoints;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, borderWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(strokeColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationLine(PDFObjectReference page,
                                                            QRectF boundingRect,
                                                            QPointF startPoint,
                                                            QPointF endPoint,
                                                            PDFReal lineWidth,
                                                            QColor fillColor,
                                                            QColor strokeColor,
                                                            QString title,
                                                            QString subject,
                                                            QString contents,
                                                            AnnotationLineEnding startLineType,
                                                            AnnotationLineEnding endLineType)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Line");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << boundingRect;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("L");
    objectBuilder.beginArray();
    objectBuilder << startPoint.x();
    objectBuilder << startPoint.y();
    objectBuilder << endPoint.x();
    objectBuilder << endPoint.y();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("LE");
    objectBuilder.beginArray();
    objectBuilder << startLineType;
    objectBuilder << endLineType;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, lineWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(strokeColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("IC");
    objectBuilder << WrapAnnotationColor(fillColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationLine(PDFObjectReference page,
                                                            QRectF boundingRect,
                                                            QPointF startPoint,
                                                            QPointF endPoint,
                                                            PDFReal lineWidth,
                                                            QColor fillColor,
                                                            QColor strokeColor,
                                                            QString title,
                                                            QString subject,
                                                            QString contents,
                                                            AnnotationLineEnding startLineType,
                                                            AnnotationLineEnding endLineType,
                                                            PDFReal leaderLineLength,
                                                            PDFReal leaderLineOffset,
                                                            PDFReal leaderLineExtension,
                                                            bool displayContents,
                                                            bool displayedContentsTopAlign)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Line");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << boundingRect;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("L");
    objectBuilder.beginArray();
    objectBuilder << startPoint.x();
    objectBuilder << startPoint.y();
    objectBuilder << endPoint.x();
    objectBuilder << endPoint.y();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("LE");
    objectBuilder.beginArray();
    objectBuilder << startLineType;
    objectBuilder << endLineType;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, lineWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(strokeColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("IC");
    objectBuilder << WrapAnnotationColor(fillColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("LL");
    objectBuilder << leaderLineLength;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("LLO");
    objectBuilder << leaderLineOffset;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("LLE");
    objectBuilder << leaderLineExtension;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Cap");
    objectBuilder << displayContents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CP");
    objectBuilder << (displayedContentsTopAlign ? WrapName("Top") : WrapName("Inline"));
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationLink(PDFObjectReference page,
                                                            QRectF linkRectangle,
                                                            QString URL,
                                                            LinkHighlightMode highlightMode)
{
    PDFObjectFactory objectBuilder;

    return createAnnotationLink(page, linkRectangle, createActionURI(URL), highlightMode);
}


PDFObjectReference PDFDocumentBuilder::createAnnotationLink(PDFObjectReference page,
                                                            QRectF linkRectangle,
                                                            PDFObjectReference action,
                                                            LinkHighlightMode highlightMode)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Link");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << linkRectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("A");
    objectBuilder << action;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("H");
    objectBuilder << highlightMode;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationReference = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationReference;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationReference);
    return annotationReference;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationPolygon(PDFObjectReference page,
                                                               QPolygonF polygon,
                                                               PDFReal borderWidth,
                                                               QColor fillColor,
                                                               QColor strokeColor,
                                                               QString title,
                                                               QString subject,
                                                               QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Polygon");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << polygon.boundingRect();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Vertices");
    objectBuilder << polygon;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, borderWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(strokeColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("IC");
    objectBuilder << WrapAnnotationColor(fillColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationPolyline(PDFObjectReference page,
                                                                QPolygonF polyline,
                                                                PDFReal borderWidth,
                                                                QColor fillColor,
                                                                QColor strokeColor,
                                                                QString title,
                                                                QString subject,
                                                                QString contents,
                                                                AnnotationLineEnding startLineType,
                                                                AnnotationLineEnding endLineType)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("PolyLine");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << polyline.boundingRect();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Vertices");
    objectBuilder << polyline;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, borderWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(strokeColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("IC");
    objectBuilder << WrapAnnotationColor(fillColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("");
    objectBuilder.beginArray();
    objectBuilder << startLineType;
    objectBuilder << endLineType;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationPopup(PDFObjectReference page,
                                                             PDFObjectReference parentAnnotation,
                                                             QRectF rectangle,
                                                             bool opened)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Popup");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Parent");
    objectBuilder << parentAnnotation;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Open");
    objectBuilder << opened;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference popupAnnotation = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Popup");
    objectBuilder << popupAnnotation;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject upgradedParentAnnotation = objectBuilder.takeObject();
    mergeTo(parentAnnotation, upgradedParentAnnotation);
    updateAnnotationAppearanceStreams(popupAnnotation);
    return popupAnnotation;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationRedact(PDFObjectReference page,
                                                              QRectF rectangle,
                                                              QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Redact");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("IC");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationRedact(PDFObjectReference page,
                                                              QPolygonF quadrilaterals,
                                                              QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Redact");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("IC");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder << quadrilaterals;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationSquare(PDFObjectReference page,
                                                              QRectF rectangle,
                                                              PDFReal borderWidth,
                                                              QColor fillColor,
                                                              QColor strokeColor,
                                                              QString title,
                                                              QString subject,
                                                              QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Square");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder << std::initializer_list<PDFReal>{ 0.0, 0.0, borderWidth };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(strokeColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("IC");
    objectBuilder << WrapAnnotationColor(fillColor);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationSquiggly(PDFObjectReference page,
                                                                QRectF rectangle,
                                                                QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Squiggly");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationSquiggly(PDFObjectReference page,
                                                                QPolygonF quadrilaterals,
                                                                QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Squiggly");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder << quadrilaterals;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationSquiggly(PDFObjectReference page,
                                                                QRectF rectangle,
                                                                QColor color,
                                                                QString title,
                                                                QString subject,
                                                                QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Squiggly");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationStamp(PDFObjectReference page,
                                                             QRectF rectangle,
                                                             Stamp stampType,
                                                             QString title,
                                                             QString subject,
                                                             QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Stamp");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Name");
    objectBuilder << stampType;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationStrikeout(PDFObjectReference page,
                                                                 QRectF rectangle,
                                                                 QColor color,
                                                                 QString title,
                                                                 QString subject,
                                                                 QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("StrikeOut");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationStrikeout(PDFObjectReference page,
                                                                 QRectF rectangle,
                                                                 QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("StrikeOut");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationStrikeout(PDFObjectReference page,
                                                                 QPolygonF quadrilaterals,
                                                                 QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("StrikeOut");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder << quadrilaterals;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationText(PDFObjectReference page,
                                                            QRectF rectangle,
                                                            TextAnnotationIcon iconType,
                                                            QString title,
                                                            QString subject,
                                                            QString contents,
                                                            bool open)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Text");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Name");
    objectBuilder << iconType;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("F");
    objectBuilder << 4;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Open");
    objectBuilder << open;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    PDFObjectReference popupAnnotation = createAnnotationPopup(page, annotationObject, getPopupWindowRect(rectangle), false);

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Popup");
    objectBuilder << popupAnnotation;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updateAnnotationPopup = objectBuilder.takeObject();
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder << popupAnnotation;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    mergeTo(annotationObject, updateAnnotationPopup);
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationUnderline(PDFObjectReference page,
                                                                 QRectF rectangle,
                                                                 QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Underline");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationUnderline(PDFObjectReference page,
                                                                 QPolygonF quadrilaterals,
                                                                 QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Underline");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder << quadrilaterals;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createAnnotationUnderline(PDFObjectReference page,
                                                                 QRectF rectangle,
                                                                 QColor color,
                                                                 QString title,
                                                                 QString subject,
                                                                 QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Underline");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rectangle;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << color;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder.beginArray();
    objectBuilder << rectangle.bottomLeft();
    objectBuilder << rectangle.bottomRight();
    objectBuilder << rectangle.topLeft();
    objectBuilder << rectangle.topRight();
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference annotationObject = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder.beginArray();
    objectBuilder << annotationObject;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageAnnots = objectBuilder.takeObject();
    appendTo(page, pageAnnots);
    updateAnnotationAppearanceStreams(annotationObject);
    return annotationObject;
}


PDFObjectReference PDFDocumentBuilder::createCatalog()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Catalog");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Pages");
    objectBuilder << createCatalogPageTreeRoot();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference catalogReference = addObject(objectBuilder.takeObject());
    return catalogReference;
}


PDFObjectReference PDFDocumentBuilder::createCatalogPageTreeRoot()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Pages");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Kids");
    objectBuilder << WrapEmptyArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Count");
    objectBuilder << 0;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference pageTreeRoot = addObject(objectBuilder.takeObject());
    return pageTreeRoot;
}


PDFObjectReference PDFDocumentBuilder::createDocumentPartItem(PDFObjectReference startPage,
                                                              PDFObjectReference endPage,
                                                              PDFObjectReference parent)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("DPart");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Parent");
    objectBuilder << parent;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Start");
    objectBuilder << startPage;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("End");
    objectBuilder << endPage;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference documentPart = addObject(objectBuilder.takeObject());
    return documentPart;
}


PDFObjectReference PDFDocumentBuilder::createDocumentPartRoot()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("DPart");
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference rootNodeReference = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("DPartRoot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("DPartRootNode");
    objectBuilder << rootNodeReference;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference documentPartReference = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Parent");
    objectBuilder << documentPartReference;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedRootNode = objectBuilder.takeObject();
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("DPartRoot");
    objectBuilder << documentPartReference;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(rootNodeReference, updatedRootNode);
    mergeTo(getCatalogReference(), updatedCatalog);
    return rootNodeReference;
}


PDFObjectReference PDFDocumentBuilder::createFileSpecification(QString fileName,
                                                               QString description)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("UF");
    objectBuilder << fileName;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Desc");
    objectBuilder << description;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference fileSpecification = addObject(objectBuilder.takeObject());
    return fileSpecification;
}


PDFObjectReference PDFDocumentBuilder::createFileSpecification(QString fileName)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("UF");
    objectBuilder << fileName;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference fileSpecification = addObject(objectBuilder.takeObject());
    return fileSpecification;
}


PDFObjectReference PDFDocumentBuilder::createFormFieldSignature(QString fieldName,
                                                                PDFObjectReferenceVector kids,
                                                                PDFObjectReference signatureValue)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("FT");
    objectBuilder << WrapName("Sig");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Kids");
    objectBuilder << kids;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << fieldName;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("V");
    objectBuilder << signatureValue;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference formFieldSignature = addObject(objectBuilder.takeObject());
    return formFieldSignature;
}


void PDFDocumentBuilder::createFormFieldWidget(PDFObjectReference formField,
                                               PDFObjectReference page,
                                               PDFObjectReference appearanceStream,
                                               QRectF rect)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Widget");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << rect;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("AP");
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("N");
    objectBuilder << appearanceStream;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject widgetObject = objectBuilder.takeObject();
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder << std::array{ formField };
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageObject = objectBuilder.takeObject();
    mergeTo(formField, widgetObject);
    appendTo(page, pageObject);
}


void PDFDocumentBuilder::createInvisibleFormFieldWidget(PDFObjectReference formField,
                                                        PDFObjectReference page)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("P");
    objectBuilder << page;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Rect");
    objectBuilder << std::array{ 0.0, 0.0, 0.0, 0.0 };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Subtype");
    objectBuilder << WrapName("Widget");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Annot");
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject widgetObject = objectBuilder.takeObject();
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Annots");
    objectBuilder << std::array{ formField };
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject pageObject = objectBuilder.takeObject();
    mergeTo(formField, widgetObject);
    appendTo(page, pageObject);
}


PDFObjectReference PDFDocumentBuilder::createSignatureDictionary(QByteArray filter,
                                                                 QByteArray subfilter,
                                                                 QByteArray contents,
                                                                 QDateTime signingTime,
                                                                 PDFInteger byteRangeItem)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Type");
    objectBuilder << WrapName("Sig");
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Filter");
    objectBuilder << WrapName(filter);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("SubFilter");
    objectBuilder << WrapName(subfilter);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("ByteRange");
    objectBuilder << std::array{ byteRangeItem, byteRangeItem, byteRangeItem, byteRangeItem };
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << WrapString(contents);
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("M");
    objectBuilder << signingTime;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference signatureDictionary = addObject(objectBuilder.takeObject());
    return signatureDictionary;
}


PDFObject PDFDocumentBuilder::createTrailerDictionary(PDFObjectReference catalog)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Producer");
    objectBuilder << getProducerString();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("ModDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObjectReference infoDictionary = addObject(objectBuilder.takeObject());
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Size");
    objectBuilder << 1;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Root");
    objectBuilder << catalog;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Info");
    objectBuilder << infoDictionary;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject trailerDictionary = objectBuilder.takeObject();
    return trailerDictionary;
}


void PDFDocumentBuilder::removeDocumentActions()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("OpenAction");
    objectBuilder << PDFObject();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("AA");
    objectBuilder << PDFObject();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::removeEncryption()
{
    PDFObjectFactory objectBuilder;

    setSecurityHandler(nullptr);
}


void PDFDocumentBuilder::removeOutline()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Outlines");
    objectBuilder << PDFObject();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::removeStructureTree()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("StructTreeRoot");
    objectBuilder << PDFObject();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("MarkInfo");
    objectBuilder << PDFObject();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::removeThreads()
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Threads");
    objectBuilder << PDFObject();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::setAnnotationAppearanceState(PDFObjectReference annotation,
                                                      QByteArray appearanceState)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("AS");
    objectBuilder << WrapName(appearanceState);
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationBorder(PDFObjectReference annotation,
                                             PDFReal hRadius,
                                             PDFReal vRadius,
                                             PDFReal width)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Border");
    objectBuilder.beginArray();
    objectBuilder << hRadius;
    objectBuilder << vRadius;
    objectBuilder << width;
    objectBuilder.endArray();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationBorderStyle(PDFObjectReference annotation,
                                                  AnnotationBorderStyle style,
                                                  PDFReal width)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("BS");
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("W");
    objectBuilder << width;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("S");
    objectBuilder << style;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationColor(PDFObjectReference annotation,
                                            QColor color)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("C");
    objectBuilder << WrapAnnotationColor(color);
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationContents(PDFObjectReference annotation,
                                               QString contents)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Contents");
    objectBuilder << contents;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationFillOpacity(PDFObjectReference annotation,
                                                  PDFReal opacity)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("ca");
    objectBuilder << opacity;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationOpacity(PDFObjectReference annotation,
                                              PDFReal opacity)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("CA");
    objectBuilder << opacity;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationOpenState(PDFObjectReference annotation,
                                                bool isOpen)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Open");
    objectBuilder << isOpen;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationQuadPoints(PDFObjectReference annotation,
                                                 QPolygonF quadrilaterals)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("QuadPoints");
    objectBuilder << quadrilaterals;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationRedactText(PDFObjectReference annotation,
                                                 QString overlayText,
                                                 bool repeat)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("OverlayText");
    objectBuilder << overlayText;
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("Repeat");
    objectBuilder << repeat;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationRichText(PDFObjectReference annotation,
                                               QString richText)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("RC");
    objectBuilder << richText;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationSubject(PDFObjectReference annotation,
                                              QString subject)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Subj");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setAnnotationTitle(PDFObjectReference annotation,
                                            QString title)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("T");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject annotationObject = objectBuilder.takeObject();
    mergeTo(annotation, annotationObject);
}


void PDFDocumentBuilder::setCatalogAcroForm(PDFObjectReference acroForm)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("AcroForm");
    objectBuilder << acroForm;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::setCatalogNames(PDFObjectReference names)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Names");
    objectBuilder << ((names.isValid()) ? PDFObject::createReference(names) : PDFObject());
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::setCatalogOptionalContentProperties(PDFObjectReference ocProperties)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("OCProperties");
    objectBuilder << ocProperties;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::setDocumentAuthor(QString author)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Author");
    objectBuilder << author;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject info = objectBuilder.takeObject();
    updateDocumentInfo(qMove(info));
}


void PDFDocumentBuilder::setDocumentCreationDate(QDateTime creationDate)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("CreationDate");
    objectBuilder << creationDate;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject info = objectBuilder.takeObject();
    updateDocumentInfo(qMove(info));
}


void PDFDocumentBuilder::setDocumentCreator(QString creator)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Creator");
    objectBuilder << creator;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject info = objectBuilder.takeObject();
    updateDocumentInfo(qMove(info));
}


void PDFDocumentBuilder::setDocumentKeywords(QString keywords)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Keywords");
    objectBuilder << keywords;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject info = objectBuilder.takeObject();
    updateDocumentInfo(qMove(info));
}


void PDFDocumentBuilder::setDocumentProducer(QString producer)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Producer");
    objectBuilder << producer;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject info = objectBuilder.takeObject();
    updateDocumentInfo(qMove(info));
}


void PDFDocumentBuilder::setDocumentSubject(QString subject)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Subject");
    objectBuilder << subject;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject info = objectBuilder.takeObject();
    updateDocumentInfo(qMove(info));
}


void PDFDocumentBuilder::setDocumentTitle(QString title)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Title");
    objectBuilder << title;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject info = objectBuilder.takeObject();
    updateDocumentInfo(qMove(info));
}


void PDFDocumentBuilder::setFormFieldChoiceIndices(PDFObjectReference formField,
                                                   PDFIntegerVector indices)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("I");
    objectBuilder << indices;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject formFieldObject = objectBuilder.takeObject();
    mergeTo(formField, formFieldObject);
}


void PDFDocumentBuilder::setFormFieldChoiceTopIndex(PDFObjectReference formField,
                                                    PDFInteger topIndex)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("TI");
    objectBuilder << topIndex;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject formFieldObject = objectBuilder.takeObject();
    mergeTo(formField, formFieldObject);
}


void PDFDocumentBuilder::setFormFieldValue(PDFObjectReference formField,
                                           PDFObject value)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("V");
    objectBuilder << value;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject formFieldObject = objectBuilder.takeObject();
    mergeTo(formField, formFieldObject);
}


void PDFDocumentBuilder::setLanguage(QLocale locale)
{
    PDFObjectFactory objectBuilder;

    setLanguage(locale.name());
}


void PDFDocumentBuilder::setLanguage(QString language)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Lang");
    objectBuilder << language;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::setOutline(PDFObjectReference outline)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Outlines");
    objectBuilder << outline;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedCatalog = objectBuilder.takeObject();
    mergeTo(getCatalogReference(), updatedCatalog);
}


void PDFDocumentBuilder::setPageArtBox(PDFObjectReference page,
                                       QRectF box)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("ArtBox");
    objectBuilder << box;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPageObject = objectBuilder.takeObject();
    mergeTo(page, updatedPageObject);
}


void PDFDocumentBuilder::setPageBleedBox(PDFObjectReference page,
                                         QRectF box)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("BleedBox");
    objectBuilder << box;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPageObject = objectBuilder.takeObject();
    mergeTo(page, updatedPageObject);
}


void PDFDocumentBuilder::setPageCropBox(PDFObjectReference page,
                                        QRectF box)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("CropBox");
    objectBuilder << box;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPageObject = objectBuilder.takeObject();
    mergeTo(page, updatedPageObject);
}


void PDFDocumentBuilder::setPageDocumentPart(PDFObjectReference page,
                                             PDFObjectReference documentPart)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("DPart");
    objectBuilder << documentPart;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPage = objectBuilder.takeObject();
    mergeTo(page, updatedPage);
}


void PDFDocumentBuilder::setPageMediaBox(PDFObjectReference page,
                                         QRectF box)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("MediaBox");
    objectBuilder << box;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPageObject = objectBuilder.takeObject();
    mergeTo(page, updatedPageObject);
}


void PDFDocumentBuilder::setPageRotation(PDFObjectReference page,
                                         PageRotation rotation)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Rotate");
    objectBuilder << rotation;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPageObject = objectBuilder.takeObject();
    mergeTo(page, updatedPageObject);
}


void PDFDocumentBuilder::setPageTrimBox(PDFObjectReference page,
                                        QRectF box)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("TrimBox");
    objectBuilder << box;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPageObject = objectBuilder.takeObject();
    mergeTo(page, updatedPageObject);
}


void PDFDocumentBuilder::setPageUserUnit(PDFObjectReference page,
                                         PDFReal unit)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("UserUnit");
    objectBuilder << unit;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPageObject = objectBuilder.takeObject();
    mergeTo(page, updatedPageObject);
}


void PDFDocumentBuilder::setSignatureContactInfo(PDFObjectReference signatureDictionary,
                                                 QString contactInfoText)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("ContactInfo");
    objectBuilder << contactInfoText;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedSignatureDictionary = objectBuilder.takeObject();
    mergeTo(signatureDictionary, updatedSignatureDictionary);
}


void PDFDocumentBuilder::setSignatureReason(PDFObjectReference signatureDictionary,
                                            QString reasonText)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Reason");
    objectBuilder << reasonText;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedSignatureDictionary = objectBuilder.takeObject();
    mergeTo(signatureDictionary, updatedSignatureDictionary);
}


void PDFDocumentBuilder::updateTrailerDictionary(PDFInteger objectCount)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Size");
    objectBuilder << objectCount;
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject trailerDictionary = objectBuilder.takeObject();
    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Producer");
    objectBuilder << getProducerString();
    objectBuilder.endDictionaryItem();
    objectBuilder.beginDictionaryItem("ModDate");
    objectBuilder << WrapCurrentDateTime();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedInfoDictionary = objectBuilder.takeObject();
    m_storage.updateTrailerDictionary(qMove(trailerDictionary));
    updateDocumentInfo(qMove(updatedInfoDictionary));
}


void PDFDocumentBuilder::removePageThumbnail(PDFObjectReference pageReference)
{
    PDFObjectFactory objectBuilder;

    objectBuilder.beginDictionary();
    objectBuilder.beginDictionaryItem("Thumb");
    objectBuilder << PDFObject();
    objectBuilder.endDictionaryItem();
    objectBuilder.endDictionary();
    PDFObject updatedPageObject = objectBuilder.takeObject();
    mergeTo(pageReference, updatedPageObject);
}


/* END GENERATED CODE */

}   // namespace pdf
