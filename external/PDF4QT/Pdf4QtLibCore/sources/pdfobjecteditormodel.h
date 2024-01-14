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
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFOBJECTEDITORABSTRACTMODEL_H
#define PDFOBJECTEDITORABSTRACTMODEL_H

#include "pdfobject.h"
#include "pdfdocument.h"

#include <QStringList>

namespace pdf
{

enum ObjectEditorAttributeType
{
    Constant,       ///< Constant attribute, which is always written to the object
    Type,           ///< Type attribute, switches between main types of object
    TextLine,       ///< Single line text
    TextBrowser,    ///< Multiple line text
    Rectangle,      ///< Rectangle defined by x,y,width,height
    DateTime,       ///< Date/time
    Flags,          ///< Flags
    Selector,       ///< Selector attribute, it is not persisted
    Color,          ///< Color
    Double,         ///< Double value
    ComboBox,       ///< Combo box with predefined set of items
    Boolean,        ///< Check box
    Invalid
};

struct PDFObjectEditorModelAttributeEnumItem
{
    PDFObjectEditorModelAttributeEnumItem() = default;
    PDFObjectEditorModelAttributeEnumItem(QString name, uint32_t flags, PDFObject value) :
        name(qMove(name)),
        flags(flags),
        value(qMove(value))
    {

    }

    QString name;
    uint32_t flags = 0;
    PDFObject value;
};

using PDFObjectEditorModelAttributeEnumItems = std::vector<PDFObjectEditorModelAttributeEnumItem>;

struct PDFObjectEditorModelAttribute
{
    enum Flag
    {
        None                    = 0x0000,
        Readonly                = 0x0001,   ///< Attribute is always read only
        HideInsteadOfDisable    = 0x0002,   ///< Hide all widgets of this attribute, if it is disabled
        Hidden                  = 0x0004,   ///< Attribute is always hidden (not viewable in gui)
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    /// Attribute type
    ObjectEditorAttributeType type = ObjectEditorAttributeType::Invalid;

    /// Attribute name in object dictionary. In case of subdictionaries,
    /// there can be a path of constisting of dictionary names and last
    /// string in the string list is key in the dictionary. If this attribute
    /// is empty, then this attribute is not represented in final object.
    QByteArrayList dictionaryAttribute;

    /// Category
    QString category;

    /// Subcategory
    QString subcategory;

    /// Name of the attribute, which is displayed in the gui.
    QString name;

    /// Default value
    PDFObject defaultValue;

    /// Type flags, this filters attributes by object type. If set to zero,
    /// then this attribute doesn't depend on object type.
    uint32_t typeFlags = 0;

    /// Attribute flags
    Flags attributeFlags = None;

    /// Selector attribute. This is a zero (no selector attribute), or valid
    /// selector attribute which is a bool attribute. Selector attribute turns
    /// on/off displaying of this attribute. If selector state is off, then default
    /// value is stored into the object. If selector state is on, then user
    /// can select the attribute value.
    size_t selectorAttribute = 0;

    /// If this value is nonzero, it marks that value is stored in the array,
    /// with 1-based index arrayIndex.
    size_t arrayIndex = 0;

    QVariant minValue;
    QVariant maxValue;

    /// Enum items
    PDFObjectEditorModelAttributeEnumItems enumItems;

    /// Value for selector attribute
    bool selectorAttributeValue = false;
};

class PDF4QTLIBCORESHARED_EXPORT PDFObjectEditorAbstractModel : public QObject
{
    Q_OBJECT

private:
    using BaseClass = QObject;

public:
    explicit PDFObjectEditorAbstractModel(QObject* parent);
    virtual ~PDFObjectEditorAbstractModel();

    size_t getAttributeCount() const;
    ObjectEditorAttributeType getAttributeType(size_t index) const;
    const QString& getAttributeCategory(size_t index) const;
    const QString& getAttributeSubcategory(size_t index) const;
    const QString& getAttributeName(size_t index) const;
    const PDFObjectEditorModelAttributeEnumItems& getAttributeEnumItems(size_t index) const;

    enum class Question
    {
        IsMapped,               ///< Is attribute mapped in gui?
        IsSelector,             ///< Is attribute's role a selector for other attributes?
        IsPersisted,            ///< Is attribute persisted to edited object?
        IsVisible,              ///< Is attribute visible (perhaps disabled)?
        HasAttribute,           ///< Does current object has given attribute?
        HasSimilarAttribute,    ///< Does current object has some attribute, which is persisted to same dictionary item as current attribute?
        IsAttributeEditable     ///< Is current attribute editable (this implies previous flag) ?
    };

    bool queryAttribute(size_t index, Question question) const;

    bool getSelectorValue(size_t index) const;
    void setSelectorValue(size_t index, bool value);

    /// Returns vector of attributes, which are of type selector
    std::vector<size_t> getSelectorAttributes() const;

    /// Returns vector of attributes, which are turned on/off
    /// by this selector.
    std::vector<size_t> getSelectorDependentAttributes(size_t selector) const;

    /// Returns value stored in the object. If array index of the attribute
    /// is specified, and resolveArrayIndex is true, then value from the array
    /// is retrieved. Otherwise, array itself is returned.
    /// \param index Attribute index
    /// \param resolveArrayIndex For array attribute, retrieve array item (true), or array itself (false)
    PDFObject getValue(size_t index, bool resolveArrayIndex) const;

    PDFObject getDefaultValue(size_t index) const;
    PDFObject getEditedObject() const { return m_editedObject; }
    void setEditedObject(PDFObject object);

    /// Writes attribute value to the object.
    /// \param attribute Attribute
    /// \param object Old object
    /// \param value Value
    PDFObject writeAttributeValueToObject(size_t attribute, PDFObject object, PDFObject value) const;

    /// Returns minimum value of the attribute
    /// \param index Attribute index
    QVariant getMinimumValue(size_t index) const;

    /// Returns maximum value of the attribute
    /// \param index Attribute index
    QVariant getMaximumValue(size_t index) const;

    const PDFObjectStorage* getStorage() const { return m_storage; }

signals:
    void editedObjectChanged();

protected:
    void initialize();
    void updateSelectorValues();

    size_t createAttribute(ObjectEditorAttributeType type,
                           QByteArray attributeName,
                           QString category,
                           QString subcategory,
                           QString name,
                           PDFObject defaultValue = PDFObject(),
                           uint32_t typeFlags = 0,
                           PDFObjectEditorModelAttribute::Flags flags = PDFObjectEditorModelAttribute::None);

    size_t createAttribute(ObjectEditorAttributeType type,
                           QByteArrayList attributesName,
                           QString category,
                           QString subcategory,
                           QString name,
                           PDFObject defaultValue = PDFObject(),
                           uint32_t typeFlags = 0,
                           PDFObjectEditorModelAttribute::Flags flags = PDFObjectEditorModelAttribute::None);

    size_t createSelectorAttribute(QString category,
                                   QString subcategory,
                                   QString name);

    uint32_t getCurrentTypeFlags() const;

    PDFObject getDefaultColor() const;

    std::vector<PDFObjectEditorModelAttribute> m_attributes;

    PDFObject m_editedObject;
    const PDFObjectStorage* m_storage;
    size_t m_typeAttribute;
    std::map<size_t, std::vector<size_t>> m_similarAttributes;
};

class PDF4QTLIBCORESHARED_EXPORT PDFObjectEditorAnnotationsModel : public PDFObjectEditorAbstractModel
{
    Q_OBJECT

private:
    using BaseClass = PDFObjectEditorAbstractModel;

    enum AnnotationTypes : uint32_t
    {
        Text            = 1 << 0,
        Link            = 1 << 1,
        FreeText        = 1 << 2,
        Line            = 1 << 3,
        Square          = 1 << 4,
        Circle          = 1 << 5,
        Polygon         = 1 << 6,
        PolyLine        = 1 << 7,
        Highlight       = 1 << 8,
        Underline       = 1 << 9,
        Squiggly        = 1 << 10,
        StrikeOut       = 1 << 11,
        Caret           = 1 << 12,
        Stamp           = 1 << 13,
        Ink             = 1 << 14,
        FileAttachment  = 1 << 15,
        Redact          = 1 << 16,

        Markup = Text | FreeText | Line | Square | Circle | Polygon | PolyLine | Highlight | Underline | Squiggly | StrikeOut | Caret | Stamp | Ink | FileAttachment | Redact
    };

public:
    explicit PDFObjectEditorAnnotationsModel(QObject* parent);

private:
    size_t createQuaddingAttribute(QByteArray attributeName,
                                   QString category,
                                   QString subcategory,
                                   QString name,
                                   uint32_t typeFlags = 0);

    size_t createLineEndingAttribute(QByteArray attributeName,
                                     QString category,
                                     QString subcategory,
                                     QString name,
                                     uint32_t typeFlags = 0);
};

} // namespace pdf

#endif // PDFOBJECTEDITORABSTRACTMODEL_H
