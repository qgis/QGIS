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

#include "pdfobjecteditormodel.h"
#include "pdfdocumentbuilder.h"
#include "pdfblendfunction.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFObjectEditorAbstractModel::PDFObjectEditorAbstractModel(QObject* parent) :
    BaseClass(parent),
    m_storage(nullptr),
    m_typeAttribute(0)
{

}

PDFObjectEditorAbstractModel::~PDFObjectEditorAbstractModel()
{

}

size_t PDFObjectEditorAbstractModel::getAttributeCount() const
{
    return m_attributes.size();
}

ObjectEditorAttributeType PDFObjectEditorAbstractModel::getAttributeType(size_t index) const
{
    return m_attributes.at(index).type;
}

const QString& PDFObjectEditorAbstractModel::getAttributeCategory(size_t index) const
{
    return m_attributes.at(index).category;
}

const QString& PDFObjectEditorAbstractModel::getAttributeSubcategory(size_t index) const
{
    return m_attributes.at(index).subcategory;
}

const QString& PDFObjectEditorAbstractModel::getAttributeName(size_t index) const
{
    return m_attributes.at(index).name;
}

const PDFObjectEditorModelAttributeEnumItems& PDFObjectEditorAbstractModel::getAttributeEnumItems(size_t index) const
{
    return m_attributes.at(index).enumItems;
}

bool PDFObjectEditorAbstractModel::queryAttribute(size_t index, Question question) const
{
    const PDFObjectEditorModelAttribute& attribute = m_attributes.at(index);

    switch (question)
    {
        case Question::IsMapped:
            return !attribute.attributeFlags.testFlag(PDFObjectEditorModelAttribute::Hidden) && attribute.type != ObjectEditorAttributeType::Constant;

        case Question::IsVisible:
        {
            if (!queryAttribute(index, Question::IsMapped))
            {
                return false;
            }

            if (!attribute.attributeFlags.testFlag(PDFObjectEditorModelAttribute::HideInsteadOfDisable))
            {
                return true;
            }

            return queryAttribute(index, Question::HasAttribute);
        }

        case Question::HasAttribute:
        {
            // Check type flags
            if (attribute.typeFlags)
            {
                uint32_t currentTypeFlags = getCurrentTypeFlags();
                if (!(attribute.typeFlags & currentTypeFlags))
                {
                    return false;
                }
            }

            // Check selector
            if (attribute.selectorAttribute)
            {
                if (!getSelectorValue(attribute.selectorAttribute))
                {
                    return false;
                }
            }

            return true;
        }

        case Question::HasSimilarAttribute:
        {
            if (!queryAttribute(index, Question::HasAttribute))
            {
                // Find similar attributes
                if (queryAttribute(index, Question::IsPersisted))
                {
                    auto it = m_similarAttributes.find(index);
                    if (it != m_similarAttributes.cend())
                    {
                        const std::vector<size_t>& similarAttributes = it->second;
                        for (const size_t similarAttribute : similarAttributes)
                        {
                            if (queryAttribute(similarAttribute, Question::HasAttribute) && queryAttribute(similarAttribute, Question::IsPersisted))
                            {
                                return true;
                            }
                        }
                    }
                }

                return false;
            }

            return true;
        }

        case Question::IsAttributeEditable:
            return queryAttribute(index, Question::HasAttribute) && !attribute.attributeFlags.testFlag(PDFObjectEditorModelAttribute::Readonly);

        case Question::IsSelector:
            return attribute.type == ObjectEditorAttributeType::Selector;

        case Question::IsPersisted:
            return !queryAttribute(index, Question::IsSelector) && !attribute.dictionaryAttribute.isEmpty();

        default:
            break;
    }

    return false;
}

bool PDFObjectEditorAbstractModel::getSelectorValue(size_t index) const
{
    return m_attributes.at(index).selectorAttributeValue;
}

void PDFObjectEditorAbstractModel::setSelectorValue(size_t index, bool value)
{
    m_attributes.at(index).selectorAttributeValue = value;
}

std::vector<size_t> PDFObjectEditorAbstractModel::getSelectorAttributes() const
{
    std::vector<size_t> result;
    result.reserve(8); // Estimate maximal number of selectors

    const size_t count = getAttributeCount();
    for (size_t i = 0; i < count; ++i)
    {
        if (queryAttribute(i, Question::IsSelector))
        {
            result.push_back(i);
        }
    }

    return result;
}

std::vector<size_t> PDFObjectEditorAbstractModel::getSelectorDependentAttributes(size_t selector) const
{
    std::vector<size_t> result;
    result.reserve(16); // Estimate maximal number of selector's attributes

    const size_t count = getAttributeCount();
    for (size_t i = 0; i < count; ++i)
    {
        if (m_attributes.at(i).selectorAttribute == selector)
        {
            result.push_back(i);
        }
    }

    return result;
}

PDFObject PDFObjectEditorAbstractModel::getValue(size_t index, bool resolveArrayIndex) const
{
    const QByteArrayList& dictionaryAttribute = m_attributes.at(index).dictionaryAttribute;
    if (dictionaryAttribute.isEmpty())
    {
        return PDFObject();
    }

    PDFDocumentDataLoaderDecorator loader(m_storage);

    if (const PDFDictionary* dictionary = m_storage->getDictionaryFromObject(m_editedObject))
    {
        const int pathDepth = dictionaryAttribute.size() - 1;

        for (int i = 0; i < pathDepth; ++i)
        {
            dictionary = m_storage->getDictionaryFromObject(dictionary->get(dictionaryAttribute[i]));
            if (!dictionary)
            {
                return PDFObject();
            }
        }

        const size_t arrayIndex = m_attributes.at(index).arrayIndex;
        if (arrayIndex && resolveArrayIndex)
        {
            PDFObject object = m_storage->getObject(dictionary->get(dictionaryAttribute.back()));
            if (object.isArray())
            {
                const PDFArray* arrayObject = object.getArray();
                if (arrayIndex <= arrayObject->getCount())
                {
                    return arrayObject->getItem(arrayIndex - 1);
                }
            }

            return PDFObject();
        }

        return dictionary->get(dictionaryAttribute.back());
    }

    return PDFObject();
}

PDFObject PDFObjectEditorAbstractModel::getDefaultValue(size_t index) const
{
    return m_attributes.at(index).defaultValue;
}

void PDFObjectEditorAbstractModel::setEditedObject(PDFObject object)
{
    if (m_editedObject != object)
    {
        m_editedObject = qMove(object);
        updateSelectorValues();
        Q_EMIT editedObjectChanged();
    }
}

PDFObject PDFObjectEditorAbstractModel::writeAttributeValueToObject(size_t attribute, PDFObject object, PDFObject value) const
{
    Q_ASSERT(queryAttribute(attribute, Question::IsPersisted));

    PDFObjectFactory factory;
    factory.beginDictionary();

    const QByteArrayList& dictionaryAttribute = m_attributes.at(attribute).dictionaryAttribute;
    const int pathDepth = dictionaryAttribute.size() - 1;
    for (int i = 0; i < pathDepth; ++i)
    {
        factory.beginDictionaryItem(dictionaryAttribute[i]);
        factory.beginDictionary();
    }

    factory.beginDictionaryItem(dictionaryAttribute.back());
    size_t arrayIndex = m_attributes.at(attribute).arrayIndex;
    if (arrayIndex)
    {
        PDFArray array;
        PDFObject arrayObject = m_storage->getObject(getValue(attribute, false));
        if (arrayObject.isArray())
        {
            array = *arrayObject.getArray();
        }

        --arrayIndex;

        while (arrayIndex >= array.getCount())
        {
            array.appendItem(PDFObject());
        }

        array.setItem(qMove(value), arrayIndex);
        factory << PDFObject::createArray(std::make_shared<PDFArray>(qMove(array)));
    }
    else
    {
        factory << qMove(value);
    }
    factory.endDictionaryItem();

    for (int i = 0; i < pathDepth; ++i)
    {
        factory.endDictionary();
        factory.endDictionaryItem();
    }

    factory.endDictionary();
    return PDFObjectManipulator::merge(qMove(object), factory.takeObject(), PDFObjectManipulator::RemoveNullObjects);
}

QVariant PDFObjectEditorAbstractModel::getMinimumValue(size_t index) const
{
    return m_attributes.at(index).minValue;
}

QVariant PDFObjectEditorAbstractModel::getMaximumValue(size_t index) const
{
    return m_attributes.at(index).maxValue;
}

void PDFObjectEditorAbstractModel::initialize()
{
    const size_t attributeCount = getAttributeCount();
    std::map<QByteArrayList, std::vector<size_t>> similarAttributes;

    for (size_t attribute = 0; attribute < attributeCount; ++attribute)
    {
        if (!queryAttribute(attribute, Question::IsPersisted))
        {
            // Non-persisted attributes are skipped
            continue;
        }

        similarAttributes[m_attributes[attribute].dictionaryAttribute].push_back(attribute);
    }

    for (const auto& similarAttributeItem : similarAttributes)
    {
        const std::vector<size_t>& attributes = similarAttributeItem.second;
        if (attributes.size() > 1)
        {
            for (const size_t attribute : attributes)
            {
                m_similarAttributes[attribute] = attributes;
            }
        }
    }
}

void PDFObjectEditorAbstractModel::updateSelectorValues()
{
    // Turn on selectors, which have some dependent attribute,
    // which have value in the persisted object.
    for (size_t index = 0; index < getAttributeCount(); ++index)
    {
        if (!queryAttribute(index, Question::IsSelector))
        {
            continue;
        }

        bool hasPersistedAttribute = false;
        for (size_t dependentAttribute : getSelectorDependentAttributes(index))
        {
            if (!getValue(dependentAttribute, true).isNull())
            {
                hasPersistedAttribute = true;
                break;
            }
        }

        if (hasPersistedAttribute)
        {
            setSelectorValue(index, true);
        }
    }
}

size_t PDFObjectEditorAbstractModel::createAttribute(ObjectEditorAttributeType type,
                                                     QByteArray attributeName,
                                                     QString category,
                                                     QString subcategory,
                                                     QString name,
                                                     PDFObject defaultValue,
                                                     uint32_t typeFlags,
                                                     PDFObjectEditorModelAttribute::Flags flags)
{
    size_t index = m_attributes.size();

    QByteArrayList attributes;
    attributes.push_back(attributeName);
    PDFObjectEditorModelAttribute attribute;
    attribute.type = type;
    attribute.dictionaryAttribute = attributes;
    attribute.category = qMove(category);
    attribute.subcategory = qMove(subcategory);
    attribute.name = qMove(name);
    attribute.defaultValue = qMove(defaultValue);
    attribute.typeFlags = typeFlags;
    attribute.attributeFlags = flags;
    m_attributes.emplace_back(qMove(attribute));

    if (type == ObjectEditorAttributeType::Type)
    {
        m_typeAttribute = index;
    }

    return index;
}

size_t PDFObjectEditorAbstractModel::createAttribute(ObjectEditorAttributeType type,
                                                     QByteArrayList attributesName,
                                                     QString category,
                                                     QString subcategory,
                                                     QString name,
                                                     PDFObject defaultValue,
                                                     uint32_t typeFlags,
                                                     PDFObjectEditorModelAttribute::Flags flags)
{
    size_t index = m_attributes.size();

    PDFObjectEditorModelAttribute attribute;
    attribute.type = type;
    attribute.dictionaryAttribute = qMove(attributesName);
    attribute.category = qMove(category);
    attribute.subcategory = qMove(subcategory);
    attribute.name = qMove(name);
    attribute.defaultValue = qMove(defaultValue);
    attribute.typeFlags = typeFlags;
    attribute.attributeFlags = flags;
    m_attributes.emplace_back(qMove(attribute));

    if (type == ObjectEditorAttributeType::Type)
    {
        m_typeAttribute = index;
    }

    return index;
}

size_t PDFObjectEditorAbstractModel::createSelectorAttribute(QString category, QString subcategory, QString name)
{
    return createAttribute(ObjectEditorAttributeType::Selector, QByteArray(), qMove(category), qMove(subcategory), qMove(name));
}

uint32_t PDFObjectEditorAbstractModel::getCurrentTypeFlags() const
{
    PDFObject value = getValue(m_typeAttribute, true);

    for (const PDFObjectEditorModelAttributeEnumItem& item : m_attributes.at(m_typeAttribute).enumItems)
    {
        if (item.value == value)
        {
            return item.flags;
        }
    }

    return 0;
}

PDFObject PDFObjectEditorAbstractModel::getDefaultColor() const
{
    PDFObjectFactory factory;
    factory.beginArray();
    factory << std::initializer_list<PDFReal>{ 0.0, 0.0, 0.0 };
    factory.endArray();
    return factory.takeObject();
}

PDFObjectEditorAnnotationsModel::PDFObjectEditorAnnotationsModel(QObject* parent) :
    BaseClass(parent)
{
    createAttribute(ObjectEditorAttributeType::Constant, "Type", tr("General"), tr("General"), tr("Type"), PDFObject::createName("Annot"), 0, PDFObjectEditorModelAttribute::Hidden);
    createAttribute(ObjectEditorAttributeType::Type, "Subtype", tr("General"), tr("General"), tr("Type"));

    PDFObjectEditorModelAttributeEnumItems typeEnumItems;
    typeEnumItems.emplace_back(tr("Text"), Text, PDFObject::createName("Text"));
    typeEnumItems.emplace_back(tr("Link"), Link, PDFObject::createName("Link"));
    typeEnumItems.emplace_back(tr("Free text"), FreeText, PDFObject::createName("FreeText"));
    typeEnumItems.emplace_back(tr("Line"), Line, PDFObject::createName("Line"));
    typeEnumItems.emplace_back(tr("Square"), Square, PDFObject::createName("Square"));
    typeEnumItems.emplace_back(tr("Circle"), Circle, PDFObject::createName("Circle"));
    typeEnumItems.emplace_back(tr("Polygon"), Polygon, PDFObject::createName("Polygon"));
    typeEnumItems.emplace_back(tr("Polyline"), PolyLine, PDFObject::createName("PolyLine"));
    typeEnumItems.emplace_back(tr("Highlight"), Highlight, PDFObject::createName("Highlight"));
    typeEnumItems.emplace_back(tr("Underline"), Underline, PDFObject::createName("Underline"));
    typeEnumItems.emplace_back(tr("Squiggly"), Squiggly, PDFObject::createName("Squiggly"));
    typeEnumItems.emplace_back(tr("Strike Out"), StrikeOut, PDFObject::createName("StrikeOut"));
    typeEnumItems.emplace_back(tr("Caret"), Caret, PDFObject::createName("Caret"));
    typeEnumItems.emplace_back(tr("Stamp"), Stamp, PDFObject::createName("Stamp"));
    typeEnumItems.emplace_back(tr("Ink"), Ink, PDFObject::createName("Ink"));
    typeEnumItems.emplace_back(tr("File attachment"), FileAttachment, PDFObject::createName("FileAttachment"));
    typeEnumItems.emplace_back(tr("Redaction"), Redact, PDFObject::createName("Redact"));
    m_attributes.back().enumItems = qMove(typeEnumItems);

    createAttribute(ObjectEditorAttributeType::Rectangle, "Rect", tr("General"), tr("General"), tr("Rectangle"), PDFObject());
    createAttribute(ObjectEditorAttributeType::TextLine, "T", tr("Contents"), tr("Contents"), tr("Author"), PDFObject(), Markup);
    createAttribute(ObjectEditorAttributeType::TextLine, "Subj", tr("Contents"), tr("Contents"), tr("Subject"), PDFObject(), Markup);
    createAttribute(ObjectEditorAttributeType::TextBrowser, "Contents", tr("Contents"), tr("Contents"), tr("Contents"));
    createAttribute(ObjectEditorAttributeType::TextLine, "NM", tr("Contents"), tr("Contents"), tr("Annotation name"));
    createAttribute(ObjectEditorAttributeType::DateTime, "M", tr("General"), tr("Info"), tr("Modified"), PDFObject(), 0, PDFObjectEditorModelAttribute::Readonly);
    createAttribute(ObjectEditorAttributeType::DateTime, "M", tr("General"), tr("Info"), tr("Created"), PDFObject(), Markup, PDFObjectEditorModelAttribute::Readonly);
    createAttribute(ObjectEditorAttributeType::Flags, "F", tr("Options"), tr("Options"), QString());

    PDFObjectEditorModelAttributeEnumItems annotationFlagItems;
    annotationFlagItems.emplace_back(tr("Invisible"), 1 << 0, PDFObject());
    annotationFlagItems.emplace_back(tr("Hidden"), 1 << 1, PDFObject());
    annotationFlagItems.emplace_back(tr("Print"), 1 << 2, PDFObject());
    annotationFlagItems.emplace_back(tr("No Zoom"), 1 << 3, PDFObject());
    annotationFlagItems.emplace_back(tr("No Rotate"), 1 << 4, PDFObject());
    annotationFlagItems.emplace_back(tr("No View"), 1 << 5, PDFObject());
    annotationFlagItems.emplace_back(tr("Readonly"), 1 << 6, PDFObject());
    annotationFlagItems.emplace_back(tr("Locked"), 1 << 7, PDFObject());
    annotationFlagItems.emplace_back(tr("Toggle No View"), 1 << 8, PDFObject());
    annotationFlagItems.emplace_back(tr("Locked Contents"), 1 << 9, PDFObject());
    m_attributes.back().enumItems = qMove(annotationFlagItems);

    size_t appearanceSelector = createSelectorAttribute(tr("General"), tr("Options"), tr("Modify appearance"));
    createAttribute(ObjectEditorAttributeType::Color, "C", tr("Appearance"), tr("Colors"), tr("Color"), getDefaultColor());
    m_attributes.back().selectorAttribute = appearanceSelector;
    createAttribute(ObjectEditorAttributeType::Color, "IC", tr("Appearance"), tr("Colors"), tr("Interior color"), getDefaultColor(), Line | Circle | Square | Polygon | PolyLine | Redact);
    m_attributes.back().selectorAttribute = appearanceSelector;

    createAttribute(ObjectEditorAttributeType::ComboBox, "BM", tr("Appearance"), tr("Transparency"), tr("Blend mode"), PDFObject::createName("Normal"));
    m_attributes.back().selectorAttribute = appearanceSelector;

    PDFObjectEditorModelAttributeEnumItems blendModeEnumItems;
    for (BlendMode mode : PDFBlendModeInfo::getBlendModes())
    {
        blendModeEnumItems.emplace_back(PDFBlendModeInfo::getBlendModeTranslatedName(mode), uint(mode), PDFObject::createName(PDFBlendModeInfo::getBlendModeName(mode).toLatin1()));
    }
    m_attributes.back().enumItems = qMove(blendModeEnumItems);

    createAttribute(ObjectEditorAttributeType::Double, "ca", tr("Appearance"), tr("Transparency"), tr("Fill opacity"), PDFObject::createReal(1.0));
    m_attributes.back().selectorAttribute = appearanceSelector;
    m_attributes.back().minValue = 0.0;
    m_attributes.back().maxValue = 1.0;
    createAttribute(ObjectEditorAttributeType::Double, "CA", tr("Appearance"), tr("Transparency"), tr("Stroke opacity"), PDFObject::createReal(1.0));
    m_attributes.back().selectorAttribute = appearanceSelector;
    m_attributes.back().minValue = 0.0;
    m_attributes.back().maxValue = 1.0;

    createAttribute(ObjectEditorAttributeType::TextLine, "Lang", tr("General"), tr("General"), tr("Language"));

    // Border style/effect
    size_t borderSelector = createSelectorAttribute(tr("General"), tr("Options"), tr("Modify border"));
    createAttribute(ObjectEditorAttributeType::Double, QByteArrayList() << "BS" << "W", tr("Border"), tr("Border Style"), tr("Width"), PDFObject::createReal(0.0), Link | Line | Circle | Square | Polygon | PolyLine | Ink);
    m_attributes.back().selectorAttribute = borderSelector;
    m_attributes.back().minValue = 0.0;

    createAttribute(ObjectEditorAttributeType::ComboBox, QByteArrayList() << "BS" << "S", tr("Border"), tr("Border Style"), tr("Style"), PDFObject::createName("S"), Link | Line | Circle | Square | Polygon | PolyLine | Ink);
    PDFObjectEditorModelAttributeEnumItems borderStyleEnumItems;
    borderStyleEnumItems.emplace_back(tr("Solid"), 1, PDFObject::createName("S"));
    borderStyleEnumItems.emplace_back(tr("Dashed"), 2, PDFObject::createName("D"));
    borderStyleEnumItems.emplace_back(tr("Beveled"), 3, PDFObject::createName("B"));
    borderStyleEnumItems.emplace_back(tr("Inset"), 4, PDFObject::createName("I"));
    borderStyleEnumItems.emplace_back(tr("Underline"), 5, PDFObject::createName("U"));
    m_attributes.back().selectorAttribute = borderSelector;
    m_attributes.back().enumItems = qMove(borderStyleEnumItems);

    createAttribute(ObjectEditorAttributeType::ComboBox, QByteArrayList() << "BE" << "S", tr("Border"), tr("Border Effect"), tr("Style"), PDFObject::createName("S"), FreeText | Circle | Square | Polygon);
    PDFObjectEditorModelAttributeEnumItems borderEffectEnumItems;
    borderEffectEnumItems.emplace_back(tr("Cloudy"), 1, PDFObject::createName("C"));
    borderEffectEnumItems.emplace_back(tr("None"), 2, PDFObject::createName("S"));
    m_attributes.back().selectorAttribute = borderSelector;
    m_attributes.back().enumItems = qMove(borderEffectEnumItems);

    createAttribute(ObjectEditorAttributeType::Double, QByteArrayList() << "BE" << "I", tr("Border"), tr("Border Effect"), tr("Intensity"), PDFObject::createReal(0.0), FreeText | Circle | Square | Polygon);
    m_attributes.back().selectorAttribute = borderSelector;
    m_attributes.back().minValue = 0.0;
    m_attributes.back().maxValue = 2.0;

    // Sticky note annotation
    createAttribute(ObjectEditorAttributeType::ComboBox, "Name", tr("Sticky note"), tr("Sticky note"), tr("Type"), PDFObject::createName("Note"), Text);

    PDFObjectEditorModelAttributeEnumItems stickyNoteEnum;
    stickyNoteEnum.emplace_back(tr("Comment"), 0, PDFObject::createName("Comment"));
    stickyNoteEnum.emplace_back(tr("Key"), 2, PDFObject::createName("Key"));
    stickyNoteEnum.emplace_back(tr("Note"), 4, PDFObject::createName("Note"));
    stickyNoteEnum.emplace_back(tr("Help"), 8, PDFObject::createName("Help"));
    stickyNoteEnum.emplace_back(tr("New Paragraph"), 16, PDFObject::createName("NewParagraph"));
    stickyNoteEnum.emplace_back(tr("Paragraph"), 32, PDFObject::createName("Paragraph"));
    stickyNoteEnum.emplace_back(tr("Insert"), 64, PDFObject::createName("Insert"));
    m_attributes.back().enumItems = qMove(stickyNoteEnum);

    createAttribute(ObjectEditorAttributeType::Boolean, "Open", tr("Sticky note"), tr("Sticky note"), tr("Open"), PDFObject::createBool(false), Text);

    // Link annotation
    createAttribute(ObjectEditorAttributeType::ComboBox, "H", tr("Link"), tr("Style"), tr("Highlight"), PDFObject::createName("I"), Link);

    PDFObjectEditorModelAttributeEnumItems linkHighlightEnumValues;
    linkHighlightEnumValues.emplace_back(tr("None"), 0, PDFObject::createName("N"));
    linkHighlightEnumValues.emplace_back(tr("Invert"), 2, PDFObject::createName("I"));
    linkHighlightEnumValues.emplace_back(tr("Outline"), 4, PDFObject::createName("O"));
    linkHighlightEnumValues.emplace_back(tr("Push"), 8, PDFObject::createName("P"));
    m_attributes.back().enumItems = qMove(linkHighlightEnumValues);

    // Free text annotation
    createQuaddingAttribute("Q", tr("Free text"), tr("Style"), tr("Alignment"), FreeText);

    createAttribute(ObjectEditorAttributeType::ComboBox, "IT", tr("Free text"), tr("Style"), tr("Intent"), PDFObject::createName("FreeText"), FreeText);
    PDFObjectEditorModelAttributeEnumItems freeTextIntent;
    freeTextIntent.emplace_back(tr("Free text"), 0, PDFObject::createName("FreeText"));
    freeTextIntent.emplace_back(tr("Callout"), 1, PDFObject::createName("FreeTextCallout"));
    freeTextIntent.emplace_back(tr("Typewriter"), 2, PDFObject::createName("FreeTextTypeWriter"));
    m_attributes.back().enumItems = qMove(freeTextIntent);

    createLineEndingAttribute("LE", tr("Free text"), tr("Style"), tr("Callout line ending"), FreeText);

    // Line annotation
    createLineEndingAttribute("LE", tr("Line"), tr("Style"), tr("Line start"), Line | PolyLine);
    m_attributes.back().arrayIndex = 1;
    createLineEndingAttribute("LE", tr("Line"), tr("Style"), tr("Line end"), Line | PolyLine);
    m_attributes.back().arrayIndex = 2;

    createAttribute(ObjectEditorAttributeType::Double, "LL", tr("Line"), tr("Style"), tr("Leader line length"), PDFObject::createReal(0.0), Line, PDFObjectEditorModelAttribute::HideInsteadOfDisable);
    m_attributes.back().minValue = 0.0;

    createAttribute(ObjectEditorAttributeType::Double, "LLE", tr("Line"), tr("Style"), tr("Leader line extension"), PDFObject::createReal(0.0), Line, PDFObjectEditorModelAttribute::HideInsteadOfDisable);
    m_attributes.back().minValue = 0.0;

    createAttribute(ObjectEditorAttributeType::Double, "LLO", tr("Line"), tr("Style"), tr("Leader line offset"), PDFObject::createReal(0.0), Line, PDFObjectEditorModelAttribute::HideInsteadOfDisable);
    m_attributes.back().minValue = 0.0;

    createAttribute(ObjectEditorAttributeType::ComboBox, "IT", tr("Line"), tr("Style"), tr("Intent"), PDFObject::createName("LineArrow"), Line, PDFObjectEditorModelAttribute::HideInsteadOfDisable);
    PDFObjectEditorModelAttributeEnumItems lineIntent;
    lineIntent.emplace_back(tr("Arrow"), 0, PDFObject::createName("LineArrow"));
    lineIntent.emplace_back(tr("Dimension"), 1, PDFObject::createName("LineDimension"));
    m_attributes.back().enumItems = qMove(lineIntent);

    createAttribute(ObjectEditorAttributeType::ComboBox, "IT", tr("Line"), tr("Style"), tr("Intent"), PDFObject(), Polygon | PolyLine, PDFObjectEditorModelAttribute::HideInsteadOfDisable);
    PDFObjectEditorModelAttributeEnumItems polygonIntent;
    polygonIntent.emplace_back(tr("None"), 0, PDFObject());
    polygonIntent.emplace_back(tr("Cloud"), 1, PDFObject::createName("PolygonCloud"));
    polygonIntent.emplace_back(tr("Line dimension"), 2, PDFObject::createName("PolyLineDimension"));
    polygonIntent.emplace_back(tr("Polygon dimension"), 3, PDFObject::createName("PolygonDimension"));
    m_attributes.back().enumItems = qMove(polygonIntent);

    createAttribute(ObjectEditorAttributeType::Boolean, "Cap", tr("Line"), tr("Text"), tr("Caption"), PDFObject::createBool(false), Line);

    createAttribute(ObjectEditorAttributeType::ComboBox, "CP", tr("Line"), tr("Text"), tr("Caption position"), PDFObject::createName("Inline"), Line);
    PDFObjectEditorModelAttributeEnumItems lineCaptionPosition;
    lineCaptionPosition.emplace_back(tr("Inline"), 0, PDFObject::createName("Inline"));
    lineCaptionPosition.emplace_back(tr("Top"), 1, PDFObject::createName("Top"));
    m_attributes.back().enumItems = qMove(lineCaptionPosition);

    createAttribute(ObjectEditorAttributeType::ComboBox, "Name", tr("Stamp"), tr("Style"), tr("Name"), PDFObject::createName("Draft"), Stamp);
    PDFObjectEditorModelAttributeEnumItems stampNameEnumItems;
    int stampIndex = 0;
    for (pdf::Stamp stampType : { Stamp::Approved, Stamp::AsIs, Stamp::Confidential, Stamp::Departmental,
         Stamp::Draft, Stamp::Experimental, Stamp::Expired, Stamp::Final, Stamp::ForComment,
         Stamp::ForPublicRelease, Stamp::NotApproved, Stamp::NotForPublicRelease, Stamp::Sold, Stamp::TopSecret })
    {
        PDFObjectFactory factory;
        factory << stampType;

        stampNameEnumItems.emplace_back(PDFStampAnnotation::getText(stampType), stampIndex++, factory.takeObject());
    }
    m_attributes.back().enumItems = qMove(stampNameEnumItems);

    createAttribute(ObjectEditorAttributeType::ComboBox, "IT", tr("Stamp"), tr("Style"), tr("Intent"), PDFObject::createName("Stamp"), Stamp);
    PDFObjectEditorModelAttributeEnumItems stampEnumItems;
    stampEnumItems.emplace_back(tr("Stamp"), 0, PDFObject::createName("Stamp"));
    stampEnumItems.emplace_back(tr("Image"), 1, PDFObject::createName("StampImage"));
    stampEnumItems.emplace_back(tr("Snapshot"), 2, PDFObject::createName("StampSnapshot"));
    m_attributes.back().enumItems = qMove(stampEnumItems);

    createAttribute(ObjectEditorAttributeType::ComboBox, "Name", tr("File attachment"), tr("Style"), tr("Icon"), PDFObject::createName("PushPin"), FileAttachment);
    PDFObjectEditorModelAttributeEnumItems fileAttachmentEnumItems;
    fileAttachmentEnumItems.emplace_back(tr("Graph"), 0, PDFObject::createName("Graph"));
    fileAttachmentEnumItems.emplace_back(tr("Push-pin"), 1, PDFObject::createName("PushPin"));
    fileAttachmentEnumItems.emplace_back(tr("Paperclip"), 2, PDFObject::createName("Paperclip"));
    fileAttachmentEnumItems.emplace_back(tr("Tag"), 3, PDFObject::createName("Tag"));
    m_attributes.back().enumItems = qMove(fileAttachmentEnumItems);

    // Redact
    createAttribute(ObjectEditorAttributeType::TextLine, "OverlayText", tr("Redact"), tr("Appearance"), tr("Overlay text"), PDFObject(), Redact);
    createAttribute(ObjectEditorAttributeType::Boolean, "Repeat", tr("Redact"), tr("Appearance"), tr("Repeat overlay text"), PDFObject::createBool(false), Redact);
    createQuaddingAttribute("Q", tr("Redact"), tr("Appearance"), tr("Alignment"), Redact);

    initialize();
}

size_t PDFObjectEditorAnnotationsModel::createQuaddingAttribute(QByteArray attributeName, QString category, QString subcategory, QString name, uint32_t typeFlags)
{
    size_t attribute = createAttribute(ObjectEditorAttributeType::ComboBox, qMove(attributeName), qMove(category), qMove(subcategory), qMove(name), PDFObject::createInteger(0), typeFlags);

    PDFObjectEditorModelAttributeEnumItems quaddingEnumValues;
    quaddingEnumValues.emplace_back(tr("Left"), 0, PDFObject::createInteger(0));
    quaddingEnumValues.emplace_back(tr("Center"), 1, PDFObject::createInteger(1));
    quaddingEnumValues.emplace_back(tr("Right"), 2, PDFObject::createInteger(2));
    m_attributes.back().enumItems = qMove(quaddingEnumValues);

    return attribute;
}

size_t PDFObjectEditorAnnotationsModel::createLineEndingAttribute(QByteArray attributeName, QString category, QString subcategory, QString name, uint32_t typeFlags)
{
    size_t attribute = createAttribute(ObjectEditorAttributeType::ComboBox, qMove(attributeName), qMove(category), qMove(subcategory), qMove(name), PDFObject::createInteger(0), typeFlags);

    PDFObjectEditorModelAttributeEnumItems lineEndingEnumValues;
    lineEndingEnumValues.emplace_back(tr("None"), 0, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::None)));
    lineEndingEnumValues.emplace_back(tr("Square"), 1, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::Square)));
    lineEndingEnumValues.emplace_back(tr("Circle"), 2, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::Circle)));
    lineEndingEnumValues.emplace_back(tr("Diamond"), 3, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::Diamond)));
    lineEndingEnumValues.emplace_back(tr("Open arrow"), 4, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::OpenArrow)));
    lineEndingEnumValues.emplace_back(tr("Closed arrow"), 5, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::ClosedArrow)));
    lineEndingEnumValues.emplace_back(tr("Butt"), 6, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::Butt)));
    lineEndingEnumValues.emplace_back(tr("Reversed open arrow"), 7, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::ROpenArrow)));
    lineEndingEnumValues.emplace_back(tr("Reversed closed arrow"), 8, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::RClosedArrow)));
    lineEndingEnumValues.emplace_back(tr("Slash"), 9, PDFObject::createName(PDFAnnotation::convertLineEndingToName(AnnotationLineEnding::Slash)));
    m_attributes.back().enumItems = qMove(lineEndingEnumValues);

    return attribute;
}

} // namespace pdf
