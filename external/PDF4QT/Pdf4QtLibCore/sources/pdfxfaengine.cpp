//    Copyright (C) 2021-2022 Jakub Melka
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

#include "pdfxfaengine.h"
#include "pdfform.h"
#include "pdfpainterutils.h"
#include "pdffont.h"

#include <QBuffer>
#include <QDomElement>
#include <QDomDocument>
#include <QStringList>
#include <QDateTime>
#include <QImageReader>
#include <QTextDocument>
#include <QTextBlock>
#include <QFontDatabase>
#include <QAbstractTextDocumentLayout>
#include <QtMath>

#include "pdfdbgheap.h"

#include <stack>
#include <optional>

namespace pdf
{

namespace xfa
{
struct XFA_InplaceTag;
struct XFA_SharedMemoryTag;

class XFA_AbstractVisitor;

template<typename Value, typename Tag>
class PDFXFAValueHolder
{
public:
    inline constexpr PDFXFAValueHolder() = default;

    constexpr inline bool hasValue() const { return false; }
    constexpr const Value* getValue() const { return nullptr; }
};

template<typename Value>
class PDFXFAValueHolder<Value, XFA_InplaceTag>
{
public:
    inline constexpr PDFXFAValueHolder() = default;

    inline constexpr PDFXFAValueHolder(std::optional<Value> value) :
        m_value(std::move(value))
    {

    }

    constexpr inline bool hasValue() const { return m_value.has_value(); }
    constexpr const Value* getValue() const { return m_value.has_value() ? &m_value.value() : nullptr; }
    constexpr Value getValueOrDefault() const { return m_value.has_value() ? m_value.value() : Value(); }

private:
    std::optional<Value> m_value;
};

template<typename Value>
class PDFXFAValueHolder<Value, XFA_SharedMemoryTag>
{
public:
    inline constexpr PDFXFAValueHolder() = default;

    inline constexpr PDFXFAValueHolder(std::optional<Value> value) :
        m_value()
    {
        if (value)
        {
            m_value = std::make_shared<Value>(std::move(*value));
        }
    }

    constexpr inline bool hasValue() const { return static_cast<bool>(m_value); }
    constexpr const Value* getValue() const { return m_value.get(); }

private:
    std::shared_ptr<Value> m_value;
};

class XFA_ParagraphSettings
{
public:
    XFA_ParagraphSettings()
    {
        m_font.setFamily("Courier");
        m_font.setPixelSize(10);
        m_font.setHintingPreference(QFont::PreferNoHinting);
    }

    bool operator ==(const XFA_ParagraphSettings&) const = default;
    bool operator !=(const XFA_ParagraphSettings&) const = default;

    PDFReal getFontEmSize() const;
    void setFontEmSize(const PDFReal& fontEmSize);

    PDFReal getFontSpaceSize() const;
    void setFontSpaceSize(const PDFReal& fontSpaceSize);

    Qt::Alignment getAlignment() const;
    void setAlignment(const Qt::Alignment& alignment);

    PDFReal getLineHeight() const;
    void setLineHeight(const PDFReal& lineHeight);

    QMarginsF getMargins() const;
    void setMargins(const QMarginsF& margins);

    PDFInteger getOrphans() const;
    void setOrphans(const PDFInteger& orphans);

    PDFReal getRadixOffset() const;
    void setRadixOffset(const PDFReal& radixOffset);

    PDFReal getTextIndent() const;
    void setTextIndent(const PDFReal& textIndent);

    PDFInteger getWidows() const;
    void setWidows(const PDFInteger& widows);

    QString getTabDefault() const;
    void setTabDefault(const QString& tabDefault);

    QString getTabStops() const;
    void setTabStops(const QString& tabStops);

    QFont getFont() const;
    void setFont(const QFont& font);

private:
    PDFReal m_lineHeight = 0.0;
    PDFReal m_fontEmSize = 0.0;
    PDFReal m_fontSpaceSize = 0.0;
    PDFReal m_radixOffset = 0.0;
    PDFReal m_textIndent = 0.0;
    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignTop;
    QMarginsF m_margins = QMarginsF();
    PDFInteger m_orphans = 0;
    PDFInteger m_widows = 0;
    QString m_tabDefault;
    QString m_tabStops;
    QFont m_font;
};

template<typename Value>
using XFA_Attribute = PDFXFAValueHolder<Value, XFA_InplaceTag>;

template<typename Value>
using XFA_Node = PDFXFAValueHolder<Value, XFA_SharedMemoryTag>;

template<typename Value>
using XFA_Value = PDFXFAValueHolder<Value, XFA_InplaceTag>;

class XFA_Measurement
{
public:
    enum Type
    {
        in,
        cm,
        mm,
        pt,
        em,
        percent
    };

    constexpr inline XFA_Measurement() :
        m_value(0.0),
        m_type(in)
    {

    }

    constexpr inline XFA_Measurement(PDFReal value, Type type) :
        m_value(value),
        m_type(type)
    {

    }

    constexpr inline XFA_Measurement(PDFReal value) :
        m_value(value),
        m_type(in)
    {

    }

    constexpr PDFReal getValue() const { return m_value; }
    constexpr Type getType() const { return m_type; }

    PDFReal getValuePt(const XFA_ParagraphSettings* paragraphSettings) const;

    static bool parseMeasurement(QString measurementText, XFA_Measurement& measurement)
    {
        XFA_Measurement::Type measurementType = XFA_Measurement::Type::in;

        constexpr std::array measurementUnits = {
            std::make_pair(XFA_Measurement::Type::in, "in"),
            std::make_pair(XFA_Measurement::Type::pt, "pt"),
            std::make_pair(XFA_Measurement::Type::cm, "cm"),
            std::make_pair(XFA_Measurement::Type::mm, "mm"),
            std::make_pair(XFA_Measurement::Type::em, "em"),
            std::make_pair(XFA_Measurement::Type::percent, "%")
        };

        for (const auto& measurementUnit : measurementUnits)
        {
            QLatin1String unit(measurementUnit.second);
            if (measurementText.endsWith(unit))
            {
                measurementType = measurementUnit.first;
                measurementText.chop(unit.size());
                break;
            }
        }

        bool ok = false;
        PDFReal value = measurementText.toDouble(&ok);

        if (ok)
        {
            measurement = XFA_Measurement(value, measurementType);
        }

        return ok;
    }

private:
    PDFReal m_value;
    Type m_type;
};

class XFA_AbstractNode
{
public:
    constexpr inline XFA_AbstractNode() = default;
    virtual ~XFA_AbstractNode() = default;

    virtual void accept(XFA_AbstractVisitor* visitor) const = 0;

    template<typename Type>
    static void parseItem(const QDomElement& element, QString value, XFA_Node<Type>& node)
    {
        // Jakub Melka: set node value to null
        node = XFA_Node<Type>();

        QDomElement child = element.firstChildElement(value);
        if (!child.isNull())
        {
            node = XFA_Node<Type>(Type::parse(child));
        }
    }

    template<typename Type>
    static void parseItem(const QDomElement& element, QString value, std::vector<XFA_Node<Type>>& nodes)
    {
        // Jakub Melka: clear node list
        nodes.clear();

        QDomElement child = element.firstChildElement(value);
        while (!child.isNull())
        {
            nodes.emplace_back(XFA_Node<Type>(Type::parse(child)));
            child = child.nextSiblingElement(value);
        }
    }

    template<typename Enum>
    static void parseEnumAttribute(const QDomElement& element,
                                   QString attributeFieldName,
                                   XFA_Attribute<Enum>& attribute,
                                   QString defaultValue,
                                   const auto& enumValues)
    {
        attribute = XFA_Attribute<Enum>();
        QString value = element.attribute(attributeFieldName, defaultValue);

        for (const auto& enumValue : enumValues)
        {
            if (enumValue.second == value)
            {
                attribute = XFA_Attribute<Enum>(enumValue.first);
                break;
            }
        }
    }

    static void parseAttribute(const QDomElement& element,
                               QString attributeFieldName,
                               XFA_Attribute<QString>& attribute,
                               QString defaultValue)
    {
        attribute = XFA_Attribute<QString>(element.attribute(attributeFieldName, defaultValue));
    }

    static void parseAttribute(const QDomElement& element,
                               QString attributeFieldName,
                               XFA_Attribute<bool>& attribute,
                               QString defaultValue)
    {
        attribute = XFA_Attribute<bool>(element.attribute(attributeFieldName, defaultValue).toInt() != 0);
    }

    static void parseAttribute(const QDomElement& element,
                               QString attributeFieldName,
                               XFA_Attribute<PDFReal>& attribute,
                               QString defaultValue)
    {
        attribute = XFA_Attribute<PDFReal>(element.attribute(attributeFieldName, defaultValue).toDouble());
    }

    static void parseAttribute(const QDomElement& element,
                               QString attributeFieldName,
                               XFA_Attribute<PDFInteger>& attribute,
                               QString defaultValue)
    {
        attribute = XFA_Attribute<PDFInteger>(element.attribute(attributeFieldName, defaultValue).toInt());
    }



    static void parseAttribute(const QDomElement& element,
                               QString attributeFieldName,
                               XFA_Attribute<XFA_Measurement>& attribute,
                               QString defaultValue)
    {
        attribute = XFA_Attribute<XFA_Measurement>();

        QString measurement = element.attribute(attributeFieldName, defaultValue);

        XFA_Measurement value;
        if (XFA_Measurement::parseMeasurement(measurement, value))
        {
            attribute = XFA_Attribute<XFA_Measurement>(std::move(value));
        }
    }

    static void parseValue(const QDomElement& element, XFA_Value<QString>& nodeValue)
    {
        nodeValue = XFA_Value<QString>();

        QString text;

        if (element.hasChildNodes())
        {
            QTextStream textStream(&text);
            QDomNode node = element.firstChild();
            while (!node.isNull())
            {
                textStream << node;
                node = node.nextSibling();
            }
        }

        if (!text.isEmpty())
        {
            nodeValue = XFA_Value<QString>(std::move(text));
        }
    }

    /// Get ordering of the element in original document
    size_t getOrder() const { return m_order; }

    void setOrderFromElement(const QDomElement& element);

    static inline constexpr void addNodesToContainer(std::vector<const XFA_AbstractNode*>&) { }

    template<typename Container, typename... Containers>
    static void addNodesToContainer(std::vector<const XFA_AbstractNode*>& nodes,
                                    const Container& container,
                                    const Containers&... containers)
    {
        if constexpr (std::is_convertible<Container, const xfa::XFA_AbstractNode*>::value)
        {
            if (container)
            {
                nodes.push_back(container);
            }
        }
        else
        {
            for (const auto& node : container)
            {
                nodes.push_back(node.getValue());
            }
        }

        addNodesToContainer(nodes, containers...);
    }

    template<typename... Containers>
    static void acceptOrdered(XFA_AbstractVisitor* visitor, const Containers&... containers)
    {
        std::vector<const XFA_AbstractNode*> nodes;
        addNodesToContainer(nodes, containers...);
        std::sort(nodes.begin(), nodes.end(), [](const auto* l, const auto* r) { return l->getOrder() < r->getOrder(); });

        for (const XFA_AbstractNode* node : nodes)
        {
            node->accept(visitor);
        }
    }

private:
    size_t m_order = 0;
};

void XFA_AbstractNode::setOrderFromElement(const QDomElement& element)
{
    const uint32_t lineNumber = element.lineNumber();
    const uint32_t columnNumber = element.columnNumber();

    if constexpr (sizeof(decltype(m_order)) > 4)
    {
        m_order = (size_t(lineNumber) << 32) + columnNumber;
    }
    else
    {
        m_order = (size_t(lineNumber) << 12) + columnNumber;
    }
}

PDFReal XFA_ParagraphSettings::getFontEmSize() const
{
    return m_fontEmSize;
}

void XFA_ParagraphSettings::setFontEmSize(const PDFReal& fontEmSize)
{
    m_fontEmSize = fontEmSize;
}

PDFReal XFA_ParagraphSettings::getFontSpaceSize() const
{
    return m_fontSpaceSize;
}

void XFA_ParagraphSettings::setFontSpaceSize(const PDFReal& fontSpaceSize)
{
    m_fontSpaceSize = fontSpaceSize;
}

Qt::Alignment XFA_ParagraphSettings::getAlignment() const
{
    return m_alignment;
}

void XFA_ParagraphSettings::setAlignment(const Qt::Alignment& alignment)
{
    m_alignment = alignment;
}

PDFReal XFA_ParagraphSettings::getLineHeight() const
{
    return m_lineHeight;
}

void XFA_ParagraphSettings::setLineHeight(const PDFReal& lineHeight)
{
    m_lineHeight = lineHeight;
}

QMarginsF XFA_ParagraphSettings::getMargins() const
{
    return m_margins;
}

void XFA_ParagraphSettings::setMargins(const QMarginsF& margins)
{
    m_margins = margins;
}

PDFInteger XFA_ParagraphSettings::getOrphans() const
{
    return m_orphans;
}

void XFA_ParagraphSettings::setOrphans(const PDFInteger& orphans)
{
    m_orphans = orphans;
}

PDFReal XFA_ParagraphSettings::getRadixOffset() const
{
    return m_radixOffset;
}

void XFA_ParagraphSettings::setRadixOffset(const PDFReal& radixOffset)
{
    m_radixOffset = radixOffset;
}

PDFReal XFA_ParagraphSettings::getTextIndent() const
{
    return m_textIndent;
}

void XFA_ParagraphSettings::setTextIndent(const PDFReal& textIndent)
{
    m_textIndent = textIndent;
}

PDFInteger XFA_ParagraphSettings::getWidows() const
{
    return m_widows;
}

void XFA_ParagraphSettings::setWidows(const PDFInteger& widows)
{
    m_widows = widows;
}

QString XFA_ParagraphSettings::getTabDefault() const
{
    return m_tabDefault;
}

void XFA_ParagraphSettings::setTabDefault(const QString& tabDefault)
{
    m_tabDefault = tabDefault;
}

QString XFA_ParagraphSettings::getTabStops() const
{
    return m_tabStops;
}

void XFA_ParagraphSettings::setTabStops(const QString& tabStops)
{
    m_tabStops = tabStops;
}

QFont XFA_ParagraphSettings::getFont() const
{
    return m_font;
}

void XFA_ParagraphSettings::setFont(const QFont& font)
{
    m_font = font;
}

PDFReal XFA_Measurement::getValuePt(const XFA_ParagraphSettings* paragraphSettings) const
{
    switch (m_type)
    {
        case pdf::xfa::XFA_Measurement::in:
            return m_value * 72.0;

        case pdf::xfa::XFA_Measurement::cm:
            return m_value / 2.54 * 72.0;

        case pdf::xfa::XFA_Measurement::mm:
            return m_value / 25.4 * 72.0;

        case pdf::xfa::XFA_Measurement::pt:
            return m_value;

        case pdf::xfa::XFA_Measurement::em:
            return paragraphSettings ? m_value * paragraphSettings->getFontEmSize() : 0.0;

        case pdf::xfa::XFA_Measurement::percent:
            return paragraphSettings ? m_value * paragraphSettings->getFontSpaceSize() : 0.0;

        default:
            Q_ASSERT(false);
            break;
    }

    return 0.0;
}

}   // namespace xfa

/* START GENERATED CODE */

namespace xfa
{

class XFA_appearanceFilter;
class XFA_bindItems;
class XFA_bookend;
class XFA_boolean;
class XFA_certificate;
class XFA_comb;
class XFA_date;
class XFA_dateTime;
class XFA_decimal;
class XFA_digestMethod;
class XFA_digestMethods;
class XFA_encoding;
class XFA_encodings;
class XFA_encrypt;
class XFA_encryption;
class XFA_encryptionMethod;
class XFA_encryptionMethods;
class XFA_exData;
class XFA_execute;
class XFA_float;
class XFA_handler;
class XFA_hyphenation;
class XFA_image;
class XFA_integer;
class XFA_issuers;
class XFA_keyUsage;
class XFA_lockDocument;
class XFA_mdp;
class XFA_medium;
class XFA_oid;
class XFA_oids;
class XFA_overflow;
class XFA_para;
class XFA_picture;
class XFA_bind;
class XFA_connect;
class XFA_reason;
class XFA_reasons;
class XFA_ref;
class XFA_script;
class XFA_breakAfter;
class XFA_breakBefore;
class XFA_setProperty;
class XFA_signing;
class XFA_speak;
class XFA_subjectDN;
class XFA_subjectDNs;
class XFA_certificates;
class XFA_text;
class XFA_message;
class XFA_time;
class XFA_desc;
class XFA_extras;
class XFA_barcode;
class XFA_break;
class XFA_button;
class XFA_calculate;
class XFA_color;
class XFA_contentArea;
class XFA_corner;
class XFA_defaultUi;
class XFA_edge;
class XFA_exObject;
class XFA_format;
class XFA_items;
class XFA_keep;
class XFA_line;
class XFA_linear;
class XFA_manifest;
class XFA_margin;
class XFA_occur;
class XFA_pattern;
class XFA_radial;
class XFA_solid;
class XFA_stipple;
class XFA_fill;
class XFA_arc;
class XFA_border;
class XFA_checkButton;
class XFA_choiceList;
class XFA_dateTimeEdit;
class XFA_font;
class XFA_imageEdit;
class XFA_numericEdit;
class XFA_passwordEdit;
class XFA_rectangle;
class XFA_textEdit;
class XFA_timeStamp;
class XFA_filter;
class XFA_encryptData;
class XFA_signData;
class XFA_signature;
class XFA_submit;
class XFA_event;
class XFA_toolTip;
class XFA_assist;
class XFA_traverse;
class XFA_traversal;
class XFA_ui;
class XFA_validate;
class XFA_value;
class XFA_caption;
class XFA_draw;
class XFA_field;
class XFA_exclGroup;
class XFA_variables;
class XFA_area;
class XFA_pageArea;
class XFA_pageSet;
class XFA_proto;
class XFA_subform;
class XFA_subformSet;
class XFA_template;

class XFA_AbstractVisitor
{
public:
    XFA_AbstractVisitor() = default;
    virtual ~XFA_AbstractVisitor() = default;

    virtual void visit(const XFA_appearanceFilter* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_bindItems* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_bookend* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_boolean* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_certificate* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_comb* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_date* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_dateTime* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_decimal* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_digestMethod* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_digestMethods* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_encoding* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_encodings* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_encrypt* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_encryption* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_encryptionMethod* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_encryptionMethods* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_exData* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_execute* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_float* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_handler* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_hyphenation* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_image* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_integer* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_issuers* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_keyUsage* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_lockDocument* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_mdp* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_medium* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_oid* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_oids* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_overflow* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_para* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_picture* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_bind* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_connect* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_reason* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_reasons* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_ref* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_script* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_breakAfter* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_breakBefore* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_setProperty* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_signing* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_speak* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_subjectDN* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_subjectDNs* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_certificates* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_text* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_message* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_time* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_desc* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_extras* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_barcode* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_break* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_button* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_calculate* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_color* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_contentArea* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_corner* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_defaultUi* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_edge* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_exObject* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_format* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_items* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_keep* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_line* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_linear* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_manifest* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_margin* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_occur* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_pattern* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_radial* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_solid* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_stipple* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_fill* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_arc* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_border* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_checkButton* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_choiceList* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_dateTimeEdit* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_font* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_imageEdit* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_numericEdit* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_passwordEdit* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_rectangle* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_textEdit* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_timeStamp* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_filter* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_encryptData* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_signData* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_signature* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_submit* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_event* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_toolTip* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_assist* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_traverse* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_traversal* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_ui* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_validate* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_value* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_caption* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_draw* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_field* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_exclGroup* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_variables* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_area* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_pageArea* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_pageSet* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_proto* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_subform* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_subformSet* node) { Q_UNUSED(node); }
    virtual void visit(const XFA_template* node) { Q_UNUSED(node); }
};

class XFA_BaseNode : public XFA_AbstractNode
{
public:
    using XFA_AbstractNode::parseAttribute;

    enum class ACCESS
    {
        Open,
        NonInteractive,
        Protected,
        ReadOnly,
    };

    enum class ACTION
    {
        Include,
        All,
        Exclude,
    };

    enum class ACTIVITY
    {
        Click,
        Change,
        DocClose,
        DocReady,
        Enter,
        Exit,
        Full,
        IndexChange,
        Initialize,
        MouseDown,
        MouseEnter,
        MouseExit,
        MouseUp,
        PostExecute,
        PostOpen,
        PostPrint,
        PostSave,
        PostSign,
        PostSubmit,
        PreExecute,
        PreOpen,
        PrePrint,
        PreSave,
        PreSign,
        PreSubmit,
        Ready,
        ValidationState,
    };

    enum class AFTER
    {
        Auto,
        ContentArea,
        PageArea,
        PageEven,
        PageOdd,
    };

    enum class ANCHORTYPE
    {
        TopLeft,
        BottomCenter,
        BottomLeft,
        BottomRight,
        MiddleCenter,
        MiddleLeft,
        MiddleRight,
        TopCenter,
        TopRight,
    };

    enum class ASPECT
    {
        Fit,
        Actual,
        Height,
        None,
        Width,
    };

    enum class BASEPROFILE
    {
        Full,
        InteractiveForms,
    };

    enum class BEFORE
    {
        Auto,
        ContentArea,
        PageArea,
        PageEven,
        PageOdd,
    };

    enum class BLANKORNOTBLANK
    {
        Any,
        Blank,
        NotBlank,
    };

    enum class BREAK
    {
        Close,
        Open,
    };

    enum class CAP
    {
        Square,
        Butt,
        Round,
    };

    enum class CHECKSUM
    {
        None,
        _1mod10,
        _1mod10_1mod11,
        _2mod10,
        Auto,
    };

    enum class COMMITON
    {
        Select,
        Exit,
    };

    enum class CREDENTIALSERVERPOLICY
    {
        Optional,
        Required,
    };

    enum class DATAPREP
    {
        None,
        FlateCompress,
    };

    enum class DATA
    {
        Link,
        Embed,
    };

    enum class DUPLEXIMPOSITION
    {
        LongEdge,
        ShortEdge,
    };

    enum class EXECUTETYPE
    {
        Import,
        Remerge,
    };

    enum class FORMATTEST
    {
        Warning,
        Disabled,
        Error,
    };

    enum class FORMAT
    {
        Xdp,
        Formdata,
        Pdf,
        Urlencoded,
        Xfd,
        Xml,
    };

    enum class HALIGN
    {
        Left,
        Center,
        Justify,
        JustifyAll,
        Radix,
        Right,
    };

    enum class HSCROLLPOLICY
    {
        Auto,
        Off,
        On,
    };

    enum class HAND
    {
        Even,
        Left,
        Right,
    };

    enum class HIGHLIGHT
    {
        Inverted,
        None,
        Outline,
        Push,
    };

    enum class INTACT
    {
        None,
        ContentArea,
        PageArea,
    };

    enum class JOIN
    {
        Square,
        Round,
    };

    enum class KERNINGMODE
    {
        None,
        Pair,
    };

    enum class LAYOUT
    {
        Position,
        Lr_tb,
        Rl_row,
        Rl_tb,
        Row,
        Table,
        Tb,
    };

    enum class LINETHROUGHPERIOD
    {
        All,
        Word,
    };

    enum class LINETHROUGH
    {
        _0,
        _1,
        _2,
    };

    enum class LISTEN
    {
        RefOnly,
        RefAndDescendents,
    };

    enum class MARK
    {
        Default,
        Check,
        Circle,
        Cross,
        Diamond,
        Square,
        Star,
    };

    enum class MATCH
    {
        Once,
        DataRef,
        Global,
        None,
    };

    enum class MERGEMODE
    {
        ConsumeData,
        MatchTemplate,
    };

    enum class MULTILINE
    {
        _1,
        _0,
    };

    enum class NEXT
    {
        None,
        ContentArea,
        PageArea,
    };

    enum class NULLTEST
    {
        Disabled,
        Error,
        Warning,
    };

    enum class ODDOREVEN
    {
        Any,
        Even,
        Odd,
    };

    enum class OPEN
    {
        UserControl,
        Always,
        MultiSelect,
        OnEntry,
    };

    enum class OPERATION
    {
        Encrypt,
        Decrypt,
    };

    enum class OPERATION2
    {
        Next,
        Back,
        Down,
        First,
        Left,
        Right,
        Up,
    };

    enum class OPERATION1
    {
        Sign,
        Clear,
        Verify,
    };

    enum class ORIENTATION
    {
        Portrait,
        Landscape,
    };

    enum class OVERLINEPERIOD
    {
        All,
        Word,
    };

    enum class OVERLINE
    {
        _0,
        _1,
        _2,
    };

    enum class OVERRIDE
    {
        Disabled,
        Error,
        Ignore,
        Warning,
    };

    enum class PAGEPOSITION
    {
        Any,
        First,
        Last,
        Only,
        Rest,
    };

    enum class PERMISSIONS
    {
        _2,
        _1,
        _3,
    };

    enum class PICKER
    {
        Host,
        None,
    };

    enum class PLACEMENT
    {
        Left,
        Bottom,
        Inline,
        Right,
        Top,
    };

    enum class POSTURE
    {
        Normal,
        Italic,
    };

    enum class PRESENCE
    {
        Visible,
        Hidden,
        Inactive,
        Invisible,
    };

    enum class PREVIOUS
    {
        None,
        ContentArea,
        PageArea,
    };

    enum class PRIORITY
    {
        Custom,
        Caption,
        Name,
        ToolTip,
    };

    enum class RELATION1
    {
        Ordered,
        Choice,
        Unordered,
    };

    enum class RELATION
    {
        OrderedOccurrence,
        DuplexPaginated,
        SimplexPaginated,
    };

    enum class RESTORESTATE
    {
        Manual,
        Auto,
    };

    enum class RUNAT
    {
        Client,
        Both,
        Server,
    };

    enum class SCOPE
    {
        Name,
        None,
    };

    enum class SCRIPTTEST
    {
        Error,
        Disabled,
        Warning,
    };

    enum class SHAPE
    {
        Square,
        Round,
    };

    enum class SIGNATURETYPE
    {
        Filler,
        Author,
    };

    enum class SLOPE
    {
        Backslash,
        Slash,
    };

    enum class STROKE
    {
        Solid,
        DashDot,
        DashDotDot,
        Dashed,
        Dotted,
        Embossed,
        Etched,
        Lowered,
        Raised,
    };

    enum class TARGETTYPE
    {
        Auto,
        ContentArea,
        PageArea,
        PageEven,
        PageOdd,
    };

    enum class TEXTLOCATION
    {
        Below,
        Above,
        AboveEmbedded,
        BelowEmbedded,
        None,
    };

    enum class TRANSFERENCODING1
    {
        Base64,
        None,
        Package,
    };

    enum class TRANSFERENCODING
    {
        None,
        Base64,
        Package,
    };

    enum class TRAYIN
    {
        Auto,
        Delegate,
        PageFront,
    };

    enum class TRAYOUT
    {
        Auto,
        Delegate,
    };

    enum class TYPE4
    {
        PDF1_3,
        PDF1_6,
    };

    enum class TYPE2
    {
        CrossHatch,
        CrossDiagonal,
        DiagonalLeft,
        DiagonalRight,
        Horizontal,
        Vertical,
    };

    enum class TYPE
    {
        Optional,
        Required,
    };

    enum class TYPE3
    {
        ToEdge,
        ToCenter,
    };

    enum class TYPE1
    {
        ToRight,
        ToBottom,
        ToLeft,
        ToTop,
    };

    enum class UNDERLINEPERIOD
    {
        All,
        Word,
    };

    enum class UNDERLINE
    {
        _0,
        _1,
        _2,
    };

    enum class UPSMODE
    {
        UsCarrier,
        InternationalCarrier,
        SecureSymbol,
        StandardSymbol,
    };

    enum class USAGE
    {
        ExportAndImport,
        ExportOnly,
        ImportOnly,
    };

    enum class VALIGN
    {
        Top,
        Bottom,
        Middle,
    };

    enum class VSCROLLPOLICY
    {
        Auto,
        Off,
        On,
    };

    enum class WEIGHT
    {
        Normal,
        Bold,
    };

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<ACCESS>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(ACCESS::Open, "open"),
            std::make_pair(ACCESS::NonInteractive, "nonInteractive"),
            std::make_pair(ACCESS::Protected, "protected"),
            std::make_pair(ACCESS::ReadOnly, "readOnly"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<ACTION>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(ACTION::Include, "include"),
            std::make_pair(ACTION::All, "all"),
            std::make_pair(ACTION::Exclude, "exclude"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<ACTIVITY>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(ACTIVITY::Click, "click"),
            std::make_pair(ACTIVITY::Change, "change"),
            std::make_pair(ACTIVITY::DocClose, "docClose"),
            std::make_pair(ACTIVITY::DocReady, "docReady"),
            std::make_pair(ACTIVITY::Enter, "enter"),
            std::make_pair(ACTIVITY::Exit, "exit"),
            std::make_pair(ACTIVITY::Full, "full"),
            std::make_pair(ACTIVITY::IndexChange, "indexChange"),
            std::make_pair(ACTIVITY::Initialize, "initialize"),
            std::make_pair(ACTIVITY::MouseDown, "mouseDown"),
            std::make_pair(ACTIVITY::MouseEnter, "mouseEnter"),
            std::make_pair(ACTIVITY::MouseExit, "mouseExit"),
            std::make_pair(ACTIVITY::MouseUp, "mouseUp"),
            std::make_pair(ACTIVITY::PostExecute, "postExecute"),
            std::make_pair(ACTIVITY::PostOpen, "postOpen"),
            std::make_pair(ACTIVITY::PostPrint, "postPrint"),
            std::make_pair(ACTIVITY::PostSave, "postSave"),
            std::make_pair(ACTIVITY::PostSign, "postSign"),
            std::make_pair(ACTIVITY::PostSubmit, "postSubmit"),
            std::make_pair(ACTIVITY::PreExecute, "preExecute"),
            std::make_pair(ACTIVITY::PreOpen, "preOpen"),
            std::make_pair(ACTIVITY::PrePrint, "prePrint"),
            std::make_pair(ACTIVITY::PreSave, "preSave"),
            std::make_pair(ACTIVITY::PreSign, "preSign"),
            std::make_pair(ACTIVITY::PreSubmit, "preSubmit"),
            std::make_pair(ACTIVITY::Ready, "ready"),
            std::make_pair(ACTIVITY::ValidationState, "validationState"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<AFTER>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(AFTER::Auto, "auto"),
            std::make_pair(AFTER::ContentArea, "contentArea"),
            std::make_pair(AFTER::PageArea, "pageArea"),
            std::make_pair(AFTER::PageEven, "pageEven"),
            std::make_pair(AFTER::PageOdd, "pageOdd"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<ANCHORTYPE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(ANCHORTYPE::TopLeft, "topLeft"),
            std::make_pair(ANCHORTYPE::BottomCenter, "bottomCenter"),
            std::make_pair(ANCHORTYPE::BottomLeft, "bottomLeft"),
            std::make_pair(ANCHORTYPE::BottomRight, "bottomRight"),
            std::make_pair(ANCHORTYPE::MiddleCenter, "middleCenter"),
            std::make_pair(ANCHORTYPE::MiddleLeft, "middleLeft"),
            std::make_pair(ANCHORTYPE::MiddleRight, "middleRight"),
            std::make_pair(ANCHORTYPE::TopCenter, "topCenter"),
            std::make_pair(ANCHORTYPE::TopRight, "topRight"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<ASPECT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(ASPECT::Fit, "fit"),
            std::make_pair(ASPECT::Actual, "actual"),
            std::make_pair(ASPECT::Height, "height"),
            std::make_pair(ASPECT::None, "none"),
            std::make_pair(ASPECT::Width, "width"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<BASEPROFILE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(BASEPROFILE::Full, "full"),
            std::make_pair(BASEPROFILE::InteractiveForms, "interactiveForms"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<BEFORE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(BEFORE::Auto, "auto"),
            std::make_pair(BEFORE::ContentArea, "contentArea"),
            std::make_pair(BEFORE::PageArea, "pageArea"),
            std::make_pair(BEFORE::PageEven, "pageEven"),
            std::make_pair(BEFORE::PageOdd, "pageOdd"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<BLANKORNOTBLANK>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(BLANKORNOTBLANK::Any, "any"),
            std::make_pair(BLANKORNOTBLANK::Blank, "blank"),
            std::make_pair(BLANKORNOTBLANK::NotBlank, "notBlank"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<BREAK>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(BREAK::Close, "close"),
            std::make_pair(BREAK::Open, "open"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<CAP>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(CAP::Square, "square"),
            std::make_pair(CAP::Butt, "butt"),
            std::make_pair(CAP::Round, "round"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<CHECKSUM>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(CHECKSUM::None, "none"),
            std::make_pair(CHECKSUM::_1mod10, "1mod10"),
            std::make_pair(CHECKSUM::_1mod10_1mod11, "1mod10_1mod11"),
            std::make_pair(CHECKSUM::_2mod10, "2mod10"),
            std::make_pair(CHECKSUM::Auto, "auto"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<COMMITON>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(COMMITON::Select, "select"),
            std::make_pair(COMMITON::Exit, "exit"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<CREDENTIALSERVERPOLICY>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(CREDENTIALSERVERPOLICY::Optional, "optional"),
            std::make_pair(CREDENTIALSERVERPOLICY::Required, "required"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<DATAPREP>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(DATAPREP::None, "none"),
            std::make_pair(DATAPREP::FlateCompress, "flateCompress"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<DATA>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(DATA::Link, "link"),
            std::make_pair(DATA::Embed, "embed"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<DUPLEXIMPOSITION>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(DUPLEXIMPOSITION::LongEdge, "longEdge"),
            std::make_pair(DUPLEXIMPOSITION::ShortEdge, "shortEdge"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<EXECUTETYPE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(EXECUTETYPE::Import, "import"),
            std::make_pair(EXECUTETYPE::Remerge, "remerge"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<FORMATTEST>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(FORMATTEST::Warning, "warning"),
            std::make_pair(FORMATTEST::Disabled, "disabled"),
            std::make_pair(FORMATTEST::Error, "error"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<FORMAT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(FORMAT::Xdp, "xdp"),
            std::make_pair(FORMAT::Formdata, "formdata"),
            std::make_pair(FORMAT::Pdf, "pdf"),
            std::make_pair(FORMAT::Urlencoded, "urlencoded"),
            std::make_pair(FORMAT::Xfd, "xfd"),
            std::make_pair(FORMAT::Xml, "xml"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<HALIGN>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(HALIGN::Left, "left"),
            std::make_pair(HALIGN::Center, "center"),
            std::make_pair(HALIGN::Justify, "justify"),
            std::make_pair(HALIGN::JustifyAll, "justifyAll"),
            std::make_pair(HALIGN::Radix, "radix"),
            std::make_pair(HALIGN::Right, "right"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<HSCROLLPOLICY>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(HSCROLLPOLICY::Auto, "auto"),
            std::make_pair(HSCROLLPOLICY::Off, "off"),
            std::make_pair(HSCROLLPOLICY::On, "on"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<HAND>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(HAND::Even, "even"),
            std::make_pair(HAND::Left, "left"),
            std::make_pair(HAND::Right, "right"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<HIGHLIGHT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(HIGHLIGHT::Inverted, "inverted"),
            std::make_pair(HIGHLIGHT::None, "none"),
            std::make_pair(HIGHLIGHT::Outline, "outline"),
            std::make_pair(HIGHLIGHT::Push, "push"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<INTACT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(INTACT::None, "none"),
            std::make_pair(INTACT::ContentArea, "contentArea"),
            std::make_pair(INTACT::PageArea, "pageArea"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<JOIN>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(JOIN::Square, "square"),
            std::make_pair(JOIN::Round, "round"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<KERNINGMODE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(KERNINGMODE::None, "none"),
            std::make_pair(KERNINGMODE::Pair, "pair"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<LAYOUT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(LAYOUT::Position, "position"),
            std::make_pair(LAYOUT::Lr_tb, "lr-tb"),
            std::make_pair(LAYOUT::Rl_row, "rl-row"),
            std::make_pair(LAYOUT::Rl_tb, "rl-tb"),
            std::make_pair(LAYOUT::Row, "row"),
            std::make_pair(LAYOUT::Table, "table"),
            std::make_pair(LAYOUT::Tb, "tb"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<LINETHROUGHPERIOD>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(LINETHROUGHPERIOD::All, "all"),
            std::make_pair(LINETHROUGHPERIOD::Word, "word"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<LINETHROUGH>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(LINETHROUGH::_0, "0"),
            std::make_pair(LINETHROUGH::_1, "1"),
            std::make_pair(LINETHROUGH::_2, "2"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<LISTEN>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(LISTEN::RefOnly, "refOnly"),
            std::make_pair(LISTEN::RefAndDescendents, "refAndDescendents"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<MARK>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(MARK::Default, "default"),
            std::make_pair(MARK::Check, "check"),
            std::make_pair(MARK::Circle, "circle"),
            std::make_pair(MARK::Cross, "cross"),
            std::make_pair(MARK::Diamond, "diamond"),
            std::make_pair(MARK::Square, "square"),
            std::make_pair(MARK::Star, "star"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<MATCH>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(MATCH::Once, "once"),
            std::make_pair(MATCH::DataRef, "dataRef"),
            std::make_pair(MATCH::Global, "global"),
            std::make_pair(MATCH::None, "none"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<MERGEMODE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(MERGEMODE::ConsumeData, "consumeData"),
            std::make_pair(MERGEMODE::MatchTemplate, "matchTemplate"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<MULTILINE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(MULTILINE::_1, "1"),
            std::make_pair(MULTILINE::_0, "0"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<NEXT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(NEXT::None, "none"),
            std::make_pair(NEXT::ContentArea, "contentArea"),
            std::make_pair(NEXT::PageArea, "pageArea"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<NULLTEST>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(NULLTEST::Disabled, "disabled"),
            std::make_pair(NULLTEST::Error, "error"),
            std::make_pair(NULLTEST::Warning, "warning"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<ODDOREVEN>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(ODDOREVEN::Any, "any"),
            std::make_pair(ODDOREVEN::Even, "even"),
            std::make_pair(ODDOREVEN::Odd, "odd"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<OPEN>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(OPEN::UserControl, "userControl"),
            std::make_pair(OPEN::Always, "always"),
            std::make_pair(OPEN::MultiSelect, "multiSelect"),
            std::make_pair(OPEN::OnEntry, "onEntry"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<OPERATION>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(OPERATION::Encrypt, "encrypt"),
            std::make_pair(OPERATION::Decrypt, "decrypt"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<OPERATION2>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(OPERATION2::Next, "next"),
            std::make_pair(OPERATION2::Back, "back"),
            std::make_pair(OPERATION2::Down, "down"),
            std::make_pair(OPERATION2::First, "first"),
            std::make_pair(OPERATION2::Left, "left"),
            std::make_pair(OPERATION2::Right, "right"),
            std::make_pair(OPERATION2::Up, "up"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<OPERATION1>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(OPERATION1::Sign, "sign"),
            std::make_pair(OPERATION1::Clear, "clear"),
            std::make_pair(OPERATION1::Verify, "verify"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<ORIENTATION>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(ORIENTATION::Portrait, "portrait"),
            std::make_pair(ORIENTATION::Landscape, "landscape"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<OVERLINEPERIOD>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(OVERLINEPERIOD::All, "all"),
            std::make_pair(OVERLINEPERIOD::Word, "word"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<OVERLINE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(OVERLINE::_0, "0"),
            std::make_pair(OVERLINE::_1, "1"),
            std::make_pair(OVERLINE::_2, "2"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<OVERRIDE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(OVERRIDE::Disabled, "disabled"),
            std::make_pair(OVERRIDE::Error, "error"),
            std::make_pair(OVERRIDE::Ignore, "ignore"),
            std::make_pair(OVERRIDE::Warning, "warning"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<PAGEPOSITION>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(PAGEPOSITION::Any, "any"),
            std::make_pair(PAGEPOSITION::First, "first"),
            std::make_pair(PAGEPOSITION::Last, "last"),
            std::make_pair(PAGEPOSITION::Only, "only"),
            std::make_pair(PAGEPOSITION::Rest, "rest"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<PERMISSIONS>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(PERMISSIONS::_2, "2"),
            std::make_pair(PERMISSIONS::_1, "1"),
            std::make_pair(PERMISSIONS::_3, "3"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<PICKER>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(PICKER::Host, "host"),
            std::make_pair(PICKER::None, "none"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<PLACEMENT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(PLACEMENT::Left, "left"),
            std::make_pair(PLACEMENT::Bottom, "bottom"),
            std::make_pair(PLACEMENT::Inline, "inline"),
            std::make_pair(PLACEMENT::Right, "right"),
            std::make_pair(PLACEMENT::Top, "top"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<POSTURE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(POSTURE::Normal, "normal"),
            std::make_pair(POSTURE::Italic, "italic"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<PRESENCE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(PRESENCE::Visible, "visible"),
            std::make_pair(PRESENCE::Hidden, "hidden"),
            std::make_pair(PRESENCE::Inactive, "inactive"),
            std::make_pair(PRESENCE::Invisible, "invisible"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<PREVIOUS>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(PREVIOUS::None, "none"),
            std::make_pair(PREVIOUS::ContentArea, "contentArea"),
            std::make_pair(PREVIOUS::PageArea, "pageArea"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<PRIORITY>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(PRIORITY::Custom, "custom"),
            std::make_pair(PRIORITY::Caption, "caption"),
            std::make_pair(PRIORITY::Name, "name"),
            std::make_pair(PRIORITY::ToolTip, "toolTip"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<RELATION1>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(RELATION1::Ordered, "ordered"),
            std::make_pair(RELATION1::Choice, "choice"),
            std::make_pair(RELATION1::Unordered, "unordered"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<RELATION>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(RELATION::OrderedOccurrence, "orderedOccurrence"),
            std::make_pair(RELATION::DuplexPaginated, "duplexPaginated"),
            std::make_pair(RELATION::SimplexPaginated, "simplexPaginated"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<RESTORESTATE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(RESTORESTATE::Manual, "manual"),
            std::make_pair(RESTORESTATE::Auto, "auto"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<RUNAT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(RUNAT::Client, "client"),
            std::make_pair(RUNAT::Both, "both"),
            std::make_pair(RUNAT::Server, "server"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<SCOPE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(SCOPE::Name, "name"),
            std::make_pair(SCOPE::None, "none"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<SCRIPTTEST>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(SCRIPTTEST::Error, "error"),
            std::make_pair(SCRIPTTEST::Disabled, "disabled"),
            std::make_pair(SCRIPTTEST::Warning, "warning"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<SHAPE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(SHAPE::Square, "square"),
            std::make_pair(SHAPE::Round, "round"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<SIGNATURETYPE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(SIGNATURETYPE::Filler, "filler"),
            std::make_pair(SIGNATURETYPE::Author, "author"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<SLOPE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(SLOPE::Backslash, "\\"),
            std::make_pair(SLOPE::Slash, "/"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<STROKE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(STROKE::Solid, "solid"),
            std::make_pair(STROKE::DashDot, "dashDot"),
            std::make_pair(STROKE::DashDotDot, "dashDotDot"),
            std::make_pair(STROKE::Dashed, "dashed"),
            std::make_pair(STROKE::Dotted, "dotted"),
            std::make_pair(STROKE::Embossed, "embossed"),
            std::make_pair(STROKE::Etched, "etched"),
            std::make_pair(STROKE::Lowered, "lowered"),
            std::make_pair(STROKE::Raised, "raised"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TARGETTYPE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TARGETTYPE::Auto, "auto"),
            std::make_pair(TARGETTYPE::ContentArea, "contentArea"),
            std::make_pair(TARGETTYPE::PageArea, "pageArea"),
            std::make_pair(TARGETTYPE::PageEven, "pageEven"),
            std::make_pair(TARGETTYPE::PageOdd, "pageOdd"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TEXTLOCATION>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TEXTLOCATION::Below, "below"),
            std::make_pair(TEXTLOCATION::Above, "above"),
            std::make_pair(TEXTLOCATION::AboveEmbedded, "aboveEmbedded"),
            std::make_pair(TEXTLOCATION::BelowEmbedded, "belowEmbedded"),
            std::make_pair(TEXTLOCATION::None, "none"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TRANSFERENCODING1>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TRANSFERENCODING1::Base64, "base64"),
            std::make_pair(TRANSFERENCODING1::None, "none"),
            std::make_pair(TRANSFERENCODING1::Package, "package"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TRANSFERENCODING>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TRANSFERENCODING::None, "none"),
            std::make_pair(TRANSFERENCODING::Base64, "base64"),
            std::make_pair(TRANSFERENCODING::Package, "package"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TRAYIN>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TRAYIN::Auto, "auto"),
            std::make_pair(TRAYIN::Delegate, "delegate"),
            std::make_pair(TRAYIN::PageFront, "pageFront"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TRAYOUT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TRAYOUT::Auto, "auto"),
            std::make_pair(TRAYOUT::Delegate, "delegate"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TYPE4>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TYPE4::PDF1_3, "PDF1.3"),
            std::make_pair(TYPE4::PDF1_6, "PDF1.6"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TYPE2>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TYPE2::CrossHatch, "crossHatch"),
            std::make_pair(TYPE2::CrossDiagonal, "crossDiagonal"),
            std::make_pair(TYPE2::DiagonalLeft, "diagonalLeft"),
            std::make_pair(TYPE2::DiagonalRight, "diagonalRight"),
            std::make_pair(TYPE2::Horizontal, "horizontal"),
            std::make_pair(TYPE2::Vertical, "vertical"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TYPE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TYPE::Optional, "optional"),
            std::make_pair(TYPE::Required, "required"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TYPE3>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TYPE3::ToEdge, "toEdge"),
            std::make_pair(TYPE3::ToCenter, "toCenter"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<TYPE1>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(TYPE1::ToRight, "toRight"),
            std::make_pair(TYPE1::ToBottom, "toBottom"),
            std::make_pair(TYPE1::ToLeft, "toLeft"),
            std::make_pair(TYPE1::ToTop, "toTop"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<UNDERLINEPERIOD>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(UNDERLINEPERIOD::All, "all"),
            std::make_pair(UNDERLINEPERIOD::Word, "word"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<UNDERLINE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(UNDERLINE::_0, "0"),
            std::make_pair(UNDERLINE::_1, "1"),
            std::make_pair(UNDERLINE::_2, "2"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<UPSMODE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(UPSMODE::UsCarrier, "usCarrier"),
            std::make_pair(UPSMODE::InternationalCarrier, "internationalCarrier"),
            std::make_pair(UPSMODE::SecureSymbol, "secureSymbol"),
            std::make_pair(UPSMODE::StandardSymbol, "standardSymbol"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<USAGE>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(USAGE::ExportAndImport, "exportAndImport"),
            std::make_pair(USAGE::ExportOnly, "exportOnly"),
            std::make_pair(USAGE::ImportOnly, "importOnly"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<VALIGN>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(VALIGN::Top, "top"),
            std::make_pair(VALIGN::Bottom, "bottom"),
            std::make_pair(VALIGN::Middle, "middle"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<VSCROLLPOLICY>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(VSCROLLPOLICY::Auto, "auto"),
            std::make_pair(VSCROLLPOLICY::Off, "off"),
            std::make_pair(VSCROLLPOLICY::On, "on"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

    static void parseAttribute(const QDomElement& element, QString attributeFieldName, XFA_Attribute<WEIGHT>& attribute, QString defaultValue)
    {
        constexpr std::array enumValues = {
            std::make_pair(WEIGHT::Normal, "normal"),
            std::make_pair(WEIGHT::Bold, "bold"),
        };
        parseEnumAttribute(element, attributeFieldName, attribute, defaultValue, enumValues);
    }

};

class XFA_appearanceFilter : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_appearanceFilter> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_appearanceFilter> XFA_appearanceFilter::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_appearanceFilter myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_bindItems : public XFA_BaseNode
{
public:

    QString getConnection() const {  return m_connection.getValueOrDefault(); }
    QString getLabelRef() const {  return m_labelRef.getValueOrDefault(); }
    QString getRef() const {  return m_ref.getValueOrDefault(); }
    QString getValueRef() const {  return m_valueRef.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_bindItems> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_connection;
    XFA_Attribute<QString> m_labelRef;
    XFA_Attribute<QString> m_ref;
    XFA_Attribute<QString> m_valueRef;

    /* subnodes */
};

std::optional<XFA_bindItems> XFA_bindItems::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_bindItems myClass;

    // load attributes
    parseAttribute(element, "connection", myClass.m_connection, "");
    parseAttribute(element, "labelRef", myClass.m_labelRef, "");
    parseAttribute(element, "ref", myClass.m_ref, "");
    parseAttribute(element, "valueRef", myClass.m_valueRef, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_bookend : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getLeader() const {  return m_leader.getValueOrDefault(); }
    QString getTrailer() const {  return m_trailer.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_bookend> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_leader;
    XFA_Attribute<QString> m_trailer;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
};

std::optional<XFA_bookend> XFA_bookend::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_bookend myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "leader", myClass.m_leader, "");
    parseAttribute(element, "trailer", myClass.m_trailer, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_boolean : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_boolean> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_boolean> XFA_boolean::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_boolean myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_certificate : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_certificate> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_certificate> XFA_certificate::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_certificate myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_comb : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFInteger getNumberOfCells() const {  return m_numberOfCells.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_comb> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFInteger> m_numberOfCells;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
};

std::optional<XFA_comb> XFA_comb::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_comb myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "numberOfCells", myClass.m_numberOfCells, "0");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_date : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_date> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_date> XFA_date::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_date myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_dateTime : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_dateTime> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_dateTime> XFA_dateTime::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_dateTime myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_decimal : public XFA_BaseNode
{
public:

    PDFInteger getFracDigits() const {  return m_fracDigits.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFInteger getLeadDigits() const {  return m_leadDigits.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_decimal> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<PDFInteger> m_fracDigits;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFInteger> m_leadDigits;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_decimal> XFA_decimal::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_decimal myClass;

    // load attributes
    parseAttribute(element, "fracDigits", myClass.m_fracDigits, "2");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "leadDigits", myClass.m_leadDigits, "-1");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_digestMethod : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_digestMethod> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_digestMethod> XFA_digestMethod::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_digestMethod myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_digestMethods : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_digestMethod>>& getDigestMethod() const {  return m_digestMethod; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_digestMethods> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_digestMethod>> m_digestMethod;
};

std::optional<XFA_digestMethods> XFA_digestMethods::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_digestMethods myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "digestMethod", myClass.m_digestMethod);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_encoding : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_encoding> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_encoding> XFA_encoding::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_encoding myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_encodings : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_encoding>>& getEncoding() const {  return m_encoding; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_encodings> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_encoding>> m_encoding;
};

std::optional<XFA_encodings> XFA_encodings::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_encodings myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "encoding", myClass.m_encoding);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_encrypt : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_certificate* getCertificate() const {  return m_certificate.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_encrypt> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_certificate> m_certificate;
};

std::optional<XFA_encrypt> XFA_encrypt::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_encrypt myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "certificate", myClass.m_certificate);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_encryption : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_certificate>>& getCertificate() const {  return m_certificate; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_encryption> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_certificate>> m_certificate;
};

std::optional<XFA_encryption> XFA_encryption::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_encryption myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "certificate", myClass.m_certificate);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_encryptionMethod : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_encryptionMethod> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_encryptionMethod> XFA_encryptionMethod::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_encryptionMethod myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_encryptionMethods : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_encryptionMethod>>& getEncryptionMethod() const {  return m_encryptionMethod; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_encryptionMethods> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_encryptionMethod>> m_encryptionMethod;
};

std::optional<XFA_encryptionMethods> XFA_encryptionMethods::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_encryptionMethods myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "encryptionMethod", myClass.m_encryptionMethod);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_exData : public XFA_BaseNode
{
public:

    QString getContentType() const {  return m_contentType.getValueOrDefault(); }
    QString getHref() const {  return m_href.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFInteger getMaxLength() const {  return m_maxLength.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getRid() const {  return m_rid.getValueOrDefault(); }
    TRANSFERENCODING getTransferEncoding() const {  return m_transferEncoding.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_exData> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_contentType;
    XFA_Attribute<QString> m_href;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFInteger> m_maxLength;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_rid;
    XFA_Attribute<TRANSFERENCODING> m_transferEncoding;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_exData> XFA_exData::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_exData myClass;

    // load attributes
    parseAttribute(element, "contentType", myClass.m_contentType, "");
    parseAttribute(element, "href", myClass.m_href, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "maxLength", myClass.m_maxLength, "-1");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "rid", myClass.m_rid, "");
    parseAttribute(element, "transferEncoding", myClass.m_transferEncoding, "none");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_execute : public XFA_BaseNode
{
public:

    QString getConnection() const {  return m_connection.getValueOrDefault(); }
    EXECUTETYPE getExecuteType() const {  return m_executeType.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    RUNAT getRunAt() const {  return m_runAt.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_execute> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_connection;
    XFA_Attribute<EXECUTETYPE> m_executeType;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<RUNAT> m_runAt;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
};

std::optional<XFA_execute> XFA_execute::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_execute myClass;

    // load attributes
    parseAttribute(element, "connection", myClass.m_connection, "");
    parseAttribute(element, "executeType", myClass.m_executeType, "import");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "runAt", myClass.m_runAt, "client");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_float : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_float> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_float> XFA_float::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_float myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_handler : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_handler> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_handler> XFA_handler::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_handler myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_hyphenation : public XFA_BaseNode
{
public:

    bool getExcludeAllCaps() const {  return m_excludeAllCaps.getValueOrDefault(); }
    bool getExcludeInitialCap() const {  return m_excludeInitialCap.getValueOrDefault(); }
    bool getHyphenate() const {  return m_hyphenate.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFInteger getPushCharacterCount() const {  return m_pushCharacterCount.getValueOrDefault(); }
    PDFInteger getRemainCharacterCount() const {  return m_remainCharacterCount.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    PDFInteger getWordCharacterCount() const {  return m_wordCharacterCount.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_hyphenation> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<bool> m_excludeAllCaps;
    XFA_Attribute<bool> m_excludeInitialCap;
    XFA_Attribute<bool> m_hyphenate;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFInteger> m_pushCharacterCount;
    XFA_Attribute<PDFInteger> m_remainCharacterCount;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<PDFInteger> m_wordCharacterCount;

    /* subnodes */
};

std::optional<XFA_hyphenation> XFA_hyphenation::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_hyphenation myClass;

    // load attributes
    parseAttribute(element, "excludeAllCaps", myClass.m_excludeAllCaps, "0");
    parseAttribute(element, "excludeInitialCap", myClass.m_excludeInitialCap, "0");
    parseAttribute(element, "hyphenate", myClass.m_hyphenate, "0");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "pushCharacterCount", myClass.m_pushCharacterCount, "3");
    parseAttribute(element, "remainCharacterCount", myClass.m_remainCharacterCount, "3");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "wordCharacterCount", myClass.m_wordCharacterCount, "7");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_image : public XFA_BaseNode
{
public:

    ASPECT getAspect() const {  return m_aspect.getValueOrDefault(); }
    QString getContentType() const {  return m_contentType.getValueOrDefault(); }
    QString getHref() const {  return m_href.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    TRANSFERENCODING1 getTransferEncoding() const {  return m_transferEncoding.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_image> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<ASPECT> m_aspect;
    XFA_Attribute<QString> m_contentType;
    XFA_Attribute<QString> m_href;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<TRANSFERENCODING1> m_transferEncoding;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_image> XFA_image::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_image myClass;

    // load attributes
    parseAttribute(element, "aspect", myClass.m_aspect, "fit");
    parseAttribute(element, "contentType", myClass.m_contentType, "");
    parseAttribute(element, "href", myClass.m_href, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "transferEncoding", myClass.m_transferEncoding, "base64");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_integer : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_integer> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_integer> XFA_integer::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_integer myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_issuers : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_certificate>>& getCertificate() const {  return m_certificate; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_issuers> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_certificate>> m_certificate;
};

std::optional<XFA_issuers> XFA_issuers::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_issuers myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "certificate", myClass.m_certificate);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_keyUsage : public XFA_BaseNode
{
public:

    QString getCrlSign() const {  return m_crlSign.getValueOrDefault(); }
    QString getDataEncipherment() const {  return m_dataEncipherment.getValueOrDefault(); }
    QString getDecipherOnly() const {  return m_decipherOnly.getValueOrDefault(); }
    QString getDigitalSignature() const {  return m_digitalSignature.getValueOrDefault(); }
    QString getEncipherOnly() const {  return m_encipherOnly.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getKeyAgreement() const {  return m_keyAgreement.getValueOrDefault(); }
    QString getKeyCertSign() const {  return m_keyCertSign.getValueOrDefault(); }
    QString getKeyEncipherment() const {  return m_keyEncipherment.getValueOrDefault(); }
    QString getNonRepudiation() const {  return m_nonRepudiation.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_keyUsage> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_crlSign;
    XFA_Attribute<QString> m_dataEncipherment;
    XFA_Attribute<QString> m_decipherOnly;
    XFA_Attribute<QString> m_digitalSignature;
    XFA_Attribute<QString> m_encipherOnly;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_keyAgreement;
    XFA_Attribute<QString> m_keyCertSign;
    XFA_Attribute<QString> m_keyEncipherment;
    XFA_Attribute<QString> m_nonRepudiation;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
};

std::optional<XFA_keyUsage> XFA_keyUsage::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_keyUsage myClass;

    // load attributes
    parseAttribute(element, "crlSign", myClass.m_crlSign, "");
    parseAttribute(element, "dataEncipherment", myClass.m_dataEncipherment, "");
    parseAttribute(element, "decipherOnly", myClass.m_decipherOnly, "");
    parseAttribute(element, "digitalSignature", myClass.m_digitalSignature, "");
    parseAttribute(element, "encipherOnly", myClass.m_encipherOnly, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "keyAgreement", myClass.m_keyAgreement, "");
    parseAttribute(element, "keyCertSign", myClass.m_keyCertSign, "");
    parseAttribute(element, "keyEncipherment", myClass.m_keyEncipherment, "");
    parseAttribute(element, "nonRepudiation", myClass.m_nonRepudiation, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_lockDocument : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_lockDocument> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_lockDocument> XFA_lockDocument::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_lockDocument myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_mdp : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    PERMISSIONS getPermissions() const {  return m_permissions.getValueOrDefault(); }
    SIGNATURETYPE getSignatureType() const {  return m_signatureType.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_mdp> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PERMISSIONS> m_permissions;
    XFA_Attribute<SIGNATURETYPE> m_signatureType;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
};

std::optional<XFA_mdp> XFA_mdp::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_mdp myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "permissions", myClass.m_permissions, "2");
    parseAttribute(element, "signatureType", myClass.m_signatureType, "filler");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_medium : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getImagingBBox() const {  return m_imagingBBox.getValueOrDefault(); }
    XFA_Measurement getLong() const {  return m_long.getValueOrDefault(); }
    ORIENTATION getOrientation() const {  return m_orientation.getValueOrDefault(); }
    XFA_Measurement getShort() const {  return m_short.getValueOrDefault(); }
    QString getStock() const {  return m_stock.getValueOrDefault(); }
    TRAYIN getTrayIn() const {  return m_trayIn.getValueOrDefault(); }
    TRAYOUT getTrayOut() const {  return m_trayOut.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_medium> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_imagingBBox;
    XFA_Attribute<XFA_Measurement> m_long;
    XFA_Attribute<ORIENTATION> m_orientation;
    XFA_Attribute<XFA_Measurement> m_short;
    XFA_Attribute<QString> m_stock;
    XFA_Attribute<TRAYIN> m_trayIn;
    XFA_Attribute<TRAYOUT> m_trayOut;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
};

std::optional<XFA_medium> XFA_medium::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_medium myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "imagingBBox", myClass.m_imagingBBox, "");
    parseAttribute(element, "long", myClass.m_long, "0in");
    parseAttribute(element, "orientation", myClass.m_orientation, "portrait");
    parseAttribute(element, "short", myClass.m_short, "0in");
    parseAttribute(element, "stock", myClass.m_stock, "");
    parseAttribute(element, "trayIn", myClass.m_trayIn, "auto");
    parseAttribute(element, "trayOut", myClass.m_trayOut, "auto");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_oid : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_oid> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_oid> XFA_oid::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_oid myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_oids : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_oid>>& getOid() const {  return m_oid; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_oids> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_oid>> m_oid;
};

std::optional<XFA_oids> XFA_oids::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_oids myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "oid", myClass.m_oid);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_overflow : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getLeader() const {  return m_leader.getValueOrDefault(); }
    QString getTarget() const {  return m_target.getValueOrDefault(); }
    QString getTrailer() const {  return m_trailer.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_overflow> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_leader;
    XFA_Attribute<QString> m_target;
    XFA_Attribute<QString> m_trailer;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
};

std::optional<XFA_overflow> XFA_overflow::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_overflow myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "leader", myClass.m_leader, "");
    parseAttribute(element, "target", myClass.m_target, "");
    parseAttribute(element, "trailer", myClass.m_trailer, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_para : public XFA_BaseNode
{
public:

    HALIGN getHAlign() const {  return m_hAlign.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    XFA_Measurement getLineHeight() const {  return m_lineHeight.getValueOrDefault(); }
    XFA_Measurement getMarginLeft() const {  return m_marginLeft.getValueOrDefault(); }
    XFA_Measurement getMarginRight() const {  return m_marginRight.getValueOrDefault(); }
    PDFInteger getOrphans() const {  return m_orphans.getValueOrDefault(); }
    QString getPreserve() const {  return m_preserve.getValueOrDefault(); }
    XFA_Measurement getRadixOffset() const {  return m_radixOffset.getValueOrDefault(); }
    XFA_Measurement getSpaceAbove() const {  return m_spaceAbove.getValueOrDefault(); }
    XFA_Measurement getSpaceBelow() const {  return m_spaceBelow.getValueOrDefault(); }
    QString getTabDefault() const {  return m_tabDefault.getValueOrDefault(); }
    QString getTabStops() const {  return m_tabStops.getValueOrDefault(); }
    XFA_Measurement getTextIndent() const {  return m_textIndent.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    VALIGN getVAlign() const {  return m_vAlign.getValueOrDefault(); }
    PDFInteger getWidows() const {  return m_widows.getValueOrDefault(); }

    const XFA_hyphenation* getHyphenation() const {  return m_hyphenation.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_para> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<HALIGN> m_hAlign;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<XFA_Measurement> m_lineHeight;
    XFA_Attribute<XFA_Measurement> m_marginLeft;
    XFA_Attribute<XFA_Measurement> m_marginRight;
    XFA_Attribute<PDFInteger> m_orphans;
    XFA_Attribute<QString> m_preserve;
    XFA_Attribute<XFA_Measurement> m_radixOffset;
    XFA_Attribute<XFA_Measurement> m_spaceAbove;
    XFA_Attribute<XFA_Measurement> m_spaceBelow;
    XFA_Attribute<QString> m_tabDefault;
    XFA_Attribute<QString> m_tabStops;
    XFA_Attribute<XFA_Measurement> m_textIndent;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<VALIGN> m_vAlign;
    XFA_Attribute<PDFInteger> m_widows;

    /* subnodes */
    XFA_Node<XFA_hyphenation> m_hyphenation;
};

std::optional<XFA_para> XFA_para::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_para myClass;

    // load attributes
    parseAttribute(element, "hAlign", myClass.m_hAlign, "left");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "lineHeight", myClass.m_lineHeight, "0pt");
    parseAttribute(element, "marginLeft", myClass.m_marginLeft, "0in");
    parseAttribute(element, "marginRight", myClass.m_marginRight, "0in");
    parseAttribute(element, "orphans", myClass.m_orphans, "0");
    parseAttribute(element, "preserve", myClass.m_preserve, "");
    parseAttribute(element, "radixOffset", myClass.m_radixOffset, "0in");
    parseAttribute(element, "spaceAbove", myClass.m_spaceAbove, "0in");
    parseAttribute(element, "spaceBelow", myClass.m_spaceBelow, "0in");
    parseAttribute(element, "tabDefault", myClass.m_tabDefault, "");
    parseAttribute(element, "tabStops", myClass.m_tabStops, "");
    parseAttribute(element, "textIndent", myClass.m_textIndent, "0in");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "vAlign", myClass.m_vAlign, "top");
    parseAttribute(element, "widows", myClass.m_widows, "0");

    // load items
    parseItem(element, "hyphenation", myClass.m_hyphenation);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_picture : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_picture> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_picture> XFA_picture::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_picture myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_bind : public XFA_BaseNode
{
public:

    MATCH getMatch() const {  return m_match.getValueOrDefault(); }
    QString getRef() const {  return m_ref.getValueOrDefault(); }

    const XFA_picture* getPicture() const {  return m_picture.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_bind> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<MATCH> m_match;
    XFA_Attribute<QString> m_ref;

    /* subnodes */
    XFA_Node<XFA_picture> m_picture;
};

std::optional<XFA_bind> XFA_bind::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_bind myClass;

    // load attributes
    parseAttribute(element, "match", myClass.m_match, "once");
    parseAttribute(element, "ref", myClass.m_ref, "");

    // load items
    parseItem(element, "picture", myClass.m_picture);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_connect : public XFA_BaseNode
{
public:

    QString getConnection() const {  return m_connection.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getRef() const {  return m_ref.getValueOrDefault(); }
    USAGE getUsage() const {  return m_usage.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_picture* getPicture() const {  return m_picture.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_connect> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_connection;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_ref;
    XFA_Attribute<USAGE> m_usage;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_picture> m_picture;
};

std::optional<XFA_connect> XFA_connect::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_connect myClass;

    // load attributes
    parseAttribute(element, "connection", myClass.m_connection, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "ref", myClass.m_ref, "");
    parseAttribute(element, "usage", myClass.m_usage, "exportAndImport");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "picture", myClass.m_picture);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_reason : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_reason> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_reason> XFA_reason::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_reason myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_reasons : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_reason>>& getReason() const {  return m_reason; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_reasons> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_reason>> m_reason;
};

std::optional<XFA_reasons> XFA_reasons::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_reasons myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "reason", myClass.m_reason);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_ref : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_ref> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_ref> XFA_ref::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_ref myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_script : public XFA_BaseNode
{
public:

    QString getBinding() const {  return m_binding.getValueOrDefault(); }
    QString getContentType() const {  return m_contentType.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    RUNAT getRunAt() const {  return m_runAt.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_script> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_binding;
    XFA_Attribute<QString> m_contentType;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<RUNAT> m_runAt;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_script> XFA_script::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_script myClass;

    // load attributes
    parseAttribute(element, "binding", myClass.m_binding, "");
    parseAttribute(element, "contentType", myClass.m_contentType, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "runAt", myClass.m_runAt, "client");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_breakAfter : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getLeader() const {  return m_leader.getValueOrDefault(); }
    bool getStartNew() const {  return m_startNew.getValueOrDefault(); }
    QString getTarget() const {  return m_target.getValueOrDefault(); }
    TARGETTYPE getTargetType() const {  return m_targetType.getValueOrDefault(); }
    QString getTrailer() const {  return m_trailer.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_script* getScript() const {  return m_script.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_breakAfter> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_leader;
    XFA_Attribute<bool> m_startNew;
    XFA_Attribute<QString> m_target;
    XFA_Attribute<TARGETTYPE> m_targetType;
    XFA_Attribute<QString> m_trailer;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_script> m_script;
};

std::optional<XFA_breakAfter> XFA_breakAfter::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_breakAfter myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "leader", myClass.m_leader, "");
    parseAttribute(element, "startNew", myClass.m_startNew, "0");
    parseAttribute(element, "target", myClass.m_target, "");
    parseAttribute(element, "targetType", myClass.m_targetType, "auto");
    parseAttribute(element, "trailer", myClass.m_trailer, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "script", myClass.m_script);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_breakBefore : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getLeader() const {  return m_leader.getValueOrDefault(); }
    bool getStartNew() const {  return m_startNew.getValueOrDefault(); }
    QString getTarget() const {  return m_target.getValueOrDefault(); }
    TARGETTYPE getTargetType() const {  return m_targetType.getValueOrDefault(); }
    QString getTrailer() const {  return m_trailer.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_script* getScript() const {  return m_script.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_breakBefore> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_leader;
    XFA_Attribute<bool> m_startNew;
    XFA_Attribute<QString> m_target;
    XFA_Attribute<TARGETTYPE> m_targetType;
    XFA_Attribute<QString> m_trailer;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_script> m_script;
};

std::optional<XFA_breakBefore> XFA_breakBefore::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_breakBefore myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "leader", myClass.m_leader, "");
    parseAttribute(element, "startNew", myClass.m_startNew, "0");
    parseAttribute(element, "target", myClass.m_target, "");
    parseAttribute(element, "targetType", myClass.m_targetType, "auto");
    parseAttribute(element, "trailer", myClass.m_trailer, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "script", myClass.m_script);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_setProperty : public XFA_BaseNode
{
public:

    QString getConnection() const {  return m_connection.getValueOrDefault(); }
    QString getRef() const {  return m_ref.getValueOrDefault(); }
    QString getTarget() const {  return m_target.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_setProperty> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_connection;
    XFA_Attribute<QString> m_ref;
    XFA_Attribute<QString> m_target;

    /* subnodes */
};

std::optional<XFA_setProperty> XFA_setProperty::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_setProperty myClass;

    // load attributes
    parseAttribute(element, "connection", myClass.m_connection, "");
    parseAttribute(element, "ref", myClass.m_ref, "");
    parseAttribute(element, "target", myClass.m_target, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_signing : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_certificate>>& getCertificate() const {  return m_certificate; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_signing> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_certificate>> m_certificate;
};

std::optional<XFA_signing> XFA_signing::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_signing myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "certificate", myClass.m_certificate);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_speak : public XFA_BaseNode
{
public:

    bool getDisable() const {  return m_disable.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PRIORITY getPriority() const {  return m_priority.getValueOrDefault(); }
    QString getRid() const {  return m_rid.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_speak> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<bool> m_disable;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PRIORITY> m_priority;
    XFA_Attribute<QString> m_rid;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_speak> XFA_speak::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_speak myClass;

    // load attributes
    parseAttribute(element, "disable", myClass.m_disable, "0");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "priority", myClass.m_priority, "custom");
    parseAttribute(element, "rid", myClass.m_rid, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_subjectDN : public XFA_BaseNode
{
public:

    QString getDelimiter() const {  return m_delimiter.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_subjectDN> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_delimiter;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_subjectDN> XFA_subjectDN::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_subjectDN myClass;

    // load attributes
    parseAttribute(element, "delimiter", myClass.m_delimiter, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_subjectDNs : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_subjectDN>>& getSubjectDN() const {  return m_subjectDN; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_subjectDNs> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_subjectDN>> m_subjectDN;
};

std::optional<XFA_subjectDNs> XFA_subjectDNs::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_subjectDNs myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "subjectDN", myClass.m_subjectDN);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_certificates : public XFA_BaseNode
{
public:

    CREDENTIALSERVERPOLICY getCredentialServerPolicy() const {  return m_credentialServerPolicy.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUrl() const {  return m_url.getValueOrDefault(); }
    QString getUrlPolicy() const {  return m_urlPolicy.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_encryption* getEncryption() const {  return m_encryption.getValue(); }
    const XFA_issuers* getIssuers() const {  return m_issuers.getValue(); }
    const XFA_keyUsage* getKeyUsage() const {  return m_keyUsage.getValue(); }
    const XFA_oids* getOids() const {  return m_oids.getValue(); }
    const XFA_signing* getSigning() const {  return m_signing.getValue(); }
    const XFA_subjectDNs* getSubjectDNs() const {  return m_subjectDNs.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_certificates> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<CREDENTIALSERVERPOLICY> m_credentialServerPolicy;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_url;
    XFA_Attribute<QString> m_urlPolicy;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_encryption> m_encryption;
    XFA_Node<XFA_issuers> m_issuers;
    XFA_Node<XFA_keyUsage> m_keyUsage;
    XFA_Node<XFA_oids> m_oids;
    XFA_Node<XFA_signing> m_signing;
    XFA_Node<XFA_subjectDNs> m_subjectDNs;
};

std::optional<XFA_certificates> XFA_certificates::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_certificates myClass;

    // load attributes
    parseAttribute(element, "credentialServerPolicy", myClass.m_credentialServerPolicy, "optional");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "url", myClass.m_url, "");
    parseAttribute(element, "urlPolicy", myClass.m_urlPolicy, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "encryption", myClass.m_encryption);
    parseItem(element, "issuers", myClass.m_issuers);
    parseItem(element, "keyUsage", myClass.m_keyUsage);
    parseItem(element, "oids", myClass.m_oids);
    parseItem(element, "signing", myClass.m_signing);
    parseItem(element, "subjectDNs", myClass.m_subjectDNs);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_text : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFInteger getMaxChars() const {  return m_maxChars.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getRid() const {  return m_rid.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_text> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFInteger> m_maxChars;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_rid;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_text> XFA_text::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_text myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "maxChars", myClass.m_maxChars, "0");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "rid", myClass.m_rid, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_message : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_text>>& getText() const {  return m_text; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_message> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_text>> m_text;
};

std::optional<XFA_message> XFA_message::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_message myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "text", myClass.m_text);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_time : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_time> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_time> XFA_time::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_time myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_desc : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_boolean>>& getBoolean() const {  return m_boolean; }
    const std::vector<XFA_Node<XFA_date>>& getDate() const {  return m_date; }
    const std::vector<XFA_Node<XFA_dateTime>>& getDateTime() const {  return m_dateTime; }
    const std::vector<XFA_Node<XFA_decimal>>& getDecimal() const {  return m_decimal; }
    const std::vector<XFA_Node<XFA_exData>>& getExData() const {  return m_exData; }
    const std::vector<XFA_Node<XFA_float>>& getFloat() const {  return m_float; }
    const std::vector<XFA_Node<XFA_image>>& getImage() const {  return m_image; }
    const std::vector<XFA_Node<XFA_integer>>& getInteger() const {  return m_integer; }
    const std::vector<XFA_Node<XFA_text>>& getText() const {  return m_text; }
    const std::vector<XFA_Node<XFA_time>>& getTime() const {  return m_time; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_desc> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_boolean>> m_boolean;
    std::vector<XFA_Node<XFA_date>> m_date;
    std::vector<XFA_Node<XFA_dateTime>> m_dateTime;
    std::vector<XFA_Node<XFA_decimal>> m_decimal;
    std::vector<XFA_Node<XFA_exData>> m_exData;
    std::vector<XFA_Node<XFA_float>> m_float;
    std::vector<XFA_Node<XFA_image>> m_image;
    std::vector<XFA_Node<XFA_integer>> m_integer;
    std::vector<XFA_Node<XFA_text>> m_text;
    std::vector<XFA_Node<XFA_time>> m_time;
};

std::optional<XFA_desc> XFA_desc::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_desc myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "boolean", myClass.m_boolean);
    parseItem(element, "date", myClass.m_date);
    parseItem(element, "dateTime", myClass.m_dateTime);
    parseItem(element, "decimal", myClass.m_decimal);
    parseItem(element, "exData", myClass.m_exData);
    parseItem(element, "float", myClass.m_float);
    parseItem(element, "image", myClass.m_image);
    parseItem(element, "integer", myClass.m_integer);
    parseItem(element, "text", myClass.m_text);
    parseItem(element, "time", myClass.m_time);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_extras : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_boolean>>& getBoolean() const {  return m_boolean; }
    const std::vector<XFA_Node<XFA_date>>& getDate() const {  return m_date; }
    const std::vector<XFA_Node<XFA_dateTime>>& getDateTime() const {  return m_dateTime; }
    const std::vector<XFA_Node<XFA_decimal>>& getDecimal() const {  return m_decimal; }
    const std::vector<XFA_Node<XFA_exData>>& getExData() const {  return m_exData; }
    const std::vector<XFA_Node<XFA_extras>>& getExtras() const {  return m_extras; }
    const std::vector<XFA_Node<XFA_float>>& getFloat() const {  return m_float; }
    const std::vector<XFA_Node<XFA_image>>& getImage() const {  return m_image; }
    const std::vector<XFA_Node<XFA_integer>>& getInteger() const {  return m_integer; }
    const std::vector<XFA_Node<XFA_text>>& getText() const {  return m_text; }
    const std::vector<XFA_Node<XFA_time>>& getTime() const {  return m_time; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_extras> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_boolean>> m_boolean;
    std::vector<XFA_Node<XFA_date>> m_date;
    std::vector<XFA_Node<XFA_dateTime>> m_dateTime;
    std::vector<XFA_Node<XFA_decimal>> m_decimal;
    std::vector<XFA_Node<XFA_exData>> m_exData;
    std::vector<XFA_Node<XFA_extras>> m_extras;
    std::vector<XFA_Node<XFA_float>> m_float;
    std::vector<XFA_Node<XFA_image>> m_image;
    std::vector<XFA_Node<XFA_integer>> m_integer;
    std::vector<XFA_Node<XFA_text>> m_text;
    std::vector<XFA_Node<XFA_time>> m_time;
};

std::optional<XFA_extras> XFA_extras::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_extras myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "boolean", myClass.m_boolean);
    parseItem(element, "date", myClass.m_date);
    parseItem(element, "dateTime", myClass.m_dateTime);
    parseItem(element, "decimal", myClass.m_decimal);
    parseItem(element, "exData", myClass.m_exData);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "float", myClass.m_float);
    parseItem(element, "image", myClass.m_image);
    parseItem(element, "integer", myClass.m_integer);
    parseItem(element, "text", myClass.m_text);
    parseItem(element, "time", myClass.m_time);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_barcode : public XFA_BaseNode
{
public:

    QString getCharEncoding() const {  return m_charEncoding.getValueOrDefault(); }
    CHECKSUM getChecksum() const {  return m_checksum.getValueOrDefault(); }
    QString getDataColumnCount() const {  return m_dataColumnCount.getValueOrDefault(); }
    QString getDataLength() const {  return m_dataLength.getValueOrDefault(); }
    DATAPREP getDataPrep() const {  return m_dataPrep.getValueOrDefault(); }
    QString getDataRowCount() const {  return m_dataRowCount.getValueOrDefault(); }
    QString getEndChar() const {  return m_endChar.getValueOrDefault(); }
    QString getErrorCorrectionLevel() const {  return m_errorCorrectionLevel.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    XFA_Measurement getModuleHeight() const {  return m_moduleHeight.getValueOrDefault(); }
    XFA_Measurement getModuleWidth() const {  return m_moduleWidth.getValueOrDefault(); }
    bool getPrintCheckDigit() const {  return m_printCheckDigit.getValueOrDefault(); }
    QString getRowColumnRatio() const {  return m_rowColumnRatio.getValueOrDefault(); }
    QString getStartChar() const {  return m_startChar.getValueOrDefault(); }
    TEXTLOCATION getTextLocation() const {  return m_textLocation.getValueOrDefault(); }
    bool getTruncate() const {  return m_truncate.getValueOrDefault(); }
    QString getType() const {  return m_type.getValueOrDefault(); }
    UPSMODE getUpsMode() const {  return m_upsMode.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    QString getWideNarrowRatio() const {  return m_wideNarrowRatio.getValueOrDefault(); }

    const XFA_encrypt* getEncrypt() const {  return m_encrypt.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_barcode> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_charEncoding;
    XFA_Attribute<CHECKSUM> m_checksum;
    XFA_Attribute<QString> m_dataColumnCount;
    XFA_Attribute<QString> m_dataLength;
    XFA_Attribute<DATAPREP> m_dataPrep;
    XFA_Attribute<QString> m_dataRowCount;
    XFA_Attribute<QString> m_endChar;
    XFA_Attribute<QString> m_errorCorrectionLevel;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<XFA_Measurement> m_moduleHeight;
    XFA_Attribute<XFA_Measurement> m_moduleWidth;
    XFA_Attribute<bool> m_printCheckDigit;
    XFA_Attribute<QString> m_rowColumnRatio;
    XFA_Attribute<QString> m_startChar;
    XFA_Attribute<TEXTLOCATION> m_textLocation;
    XFA_Attribute<bool> m_truncate;
    XFA_Attribute<QString> m_type;
    XFA_Attribute<UPSMODE> m_upsMode;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<QString> m_wideNarrowRatio;

    /* subnodes */
    XFA_Node<XFA_encrypt> m_encrypt;
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_barcode> XFA_barcode::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_barcode myClass;

    // load attributes
    parseAttribute(element, "charEncoding", myClass.m_charEncoding, "");
    parseAttribute(element, "checksum", myClass.m_checksum, "none");
    parseAttribute(element, "dataColumnCount", myClass.m_dataColumnCount, "");
    parseAttribute(element, "dataLength", myClass.m_dataLength, "");
    parseAttribute(element, "dataPrep", myClass.m_dataPrep, "none");
    parseAttribute(element, "dataRowCount", myClass.m_dataRowCount, "");
    parseAttribute(element, "endChar", myClass.m_endChar, "");
    parseAttribute(element, "errorCorrectionLevel", myClass.m_errorCorrectionLevel, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "moduleHeight", myClass.m_moduleHeight, "5mm");
    parseAttribute(element, "moduleWidth", myClass.m_moduleWidth, "0.25mm");
    parseAttribute(element, "printCheckDigit", myClass.m_printCheckDigit, "0");
    parseAttribute(element, "rowColumnRatio", myClass.m_rowColumnRatio, "");
    parseAttribute(element, "startChar", myClass.m_startChar, "");
    parseAttribute(element, "textLocation", myClass.m_textLocation, "below");
    parseAttribute(element, "truncate", myClass.m_truncate, "");
    parseAttribute(element, "type", myClass.m_type, "");
    parseAttribute(element, "upsMode", myClass.m_upsMode, "usCarrier");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "wideNarrowRatio", myClass.m_wideNarrowRatio, "");

    // load items
    parseItem(element, "encrypt", myClass.m_encrypt);
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_break : public XFA_BaseNode
{
public:

    AFTER getAfter() const {  return m_after.getValueOrDefault(); }
    QString getAfterTarget() const {  return m_afterTarget.getValueOrDefault(); }
    BEFORE getBefore() const {  return m_before.getValueOrDefault(); }
    QString getBeforeTarget() const {  return m_beforeTarget.getValueOrDefault(); }
    QString getBookendLeader() const {  return m_bookendLeader.getValueOrDefault(); }
    QString getBookendTrailer() const {  return m_bookendTrailer.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getOverflowLeader() const {  return m_overflowLeader.getValueOrDefault(); }
    QString getOverflowTarget() const {  return m_overflowTarget.getValueOrDefault(); }
    QString getOverflowTrailer() const {  return m_overflowTrailer.getValueOrDefault(); }
    bool getStartNew() const {  return m_startNew.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_break> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<AFTER> m_after;
    XFA_Attribute<QString> m_afterTarget;
    XFA_Attribute<BEFORE> m_before;
    XFA_Attribute<QString> m_beforeTarget;
    XFA_Attribute<QString> m_bookendLeader;
    XFA_Attribute<QString> m_bookendTrailer;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_overflowLeader;
    XFA_Attribute<QString> m_overflowTarget;
    XFA_Attribute<QString> m_overflowTrailer;
    XFA_Attribute<bool> m_startNew;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_break> XFA_break::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_break myClass;

    // load attributes
    parseAttribute(element, "after", myClass.m_after, "auto");
    parseAttribute(element, "afterTarget", myClass.m_afterTarget, "");
    parseAttribute(element, "before", myClass.m_before, "auto");
    parseAttribute(element, "beforeTarget", myClass.m_beforeTarget, "");
    parseAttribute(element, "bookendLeader", myClass.m_bookendLeader, "");
    parseAttribute(element, "bookendTrailer", myClass.m_bookendTrailer, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "overflowLeader", myClass.m_overflowLeader, "");
    parseAttribute(element, "overflowTarget", myClass.m_overflowTarget, "");
    parseAttribute(element, "overflowTrailer", myClass.m_overflowTrailer, "");
    parseAttribute(element, "startNew", myClass.m_startNew, "0");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_button : public XFA_BaseNode
{
public:

    HIGHLIGHT getHighlight() const {  return m_highlight.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_button> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<HIGHLIGHT> m_highlight;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_button> XFA_button::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_button myClass;

    // load attributes
    parseAttribute(element, "highlight", myClass.m_highlight, "inverted");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_calculate : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    OVERRIDE getOverride() const {  return m_override.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_message* getMessage() const {  return m_message.getValue(); }
    const XFA_script* getScript() const {  return m_script.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_calculate> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<OVERRIDE> m_override;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_message> m_message;
    XFA_Node<XFA_script> m_script;
};

std::optional<XFA_calculate> XFA_calculate::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_calculate myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "override", myClass.m_override, "disabled");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "message", myClass.m_message);
    parseItem(element, "script", myClass.m_script);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_color : public XFA_BaseNode
{
public:

    QString getCSpace() const {  return m_cSpace.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    QString getValue() const {  return m_value.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_color> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_cSpace;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<QString> m_value;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_color> XFA_color::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_color myClass;

    // load attributes
    parseAttribute(element, "cSpace", myClass.m_cSpace, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "value", myClass.m_value, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_contentArea : public XFA_BaseNode
{
public:

    XFA_Measurement getH() const {  return m_h.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    XFA_Measurement getW() const {  return m_w.getValueOrDefault(); }
    XFA_Measurement getX() const {  return m_x.getValueOrDefault(); }
    XFA_Measurement getY() const {  return m_y.getValueOrDefault(); }

    const XFA_desc* getDesc() const {  return m_desc.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_contentArea> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<XFA_Measurement> m_h;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<XFA_Measurement> m_w;
    XFA_Attribute<XFA_Measurement> m_x;
    XFA_Attribute<XFA_Measurement> m_y;

    /* subnodes */
    XFA_Node<XFA_desc> m_desc;
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_contentArea> XFA_contentArea::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_contentArea myClass;

    // load attributes
    parseAttribute(element, "h", myClass.m_h, "0in");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "w", myClass.m_w, "0in");
    parseAttribute(element, "x", myClass.m_x, "0in");
    parseAttribute(element, "y", myClass.m_y, "0in");

    // load items
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_corner : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    bool getInverted() const {  return m_inverted.getValueOrDefault(); }
    JOIN getJoin() const {  return m_join.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    XFA_Measurement getRadius() const {  return m_radius.getValueOrDefault(); }
    STROKE getStroke() const {  return m_stroke.getValueOrDefault(); }
    XFA_Measurement getThickness() const {  return m_thickness.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_color* getColor() const {  return m_color.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_corner> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<bool> m_inverted;
    XFA_Attribute<JOIN> m_join;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<XFA_Measurement> m_radius;
    XFA_Attribute<STROKE> m_stroke;
    XFA_Attribute<XFA_Measurement> m_thickness;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_color> m_color;
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_corner> XFA_corner::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_corner myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "inverted", myClass.m_inverted, "0");
    parseAttribute(element, "join", myClass.m_join, "square");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "radius", myClass.m_radius, "0in");
    parseAttribute(element, "stroke", myClass.m_stroke, "solid");
    parseAttribute(element, "thickness", myClass.m_thickness, "0.5pt");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "color", myClass.m_color);
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_defaultUi : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_defaultUi> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_defaultUi> XFA_defaultUi::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_defaultUi myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_edge : public XFA_BaseNode
{
public:

    CAP getCap() const {  return m_cap.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    STROKE getStroke() const {  return m_stroke.getValueOrDefault(); }
    XFA_Measurement getThickness() const {  return m_thickness.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_color* getColor() const {  return m_color.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_edge> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<CAP> m_cap;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<STROKE> m_stroke;
    XFA_Attribute<XFA_Measurement> m_thickness;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_color> m_color;
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_edge> XFA_edge::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_edge myClass;

    // load attributes
    parseAttribute(element, "cap", myClass.m_cap, "square");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "stroke", myClass.m_stroke, "solid");
    parseAttribute(element, "thickness", myClass.m_thickness, "0.5pt");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "color", myClass.m_color);
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_exObject : public XFA_BaseNode
{
public:

    QString getArchive() const {  return m_archive.getValueOrDefault(); }
    QString getClassId() const {  return m_classId.getValueOrDefault(); }
    QString getCodeBase() const {  return m_codeBase.getValueOrDefault(); }
    QString getCodeType() const {  return m_codeType.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const std::vector<XFA_Node<XFA_boolean>>& getBoolean() const {  return m_boolean; }
    const std::vector<XFA_Node<XFA_date>>& getDate() const {  return m_date; }
    const std::vector<XFA_Node<XFA_dateTime>>& getDateTime() const {  return m_dateTime; }
    const std::vector<XFA_Node<XFA_decimal>>& getDecimal() const {  return m_decimal; }
    const std::vector<XFA_Node<XFA_exData>>& getExData() const {  return m_exData; }
    const std::vector<XFA_Node<XFA_exObject>>& getExObject() const {  return m_exObject; }
    const std::vector<XFA_Node<XFA_float>>& getFloat() const {  return m_float; }
    const std::vector<XFA_Node<XFA_image>>& getImage() const {  return m_image; }
    const std::vector<XFA_Node<XFA_integer>>& getInteger() const {  return m_integer; }
    const std::vector<XFA_Node<XFA_text>>& getText() const {  return m_text; }
    const std::vector<XFA_Node<XFA_time>>& getTime() const {  return m_time; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_exObject> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_archive;
    XFA_Attribute<QString> m_classId;
    XFA_Attribute<QString> m_codeBase;
    XFA_Attribute<QString> m_codeType;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    std::vector<XFA_Node<XFA_boolean>> m_boolean;
    std::vector<XFA_Node<XFA_date>> m_date;
    std::vector<XFA_Node<XFA_dateTime>> m_dateTime;
    std::vector<XFA_Node<XFA_decimal>> m_decimal;
    std::vector<XFA_Node<XFA_exData>> m_exData;
    std::vector<XFA_Node<XFA_exObject>> m_exObject;
    std::vector<XFA_Node<XFA_float>> m_float;
    std::vector<XFA_Node<XFA_image>> m_image;
    std::vector<XFA_Node<XFA_integer>> m_integer;
    std::vector<XFA_Node<XFA_text>> m_text;
    std::vector<XFA_Node<XFA_time>> m_time;
};

std::optional<XFA_exObject> XFA_exObject::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_exObject myClass;

    // load attributes
    parseAttribute(element, "archive", myClass.m_archive, "");
    parseAttribute(element, "classId", myClass.m_classId, "");
    parseAttribute(element, "codeBase", myClass.m_codeBase, "");
    parseAttribute(element, "codeType", myClass.m_codeType, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "boolean", myClass.m_boolean);
    parseItem(element, "date", myClass.m_date);
    parseItem(element, "dateTime", myClass.m_dateTime);
    parseItem(element, "decimal", myClass.m_decimal);
    parseItem(element, "exData", myClass.m_exData);
    parseItem(element, "exObject", myClass.m_exObject);
    parseItem(element, "float", myClass.m_float);
    parseItem(element, "image", myClass.m_image);
    parseItem(element, "integer", myClass.m_integer);
    parseItem(element, "text", myClass.m_text);
    parseItem(element, "time", myClass.m_time);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_format : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_picture* getPicture() const {  return m_picture.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_format> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_picture> m_picture;
};

std::optional<XFA_format> XFA_format::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_format myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "picture", myClass.m_picture);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_items : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    QString getRef() const {  return m_ref.getValueOrDefault(); }
    bool getSave() const {  return m_save.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_boolean>>& getBoolean() const {  return m_boolean; }
    const std::vector<XFA_Node<XFA_date>>& getDate() const {  return m_date; }
    const std::vector<XFA_Node<XFA_dateTime>>& getDateTime() const {  return m_dateTime; }
    const std::vector<XFA_Node<XFA_decimal>>& getDecimal() const {  return m_decimal; }
    const std::vector<XFA_Node<XFA_exData>>& getExData() const {  return m_exData; }
    const std::vector<XFA_Node<XFA_float>>& getFloat() const {  return m_float; }
    const std::vector<XFA_Node<XFA_image>>& getImage() const {  return m_image; }
    const std::vector<XFA_Node<XFA_integer>>& getInteger() const {  return m_integer; }
    const std::vector<XFA_Node<XFA_text>>& getText() const {  return m_text; }
    const std::vector<XFA_Node<XFA_time>>& getTime() const {  return m_time; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_items> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<QString> m_ref;
    XFA_Attribute<bool> m_save;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_boolean>> m_boolean;
    std::vector<XFA_Node<XFA_date>> m_date;
    std::vector<XFA_Node<XFA_dateTime>> m_dateTime;
    std::vector<XFA_Node<XFA_decimal>> m_decimal;
    std::vector<XFA_Node<XFA_exData>> m_exData;
    std::vector<XFA_Node<XFA_float>> m_float;
    std::vector<XFA_Node<XFA_image>> m_image;
    std::vector<XFA_Node<XFA_integer>> m_integer;
    std::vector<XFA_Node<XFA_text>> m_text;
    std::vector<XFA_Node<XFA_time>> m_time;
};

std::optional<XFA_items> XFA_items::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_items myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "ref", myClass.m_ref, "");
    parseAttribute(element, "save", myClass.m_save, "0");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "boolean", myClass.m_boolean);
    parseItem(element, "date", myClass.m_date);
    parseItem(element, "dateTime", myClass.m_dateTime);
    parseItem(element, "decimal", myClass.m_decimal);
    parseItem(element, "exData", myClass.m_exData);
    parseItem(element, "float", myClass.m_float);
    parseItem(element, "image", myClass.m_image);
    parseItem(element, "integer", myClass.m_integer);
    parseItem(element, "text", myClass.m_text);
    parseItem(element, "time", myClass.m_time);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_keep : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    INTACT getIntact() const {  return m_intact.getValueOrDefault(); }
    NEXT getNext() const {  return m_next.getValueOrDefault(); }
    PREVIOUS getPrevious() const {  return m_previous.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_keep> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<INTACT> m_intact;
    XFA_Attribute<NEXT> m_next;
    XFA_Attribute<PREVIOUS> m_previous;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_keep> XFA_keep::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_keep myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "intact", myClass.m_intact, "none");
    parseAttribute(element, "next", myClass.m_next, "none");
    parseAttribute(element, "previous", myClass.m_previous, "none");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_line : public XFA_BaseNode
{
public:

    HAND getHand() const {  return m_hand.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    SLOPE getSlope() const {  return m_slope.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_edge* getEdge() const {  return m_edge.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_line> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<HAND> m_hand;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<SLOPE> m_slope;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_edge> m_edge;
};

std::optional<XFA_line> XFA_line::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_line myClass;

    // load attributes
    parseAttribute(element, "hand", myClass.m_hand, "even");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "slope", myClass.m_slope, "\\");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "edge", myClass.m_edge);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_linear : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE1 getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_color* getColor() const {  return m_color.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_linear> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE1> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_color> m_color;
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_linear> XFA_linear::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_linear myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "toRight");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "color", myClass.m_color);
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_manifest : public XFA_BaseNode
{
public:

    ACTION getAction() const {  return m_action.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const std::vector<XFA_Node<XFA_ref>>& getRef() const {  return m_ref; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_manifest> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<ACTION> m_action;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    std::vector<XFA_Node<XFA_ref>> m_ref;
};

std::optional<XFA_manifest> XFA_manifest::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_manifest myClass;

    // load attributes
    parseAttribute(element, "action", myClass.m_action, "include");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "ref", myClass.m_ref);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_margin : public XFA_BaseNode
{
public:

    XFA_Measurement getBottomInset() const {  return m_bottomInset.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    XFA_Measurement getLeftInset() const {  return m_leftInset.getValueOrDefault(); }
    XFA_Measurement getRightInset() const {  return m_rightInset.getValueOrDefault(); }
    XFA_Measurement getTopInset() const {  return m_topInset.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_margin> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<XFA_Measurement> m_bottomInset;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<XFA_Measurement> m_leftInset;
    XFA_Attribute<XFA_Measurement> m_rightInset;
    XFA_Attribute<XFA_Measurement> m_topInset;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_margin> XFA_margin::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_margin myClass;

    // load attributes
    parseAttribute(element, "bottomInset", myClass.m_bottomInset, "0in");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "leftInset", myClass.m_leftInset, "0in");
    parseAttribute(element, "rightInset", myClass.m_rightInset, "0in");
    parseAttribute(element, "topInset", myClass.m_topInset, "0in");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_occur : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFInteger getInitial() const {  return m_initial.getValueOrDefault(); }
    PDFInteger getMax() const {  return m_max.getValueOrDefault(); }
    PDFInteger getMin() const {  return m_min.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_occur> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFInteger> m_initial;
    XFA_Attribute<PDFInteger> m_max;
    XFA_Attribute<PDFInteger> m_min;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_occur> XFA_occur::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_occur myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "initial", myClass.m_initial, "1");
    parseAttribute(element, "max", myClass.m_max, "1");
    parseAttribute(element, "min", myClass.m_min, "1");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_pattern : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE2 getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_color* getColor() const {  return m_color.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_pattern> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE2> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_color> m_color;
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_pattern> XFA_pattern::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_pattern myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "crossHatch");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "color", myClass.m_color);
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_radial : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE3 getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_color* getColor() const {  return m_color.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_radial> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE3> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_color> m_color;
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_radial> XFA_radial::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_radial myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "toEdge");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "color", myClass.m_color);
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_solid : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_solid> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_solid> XFA_solid::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_solid myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_stipple : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFInteger getRate() const {  return m_rate.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_color* getColor() const {  return m_color.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_stipple> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFInteger> m_rate;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_color> m_color;
    XFA_Node<XFA_extras> m_extras;
};

std::optional<XFA_stipple> XFA_stipple::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_stipple myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "rate", myClass.m_rate, "50");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "color", myClass.m_color);
    parseItem(element, "extras", myClass.m_extras);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_fill : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_color* getColor() const {  return m_color.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_linear* getLinear() const {  return m_linear.getValue(); }
    const XFA_pattern* getPattern() const {  return m_pattern.getValue(); }
    const XFA_radial* getRadial() const {  return m_radial.getValue(); }
    const XFA_solid* getSolid() const {  return m_solid.getValue(); }
    const XFA_stipple* getStipple() const {  return m_stipple.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_fill> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_color> m_color;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_linear> m_linear;
    XFA_Node<XFA_pattern> m_pattern;
    XFA_Node<XFA_radial> m_radial;
    XFA_Node<XFA_solid> m_solid;
    XFA_Node<XFA_stipple> m_stipple;
};

std::optional<XFA_fill> XFA_fill::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_fill myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "color", myClass.m_color);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "linear", myClass.m_linear);
    parseItem(element, "pattern", myClass.m_pattern);
    parseItem(element, "radial", myClass.m_radial);
    parseItem(element, "solid", myClass.m_solid);
    parseItem(element, "stipple", myClass.m_stipple);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_arc : public XFA_BaseNode
{
public:

    bool getCircular() const {  return m_circular.getValueOrDefault(); }
    HAND getHand() const {  return m_hand.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFReal getStartAngle() const {  return m_startAngle.getValueOrDefault(); }
    PDFReal getSweepAngle() const {  return m_sweepAngle.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_edge* getEdge() const {  return m_edge.getValue(); }
    const XFA_fill* getFill() const {  return m_fill.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_arc> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<bool> m_circular;
    XFA_Attribute<HAND> m_hand;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFReal> m_startAngle;
    XFA_Attribute<PDFReal> m_sweepAngle;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_edge> m_edge;
    XFA_Node<XFA_fill> m_fill;
};

std::optional<XFA_arc> XFA_arc::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_arc myClass;

    // load attributes
    parseAttribute(element, "circular", myClass.m_circular, "0");
    parseAttribute(element, "hand", myClass.m_hand, "even");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "startAngle", myClass.m_startAngle, "0");
    parseAttribute(element, "sweepAngle", myClass.m_sweepAngle, "360");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "edge", myClass.m_edge);
    parseItem(element, "fill", myClass.m_fill);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_border : public XFA_BaseNode
{
public:

    BREAK getBreak() const {  return m_break.getValueOrDefault(); }
    HAND getHand() const {  return m_hand.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_corner>>& getCorner() const {  return m_corner; }
    const std::vector<XFA_Node<XFA_edge>>& getEdge() const {  return m_edge; }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_fill* getFill() const {  return m_fill.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_border> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<BREAK> m_break;
    XFA_Attribute<HAND> m_hand;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_corner>> m_corner;
    std::vector<XFA_Node<XFA_edge>> m_edge;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_fill> m_fill;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_border> XFA_border::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_border myClass;

    // load attributes
    parseAttribute(element, "break", myClass.m_break, "close");
    parseAttribute(element, "hand", myClass.m_hand, "even");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "corner", myClass.m_corner);
    parseItem(element, "edge", myClass.m_edge);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "fill", myClass.m_fill);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_checkButton : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    MARK getMark() const {  return m_mark.getValueOrDefault(); }
    SHAPE getShape() const {  return m_shape.getValueOrDefault(); }
    XFA_Measurement getSize() const {  return m_size.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_checkButton> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<MARK> m_mark;
    XFA_Attribute<SHAPE> m_shape;
    XFA_Attribute<XFA_Measurement> m_size;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_checkButton> XFA_checkButton::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_checkButton myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "mark", myClass.m_mark, "default");
    parseAttribute(element, "shape", myClass.m_shape, "square");
    parseAttribute(element, "size", myClass.m_size, "10pt");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_choiceList : public XFA_BaseNode
{
public:

    COMMITON getCommitOn() const {  return m_commitOn.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    OPEN getOpen() const {  return m_open.getValueOrDefault(); }
    bool getTextEntry() const {  return m_textEntry.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_choiceList> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<COMMITON> m_commitOn;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<OPEN> m_open;
    XFA_Attribute<bool> m_textEntry;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_choiceList> XFA_choiceList::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_choiceList myClass;

    // load attributes
    parseAttribute(element, "commitOn", myClass.m_commitOn, "select");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "open", myClass.m_open, "userControl");
    parseAttribute(element, "textEntry", myClass.m_textEntry, "0");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_dateTimeEdit : public XFA_BaseNode
{
public:

    HSCROLLPOLICY getHScrollPolicy() const {  return m_hScrollPolicy.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PICKER getPicker() const {  return m_picker.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_comb* getComb() const {  return m_comb.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_dateTimeEdit> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<HSCROLLPOLICY> m_hScrollPolicy;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PICKER> m_picker;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_comb> m_comb;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_dateTimeEdit> XFA_dateTimeEdit::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_dateTimeEdit myClass;

    // load attributes
    parseAttribute(element, "hScrollPolicy", myClass.m_hScrollPolicy, "auto");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "picker", myClass.m_picker, "host");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "comb", myClass.m_comb);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_font : public XFA_BaseNode
{
public:

    XFA_Measurement getBaselineShift() const {  return m_baselineShift.getValueOrDefault(); }
    QString getFontHorizontalScale() const {  return m_fontHorizontalScale.getValueOrDefault(); }
    QString getFontVerticalScale() const {  return m_fontVerticalScale.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    KERNINGMODE getKerningMode() const {  return m_kerningMode.getValueOrDefault(); }
    QString getLetterSpacing() const {  return m_letterSpacing.getValueOrDefault(); }
    LINETHROUGH getLineThrough() const {  return m_lineThrough.getValueOrDefault(); }
    LINETHROUGHPERIOD getLineThroughPeriod() const {  return m_lineThroughPeriod.getValueOrDefault(); }
    OVERLINE getOverline() const {  return m_overline.getValueOrDefault(); }
    OVERLINEPERIOD getOverlinePeriod() const {  return m_overlinePeriod.getValueOrDefault(); }
    POSTURE getPosture() const {  return m_posture.getValueOrDefault(); }
    XFA_Measurement getSize() const {  return m_size.getValueOrDefault(); }
    QString getTypeface() const {  return m_typeface.getValueOrDefault(); }
    UNDERLINE getUnderline() const {  return m_underline.getValueOrDefault(); }
    UNDERLINEPERIOD getUnderlinePeriod() const {  return m_underlinePeriod.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    WEIGHT getWeight() const {  return m_weight.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_fill* getFill() const {  return m_fill.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_font> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<XFA_Measurement> m_baselineShift;
    XFA_Attribute<QString> m_fontHorizontalScale;
    XFA_Attribute<QString> m_fontVerticalScale;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<KERNINGMODE> m_kerningMode;
    XFA_Attribute<QString> m_letterSpacing;
    XFA_Attribute<LINETHROUGH> m_lineThrough;
    XFA_Attribute<LINETHROUGHPERIOD> m_lineThroughPeriod;
    XFA_Attribute<OVERLINE> m_overline;
    XFA_Attribute<OVERLINEPERIOD> m_overlinePeriod;
    XFA_Attribute<POSTURE> m_posture;
    XFA_Attribute<XFA_Measurement> m_size;
    XFA_Attribute<QString> m_typeface;
    XFA_Attribute<UNDERLINE> m_underline;
    XFA_Attribute<UNDERLINEPERIOD> m_underlinePeriod;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<WEIGHT> m_weight;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_fill> m_fill;
};

std::optional<XFA_font> XFA_font::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_font myClass;

    // load attributes
    parseAttribute(element, "baselineShift", myClass.m_baselineShift, "0in");
    parseAttribute(element, "fontHorizontalScale", myClass.m_fontHorizontalScale, "");
    parseAttribute(element, "fontVerticalScale", myClass.m_fontVerticalScale, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "kerningMode", myClass.m_kerningMode, "none");
    parseAttribute(element, "letterSpacing", myClass.m_letterSpacing, "");
    parseAttribute(element, "lineThrough", myClass.m_lineThrough, "0");
    parseAttribute(element, "lineThroughPeriod", myClass.m_lineThroughPeriod, "all");
    parseAttribute(element, "overline", myClass.m_overline, "0");
    parseAttribute(element, "overlinePeriod", myClass.m_overlinePeriod, "all");
    parseAttribute(element, "posture", myClass.m_posture, "normal");
    parseAttribute(element, "size", myClass.m_size, "10pt");
    parseAttribute(element, "typeface", myClass.m_typeface, "");
    parseAttribute(element, "underline", myClass.m_underline, "0");
    parseAttribute(element, "underlinePeriod", myClass.m_underlinePeriod, "all");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "weight", myClass.m_weight, "normal");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "fill", myClass.m_fill);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_imageEdit : public XFA_BaseNode
{
public:

    DATA getData() const {  return m_data.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_imageEdit> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<DATA> m_data;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_imageEdit> XFA_imageEdit::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_imageEdit myClass;

    // load attributes
    parseAttribute(element, "data", myClass.m_data, "link");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_numericEdit : public XFA_BaseNode
{
public:

    HSCROLLPOLICY getHScrollPolicy() const {  return m_hScrollPolicy.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_comb* getComb() const {  return m_comb.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_numericEdit> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<HSCROLLPOLICY> m_hScrollPolicy;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_comb> m_comb;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_numericEdit> XFA_numericEdit::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_numericEdit myClass;

    // load attributes
    parseAttribute(element, "hScrollPolicy", myClass.m_hScrollPolicy, "auto");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "comb", myClass.m_comb);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_passwordEdit : public XFA_BaseNode
{
public:

    HSCROLLPOLICY getHScrollPolicy() const {  return m_hScrollPolicy.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getPasswordChar() const {  return m_passwordChar.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_passwordEdit> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<HSCROLLPOLICY> m_hScrollPolicy;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_passwordChar;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_passwordEdit> XFA_passwordEdit::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_passwordEdit myClass;

    // load attributes
    parseAttribute(element, "hScrollPolicy", myClass.m_hScrollPolicy, "auto");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "passwordChar", myClass.m_passwordChar, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_rectangle : public XFA_BaseNode
{
public:

    HAND getHand() const {  return m_hand.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_corner>>& getCorner() const {  return m_corner; }
    const std::vector<XFA_Node<XFA_edge>>& getEdge() const {  return m_edge; }
    const XFA_fill* getFill() const {  return m_fill.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_rectangle> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<HAND> m_hand;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_corner>> m_corner;
    std::vector<XFA_Node<XFA_edge>> m_edge;
    XFA_Node<XFA_fill> m_fill;
};

std::optional<XFA_rectangle> XFA_rectangle::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_rectangle myClass;

    // load attributes
    parseAttribute(element, "hand", myClass.m_hand, "even");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "corner", myClass.m_corner);
    parseItem(element, "edge", myClass.m_edge);
    parseItem(element, "fill", myClass.m_fill);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_textEdit : public XFA_BaseNode
{
public:

    bool getAllowRichText() const {  return m_allowRichText.getValueOrDefault(); }
    HSCROLLPOLICY getHScrollPolicy() const {  return m_hScrollPolicy.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    MULTILINE getMultiLine() const {  return m_multiLine.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    VSCROLLPOLICY getVScrollPolicy() const {  return m_vScrollPolicy.getValueOrDefault(); }

    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_comb* getComb() const {  return m_comb.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_textEdit> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<bool> m_allowRichText;
    XFA_Attribute<HSCROLLPOLICY> m_hScrollPolicy;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<MULTILINE> m_multiLine;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<VSCROLLPOLICY> m_vScrollPolicy;

    /* subnodes */
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_comb> m_comb;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_textEdit> XFA_textEdit::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_textEdit myClass;

    // load attributes
    parseAttribute(element, "allowRichText", myClass.m_allowRichText, "0");
    parseAttribute(element, "hScrollPolicy", myClass.m_hScrollPolicy, "auto");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "multiLine", myClass.m_multiLine, "1");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "vScrollPolicy", myClass.m_vScrollPolicy, "auto");

    // load items
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "comb", myClass.m_comb);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_timeStamp : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getServer() const {  return m_server.getValueOrDefault(); }
    TYPE getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_timeStamp> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_server;
    XFA_Attribute<TYPE> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
};

std::optional<XFA_timeStamp> XFA_timeStamp::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_timeStamp myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "server", myClass.m_server, "");
    parseAttribute(element, "type", myClass.m_type, "optional");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_filter : public XFA_BaseNode
{
public:

    QString getAddRevocationInfo() const {  return m_addRevocationInfo.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    QString getVersion() const {  return m_version.getValueOrDefault(); }

    const XFA_appearanceFilter* getAppearanceFilter() const {  return m_appearanceFilter.getValue(); }
    const XFA_certificates* getCertificates() const {  return m_certificates.getValue(); }
    const XFA_digestMethods* getDigestMethods() const {  return m_digestMethods.getValue(); }
    const XFA_encodings* getEncodings() const {  return m_encodings.getValue(); }
    const XFA_encryptionMethods* getEncryptionMethods() const {  return m_encryptionMethods.getValue(); }
    const XFA_handler* getHandler() const {  return m_handler.getValue(); }
    const XFA_lockDocument* getLockDocument() const {  return m_lockDocument.getValue(); }
    const XFA_mdp* getMdp() const {  return m_mdp.getValue(); }
    const XFA_reasons* getReasons() const {  return m_reasons.getValue(); }
    const XFA_timeStamp* getTimeStamp() const {  return m_timeStamp.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_filter> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_addRevocationInfo;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<QString> m_version;

    /* subnodes */
    XFA_Node<XFA_appearanceFilter> m_appearanceFilter;
    XFA_Node<XFA_certificates> m_certificates;
    XFA_Node<XFA_digestMethods> m_digestMethods;
    XFA_Node<XFA_encodings> m_encodings;
    XFA_Node<XFA_encryptionMethods> m_encryptionMethods;
    XFA_Node<XFA_handler> m_handler;
    XFA_Node<XFA_lockDocument> m_lockDocument;
    XFA_Node<XFA_mdp> m_mdp;
    XFA_Node<XFA_reasons> m_reasons;
    XFA_Node<XFA_timeStamp> m_timeStamp;
};

std::optional<XFA_filter> XFA_filter::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_filter myClass;

    // load attributes
    parseAttribute(element, "addRevocationInfo", myClass.m_addRevocationInfo, "");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "version", myClass.m_version, "");

    // load items
    parseItem(element, "appearanceFilter", myClass.m_appearanceFilter);
    parseItem(element, "certificates", myClass.m_certificates);
    parseItem(element, "digestMethods", myClass.m_digestMethods);
    parseItem(element, "encodings", myClass.m_encodings);
    parseItem(element, "encryptionMethods", myClass.m_encryptionMethods);
    parseItem(element, "handler", myClass.m_handler);
    parseItem(element, "lockDocument", myClass.m_lockDocument);
    parseItem(element, "mdp", myClass.m_mdp);
    parseItem(element, "reasons", myClass.m_reasons);
    parseItem(element, "timeStamp", myClass.m_timeStamp);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_encryptData : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    OPERATION getOperation() const {  return m_operation.getValueOrDefault(); }
    QString getTarget() const {  return m_target.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_filter* getFilter() const {  return m_filter.getValue(); }
    const XFA_manifest* getManifest() const {  return m_manifest.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_encryptData> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<OPERATION> m_operation;
    XFA_Attribute<QString> m_target;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_filter> m_filter;
    XFA_Node<XFA_manifest> m_manifest;
};

std::optional<XFA_encryptData> XFA_encryptData::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_encryptData myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "operation", myClass.m_operation, "encrypt");
    parseAttribute(element, "target", myClass.m_target, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "filter", myClass.m_filter);
    parseItem(element, "manifest", myClass.m_manifest);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_signData : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    OPERATION1 getOperation() const {  return m_operation.getValueOrDefault(); }
    QString getRef() const {  return m_ref.getValueOrDefault(); }
    QString getTarget() const {  return m_target.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_filter* getFilter() const {  return m_filter.getValue(); }
    const XFA_manifest* getManifest() const {  return m_manifest.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_signData> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<OPERATION1> m_operation;
    XFA_Attribute<QString> m_ref;
    XFA_Attribute<QString> m_target;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_filter> m_filter;
    XFA_Node<XFA_manifest> m_manifest;
};

std::optional<XFA_signData> XFA_signData::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_signData myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "operation", myClass.m_operation, "sign");
    parseAttribute(element, "ref", myClass.m_ref, "");
    parseAttribute(element, "target", myClass.m_target, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "filter", myClass.m_filter);
    parseItem(element, "manifest", myClass.m_manifest);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_signature : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    TYPE4 getType() const {  return m_type.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_filter* getFilter() const {  return m_filter.getValue(); }
    const XFA_manifest* getManifest() const {  return m_manifest.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_signature> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<TYPE4> m_type;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_filter> m_filter;
    XFA_Node<XFA_manifest> m_manifest;
    XFA_Node<XFA_margin> m_margin;
};

std::optional<XFA_signature> XFA_signature::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_signature myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "type", myClass.m_type, "PDF1.3");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "filter", myClass.m_filter);
    parseItem(element, "manifest", myClass.m_manifest);
    parseItem(element, "margin", myClass.m_margin);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_submit : public XFA_BaseNode
{
public:

    bool getEmbedPDF() const {  return m_embedPDF.getValueOrDefault(); }
    FORMAT getFormat() const {  return m_format.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getTarget() const {  return m_target.getValueOrDefault(); }
    QString getTextEncoding() const {  return m_textEncoding.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    QString getXdpContent() const {  return m_xdpContent.getValueOrDefault(); }

    const XFA_encrypt* getEncrypt() const {  return m_encrypt.getValue(); }
    const std::vector<XFA_Node<XFA_encryptData>>& getEncryptData() const {  return m_encryptData; }
    const std::vector<XFA_Node<XFA_signData>>& getSignData() const {  return m_signData; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_submit> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<bool> m_embedPDF;
    XFA_Attribute<FORMAT> m_format;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_target;
    XFA_Attribute<QString> m_textEncoding;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<QString> m_xdpContent;

    /* subnodes */
    XFA_Node<XFA_encrypt> m_encrypt;
    std::vector<XFA_Node<XFA_encryptData>> m_encryptData;
    std::vector<XFA_Node<XFA_signData>> m_signData;
};

std::optional<XFA_submit> XFA_submit::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_submit myClass;

    // load attributes
    parseAttribute(element, "embedPDF", myClass.m_embedPDF, "0");
    parseAttribute(element, "format", myClass.m_format, "xdp");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "target", myClass.m_target, "");
    parseAttribute(element, "textEncoding", myClass.m_textEncoding, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "xdpContent", myClass.m_xdpContent, "");

    // load items
    parseItem(element, "encrypt", myClass.m_encrypt);
    parseItem(element, "encryptData", myClass.m_encryptData);
    parseItem(element, "signData", myClass.m_signData);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_event : public XFA_BaseNode
{
public:

    ACTIVITY getActivity() const {  return m_activity.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    LISTEN getListen() const {  return m_listen.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getRef() const {  return m_ref.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_encryptData* getEncryptData() const {  return m_encryptData.getValue(); }
    const XFA_execute* getExecute() const {  return m_execute.getValue(); }
    const XFA_script* getScript() const {  return m_script.getValue(); }
    const XFA_signData* getSignData() const {  return m_signData.getValue(); }
    const XFA_submit* getSubmit() const {  return m_submit.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_event> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<ACTIVITY> m_activity;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<LISTEN> m_listen;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_ref;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_encryptData> m_encryptData;
    XFA_Node<XFA_execute> m_execute;
    XFA_Node<XFA_script> m_script;
    XFA_Node<XFA_signData> m_signData;
    XFA_Node<XFA_submit> m_submit;
};

std::optional<XFA_event> XFA_event::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_event myClass;

    // load attributes
    parseAttribute(element, "activity", myClass.m_activity, "click");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "listen", myClass.m_listen, "refOnly");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "ref", myClass.m_ref, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "encryptData", myClass.m_encryptData);
    parseItem(element, "execute", myClass.m_execute);
    parseItem(element, "script", myClass.m_script);
    parseItem(element, "signData", myClass.m_signData);
    parseItem(element, "submit", myClass.m_submit);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_toolTip : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getRid() const {  return m_rid.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }


    const QString* getNodeValue() const {  return m_nodeValue.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_toolTip> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_rid;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */

    XFA_Value<QString> m_nodeValue;
};

std::optional<XFA_toolTip> XFA_toolTip::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_toolTip myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "rid", myClass.m_rid, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items

    // load node value
    parseValue(element, myClass.m_nodeValue);

    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_assist : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getRole() const {  return m_role.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_speak* getSpeak() const {  return m_speak.getValue(); }
    const XFA_toolTip* getToolTip() const {  return m_toolTip.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_assist> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_role;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_speak> m_speak;
    XFA_Node<XFA_toolTip> m_toolTip;
};

std::optional<XFA_assist> XFA_assist::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_assist myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "role", myClass.m_role, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "speak", myClass.m_speak);
    parseItem(element, "toolTip", myClass.m_toolTip);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_traverse : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    OPERATION2 getOperation() const {  return m_operation.getValueOrDefault(); }
    QString getRef() const {  return m_ref.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_script* getScript() const {  return m_script.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_traverse> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<OPERATION2> m_operation;
    XFA_Attribute<QString> m_ref;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_script> m_script;
};

std::optional<XFA_traverse> XFA_traverse::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_traverse myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "operation", myClass.m_operation, "next");
    parseAttribute(element, "ref", myClass.m_ref, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "script", myClass.m_script);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_traversal : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const std::vector<XFA_Node<XFA_traverse>>& getTraverse() const {  return m_traverse; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_traversal> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    std::vector<XFA_Node<XFA_traverse>> m_traverse;
};

std::optional<XFA_traversal> XFA_traversal::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_traversal myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "traverse", myClass.m_traverse);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_ui : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_picture* getPicture() const {  return m_picture.getValue(); }
    const XFA_barcode* getBarcode() const {  return m_barcode.getValue(); }
    const XFA_button* getButton() const {  return m_button.getValue(); }
    const XFA_checkButton* getCheckButton() const {  return m_checkButton.getValue(); }
    const XFA_choiceList* getChoiceList() const {  return m_choiceList.getValue(); }
    const XFA_dateTimeEdit* getDateTimeEdit() const {  return m_dateTimeEdit.getValue(); }
    const XFA_defaultUi* getDefaultUi() const {  return m_defaultUi.getValue(); }
    const XFA_imageEdit* getImageEdit() const {  return m_imageEdit.getValue(); }
    const XFA_numericEdit* getNumericEdit() const {  return m_numericEdit.getValue(); }
    const XFA_passwordEdit* getPasswordEdit() const {  return m_passwordEdit.getValue(); }
    const XFA_signature* getSignature() const {  return m_signature.getValue(); }
    const XFA_textEdit* getTextEdit() const {  return m_textEdit.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_ui> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_picture> m_picture;
    XFA_Node<XFA_barcode> m_barcode;
    XFA_Node<XFA_button> m_button;
    XFA_Node<XFA_checkButton> m_checkButton;
    XFA_Node<XFA_choiceList> m_choiceList;
    XFA_Node<XFA_dateTimeEdit> m_dateTimeEdit;
    XFA_Node<XFA_defaultUi> m_defaultUi;
    XFA_Node<XFA_imageEdit> m_imageEdit;
    XFA_Node<XFA_numericEdit> m_numericEdit;
    XFA_Node<XFA_passwordEdit> m_passwordEdit;
    XFA_Node<XFA_signature> m_signature;
    XFA_Node<XFA_textEdit> m_textEdit;
};

std::optional<XFA_ui> XFA_ui::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_ui myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "picture", myClass.m_picture);
    parseItem(element, "barcode", myClass.m_barcode);
    parseItem(element, "button", myClass.m_button);
    parseItem(element, "checkButton", myClass.m_checkButton);
    parseItem(element, "choiceList", myClass.m_choiceList);
    parseItem(element, "dateTimeEdit", myClass.m_dateTimeEdit);
    parseItem(element, "defaultUi", myClass.m_defaultUi);
    parseItem(element, "imageEdit", myClass.m_imageEdit);
    parseItem(element, "numericEdit", myClass.m_numericEdit);
    parseItem(element, "passwordEdit", myClass.m_passwordEdit);
    parseItem(element, "signature", myClass.m_signature);
    parseItem(element, "textEdit", myClass.m_textEdit);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_validate : public XFA_BaseNode
{
public:

    FORMATTEST getFormatTest() const {  return m_formatTest.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    NULLTEST getNullTest() const {  return m_nullTest.getValueOrDefault(); }
    SCRIPTTEST getScriptTest() const {  return m_scriptTest.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_message* getMessage() const {  return m_message.getValue(); }
    const XFA_picture* getPicture() const {  return m_picture.getValue(); }
    const XFA_script* getScript() const {  return m_script.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_validate> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<FORMATTEST> m_formatTest;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<NULLTEST> m_nullTest;
    XFA_Attribute<SCRIPTTEST> m_scriptTest;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_message> m_message;
    XFA_Node<XFA_picture> m_picture;
    XFA_Node<XFA_script> m_script;
};

std::optional<XFA_validate> XFA_validate::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_validate myClass;

    // load attributes
    parseAttribute(element, "formatTest", myClass.m_formatTest, "warning");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "nullTest", myClass.m_nullTest, "disabled");
    parseAttribute(element, "scriptTest", myClass.m_scriptTest, "error");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "message", myClass.m_message);
    parseItem(element, "picture", myClass.m_picture);
    parseItem(element, "script", myClass.m_script);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_value : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    bool getOverride() const {  return m_override.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_arc* getArc() const {  return m_arc.getValue(); }
    const XFA_boolean* getBoolean() const {  return m_boolean.getValue(); }
    const XFA_date* getDate() const {  return m_date.getValue(); }
    const XFA_dateTime* getDateTime() const {  return m_dateTime.getValue(); }
    const XFA_decimal* getDecimal() const {  return m_decimal.getValue(); }
    const XFA_exData* getExData() const {  return m_exData.getValue(); }
    const XFA_float* getFloat() const {  return m_float.getValue(); }
    const XFA_image* getImage() const {  return m_image.getValue(); }
    const XFA_integer* getInteger() const {  return m_integer.getValue(); }
    const XFA_line* getLine() const {  return m_line.getValue(); }
    const XFA_rectangle* getRectangle() const {  return m_rectangle.getValue(); }
    const XFA_text* getText() const {  return m_text.getValue(); }
    const XFA_time* getTime() const {  return m_time.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_value> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<bool> m_override;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_arc> m_arc;
    XFA_Node<XFA_boolean> m_boolean;
    XFA_Node<XFA_date> m_date;
    XFA_Node<XFA_dateTime> m_dateTime;
    XFA_Node<XFA_decimal> m_decimal;
    XFA_Node<XFA_exData> m_exData;
    XFA_Node<XFA_float> m_float;
    XFA_Node<XFA_image> m_image;
    XFA_Node<XFA_integer> m_integer;
    XFA_Node<XFA_line> m_line;
    XFA_Node<XFA_rectangle> m_rectangle;
    XFA_Node<XFA_text> m_text;
    XFA_Node<XFA_time> m_time;
};

std::optional<XFA_value> XFA_value::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_value myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "override", myClass.m_override, "0");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "arc", myClass.m_arc);
    parseItem(element, "boolean", myClass.m_boolean);
    parseItem(element, "date", myClass.m_date);
    parseItem(element, "dateTime", myClass.m_dateTime);
    parseItem(element, "decimal", myClass.m_decimal);
    parseItem(element, "exData", myClass.m_exData);
    parseItem(element, "float", myClass.m_float);
    parseItem(element, "image", myClass.m_image);
    parseItem(element, "integer", myClass.m_integer);
    parseItem(element, "line", myClass.m_line);
    parseItem(element, "rectangle", myClass.m_rectangle);
    parseItem(element, "text", myClass.m_text);
    parseItem(element, "time", myClass.m_time);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_caption : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    PLACEMENT getPlacement() const {  return m_placement.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    XFA_Measurement getReserve() const {  return m_reserve.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_font* getFont() const {  return m_font.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }
    const XFA_para* getPara() const {  return m_para.getValue(); }
    const XFA_value* getValue() const {  return m_value.getValue(); }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_caption> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PLACEMENT> m_placement;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<XFA_Measurement> m_reserve;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_font> m_font;
    XFA_Node<XFA_margin> m_margin;
    XFA_Node<XFA_para> m_para;
    XFA_Node<XFA_value> m_value;
};

std::optional<XFA_caption> XFA_caption::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_caption myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "placement", myClass.m_placement, "left");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "reserve", myClass.m_reserve, "-1");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "font", myClass.m_font);
    parseItem(element, "margin", myClass.m_margin);
    parseItem(element, "para", myClass.m_para);
    parseItem(element, "value", myClass.m_value);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_draw : public XFA_BaseNode
{
public:

    ANCHORTYPE getAnchorType() const {  return m_anchorType.getValueOrDefault(); }
    PDFInteger getColSpan() const {  return m_colSpan.getValueOrDefault(); }
    XFA_Measurement getH() const {  return m_h.getValueOrDefault(); }
    HALIGN getHAlign() const {  return m_hAlign.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getLocale() const {  return m_locale.getValueOrDefault(); }
    XFA_Measurement getMaxH() const {  return m_maxH.getValueOrDefault(); }
    XFA_Measurement getMaxW() const {  return m_maxW.getValueOrDefault(); }
    XFA_Measurement getMinH() const {  return m_minH.getValueOrDefault(); }
    XFA_Measurement getMinW() const {  return m_minW.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    PDFReal getRotate() const {  return m_rotate.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    XFA_Measurement getW() const {  return m_w.getValueOrDefault(); }
    XFA_Measurement getX() const {  return m_x.getValueOrDefault(); }
    XFA_Measurement getY() const {  return m_y.getValueOrDefault(); }

    const XFA_assist* getAssist() const {  return m_assist.getValue(); }
    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_caption* getCaption() const {  return m_caption.getValue(); }
    const XFA_desc* getDesc() const {  return m_desc.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_font* getFont() const {  return m_font.getValue(); }
    const XFA_keep* getKeep() const {  return m_keep.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }
    const XFA_para* getPara() const {  return m_para.getValue(); }
    const XFA_traversal* getTraversal() const {  return m_traversal.getValue(); }
    const XFA_ui* getUi() const {  return m_ui.getValue(); }
    const XFA_value* getValue() const {  return m_value.getValue(); }
    const std::vector<XFA_Node<XFA_setProperty>>& getSetProperty() const {  return m_setProperty; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_draw> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<ANCHORTYPE> m_anchorType;
    XFA_Attribute<PDFInteger> m_colSpan;
    XFA_Attribute<XFA_Measurement> m_h;
    XFA_Attribute<HALIGN> m_hAlign;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_locale;
    XFA_Attribute<XFA_Measurement> m_maxH;
    XFA_Attribute<XFA_Measurement> m_maxW;
    XFA_Attribute<XFA_Measurement> m_minH;
    XFA_Attribute<XFA_Measurement> m_minW;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<PDFReal> m_rotate;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<XFA_Measurement> m_w;
    XFA_Attribute<XFA_Measurement> m_x;
    XFA_Attribute<XFA_Measurement> m_y;

    /* subnodes */
    XFA_Node<XFA_assist> m_assist;
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_caption> m_caption;
    XFA_Node<XFA_desc> m_desc;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_font> m_font;
    XFA_Node<XFA_keep> m_keep;
    XFA_Node<XFA_margin> m_margin;
    XFA_Node<XFA_para> m_para;
    XFA_Node<XFA_traversal> m_traversal;
    XFA_Node<XFA_ui> m_ui;
    XFA_Node<XFA_value> m_value;
    std::vector<XFA_Node<XFA_setProperty>> m_setProperty;
};

std::optional<XFA_draw> XFA_draw::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_draw myClass;

    // load attributes
    parseAttribute(element, "anchorType", myClass.m_anchorType, "topleft");
    parseAttribute(element, "colSpan", myClass.m_colSpan, "1");
    parseAttribute(element, "h", myClass.m_h, "0in");
    parseAttribute(element, "hAlign", myClass.m_hAlign, "left");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "locale", myClass.m_locale, "");
    parseAttribute(element, "maxH", myClass.m_maxH, "0in");
    parseAttribute(element, "maxW", myClass.m_maxW, "0in");
    parseAttribute(element, "minH", myClass.m_minH, "0in");
    parseAttribute(element, "minW", myClass.m_minW, "0in");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "rotate", myClass.m_rotate, "0");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "w", myClass.m_w, "0in");
    parseAttribute(element, "x", myClass.m_x, "0in");
    parseAttribute(element, "y", myClass.m_y, "0in");

    // load items
    parseItem(element, "assist", myClass.m_assist);
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "caption", myClass.m_caption);
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "font", myClass.m_font);
    parseItem(element, "keep", myClass.m_keep);
    parseItem(element, "margin", myClass.m_margin);
    parseItem(element, "para", myClass.m_para);
    parseItem(element, "traversal", myClass.m_traversal);
    parseItem(element, "ui", myClass.m_ui);
    parseItem(element, "value", myClass.m_value);
    parseItem(element, "setProperty", myClass.m_setProperty);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_field : public XFA_BaseNode
{
public:

    ACCESS getAccess() const {  return m_access.getValueOrDefault(); }
    QString getAccessKey() const {  return m_accessKey.getValueOrDefault(); }
    ANCHORTYPE getAnchorType() const {  return m_anchorType.getValueOrDefault(); }
    PDFInteger getColSpan() const {  return m_colSpan.getValueOrDefault(); }
    XFA_Measurement getH() const {  return m_h.getValueOrDefault(); }
    HALIGN getHAlign() const {  return m_hAlign.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getLocale() const {  return m_locale.getValueOrDefault(); }
    XFA_Measurement getMaxH() const {  return m_maxH.getValueOrDefault(); }
    XFA_Measurement getMaxW() const {  return m_maxW.getValueOrDefault(); }
    XFA_Measurement getMinH() const {  return m_minH.getValueOrDefault(); }
    XFA_Measurement getMinW() const {  return m_minW.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    PDFReal getRotate() const {  return m_rotate.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    XFA_Measurement getW() const {  return m_w.getValueOrDefault(); }
    XFA_Measurement getX() const {  return m_x.getValueOrDefault(); }
    XFA_Measurement getY() const {  return m_y.getValueOrDefault(); }

    const XFA_assist* getAssist() const {  return m_assist.getValue(); }
    const XFA_bind* getBind() const {  return m_bind.getValue(); }
    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_calculate* getCalculate() const {  return m_calculate.getValue(); }
    const XFA_caption* getCaption() const {  return m_caption.getValue(); }
    const XFA_desc* getDesc() const {  return m_desc.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_font* getFont() const {  return m_font.getValue(); }
    const XFA_format* getFormat() const {  return m_format.getValue(); }
    const std::vector<XFA_Node<XFA_items>>& getItems() const {  return m_items; }
    const XFA_keep* getKeep() const {  return m_keep.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }
    const XFA_para* getPara() const {  return m_para.getValue(); }
    const XFA_traversal* getTraversal() const {  return m_traversal.getValue(); }
    const XFA_ui* getUi() const {  return m_ui.getValue(); }
    const XFA_validate* getValidate() const {  return m_validate.getValue(); }
    const XFA_value* getValue() const {  return m_value.getValue(); }
    const std::vector<XFA_Node<XFA_bindItems>>& getBindItems() const {  return m_bindItems; }
    const std::vector<XFA_Node<XFA_connect>>& getConnect() const {  return m_connect; }
    const std::vector<XFA_Node<XFA_event>>& getEvent() const {  return m_event; }
    const std::vector<XFA_Node<XFA_setProperty>>& getSetProperty() const {  return m_setProperty; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_field> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<ACCESS> m_access;
    XFA_Attribute<QString> m_accessKey;
    XFA_Attribute<ANCHORTYPE> m_anchorType;
    XFA_Attribute<PDFInteger> m_colSpan;
    XFA_Attribute<XFA_Measurement> m_h;
    XFA_Attribute<HALIGN> m_hAlign;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_locale;
    XFA_Attribute<XFA_Measurement> m_maxH;
    XFA_Attribute<XFA_Measurement> m_maxW;
    XFA_Attribute<XFA_Measurement> m_minH;
    XFA_Attribute<XFA_Measurement> m_minW;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<PDFReal> m_rotate;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<XFA_Measurement> m_w;
    XFA_Attribute<XFA_Measurement> m_x;
    XFA_Attribute<XFA_Measurement> m_y;

    /* subnodes */
    XFA_Node<XFA_assist> m_assist;
    XFA_Node<XFA_bind> m_bind;
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_calculate> m_calculate;
    XFA_Node<XFA_caption> m_caption;
    XFA_Node<XFA_desc> m_desc;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_font> m_font;
    XFA_Node<XFA_format> m_format;
    std::vector<XFA_Node<XFA_items>> m_items;
    XFA_Node<XFA_keep> m_keep;
    XFA_Node<XFA_margin> m_margin;
    XFA_Node<XFA_para> m_para;
    XFA_Node<XFA_traversal> m_traversal;
    XFA_Node<XFA_ui> m_ui;
    XFA_Node<XFA_validate> m_validate;
    XFA_Node<XFA_value> m_value;
    std::vector<XFA_Node<XFA_bindItems>> m_bindItems;
    std::vector<XFA_Node<XFA_connect>> m_connect;
    std::vector<XFA_Node<XFA_event>> m_event;
    std::vector<XFA_Node<XFA_setProperty>> m_setProperty;
};

std::optional<XFA_field> XFA_field::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_field myClass;

    // load attributes
    parseAttribute(element, "access", myClass.m_access, "open");
    parseAttribute(element, "accessKey", myClass.m_accessKey, "");
    parseAttribute(element, "anchorType", myClass.m_anchorType, "topleft");
    parseAttribute(element, "colSpan", myClass.m_colSpan, "1");
    parseAttribute(element, "h", myClass.m_h, "0in");
    parseAttribute(element, "hAlign", myClass.m_hAlign, "left");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "locale", myClass.m_locale, "");
    parseAttribute(element, "maxH", myClass.m_maxH, "0in");
    parseAttribute(element, "maxW", myClass.m_maxW, "0in");
    parseAttribute(element, "minH", myClass.m_minH, "0in");
    parseAttribute(element, "minW", myClass.m_minW, "0in");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "rotate", myClass.m_rotate, "0");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "w", myClass.m_w, "0in");
    parseAttribute(element, "x", myClass.m_x, "0in");
    parseAttribute(element, "y", myClass.m_y, "0in");

    // load items
    parseItem(element, "assist", myClass.m_assist);
    parseItem(element, "bind", myClass.m_bind);
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "calculate", myClass.m_calculate);
    parseItem(element, "caption", myClass.m_caption);
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "font", myClass.m_font);
    parseItem(element, "format", myClass.m_format);
    parseItem(element, "items", myClass.m_items);
    parseItem(element, "keep", myClass.m_keep);
    parseItem(element, "margin", myClass.m_margin);
    parseItem(element, "para", myClass.m_para);
    parseItem(element, "traversal", myClass.m_traversal);
    parseItem(element, "ui", myClass.m_ui);
    parseItem(element, "validate", myClass.m_validate);
    parseItem(element, "value", myClass.m_value);
    parseItem(element, "bindItems", myClass.m_bindItems);
    parseItem(element, "connect", myClass.m_connect);
    parseItem(element, "event", myClass.m_event);
    parseItem(element, "setProperty", myClass.m_setProperty);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_exclGroup : public XFA_BaseNode
{
public:

    ACCESS getAccess() const {  return m_access.getValueOrDefault(); }
    QString getAccessKey() const {  return m_accessKey.getValueOrDefault(); }
    ANCHORTYPE getAnchorType() const {  return m_anchorType.getValueOrDefault(); }
    PDFInteger getColSpan() const {  return m_colSpan.getValueOrDefault(); }
    XFA_Measurement getH() const {  return m_h.getValueOrDefault(); }
    HALIGN getHAlign() const {  return m_hAlign.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    LAYOUT getLayout() const {  return m_layout.getValueOrDefault(); }
    XFA_Measurement getMaxH() const {  return m_maxH.getValueOrDefault(); }
    XFA_Measurement getMaxW() const {  return m_maxW.getValueOrDefault(); }
    XFA_Measurement getMinH() const {  return m_minH.getValueOrDefault(); }
    XFA_Measurement getMinW() const {  return m_minW.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    XFA_Measurement getW() const {  return m_w.getValueOrDefault(); }
    XFA_Measurement getX() const {  return m_x.getValueOrDefault(); }
    XFA_Measurement getY() const {  return m_y.getValueOrDefault(); }

    const XFA_assist* getAssist() const {  return m_assist.getValue(); }
    const XFA_bind* getBind() const {  return m_bind.getValue(); }
    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_calculate* getCalculate() const {  return m_calculate.getValue(); }
    const XFA_caption* getCaption() const {  return m_caption.getValue(); }
    const XFA_desc* getDesc() const {  return m_desc.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }
    const XFA_para* getPara() const {  return m_para.getValue(); }
    const XFA_traversal* getTraversal() const {  return m_traversal.getValue(); }
    const XFA_validate* getValidate() const {  return m_validate.getValue(); }
    const std::vector<XFA_Node<XFA_connect>>& getConnect() const {  return m_connect; }
    const std::vector<XFA_Node<XFA_event>>& getEvent() const {  return m_event; }
    const std::vector<XFA_Node<XFA_field>>& getField() const {  return m_field; }
    const std::vector<XFA_Node<XFA_setProperty>>& getSetProperty() const {  return m_setProperty; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_exclGroup> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<ACCESS> m_access;
    XFA_Attribute<QString> m_accessKey;
    XFA_Attribute<ANCHORTYPE> m_anchorType;
    XFA_Attribute<PDFInteger> m_colSpan;
    XFA_Attribute<XFA_Measurement> m_h;
    XFA_Attribute<HALIGN> m_hAlign;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<LAYOUT> m_layout;
    XFA_Attribute<XFA_Measurement> m_maxH;
    XFA_Attribute<XFA_Measurement> m_maxW;
    XFA_Attribute<XFA_Measurement> m_minH;
    XFA_Attribute<XFA_Measurement> m_minW;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<XFA_Measurement> m_w;
    XFA_Attribute<XFA_Measurement> m_x;
    XFA_Attribute<XFA_Measurement> m_y;

    /* subnodes */
    XFA_Node<XFA_assist> m_assist;
    XFA_Node<XFA_bind> m_bind;
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_calculate> m_calculate;
    XFA_Node<XFA_caption> m_caption;
    XFA_Node<XFA_desc> m_desc;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_margin> m_margin;
    XFA_Node<XFA_para> m_para;
    XFA_Node<XFA_traversal> m_traversal;
    XFA_Node<XFA_validate> m_validate;
    std::vector<XFA_Node<XFA_connect>> m_connect;
    std::vector<XFA_Node<XFA_event>> m_event;
    std::vector<XFA_Node<XFA_field>> m_field;
    std::vector<XFA_Node<XFA_setProperty>> m_setProperty;
};

std::optional<XFA_exclGroup> XFA_exclGroup::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_exclGroup myClass;

    // load attributes
    parseAttribute(element, "access", myClass.m_access, "open");
    parseAttribute(element, "accessKey", myClass.m_accessKey, "");
    parseAttribute(element, "anchorType", myClass.m_anchorType, "topleft");
    parseAttribute(element, "colSpan", myClass.m_colSpan, "1");
    parseAttribute(element, "h", myClass.m_h, "0in");
    parseAttribute(element, "hAlign", myClass.m_hAlign, "left");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "layout", myClass.m_layout, "position");
    parseAttribute(element, "maxH", myClass.m_maxH, "0in");
    parseAttribute(element, "maxW", myClass.m_maxW, "0in");
    parseAttribute(element, "minH", myClass.m_minH, "0in");
    parseAttribute(element, "minW", myClass.m_minW, "0in");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "w", myClass.m_w, "0in");
    parseAttribute(element, "x", myClass.m_x, "0in");
    parseAttribute(element, "y", myClass.m_y, "0in");

    // load items
    parseItem(element, "assist", myClass.m_assist);
    parseItem(element, "bind", myClass.m_bind);
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "calculate", myClass.m_calculate);
    parseItem(element, "caption", myClass.m_caption);
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "margin", myClass.m_margin);
    parseItem(element, "para", myClass.m_para);
    parseItem(element, "traversal", myClass.m_traversal);
    parseItem(element, "validate", myClass.m_validate);
    parseItem(element, "connect", myClass.m_connect);
    parseItem(element, "event", myClass.m_event);
    parseItem(element, "field", myClass.m_field);
    parseItem(element, "setProperty", myClass.m_setProperty);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_variables : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const std::vector<XFA_Node<XFA_boolean>>& getBoolean() const {  return m_boolean; }
    const std::vector<XFA_Node<XFA_date>>& getDate() const {  return m_date; }
    const std::vector<XFA_Node<XFA_dateTime>>& getDateTime() const {  return m_dateTime; }
    const std::vector<XFA_Node<XFA_decimal>>& getDecimal() const {  return m_decimal; }
    const std::vector<XFA_Node<XFA_exData>>& getExData() const {  return m_exData; }
    const std::vector<XFA_Node<XFA_float>>& getFloat() const {  return m_float; }
    const std::vector<XFA_Node<XFA_image>>& getImage() const {  return m_image; }
    const std::vector<XFA_Node<XFA_integer>>& getInteger() const {  return m_integer; }
    const std::vector<XFA_Node<XFA_manifest>>& getManifest() const {  return m_manifest; }
    const std::vector<XFA_Node<XFA_script>>& getScript() const {  return m_script; }
    const std::vector<XFA_Node<XFA_text>>& getText() const {  return m_text; }
    const std::vector<XFA_Node<XFA_time>>& getTime() const {  return m_time; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_variables> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    std::vector<XFA_Node<XFA_boolean>> m_boolean;
    std::vector<XFA_Node<XFA_date>> m_date;
    std::vector<XFA_Node<XFA_dateTime>> m_dateTime;
    std::vector<XFA_Node<XFA_decimal>> m_decimal;
    std::vector<XFA_Node<XFA_exData>> m_exData;
    std::vector<XFA_Node<XFA_float>> m_float;
    std::vector<XFA_Node<XFA_image>> m_image;
    std::vector<XFA_Node<XFA_integer>> m_integer;
    std::vector<XFA_Node<XFA_manifest>> m_manifest;
    std::vector<XFA_Node<XFA_script>> m_script;
    std::vector<XFA_Node<XFA_text>> m_text;
    std::vector<XFA_Node<XFA_time>> m_time;
};

std::optional<XFA_variables> XFA_variables::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_variables myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "boolean", myClass.m_boolean);
    parseItem(element, "date", myClass.m_date);
    parseItem(element, "dateTime", myClass.m_dateTime);
    parseItem(element, "decimal", myClass.m_decimal);
    parseItem(element, "exData", myClass.m_exData);
    parseItem(element, "float", myClass.m_float);
    parseItem(element, "image", myClass.m_image);
    parseItem(element, "integer", myClass.m_integer);
    parseItem(element, "manifest", myClass.m_manifest);
    parseItem(element, "script", myClass.m_script);
    parseItem(element, "text", myClass.m_text);
    parseItem(element, "time", myClass.m_time);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_area : public XFA_BaseNode
{
public:

    PDFInteger getColSpan() const {  return m_colSpan.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    XFA_Measurement getX() const {  return m_x.getValueOrDefault(); }
    XFA_Measurement getY() const {  return m_y.getValueOrDefault(); }

    const XFA_desc* getDesc() const {  return m_desc.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const std::vector<XFA_Node<XFA_area>>& getArea() const {  return m_area; }
    const std::vector<XFA_Node<XFA_draw>>& getDraw() const {  return m_draw; }
    const std::vector<XFA_Node<XFA_exObject>>& getExObject() const {  return m_exObject; }
    const std::vector<XFA_Node<XFA_exclGroup>>& getExclGroup() const {  return m_exclGroup; }
    const std::vector<XFA_Node<XFA_field>>& getField() const {  return m_field; }
    const std::vector<XFA_Node<XFA_subform>>& getSubform() const {  return m_subform; }
    const std::vector<XFA_Node<XFA_subformSet>>& getSubformSet() const {  return m_subformSet; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_area> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<PDFInteger> m_colSpan;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<XFA_Measurement> m_x;
    XFA_Attribute<XFA_Measurement> m_y;

    /* subnodes */
    XFA_Node<XFA_desc> m_desc;
    XFA_Node<XFA_extras> m_extras;
    std::vector<XFA_Node<XFA_area>> m_area;
    std::vector<XFA_Node<XFA_draw>> m_draw;
    std::vector<XFA_Node<XFA_exObject>> m_exObject;
    std::vector<XFA_Node<XFA_exclGroup>> m_exclGroup;
    std::vector<XFA_Node<XFA_field>> m_field;
    std::vector<XFA_Node<XFA_subform>> m_subform;
    std::vector<XFA_Node<XFA_subformSet>> m_subformSet;
};

std::optional<XFA_area> XFA_area::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_area myClass;

    // load attributes
    parseAttribute(element, "colSpan", myClass.m_colSpan, "1");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "x", myClass.m_x, "0in");
    parseAttribute(element, "y", myClass.m_y, "0in");

    // load items
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "area", myClass.m_area);
    parseItem(element, "draw", myClass.m_draw);
    parseItem(element, "exObject", myClass.m_exObject);
    parseItem(element, "exclGroup", myClass.m_exclGroup);
    parseItem(element, "field", myClass.m_field);
    parseItem(element, "subform", myClass.m_subform);
    parseItem(element, "subformSet", myClass.m_subformSet);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_pageArea : public XFA_BaseNode
{
public:

    BLANKORNOTBLANK getBlankOrNotBlank() const {  return m_blankOrNotBlank.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    PDFInteger getInitialNumber() const {  return m_initialNumber.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    PDFInteger getNumbered() const {  return m_numbered.getValueOrDefault(); }
    ODDOREVEN getOddOrEven() const {  return m_oddOrEven.getValueOrDefault(); }
    PAGEPOSITION getPagePosition() const {  return m_pagePosition.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_desc* getDesc() const {  return m_desc.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_medium* getMedium() const {  return m_medium.getValue(); }
    const XFA_occur* getOccur() const {  return m_occur.getValue(); }
    const std::vector<XFA_Node<XFA_area>>& getArea() const {  return m_area; }
    const std::vector<XFA_Node<XFA_contentArea>>& getContentArea() const {  return m_contentArea; }
    const std::vector<XFA_Node<XFA_draw>>& getDraw() const {  return m_draw; }
    const std::vector<XFA_Node<XFA_exclGroup>>& getExclGroup() const {  return m_exclGroup; }
    const std::vector<XFA_Node<XFA_field>>& getField() const {  return m_field; }
    const std::vector<XFA_Node<XFA_subform>>& getSubform() const {  return m_subform; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_pageArea> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<BLANKORNOTBLANK> m_blankOrNotBlank;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<PDFInteger> m_initialNumber;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<PDFInteger> m_numbered;
    XFA_Attribute<ODDOREVEN> m_oddOrEven;
    XFA_Attribute<PAGEPOSITION> m_pagePosition;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_desc> m_desc;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_medium> m_medium;
    XFA_Node<XFA_occur> m_occur;
    std::vector<XFA_Node<XFA_area>> m_area;
    std::vector<XFA_Node<XFA_contentArea>> m_contentArea;
    std::vector<XFA_Node<XFA_draw>> m_draw;
    std::vector<XFA_Node<XFA_exclGroup>> m_exclGroup;
    std::vector<XFA_Node<XFA_field>> m_field;
    std::vector<XFA_Node<XFA_subform>> m_subform;
};

std::optional<XFA_pageArea> XFA_pageArea::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_pageArea myClass;

    // load attributes
    parseAttribute(element, "blankOrNotBlank", myClass.m_blankOrNotBlank, "any");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "initialNumber", myClass.m_initialNumber, "1");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "numbered", myClass.m_numbered, "1");
    parseAttribute(element, "oddOrEven", myClass.m_oddOrEven, "any");
    parseAttribute(element, "pagePosition", myClass.m_pagePosition, "any");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "medium", myClass.m_medium);
    parseItem(element, "occur", myClass.m_occur);
    parseItem(element, "area", myClass.m_area);
    parseItem(element, "contentArea", myClass.m_contentArea);
    parseItem(element, "draw", myClass.m_draw);
    parseItem(element, "exclGroup", myClass.m_exclGroup);
    parseItem(element, "field", myClass.m_field);
    parseItem(element, "subform", myClass.m_subform);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_pageSet : public XFA_BaseNode
{
public:

    DUPLEXIMPOSITION getDuplexImposition() const {  return m_duplexImposition.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    RELATION getRelation() const {  return m_relation.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_occur* getOccur() const {  return m_occur.getValue(); }
    const std::vector<XFA_Node<XFA_pageArea>>& getPageArea() const {  return m_pageArea; }
    const std::vector<XFA_Node<XFA_pageSet>>& getPageSet() const {  return m_pageSet; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_pageSet> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<DUPLEXIMPOSITION> m_duplexImposition;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<RELATION> m_relation;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_occur> m_occur;
    std::vector<XFA_Node<XFA_pageArea>> m_pageArea;
    std::vector<XFA_Node<XFA_pageSet>> m_pageSet;
};

std::optional<XFA_pageSet> XFA_pageSet::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_pageSet myClass;

    // load attributes
    parseAttribute(element, "duplexImposition", myClass.m_duplexImposition, "longEdge");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "relation", myClass.m_relation, "orderedOccurrence");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "occur", myClass.m_occur);
    parseItem(element, "pageArea", myClass.m_pageArea);
    parseItem(element, "pageSet", myClass.m_pageSet);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_proto : public XFA_BaseNode
{
public:


    const std::vector<XFA_Node<XFA_appearanceFilter>>& getAppearanceFilter() const {  return m_appearanceFilter; }
    const std::vector<XFA_Node<XFA_arc>>& getArc() const {  return m_arc; }
    const std::vector<XFA_Node<XFA_area>>& getArea() const {  return m_area; }
    const std::vector<XFA_Node<XFA_assist>>& getAssist() const {  return m_assist; }
    const std::vector<XFA_Node<XFA_barcode>>& getBarcode() const {  return m_barcode; }
    const std::vector<XFA_Node<XFA_bindItems>>& getBindItems() const {  return m_bindItems; }
    const std::vector<XFA_Node<XFA_bookend>>& getBookend() const {  return m_bookend; }
    const std::vector<XFA_Node<XFA_boolean>>& getBoolean() const {  return m_boolean; }
    const std::vector<XFA_Node<XFA_border>>& getBorder() const {  return m_border; }
    const std::vector<XFA_Node<XFA_break>>& getBreak() const {  return m_break; }
    const std::vector<XFA_Node<XFA_breakAfter>>& getBreakAfter() const {  return m_breakAfter; }
    const std::vector<XFA_Node<XFA_breakBefore>>& getBreakBefore() const {  return m_breakBefore; }
    const std::vector<XFA_Node<XFA_button>>& getButton() const {  return m_button; }
    const std::vector<XFA_Node<XFA_calculate>>& getCalculate() const {  return m_calculate; }
    const std::vector<XFA_Node<XFA_caption>>& getCaption() const {  return m_caption; }
    const std::vector<XFA_Node<XFA_certificate>>& getCertificate() const {  return m_certificate; }
    const std::vector<XFA_Node<XFA_certificates>>& getCertificates() const {  return m_certificates; }
    const std::vector<XFA_Node<XFA_checkButton>>& getCheckButton() const {  return m_checkButton; }
    const std::vector<XFA_Node<XFA_choiceList>>& getChoiceList() const {  return m_choiceList; }
    const std::vector<XFA_Node<XFA_color>>& getColor() const {  return m_color; }
    const std::vector<XFA_Node<XFA_comb>>& getComb() const {  return m_comb; }
    const std::vector<XFA_Node<XFA_connect>>& getConnect() const {  return m_connect; }
    const std::vector<XFA_Node<XFA_contentArea>>& getContentArea() const {  return m_contentArea; }
    const std::vector<XFA_Node<XFA_corner>>& getCorner() const {  return m_corner; }
    const std::vector<XFA_Node<XFA_date>>& getDate() const {  return m_date; }
    const std::vector<XFA_Node<XFA_dateTime>>& getDateTime() const {  return m_dateTime; }
    const std::vector<XFA_Node<XFA_dateTimeEdit>>& getDateTimeEdit() const {  return m_dateTimeEdit; }
    const std::vector<XFA_Node<XFA_decimal>>& getDecimal() const {  return m_decimal; }
    const std::vector<XFA_Node<XFA_defaultUi>>& getDefaultUi() const {  return m_defaultUi; }
    const std::vector<XFA_Node<XFA_desc>>& getDesc() const {  return m_desc; }
    const std::vector<XFA_Node<XFA_digestMethod>>& getDigestMethod() const {  return m_digestMethod; }
    const std::vector<XFA_Node<XFA_digestMethods>>& getDigestMethods() const {  return m_digestMethods; }
    const std::vector<XFA_Node<XFA_draw>>& getDraw() const {  return m_draw; }
    const std::vector<XFA_Node<XFA_edge>>& getEdge() const {  return m_edge; }
    const std::vector<XFA_Node<XFA_encoding>>& getEncoding() const {  return m_encoding; }
    const std::vector<XFA_Node<XFA_encodings>>& getEncodings() const {  return m_encodings; }
    const std::vector<XFA_Node<XFA_encrypt>>& getEncrypt() const {  return m_encrypt; }
    const std::vector<XFA_Node<XFA_encryptData>>& getEncryptData() const {  return m_encryptData; }
    const std::vector<XFA_Node<XFA_encryption>>& getEncryption() const {  return m_encryption; }
    const std::vector<XFA_Node<XFA_encryptionMethod>>& getEncryptionMethod() const {  return m_encryptionMethod; }
    const std::vector<XFA_Node<XFA_encryptionMethods>>& getEncryptionMethods() const {  return m_encryptionMethods; }
    const std::vector<XFA_Node<XFA_event>>& getEvent() const {  return m_event; }
    const std::vector<XFA_Node<XFA_exData>>& getExData() const {  return m_exData; }
    const std::vector<XFA_Node<XFA_exObject>>& getExObject() const {  return m_exObject; }
    const std::vector<XFA_Node<XFA_exclGroup>>& getExclGroup() const {  return m_exclGroup; }
    const std::vector<XFA_Node<XFA_execute>>& getExecute() const {  return m_execute; }
    const std::vector<XFA_Node<XFA_extras>>& getExtras() const {  return m_extras; }
    const std::vector<XFA_Node<XFA_field>>& getField() const {  return m_field; }
    const std::vector<XFA_Node<XFA_fill>>& getFill() const {  return m_fill; }
    const std::vector<XFA_Node<XFA_filter>>& getFilter() const {  return m_filter; }
    const std::vector<XFA_Node<XFA_float>>& getFloat() const {  return m_float; }
    const std::vector<XFA_Node<XFA_font>>& getFont() const {  return m_font; }
    const std::vector<XFA_Node<XFA_format>>& getFormat() const {  return m_format; }
    const std::vector<XFA_Node<XFA_handler>>& getHandler() const {  return m_handler; }
    const std::vector<XFA_Node<XFA_hyphenation>>& getHyphenation() const {  return m_hyphenation; }
    const std::vector<XFA_Node<XFA_image>>& getImage() const {  return m_image; }
    const std::vector<XFA_Node<XFA_imageEdit>>& getImageEdit() const {  return m_imageEdit; }
    const std::vector<XFA_Node<XFA_integer>>& getInteger() const {  return m_integer; }
    const std::vector<XFA_Node<XFA_issuers>>& getIssuers() const {  return m_issuers; }
    const std::vector<XFA_Node<XFA_items>>& getItems() const {  return m_items; }
    const std::vector<XFA_Node<XFA_keep>>& getKeep() const {  return m_keep; }
    const std::vector<XFA_Node<XFA_keyUsage>>& getKeyUsage() const {  return m_keyUsage; }
    const std::vector<XFA_Node<XFA_line>>& getLine() const {  return m_line; }
    const std::vector<XFA_Node<XFA_linear>>& getLinear() const {  return m_linear; }
    const std::vector<XFA_Node<XFA_lockDocument>>& getLockDocument() const {  return m_lockDocument; }
    const std::vector<XFA_Node<XFA_manifest>>& getManifest() const {  return m_manifest; }
    const std::vector<XFA_Node<XFA_margin>>& getMargin() const {  return m_margin; }
    const std::vector<XFA_Node<XFA_mdp>>& getMdp() const {  return m_mdp; }
    const std::vector<XFA_Node<XFA_medium>>& getMedium() const {  return m_medium; }
    const std::vector<XFA_Node<XFA_message>>& getMessage() const {  return m_message; }
    const std::vector<XFA_Node<XFA_numericEdit>>& getNumericEdit() const {  return m_numericEdit; }
    const std::vector<XFA_Node<XFA_occur>>& getOccur() const {  return m_occur; }
    const std::vector<XFA_Node<XFA_oid>>& getOid() const {  return m_oid; }
    const std::vector<XFA_Node<XFA_oids>>& getOids() const {  return m_oids; }
    const std::vector<XFA_Node<XFA_overflow>>& getOverflow() const {  return m_overflow; }
    const std::vector<XFA_Node<XFA_pageArea>>& getPageArea() const {  return m_pageArea; }
    const std::vector<XFA_Node<XFA_pageSet>>& getPageSet() const {  return m_pageSet; }
    const std::vector<XFA_Node<XFA_para>>& getPara() const {  return m_para; }
    const std::vector<XFA_Node<XFA_passwordEdit>>& getPasswordEdit() const {  return m_passwordEdit; }
    const std::vector<XFA_Node<XFA_pattern>>& getPattern() const {  return m_pattern; }
    const std::vector<XFA_Node<XFA_picture>>& getPicture() const {  return m_picture; }
    const std::vector<XFA_Node<XFA_radial>>& getRadial() const {  return m_radial; }
    const std::vector<XFA_Node<XFA_reason>>& getReason() const {  return m_reason; }
    const std::vector<XFA_Node<XFA_reasons>>& getReasons() const {  return m_reasons; }
    const std::vector<XFA_Node<XFA_rectangle>>& getRectangle() const {  return m_rectangle; }
    const std::vector<XFA_Node<XFA_ref>>& getRef() const {  return m_ref; }
    const std::vector<XFA_Node<XFA_script>>& getScript() const {  return m_script; }
    const std::vector<XFA_Node<XFA_setProperty>>& getSetProperty() const {  return m_setProperty; }
    const std::vector<XFA_Node<XFA_signData>>& getSignData() const {  return m_signData; }
    const std::vector<XFA_Node<XFA_signature>>& getSignature() const {  return m_signature; }
    const std::vector<XFA_Node<XFA_signing>>& getSigning() const {  return m_signing; }
    const std::vector<XFA_Node<XFA_solid>>& getSolid() const {  return m_solid; }
    const std::vector<XFA_Node<XFA_speak>>& getSpeak() const {  return m_speak; }
    const std::vector<XFA_Node<XFA_stipple>>& getStipple() const {  return m_stipple; }
    const std::vector<XFA_Node<XFA_subform>>& getSubform() const {  return m_subform; }
    const std::vector<XFA_Node<XFA_subformSet>>& getSubformSet() const {  return m_subformSet; }
    const std::vector<XFA_Node<XFA_subjectDN>>& getSubjectDN() const {  return m_subjectDN; }
    const std::vector<XFA_Node<XFA_subjectDNs>>& getSubjectDNs() const {  return m_subjectDNs; }
    const std::vector<XFA_Node<XFA_submit>>& getSubmit() const {  return m_submit; }
    const std::vector<XFA_Node<XFA_text>>& getText() const {  return m_text; }
    const std::vector<XFA_Node<XFA_textEdit>>& getTextEdit() const {  return m_textEdit; }
    const std::vector<XFA_Node<XFA_time>>& getTime() const {  return m_time; }
    const std::vector<XFA_Node<XFA_timeStamp>>& getTimeStamp() const {  return m_timeStamp; }
    const std::vector<XFA_Node<XFA_toolTip>>& getToolTip() const {  return m_toolTip; }
    const std::vector<XFA_Node<XFA_traversal>>& getTraversal() const {  return m_traversal; }
    const std::vector<XFA_Node<XFA_traverse>>& getTraverse() const {  return m_traverse; }
    const std::vector<XFA_Node<XFA_ui>>& getUi() const {  return m_ui; }
    const std::vector<XFA_Node<XFA_validate>>& getValidate() const {  return m_validate; }
    const std::vector<XFA_Node<XFA_value>>& getValue() const {  return m_value; }
    const std::vector<XFA_Node<XFA_variables>>& getVariables() const {  return m_variables; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_proto> parse(const QDomElement& element);

private:
    /* properties */

    /* subnodes */
    std::vector<XFA_Node<XFA_appearanceFilter>> m_appearanceFilter;
    std::vector<XFA_Node<XFA_arc>> m_arc;
    std::vector<XFA_Node<XFA_area>> m_area;
    std::vector<XFA_Node<XFA_assist>> m_assist;
    std::vector<XFA_Node<XFA_barcode>> m_barcode;
    std::vector<XFA_Node<XFA_bindItems>> m_bindItems;
    std::vector<XFA_Node<XFA_bookend>> m_bookend;
    std::vector<XFA_Node<XFA_boolean>> m_boolean;
    std::vector<XFA_Node<XFA_border>> m_border;
    std::vector<XFA_Node<XFA_break>> m_break;
    std::vector<XFA_Node<XFA_breakAfter>> m_breakAfter;
    std::vector<XFA_Node<XFA_breakBefore>> m_breakBefore;
    std::vector<XFA_Node<XFA_button>> m_button;
    std::vector<XFA_Node<XFA_calculate>> m_calculate;
    std::vector<XFA_Node<XFA_caption>> m_caption;
    std::vector<XFA_Node<XFA_certificate>> m_certificate;
    std::vector<XFA_Node<XFA_certificates>> m_certificates;
    std::vector<XFA_Node<XFA_checkButton>> m_checkButton;
    std::vector<XFA_Node<XFA_choiceList>> m_choiceList;
    std::vector<XFA_Node<XFA_color>> m_color;
    std::vector<XFA_Node<XFA_comb>> m_comb;
    std::vector<XFA_Node<XFA_connect>> m_connect;
    std::vector<XFA_Node<XFA_contentArea>> m_contentArea;
    std::vector<XFA_Node<XFA_corner>> m_corner;
    std::vector<XFA_Node<XFA_date>> m_date;
    std::vector<XFA_Node<XFA_dateTime>> m_dateTime;
    std::vector<XFA_Node<XFA_dateTimeEdit>> m_dateTimeEdit;
    std::vector<XFA_Node<XFA_decimal>> m_decimal;
    std::vector<XFA_Node<XFA_defaultUi>> m_defaultUi;
    std::vector<XFA_Node<XFA_desc>> m_desc;
    std::vector<XFA_Node<XFA_digestMethod>> m_digestMethod;
    std::vector<XFA_Node<XFA_digestMethods>> m_digestMethods;
    std::vector<XFA_Node<XFA_draw>> m_draw;
    std::vector<XFA_Node<XFA_edge>> m_edge;
    std::vector<XFA_Node<XFA_encoding>> m_encoding;
    std::vector<XFA_Node<XFA_encodings>> m_encodings;
    std::vector<XFA_Node<XFA_encrypt>> m_encrypt;
    std::vector<XFA_Node<XFA_encryptData>> m_encryptData;
    std::vector<XFA_Node<XFA_encryption>> m_encryption;
    std::vector<XFA_Node<XFA_encryptionMethod>> m_encryptionMethod;
    std::vector<XFA_Node<XFA_encryptionMethods>> m_encryptionMethods;
    std::vector<XFA_Node<XFA_event>> m_event;
    std::vector<XFA_Node<XFA_exData>> m_exData;
    std::vector<XFA_Node<XFA_exObject>> m_exObject;
    std::vector<XFA_Node<XFA_exclGroup>> m_exclGroup;
    std::vector<XFA_Node<XFA_execute>> m_execute;
    std::vector<XFA_Node<XFA_extras>> m_extras;
    std::vector<XFA_Node<XFA_field>> m_field;
    std::vector<XFA_Node<XFA_fill>> m_fill;
    std::vector<XFA_Node<XFA_filter>> m_filter;
    std::vector<XFA_Node<XFA_float>> m_float;
    std::vector<XFA_Node<XFA_font>> m_font;
    std::vector<XFA_Node<XFA_format>> m_format;
    std::vector<XFA_Node<XFA_handler>> m_handler;
    std::vector<XFA_Node<XFA_hyphenation>> m_hyphenation;
    std::vector<XFA_Node<XFA_image>> m_image;
    std::vector<XFA_Node<XFA_imageEdit>> m_imageEdit;
    std::vector<XFA_Node<XFA_integer>> m_integer;
    std::vector<XFA_Node<XFA_issuers>> m_issuers;
    std::vector<XFA_Node<XFA_items>> m_items;
    std::vector<XFA_Node<XFA_keep>> m_keep;
    std::vector<XFA_Node<XFA_keyUsage>> m_keyUsage;
    std::vector<XFA_Node<XFA_line>> m_line;
    std::vector<XFA_Node<XFA_linear>> m_linear;
    std::vector<XFA_Node<XFA_lockDocument>> m_lockDocument;
    std::vector<XFA_Node<XFA_manifest>> m_manifest;
    std::vector<XFA_Node<XFA_margin>> m_margin;
    std::vector<XFA_Node<XFA_mdp>> m_mdp;
    std::vector<XFA_Node<XFA_medium>> m_medium;
    std::vector<XFA_Node<XFA_message>> m_message;
    std::vector<XFA_Node<XFA_numericEdit>> m_numericEdit;
    std::vector<XFA_Node<XFA_occur>> m_occur;
    std::vector<XFA_Node<XFA_oid>> m_oid;
    std::vector<XFA_Node<XFA_oids>> m_oids;
    std::vector<XFA_Node<XFA_overflow>> m_overflow;
    std::vector<XFA_Node<XFA_pageArea>> m_pageArea;
    std::vector<XFA_Node<XFA_pageSet>> m_pageSet;
    std::vector<XFA_Node<XFA_para>> m_para;
    std::vector<XFA_Node<XFA_passwordEdit>> m_passwordEdit;
    std::vector<XFA_Node<XFA_pattern>> m_pattern;
    std::vector<XFA_Node<XFA_picture>> m_picture;
    std::vector<XFA_Node<XFA_radial>> m_radial;
    std::vector<XFA_Node<XFA_reason>> m_reason;
    std::vector<XFA_Node<XFA_reasons>> m_reasons;
    std::vector<XFA_Node<XFA_rectangle>> m_rectangle;
    std::vector<XFA_Node<XFA_ref>> m_ref;
    std::vector<XFA_Node<XFA_script>> m_script;
    std::vector<XFA_Node<XFA_setProperty>> m_setProperty;
    std::vector<XFA_Node<XFA_signData>> m_signData;
    std::vector<XFA_Node<XFA_signature>> m_signature;
    std::vector<XFA_Node<XFA_signing>> m_signing;
    std::vector<XFA_Node<XFA_solid>> m_solid;
    std::vector<XFA_Node<XFA_speak>> m_speak;
    std::vector<XFA_Node<XFA_stipple>> m_stipple;
    std::vector<XFA_Node<XFA_subform>> m_subform;
    std::vector<XFA_Node<XFA_subformSet>> m_subformSet;
    std::vector<XFA_Node<XFA_subjectDN>> m_subjectDN;
    std::vector<XFA_Node<XFA_subjectDNs>> m_subjectDNs;
    std::vector<XFA_Node<XFA_submit>> m_submit;
    std::vector<XFA_Node<XFA_text>> m_text;
    std::vector<XFA_Node<XFA_textEdit>> m_textEdit;
    std::vector<XFA_Node<XFA_time>> m_time;
    std::vector<XFA_Node<XFA_timeStamp>> m_timeStamp;
    std::vector<XFA_Node<XFA_toolTip>> m_toolTip;
    std::vector<XFA_Node<XFA_traversal>> m_traversal;
    std::vector<XFA_Node<XFA_traverse>> m_traverse;
    std::vector<XFA_Node<XFA_ui>> m_ui;
    std::vector<XFA_Node<XFA_validate>> m_validate;
    std::vector<XFA_Node<XFA_value>> m_value;
    std::vector<XFA_Node<XFA_variables>> m_variables;
};

std::optional<XFA_proto> XFA_proto::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_proto myClass;

    // load attributes

    // load items
    parseItem(element, "appearanceFilter", myClass.m_appearanceFilter);
    parseItem(element, "arc", myClass.m_arc);
    parseItem(element, "area", myClass.m_area);
    parseItem(element, "assist", myClass.m_assist);
    parseItem(element, "barcode", myClass.m_barcode);
    parseItem(element, "bindItems", myClass.m_bindItems);
    parseItem(element, "bookend", myClass.m_bookend);
    parseItem(element, "boolean", myClass.m_boolean);
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "break", myClass.m_break);
    parseItem(element, "breakAfter", myClass.m_breakAfter);
    parseItem(element, "breakBefore", myClass.m_breakBefore);
    parseItem(element, "button", myClass.m_button);
    parseItem(element, "calculate", myClass.m_calculate);
    parseItem(element, "caption", myClass.m_caption);
    parseItem(element, "certificate", myClass.m_certificate);
    parseItem(element, "certificates", myClass.m_certificates);
    parseItem(element, "checkButton", myClass.m_checkButton);
    parseItem(element, "choiceList", myClass.m_choiceList);
    parseItem(element, "color", myClass.m_color);
    parseItem(element, "comb", myClass.m_comb);
    parseItem(element, "connect", myClass.m_connect);
    parseItem(element, "contentArea", myClass.m_contentArea);
    parseItem(element, "corner", myClass.m_corner);
    parseItem(element, "date", myClass.m_date);
    parseItem(element, "dateTime", myClass.m_dateTime);
    parseItem(element, "dateTimeEdit", myClass.m_dateTimeEdit);
    parseItem(element, "decimal", myClass.m_decimal);
    parseItem(element, "defaultUi", myClass.m_defaultUi);
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "digestMethod", myClass.m_digestMethod);
    parseItem(element, "digestMethods", myClass.m_digestMethods);
    parseItem(element, "draw", myClass.m_draw);
    parseItem(element, "edge", myClass.m_edge);
    parseItem(element, "encoding", myClass.m_encoding);
    parseItem(element, "encodings", myClass.m_encodings);
    parseItem(element, "encrypt", myClass.m_encrypt);
    parseItem(element, "encryptData", myClass.m_encryptData);
    parseItem(element, "encryption", myClass.m_encryption);
    parseItem(element, "encryptionMethod", myClass.m_encryptionMethod);
    parseItem(element, "encryptionMethods", myClass.m_encryptionMethods);
    parseItem(element, "event", myClass.m_event);
    parseItem(element, "exData", myClass.m_exData);
    parseItem(element, "exObject", myClass.m_exObject);
    parseItem(element, "exclGroup", myClass.m_exclGroup);
    parseItem(element, "execute", myClass.m_execute);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "field", myClass.m_field);
    parseItem(element, "fill", myClass.m_fill);
    parseItem(element, "filter", myClass.m_filter);
    parseItem(element, "float", myClass.m_float);
    parseItem(element, "font", myClass.m_font);
    parseItem(element, "format", myClass.m_format);
    parseItem(element, "handler", myClass.m_handler);
    parseItem(element, "hyphenation", myClass.m_hyphenation);
    parseItem(element, "image", myClass.m_image);
    parseItem(element, "imageEdit", myClass.m_imageEdit);
    parseItem(element, "integer", myClass.m_integer);
    parseItem(element, "issuers", myClass.m_issuers);
    parseItem(element, "items", myClass.m_items);
    parseItem(element, "keep", myClass.m_keep);
    parseItem(element, "keyUsage", myClass.m_keyUsage);
    parseItem(element, "line", myClass.m_line);
    parseItem(element, "linear", myClass.m_linear);
    parseItem(element, "lockDocument", myClass.m_lockDocument);
    parseItem(element, "manifest", myClass.m_manifest);
    parseItem(element, "margin", myClass.m_margin);
    parseItem(element, "mdp", myClass.m_mdp);
    parseItem(element, "medium", myClass.m_medium);
    parseItem(element, "message", myClass.m_message);
    parseItem(element, "numericEdit", myClass.m_numericEdit);
    parseItem(element, "occur", myClass.m_occur);
    parseItem(element, "oid", myClass.m_oid);
    parseItem(element, "oids", myClass.m_oids);
    parseItem(element, "overflow", myClass.m_overflow);
    parseItem(element, "pageArea", myClass.m_pageArea);
    parseItem(element, "pageSet", myClass.m_pageSet);
    parseItem(element, "para", myClass.m_para);
    parseItem(element, "passwordEdit", myClass.m_passwordEdit);
    parseItem(element, "pattern", myClass.m_pattern);
    parseItem(element, "picture", myClass.m_picture);
    parseItem(element, "radial", myClass.m_radial);
    parseItem(element, "reason", myClass.m_reason);
    parseItem(element, "reasons", myClass.m_reasons);
    parseItem(element, "rectangle", myClass.m_rectangle);
    parseItem(element, "ref", myClass.m_ref);
    parseItem(element, "script", myClass.m_script);
    parseItem(element, "setProperty", myClass.m_setProperty);
    parseItem(element, "signData", myClass.m_signData);
    parseItem(element, "signature", myClass.m_signature);
    parseItem(element, "signing", myClass.m_signing);
    parseItem(element, "solid", myClass.m_solid);
    parseItem(element, "speak", myClass.m_speak);
    parseItem(element, "stipple", myClass.m_stipple);
    parseItem(element, "subform", myClass.m_subform);
    parseItem(element, "subformSet", myClass.m_subformSet);
    parseItem(element, "subjectDN", myClass.m_subjectDN);
    parseItem(element, "subjectDNs", myClass.m_subjectDNs);
    parseItem(element, "submit", myClass.m_submit);
    parseItem(element, "text", myClass.m_text);
    parseItem(element, "textEdit", myClass.m_textEdit);
    parseItem(element, "time", myClass.m_time);
    parseItem(element, "timeStamp", myClass.m_timeStamp);
    parseItem(element, "toolTip", myClass.m_toolTip);
    parseItem(element, "traversal", myClass.m_traversal);
    parseItem(element, "traverse", myClass.m_traverse);
    parseItem(element, "ui", myClass.m_ui);
    parseItem(element, "validate", myClass.m_validate);
    parseItem(element, "value", myClass.m_value);
    parseItem(element, "variables", myClass.m_variables);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_subform : public XFA_BaseNode
{
public:

    ACCESS getAccess() const {  return m_access.getValueOrDefault(); }
    bool getAllowMacro() const {  return m_allowMacro.getValueOrDefault(); }
    ANCHORTYPE getAnchorType() const {  return m_anchorType.getValueOrDefault(); }
    PDFInteger getColSpan() const {  return m_colSpan.getValueOrDefault(); }
    QString getColumnWidths() const {  return m_columnWidths.getValueOrDefault(); }
    XFA_Measurement getH() const {  return m_h.getValueOrDefault(); }
    HALIGN getHAlign() const {  return m_hAlign.getValueOrDefault(); }
    QString getId() const {  return m_id.getValueOrDefault(); }
    LAYOUT getLayout() const {  return m_layout.getValueOrDefault(); }
    QString getLocale() const {  return m_locale.getValueOrDefault(); }
    XFA_Measurement getMaxH() const {  return m_maxH.getValueOrDefault(); }
    XFA_Measurement getMaxW() const {  return m_maxW.getValueOrDefault(); }
    MERGEMODE getMergeMode() const {  return m_mergeMode.getValueOrDefault(); }
    XFA_Measurement getMinH() const {  return m_minH.getValueOrDefault(); }
    XFA_Measurement getMinW() const {  return m_minW.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    PRESENCE getPresence() const {  return m_presence.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    RESTORESTATE getRestoreState() const {  return m_restoreState.getValueOrDefault(); }
    SCOPE getScope() const {  return m_scope.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }
    XFA_Measurement getW() const {  return m_w.getValueOrDefault(); }
    XFA_Measurement getX() const {  return m_x.getValueOrDefault(); }
    XFA_Measurement getY() const {  return m_y.getValueOrDefault(); }

    const XFA_assist* getAssist() const {  return m_assist.getValue(); }
    const XFA_bind* getBind() const {  return m_bind.getValue(); }
    const XFA_bookend* getBookend() const {  return m_bookend.getValue(); }
    const XFA_border* getBorder() const {  return m_border.getValue(); }
    const XFA_break* getBreak() const {  return m_break.getValue(); }
    const XFA_calculate* getCalculate() const {  return m_calculate.getValue(); }
    const XFA_desc* getDesc() const {  return m_desc.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_keep* getKeep() const {  return m_keep.getValue(); }
    const XFA_margin* getMargin() const {  return m_margin.getValue(); }
    const XFA_occur* getOccur() const {  return m_occur.getValue(); }
    const XFA_overflow* getOverflow() const {  return m_overflow.getValue(); }
    const XFA_pageSet* getPageSet() const {  return m_pageSet.getValue(); }
    const XFA_para* getPara() const {  return m_para.getValue(); }
    const XFA_traversal* getTraversal() const {  return m_traversal.getValue(); }
    const XFA_validate* getValidate() const {  return m_validate.getValue(); }
    const XFA_variables* getVariables() const {  return m_variables.getValue(); }
    const std::vector<XFA_Node<XFA_area>>& getArea() const {  return m_area; }
    const std::vector<XFA_Node<XFA_breakAfter>>& getBreakAfter() const {  return m_breakAfter; }
    const std::vector<XFA_Node<XFA_breakBefore>>& getBreakBefore() const {  return m_breakBefore; }
    const std::vector<XFA_Node<XFA_connect>>& getConnect() const {  return m_connect; }
    const std::vector<XFA_Node<XFA_draw>>& getDraw() const {  return m_draw; }
    const std::vector<XFA_Node<XFA_event>>& getEvent() const {  return m_event; }
    const std::vector<XFA_Node<XFA_exObject>>& getExObject() const {  return m_exObject; }
    const std::vector<XFA_Node<XFA_exclGroup>>& getExclGroup() const {  return m_exclGroup; }
    const std::vector<XFA_Node<XFA_field>>& getField() const {  return m_field; }
    const std::vector<XFA_Node<XFA_proto>>& getProto() const {  return m_proto; }
    const std::vector<XFA_Node<XFA_setProperty>>& getSetProperty() const {  return m_setProperty; }
    const std::vector<XFA_Node<XFA_subform>>& getSubform() const {  return m_subform; }
    const std::vector<XFA_Node<XFA_subformSet>>& getSubformSet() const {  return m_subformSet; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_subform> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<ACCESS> m_access;
    XFA_Attribute<bool> m_allowMacro;
    XFA_Attribute<ANCHORTYPE> m_anchorType;
    XFA_Attribute<PDFInteger> m_colSpan;
    XFA_Attribute<QString> m_columnWidths;
    XFA_Attribute<XFA_Measurement> m_h;
    XFA_Attribute<HALIGN> m_hAlign;
    XFA_Attribute<QString> m_id;
    XFA_Attribute<LAYOUT> m_layout;
    XFA_Attribute<QString> m_locale;
    XFA_Attribute<XFA_Measurement> m_maxH;
    XFA_Attribute<XFA_Measurement> m_maxW;
    XFA_Attribute<MERGEMODE> m_mergeMode;
    XFA_Attribute<XFA_Measurement> m_minH;
    XFA_Attribute<XFA_Measurement> m_minW;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<PRESENCE> m_presence;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<RESTORESTATE> m_restoreState;
    XFA_Attribute<SCOPE> m_scope;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;
    XFA_Attribute<XFA_Measurement> m_w;
    XFA_Attribute<XFA_Measurement> m_x;
    XFA_Attribute<XFA_Measurement> m_y;

    /* subnodes */
    XFA_Node<XFA_assist> m_assist;
    XFA_Node<XFA_bind> m_bind;
    XFA_Node<XFA_bookend> m_bookend;
    XFA_Node<XFA_border> m_border;
    XFA_Node<XFA_break> m_break;
    XFA_Node<XFA_calculate> m_calculate;
    XFA_Node<XFA_desc> m_desc;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_keep> m_keep;
    XFA_Node<XFA_margin> m_margin;
    XFA_Node<XFA_occur> m_occur;
    XFA_Node<XFA_overflow> m_overflow;
    XFA_Node<XFA_pageSet> m_pageSet;
    XFA_Node<XFA_para> m_para;
    XFA_Node<XFA_traversal> m_traversal;
    XFA_Node<XFA_validate> m_validate;
    XFA_Node<XFA_variables> m_variables;
    std::vector<XFA_Node<XFA_area>> m_area;
    std::vector<XFA_Node<XFA_breakAfter>> m_breakAfter;
    std::vector<XFA_Node<XFA_breakBefore>> m_breakBefore;
    std::vector<XFA_Node<XFA_connect>> m_connect;
    std::vector<XFA_Node<XFA_draw>> m_draw;
    std::vector<XFA_Node<XFA_event>> m_event;
    std::vector<XFA_Node<XFA_exObject>> m_exObject;
    std::vector<XFA_Node<XFA_exclGroup>> m_exclGroup;
    std::vector<XFA_Node<XFA_field>> m_field;
    std::vector<XFA_Node<XFA_proto>> m_proto;
    std::vector<XFA_Node<XFA_setProperty>> m_setProperty;
    std::vector<XFA_Node<XFA_subform>> m_subform;
    std::vector<XFA_Node<XFA_subformSet>> m_subformSet;
};

std::optional<XFA_subform> XFA_subform::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_subform myClass;

    // load attributes
    parseAttribute(element, "access", myClass.m_access, "open");
    parseAttribute(element, "allowMacro", myClass.m_allowMacro, "0");
    parseAttribute(element, "anchorType", myClass.m_anchorType, "topLeft");
    parseAttribute(element, "colSpan", myClass.m_colSpan, "1");
    parseAttribute(element, "columnWidths", myClass.m_columnWidths, "");
    parseAttribute(element, "h", myClass.m_h, "0in");
    parseAttribute(element, "hAlign", myClass.m_hAlign, "left");
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "layout", myClass.m_layout, "position");
    parseAttribute(element, "locale", myClass.m_locale, "");
    parseAttribute(element, "maxH", myClass.m_maxH, "0in");
    parseAttribute(element, "maxW", myClass.m_maxW, "0in");
    parseAttribute(element, "mergeMode", myClass.m_mergeMode, "consumeData");
    parseAttribute(element, "minH", myClass.m_minH, "0in");
    parseAttribute(element, "minW", myClass.m_minW, "0in");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "presence", myClass.m_presence, "visible");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "restoreState", myClass.m_restoreState, "manual");
    parseAttribute(element, "scope", myClass.m_scope, "name");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");
    parseAttribute(element, "w", myClass.m_w, "0in");
    parseAttribute(element, "x", myClass.m_x, "0in");
    parseAttribute(element, "y", myClass.m_y, "0in");

    // load items
    parseItem(element, "assist", myClass.m_assist);
    parseItem(element, "bind", myClass.m_bind);
    parseItem(element, "bookend", myClass.m_bookend);
    parseItem(element, "border", myClass.m_border);
    parseItem(element, "break", myClass.m_break);
    parseItem(element, "calculate", myClass.m_calculate);
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "keep", myClass.m_keep);
    parseItem(element, "margin", myClass.m_margin);
    parseItem(element, "occur", myClass.m_occur);
    parseItem(element, "overflow", myClass.m_overflow);
    parseItem(element, "pageSet", myClass.m_pageSet);
    parseItem(element, "para", myClass.m_para);
    parseItem(element, "traversal", myClass.m_traversal);
    parseItem(element, "validate", myClass.m_validate);
    parseItem(element, "variables", myClass.m_variables);
    parseItem(element, "area", myClass.m_area);
    parseItem(element, "breakAfter", myClass.m_breakAfter);
    parseItem(element, "breakBefore", myClass.m_breakBefore);
    parseItem(element, "connect", myClass.m_connect);
    parseItem(element, "draw", myClass.m_draw);
    parseItem(element, "event", myClass.m_event);
    parseItem(element, "exObject", myClass.m_exObject);
    parseItem(element, "exclGroup", myClass.m_exclGroup);
    parseItem(element, "field", myClass.m_field);
    parseItem(element, "proto", myClass.m_proto);
    parseItem(element, "setProperty", myClass.m_setProperty);
    parseItem(element, "subform", myClass.m_subform);
    parseItem(element, "subformSet", myClass.m_subformSet);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_subformSet : public XFA_BaseNode
{
public:

    QString getId() const {  return m_id.getValueOrDefault(); }
    QString getName() const {  return m_name.getValueOrDefault(); }
    RELATION1 getRelation() const {  return m_relation.getValueOrDefault(); }
    QString getRelevant() const {  return m_relevant.getValueOrDefault(); }
    QString getUse() const {  return m_use.getValueOrDefault(); }
    QString getUsehref() const {  return m_usehref.getValueOrDefault(); }

    const XFA_bookend* getBookend() const {  return m_bookend.getValue(); }
    const XFA_break* getBreak() const {  return m_break.getValue(); }
    const XFA_desc* getDesc() const {  return m_desc.getValue(); }
    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const XFA_occur* getOccur() const {  return m_occur.getValue(); }
    const XFA_overflow* getOverflow() const {  return m_overflow.getValue(); }
    const std::vector<XFA_Node<XFA_breakAfter>>& getBreakAfter() const {  return m_breakAfter; }
    const std::vector<XFA_Node<XFA_breakBefore>>& getBreakBefore() const {  return m_breakBefore; }
    const std::vector<XFA_Node<XFA_subform>>& getSubform() const {  return m_subform; }
    const std::vector<XFA_Node<XFA_subformSet>>& getSubformSet() const {  return m_subformSet; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_subformSet> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<QString> m_id;
    XFA_Attribute<QString> m_name;
    XFA_Attribute<RELATION1> m_relation;
    XFA_Attribute<QString> m_relevant;
    XFA_Attribute<QString> m_use;
    XFA_Attribute<QString> m_usehref;

    /* subnodes */
    XFA_Node<XFA_bookend> m_bookend;
    XFA_Node<XFA_break> m_break;
    XFA_Node<XFA_desc> m_desc;
    XFA_Node<XFA_extras> m_extras;
    XFA_Node<XFA_occur> m_occur;
    XFA_Node<XFA_overflow> m_overflow;
    std::vector<XFA_Node<XFA_breakAfter>> m_breakAfter;
    std::vector<XFA_Node<XFA_breakBefore>> m_breakBefore;
    std::vector<XFA_Node<XFA_subform>> m_subform;
    std::vector<XFA_Node<XFA_subformSet>> m_subformSet;
};

std::optional<XFA_subformSet> XFA_subformSet::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_subformSet myClass;

    // load attributes
    parseAttribute(element, "id", myClass.m_id, "");
    parseAttribute(element, "name", myClass.m_name, "");
    parseAttribute(element, "relation", myClass.m_relation, "ordered");
    parseAttribute(element, "relevant", myClass.m_relevant, "");
    parseAttribute(element, "use", myClass.m_use, "");
    parseAttribute(element, "usehref", myClass.m_usehref, "");

    // load items
    parseItem(element, "bookend", myClass.m_bookend);
    parseItem(element, "break", myClass.m_break);
    parseItem(element, "desc", myClass.m_desc);
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "occur", myClass.m_occur);
    parseItem(element, "overflow", myClass.m_overflow);
    parseItem(element, "breakAfter", myClass.m_breakAfter);
    parseItem(element, "breakBefore", myClass.m_breakBefore);
    parseItem(element, "subform", myClass.m_subform);
    parseItem(element, "subformSet", myClass.m_subformSet);
    myClass.setOrderFromElement(element);
    return myClass;
}


class XFA_template : public XFA_BaseNode
{
public:

    BASEPROFILE getBaseProfile() const {  return m_baseProfile.getValueOrDefault(); }

    const XFA_extras* getExtras() const {  return m_extras.getValue(); }
    const std::vector<XFA_Node<XFA_subform>>& getSubform() const {  return m_subform; }

    virtual void accept(XFA_AbstractVisitor* visitor) const override { visitor->visit(this); }

    static std::optional<XFA_template> parse(const QDomElement& element);

private:
    /* properties */
    XFA_Attribute<BASEPROFILE> m_baseProfile;

    /* subnodes */
    XFA_Node<XFA_extras> m_extras;
    std::vector<XFA_Node<XFA_subform>> m_subform;
};

std::optional<XFA_template> XFA_template::parse(const QDomElement& element)
{
    if (element.isNull())
    {
        return std::nullopt;
    }

    XFA_template myClass;

    // load attributes
    parseAttribute(element, "baseProfile", myClass.m_baseProfile, "full");

    // load items
    parseItem(element, "extras", myClass.m_extras);
    parseItem(element, "subform", myClass.m_subform);
    myClass.setOrderFromElement(element);
    return myClass;
}


} // namespace xfa


/* END GENERATED CODE */

class PDFXFAEngineImpl
{
public:
    PDFXFAEngineImpl();

    void setDocument(const PDFModifiedDocument& document, PDFForm* form);

    struct LayoutItem
    {
        QRectF nominalExtent;
        const xfa::XFA_draw* draw = nullptr;
        const xfa::XFA_field* field = nullptr;
        const xfa::XFA_subform* subform = nullptr;
        const xfa::XFA_exclGroup* exclGroup = nullptr;
        size_t paragraphSettingsIndex = 0;
        size_t captionParagraphSettingsIndex = 0;
    };

    using LayoutItems = std::vector<LayoutItem>;

    std::vector<QSizeF> getPageSizes() const { return m_layout.pageSizes; }

    inline void setPageSizes(std::vector<QSizeF> pageSizes) { m_layout.pageSizes = std::move(pageSizes); }
    inline void setLayoutItems(PDFInteger pageIndex, LayoutItems layoutItems) { m_layout.layoutItems[pageIndex] = std::move(layoutItems); }
    inline void setParagraphSettings(std::vector<xfa::XFA_ParagraphSettings> paragraphSettings) { m_layout.paragraphSettings = std::move(paragraphSettings); }

    void draw(const QTransform& pagePointToDevicePointMatrix,
              const PDFPage* page,
              QList<PDFRenderError>& errors,
              QPainter* painter);

private:

    struct Layout
    {
        std::vector<QSizeF> pageSizes;
        std::map<PDFInteger, LayoutItems> layoutItems;
        std::vector<xfa::XFA_ParagraphSettings> paragraphSettings;
    };

    void updateResources(const PDFObject& resources);
    void clear();

    QMarginsF createMargin(const xfa::XFA_margin* margin);

    QColor createColor(const xfa::XFA_color* color) const;
    QPen createPenFromEdge(const xfa::XFA_edge* edge, QList<PDFRenderError>& errors) const;
    QPen createPenFromCorner(const xfa::XFA_corner* corner, QList<PDFRenderError>& errors) const;
    QPen createPenFromEdge(const std::vector<xfa::XFA_Node<xfa::XFA_edge>>& edges, size_t index, QList<PDFRenderError>& errors) const;
    QPen createPenFromCorner(const std::vector<xfa::XFA_Node<xfa::XFA_corner>>& corners, size_t index, QList<PDFRenderError>& errors) const;

    struct NodeValue
    {
        QVariant value;
        PDFInteger hintTextMaxChars = 0;
        PDFInteger hintFloatFracDigits = 2;
        PDFInteger hintFloatLeadDigits = -1;
        xfa::XFA_BaseNode::ASPECT hintImageAspect = xfa::XFA_BaseNode::ASPECT::Fit;
        bool hintTextHtml = false;
    };

    void drawItemBorder(const xfa::XFA_border* item,
                        QList<PDFRenderError>& errors,
                        QRectF nominalContentArea,
                        QPainter* painter);

    void drawItemFill(const xfa::XFA_fill* item,
                      QList<PDFRenderError>& errors,
                      QRectF nominalContentArea,
                      QPainter* painter);

    void drawItemLine(const xfa::XFA_line* item,
                      QList<PDFRenderError>& errors,
                      QRectF nominalContentArea,
                      QPainter* painter);

    void drawItemArc(const xfa::XFA_arc* item,
                     QList<PDFRenderError>& errors,
                     QRectF nominalContentArea,
                     QPainter* painter);

    void drawItemRectEdges(const std::vector<xfa::XFA_Node<xfa::XFA_edge>>& edges,
                           const std::vector<xfa::XFA_Node<xfa::XFA_corner>>& corners,
                           QList<PDFRenderError>& errors,
                           QRectF nominalContentArea,
                           const xfa::XFA_BaseNode::HAND hand,
                           QPainter* painter);

    void drawItemDraw(const xfa::XFA_draw* item,
                      QList<PDFRenderError>& errors,
                      QRectF nominalExtentArea,
                      size_t paragraphSettingsIndex,
                      size_t captionParagraphSettingsIndex,
                      QPainter* painter);

    void drawItemField(const xfa::XFA_field* item,
                       QList<PDFRenderError>& errors,
                       QRectF nominalExtentArea,
                       size_t paragraphSettingsIndex,
                       size_t captionParagraphSettingsIndex,
                       QPainter* painter);

    void drawItemSubform(const xfa::XFA_subform* item,
                         QList<PDFRenderError>& errors,
                         QRectF nominalExtentArea,
                         QPainter* painter);

    void drawItemExclGroup(const xfa::XFA_exclGroup* item,
                           QList<PDFRenderError>& errors,
                           QRectF nominalExtentArea,
                           QPainter* painter);

    void drawItemCaption(const xfa::XFA_caption* item,
                         QList<PDFRenderError>& errors,
                         QRectF& nominalExtentArea,
                         size_t captionParagraphSettingsIndex,
                         QPainter* painter);

    void drawItemValue(const xfa::XFA_value* value,
                       const xfa::XFA_ui* ui,
                       QList<PDFRenderError>& errors,
                       QRectF nominalContentArea,
                       size_t paragraphSettingsIndex,
                       QPainter* painter);

    void drawUi(const xfa::XFA_ui* ui,
                const NodeValue& value,
                QList<PDFRenderError>& errors,
                QRectF nominalExtentArea,
                size_t paragraphSettingsIndex,
                QPainter* painter);

    void drawUiTextEdit(const xfa::XFA_textEdit* textEdit,
                        const NodeValue& value,
                        QList<PDFRenderError>& errors,
                        QRectF nominalExtentArea,
                        size_t paragraphSettingsIndex,
                        QPainter* painter);

    void drawUiChoiceList(const xfa::XFA_choiceList* choiceList,
                          const NodeValue& value,
                          QList<PDFRenderError>& errors,
                          QRectF nominalExtentArea,
                          size_t paragraphSettingsIndex,
                          QPainter* painter);

    void drawUiDateTimeEdit(const xfa::XFA_dateTimeEdit* dateTimeEdit,
                            const NodeValue& value,
                            QList<PDFRenderError>& errors,
                            QRectF nominalExtentArea,
                            size_t paragraphSettingsIndex,
                            QPainter* painter);

    void drawUiNumericEdit(const xfa::XFA_numericEdit* numericEdit,
                           const NodeValue& value,
                           QList<PDFRenderError>& errors,
                           QRectF nominalExtentArea,
                           size_t paragraphSettingsIndex,
                           QPainter* painter);

    void drawUiPasswordEdit(const xfa::XFA_passwordEdit* passwordEdit,
                            const NodeValue& value,
                            QList<PDFRenderError>& errors,
                            QRectF nominalExtentArea,
                            size_t paragraphSettingsIndex,
                            QPainter* painter);

    void drawUiSignature(const xfa::XFA_signature* signature,
                         QList<PDFRenderError>& errors,
                         QRectF nominalExtentArea,
                         QPainter* painter);

    void drawUiBarcode(const xfa::XFA_barcode* barcode,
                       QList<PDFRenderError>& errors,
                       QRectF nominalExtentArea,
                       QPainter* painter);

    void drawUiCheckButton(const xfa::XFA_checkButton* checkButton,
                           const NodeValue& value,
                           QRectF nominalExtentArea,
                           QPainter* painter);

    void drawUiImageEdit(const xfa::XFA_imageEdit* imageEdit,
                         const NodeValue& value,
                         QList<PDFRenderError>& errors,
                         QRectF nominalExtentArea,
                         QPainter* painter);

    xfa::XFA_Node<xfa::XFA_template> m_template;
    const PDFDocument* m_document;
    Layout m_layout;
    std::map<int, QByteArray> m_fonts;
};

class PDFXFALayoutEngine : public xfa::XFA_AbstractVisitor
{
    // XFA_AbstractVisitor interface
public:
    virtual void visit(const xfa::XFA_template* node) override;
    virtual void visit(const xfa::XFA_pageArea* node) override;
    virtual void visit(const xfa::XFA_pageSet* node) override;
    virtual void visit(const xfa::XFA_subformSet* node) override;
    virtual void visit(const xfa::XFA_subform* node) override;
    virtual void visit(const xfa::XFA_area* node) override;
    virtual void visit(const xfa::XFA_draw* node) override;
    virtual void visit(const xfa::XFA_field* node) override;
    virtual void visit(const xfa::XFA_exclGroup* node) override;

    void performLayout(PDFXFAEngineImpl* engine, const xfa::XFA_template* node);

private:
    enum class ContentAreaScope
    {
        Page,
        ContentContainer
    };

    void moveToNextArea(ContentAreaScope scope);

    bool isCurrentPageValid() const { return m_currentPageIndex < m_pages.size(); }
    bool isCurrentPageOdd() const { return isCurrentPageValid() && m_pages[m_currentPageIndex].pageIndex % 2 == 1; }
    bool isCurrentPageEven() const { return isCurrentPageValid() && m_pages[m_currentPageIndex].pageIndex % 2 == 0; }
    bool isMaximalPageCountReached() const { return m_pages.size() >= m_maximalPageCount; }

    PDFInteger getCurrentPageIndex() const { return isCurrentPageValid() ? m_pages[m_currentPageIndex].pageIndex : -1; }

    void handleMargin(const xfa::XFA_margin* margin);
    void handlePara(const xfa::XFA_para* para);
    void handleFont(const xfa::XFA_font* font);

    size_t handleCaption(const xfa::XFA_caption* caption);

    void handleBreak(const xfa::XFA_break* node, bool isBeforeLayout);
    void handleBreak(const xfa::XFA_breakBefore* node);
    void handleBreak(const xfa::XFA_breakAfter* node);
    void handleBreak(const std::vector<xfa::XFA_Node<xfa::XFA_breakBefore>>& nodes);
    void handleBreak(const std::vector<xfa::XFA_Node<xfa::XFA_breakAfter>>& nodes);

    QMarginsF createMargin(const xfa::XFA_margin* margin);
    PDFInteger getOccurenceCount(const xfa::XFA_occur* occur);
    PDFInteger getPageOrPageSetMaxOccurenceCount(const xfa::XFA_occur* occur);

    QSizeF getSizeFromMeasurement(const xfa::XFA_Measurement& w, const xfa::XFA_Measurement& h) const;
    QPointF getPointFromMeasurement(const xfa::XFA_Measurement& x, const xfa::XFA_Measurement& y) const;

    struct PageInfo
    {
        const xfa::XFA_pageArea* pageArea = nullptr;
        PDFInteger pageIndex = 0;
        PDFInteger contentBoxIndex = 0;
        QRectF mediaBox;
        QRectF contentBox;
    };

    struct SizeInfo
    {
        /// Size of item
        QSizeF origSize;

        /// Effective size - size computed, bounded by min/max size
        QSizeF effSize;

        /// Minimal size of item
        QSizeF minSize;

        /// Maximal size of item
        QSizeF maxSize;

        QSizeF adjustNominalExtentSize(const QSizeF size) const;
        QSizeF getSizeForLayout() const;
    };

    template<typename Node>
    SizeInfo getSizeInfo(const Node* node) const;

    /// Represents single item layouted onto page (for example,
    /// field, graphics etc.). Nominal extent can be relative to the
    /// parent, or it can be absolute (if parent is a page).
    struct LayoutItem
    {
        void translate(PDFReal dx, PDFReal dy) { nominalExtent.translate(dx, dy); }

        void updatePresence(xfa::XFA_BaseNode::PRESENCE nodePresence)
        {
            switch (nodePresence)
            {
                case xfa::XFA_BaseNode::PRESENCE::Visible:
                    break;
                case xfa::XFA_BaseNode::PRESENCE::Hidden:
                case xfa::XFA_BaseNode::PRESENCE::Inactive:
                    Q_ASSERT(false); // These should not be lay-out
                    break;
                case xfa::XFA_BaseNode::PRESENCE::Invisible:
                    this->presence = xfa::XFA_BaseNode::PRESENCE::Invisible;
                    break;

                default:
                    Q_ASSERT(false);
                    break;
            }
        }

        /// Nominal extent for this item (relative to the parent item)
        QRectF nominalExtent;
        xfa::XFA_BaseNode::PRESENCE presence = xfa::XFA_BaseNode::PRESENCE::Visible;

        /// Paragraph settings index
        size_t paragraphSettingsIndex = 0;

        /// Paragraph settings index for caption
        size_t captionParagraphSettingsIndex = 0;

        const xfa::XFA_draw* draw = nullptr;
        const xfa::XFA_field* field = nullptr;
        const xfa::XFA_subform* subform = nullptr;
        const xfa::XFA_exclGroup* exclGroup = nullptr;
    };

    struct Layout
    {
        void translate(PDFReal dx, PDFReal dy)
        {
            nominalExtent.translate(dx, dy);
            std::for_each(items.begin(), items.end(), [dx, dy](auto& item) { item.translate(dx, dy); });
        }

        void updatePresence(xfa::XFA_BaseNode::PRESENCE presence)
        {
            std::for_each(items.begin(), items.end(), [presence](auto& item) { item.updatePresence(presence); });
        }

        void resize(QSizeF newSize)
        {
            QSizeF oldSize = nominalExtent.size();
            nominalExtent.setSize(newSize);

            qreal diffX = newSize.width() - oldSize.width();
            qreal diffY = newSize.height() - oldSize.height();

            for (LayoutItem& layoutItem : items)
            {
                layoutItem.nominalExtent.setWidth(layoutItem.nominalExtent.width() + diffX);
                layoutItem.nominalExtent.setHeight(layoutItem.nominalExtent.height() + diffY);
            }
        }

        /// Page index (or, more precisely, index of content area)
        size_t pageIndex = 0;

        /// Are we performing layout into content area (not to a page)?
        bool isContentAreaLayout = true;

        /// Nominal extent of the parent node (where items are children
        /// of this node).
        QRectF nominalExtent;
        std::vector<LayoutItem> items;

        int colSpan = 1;
    };

    struct LayoutParameters
    {
        xfa::XFA_BaseNode::PRESENCE presence = xfa::XFA_BaseNode::PRESENCE::Visible;
        xfa::XFA_BaseNode::ANCHORTYPE anchorType = xfa::XFA_BaseNode::ANCHORTYPE::TopLeft;
        const xfa::XFA_subform* nodeSubform = nullptr;
        const xfa::XFA_exclGroup* nodeExclGroup = nullptr;
        const xfa::XFA_area* nodeArea = nullptr;
        const xfa::XFA_caption* nodeCaption = nullptr;
        xfa::XFA_ParagraphSettings paragraphSettings;
        QMarginsF margins;

        /// Type of layout for actual item (so, if some subitems are being
        /// layouted, then this layout is used - for direct subitems)
        xfa::XFA_BaseNode::LAYOUT layoutType = xfa::XFA_BaseNode::LAYOUT::Position;

        /// Size info
        SizeInfo sizeInfo;

        /// Actual x-offset relative to the parent container (in case of positional
        /// layout settings)
        PDFReal xOffset = 0.0;

        /// Actual x-offset relative to the parent container (in case of positional
        /// layout settings)
        PDFReal yOffset = 0.0;

        /// Actual column span if this item is a table cell
        int columnSpan = 0;

        /// Layout of the subitems
        std::vector<Layout> layout;

        /// Rows for table cells
        std::vector<std::vector<Layout>> tableRows;

        QString colWidths;
    };

    class LayoutParametersStackGuard
    {
    public:
        explicit inline LayoutParametersStackGuard(PDFXFALayoutEngine* engine) :
            m_engine(engine)
        {
            if (m_engine->m_layoutParameters.empty())
            {
                m_engine->m_layoutParameters.push(LayoutParameters());
            }
            else
            {
                // Copy paragraph settings
                LayoutParameters oldParameters = m_engine->m_layoutParameters.top();
                LayoutParameters newParameters;
                newParameters.paragraphSettings = oldParameters.paragraphSettings;
                m_engine->m_layoutParameters.push(newParameters);
            }
        }

        inline ~LayoutParametersStackGuard()
        {
            LayoutParameters parameters = std::move(m_engine->m_layoutParameters.top());
            m_engine->m_layoutParameters.pop();

            if (!m_engine->m_layoutParameters.empty())
            {
                m_engine->layout(std::move(parameters));
            }
            else
            {
                m_engine->layoutFlow(parameters, true);

                // Store final layout
                m_engine->m_layout.insert(m_engine->m_layout.end(), std::make_move_iterator(parameters.layout.begin()), std::make_move_iterator(parameters.layout.end()));
            }
        }

    private:
        PDFXFALayoutEngine* m_engine;
    };

    /// Creates paragraph settings index from current paragraph settings
    size_t createParagraphSettings();

    /// Initializes single layout item
    Layout initializeSingleLayout(QRectF nominalExtent);

    /// Returns margins computed from the caption
    QMarginsF getCaptionMargins(const LayoutParameters& layoutParameters) const;

    /// Performs layout (so, using current layout parameters
    /// and given layouts, it layouts them to the current layout node)
    /// \param layoutParameters Parameters, which will be lay out
    void layout(LayoutParameters layoutParameters);

    /// Perform flow layout. Does nothing, if layout has positional
    /// layout type. Final layout is stored also in the layout parameters.
    /// \param layoutParameters Layout parameters
    void layoutFlow(LayoutParameters& layoutParameters, bool breakPages);

    void addSubformToLayout(LayoutParameters& layoutParameters);

    /// Performs positiona layout from source layout to target layout.
    /// Target layout must have positional layout type.
    /// \param sourceLayoutParameters Source layout
    /// \param targetLayoutParameters Target layout
    bool layoutPositional(LayoutParameters& sourceLayoutParameters,
                          LayoutParameters& targetLayoutParameters);

    void finalizeAndAddLayout(QMarginsF captionMargins,
                              Layout finalLayout,
                              LayoutParameters& layoutParameters,
                              QSizeF nominalContentSize);

    LayoutParameters& getLayoutParameters() { return m_layoutParameters.top(); }
    const LayoutParameters& getLayoutParameters() const { return m_layoutParameters.top(); }

    std::vector<PageInfo> m_pages;
    std::vector<Layout> m_layout;
    std::map<const xfa::XFA_pageArea*, std::vector<Layout>> m_pageLayouts;
    std::vector<xfa::XFA_ParagraphSettings> m_paragraphSettings;
    std::stack<LayoutParameters> m_layoutParameters;
    size_t m_currentPageIndex = 0;
    size_t m_maximalPageCount = 128;
};

void PDFXFALayoutEngine::visit(const xfa::XFA_area* node)
{
    LayoutParametersStackGuard guard(this);

    LayoutParameters& parameters = getLayoutParameters();
    parameters.xOffset = node->getX().getValuePt(&parameters.paragraphSettings);
    parameters.yOffset = node->getY().getValuePt(&parameters.paragraphSettings);
    parameters.nodeArea = node;
    parameters.columnSpan = node->getColSpan();

    xfa::XFA_AbstractNode::acceptOrdered(this,
                                         node->getArea(),
                                         node->getDraw(),
                                         node->getExclGroup(),
                                         node->getField(),
                                         node->getSubform(),
                                         node->getSubformSet());
}

PDFXFALayoutEngine::Layout PDFXFALayoutEngine::initializeSingleLayout(QRectF nominalExtent)
{
    Layout layout;
    layout.pageIndex = getCurrentPageIndex();
    layout.nominalExtent = nominalExtent;

    LayoutItem item;
    item.nominalExtent = nominalExtent;
    item.paragraphSettingsIndex = createParagraphSettings();
    layout.items.push_back(std::move(item));
    return layout;
}

QMarginsF PDFXFALayoutEngine::getCaptionMargins(const LayoutParameters& layoutParameters) const
{
    // Do we have visible caption?
    QMarginsF captionMargins(0.0, 0.0, 0.0, 0.0);
    if (layoutParameters.nodeCaption &&
        (layoutParameters.nodeCaption->getPresence() == xfa::XFA_BaseNode::PRESENCE::Visible ||
         layoutParameters.nodeCaption->getPresence() == xfa::XFA_BaseNode::PRESENCE::Invisible))
    {
        PDFReal reserveSize = layoutParameters.nodeCaption->getReserve().getValuePt(&layoutParameters.paragraphSettings);
        switch (layoutParameters.nodeCaption->getPlacement())
        {
            case xfa::XFA_BaseNode::PLACEMENT::Left:
                captionMargins.setLeft(reserveSize);
                break;
            case xfa::XFA_BaseNode::PLACEMENT::Bottom:
                captionMargins.setBottom(reserveSize);
                break;
            case xfa::XFA_BaseNode::PLACEMENT::Inline:
                // Do nothing
                break;
            case xfa::XFA_BaseNode::PLACEMENT::Right:
                captionMargins.setRight(reserveSize);
                break;
            case xfa::XFA_BaseNode::PLACEMENT::Top:
                captionMargins.setTop(reserveSize);
                break;
        }
    }

    return captionMargins;
}

void PDFXFALayoutEngine::layout(LayoutParameters layoutParameters)
{
    // Current layout parameters are layout parameters, to which we are performing
    // layout - new layout nodes will emerge in current layout. Layout parameters
    // as parameter of this function comes from node, which layout is finalizing.
    // We have several issues:
    //
    //     1) We must finish layout of the finalized node, if it is
    //        not a positional layout (positional layout is performed
    //        differently)
    //
    //     2) We must check, if we are performing layouting of the "area" node,
    //        which just translates the nodes by given offset.
    //
    //     3) Finally, we move layout from child parameters to parent layout parameters,
    //        and in case of positional layout, we move them to the right position.

    // Case 1)
    LayoutParameters& currentLayoutParameters = getLayoutParameters();
    layoutFlow(layoutParameters, false);

    std::vector<Layout>& layouts = currentLayoutParameters.layout;
    if (layoutParameters.nodeArea)
    {
        // Case 2)
        const PDFReal x = layoutParameters.xOffset;
        const PDFReal y = layoutParameters.yOffset;

        // Just translate the layout by area offset
        for (Layout& areaLayout : layoutParameters.layout)
        {
            areaLayout.translate(x, y);

            if (currentLayoutParameters.layoutType == xfa::XFA_BaseNode::LAYOUT::Position)
            {
                auto it = std::find_if(layouts.begin(), layouts.end(), [&](const auto& layout) { return layout.pageIndex == areaLayout.pageIndex; });
                if (it == layouts.end())
                {
                    layouts.emplace_back(std::move(areaLayout));
                }
                else
                {
                    // Merge layouts
                    Layout& layout = *it;
                    layout.items.insert(layout.items.end(),
                                        std::make_move_iterator(areaLayout.items.begin()),
                                        std::make_move_iterator(areaLayout.items.end()));
                }
            }
        }

        if (currentLayoutParameters.layoutType != xfa::XFA_BaseNode::LAYOUT::Position)
        {
            layouts.insert(layouts.end(),
                           std::make_move_iterator(layoutParameters.layout.begin()),
                           std::make_move_iterator(layoutParameters.layout.end()));
        }
    }
    else
    {
        // Case 3)
        if (!layoutPositional(layoutParameters, currentLayoutParameters))
        {
            // Not a positional layout
            layouts.insert(layouts.end(),
                           std::make_move_iterator(layoutParameters.layout.begin()),
                           std::make_move_iterator(layoutParameters.layout.end()));
        }
    }

    currentLayoutParameters.tableRows.insert(currentLayoutParameters.tableRows.end(),
                                             std::make_move_iterator(layoutParameters.tableRows.begin()),
                                             std::make_move_iterator(layoutParameters.tableRows.end()));
}

void PDFXFALayoutEngine::finalizeAndAddLayout(QMarginsF captionMargins,
                                              Layout finalLayout,
                                              LayoutParameters& layoutParameters,
                                              QSizeF nominalContentSize)
{
    finalLayout.translate(layoutParameters.margins.left(), layoutParameters.margins.top());

    QSizeF nominalExtentSizeWithoutCaption = nominalContentSize.grownBy(layoutParameters.margins);
    QSizeF nominalExtentSize = nominalExtentSizeWithoutCaption.grownBy(captionMargins);
    nominalExtentSize = layoutParameters.sizeInfo.adjustNominalExtentSize(nominalExtentSize);
    QRectF nominalExtentRegion(QPointF(0, 0), nominalExtentSize);
    finalLayout.nominalExtent = nominalExtentRegion;
    finalLayout.colSpan = layoutParameters.columnSpan;

    if (!finalLayout.items.empty())
    {
        layoutParameters.layout.emplace_back(std::move(finalLayout));
    }
}

void PDFXFALayoutEngine::layoutFlow(LayoutParameters& layoutParameters, bool breakPages)
{
    if (layoutParameters.layoutType == xfa::XFA_BaseNode::LAYOUT::Position)
    {
        return;
    }

    QMarginsF captionMargins = getCaptionMargins(layoutParameters);

    if (layoutParameters.layoutType == xfa::XFA_BaseNode::LAYOUT::Table)
    {
        if (layoutParameters.tableRows.empty())
        {
            // No table to lay out
            return;
        }

        size_t pageIndex = 0;
        std::vector<PDFReal> rowHeights(layoutParameters.tableRows.size(), 0.0);
        std::vector<PDFReal> columnWidths;

        size_t maxColumns = 0;
        for (size_t rowIndex = 0; rowIndex < layoutParameters.tableRows.size(); ++rowIndex)
        {
            size_t columns = 0;
            const std::vector<Layout>& row = layoutParameters.tableRows[rowIndex];

            if (columnWidths.size() < row.size())
            {
                columnWidths.resize(row.size(), 0.0);
            }

            for (size_t columnIndex = 0; columnIndex < row.size(); ++columnIndex)
            {
                const PDFReal cellHeight = row[columnIndex].nominalExtent.height();
                const PDFReal cellWidth = row[columnIndex].nominalExtent.width();
                pageIndex = row[columnIndex].pageIndex;
                if (rowHeights[rowIndex] < cellHeight)
                {
                    rowHeights[rowIndex] = cellHeight;
                }
                if (row[columnIndex].colSpan == 1)
                {
                    ++columns;
                }
                columnWidths[columnIndex] = qMax(columnWidths[columnIndex], cellWidth);
            }
            maxColumns = qMax(maxColumns, columns);
        }

        QStringList colWidths = layoutParameters.colWidths.split(' ', Qt::SkipEmptyParts);
        for (size_t i = 0; i < columnWidths.size(); ++i)
        {
            if (i >= static_cast<size_t>(colWidths.size()))
            {
                break;
            }

            xfa::XFA_Measurement measurement;
            if (xfa::XFA_Measurement::parseMeasurement(colWidths[int(i)], measurement))
            {
                const PDFReal colWidth = measurement.getValuePt(&layoutParameters.paragraphSettings);
                if (colWidth > 0)
                {
                    columnWidths[i] = colWidth;
                }
            }
        }

        Layout finalLayout;
        finalLayout.pageIndex = pageIndex;
        finalLayout.colSpan = layoutParameters.columnSpan;

        PDFReal xOffset = 0;
        PDFReal yOffset = 0;

        for (size_t rowIndex = 0; rowIndex < layoutParameters.tableRows.size(); ++rowIndex)
        {
            std::vector<Layout>& tableRow = layoutParameters.tableRows[rowIndex];

            xOffset = 0;
            size_t columnTargetIndex = 0;
            for (size_t columnSourceIndex = 0; columnSourceIndex < tableRow.size(); ++columnSourceIndex)
            {
                Layout& sourceLayout = tableRow[columnSourceIndex];
                sourceLayout.translate(xOffset, yOffset);
                sourceLayout.updatePresence(layoutParameters.presence);

                PDFReal width = 0;
                for (int i = 0; i < sourceLayout.colSpan; ++i)
                {
                    if (columnTargetIndex >= columnWidths.size())
                    {
                        // We do not have enough space in the table
                        break;
                    }

                    width += columnWidths[columnTargetIndex++];
                }

                sourceLayout.resize(QSizeF(width, rowHeights[rowIndex]));
                finalLayout.items.insert(finalLayout.items.end(), sourceLayout.items.begin(), sourceLayout.items.end());

                xOffset += width;
            }

            yOffset += rowHeights[rowIndex];
        }

        // Translate by margin
        finalLayout.translate(layoutParameters.margins.left(), layoutParameters.margins.top());

        QSizeF nominalContentSize(xOffset, yOffset);
        QSizeF nominalExtentSizeWithoutCaption = nominalContentSize.grownBy(layoutParameters.margins);
        QSizeF nominalExtentSize = nominalExtentSizeWithoutCaption.grownBy(captionMargins);
        QRectF nominalExtentRegion(QPointF(0, 0), nominalExtentSize);
        finalLayout.nominalExtent = nominalExtentRegion;
        finalLayout.colSpan = layoutParameters.columnSpan;

        if (!finalLayout.items.empty())
        {
            layoutParameters.layout.emplace_back(std::move(finalLayout));
        }

        addSubformToLayout(layoutParameters);
        layoutParameters.tableRows.clear();
        layoutParameters.colWidths.clear();
        return;
    }

    std::map<size_t, std::vector<Layout>> layoutsPerPage;

    for (Layout& layout : layoutParameters.layout)
    {
        layoutsPerPage[layout.pageIndex].emplace_back(std::move(layout));
    }
    layoutParameters.layout.clear();

    PDFInteger pageOffset = 0;
    for (auto& item : layoutsPerPage)
    {
        const size_t sourcePageIndex = item.first;
        std::vector<Layout> layouts = std::move(item.second);

        for (Layout& layout : layouts)
        {
            layout.updatePresence(layoutParameters.presence);
        }

        switch (layoutParameters.layoutType)
        {
            case xfa::XFA_BaseNode::LAYOUT::Position:
                // Do nothing - positional layout is done elsewhere
                break;

            case xfa::XFA_BaseNode::LAYOUT::Lr_tb:
            {
                // Left-to-right, top to bottom layout
                Layout finalLayout;
                PDFInteger targetPageIndex = sourcePageIndex + pageOffset;
                finalLayout.pageIndex = targetPageIndex;

                PDFReal x = 0.0;
                PDFReal y = 0.0;
                PDFReal maxW = 0.0;
                PDFReal maxH = 0.0;

                QSizeF size = layoutParameters.sizeInfo.getSizeForLayout();

                for (Layout& layout : layouts)
                {
                    if (x + layout.nominalExtent.width() > size.width())
                    {
                        // New line
                        y += maxH;
                        x = 0.0;
                        maxH = 0.0;
                    }

                    layout.translate(x, y);
                    x += layout.nominalExtent.width();
                    maxW = qMax(maxW, x);
                    maxH = qMax(maxH, layout.nominalExtent.height());

                    finalLayout.items.insert(finalLayout.items.end(), layout.items.begin(), layout.items.end());
                }

                finalizeAndAddLayout(captionMargins, finalLayout, layoutParameters, QSizeF(maxW, y));
                break;
            }

            case xfa::XFA_BaseNode::LAYOUT::Rl_row:
            {
                std::reverse(layouts.begin(), layouts.end());
                layoutParameters.tableRows.emplace_back(std::move(layouts));
                break;
            }

            case xfa::XFA_BaseNode::LAYOUT::Rl_tb:
            {
                // Left-to-right, top to bottom layout
                Layout finalLayout;
                PDFInteger targetPageIndex = sourcePageIndex + pageOffset;
                finalLayout.pageIndex = targetPageIndex;

                QSizeF size = layoutParameters.sizeInfo.getSizeForLayout();

                PDFReal x = size.width();
                PDFReal y = 0.0;
                PDFReal maxH = 0.0;

                for (Layout& layout : layouts)
                {
                    if (x - layout.nominalExtent.width() < 0.0)
                    {
                        // New line
                        y += maxH;
                        x = size.width();
                        maxH = 0.0;
                    }

                    x -= layout.nominalExtent.width();
                    layout.translate(x, y);

                    maxH = qMax(maxH, layout.nominalExtent.height());
                    finalLayout.items.insert(finalLayout.items.end(), layout.items.begin(), layout.items.end());
                }

                finalizeAndAddLayout(captionMargins, finalLayout, layoutParameters, QSizeF(size.width(), y));
                break;
            }

            case xfa::XFA_BaseNode::LAYOUT::Row:
            {
                layoutParameters.tableRows.emplace_back(std::move(layouts));
                break;
            }

            case xfa::XFA_BaseNode::LAYOUT::Table:
            {
                Q_ASSERT(false);
                break;
            }

            case xfa::XFA_BaseNode::LAYOUT::Tb:
            {
                // Top-to-bottom layout
                Layout finalLayout;
                PDFInteger targetPageIndex = sourcePageIndex + pageOffset;
                finalLayout.pageIndex = targetPageIndex;

                PDFReal x = 0.0;
                PDFReal y = 0.0;
                PDFReal h = 0.0;
                PDFReal maxW = 0.0;
                PDFReal maxH = 0.0;

                if (breakPages && targetPageIndex < PDFInteger(m_pages.size()))
                {
                    maxH = m_pages[targetPageIndex].contentBox.height() - layoutParameters.margins.top() - layoutParameters.margins.bottom();
                }

                for (Layout& layout : layouts)
                {
                    if (breakPages && !qFuzzyIsNull(maxH))
                    {
                        PDFReal currentH = h + layout.nominalExtent.height();

                        if (currentH > maxH)
                        {
                            finalizeAndAddLayout(captionMargins, finalLayout, layoutParameters, QSizeF(maxW, y));

                            // Update page offsets and maximal height
                            ++pageOffset;
                            ++targetPageIndex;
                            if (targetPageIndex < PDFInteger(m_pages.size()))
                            {
                                maxH = m_pages[targetPageIndex].contentBox.height() - layoutParameters.margins.top() - layoutParameters.margins.bottom();
                            }

                            // Reinitialize final layout
                            finalLayout = Layout();
                            finalLayout.pageIndex = targetPageIndex;

                            // Current item is layouted onto the next page
                            h = layout.nominalExtent.height();
                            y = 0.0;
                        }
                        else
                        {
                            h = currentH;
                        }
                    }

                    layout.translate(x, y);
                    finalLayout.items.insert(finalLayout.items.end(), layout.items.begin(), layout.items.end());
                    y += layout.nominalExtent.height();
                    maxW = qMax(maxW, layout.nominalExtent.width());
                }

                // Translate by margin
                finalizeAndAddLayout(captionMargins, finalLayout, layoutParameters, QSizeF(maxW, y));
                break;
            }

            default:
            {
                Q_ASSERT(false);
                break;
            }
        }
    }

    addSubformToLayout(layoutParameters);
}

void PDFXFALayoutEngine::addSubformToLayout(LayoutParameters& layoutParameters)
{
    if (layoutParameters.nodeSubform || layoutParameters.nodeExclGroup)
    {
        for (Layout& currentLayout : layoutParameters.layout)
        {
            if (currentLayout.nominalExtent.isValid())
            {
                LayoutItem item;
                item.nominalExtent = currentLayout.nominalExtent;
                item.paragraphSettingsIndex = 0;
                item.presence = layoutParameters.nodeSubform ? layoutParameters.nodeSubform->getPresence() :
                                                               layoutParameters.nodeExclGroup->getPresence();
                item.subform = layoutParameters.nodeSubform;
                item.exclGroup = layoutParameters.nodeExclGroup;
                item.captionParagraphSettingsIndex = 0;
                currentLayout.items.insert(currentLayout.items.begin(), std::move(item));
            }
        }
    }
}

bool PDFXFALayoutEngine::layoutPositional(LayoutParameters& sourceLayoutParameters,
                                          LayoutParameters& targetLayoutParameters)
{
    if (targetLayoutParameters.layoutType != xfa::XFA_BaseNode::LAYOUT::Position)
    {
        // Not a positional layout
        return false;
    }

    std::map<size_t, std::vector<Layout>> layoutsPerPage;

    for (Layout& layout : sourceLayoutParameters.layout)
    {
        layoutsPerPage[layout.pageIndex].emplace_back(std::move(layout));
    }

    QMarginsF captionMargins = getCaptionMargins(targetLayoutParameters);

    for (auto& item : layoutsPerPage)
    {
        const size_t pageIndex = item.first;
        std::vector<Layout> layouts = std::move(item.second);

        for (Layout& layout : layouts)
        {
            layout.updatePresence(targetLayoutParameters.presence);
        }

        auto it = std::find_if(targetLayoutParameters.layout.begin(), targetLayoutParameters.layout.end(), [pageIndex](const auto& layout) { return layout.pageIndex == pageIndex; });
        if (it == targetLayoutParameters.layout.end())
        {
            QSizeF nominalExtentSize = targetLayoutParameters.sizeInfo.effSize;

            Layout createdLayout;
            createdLayout.pageIndex = pageIndex;
            createdLayout.nominalExtent = QRectF(QPointF(0, 0), nominalExtentSize);
            createdLayout.colSpan = targetLayoutParameters.columnSpan;
            targetLayoutParameters.layout.push_back(createdLayout);
            it = std::next(targetLayoutParameters.layout.begin(), targetLayoutParameters.layout.size() - 1);
        }

        Layout& finalLayout = *it;
        PDFReal x = sourceLayoutParameters.xOffset + targetLayoutParameters.margins.left() + captionMargins.left();
        PDFReal y = sourceLayoutParameters.yOffset + targetLayoutParameters.margins.top() + captionMargins.top();

        for (Layout& layout : layouts)
        {
            // Jakub Melka: we must add offset from anchor type
            switch (sourceLayoutParameters.anchorType)
            {
                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::TopLeft:
                    break;

                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::BottomCenter:
                    x -= layout.nominalExtent.width() * 0.5;
                    y -= layout.nominalExtent.height();
                    break;

                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::BottomLeft:
                    y -= layout.nominalExtent.height();
                    break;

                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::BottomRight:
                    x -= layout.nominalExtent.width();
                    y -= layout.nominalExtent.height();
                    break;

                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::MiddleCenter:
                    x -= layout.nominalExtent.width() * 0.5;
                    y -= layout.nominalExtent.height() * 0.5;
                    break;

                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::MiddleLeft:
                    y -= layout.nominalExtent.height() * 0.5;
                    break;

                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::MiddleRight:
                    x -= layout.nominalExtent.width();
                    y -= layout.nominalExtent.height() * 0.5;
                    break;

                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::TopCenter:
                    x -= layout.nominalExtent.width() * 0.5;
                    break;

                case pdf::xfa::XFA_BaseNode::ANCHORTYPE::TopRight:
                    x -= layout.nominalExtent.width();
                    break;

                default:
                    Q_ASSERT(false);
                    break;
            }

            layout.translate(x, y);
            finalLayout.items.insert(finalLayout.items.end(), layout.items.begin(), layout.items.end());
        }

        if (finalLayout.items.empty())
        {
            targetLayoutParameters.layout.pop_back();
        }
    }

    return true;
}

void PDFXFALayoutEngine::visit(const xfa::XFA_draw* node)
{
    switch (node->getPresence())
    {
        case xfa::XFA_BaseNode::PRESENCE::Visible:
            break;

        case xfa::XFA_BaseNode::PRESENCE::Hidden:
        case xfa::XFA_BaseNode::PRESENCE::Inactive:
            return;

        case xfa::XFA_BaseNode::PRESENCE::Invisible:
            break;
    }

    SizeInfo sizeInfo = getSizeInfo(node);

    if (sizeInfo.effSize.isNull())
    {
        // Empty draw, do nothing
        return;
    }

    QPointF point = getPointFromMeasurement(node->getX(), node->getY());
    QRectF nominalExtent(QPoint(0, 0), sizeInfo.effSize);

    LayoutParametersStackGuard guard(this);
    LayoutParameters& parameters = getLayoutParameters();
    parameters.xOffset = point.x();
    parameters.yOffset = point.y();
    parameters.anchorType = node->getAnchorType();
    parameters.presence = node->getPresence();
    parameters.sizeInfo = sizeInfo;
    parameters.nodeCaption = node->getCaption();
    parameters.columnSpan = node->getColSpan();

    handlePara(node->getPara());
    handleFont(node->getFont());

    Layout layout = initializeSingleLayout(nominalExtent);
    layout.items.back().presence = node->getPresence();
    layout.items.back().draw = node;
    layout.items.back().captionParagraphSettingsIndex = handleCaption(node->getCaption());
    layout.colSpan = parameters.columnSpan;

    parameters.layout.emplace_back(std::move(layout));
}

void PDFXFALayoutEngine::visit(const xfa::XFA_field* node)
{
    switch (node->getPresence())
    {
        case xfa::XFA_BaseNode::PRESENCE::Visible:
            break;

        case xfa::XFA_BaseNode::PRESENCE::Hidden:
        case xfa::XFA_BaseNode::PRESENCE::Inactive:
            return;

        case xfa::XFA_BaseNode::PRESENCE::Invisible:
            break;
    }

    SizeInfo sizeInfo = getSizeInfo(node);
    if (sizeInfo.effSize.isNull())
    {
        // Empty draw, do nothing
        return;
    }

    QPointF point = getPointFromMeasurement(node->getX(), node->getY());
    QRectF nominalExtent(QPoint(0, 0), sizeInfo.effSize);

    LayoutParametersStackGuard guard(this);
    LayoutParameters& parameters = getLayoutParameters();
    parameters.xOffset = point.x();
    parameters.yOffset = point.y();
    parameters.anchorType = node->getAnchorType();
    parameters.presence = node->getPresence();
    parameters.sizeInfo = sizeInfo;
    parameters.nodeCaption = node->getCaption();
    parameters.columnSpan = node->getColSpan();

    handlePara(node->getPara());
    handleFont(node->getFont());

    Layout layout = initializeSingleLayout(nominalExtent);
    layout.items.back().presence = node->getPresence();
    layout.items.back().field = node;
    layout.items.back().captionParagraphSettingsIndex = handleCaption(node->getCaption());
    layout.colSpan = parameters.columnSpan;

    parameters.layout.emplace_back(std::move(layout));
}

void PDFXFALayoutEngine::performLayout(PDFXFAEngineImpl* engine, const xfa::XFA_template* node)
{
    // Create default paragraph settings
    m_paragraphSettings = { xfa::XFA_ParagraphSettings() };

    node->accept(this);

    std::vector<QSizeF> pageSizes;
    std::map<PDFInteger, std::vector<Layout>> layoutPerPage;

    for (Layout& layout : m_layout)
    {
        if (layout.isContentAreaLayout)
        {
            const PageInfo& pageInfo = m_pages.at(layout.pageIndex);
            layout.translate(pageInfo.contentBox.left(), pageInfo.contentBox.top());
        }

        layoutPerPage[layout.pageIndex].emplace_back(std::move(layout));
    }

    pageSizes.reserve(layoutPerPage.size());

    PDFInteger pageIndex = 0;
    for (const auto& layoutSinglePage : layoutPerPage)
    {
        PDFXFAEngineImpl::LayoutItems layoutItems;

        const PageInfo& pageInfo = m_pages.at(layoutSinglePage.first);
        pageSizes.push_back(pageInfo.mediaBox.size());

        for (const Layout& layout : m_pageLayouts[pageInfo.pageArea])
        {
            for (const LayoutItem& layoutItem : layout.items)
            {
                // Invisible item?
                if (layoutItem.presence != xfa::XFA_BaseNode::PRESENCE::Visible)
                {
                    continue;
                }

                PDFXFAEngineImpl::LayoutItem engineLayoutItem;
                engineLayoutItem.nominalExtent = layoutItem.nominalExtent;
                engineLayoutItem.draw = layoutItem.draw;
                engineLayoutItem.field = layoutItem.field;
                engineLayoutItem.subform = layoutItem.subform;
                engineLayoutItem.exclGroup = layoutItem.exclGroup;
                engineLayoutItem.paragraphSettingsIndex = layoutItem.paragraphSettingsIndex;
                engineLayoutItem.captionParagraphSettingsIndex = layoutItem.captionParagraphSettingsIndex;
                layoutItems.emplace_back(std::move(engineLayoutItem));
            }
        }

        for (const Layout& layout : layoutSinglePage.second)
        {
            for (const LayoutItem& layoutItem : layout.items)
            {
                // Invisible item?
                if (layoutItem.presence != xfa::XFA_BaseNode::PRESENCE::Visible)
                {
                    continue;
                }

                PDFXFAEngineImpl::LayoutItem engineLayoutItem;
                engineLayoutItem.nominalExtent = layoutItem.nominalExtent;
                engineLayoutItem.draw = layoutItem.draw;
                engineLayoutItem.field = layoutItem.field;
                engineLayoutItem.subform = layoutItem.subform;
                engineLayoutItem.exclGroup = layoutItem.exclGroup;
                engineLayoutItem.paragraphSettingsIndex = layoutItem.paragraphSettingsIndex;
                engineLayoutItem.captionParagraphSettingsIndex = layoutItem.captionParagraphSettingsIndex;
                layoutItems.emplace_back(std::move(engineLayoutItem));
            }
        }

        engine->setLayoutItems(pageIndex++, std::move(layoutItems));
    }

    engine->setPageSizes(std::move(pageSizes));
    engine->setParagraphSettings(std::move(m_paragraphSettings));
}

void PDFXFALayoutEngine::visit(const xfa::XFA_pageArea* node)
{
    auto isPageAddPossible = [this, node]()
    {
        switch (node->getOddOrEven())
        {
            case pdf::xfa::XFA_BaseNode::ODDOREVEN::Any:
                break;

            case pdf::xfa::XFA_BaseNode::ODDOREVEN::Even:
            {
                // Can we add even page (old page must be odd)
                if (m_pages.empty() || m_pages.back().pageIndex % 2 == 0)
                {
                    return false;
                }
                break;
            }

            case pdf::xfa::XFA_BaseNode::ODDOREVEN::Odd:
            {
                // Can we add even page (old page must be odd)
                if (m_pages.empty() && m_pages.back().pageIndex % 2 == 1)
                {
                    return false;
                }
                break;
            }
        }

        return true;
    };

    const PDFInteger max = getPageOrPageSetMaxOccurenceCount(node->getOccur());
    for (PDFInteger i = 0; i < max; ++i)
    {
        if (isMaximalPageCountReached() || !isPageAddPossible())
        {
            break;
        }

        const std::vector<xfa::XFA_Node<xfa::XFA_contentArea>>& contentAreas = node->getContentArea();
        if (contentAreas.empty())
        {
            // Content area is empty... nothing to process at all.
            break;
        }

        const xfa::XFA_medium* medium = node->getMedium();
        if (!medium)
        {
            // Page doesn't contain medium - which is required, so we will
            // stop processing.
            break;
        }

        const PDFReal shortPageSize = medium->getShort().getValuePt(nullptr);
        const PDFReal longPageSize = medium->getLong().getValuePt(nullptr);
        const bool paperOrientationPortrait = medium->getOrientation() == xfa::XFA_medium::ORIENTATION::Portrait;
        QSizeF pageSize = paperOrientationPortrait ? QSizeF(shortPageSize, longPageSize) : QSizeF(longPageSize, shortPageSize);

        PageInfo pageInfo;
        pageInfo.pageIndex = m_pages.empty() ? 1 : m_pages.back().pageIndex + 1;
        pageInfo.mediaBox = QRectF(QPointF(0, 0), pageSize);
        pageInfo.contentBoxIndex = 0;
        pageInfo.pageArea = node;

        for (const auto& contentAreaNode : contentAreas)
        {
            const xfa::XFA_contentArea* contentArea = contentAreaNode.getValue();

            const PDFReal x = contentArea->getX().getValuePt(nullptr);
            const PDFReal y = contentArea->getY().getValuePt(nullptr);
            const PDFReal w = contentArea->getW().getValuePt(nullptr);
            const PDFReal h = contentArea->getH().getValuePt(nullptr);

            pageInfo.contentBox = QRectF(x, y, w, h);
            m_pages.push_back(pageInfo);

            ++pageInfo.contentBoxIndex;
        }
    }

    LayoutParametersStackGuard guard(this);
    xfa::XFA_AbstractNode::acceptOrdered(this, node->getArea(), node->getDraw(), node->getExclGroup(), node->getField(), node->getSubform());

    LayoutParameters& layoutParameters = getLayoutParameters();
    for (Layout& layout : layoutParameters.layout)
    {
        layout.isContentAreaLayout = false;
    }

    m_pageLayouts[node] = std::move(layoutParameters.layout);
    layoutParameters.layout.clear();
}

void PDFXFALayoutEngine::visit(const xfa::XFA_pageSet* node)
{
    const PDFInteger max = getPageOrPageSetMaxOccurenceCount(node->getOccur());
    for (PDFInteger i = 0; i < max && !isMaximalPageCountReached(); ++i)
    {
        const size_t oldPageCount = m_pages.size();
        xfa::XFA_AbstractNode::acceptOrdered(this, node->getPageArea(), node->getPageSet());
        const size_t newPageCount = m_pages.size();

        // This is fallback case for malformed template. If page set
        // element contains no subnodes, we will potentially iterate
        // forever.
        if (oldPageCount == newPageCount)
        {
            // Jakub Melka: No pages generated.
            break;
        }
    }
}

void PDFXFALayoutEngine::visit(const xfa::XFA_subformSet* node)
{
    // Handle break before
    handleBreak(node->getBreak(), true);
    handleBreak(node->getBreakBefore());

    // Perform layout, layout subforms so many times
    const PDFInteger occurenceCount = getOccurenceCount(node->getOccur());
    for (PDFInteger index = 0; index < occurenceCount; ++index)
    {
        xfa::XFA_AbstractNode::acceptOrdered(this, node->getSubform(), node->getSubformSet());
    }

    // Handle break after
    handleBreak(node->getBreak(), false);
    handleBreak(node->getBreakAfter());
}

void PDFXFALayoutEngine::visit(const xfa::XFA_subform* node)
{
    switch (node->getPresence())
    {
        case xfa::XFA_BaseNode::PRESENCE::Visible:
            break;

        case xfa::XFA_BaseNode::PRESENCE::Hidden:
        case xfa::XFA_BaseNode::PRESENCE::Inactive:
            return;

        case xfa::XFA_BaseNode::PRESENCE::Invisible:
            break;
    }

    LayoutParametersStackGuard guard(this);

    SizeInfo sizeInfo = getSizeInfo(node);
    QPointF point = getPointFromMeasurement(node->getX(), node->getY());

    LayoutParameters& parameters = getLayoutParameters();
    parameters.xOffset = point.x();
    parameters.yOffset = point.y();
    parameters.anchorType = node->getAnchorType();
    parameters.presence = node->getPresence();
    parameters.sizeInfo = sizeInfo;
    parameters.layoutType = node->getLayout();
    parameters.colWidths = node->getColumnWidths();
    parameters.nodeSubform = node;
    parameters.columnSpan = node->getColSpan();

    // Handle break before
    handleBreak(node->getBreak(), true);
    handleBreak(node->getBreakBefore());

    // Handle settings
    handlePara(node->getPara());
    handleMargin(node->getMargin());

    // Perform layout, layout subforms so many times
    const PDFInteger occurenceCount = getOccurenceCount(node->getOccur());
    for (PDFInteger index = 0; index < occurenceCount; ++index)
    {
        xfa::XFA_AbstractNode::acceptOrdered(this,
                                             node->getSubform(),
                                             node->getSubformSet(),
                                             node->getField(),
                                             node->getExclGroup(),
                                             node->getDraw(),
                                             node->getArea(),
                                             node->getPageSet());
    }

    // Handle break after
    handleBreak(node->getBreak(), false);
    handleBreak(node->getBreakAfter());
}

void PDFXFALayoutEngine::visit(const xfa::XFA_exclGroup* node)
{
    switch (node->getPresence())
    {
        case xfa::XFA_BaseNode::PRESENCE::Visible:
            break;

        case xfa::XFA_BaseNode::PRESENCE::Hidden:
        case xfa::XFA_BaseNode::PRESENCE::Inactive:
            return;

        case xfa::XFA_BaseNode::PRESENCE::Invisible:
            break;
    }

    LayoutParametersStackGuard guard(this);

    SizeInfo sizeInfo = getSizeInfo(node);
    QPointF point = getPointFromMeasurement(node->getX(), node->getY());

    LayoutParameters& parameters = getLayoutParameters();
    parameters.xOffset = point.x();
    parameters.yOffset = point.y();
    parameters.anchorType = node->getAnchorType();
    parameters.presence = node->getPresence();
    parameters.sizeInfo = sizeInfo;
    parameters.layoutType = node->getLayout();
    parameters.nodeExclGroup = node;
    parameters.columnSpan = node->getColSpan();

    // Handle settings
    handlePara(node->getPara());
    handleMargin(node->getMargin());

    xfa::XFA_AbstractNode::acceptOrdered(this, node->getField());
}

void PDFXFALayoutEngine::moveToNextArea(ContentAreaScope scope)
{
    switch (scope)
    {
        case pdf::PDFXFALayoutEngine::ContentAreaScope::Page:
        {
            if (isCurrentPageValid())
            {
                const PDFInteger pageIndex = getCurrentPageIndex();
                while (isCurrentPageValid() && pageIndex == getCurrentPageIndex())
                {
                    ++m_currentPageIndex;
                }
            }
            else
            {
                ++m_currentPageIndex;
            }
            break;
        }

        case pdf::PDFXFALayoutEngine::ContentAreaScope::ContentContainer:
            ++m_currentPageIndex;
            break;

        default:
            Q_ASSERT(false);
            break;
    }
}

QMarginsF PDFXFALayoutEngine::createMargin(const xfa::XFA_margin* margin)
{
    if (!margin)
    {
        return QMarginsF();
    }

    const xfa::XFA_ParagraphSettings* paragraphSettings = &getLayoutParameters().paragraphSettings;

    const PDFReal leftMargin = margin->getLeftInset().getValuePt(paragraphSettings);
    const PDFReal topMargin = margin->getTopInset().getValuePt(paragraphSettings);
    const PDFReal rightMargin = margin->getRightInset().getValuePt(paragraphSettings);
    const PDFReal bottomMargin = margin->getBottomInset().getValuePt(paragraphSettings);

    QMarginsF margins(leftMargin, topMargin, rightMargin, bottomMargin);
    return margins;
}

void PDFXFALayoutEngine::handleMargin(const xfa::XFA_margin* margin)
{
    getLayoutParameters().margins = createMargin(margin);
}

void PDFXFALayoutEngine::handlePara(const xfa::XFA_para* para)
{
    if (!para)
    {
        return;
    }

    xfa::XFA_ParagraphSettings& settings = getLayoutParameters().paragraphSettings;
    settings.setWidows(para->getWidows());
    settings.setTextIndent(para->getTextIndent().getValuePt(&settings));
    settings.setTabDefault(para->getTabDefault());
    settings.setTabStops(para->getTabStops());
    settings.setOrphans(para->getOrphans());
    settings.setLineHeight(para->getLineHeight().getValuePt(&settings));
    settings.setRadixOffset(para->getRadixOffset().getValuePt(&settings));

    QMarginsF margins(para->getMarginLeft().getValuePt(&settings),
                      para->getSpaceAbove().getValuePt(&settings),
                      para->getMarginRight().getValuePt(&settings),
                      para->getSpaceBelow().getValuePt(&settings));

    settings.setMargins(margins);

    Qt::Alignment alignment = Qt::Alignment();

    switch (para->getHAlign())
    {
        case pdf::xfa::XFA_BaseNode::HALIGN::Left:
            alignment |= Qt::AlignLeft;
            break;
        case pdf::xfa::XFA_BaseNode::HALIGN::Center:
            alignment |= Qt::AlignHCenter;
            break;
        case pdf::xfa::XFA_BaseNode::HALIGN::Justify:
            alignment |= Qt::AlignJustify;
            break;
        case pdf::xfa::XFA_BaseNode::HALIGN::JustifyAll:
            alignment |= Qt::AlignJustify;
            break;
        case pdf::xfa::XFA_BaseNode::HALIGN::Radix:
            alignment |= Qt::AlignRight;
            break;
        case pdf::xfa::XFA_BaseNode::HALIGN::Right:
            alignment |= Qt::AlignRight;
            break;
    }

    switch (para->getVAlign())
    {
        case pdf::xfa::XFA_BaseNode::VALIGN::Top:
            alignment |= Qt::AlignTop;
            break;
        case pdf::xfa::XFA_BaseNode::VALIGN::Bottom:
            alignment |= Qt::AlignBottom;
            break;
        case pdf::xfa::XFA_BaseNode::VALIGN::Middle:
            alignment |= Qt::AlignVCenter;
            break;
    }

    settings.setAlignment(alignment);
}

void PDFXFALayoutEngine::handleFont(const xfa::XFA_font* font)
{
    if (!font)
    {
        return;
    }

    QString faceName = font->getTypeface();
    PDFReal size = font->getSize().getValuePt(nullptr);
    QFont createdFont(faceName);
    createdFont.setPixelSize(size);

    switch (font->getWeight())
    {
        case xfa::XFA_BaseNode::WEIGHT::Normal:
            createdFont.setBold(false);
            break;
        case xfa::XFA_BaseNode::WEIGHT::Bold:
            createdFont.setBold(true);
            break;
    }

    switch (font->getPosture())
    {
        case xfa::XFA_BaseNode::POSTURE::Normal:
            createdFont.setItalic(false);
            break;
        case xfa::XFA_BaseNode::POSTURE::Italic:
            createdFont.setItalic(true);
            break;
    }

    switch (font->getKerningMode())
    {
        case xfa::XFA_BaseNode::KERNINGMODE::None:
            createdFont.setKerning(false);
            break;
        case xfa::XFA_BaseNode::KERNINGMODE::Pair:
            createdFont.setKerning(true);
            break;
    }

    switch (font->getUnderline())
    {
        case xfa::XFA_BaseNode::UNDERLINE::_0:
            createdFont.setUnderline(false);
            break;
        case xfa::XFA_BaseNode::UNDERLINE::_1:
        case xfa::XFA_BaseNode::UNDERLINE::_2:
            createdFont.setUnderline(true);
            break;
    }

    switch (font->getOverline())
    {
        case pdf::xfa::XFA_BaseNode::OVERLINE::_0:
            createdFont.setOverline(false);
            break;
        case pdf::xfa::XFA_BaseNode::OVERLINE::_1:
        case pdf::xfa::XFA_BaseNode::OVERLINE::_2:
            createdFont.setOverline(true);
            break;
    }

    switch (font->getLineThrough())
    {
        case pdf::xfa::XFA_BaseNode::LINETHROUGH::_0:
            createdFont.setStrikeOut(false);
            break;
        case pdf::xfa::XFA_BaseNode::LINETHROUGH::_1:
        case pdf::xfa::XFA_BaseNode::LINETHROUGH::_2:
            createdFont.setStrikeOut(true);
            break;
    }

    createdFont.setHintingPreference(QFont::PreferNoHinting);

    xfa::XFA_ParagraphSettings& settings = getLayoutParameters().paragraphSettings;
    settings.setFont(createdFont);
}

size_t PDFXFALayoutEngine::handleCaption(const xfa::XFA_caption* caption)
{
    if (!caption)
    {
        return 0;
    }

    LayoutParameters& layoutParameters = getLayoutParameters();
    PDFTemporaryValueChange guard(&layoutParameters.paragraphSettings, layoutParameters.paragraphSettings);

    handlePara(caption->getPara());
    handleFont(caption->getFont());
    return createParagraphSettings();
}

void PDFXFALayoutEngine::handleBreak(const xfa::XFA_break* node, bool isBeforeLayout)
{
    if (!node)
    {
        return;
    }

    xfa::XFA_break::BEFORE before = isBeforeLayout ? node->getBefore() : xfa::XFA_break::BEFORE::Auto;
    xfa::XFA_break::AFTER after = !isBeforeLayout ? node->getAfter() : xfa::XFA_break::AFTER::Auto;

    if (before == xfa::XFA_break::BEFORE::PageArea ||
        after == xfa::XFA_break::AFTER::PageArea)
    {
        moveToNextArea(ContentAreaScope::Page);
    }
    else if (before == xfa::XFA_break::BEFORE::ContentArea ||
             after == xfa::XFA_break::AFTER::ContentArea)
    {
        moveToNextArea(ContentAreaScope::ContentContainer);
    }
    else if (before == xfa::XFA_break::BEFORE::PageEven ||
             after == xfa::XFA_break::AFTER::PageEven)
    {
        if (isCurrentPageOdd())
        {
            moveToNextArea(ContentAreaScope::Page);
        }
    }
    else if (before == xfa::XFA_break::BEFORE::PageOdd ||
             after == xfa::XFA_break::AFTER::PageOdd)
    {
        if (isCurrentPageEven())
        {
            moveToNextArea(ContentAreaScope::Page);
        }
    }
}

void PDFXFALayoutEngine::visit(const xfa::XFA_template* node)
{
    for (const auto& subform : node->getSubform())
    {
        subform.getValue()->accept(this);
    }
}

PDFInteger PDFXFALayoutEngine::getOccurenceCount(const xfa::XFA_occur* occur)
{
    if (!occur)
    {
        return 1;
    }

    const PDFInteger initialOccurenceCount = occur->getInitial();
    const PDFInteger minOccurenceCount = occur->getMin();
    const PDFInteger maxOccurenceCount = occur->getMax();

    PDFInteger occurenceCount = qMax(initialOccurenceCount, minOccurenceCount);

    if (maxOccurenceCount >= 0)
    {
        occurenceCount = qMin(occurenceCount, maxOccurenceCount);
    }

    return occurenceCount;
}

PDFInteger PDFXFALayoutEngine::getPageOrPageSetMaxOccurenceCount(const xfa::XFA_occur* occur)
{
    PDFInteger max = std::numeric_limits<PDFInteger>::max();

    if (occur)
    {
        const PDFInteger occurMax = occur->getMax();

        if (occurMax != -1)
        {
            max = occurMax;
        }
    }

    return max;
}

QSizeF PDFXFALayoutEngine::getSizeFromMeasurement(const xfa::XFA_Measurement& w,
                                                  const xfa::XFA_Measurement& h) const
{
    const xfa::XFA_ParagraphSettings& paragraphSettings = getLayoutParameters().paragraphSettings;

    const PDFReal width = w.getValuePt(&paragraphSettings);
    const PDFReal height = h.getValuePt(&paragraphSettings);

    return QSizeF(width, height);
}

QPointF PDFXFALayoutEngine::getPointFromMeasurement(const xfa::XFA_Measurement& x,
                                                    const xfa::XFA_Measurement& y) const
{
    const xfa::XFA_ParagraphSettings& paragraphSettings = getLayoutParameters().paragraphSettings;

    const PDFReal xPos = x.getValuePt(&paragraphSettings);
    const PDFReal yPos = y.getValuePt(&paragraphSettings);

    return QPointF(xPos, yPos);
}

size_t PDFXFALayoutEngine::createParagraphSettings()
{
    const LayoutParameters& layoutParameters = getLayoutParameters();

    auto it = std::find(m_paragraphSettings.begin(), m_paragraphSettings.end(), layoutParameters.paragraphSettings);
    if (it == m_paragraphSettings.end())
    {
        it = m_paragraphSettings.insert(m_paragraphSettings.end(), layoutParameters.paragraphSettings);
    }

    return std::distance(m_paragraphSettings.begin(), it);
}

void PDFXFALayoutEngine::handleBreak(const std::vector<xfa::XFA_Node<xfa::XFA_breakBefore>>& nodes)
{
    for (const auto& node : nodes)
    {
        handleBreak(node.getValue());
    }
}

void PDFXFALayoutEngine::handleBreak(const xfa::XFA_breakBefore* node)
{
    if (!node)
    {
        return;
    }

    switch (node->getTargetType())
    {
        case pdf::xfa::XFA_BaseNode::TARGETTYPE::Auto:
            // Nothing happens
            break;

        case pdf::xfa::XFA_BaseNode::TARGETTYPE::ContentArea:
            moveToNextArea(ContentAreaScope::ContentContainer);
            break;

        case pdf::xfa::XFA_BaseNode::TARGETTYPE::PageArea:
            moveToNextArea(ContentAreaScope::Page);
            break;

        case pdf::xfa::XFA_BaseNode::TARGETTYPE::PageEven:
        {
            if (isCurrentPageOdd())
            {
                moveToNextArea(ContentAreaScope::Page);
            }
            break;
        }

        case pdf::xfa::XFA_BaseNode::TARGETTYPE::PageOdd:
        {
            if (isCurrentPageEven())
            {
                moveToNextArea(ContentAreaScope::Page);
            }
            break;
        }
    }
}

void PDFXFALayoutEngine::handleBreak(const std::vector<xfa::XFA_Node<xfa::XFA_breakAfter> >& nodes)
{
    for (const auto& node : nodes)
    {
        handleBreak(node.getValue());
    }
}

void PDFXFALayoutEngine::handleBreak(const xfa::XFA_breakAfter* node)
{
    if (!node)
    {
        return;
    }

    switch (node->getTargetType())
    {
        case pdf::xfa::XFA_BaseNode::TARGETTYPE::Auto:
            // Nothing happens
            break;

        case pdf::xfa::XFA_BaseNode::TARGETTYPE::ContentArea:
            moveToNextArea(ContentAreaScope::ContentContainer);
            break;

        case pdf::xfa::XFA_BaseNode::TARGETTYPE::PageArea:
            moveToNextArea(ContentAreaScope::Page);
            break;

        case pdf::xfa::XFA_BaseNode::TARGETTYPE::PageEven:
        {
            if (isCurrentPageOdd())
            {
                moveToNextArea(ContentAreaScope::Page);
            }
            break;
        }

        case pdf::xfa::XFA_BaseNode::TARGETTYPE::PageOdd:
        {
            if (isCurrentPageEven())
            {
                moveToNextArea(ContentAreaScope::Page);
            }
            break;
        }
    }
}

PDFXFAEngine::PDFXFAEngine() :
    m_impl(std::make_unique<PDFXFAEngineImpl>())
{

}

PDFXFAEngine::~PDFXFAEngine()
{

}

void PDFXFAEngine::setDocument(const PDFModifiedDocument& document, PDFForm* form)
{
    m_impl->setDocument(document, form);
}

std::vector<QSizeF> PDFXFAEngine::getPageSizes() const
{
    return m_impl->getPageSizes();
}

void PDFXFAEngine::draw(const QTransform& pagePointToDevicePointMatrix,
                        const PDFPage* page,
                        QList<PDFRenderError>& errors,
                        QPainter* painter)
{
    m_impl->draw(pagePointToDevicePointMatrix, page, errors, painter);
}

PDFXFAEngineImpl::PDFXFAEngineImpl() :
    m_document(nullptr)
{

}

void PDFXFAEngineImpl::setDocument(const PDFModifiedDocument& document, PDFForm* form)
{
    if (document.hasFlag(PDFModifiedDocument::XFA_Pagination))
    {
        // Do nothing - pagination of XFA was performed,
        // nothing else has changed.
        return;
    }

    if (m_document != document || document.hasReset())
    {
        m_document = document;

        if (document.hasReset())
        {
            clear();

            if (form->getFormType() == PDFForm::FormType::XFAForm)
            {
                try
                {
                    const PDFObject& xfaObject = m_document->getObject(form->getXFA());
                    updateResources(m_document->getObject(form->getResources()));

                    std::map<QByteArray, QByteArray> xfaData;
                    if (xfaObject.isArray())
                    {
                        const PDFArray* xfaArrayData = xfaObject.getArray();
                        const size_t pairCount = xfaArrayData->getCount() / 2;

                        for (size_t i = 0; i < pairCount; ++i)
                        {
                            const PDFObject& itemName = m_document->getObject(xfaArrayData->getItem(2 * i + 0));
                            const PDFObject& streamObject = m_document->getObject(xfaArrayData->getItem(2 * i + 1));

                            if (itemName.isString() && streamObject.isStream())
                            {
                                xfaData[itemName.getString()] = m_document->getDecodedStream(streamObject.getStream());
                            }
                        }
                    }
                    else if (xfaObject.isStream())
                    {
                        xfaData["template"] = m_document->getDecodedStream(xfaObject.getStream());
                    }

                    QDomDocument templateDocument;
                    if (templateDocument.setContent(xfaData["template"]))
                    {
                        m_template = xfa::XFA_template::parse(templateDocument.firstChildElement("template"));
                    }
                }
                catch (const PDFException&)
                {
                    // Just clear once again - if some errorneous data
                    // were read, we want to clear them.
                    clear();
                }
            }

            // Perform layout
            if (m_template.hasValue())
            {
                PDFXFALayoutEngine layoutEngine;
                layoutEngine.performLayout(this, m_template.getValue());
            }
        }
    }
}

void PDFXFAEngineImpl::draw(const QTransform& pagePointToDevicePointMatrix,
                            const PDFPage* page,
                            QList<PDFRenderError>& errors,
                            QPainter* painter)
{
    if (!m_document || m_layout.layoutItems.empty())
    {
        // Nothing to draw
        return;
    }

    PDFInteger pageIndex = m_document->getCatalog()->getPageIndexFromPageReference(page->getPageReference());
    auto it = m_layout.layoutItems.find(pageIndex);
    if (it == m_layout.layoutItems.cend())
    {
        // Nothing to draw, page is not present
        return;
    }

    PDFPainterStateGuard guard(painter);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setWorldTransform(QTransform(pagePointToDevicePointMatrix), true);
    painter->translate(0, page->getMediaBox().height());
    painter->scale(1.0, -1.0);
    painter->fillRect(page->getMediaBox(), Qt::white);

    const LayoutItems& items = it->second;
    for (const LayoutItem& item : items)
    {
        PDFPainterStateGuard guard2(painter);
        drawItemDraw(item.draw, errors, item.nominalExtent, item.paragraphSettingsIndex, item.captionParagraphSettingsIndex, painter);
        drawItemField(item.field, errors, item.nominalExtent, item.paragraphSettingsIndex, item.captionParagraphSettingsIndex, painter);
        drawItemSubform(item.subform, errors, item.nominalExtent, painter);
        drawItemExclGroup(item.exclGroup, errors, item.nominalExtent, painter);
    }
}

void PDFXFAEngineImpl::updateResources(const PDFObject& resources)
{
    try
    {
        std::set<int> usedFonts;

        if (m_document)
        {
            if (const PDFDictionary* resourcesDictionary = m_document->getDictionaryFromObject(resources))
            {
                if (const PDFDictionary* fontsDictionary = m_document->getDictionaryFromObject(resourcesDictionary->get("Font")))
                {
                    const size_t size = fontsDictionary->getCount();
                    for (size_t i = 0; i < size; ++i)
                    {
                        const PDFDictionary* fontDictionary = m_document->getDictionaryFromObject(fontsDictionary->getValue(i));
                        FontDescriptor descriptor = PDFFont::readFontDescriptor(m_document->getObject(fontDictionary->get("FontDescriptor")), m_document);

                        if (const QByteArray* data = descriptor.getEmbeddedFontData())
                        {
                            int fontId = -1;

                            for (const auto& font : m_fonts)
                            {
                                if (font.second == *data)
                                {
                                    fontId = font.first;
                                    break;
                                }
                            }

                            if (fontId == -1)
                            {
                                // Try to create application font from data
                                fontId = QFontDatabase::addApplicationFontFromData(*data);
                            }

                            if (fontId != -1)
                            {
                                if (!m_fonts.count(fontId))
                                {
                                    m_fonts[fontId] = *data;
                                }

                                // Mark font as used
                                usedFonts.insert(fontId);
                            }
                        }
                    }
                }
            }
        }

        // Remove unused fonts
        for (auto it = m_fonts.begin(); it != m_fonts.end();)
        {
            if (usedFonts.count(it->first))
            {
                ++it;
            }
            else
            {
                QFontDatabase::removeApplicationFont(it->first);
                it = m_fonts.erase(it);
            }
        }
    }
    catch (const PDFException&)
    {
        // Just clear the fonts
    }
}

void PDFXFAEngineImpl::drawItemValue(const xfa::XFA_value* value,
                                     const xfa::XFA_ui* ui,
                                     QList<PDFRenderError>& errors,
                                     QRectF nominalContentArea,
                                     size_t paragraphSettingsIndex,
                                     QPainter* painter)
{
    if (value)
    {
        std::optional<NodeValue> nodeValue;

        if (const xfa::XFA_rectangle* rectangle = value->getRectangle())
        {
            drawItemFill(rectangle->getFill(), errors, nominalContentArea, painter);
            drawItemRectEdges(rectangle->getEdge(), rectangle->getCorner(), errors, nominalContentArea, rectangle->getHand(), painter);
        }
        else if (const xfa::XFA_line* line = value->getLine())
        {
            drawItemLine(line, errors, nominalContentArea, painter);
        }
        else if (const xfa::XFA_arc* arc = value->getArc())
        {
            drawItemArc(arc, errors, nominalContentArea, painter);
        }
        else if (const xfa::XFA_text* text = value->getText())
        {
            nodeValue.emplace();
            nodeValue->value = text->getNodeValue() ? *text->getNodeValue() : QString();
            nodeValue->hintTextMaxChars = text->getMaxChars();
        }
        else if (const xfa::XFA_exData* exData = value->getExData())
        {
            if (exData->getContentType() == "text/html")
            {
                nodeValue.emplace();
                nodeValue->value = exData->getNodeValue() ? *exData->getNodeValue() : QString();
                nodeValue->hintTextMaxChars = exData->getMaxLength();
                nodeValue->hintTextHtml = true;
            }
            else
            {
                errors << PDFRenderError(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Rendering of content type '%1' is not implemented.").arg(exData->getContentType()));
            }
        }
        else if (const xfa::XFA_integer* integer = value->getInteger())
        {
            QString textValue = integer->getNodeValue() ? *integer->getNodeValue() : QString();
            nodeValue.emplace();
            nodeValue->value = textValue.isEmpty() ? QVariant() : textValue.toInt();
        }
        else if (const xfa::XFA_boolean* boolean = value->getBoolean())
        {
            QString textValue = boolean->getNodeValue() ? *boolean->getNodeValue() : QString();
            nodeValue.emplace();
            nodeValue->value = bool(textValue.toInt() != 0);
        }
        else if (const xfa::XFA_decimal* decimal = value->getDecimal())
        {
            QString textValue = decimal->getNodeValue() ? *decimal->getNodeValue() : QString();
            nodeValue.emplace();
            nodeValue->hintFloatFracDigits = decimal->getFracDigits();
            nodeValue->hintFloatLeadDigits = decimal->getLeadDigits();
            nodeValue->value = textValue.isEmpty() ? QVariant() : textValue.toDouble();
        }
        else if (const xfa::XFA_float* floatItem = value->getFloat())
        {
            QString textValue = floatItem->getNodeValue() ? *floatItem->getNodeValue() : QString();
            nodeValue.emplace();
            nodeValue->hintFloatFracDigits = -1;
            nodeValue->hintFloatLeadDigits = -1;
            nodeValue->value = textValue.isEmpty() ? QVariant() : textValue.toDouble();
        }
        else if (const xfa::XFA_dateTime* dateTime = value->getDateTime())
        {
            QString textValue = dateTime->getNodeValue() ? *dateTime->getNodeValue() : QString();
            QDateTime dateTimeVal = QDateTime::fromString(textValue, Qt::ISODate);
            if (dateTimeVal.isValid())
            {
                nodeValue.emplace();
                nodeValue->value = std::move(dateTimeVal);
            }
        }
        else if (const xfa::XFA_date* date = value->getDate())
        {
            QString textValue = date->getNodeValue() ? *date->getNodeValue() : QString();
            QDate dateVal = QDate::fromString(textValue, Qt::ISODate);
            if (dateVal.isValid())
            {
                nodeValue.emplace();
                nodeValue->value = std::move(dateVal);
            }
        }
        else if (const xfa::XFA_time* time = value->getTime())
        {
            QString textValue = time->getNodeValue() ? *time->getNodeValue() : QString();
            QTime timeVal = QTime::fromString(textValue, Qt::ISODate);
            if (timeVal.isValid())
            {
                nodeValue.emplace();
                nodeValue->value = std::move(timeVal);
            }
        }
        else if (const xfa::XFA_image* image = value->getImage())
        {
            QByteArray ba;
            QString textValue = image->getNodeValue() ? *image->getNodeValue() : QString();

            switch (image->getTransferEncoding())
            {
                case pdf::xfa::XFA_BaseNode::TRANSFERENCODING1::Base64:
                    ba = QByteArray::fromBase64(textValue.toLatin1());
                    break;
                case pdf::xfa::XFA_BaseNode::TRANSFERENCODING1::None:
                    ba = textValue.toLatin1();
                    break;
                case pdf::xfa::XFA_BaseNode::TRANSFERENCODING1::Package:
                    errors << PDFRenderError(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Image encoded by 'package' mode not decoded."));
                    break;
            }

            QBuffer buffer(&ba);
            QImageReader reader(&buffer);
            reader.setDecideFormatFromContent(true);
            QImage imageValue = reader.read();

            if (!imageValue.isNull())
            {
                nodeValue.emplace();
                nodeValue->value = std::move(imageValue);
                nodeValue->hintImageAspect = image->getAspect();
            }
            else
            {
                errors << PDFRenderError(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Image of type '%1' not decoded.").arg(image->getContentType()));
            }
        }

        if (nodeValue)
        {
            drawUi(ui, nodeValue.value(), errors, nominalContentArea, paragraphSettingsIndex, painter);
        }
        else
        {
            drawUi(ui, NodeValue(), errors, nominalContentArea, paragraphSettingsIndex, painter);
        }
    }
    else
    {
        drawUi(ui, NodeValue(), errors, nominalContentArea, paragraphSettingsIndex, painter);
    }
}

void PDFXFAEngineImpl::drawItemDraw(const xfa::XFA_draw* item,
                                    QList<PDFRenderError>& errors,
                                    QRectF nominalExtentArea,
                                    size_t paragraphSettingsIndex,
                                    size_t captionParagraphSettingsIndex,
                                    QPainter* painter)
{
    if (!item)
    {
        // Not a draw
        return;
    }

    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = createMargin(item->getMargin());
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    drawItemBorder(item->getBorder(), errors, nominalExtent, painter);
    drawItemCaption(item->getCaption(), errors, nominalContentArea, captionParagraphSettingsIndex, painter);
    drawItemValue(item->getValue(), item->getUi(), errors, nominalContentArea, paragraphSettingsIndex, painter);
}

void PDFXFAEngineImpl::drawItemField(const xfa::XFA_field* item,
                                     QList<PDFRenderError>& errors,
                                     QRectF nominalExtentArea,
                                     size_t paragraphSettingsIndex,
                                     size_t captionParagraphSettingsIndex,
                                     QPainter* painter)
{
    if (!item)
    {
        // Not a field
        return;
    }

    if (item->getUi() && item->getUi()->getButton())
    {
        // We do not display buttons - we do not implement click functionality
        errors << PDFRenderError(RenderErrorType::NotImplemented, PDFTranslationContext::tr("XFA: Buttons not implemented."));
        return;
    }

    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = createMargin(item->getMargin());
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    drawItemBorder(item->getBorder(), errors, nominalExtent, painter);
    drawItemCaption(item->getCaption(), errors, nominalContentArea, captionParagraphSettingsIndex, painter);
    drawItemValue(item->getValue(), item->getUi(), errors, nominalContentArea, paragraphSettingsIndex, painter);
}

void PDFXFAEngineImpl::drawItemSubform(const xfa::XFA_subform* item,
                                       QList<PDFRenderError>& errors,
                                       QRectF nominalExtentArea,
                                       QPainter* painter)
{
    if (!item)
    {
        // Not a form
        return;
    }

    drawItemBorder(item->getBorder(), errors, nominalExtentArea, painter);
}

void PDFXFAEngineImpl::drawItemExclGroup(const xfa::XFA_exclGroup* item,
                                         QList<PDFRenderError>& errors,
                                         QRectF nominalExtentArea,
                                         QPainter* painter)
{
    if (!item)
    {
        // Not an exclusion group
        return;
    }

    drawItemBorder(item->getBorder(), errors, nominalExtentArea, painter);
}

void PDFXFAEngineImpl::drawItemCaption(const xfa::XFA_caption* item,
                                       QList<PDFRenderError>& errors,
                                       QRectF& nominalExtentArea,
                                       size_t captionParagraphSettingsIndex,
                                       QPainter* painter)
{
    if (!item)
    {
        return;
    }

    QRectF captionArea = nominalExtentArea;

    // Do we have visible caption?
    QMarginsF captionsNominalExtentMargins(0.0, 0.0, 0.0, 0.0);
    if (item->getPresence() == xfa::XFA_BaseNode::PRESENCE::Visible ||
        item->getPresence() == xfa::XFA_BaseNode::PRESENCE::Invisible)
    {
        PDFReal reserveSize = item->getReserve().getValuePt(nullptr);
        switch (item->getPlacement())
        {
            case xfa::XFA_BaseNode::PLACEMENT::Left:
                captionsNominalExtentMargins.setLeft(reserveSize);
                captionArea.setWidth(reserveSize);
                break;
            case xfa::XFA_BaseNode::PLACEMENT::Bottom:
                captionsNominalExtentMargins.setBottom(reserveSize);
                captionArea.setTop(captionArea.bottom() - reserveSize);
                break;
            case xfa::XFA_BaseNode::PLACEMENT::Inline:
                // Do nothing
                break;
            case xfa::XFA_BaseNode::PLACEMENT::Right:
                captionsNominalExtentMargins.setRight(reserveSize);
                captionArea.setLeft(captionArea.right() - reserveSize);
                break;
            case xfa::XFA_BaseNode::PLACEMENT::Top:
                captionsNominalExtentMargins.setTop(reserveSize);
                captionArea.setHeight(reserveSize);
                break;
        }
    }

    if (captionsNominalExtentMargins.isNull())
    {
        return;
    }

    // Remove caption area from nominal extent area
    nominalExtentArea = nominalExtentArea.marginsRemoved(captionsNominalExtentMargins);

    // Caption can have its own margin
    QMarginsF captionNominalContentMargins = createMargin(item->getMargin());
    captionArea = captionArea.marginsRemoved(captionNominalContentMargins);

    drawItemValue(item->getValue(), nullptr, errors, captionArea, captionParagraphSettingsIndex, painter);
}

void PDFXFAEngineImpl::drawUi(const xfa::XFA_ui* ui,
                              const NodeValue& value,
                              QList<PDFRenderError>& errors,
                              QRectF nominalExtentArea,
                              size_t paragraphSettingsIndex,
                              QPainter* painter)
{
    const xfa::XFA_barcode* barcode = nullptr;
    const xfa::XFA_button* button = nullptr;
    const xfa::XFA_checkButton* checkButton = nullptr;
    const xfa::XFA_choiceList* choiceList = nullptr;
    const xfa::XFA_dateTimeEdit* dateTimeEdit = nullptr;
    const xfa::XFA_defaultUi* defaultUi = nullptr;
    const xfa::XFA_imageEdit* imageEdit = nullptr;
    const xfa::XFA_numericEdit* numericEdit = nullptr;
    const xfa::XFA_passwordEdit* passwordEdit = nullptr;
    const xfa::XFA_signature* signature = nullptr;
    const xfa::XFA_textEdit* textEdit = nullptr;

    if (ui)
    {
        barcode = ui->getBarcode();
        button = ui->getButton();
        checkButton = ui->getCheckButton();
        choiceList = ui->getChoiceList();
        dateTimeEdit = ui->getDateTimeEdit();
        defaultUi = ui->getDefaultUi();
        imageEdit = ui->getImageEdit();
        numericEdit = ui->getNumericEdit();
        passwordEdit = ui->getPasswordEdit();
        signature = ui->getSignature();
        textEdit = ui->getTextEdit();
    }

    const bool isNonDefaultUi = barcode || button || checkButton || choiceList ||
                                dateTimeEdit || imageEdit || numericEdit ||
                                passwordEdit || signature || textEdit;
    const bool isDefaultUi = !isNonDefaultUi || defaultUi;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (textEdit || (isDefaultUi && value.value.typeId() == QMetaType::QString))
#else
    if (textEdit || (isDefaultUi && value.value.type() == QVariant::String))
#endif
    {
        drawUiTextEdit(textEdit, value, errors, nominalExtentArea, paragraphSettingsIndex, painter);
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    else if (checkButton || (isDefaultUi && value.value.typeId() == QMetaType::Bool))
#else
    else if (checkButton || (isDefaultUi && value.value.type() == QVariant::Bool))
#endif
    {
        drawUiCheckButton(checkButton, value, nominalExtentArea, painter);
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    else if (imageEdit || (isDefaultUi && value.value.typeId() == QMetaType::QImage))
#else
    else if (imageEdit || (isDefaultUi && value.value.type() == QVariant::Image))
#endif
    {
        drawUiImageEdit(imageEdit, value, errors, nominalExtentArea, painter);
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    else if (numericEdit || (isDefaultUi && value.value.typeId() == QMetaType::Double))
#else
    else if (numericEdit || (isDefaultUi && value.value.type() == QVariant::Double))
#endif
    {
        drawUiNumericEdit(numericEdit, value, errors, nominalExtentArea, paragraphSettingsIndex, painter);
    }
    else if (signature)
    {
        drawUiSignature(signature, errors, nominalExtentArea, painter);
    }
    else if (passwordEdit)
    {
        drawUiPasswordEdit(passwordEdit, value, errors, nominalExtentArea, paragraphSettingsIndex, painter);
    }
    else if (barcode)
    {
        drawUiBarcode(barcode, errors, nominalExtentArea, painter);
    }
    else if (button)
    {
        errors << PDFRenderError(RenderErrorType::NotImplemented, PDFTranslationContext::tr("XFA: Buttons not implemented."));
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    else if (dateTimeEdit || (isDefaultUi && (value.value.typeId() == QMetaType::QDateTime ||
                                              value.value.typeId() == QMetaType::QDate ||
                                              value.value.typeId() == QMetaType::QTime)))
#else
    else if (dateTimeEdit || (isDefaultUi && (value.value.type() == QVariant::DateTime ||
                                              value.value.type() == QVariant::Date ||
                                              value.value.type() == QVariant::Time)))
#endif
    {
        drawUiDateTimeEdit(dateTimeEdit, value, errors, nominalExtentArea, paragraphSettingsIndex, painter);
    }
    else if (choiceList)
    {
        drawUiChoiceList(choiceList, value, errors, nominalExtentArea, paragraphSettingsIndex, painter);
    }
    else
    {
        errors << PDFRenderError(RenderErrorType::NotSupported, PDFTranslationContext::tr("XFA: Uknown ui."));
    }
}

void PDFXFAEngineImpl::drawUiTextEdit(const xfa::XFA_textEdit* textEdit,
                                      const NodeValue& value,
                                      QList<PDFRenderError>& errors,
                                      QRectF nominalExtentArea,
                                      size_t paragraphSettingsIndex,
                                      QPainter* painter)
{
    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = textEdit ? createMargin(textEdit->getMargin()) : QMarginsF();
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    if (textEdit && textEdit->getBorder())
    {
        drawItemBorder(textEdit->getBorder(), errors, nominalExtentArea, painter);
    }

    bool isComb = false;
    bool isMultiline = true;
    bool isRichTextAllowed = value.hintTextHtml;
    PDFInteger combNumberOfCells = value.hintTextMaxChars;

    if (textEdit)
    {
        isMultiline = textEdit->getMultiLine() == xfa::XFA_textEdit::MULTILINE::_1;
        isRichTextAllowed = isRichTextAllowed || textEdit->getAllowRichText();

        if (const xfa::XFA_comb* comb = textEdit->getComb())
        {
            isComb = true;
            PDFInteger combCells = comb->getNumberOfCells();
            if (combCells > 0)
            {
                combNumberOfCells = combCells;
            }
        }

        drawItemBorder(textEdit->getBorder(), errors, nominalExtent, painter);
    }

    combNumberOfCells = qMax(combNumberOfCells, PDFInteger(1));
    const xfa::XFA_ParagraphSettings& settings = m_layout.paragraphSettings.at(paragraphSettingsIndex);

    QRectF textRect = nominalContentArea;
    textRect = textRect.marginsRemoved(settings.getMargins());

    if (!isComb)
    {
        painter->setFont(settings.getFont());

        // Do we have space for at least one line?
        if (isMultiline && textRect.height() < 1.8 * painter->fontMetrics().lineSpacing())
        {
            isMultiline = false;
        }

        if (!isRichTextAllowed)
        {
            int textFlag = int(settings.getAlignment()) | Qt::TextDontClip;

            if (isMultiline)
            {
                textFlag = textFlag | Qt::TextWordWrap;
            }
            else
            {
                textFlag = textFlag | Qt::TextSingleLine;
            }

            painter->drawText(textRect, textFlag, value.value.toString());
        }
        else
        {
            PDFPainterStateGuard guard(painter);
            painter->translate(textRect.topLeft());

            QString html = value.value.toString();

            QTextDocument document;
            document.setDocumentMargin(0);
            document.setUseDesignMetrics(true);
            document.setDefaultFont(painter->font());

            QTextOption textOption = document.defaultTextOption();
            textOption.setAlignment(settings.getAlignment());
            textOption.setWrapMode(isMultiline ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::ManualWrap);
            document.setDefaultTextOption(textOption);
            document.setHtml(html);

            QTextBlock block = document.firstBlock();
            while (block.isValid())
            {
                QTextBlockFormat format = block.blockFormat();
                format.setTopMargin(0);
                format.setBottomMargin(0);

                QTextCursor cursor(block);
                cursor.select(QTextCursor::BlockUnderCursor);
                cursor.mergeBlockFormat(format);

                QVector<QTextLayout::FormatRange> textFormats = block.textFormats();
                for (const QTextLayout::FormatRange& formatRange : textFormats)
                {
                    QTextCharFormat charFormat = formatRange.format;
                    QFont font = charFormat.font();

                    if (font.pointSizeF() >  0)
                    {
                        font.setPixelSize(qFloor(font.pointSizeF()));
                    }

                    font.setHintingPreference(QFont::PreferNoHinting);
                    charFormat.setFont(font, QTextCharFormat::FontPropertiesAll);

                    QTextCursor blockCursor(block);
                    blockCursor.setPosition(block.position() + formatRange.start, QTextCursor::MoveAnchor);
                    blockCursor.setPosition(block.position() + formatRange.start + formatRange.length, QTextCursor::KeepAnchor);
                    blockCursor.mergeCharFormat(charFormat);
                }

                block = block.next();
            }

            document.setPageSize(textRect.size());

            if (document.pageCount() > 1)
            {
                // Jakub Melka: we do not have enough space, try to make more,
                // we are still using clip rectangle.
                QSizeF size = document.documentLayout()->documentSize();
                document.setPageSize(size);
            }

            document.drawContents(painter);
        }
    }
    else
    {
        qreal combWidth = textRect.width() / combNumberOfCells;
        QRectF combRect(0.0, 0.0, combWidth, textRect.height());
        painter->setFont(settings.getFont());

        QString text = value.value.toString();

        QColor textColor = Qt::black;
        painter->setPen(textColor);

        for (int i = 0; i < text.size(); ++i)
        {
            if (i >= combNumberOfCells)
            {
                break;
            }

            QString textLine = QString(text[i]);
            painter->drawText(combRect, Qt::AlignCenter, textLine);

            combRect.translate(combWidth, 0.0);
        }
    }
}

void PDFXFAEngineImpl::drawUiChoiceList(const xfa::XFA_choiceList* choiceList,
                                        const NodeValue& value,
                                        QList<PDFRenderError>& errors,
                                        QRectF nominalExtentArea,
                                        size_t paragraphSettingsIndex,
                                        QPainter* painter)
{
    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = choiceList ? createMargin(choiceList->getMargin()) : QMarginsF();
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    if (choiceList && choiceList->getBorder())
    {
        drawItemBorder(choiceList->getBorder(), errors, nominalExtentArea, painter);
    }

    QString text = value.value.toString();

    if (!text.isEmpty())
    {
        NodeValue textValue;
        textValue.value = text;

        drawUiTextEdit(nullptr, textValue, errors, nominalContentArea, paragraphSettingsIndex, painter);
    }
}

void PDFXFAEngineImpl::drawUiDateTimeEdit(const xfa::XFA_dateTimeEdit* dateTimeEdit,
                                          const NodeValue& value,
                                          QList<PDFRenderError>& errors,
                                          QRectF nominalExtentArea,
                                          size_t paragraphSettingsIndex,
                                          QPainter* painter)
{
    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = dateTimeEdit ? createMargin(dateTimeEdit->getMargin()) : QMarginsF();
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    if (dateTimeEdit && dateTimeEdit->getBorder())
    {
        drawItemBorder(dateTimeEdit->getBorder(), errors, nominalExtentArea, painter);
    }

    QString text;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (value.value.typeId() == QMetaType::QDateTime)
#else
    if (value.value.type() == QVariant::DateTime)
#endif
    {
        text = value.value.toDateTime().toString();
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    else if (value.value.typeId() == QMetaType::QTime)
#else
    else if (value.value.type() == QVariant::Time)
#endif
    {
        text = value.value.toTime().toString();
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    else if (value.value.typeId() == QMetaType::QDate)
#else
    else if (value.value.type() == QVariant::Date)
#endif
    {
        text = value.value.toDate().toString();
    }

    if (!text.isEmpty())
    {
        NodeValue textValue;
        textValue.value = text;

        drawUiTextEdit(nullptr, textValue, errors, nominalContentArea, paragraphSettingsIndex, painter);
    }
}

void PDFXFAEngineImpl::drawUiNumericEdit(const xfa::XFA_numericEdit* numericEdit,
                                         const NodeValue& value,
                                         QList<PDFRenderError>& errors,
                                         QRectF nominalExtentArea,
                                         size_t paragraphSettingsIndex,
                                         QPainter* painter)
{
    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = numericEdit ? createMargin(numericEdit->getMargin()) : QMarginsF();
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    if (numericEdit && numericEdit->getBorder())
    {
        drawItemBorder(numericEdit->getBorder(), errors, nominalExtentArea, painter);
    }

    if (!value.value.isValid())
    {
        return;
    }

    QString text;

    if (value.hintFloatFracDigits != -1)
    {
        text = QString::number(value.value.toDouble(), 'f', value.hintFloatFracDigits);
    }
    else
    {
        text = QString::number(value.value.toDouble());
    }

    if (value.hintFloatLeadDigits != -1)
    {
        int leadLength = text.indexOf('.');
        if (leadLength == -1)
        {
            leadLength = text.length();
        }

        while (leadLength < value.hintFloatLeadDigits)
        {
            ++leadLength;
            text.prepend('0');
        }
    }

    const xfa::XFA_ParagraphSettings& settings = m_layout.paragraphSettings.at(paragraphSettingsIndex);
    painter->setFont(settings.getFont());
    int textFlag = int(settings.getAlignment()) | Qt::TextDontClip | Qt::TextSingleLine;

    QRectF textRect = nominalContentArea;
    textRect = textRect.marginsRemoved(settings.getMargins());

    painter->drawText(textRect, textFlag, value.value.toString());
}

void PDFXFAEngineImpl::drawUiPasswordEdit(const xfa::XFA_passwordEdit* passwordEdit,
                                          const NodeValue& value,
                                          QList<PDFRenderError>& errors,
                                          QRectF nominalExtentArea,
                                          size_t paragraphSettingsIndex,
                                          QPainter* painter)
{
    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = createMargin(passwordEdit->getMargin());
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    if (passwordEdit->getBorder())
    {
        drawItemBorder(passwordEdit->getBorder(), errors, nominalExtentArea, painter);
    }

    QString passwordChar = passwordEdit->getPasswordChar();
    if (passwordChar.isEmpty())
    {
        passwordChar = "*";
    }

    QString passwordString(value.value.toString().length(), passwordChar.front());

    if (!passwordString.isEmpty())
    {
        const xfa::XFA_ParagraphSettings& settings = m_layout.paragraphSettings.at(paragraphSettingsIndex);
        painter->setFont(settings.getFont());
        painter->drawText(nominalContentArea, passwordString);
    }
}

void PDFXFAEngineImpl::drawUiSignature(const xfa::XFA_signature* signature,
                                       QList<PDFRenderError>& errors,
                                       QRectF nominalExtentArea,
                                       QPainter* painter)
{
    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = createMargin(signature->getMargin());
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    if (signature && signature->getBorder())
    {
        drawItemBorder(signature->getBorder(), errors, nominalExtentArea, painter);
    }

    painter->setPen(Qt::black);
    painter->fillRect(nominalContentArea, Qt::lightGray);
    painter->drawLine(nominalContentArea.bottomLeft(), nominalContentArea.bottomRight());
}

void PDFXFAEngineImpl::drawUiBarcode(const xfa::XFA_barcode* barcode,
                                     QList<PDFRenderError>& errors,
                                     QRectF nominalExtentArea,
                                     QPainter* painter)
{
    Q_UNUSED(barcode);
    Q_UNUSED(nominalExtentArea);
    Q_UNUSED(painter);

    errors << PDFRenderError(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Barcode not implemented!"));
}

void PDFXFAEngineImpl::drawUiCheckButton(const xfa::XFA_checkButton* checkButton,
                                         const NodeValue& value,
                                         QRectF nominalExtentArea,
                                         QPainter* painter)
{
    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = checkButton ? createMargin(checkButton->getMargin()) : QMarginsF();
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    PDFReal markSize = xfa::XFA_Measurement(10.0, xfa::XFA_Measurement::Type::pt).getValuePt(nullptr);
    xfa::XFA_BaseNode::SHAPE shape = xfa::XFA_BaseNode::SHAPE::Square;
    xfa::XFA_BaseNode::MARK mark = xfa::XFA_BaseNode::MARK::Default;

    if (checkButton)
    {
        // We ignore border for check buttons
        markSize = checkButton->getSize().getValuePt(nullptr);
        shape = checkButton->getShape();
        mark = checkButton->getMark();
    }

    QRectF checkBoxRect = nominalContentArea;
    checkBoxRect.setSize(QSizeF(markSize, markSize));
    checkBoxRect.moveCenter(nominalContentArea.center());

    QPen pen(Qt::black);
    pen.setWidthF(1.0);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    bool showBorder = true;
    if (checkButton)
    {
        if (const xfa::XFA_border* border = checkButton->getBorder())
        {
            const xfa::XFA_BaseNode::PRESENCE presence = border->getPresence();
            showBorder = presence == xfa::XFA_BaseNode::PRESENCE::Visible;
        }
    }

    if (showBorder)
    {
        switch (shape)
        {
            case pdf::xfa::XFA_BaseNode::SHAPE::Square:
                painter->drawRect(checkBoxRect);
                break;
            case pdf::xfa::XFA_BaseNode::SHAPE::Round:
                painter->drawEllipse(checkBoxRect);
                break;
        }
    }

    if (value.value.toBool())
    {
        switch (mark)
        {
            case pdf::xfa::XFA_BaseNode::MARK::Default:
            {
                switch (shape)
                {
                    case pdf::xfa::XFA_BaseNode::SHAPE::Square:
                        painter->fillRect(checkBoxRect, Qt::black);
                        break;
                    case pdf::xfa::XFA_BaseNode::SHAPE::Round:
                    {
                        QPainterPath path;
                        path.addEllipse(checkBoxRect);
                        painter->fillPath(path, Qt::black);
                        break;
                    }
                }
                break;
            }

            case pdf::xfa::XFA_BaseNode::MARK::Check:
            {
                PDFReal starSize = markSize * 0.9;

                QPainterPath path;
                path.addText(QPointF(0, 0), QFont("Arial", starSize), "✓");
                path.translate(checkBoxRect.center() - path.boundingRect().center());
                painter->drawPath(path);
                break;
            }

            case pdf::xfa::XFA_BaseNode::MARK::Circle:
            {
                QPainterPath path;
                path.addEllipse(checkBoxRect);
                painter->fillPath(path, Qt::black);
                break;
            }

            case pdf::xfa::XFA_BaseNode::MARK::Cross:
            {
                painter->drawLine(checkBoxRect.topLeft(), checkBoxRect.bottomRight());
                painter->drawLine(checkBoxRect.bottomLeft(), checkBoxRect.topRight());
                break;
            }

            case pdf::xfa::XFA_BaseNode::MARK::Diamond:
            {
                PDFReal diamondSize = markSize * 0.4;

                QPainterPath path;
                path.moveTo(0, -diamondSize);
                path.lineTo(diamondSize, 0);
                path.lineTo(0, diamondSize);
                path.lineTo(-diamondSize, 0);
                path.closeSubpath();
                path.translate(checkBoxRect.center());
                painter->fillPath(path, Qt::black);
                break;
            }

            case pdf::xfa::XFA_BaseNode::MARK::Square:
            {
                painter->fillRect(checkBoxRect, Qt::black);
                break;
            }

            case pdf::xfa::XFA_BaseNode::MARK::Star:
            {
                PDFReal starSize = markSize * 0.9;

                QPainterPath path;
                path.addText(QPointF(0, 0), QFont("Arial", starSize), "☆");
                path.translate(checkBoxRect.center() - path.boundingRect().center());
                painter->drawPath(path);
                break;
            }
        }
    }
}

void PDFXFAEngineImpl::drawUiImageEdit(const xfa::XFA_imageEdit* imageEdit,
                                       const NodeValue& value,
                                       QList<PDFRenderError>& errors,
                                       QRectF nominalExtentArea,
                                       QPainter* painter)
{
    QRectF nominalExtent = nominalExtentArea;
    QRectF nominalContentArea = nominalExtent;
    QMarginsF contentMargins = imageEdit ? createMargin(imageEdit->getMargin()) : QMarginsF();
    nominalContentArea = nominalExtent.marginsRemoved(contentMargins);

    if (imageEdit && imageEdit->getBorder())
    {
        drawItemBorder(imageEdit->getBorder(), errors, nominalExtentArea, painter);
    }

    QImage image = value.value.value<QImage>();
    switch (value.hintImageAspect)
    {
        case pdf::xfa::XFA_BaseNode::ASPECT::Fit:
        {
            QRectF imageRect = nominalContentArea;
            QSizeF imageSize = QSizeF(image.size()).scaled(imageRect.size(), Qt::KeepAspectRatio);
            imageRect.setSize(imageSize);
            imageRect.moveCenter(nominalContentArea.center());
            painter->drawImage(imageRect, image);
            break;
        }

        case pdf::xfa::XFA_BaseNode::ASPECT::Actual:
        {
            painter->drawImage(nominalContentArea.topLeft(), image);
            break;
        }

        case pdf::xfa::XFA_BaseNode::ASPECT::Height:
        {
            QRectF imageRect = nominalContentArea;
            QSizeF imageSize = QSizeF(image.size());

            if (qFuzzyIsNull(imageSize.height()))
            {
                imageRect.setWidth(imageSize.width() * imageRect.height() / imageSize.height());
                imageRect.moveCenter(nominalContentArea.center());
                painter->drawImage(imageRect, image);
            }
            break;
        }

        case pdf::xfa::XFA_BaseNode::ASPECT::None:
        {
            painter->drawImage(nominalContentArea, image);
            break;
        }

        case pdf::xfa::XFA_BaseNode::ASPECT::Width:
        {
            QRectF imageRect = nominalContentArea;
            QSizeF imageSize = QSizeF(image.size());

            if (qFuzzyIsNull(imageSize.width()))
            {
                imageRect.setHeight(imageSize.height() * imageRect.width() / imageSize.width());
                imageRect.moveCenter(nominalContentArea.center());
                painter->drawImage(imageRect, image);
            }
            break;
        }
    }
}

void PDFXFAEngineImpl::drawItemBorder(const xfa::XFA_border* item,
                                      QList<PDFRenderError>& errors,
                                      QRectF nominalContentArea,
                                      QPainter* painter)
{
    if (!item || item->getPresence() != xfa::XFA_BaseNode::PRESENCE::Visible)
    {
        return;
    }

    QMarginsF contentMargins = createMargin(item->getMargin());
    nominalContentArea = nominalContentArea.marginsRemoved(contentMargins);

    if (nominalContentArea.isEmpty())
    {
        // Jakub Melka: nothing to draw
        return;
    }

    drawItemFill(item->getFill(), errors, nominalContentArea, painter);
    drawItemRectEdges(item->getEdge(), item->getCorner(), errors, nominalContentArea, item->getHand(), painter);
}

void PDFXFAEngineImpl::drawItemFill(const xfa::XFA_fill* item,
                                    QList<PDFRenderError>& errors,
                                    QRectF nominalContentArea,
                                    QPainter* painter)
{
    if (!item)
    {
        return;
    }

    QColor startColor = createColor(item->getColor());
    if (!startColor.isValid())
    {
        startColor = Qt::white;
    }

    if (item->getSolid())
    {
        painter->fillRect(nominalContentArea, startColor);
    }
    else if (const xfa::XFA_linear* linear = item->getLinear())
    {
        QLinearGradient linearGradient;

        switch (linear->getType())
        {
            case pdf::xfa::XFA_BaseNode::TYPE1::ToRight:
                linearGradient.setStart(nominalContentArea.topLeft());
                linearGradient.setFinalStop(nominalContentArea.topRight());
                break;
            case pdf::xfa::XFA_BaseNode::TYPE1::ToBottom:
                linearGradient.setStart(nominalContentArea.topLeft());
                linearGradient.setFinalStop(nominalContentArea.bottomLeft());
                break;
            case pdf::xfa::XFA_BaseNode::TYPE1::ToLeft:
                linearGradient.setStart(nominalContentArea.topRight());
                linearGradient.setFinalStop(nominalContentArea.topLeft());
                break;
            case pdf::xfa::XFA_BaseNode::TYPE1::ToTop:
                linearGradient.setStart(nominalContentArea.bottomLeft());
                linearGradient.setFinalStop(nominalContentArea.topLeft());
                break;
        }

        QColor endColor = createColor(linear->getColor());
        linearGradient.setColorAt(0.0, startColor);
        linearGradient.setColorAt(1.0, endColor);
        painter->fillRect(nominalContentArea, QBrush(std::move(linearGradient)));
    }
    else if (const xfa::XFA_radial* radial = item->getRadial())
    {
        QRadialGradient radialGradient(nominalContentArea.center(), qMin(nominalContentArea.width(), nominalContentArea.height()));
        QColor endColor = createColor(radial->getColor());

        switch (radial->getType())
        {
            case pdf::xfa::XFA_BaseNode::TYPE3::ToEdge:
                radialGradient.setColorAt(0.0, startColor);
                radialGradient.setColorAt(1.0, endColor);
                break;
            case pdf::xfa::XFA_BaseNode::TYPE3::ToCenter:
                radialGradient.setColorAt(1.0, startColor);
                radialGradient.setColorAt(0.0, endColor);
                break;
        }

        painter->fillRect(nominalContentArea, QBrush(std::move(radialGradient)));
    }
    else if (item->getPattern() || item->getStipple())
    {
        errors << PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("XFA: Unknown fill pattern."));
    }
    else
    {
        painter->fillRect(nominalContentArea, startColor);
    }
}

void PDFXFAEngineImpl::drawItemLine(const xfa::XFA_line* item,
                                    QList<PDFRenderError>& errors,
                                    QRectF nominalContentArea,
                                    QPainter* painter)
{
    if (!item)
    {
        return;
    }

    QPen pen = createPenFromEdge(item->getEdge(), errors);

    if (pen.style() == Qt::NoPen)
    {
        return;
    }

    QLineF line;
    switch (item->getSlope())
    {
        case pdf::xfa::XFA_BaseNode::SLOPE::Backslash:
            line = QLineF(nominalContentArea.topLeft(), nominalContentArea.bottomRight());
            break;
        case pdf::xfa::XFA_BaseNode::SLOPE::Slash:
            line = QLineF(nominalContentArea.bottomLeft(), nominalContentArea.topRight());
            break;
    }

    PDFReal offset = 0.0;
    switch (item->getHand())
    {
        case xfa::XFA_BaseNode::HAND::Even:
            break;
        case xfa::XFA_BaseNode::HAND::Left:
            offset = -pen.widthF() * 0.5;
            break;
        case xfa::XFA_BaseNode::HAND::Right:
            offset = +pen.widthF() * 0.5;
            break;
    }

    if (!qFuzzyIsNull(offset))
    {
        const QLineF unitVector = line.normalVector().unitVector();
        const qreal offsetX = unitVector.dx() * offset;
        const qreal offsetY = unitVector.dy() * offset;
        line.translate(offsetX, offsetY);
    }

    painter->setPen(std::move(pen));
    painter->drawLine(line);
}

void PDFXFAEngineImpl::drawItemArc(const xfa::XFA_arc* item,
                                   QList<PDFRenderError>& errors,
                                   QRectF nominalContentArea,
                                   QPainter* painter)
{
    if (!item)
    {
        return;
    }

    // Draw fill
    drawItemFill(item->getFill(), errors, nominalContentArea, painter);

    // Draw arc
    QPen pen = createPenFromEdge(item->getEdge(), errors);
    if (pen.style() == Qt::NoPen)
    {
        return;
    }

    QRectF arcArea = nominalContentArea;
    if (item->getCircular())
    {
        qreal minEdge = qMin(arcArea.width(), arcArea.height());
        arcArea.setWidth(minEdge);
        arcArea.setHeight(minEdge);
    }

    switch (item->getHand())
    {
        case pdf::xfa::XFA_BaseNode::HAND::Even:
            break;

        case pdf::xfa::XFA_BaseNode::HAND::Left:
        {
            const qreal halfEdgeWidth = pen.widthF() * 0.5;
            arcArea.adjust(0, 0, -halfEdgeWidth, -halfEdgeWidth);
            break;
        }

        case pdf::xfa::XFA_BaseNode::HAND::Right:
        {
            const qreal halfEdgeWidth = pen.widthF() * 0.5;
            arcArea.adjust(0, 0, +halfEdgeWidth, +halfEdgeWidth);
            break;
        }
    }

    arcArea.moveCenter(nominalContentArea.center());

    painter->setPen(std::move(pen));
    painter->drawArc(arcArea, item->getStartAngle() * 16, item->getSweepAngle() * 16);
}

void PDFXFAEngineImpl::drawItemRectEdges(const std::vector<xfa::XFA_Node<xfa::XFA_edge>>& edges,
                                         const std::vector<xfa::XFA_Node<xfa::XFA_corner>>& corners,
                                         QList<PDFRenderError>& errors,
                                         QRectF nominalContentArea,
                                         const xfa::XFA_BaseNode::HAND hand,
                                         QPainter* painter)
{
    if (edges.empty() && corners.empty())
    {
        // We draw nothing
        return;
    }

    // Step 1) prepare edge array. We must note, that edges
    // can be nullptr, in that case, do not draw anything.
    // All edges are null, then empty pen is returned.
    std::array edgePens = {
        createPenFromEdge(edges, 0, errors),
        createPenFromEdge(edges, 1, errors),
        createPenFromEdge(edges, 2, errors),
        createPenFromEdge(edges, 3, errors)
    };

    // Step 2) prepare draw rectangle, which can be different
    // from nominal content area, because we must handle handedness.

    auto getOffset = [&edgePens, hand](const size_t index)
    {
        PDFReal offset = 0.0;
        switch (hand)
        {
            case pdf::xfa::XFA_BaseNode::HAND::Even:
                break;
            case pdf::xfa::XFA_BaseNode::HAND::Left:
                offset = -edgePens[index].widthF() * 0.5;
                break;
            case pdf::xfa::XFA_BaseNode::HAND::Right:
                offset = +edgePens[index].widthF() * 0.5;
                break;
        }

        return offset;
    };

    QMarginsF handMargins(getOffset(3), getOffset(0), -getOffset(1), -getOffset(2));
    QRectF drawRect = nominalContentArea.marginsRemoved(handMargins);

    // Step 3) Draw lines without corners. We must  consider corner radius.
    // Corners will be then drawm afterwards.

    constexpr size_t CORNER_TOP_LEFT_INDEX = 0;
    constexpr size_t CORNER_TOP_RIGHT_INDEX = 1;
    constexpr size_t CORNER_BOTTOM_RIGHT_INDEX = 2;
    constexpr size_t CORNER_BOTTOM_LEFT_INDEX = 3;

    auto getRadiusFromCorner = [&corners](const size_t index) -> PDFReal
    {
        if (index < corners.size())
        {
            return corners[index].getValue()->getRadius().getValuePt(nullptr);
        }
        else if (!corners.empty())
        {
            return corners.back().getValue()->getRadius().getValuePt(nullptr);
        }

        return 0.0;
    };

    std::array cornerOffsets = {
        getRadiusFromCorner(CORNER_TOP_LEFT_INDEX),
        getRadiusFromCorner(CORNER_TOP_RIGHT_INDEX),
        getRadiusFromCorner(CORNER_BOTTOM_RIGHT_INDEX),
        getRadiusFromCorner(CORNER_BOTTOM_LEFT_INDEX)
    };

    auto drawLine = [painter](QPointF start, QPointF end, QPen pen, qreal offsetStart, qreal offsetEnd)
    {
        if (pen.style() == Qt::NoPen)
        {
            return;
        }

        QPointF adjustedStart = start;
        QPointF adjustedEnd = end;

        if (!qFuzzyIsNull(offsetStart))
        {
            QLineF line(start, end);
            QLineF unitVector = line.unitVector();

            adjustedStart += (unitVector.p2() - unitVector.p1()) * offsetStart;
        }

        if (!qFuzzyIsNull(offsetEnd))
        {
            QLineF line(end, start);
            QLineF unitVector = line.unitVector();

            adjustedEnd += (unitVector.p2() - unitVector.p1()) * offsetEnd;
        }

        painter->setPen(std::move(pen));
        painter->drawLine(adjustedStart, adjustedEnd);
    };

    drawLine(drawRect.topLeft(), drawRect.topRight(), edgePens[0], cornerOffsets[CORNER_TOP_LEFT_INDEX], cornerOffsets[CORNER_TOP_RIGHT_INDEX]);
    drawLine(drawRect.topRight(), drawRect.bottomRight(), edgePens[1], cornerOffsets[CORNER_TOP_RIGHT_INDEX], cornerOffsets[CORNER_BOTTOM_RIGHT_INDEX]);
    drawLine(drawRect.bottomRight(), drawRect.bottomLeft(), edgePens[2], cornerOffsets[CORNER_BOTTOM_RIGHT_INDEX], cornerOffsets[CORNER_BOTTOM_LEFT_INDEX]);
    drawLine(drawRect.bottomLeft(), drawRect.topLeft(), edgePens[3], cornerOffsets[CORNER_BOTTOM_LEFT_INDEX], cornerOffsets[CORNER_TOP_LEFT_INDEX]);

    // Step 4) Draw corners
    auto getCorner = [&corners](size_t index) -> const xfa::XFA_corner*
    {
        if (index < corners.size())
        {
            return corners[index].getValue();
        }
        else if (!corners.empty())
        {
            return corners.back().getValue();
        }

        return nullptr;
    };

    // Draws corner with down line and right line orientation (when rotation is zero).

    std::array cornerPens = {
        createPenFromCorner(corners, CORNER_TOP_LEFT_INDEX, errors),
        createPenFromCorner(corners, CORNER_TOP_RIGHT_INDEX, errors),
        createPenFromCorner(corners, CORNER_BOTTOM_RIGHT_INDEX, errors),
        createPenFromCorner(corners, CORNER_BOTTOM_LEFT_INDEX, errors)
    };

    auto drawCorner = [painter](QPointF point, QPen pen, qreal rotation, const xfa::XFA_corner* corner)
    {
        if (!corner || pen.style() == Qt::NoPen)
        {
            return;
        }

        const xfa::XFA_BaseNode::JOIN join = corner->getJoin();
        bool isInverted = corner->getInverted();
        const PDFReal radius = corner->getRadius().getValuePt(nullptr);

        PDFPainterStateGuard guard(painter);
        painter->translate(point);
        painter->rotate(rotation);
        painter->setPen(std::move(pen));

        switch (join)
        {
            case xfa::XFA_BaseNode::JOIN::Square:
            {
                painter->drawLine(0, 0, 0, radius);
                painter->drawLine(0, 0, radius, 0);
                break;
            }

            case xfa::XFA_BaseNode::JOIN::Round:
            {
                if (!isInverted)
                {
                    painter->drawArc(0, 0, 2.0 * radius, 2.0 * radius, -180 * 16, -90 * 16);
                }
                else
                {
                    painter->drawArc(-radius, -radius, 2.0 * radius, 2.0 * radius, 0, -90 * 16);
                }
                break;
            }
        }
    };

    drawCorner(drawRect.topLeft(), cornerPens[CORNER_TOP_LEFT_INDEX],   0, getCorner(CORNER_TOP_LEFT_INDEX));
    drawCorner(drawRect.topRight(), cornerPens[CORNER_TOP_RIGHT_INDEX],  90, getCorner(CORNER_TOP_RIGHT_INDEX));
    drawCorner(drawRect.bottomRight(), cornerPens[CORNER_BOTTOM_RIGHT_INDEX], 180, getCorner(CORNER_BOTTOM_RIGHT_INDEX));
    drawCorner(drawRect.bottomLeft(), cornerPens[CORNER_BOTTOM_LEFT_INDEX], 270, getCorner(CORNER_BOTTOM_LEFT_INDEX));
}

void PDFXFAEngineImpl::clear()
{
    // Clear the template
    m_template = xfa::XFA_Node<xfa::XFA_template>();
    m_layout = Layout();

    for (const auto& font : m_fonts)
    {
        QFontDatabase::removeApplicationFont(font.first);
    }
    m_fonts.clear();
}

QMarginsF PDFXFAEngineImpl::createMargin(const xfa::XFA_margin* margin)
{
    if (!margin)
    {
        return QMarginsF();
    }

    const PDFReal leftMargin = margin->getLeftInset().getValuePt(nullptr);
    const PDFReal topMargin = margin->getTopInset().getValuePt(nullptr);
    const PDFReal rightMargin = margin->getRightInset().getValuePt(nullptr);
    const PDFReal bottomMargin = margin->getBottomInset().getValuePt(nullptr);

    QMarginsF margins(leftMargin, topMargin, rightMargin, bottomMargin);
    return margins;
}

QColor PDFXFAEngineImpl::createColor(const xfa::XFA_color* color) const
{
    if (color)
    {
        QStringList sl = color->getValue().split(",");
        const int r = sl.size() > 0 ? sl[0].toInt() : 255;
        const int g = sl.size() > 1 ? sl[1].toInt() : 255;
        const int b = sl.size() > 2 ? sl[2].toInt() : 255;
        return QColor(r, g, b, 255);
    }

    return QColor();
}

QPen PDFXFAEngineImpl::createPenFromEdge(const xfa::XFA_edge* edge, QList<PDFRenderError>& errors) const
{
    QPen pen(Qt::NoPen);

    if (!edge)
    {
        return pen;
    }

    const xfa::XFA_BaseNode::PRESENCE presence = edge->getPresence();
    switch (presence)
    {
        case xfa::XFA_BaseNode::PRESENCE::Visible:
            break;
        case xfa::XFA_BaseNode::PRESENCE::Hidden:
        case xfa::XFA_BaseNode::PRESENCE::Inactive:
        case xfa::XFA_BaseNode::PRESENCE::Invisible:
            return pen;

        default:
            Q_ASSERT(false);
            return pen;
    }

    switch (edge->getCap())
    {
        case pdf::xfa::XFA_BaseNode::CAP::Square:
            pen.setCapStyle(Qt::SquareCap);
            break;
        case pdf::xfa::XFA_BaseNode::CAP::Butt:
            pen.setCapStyle(Qt::FlatCap);
            break;
        case pdf::xfa::XFA_BaseNode::CAP::Round:
            pen.setCapStyle(Qt::RoundCap);
            break;
    }

    switch (edge->getStroke())
    {
        case pdf::xfa::XFA_BaseNode::STROKE::Solid:
            pen.setStyle(Qt::SolidLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::DashDot:
            pen.setStyle(Qt::DashDotLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::DashDotDot:
            pen.setStyle(Qt::DashDotDotLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::Dashed:
            pen.setStyle(Qt::DashLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::Dotted:
            pen.setStyle(Qt::DotLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::Embossed:
        case pdf::xfa::XFA_BaseNode::STROKE::Etched:
        case pdf::xfa::XFA_BaseNode::STROKE::Lowered:
        case pdf::xfa::XFA_BaseNode::STROKE::Raised:
            pen.setStyle(Qt::SolidLine); // Ignore these line types
            errors << PDFRenderError(RenderErrorType::NotSupported, PDFTranslationContext::tr("XFA: special stroke is not supported."));
            break;
    }

    pen.setWidthF(edge->getThickness().getValuePt(nullptr));

    QColor color = createColor(edge->getColor());
    pen.setColor(color);
    return pen;
}

QPen PDFXFAEngineImpl::createPenFromCorner(const xfa::XFA_corner* corner, QList<PDFRenderError>& errors) const
{
    QPen pen(Qt::NoPen);

    if (!corner)
    {
        return pen;
    }

    const xfa::XFA_BaseNode::PRESENCE presence = corner->getPresence();
    switch (presence)
    {
        case xfa::XFA_BaseNode::PRESENCE::Visible:
            break;
        case xfa::XFA_BaseNode::PRESENCE::Hidden:
        case xfa::XFA_BaseNode::PRESENCE::Inactive:
        case xfa::XFA_BaseNode::PRESENCE::Invisible:
            return pen;

        default:
            Q_ASSERT(false);
            return pen;
    }

    switch (corner->getStroke())
    {
        case pdf::xfa::XFA_BaseNode::STROKE::Solid:
            pen.setStyle(Qt::SolidLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::DashDot:
            pen.setStyle(Qt::DashDotLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::DashDotDot:
            pen.setStyle(Qt::DashDotDotLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::Dashed:
            pen.setStyle(Qt::DashLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::Dotted:
            pen.setStyle(Qt::DotLine);
            break;
        case pdf::xfa::XFA_BaseNode::STROKE::Embossed:
        case pdf::xfa::XFA_BaseNode::STROKE::Etched:
        case pdf::xfa::XFA_BaseNode::STROKE::Lowered:
        case pdf::xfa::XFA_BaseNode::STROKE::Raised:
            pen.setStyle(Qt::SolidLine); // Ignore these line types
            errors << PDFRenderError(RenderErrorType::NotSupported, PDFTranslationContext::tr("XFA: special stroke is not supported."));
            break;
    }

    pen.setWidthF(corner->getThickness().getValuePt(nullptr));

    QColor color = createColor(corner->getColor());
    pen.setColor(color);
    return pen;
}

QPen PDFXFAEngineImpl::createPenFromEdge(const std::vector<xfa::XFA_Node<xfa::XFA_edge>>& edges,
                                         size_t index,
                                         QList<PDFRenderError>& errors) const
{
    if (index < edges.size())
    {
        return createPenFromEdge(edges[index].getValue(), errors);
    }
    else if (!edges.empty())
    {
        return createPenFromEdge(edges.back().getValue(), errors);
    }

    return QPen(Qt::NoPen);
}

QPen PDFXFAEngineImpl::createPenFromCorner(const std::vector<xfa::XFA_Node<xfa::XFA_corner>>& corners,
                                           size_t index,
                                           QList<PDFRenderError>& errors) const
{
    if (index < corners.size())
    {
        return createPenFromCorner(corners[index].getValue(), errors);
    }
    else if (!corners.empty())
    {
        return createPenFromCorner(corners.back().getValue(), errors);
    }

    return QPen(Qt::NoPen);
}

template<typename Node>
PDFXFALayoutEngine::SizeInfo PDFXFALayoutEngine::getSizeInfo(const Node* node) const
{
    SizeInfo info;
    info.origSize = getSizeFromMeasurement(node->getW(), node->getH());
    info.minSize = getSizeFromMeasurement(node->getMinW(), node->getMinH());
    info.maxSize = getSizeFromMeasurement(node->getMaxW(), node->getMaxH());
    info.effSize = info.origSize;

    info.effSize.setWidth(qMax(info.effSize.width(), info.minSize.width()));
    info.effSize.setHeight(qMax(info.effSize.height(), info.minSize.height()));

    if (!qFuzzyIsNull(info.maxSize.width()))
    {
        info.effSize.setWidth(qMin(info.effSize.width(), info.maxSize.width()));
    }

    if (!qFuzzyIsNull(info.maxSize.height()))
    {
        info.effSize.setHeight(qMin(info.effSize.height(), info.maxSize.height()));
    }

    return info;
}

QSizeF PDFXFALayoutEngine::SizeInfo::adjustNominalExtentSize(const QSizeF size) const
{
    PDFReal minW = origSize.width();
    PDFReal minH = origSize.height();

    if (qFuzzyIsNull(minW))
    {
        minW = minSize.width();
    }

    if (qFuzzyIsNull(minH))
    {
        minH = minSize.height();
    }

    PDFReal maxW = origSize.width();
    PDFReal maxH = origSize.height();

    if (qFuzzyIsNull(maxW))
    {
        maxW = maxSize.width();
    }

    if (qFuzzyIsNull(maxH))
    {
        maxH = maxSize.height();
    }

    if (qFuzzyIsNull(maxW))
    {
        maxW = size.width();
    }

    if (qFuzzyIsNull(maxH))
    {
        maxH = size.height();
    }

    PDFReal correctedWidth = qBound(minW, size.width(), maxW);
    PDFReal correctedHeight = qBound(minH, size.height(), maxH);

    return QSizeF(correctedWidth, correctedHeight);
}

QSizeF PDFXFALayoutEngine::SizeInfo::getSizeForLayout() const
{
    return adjustNominalExtentSize(maxSize);
}

}   // namespace pdf
