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

#include "pdfstructuretree.h"
#include "pdfdocument.h"
#include "pdfnametreeloader.h"
#include "pdfnumbertreeloader.h"
#include "pdfpagecontentprocessor.h"
#include "pdfcms.h"
#include "pdfexecutionpolicy.h"
#include "pdfconstants.h"
#include "pdfdbgheap.h"

#include <array>

namespace pdf
{

/// Attribute definition structure
struct PDFStructureTreeAttributeDefinition
{
    constexpr inline PDFStructureTreeAttributeDefinition() = default;
    constexpr inline PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute type,
                                                         const char* name,
                                                         bool inheritable) :
        type(type),
        name(name),
        inheritable(inheritable)
    {

    }

    /// Returns attribute definition for given attribute name. This function
    /// always returns valid pointer. For uknown attribute, it returns
    /// user attribute definition.
    /// \param name Attribute name
    static const PDFStructureTreeAttributeDefinition* getDefinition(const QByteArray& name);

    /// Returns attribute definition for given attribute type. This function
    /// always returns valid pointer. For uknown attribute, it returns
    /// user attribute definition.
    /// \param name Attribute name
    static const PDFStructureTreeAttributeDefinition* getDefinition(PDFStructureTreeAttribute::Attribute type);

    /// Returns owner from string. If owner is not valid, then invalid
    /// owner is returned.
    /// \param string String
    static PDFStructureTreeAttribute::Owner getOwnerFromString(const QByteArray& string);

    /// Returns string from owner. If owner is not valid, then invalid string is returned.
    /// \param owner Owner
    static QString getOwnerName(PDFStructureTreeAttribute::Owner owner);

    PDFStructureTreeAttribute::Attribute type = PDFStructureTreeAttribute::Attribute::User;
    const char* name = nullptr;
    bool inheritable = false;
};


static constexpr std::array<std::pair<const char*, const PDFStructureTreeAttribute::Owner>, 16> s_ownerDefinitions =
{
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("Layout", PDFStructureTreeAttribute::Owner::Layout),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("List", PDFStructureTreeAttribute::Owner::List),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("PrintField", PDFStructureTreeAttribute::Owner::PrintField),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("Table", PDFStructureTreeAttribute::Owner::Table),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("Artifact", PDFStructureTreeAttribute::Owner::Artifact),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("XML-1.00", PDFStructureTreeAttribute::Owner::XML_1_00),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("HTML-3.20", PDFStructureTreeAttribute::Owner::HTML_3_20),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("HTML-4.01", PDFStructureTreeAttribute::Owner::HTML_4_01),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("HTML-5.00", PDFStructureTreeAttribute::Owner::HTML_5_00),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("OEB-1.00", PDFStructureTreeAttribute::Owner::OEB_1_00),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("RTF-1.05", PDFStructureTreeAttribute::Owner::RTF_1_05),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("CSS-1.00", PDFStructureTreeAttribute::Owner::CSS_1_00),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("CSS-2.00", PDFStructureTreeAttribute::Owner::CSS_2_00),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("CSS-3.00", PDFStructureTreeAttribute::Owner::CSS_3_00),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("RDFa-1.10", PDFStructureTreeAttribute::Owner::RDFa_1_10),
    std::pair<const char*, const PDFStructureTreeAttribute::Owner>("ARIA-1.1", PDFStructureTreeAttribute::Owner::ARIA_1_1)
};

static constexpr std::array<const PDFStructureTreeAttributeDefinition, PDFStructureTreeAttribute::Attribute::LastAttribute + 1> s_attributeDefinitions =
{
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::User, "", false), // User

    // Standard layout attributes
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Placement, "Placement", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::WritingMode, "WritingMode", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::BackgroundColor, "BackgroundColor", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::BorderColor, "BorderColor", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::BorderStyle, "BorderStyle", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::BorderThickness, "BorderThickness", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Color, "Color", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Padding, "Padding", false),

    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::SpaceBefore, "SpaceBefore", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::SpaceAfter, "SpaceAfter", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::StartIndent, "StartIndent", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::EndIndent, "EndIndent", true),

    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::TextIndent, "TextIndent", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::TextAlign, "TextAlign", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::BBox, "BBox", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Width, "Width", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Height, "Height", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::BlockAlign, "BlockAlign", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::InlineAlign, "InlineAlign", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::TBorderStyle, "TBorderStyle", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::TPadding, "TPadding", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::LineHeight, "LineHeight", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::BaselineShift, "BaselineShift", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::TextPosition, "TextPosition", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::TextDecorationType, "TextDecorationType", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::TextDecorationColor, "TextDecorationColor", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::TextDecorationThickness, "TextDecorationThickness", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::ColumnCount, "ColumnCount", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::ColumnWidths, "ColumnWidths", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::ColumnGap, "ColumnGap", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::GlyphOrientationVertical, "GlyphOrientationVertical", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::RubyAlign, "RubyAlign", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::RubyPosition, "RubyPosition", true),

    // List attributes
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::ListNumbering, "ListNumbering", true),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::ContinuedList, "ContinuedList", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::ContinuedFrom, "ContinuedFrom", false),

    // PrintField attributes
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Role, "Role", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Checked, "Checked", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Checked, "checked", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Desc, "Desc", false),

    // Table attributes
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::RowSpan, "RowSpan", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::ColSpan, "ColSpan", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Headers, "Headers", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Scope, "Scope", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Short, "Short", false),

    // Artifact attributes
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Type, "Type", false),
    PDFStructureTreeAttributeDefinition(PDFStructureTreeAttribute::Attribute::Subtype, "Subtype", false)
};

static constexpr std::pair<PDFStructureItem::Type, const char*> s_structureTreeItemTypes[] = {
    std::make_pair(PDFStructureItem::Document, "Document"),
    std::make_pair(PDFStructureItem::DocumentFragment, "DocumentFragment"),
    std::make_pair(PDFStructureItem::Part, "Part"),
    std::make_pair(PDFStructureItem::Div, "Div"),
    std::make_pair(PDFStructureItem::Aside, "Aside"),
    std::make_pair(PDFStructureItem::P, "P"),
    std::make_pair(PDFStructureItem::H1, "H1"),
    std::make_pair(PDFStructureItem::H2, "H2"),
    std::make_pair(PDFStructureItem::H3, "H3"),
    std::make_pair(PDFStructureItem::H4, "H4"),
    std::make_pair(PDFStructureItem::H5, "H5"),
    std::make_pair(PDFStructureItem::H6, "H6"),
    std::make_pair(PDFStructureItem::H7, "H7"),
    std::make_pair(PDFStructureItem::H, "H"),
    std::make_pair(PDFStructureItem::Title, "Title"),
    std::make_pair(PDFStructureItem::FENote, "FENote"),
    std::make_pair(PDFStructureItem::Sub, "Sub"),
    std::make_pair(PDFStructureItem::Lbl, "Lbl"),
    std::make_pair(PDFStructureItem::Span, "Span"),
    std::make_pair(PDFStructureItem::Em, "Em"),
    std::make_pair(PDFStructureItem::Strong, "Strong"),
    std::make_pair(PDFStructureItem::Link, "Link"),
    std::make_pair(PDFStructureItem::Annot, "Annot"),
    std::make_pair(PDFStructureItem::Form, "Form"),
    std::make_pair(PDFStructureItem::Ruby, "Ruby"),
    std::make_pair(PDFStructureItem::RB, "RB"),
    std::make_pair(PDFStructureItem::RT, "RT"),
    std::make_pair(PDFStructureItem::RP, "RP"),
    std::make_pair(PDFStructureItem::Warichu, "Warichu"),
    std::make_pair(PDFStructureItem::WR, "WR"),
    std::make_pair(PDFStructureItem::WP, "WP"),
    std::make_pair(PDFStructureItem::L, "L"),
    std::make_pair(PDFStructureItem::LI, "LI"),
    std::make_pair(PDFStructureItem::LBody, "LBody"),
    std::make_pair(PDFStructureItem::Table, "Table"),
    std::make_pair(PDFStructureItem::TR, "TR"),
    std::make_pair(PDFStructureItem::TH, "TH"),
    std::make_pair(PDFStructureItem::TD, "TD"),
    std::make_pair(PDFStructureItem::THead, "THead"),
    std::make_pair(PDFStructureItem::TBody, "TBody"),
    std::make_pair(PDFStructureItem::TFoot, "TFoot"),
    std::make_pair(PDFStructureItem::Caption, "Caption"),
    std::make_pair(PDFStructureItem::Figure, "Figure"),
    std::make_pair(PDFStructureItem::Formula, "Formula"),
    std::make_pair(PDFStructureItem::Artifact, "Artifact"),
    std::make_pair(PDFStructureItem::Sect, "Sect"),
    std::make_pair(PDFStructureItem::Art, "Art"),
    std::make_pair(PDFStructureItem::BlockQuote, "BlockQuote"),
    std::make_pair(PDFStructureItem::TOC, "TOC"),
    std::make_pair(PDFStructureItem::TOCI, "TOCI"),
    std::make_pair(PDFStructureItem::Index, "Index"),
    std::make_pair(PDFStructureItem::NonStruct, "NonStruct"),
    std::make_pair(PDFStructureItem::Private, "Private"),
    std::make_pair(PDFStructureItem::Quote, "Quote"),
    std::make_pair(PDFStructureItem::Note, "Note"),
    std::make_pair(PDFStructureItem::Reference, "Reference"),
    std::make_pair(PDFStructureItem::BibEntry, "BibEntry"),
    std::make_pair(PDFStructureItem::Code, "Code")
};

const PDFStructureTreeAttributeDefinition* PDFStructureTreeAttributeDefinition::getDefinition(const QByteArray& name)
{
    for (const PDFStructureTreeAttributeDefinition& definition : s_attributeDefinitions)
    {
        if (name == definition.name)
        {
            return &definition;
        }
    }

    Q_ASSERT(s_attributeDefinitions.front().type == PDFStructureTreeAttribute::Attribute::User);
    return &s_attributeDefinitions.front();
}

const PDFStructureTreeAttributeDefinition* PDFStructureTreeAttributeDefinition::getDefinition(PDFStructureTreeAttribute::Attribute type)
{
    for (const PDFStructureTreeAttributeDefinition& definition : s_attributeDefinitions)
    {
        if (type == definition.type)
        {
            return &definition;
        }
    }

    Q_ASSERT(s_attributeDefinitions.front().type == PDFStructureTreeAttribute::Attribute::User);
    return &s_attributeDefinitions.front();
}

PDFStructureTreeAttribute::Owner PDFStructureTreeAttributeDefinition::getOwnerFromString(const QByteArray& string)
{
    for (const auto& item : s_ownerDefinitions)
    {
        if (string == item.first)
        {
            return item.second;
        }
    }

    return PDFStructureTreeAttribute::Owner::Invalid;
}

QString PDFStructureTreeAttributeDefinition::getOwnerName(PDFStructureTreeAttribute::Owner owner)
{
    for (const auto& item : s_ownerDefinitions)
    {
        if (owner == item.second)
        {
            return QString::fromLatin1(item.first);
        }
    }

    return QString();
}

PDFStructureTreeAttribute::PDFStructureTreeAttribute() :
    m_definition(&s_attributeDefinitions.front()),
    m_owner(Owner::Invalid),
    m_revision(0),
    m_namespace(),
    m_value()
{

}

PDFStructureTreeAttribute::PDFStructureTreeAttribute(const PDFStructureTreeAttributeDefinition* definition,
                                                     PDFStructureTreeAttribute::Owner owner,
                                                     PDFInteger revision,
                                                     PDFObjectReference namespaceReference,
                                                     PDFObject value) :
    m_definition(definition),
    m_owner(owner),
    m_revision(revision),
    m_namespace(namespaceReference),
    m_value(qMove(value))
{

}

PDFStructureTreeAttribute::Attribute PDFStructureTreeAttribute::getType() const
{
    Q_ASSERT(m_definition);
    return m_definition->type;
}

QString PDFStructureTreeAttribute::getTypeName(const PDFObjectStorage* storage) const
{
    if (isUser())
    {
        return getUserPropertyName(storage);
    }

    Q_ASSERT(m_definition);
    return QString::fromLatin1(m_definition->name);
}

QString PDFStructureTreeAttribute::getOwnerName() const
{
    return PDFStructureTreeAttributeDefinition::getOwnerName(getOwner());
}

bool PDFStructureTreeAttribute::isInheritable() const
{
    Q_ASSERT(m_definition);
    return m_definition->inheritable;
}

PDFObject PDFStructureTreeAttribute::getDefaultValue() const
{
    switch (m_definition->type)
    {
        case PDFStructureTreeAttribute::WritingMode:
            return PDFObject::createName("LrTb");

        case PDFStructureTreeAttribute::BorderStyle:
        case PDFStructureTreeAttribute::TBorderStyle:
        case PDFStructureTreeAttribute::TextDecorationType:
        case PDFStructureTreeAttribute::ListNumbering:
            return PDFObject::createName("None");

        case PDFStructureTreeAttribute::BorderThickness:
        case PDFStructureTreeAttribute::Padding:
        case PDFStructureTreeAttribute::SpaceBefore:
        case PDFStructureTreeAttribute::SpaceAfter:
        case PDFStructureTreeAttribute::StartIndent:
        case PDFStructureTreeAttribute::EndIndent:
        case PDFStructureTreeAttribute::TextIndent:
        case PDFStructureTreeAttribute::TPadding:
        case PDFStructureTreeAttribute::BaselineShift:
            return PDFObject::createReal(0.0);

        case PDFStructureTreeAttribute::TextAlign:
        case PDFStructureTreeAttribute::InlineAlign:
            return PDFObject::createName("Start");

        case PDFStructureTreeAttribute::Width:
        case PDFStructureTreeAttribute::Height:
        case PDFStructureTreeAttribute::GlyphOrientationVertical:
            return PDFObject::createName("Auto");

        case PDFStructureTreeAttribute::BlockAlign:
        case PDFStructureTreeAttribute::RubyPosition:
            return PDFObject::createName("Before");

        case PDFStructureTreeAttribute::LineHeight:
        case PDFStructureTreeAttribute::TextPosition:
            return PDFObject::createName("Normal");

        case PDFStructureTreeAttribute::RubyAlign:
            return PDFObject::createName("Distribute");

        case PDFStructureTreeAttribute::ColumnCount:
        case PDFStructureTreeAttribute::RowSpan:
        case PDFStructureTreeAttribute::ColSpan:
            return PDFObject::createInteger(1);

        case PDFStructureTreeAttribute::Checked:
            return PDFObject::createName("off");

        default:
            break;
    }

    return PDFObject();
}

QString PDFStructureTreeAttribute::getUserPropertyName(const PDFObjectStorage* storage) const
{
    if (const PDFDictionary* value = storage->getDictionaryFromObject(m_value))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        return loader.readTextStringFromDictionary(value, "N", QString());
    }

    return QString();
}

PDFObject PDFStructureTreeAttribute::getUserPropertyValue(const PDFObjectStorage* storage) const
{
    if (const PDFDictionary* value = storage->getDictionaryFromObject(m_value))
    {
        return value->get("V");
    }

    return PDFObject();
}

QString PDFStructureTreeAttribute::getUserPropertyFormattedValue(const PDFObjectStorage* storage) const
{
    if (const PDFDictionary* value = storage->getDictionaryFromObject(m_value))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        return loader.readTextStringFromDictionary(value, "F", QString());
    }

    return QString();
}

bool PDFStructureTreeAttribute::getUserPropertyIsHidden(const PDFObjectStorage* storage) const
{
    if (const PDFDictionary* value = storage->getDictionaryFromObject(m_value))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        return loader.readBooleanFromDictionary(value, "H", false);
    }

    return false;
}

void PDFStructureTreeAttribute::parseAttributes(const PDFObjectStorage* storage, PDFObject object, std::vector<PDFStructureTreeAttribute>& attributes)
{
    object = storage->getObject(object);
    if (object.isDictionary())
    {
        parseAttributeDictionary(storage, object, attributes);
    }
    else if (object.isArray())
    {
        size_t startIndex = attributes.size();

        for (PDFObject itemObject : *object.getArray())
        {
            itemObject = storage->getObject(itemObject);
            if (itemObject.isInt())
            {
                // It is revision number
                const PDFInteger revision = itemObject.getInteger();
                for (; startIndex < attributes.size(); ++startIndex)
                {
                    attributes[startIndex].setRevision(revision);
                }
            }
            else if (itemObject.isDictionary())
            {
                // It is attribute
                parseAttributeDictionary(storage, itemObject, attributes);
            }
        }
    }
}

void PDFStructureTreeAttribute::parseAttributeDictionary(const PDFObjectStorage* storage, PDFObject object, std::vector<PDFStructureTreeAttribute>& attributes)
{
    Q_ASSERT(object.isDictionary());
    const PDFDictionary* attributeDictionary = object.getDictionary();

    PDFDocumentDataLoaderDecorator loader(storage);
    const QByteArray ownerName = loader.readNameFromDictionary(attributeDictionary, "O");
    const Owner owner = PDFStructureTreeAttributeDefinition::getOwnerFromString(ownerName);
    if (owner == Owner::UserProperties)
    {
        // User properties
        PDFObject userPropertiesArrayObject = storage->getObject(attributeDictionary->get("P"));
        if (userPropertiesArrayObject.isArray())
        {
            const PDFArray* userPropertiesArray = userPropertiesArrayObject.getArray();
            for (const PDFObject& userPropertyObject : *userPropertiesArray)
            {
                attributes.emplace_back(&s_attributeDefinitions.front(), owner, 0, PDFObjectReference(), userPropertyObject);
            }
        }
    }
    else
    {
        const PDFObjectReference namespaceReference = loader.readReferenceFromDictionary(attributeDictionary, "NS");
        const size_t count = attributeDictionary->getCount();
        for (size_t i = 0; i < count; ++i)
        {
            const PDFInplaceOrMemoryString& key = attributeDictionary->getKey(i);
            if (key == "O" || key == "NS")
            {
                continue;
            }

            attributes.emplace_back(PDFStructureTreeAttributeDefinition::getDefinition(key.getString()), owner, 0, namespaceReference, attributeDictionary->getValue(i));
        }
    }
}

std::vector<PDFObjectReference> PDFStructureTree::getParents(PDFInteger id) const
{
    std::vector<PDFObjectReference> result;
    ParentTreeEntry entry{ id, PDFObjectReference() };

    Q_ASSERT(std::is_sorted(m_parentTreeEntries.cbegin(), m_parentTreeEntries.cend()));
    auto iterators = std::equal_range(m_parentTreeEntries.cbegin(), m_parentTreeEntries.cend(), entry);
    result.reserve(std::distance(iterators.first, iterators.second));
    std::transform(iterators.first, iterators.second, std::back_inserter(result), [](const auto& item) { return item.reference; });
    return result;
}

PDFObjectReference PDFStructureTree::getParent(PDFInteger id, PDFInteger index) const
{
    Q_ASSERT(std::is_sorted(m_parentTreeEntries.cbegin(), m_parentTreeEntries.cend()));
    ParentTreeEntry entry{ id, PDFObjectReference() };
    auto [it, itEnd] = std::equal_range(m_parentTreeEntries.cbegin(), m_parentTreeEntries.cend(), entry);
    const PDFInteger count = std::distance(it, itEnd);
    if (index >= 0 && index < count)
    {
        return (*std::next(it, index)).reference;
    }
    return PDFObjectReference();
}

PDFStructureItem::Type PDFStructureTree::getTypeFromRole(const QByteArray& role) const
{
    auto it = m_roleMap.find(role);
    if (it != m_roleMap.cend())
    {
        return it->second;
    }

    return getTypeFromName(role);
}

const std::vector<PDFStructureTreeAttribute>& PDFStructureTree::getClassAttributes(const QByteArray& className) const
{
    auto it = m_classMap.find(className);
    if (it != m_classMap.cend())
    {
        return it->second;
    }

    static const std::vector<PDFStructureTreeAttribute> dummy;
    return dummy;
}

PDFStructureTree PDFStructureTree::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFStructureTree tree;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        PDFMarkedObjectsContext context;
        parseKids(storage, &tree, dictionary, &context);

        if (dictionary->hasKey("IDTree"))
        {
            tree.m_idTreeMap = PDFNameTreeLoader<PDFObjectReference>::parse(storage, dictionary->get("IDTree"), [](const PDFObjectStorage*, const PDFObject& object) { return object.isReference() ? object.getReference() : PDFObjectReference(); });
        }

        if (dictionary->hasKey("ParentTree"))
        {
            struct ParentTreeParseEntry
            {
                PDFInteger id = 0;
                std::vector<PDFObjectReference> references;

                bool operator<(const ParentTreeParseEntry& other) const
                {
                    return id < other.id;
                }

                static ParentTreeParseEntry parse(PDFInteger id, const PDFObjectStorage* storage, const PDFObject& object)
                {
                    const PDFObject& dereferencedObject = storage->getObject(object);

                    if (dereferencedObject.isArray())
                    {
                        std::vector<PDFObjectReference> references;
                        for (const PDFObject& objectInArray : *dereferencedObject.getArray())
                        {
                            if (objectInArray.isReference())
                            {
                                references.emplace_back(objectInArray.getReference());
                            }
                        }

                        return ParentTreeParseEntry{ id, qMove(references) };
                    }
                    else if (object.isReference())
                    {
                        return ParentTreeParseEntry{ id, { object.getReference() } };
                    }

                    return ParentTreeParseEntry{ id, { } };
                }
            };
            auto entries = PDFNumberTreeLoader<ParentTreeParseEntry>::parse(storage, dictionary->get("ParentTree"));
            for (const auto& entry : entries)
            {
                for (const PDFObjectReference& reference : entry.references)
                {
                    tree.m_parentTreeEntries.emplace_back(ParentTreeEntry{entry.id, reference});
                }
            }
            std::stable_sort(tree.m_parentTreeEntries.begin(), tree.m_parentTreeEntries.end());
        }

        tree.m_parentNextKey = loader.readIntegerFromDictionary(dictionary, "ParentTreeNextKey", 0);

        if (const PDFDictionary* roleMapDictionary = storage->getDictionaryFromObject(dictionary->get("RoleMap")))
        {
            const size_t size = roleMapDictionary->getCount();
            for (size_t i = 0; i < size; ++i)
            {
                tree.m_roleMap[roleMapDictionary->getKey(i).getString()] = getTypeFromName(loader.readName(roleMapDictionary->getValue(i)));
            }
        }

        if (const PDFDictionary* classMapDictionary = storage->getDictionaryFromObject(dictionary->get("ClassMap")))
        {
            const size_t size = classMapDictionary->getCount();
            for (size_t i = 0; i < size; ++i)
            {
                PDFStructureTreeAttribute::parseAttributes(storage, classMapDictionary->getValue(i), tree.m_classMap[classMapDictionary->getKey(i).getString()]);
            }
        }

        if (dictionary->hasKey("Namespaces"))
        {
            tree.m_namespaces = loader.readObjectList<PDFStructureTreeNamespace>(dictionary->get("Namespaces"));
        }

        if (dictionary->hasKey("PronunciationLexicon"))
        {
            tree.m_pronunciationLexicons = loader.readObjectList<PDFFileSpecification>(dictionary->get("PronunciationLexicon"));
        }

        if (dictionary->hasKey("AF"))
        {
            tree.m_associatedFiles = loader.readObjectList<PDFFileSpecification>(dictionary->get("AF"));
        }
    }

    return tree;
}

PDFStructureTree::ParentTreeEntry PDFStructureTree::getParentTreeEntry(PDFInteger index) const
{
    if (index >= 0 && index < PDFInteger(m_parentTreeEntries.size()))
    {
        return m_parentTreeEntries[index];
    }

    return ParentTreeEntry();
}

PDFStructureItemPointer PDFStructureItem::parse(const PDFObjectStorage* storage, PDFObject object, PDFMarkedObjectsContext* context, PDFStructureItem* parent)
{
    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        QByteArray typeName = loader.readNameFromDictionary(dictionary, "Type");

        if (typeName == "MCR")
        {
            return PDFStructureMarkedContentReference::parseMarkedContentReference(storage, object, context, parent, parent->getTree());
        }
        else if (typeName == "OBJR")
        {
            return PDFStructureObjectReference::parseObjectReference(storage, object, context, parent, parent->getTree());
        }
        else
        {
            return PDFStructureElement::parseElement(storage, object, context, parent, parent->getTree());
        }
    }

    return nullptr;
}

PDFStructureItem::Type PDFStructureItem::getTypeFromName(const QByteArray& name)
{
    for (const auto& item : s_structureTreeItemTypes)
    {
        if (name == item.second)
        {
            return item.first;
        }
    }

    return Invalid;
}

void PDFStructureItem::parseKids(const PDFObjectStorage* storage, PDFStructureItem* parentItem, const PDFDictionary* dictionary, PDFMarkedObjectsContext* context)
{
    PDFObject kids = dictionary->get("K");
    if (kids.isArray())
    {
        const PDFArray* kidsArray = kids.getArray();
        for (const PDFObject& object : *kidsArray)
        {
            PDFStructureItemPointer item = PDFStructureItem::parse(storage, object, context, parentItem);
            if (item)
            {
                parentItem->m_children.emplace_back(qMove(item));
            }
        }
    }
    else if (!kids.isNull())
    {
        PDFStructureItemPointer item = PDFStructureItem::parse(storage, kids, context, parentItem);
        if (item)
        {
            parentItem->m_children.emplace_back(qMove(item));
        }
    }
}

PDFStructureTreeNamespace PDFStructureTreeNamespace::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFStructureTreeNamespace result;

    if (object.isReference())
    {
        result.m_selfReference = object.getReference();
    }
    object = storage->getObject(object);

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_namespace = loader.readTextStringFromDictionary(dictionary, "NS", QString());
        result.m_schema = PDFFileSpecification::parse(storage, dictionary->get("Schema"));
        result.m_roleMapNS = dictionary->get("RoleMapNS");
    }

    return result;
}

const PDFStructureTreeAttribute* PDFStructureElement::findAttribute(Attribute attribute,
                                                                    AttributeOwner owner,
                                                                    RevisionPolicy policy) const
{
    const PDFStructureTreeAttributeDefinition* definition = PDFStructureTreeAttributeDefinition::getDefinition(attribute);

    if (const PDFStructureTreeAttribute* result = findAttributeImpl(attribute, owner, policy, definition))
    {
        return result;
    }

    if (owner != AttributeOwner::Invalid)
    {
        return findAttributeImpl(attribute, AttributeOwner::Invalid, policy, definition);
    }

    return nullptr;
}

const PDFStructureTreeAttribute* PDFStructureElement::findAttributeImpl(Attribute attribute,
                                                                        AttributeOwner owner,
                                                                        RevisionPolicy policy,
                                                                        const PDFStructureTreeAttributeDefinition* definition) const
{
    // We do not search for user properties
    if (attribute == Attribute::User)
    {
        return nullptr;
    }

    // Try to search for attribute in attribute list
    for (const PDFStructureTreeAttribute& attributeObject : m_attributes)
    {
        if ((attributeObject.getType() == attribute) &&
            (attributeObject.getOwner() == owner || owner == AttributeOwner::Invalid) &&
            (attributeObject.getRevision() == m_revision || policy == RevisionPolicy::Ignore))
        {
            return &attributeObject;
        }
    }

    // Check, if attribute is inheritable and then search for it in parent
    if (definition->inheritable && m_parent && m_parent->asStructureElement())
    {
        return m_parent->asStructureElement()->findAttributeImpl(attribute, owner, policy, definition);
    }

    return nullptr;
}

PDFStructureItemPointer PDFStructureElement::parseElement(const PDFObjectStorage* storage,
                                                          PDFObject object,
                                                          PDFMarkedObjectsContext* context,
                                                          PDFStructureItem* parent,
                                                          PDFStructureTree* root)
{
    PDFStructureItemPointer pointer;

    Q_ASSERT(root);

    if (auto lock = PDFMarkedObjectsLock(context, object))
    {
        if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
        {
            PDFStructureElement* item = new PDFStructureElement(parent, root);
            pointer.reset(item);

            if (object.isReference())
            {
                item->m_selfReference = object.getReference();
            }

            PDFDocumentDataLoaderDecorator loader(storage);
            item->m_typeName = loader.readNameFromDictionary(dictionary, "S");
            item->m_standardType = root->getTypeFromRole(item->m_typeName);
            item->m_id = loader.readStringFromDictionary(dictionary, "ID");
            item->m_references = loader.readReferenceArrayFromDictionary(dictionary, "Ref");
            item->m_pageReference = loader.readReferenceFromDictionary(dictionary, "Pg");

            std::vector<PDFStructureTreeAttribute> attributes;
            PDFObject classObject = storage->getObject(dictionary->get("C"));
            if (classObject.isName())
            {
                QByteArray name = classObject.getString();
                const std::vector<PDFStructureTreeAttribute>& classAttributes = root->getClassAttributes(name);
                attributes.insert(attributes.end(), classAttributes.begin(), classAttributes.end());
            }
            else if (classObject.isArray())
            {
                size_t startIndex = attributes.size();

                for (PDFObject itemObject : *classObject.getArray())
                {
                    itemObject = storage->getObject(itemObject);
                    if (itemObject.isInt())
                    {
                        // It is revision number
                        const PDFInteger revision = itemObject.getInteger();
                        for (; startIndex < attributes.size(); ++startIndex)
                        {
                            attributes[startIndex].setRevision(revision);
                        }
                    }
                    else if (itemObject.isName())
                    {
                        // It is class name
                        QByteArray name = itemObject.getString();
                        const std::vector<PDFStructureTreeAttribute>& classAttributes = root->getClassAttributes(name);
                        attributes.insert(attributes.end(), classAttributes.begin(), classAttributes.end());
                    }
                }
            }
            PDFStructureTreeAttribute::parseAttributes(storage, dictionary->get("A"), attributes);
            std::reverse(attributes.begin(), attributes.end());
            item->m_attributes = qMove(attributes);
            item->m_revision = loader.readIntegerFromDictionary(dictionary, "R", 0);
            item->m_texts[Title] = loader.readTextStringFromDictionary(dictionary, "T", QString());
            item->m_texts[Language] = loader.readTextStringFromDictionary(dictionary, "Lang", QString());
            item->m_texts[AlternativeDescription] = loader.readTextStringFromDictionary(dictionary, "Alt", QString());
            item->m_texts[ExpandedForm] = loader.readTextStringFromDictionary(dictionary, "E", QString());
            item->m_texts[ActualText] = loader.readTextStringFromDictionary(dictionary, "ActualText", QString());
            item->m_texts[Phoneme] = loader.readTextStringFromDictionary(dictionary, "Phoneme", QString());

            item->m_associatedFiles = loader.readObjectList<PDFFileSpecification>(dictionary->get("AF"));
            item->m_namespace = loader.readReferenceFromDictionary(dictionary, "NS");
            item->m_phoneticAlphabet = loader.readNameFromDictionary(dictionary, "PhoneticAlphabet");

            parseKids(storage, item, dictionary, context);
        }
    }

    return pointer;
}

PDFStructureItemPointer PDFStructureMarkedContentReference::parseMarkedContentReference(const PDFObjectStorage* storage,
                                                                                        PDFObject object,
                                                                                        PDFMarkedObjectsContext* context,
                                                                                        PDFStructureItem* parent,
                                                                                        PDFStructureTree* root)
{
    PDFStructureItemPointer pointer;

    Q_ASSERT(root);

    if (auto lock = PDFMarkedObjectsLock(context, object))
    {
        if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
        {
            PDFStructureMarkedContentReference* item = new PDFStructureMarkedContentReference(parent, root);
            pointer.reset(item);

            if (object.isReference())
            {
                item->m_selfReference = object.getReference();
            }

            PDFDocumentDataLoaderDecorator loader(storage);
            item->m_pageReference = loader.readReferenceFromDictionary(dictionary, "Pg");
            item->m_contentStreamReference = loader.readReferenceFromDictionary(dictionary, "Stm");
            item->m_contentStreamOwnerReference = loader.readReferenceFromDictionary(dictionary, "StmOwn");
            item->m_markedContentIdentifier = loader.readIntegerFromDictionary(dictionary, "MCID", 0);
        }
    }

    return pointer;
}

PDFStructureItemPointer PDFStructureObjectReference::parseObjectReference(const PDFObjectStorage* storage,
                                                                          PDFObject object,
                                                                          PDFMarkedObjectsContext* context,
                                                                          PDFStructureItem* parent,
                                                                          PDFStructureTree* root)
{
    PDFStructureItemPointer pointer;

    Q_ASSERT(root);

    if (auto lock = PDFMarkedObjectsLock(context, object))
    {
        if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
        {
            PDFStructureObjectReference* item = new PDFStructureObjectReference(parent, root);
            pointer.reset(item);

            if (object.isReference())
            {
                item->m_selfReference = object.getReference();
            }

            PDFDocumentDataLoaderDecorator loader(storage);
            item->m_pageReference = loader.readReferenceFromDictionary(dictionary, "Pg");
            item->m_objectReference = loader.readReferenceFromDictionary(dictionary, "Obj");
        }
    }

    return pointer;
}

void PDFStructureTreeAbstractVisitor::visitStructureTree(const PDFStructureTree* structureTree)
{
    acceptChildren(structureTree);
}

void PDFStructureTreeAbstractVisitor::visitStructureElement(const PDFStructureElement* structureElement)
{
    acceptChildren(structureElement);
}

void PDFStructureTreeAbstractVisitor::visitStructureMarkedContentReference(const PDFStructureMarkedContentReference* structureMarkedContentReference)
{
    acceptChildren(structureMarkedContentReference);
}

void PDFStructureTreeAbstractVisitor::visitStructureObjectReference(const PDFStructureObjectReference* structureObjectReference)
{
    acceptChildren(structureObjectReference);
}

void PDFStructureTreeAbstractVisitor::acceptChildren(const PDFStructureItem* item)
{
    const size_t childCount = item->getChildCount();
    for (size_t i = 0; i < childCount; ++i)
    {
        item->getChild(i)->accept(this);
    }
}

}   // namespace pdf
