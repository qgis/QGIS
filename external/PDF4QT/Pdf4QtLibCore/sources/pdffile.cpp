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

#include "pdffile.h"
#include "pdfdocument.h"
#include "pdfencoding.h"
#include "pdfdbgheap.h"

namespace pdf
{

QString PDFFileSpecification::getPlatformFileName() const
{
    // UF has maximal precedence, because it is unicode string
    if (!m_UF.isEmpty())
    {
        return m_UF;
    }

    if (!m_F.isEmpty())
    {
        return QString::fromLatin1(m_F);
    }

#ifdef Q_OS_WIN
    for (const QByteArray& platformName : { m_DOS, m_Mac, m_Unix })
    {
        if (!platformName.isEmpty())
        {
            return QString::fromLatin1(platformName);
        }
    }
#endif

#ifdef Q_OS_UNIX
    for (const QByteArray& platformName : { m_Unix, m_Mac, m_DOS })
    {
        if (!platformName.isEmpty())
        {
            return QString::fromLatin1(platformName);
        }
    }
#endif

#ifdef Q_OS_MAC
    for (const QByteArray& platformName : { m_Mac, m_Unix, m_DOS })
    {
        if (!platformName.isEmpty())
        {
            return QString::fromLatin1(platformName);
        }
    }
#endif

    return QString();
}

const PDFEmbeddedFile* PDFFileSpecification::getPlatformFile() const
{
    if (m_embeddedFiles.count("UF"))
    {
        return &m_embeddedFiles.at("UF");
    }

    if (m_embeddedFiles.count("F"))
    {
        return &m_embeddedFiles.at("F");
    }

#ifdef Q_OS_WIN
    if (m_embeddedFiles.count("DOS"))
    {
        return &m_embeddedFiles.at("DOS");
    }
#endif

#ifdef Q_OS_UNIX
    if (m_embeddedFiles.count("Unix"))
    {
        return &m_embeddedFiles.at("Unix");
    }
#endif

#ifdef Q_OS_MAC
    if (m_embeddedFiles.count("Mac"))
    {
        return &m_embeddedFiles.at("Mac");
    }
#endif

    return nullptr;
}

PDFFileSpecification PDFFileSpecification::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFFileSpecification result;

    if (object.isReference())
    {
        result.m_selfReference = object.getReference();
    }

    object = storage->getObject(object);

    if (object.isString())
    {
        result.m_UF = PDFEncoding::convertTextString(object.getString());
    }
    else if (object.isDictionary())
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        const PDFDictionary* dictionary = object.getDictionary();

        result.m_fileSystem = loader.readNameFromDictionary(dictionary, "FS");
        result.m_F = loader.readStringFromDictionary(dictionary, "F");
        result.m_UF = loader.readTextStringFromDictionary(dictionary, "UF", QString());
        result.m_DOS = loader.readStringFromDictionary(dictionary, "DOS");
        result.m_Mac = loader.readStringFromDictionary(dictionary, "Mac");
        result.m_Unix = loader.readStringFromDictionary(dictionary, "Unix");
        result.m_id = PDFFileIdentifier::parse(storage, dictionary->get("ID"));
        result.m_volatile = loader.readBooleanFromDictionary(dictionary, "V", false);
        result.m_description = loader.readTextStringFromDictionary(dictionary, "Desc", QString());
        result.m_collection = loader.readReferenceFromDictionary(dictionary, "CI");
        result.m_thumbnailReference = loader.readReferenceFromDictionary(dictionary, "Thumb");
        result.m_encryptedPayload = dictionary->get("EP");

        constexpr const std::array relationships = {
            std::pair<const char*, AssociatedFileRelationship>{ "Unspecified", AssociatedFileRelationship::Unspecified },
            std::pair<const char*, AssociatedFileRelationship>{ "Source", AssociatedFileRelationship::Source },
            std::pair<const char*, AssociatedFileRelationship>{ "Data", AssociatedFileRelationship::Data },
            std::pair<const char*, AssociatedFileRelationship>{ "Alternative", AssociatedFileRelationship::Alternative },
            std::pair<const char*, AssociatedFileRelationship>{ "Supplement", AssociatedFileRelationship::Supplement },
            std::pair<const char*, AssociatedFileRelationship>{ "EncryptedPayload", AssociatedFileRelationship::EncryptedPayload },
            std::pair<const char*, AssociatedFileRelationship>{ "FormData", AssociatedFileRelationship::FormData },
            std::pair<const char*, AssociatedFileRelationship>{ "Schema", AssociatedFileRelationship::Schema },
        };

        result.m_associatedFileRelationship = loader.readEnumByName(dictionary->get("AFRelationship"), relationships.begin(), relationships.end(), AssociatedFileRelationship::Unspecified);

        PDFObject embeddedFiles = storage->getObject(dictionary->get("EF"));
        PDFObject relatedFiles = storage->getObject(dictionary->get("RF"));
        if (embeddedFiles.isDictionary())
        {
            const PDFDictionary* embeddedFilesDictionary = embeddedFiles.getDictionary();
            const PDFDictionary* relatedFilesDictionary = relatedFiles.isDictionary() ? relatedFiles.getDictionary() : nullptr;
            for (size_t i = 0; i < embeddedFilesDictionary->getCount(); ++i)
            {
                QByteArray key = embeddedFilesDictionary->getKey(i).getString();
                result.m_embeddedFiles[key] = PDFEmbeddedFile::parse(storage, embeddedFilesDictionary->getValue(i));

                if (relatedFilesDictionary)
                {
                    PDFObject relatedFileArrayObject = storage->getObject(relatedFilesDictionary->get(key));
                    if (relatedFileArrayObject.isArray())
                    {
                        const PDFArray* relatedFileArray = relatedFileArrayObject.getArray();
                        const size_t relatedFilesCount = relatedFileArray->getCount() / 2;

                        RelatedFiles& currentRelatedFiles = result.m_relatedFiles[key];
                        currentRelatedFiles.reserve(relatedFilesCount);
                        for (size_t ii = 0; ii < relatedFilesCount; ++ii)
                        {
                            RelatedFile relatedFile;
                            relatedFile.name = loader.readString(relatedFileArray->getItem(2 * ii));
                            relatedFile.fileReference = loader.readReference(relatedFileArray->getItem(2 * ii + 1));
                            currentRelatedFiles.emplace_back(qMove(relatedFile));
                        }
                    }
                }
            }
        }
    }

    return result;
}

PDFEmbeddedFile PDFEmbeddedFile::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFEmbeddedFile result;
    object = storage->getObject(object);

    if (object.isStream())
    {
        const PDFStream* stream = object.getStream();
        const PDFDictionary* dictionary = stream->getDictionary();
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_stream = object;
        result.m_subtype = loader.readNameFromDictionary(dictionary, "Subtype");

        const PDFObject& paramsObject = storage->getObject(dictionary->get("Params"));
        if (paramsObject.isDictionary())
        {
            const PDFDictionary* paramsDictionary = paramsObject.getDictionary();
            auto getDateTime = [&loader, paramsDictionary](const char* name)
            {
                QByteArray ba = loader.readStringFromDictionary(paramsDictionary, name);
                if (!ba.isEmpty())
                {
                    return PDFEncoding::convertToDateTime(ba);
                }
                return QDateTime();
            };

            result.m_size = loader.readIntegerFromDictionary(paramsDictionary, "Size", -1);
            result.m_creationDate = getDateTime("CreationDate");
            result.m_modifiedDate = getDateTime("ModDate");
            result.m_checksum = loader.readStringFromDictionary(paramsDictionary, "CheckSum");
        }
    }

    return result;
}

PDFFileIdentifier PDFFileIdentifier::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFFileIdentifier result;
    PDFDocumentDataLoaderDecorator loader(storage);
    std::vector<QByteArray> identifiers = loader.readStringArray(object);

    if (identifiers.size() >= 1)
    {
        result.m_permanentIdentifier = qMove(identifiers[0]);
    }

    if (identifiers.size() >= 2)
    {
        result.m_changingIdentifier = qMove(identifiers[1]);
    }

    return result;
}

PDFCollectionField PDFCollectionField::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFCollectionField result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        constexpr const std::array fieldKinds = {
            std::pair<const char*, Kind>{ "S", Kind::TextField },
            std::pair<const char*, Kind>{ "D", Kind::DateField },
            std::pair<const char*, Kind>{ "N", Kind::NumberField },
            std::pair<const char*, Kind>{ "F", Kind::FileName },
            std::pair<const char*, Kind>{ "Desc", Kind::Description },
            std::pair<const char*, Kind>{ "ModDate", Kind::ModifiedDate },
            std::pair<const char*, Kind>{ "CreationDate", Kind::CreationDate },
            std::pair<const char*, Kind>{ "Size", Kind::Size },
            std::pair<const char*, Kind>{ "CompressedSize", Kind::CompressedSize },
        };

        result.m_kind = loader.readEnumByName(dictionary->get("Subtype"), fieldKinds.begin(), fieldKinds.end(), Kind::Invalid);

        switch (result.m_kind)
        {
            case Kind::Invalid:
                result.m_value = Value::Invalid;
                break;

            case Kind::TextField:
                result.m_value = Value::TextString;
                break;

            case Kind::DateField:
                result.m_value = Value::DateTime;
                break;

            case Kind::NumberField:
                result.m_value = Value::Number;
                break;

            case Kind::FileName:
                result.m_value = Value::TextString;
                break;

            case Kind::Description:
                result.m_value = Value::TextString;
                break;

            case Kind::ModifiedDate:
                result.m_value = Value::DateTime;
                break;

            case Kind::CreationDate:
                result.m_value = Value::DateTime;
                break;

            case Kind::Size:
                result.m_value = Value::Number;
                break;

            case Kind::CompressedSize:
                result.m_value = Value::Number;
                break;

            default:
                Q_ASSERT(false);
                break;
        }

        result.m_fieldName = loader.readTextStringFromDictionary(dictionary, "N", QString());
        result.m_order = loader.readIntegerFromDictionary(dictionary, "O", 0);
        result.m_visible = loader.readBooleanFromDictionary(dictionary, "V", true);
        result.m_editable = loader.readBooleanFromDictionary(dictionary, "E", false);
    }

    return result;
}

const PDFCollectionField* PDFCollectionSchema::getField(const QByteArray& key) const
{
    auto it = m_fields.find(key);
    if (it != m_fields.cend())
    {
        return &it->second;
    }
    else
    {
        static PDFCollectionField dummy;
        return &dummy;
    }
}

PDFCollectionSchema PDFCollectionSchema::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFCollectionSchema result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        const size_t count = dictionary->getCount();
        for (size_t i = 0; i < count; ++i)
        {
            QByteArray key = dictionary->getKey(i).getString();

            if (key == "Type")
            {
                continue;
            }

            result.m_fields[key] = PDFCollectionField::parse(storage, dictionary->getValue(i));
        }
    }

    return result;
}

PDFCollection PDFCollection::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFCollection result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        constexpr const std::array viewModes = {
            std::pair<const char*, ViewMode>{ "D", ViewMode::Details },
            std::pair<const char*, ViewMode>{ "T", ViewMode::Tiles },
            std::pair<const char*, ViewMode>{ "H", ViewMode::Hidden },
            std::pair<const char*, ViewMode>{ "C", ViewMode::Navigation }
        };

        result.m_schema = PDFCollectionSchema::parse(storage, dictionary->get("Schema"));
        result.m_document = loader.readStringFromDictionary(dictionary, "D");
        result.m_viewMode = loader.readEnumByName(dictionary->get("View"), viewModes.begin(), viewModes.end(), ViewMode::Details);
        result.m_navigator = loader.readReferenceFromDictionary(dictionary, "Navigator");

        const PDFDictionary* colorDictionary = storage->getDictionaryFromObject(dictionary->get("Colors"));
        if (colorDictionary)
        {
            result.m_colors[Background] = loader.readRGBColorFromDictionary(colorDictionary, "Background", Qt::white);
            result.m_colors[CardBackground] = loader.readRGBColorFromDictionary(colorDictionary, "CardBackground", Qt::white);
            result.m_colors[CardBorder] = loader.readRGBColorFromDictionary(colorDictionary, "CardBorder", Qt::white);
            result.m_colors[PrimaryText] = loader.readRGBColorFromDictionary(colorDictionary, "PrimaryText", Qt::black);
            result.m_colors[SecondaryText] = loader.readRGBColorFromDictionary(colorDictionary, "SecondaryText", Qt::black);
        }

        const PDFDictionary* sortDictionary = storage->getDictionaryFromObject(dictionary->get("Sort"));
        if (sortDictionary)
        {
            result.m_sortAscending = loader.readBooleanFromDictionary(sortDictionary, "A", true);
            const PDFObject& columns = storage->getObject(sortDictionary->get("S"));
            if (columns.isName())
            {
                result.m_sortColumns.emplace_back(SortColumn{loader.readName(columns), result.m_sortAscending});
            }
            else
            {
                std::vector<QByteArray> names = loader.readNameArray(columns);
                for (QByteArray& name : names)
                {
                    result.m_sortColumns.emplace_back(SortColumn{qMove(name), result.m_sortAscending});
                }
            }

            const PDFObject& sortDirection = storage->getObject(sortDictionary->get("A"));
            if (sortDirection.isArray())
            {
                const PDFArray* sortDirectionArray = sortDirection.getArray();
                const size_t size = qMin(result.m_sortColumns.size(), sortDirectionArray->getCount());
                for (size_t i = 0; i < size; ++i)
                {
                    result.m_sortColumns[i].ascending = loader.readBoolean(sortDirectionArray->getItem(i), result.m_sortAscending);
                }
            }
        }

        result.m_folderRoot = loader.readReferenceFromDictionary(dictionary, "Folders");

        const PDFDictionary* splitDictionary = storage->getDictionaryFromObject(dictionary->get("Split"));
        if (splitDictionary)
        {
            constexpr const std::array splitModes = {
                std::pair<const char*, SplitMode>{ "H", SplitMode::Horizontally },
                std::pair<const char*, SplitMode>{ "V", SplitMode::Vertically },
                std::pair<const char*, SplitMode>{ "N", SplitMode::None }
            };

            result.m_splitMode = loader.readEnumByName(splitDictionary->get("Direction"), splitModes.begin(), splitModes.end(), SplitMode::None);
            result.m_splitProportion = loader.readNumberFromDictionary(splitDictionary, "Position", 30);
        }
    }

    return result;
}

PDFCollectionFolder PDFCollectionFolder::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFCollectionFolder result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_ID = loader.readIntegerFromDictionary(dictionary, "ID", 0);
        result.m_name = loader.readTextStringFromDictionary(dictionary, "Name", QString());
        result.m_parent = loader.readReferenceFromDictionary(dictionary, "Parent");
        result.m_child = loader.readReferenceFromDictionary(dictionary, "Child");
        result.m_next = loader.readReferenceFromDictionary(dictionary, "Next");
        result.m_collection = loader.readReferenceFromDictionary(dictionary, "CI");
        result.m_description = loader.readTextStringFromDictionary(dictionary, "Desc", QString());

        QByteArray createdString = loader.readStringFromDictionary(dictionary, "CreationDate");
        if (!createdString.isEmpty())
        {
            result.m_created = PDFEncoding::convertToDateTime(createdString);
        }
        QByteArray modifiedString = loader.readStringFromDictionary(dictionary, "ModDate");
        if (!modifiedString.isEmpty())
        {
            result.m_modified = PDFEncoding::convertToDateTime(modifiedString);
        }

        result.m_thumbnail = loader.readReferenceFromDictionary(dictionary, "Thumb");
        result.m_freeIds = loader.readIntegerArrayFromDictionary(dictionary, "Free");
    }

    return result;
}

PDFCollectionNavigator PDFCollectionNavigator::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFCollectionNavigator result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        auto getLayout = [&loader](const PDFObject& object)
        {
            constexpr const std::array layouts = {
                std::pair<const char*, Layout>{ "D", Layout::Details },
                std::pair<const char*, Layout>{ "T", Layout::Tile },
                std::pair<const char*, Layout>{ "H", Layout::Hidden },
                std::pair<const char*, Layout>{ "FilmStrip", Layout::FilmStrip },
                std::pair<const char*, Layout>{ "FreeForm", Layout::FreeForm },
                std::pair<const char*, Layout>{ "Linear", Layout::Linear },
                std::pair<const char*, Layout>{ "Tree", Layout::Tree }
            };

            return loader.readEnumByName(object, layouts.begin(), layouts.end(), Layout::Invalid);
        };

        result.m_layouts = None;

        PDFObject layoutObject = storage->getObject(dictionary->get("Layout"));
        if (layoutObject.isArray())
        {
            for (const PDFObject& objectInLayourObjects : *layoutObject.getArray())
            {
                result.m_layouts |= getLayout(objectInLayourObjects);
            }
        }
        else
        {
            result.m_layouts = getLayout(layoutObject);
        }
    }

    return result;
}

QString PDFCollectionItem::getString(const QByteArray& key, const PDFObjectStorage* storage) const
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(m_object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        PDFObject valueObject = storage->getObject(dictionary->get(key));

        if (valueObject.isDictionary())
        {
            return loader.readTextString(valueObject.getDictionary()->get("D"), QString());
        }

        return loader.readTextString(valueObject, QString());
    }

    return QString();
}

QDateTime PDFCollectionItem::getDateTime(const QByteArray& key, const PDFObjectStorage* storage) const
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(m_object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        PDFObject valueObject = storage->getObject(dictionary->get(key));

        if (valueObject.isDictionary())
        {
            valueObject = storage->getObject(valueObject.getDictionary()->get("D"));
        }

        if (valueObject.isString())
        {
            return PDFEncoding::convertToDateTime(valueObject.getString());
        }
    }

    return QDateTime();
}

PDFInteger PDFCollectionItem::getNumber(const QByteArray& key, const PDFObjectStorage* storage) const
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(m_object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        PDFObject valueObject = storage->getObject(dictionary->get(key));

        if (valueObject.isDictionary())
        {
            return loader.readInteger(valueObject.getDictionary()->get("D"), 0);
        }

        return loader.readInteger(valueObject, 0);
    }

    return 0;
}

QString PDFCollectionItem::getPrefixString(const QByteArray& key, const PDFObjectStorage* storage) const
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(m_object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        PDFObject valueObject = storage->getObject(dictionary->get(key));

        if (valueObject.isDictionary())
        {
            return loader.readTextString(valueObject.getDictionary()->get("P"), QString());
        }
    }

    return QString();
}

}   // namespace pdf
