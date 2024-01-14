//    Copyright (C) 2019-2021 Jakub Melka
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


#ifndef PDFOPTIONALCONTENT_H
#define PDFOPTIONALCONTENT_H

#include "pdfobject.h"

namespace pdf
{

class PDFDocument;
class PDFOptionalContentActivity;
class PDFOptionalContentProperties;
class PDFOptionalContentConfiguration;

/// State of the optional content group, or result of expression
enum class OCState
{
    ON,
    OFF,
    Unknown
};

/// Type of optional content usage
enum class OCUsage
{
    View,
    Print,
    Export,
    Invalid
};

constexpr OCState operator &(OCState left, OCState right)
{
    if (left == OCState::Unknown)
    {
        return right;
    }
    if (right == OCState::Unknown)
    {
        return left;
    }

    return (left == OCState::ON && right == OCState::ON) ? OCState::ON : OCState::OFF;
}

constexpr OCState operator |(OCState left, OCState right)
{
    if (left == OCState::Unknown)
    {
        return right;
    }
    if (right == OCState::Unknown)
    {
        return left;
    }

    return (left == OCState::ON || right == OCState::ON) ? OCState::ON : OCState::OFF;
}

/// Object describing optional content membership dictionary
class PDFOptionalContentMembershipObject
{
public:
    explicit PDFOptionalContentMembershipObject() = default;

    inline PDFOptionalContentMembershipObject(const PDFOptionalContentMembershipObject&) = delete;
    inline PDFOptionalContentMembershipObject(PDFOptionalContentMembershipObject&&) = default;
    inline PDFOptionalContentMembershipObject& operator=(const PDFOptionalContentMembershipObject&) = delete;
    inline PDFOptionalContentMembershipObject& operator=(PDFOptionalContentMembershipObject&&) = default;

    /// Creates optional content membership dictionary. If creation fails, then
    /// exception is thrown.
    /// \param document Document owning the membership dictionary
    /// \param object Object to be parsed
    static PDFOptionalContentMembershipObject create(const PDFDocument* document, const PDFObject& object);

    /// Returns true, if this object is valid
    bool isValid() const { return static_cast<bool>(m_expression); }

    /// Evaluate objects. If error occurs, then Uknown state is returned.
    /// \param activity Activity
    OCState evaluate(const PDFOptionalContentActivity* activity) const;

private:

    enum class Operator
    {
        And,
        Or,
        Not
    };

    /// Node in the expression tree
    class Node
    {
    public:
        inline explicit Node() = default;
        virtual ~Node() = default;

        virtual OCState evaluate(const PDFOptionalContentActivity* activity) const = 0;
    };

    /// Node reprsenting optional content group
    class OptionalContentGroupNode : public Node
    {
    public:
        inline explicit OptionalContentGroupNode(PDFObjectReference optionalContentGroup) :
            m_optionalContentGroup(optionalContentGroup)
        {

        }

        virtual OCState evaluate(const PDFOptionalContentActivity* activity) const override;

    private:
        PDFObjectReference m_optionalContentGroup;
    };

    /// Node representing operator
    class OperatorNode : public Node
    {
    public:
        inline explicit OperatorNode(Operator operator_, std::vector<std::unique_ptr<Node>>&& nodes) :
            m_operator(operator_),
            m_children(qMove(nodes))
        {

        }

        virtual OCState evaluate(const PDFOptionalContentActivity* activity) const override;

    private:
        Operator m_operator;
        std::vector<std::unique_ptr<Node>> m_children;
    };

    std::unique_ptr<Node> m_expression;
};

/// Activeness of the optional content
class PDF4QTLIBCORESHARED_EXPORT PDFOptionalContentActivity : public QObject
{
    Q_OBJECT

public:
    explicit PDFOptionalContentActivity(const PDFDocument* document, OCUsage usage, QObject* parent);

    /// Gets the optional content groups state. If optional content group doesn't exist,
    /// then it returns Unknown state.
    /// \param ocg Optional content group
    OCState getState(PDFObjectReference ocg) const;

    /// Sets document to this object. Optional content settings
    /// must be compatible and applicable to new document.
    /// \param document Document
    void setDocument(const PDFDocument* document);

    /// Sets the state of optional content group. If optional content group doesn't exist,
    /// then nothing happens. If optional content group is contained in radio button group, then
    /// all other optional content groups in the group are switched off, if we are
    /// switching this one to ON state. If we are switching it off, then nothing happens (as all
    /// optional content groups in radio button group can be switched off). This behaviour can be
    /// controlled via parameter \p preserveRadioButtons.
    /// \param ocg Optional content group
    /// \param state New state of the optional content group
    /// \param preserveRadioButtons Switch off other radio buttons in group?
    /// \note If something changed, then signal \p optionalContentGroupStateChanged is emitted.
    void setState(PDFObjectReference ocg, OCState state, bool preserveRadioButtons = true);

    /// Applies configuration to the current state of optional content groups
    void applyConfiguration(const PDFOptionalContentConfiguration& configuration);

    /// Returns the properties of optional content
    const PDFOptionalContentProperties* getProperties() const { return m_properties; }

signals:
    void optionalContentGroupStateChanged(PDFObjectReference ocg, OCState state);

private:
    const PDFDocument* m_document;
    const PDFOptionalContentProperties* m_properties;
    OCUsage m_usage;
    std::map<PDFObjectReference, OCState> m_states;
};

/// Configuration of optional content configuration.
class PDFOptionalContentConfiguration
{
public:

    enum class BaseState
    {
        ON,
        OFF,
        Unchanged
    };

    enum class ListMode
    {
        AllPages,
        VisiblePages
    };

    struct UsageApplication
    {
        QByteArray event;
        std::vector<PDFObjectReference> optionalContentGroups;
        std::vector<QByteArray> categories;
    };

    /// Creates new optional content properties configuration from the object. If object is not valid,
    /// then exception is thrown.
    /// \param document Document
    /// \param object Object containing documents optional content configuration
    static PDFOptionalContentConfiguration create(const PDFDocument* document, const PDFObject& object);

    /// Converts usage name to the enum. If value can't be converted, then Invalid usage is returned.
    /// \param name Name of the usage
    static OCUsage getUsageFromName(const QByteArray& name);

    const QString& getName() const { return m_name; }
    const QString& getCreator() const { return m_creator; }
    BaseState getBaseState() const { return m_baseState; }
    const std::vector<PDFObjectReference>& getOnArray() const { return m_OnArray; }
    const std::vector<PDFObjectReference>& getOffArray() const { return m_OffArray; }
    const std::vector<QByteArray>& getIntents() const { return m_intents; }
    const std::vector<UsageApplication>& getUsageApplications() const { return m_usageApplications; }
    const PDFObject& getOrder() const { return m_order; }
    ListMode getListMode() const { return m_listMode; }
    const std::vector<std::vector<PDFObjectReference>>& getRadioButtonGroups() const { return m_radioButtonGroups; }
    const std::vector<PDFObjectReference>& getLocked() const { return m_locked; }

private:
    /// Creates usage application
    /// \param document Document
    /// \param object Object containing usage application
    static UsageApplication createUsageApplication(const PDFDocument* document, const PDFObject& object);

    QString m_name;
    QString m_creator;
    BaseState m_baseState = BaseState::ON;
    std::vector<PDFObjectReference> m_OnArray;
    std::vector<PDFObjectReference> m_OffArray;
    std::vector<QByteArray> m_intents;
    std::vector<UsageApplication> m_usageApplications;
    PDFObject m_order;
    ListMode m_listMode = ListMode::AllPages;
    std::vector<std::vector<PDFObjectReference>> m_radioButtonGroups;
    std::vector<PDFObjectReference> m_locked;
};

/// Class reprezenting optional content group - it contains properties of the group,
/// such as name, usage etc.
class PDFOptionalContentGroup
{
public:
    explicit PDFOptionalContentGroup();

    /// Creates optional content group from the object. Object must be valid optional
    /// content group, if it is invalid, then exception is thrown.
    /// \param document Document
    /// \param object Object containing optional content group
    static PDFOptionalContentGroup create(const PDFDocument* document, const PDFObject& object);

    PDFObjectReference getReference() const { return m_reference; }
    const QString& getName() const { return m_name; }
    const std::vector<QByteArray>& getIntents() const { return m_intents; }
    const PDFObject& getCreatorInfo() const { return m_creatorInfo; }
    const QString& getCreator() const { return m_creator; }
    const QByteArray& getSubtype() const { return m_subtype; }
    const QString& getLanguage() const { return m_language; }
    bool isLanguagePreferred() const { return m_languagePreferred; }
    PDFReal getUsageZoomMin() const { return m_usageZoomMin; }
    PDFReal getUsageZoomMax() const { return m_usageZoomMax; }
    OCState getUsagePrintState() const { return m_usagePrintState; }
    OCState getUsageViewState() const { return m_usageViewState; }
    OCState getUsageExportState() const { return m_usageExportState; }
    OCState getUsageState(OCUsage usage) const;
    const QByteArray& getUserType() const { return m_userType; }
    const QStringList& getUserNames() const { return m_userNames; }
    const PDFObject& getPageElement() const { return m_pageElement; }

private:
    PDFObjectReference m_reference;
    QString m_name;
    std::vector<QByteArray> m_intents;
    PDFObject m_creatorInfo;
    QString m_creator;
    QByteArray m_subtype;
    QString m_language;
    QByteArray m_userType;
    QStringList m_userNames;
    bool m_languagePreferred;
    PDFReal m_usageZoomMin;
    PDFReal m_usageZoomMax;
    OCState m_usagePrintState;
    OCState m_usageViewState;
    OCState m_usageExportState;
    PDFObject m_pageElement;
};

/// Object containing properties of the optional content of the PDF document. It contains
/// for example all documents optional content groups.
class PDFOptionalContentProperties
{
public:
    explicit PDFOptionalContentProperties() = default;

    /// Returns, if object is valid - at least one optional content group exists
    bool isValid() const { return !m_allOptionalContentGroups.empty() && m_allOptionalContentGroups.size() == m_optionalContentGroups.size(); }

    /// Creates new optional content properties from the object. If object is not valid,
    /// then exception is thrown.
    /// \param document Document
    /// \param object Object containing documents optional content properties
    static PDFOptionalContentProperties create(const PDFDocument* document, const PDFObject& object);

    const std::vector<PDFObjectReference>& getAllOptionalContentGroups() const { return m_allOptionalContentGroups; }
    const PDFOptionalContentConfiguration& getDefaultConfiguration() const { return m_defaultConfiguration; }
    const PDFOptionalContentGroup& getOptionalContentGroup(PDFObjectReference reference) const { return m_optionalContentGroups.at(reference); }

    /// Returns true, if optional content group exists
    bool hasOptionalContentGroup(PDFObjectReference reference) const { return m_optionalContentGroups.count(reference); }

private:
    std::vector<PDFObjectReference> m_allOptionalContentGroups;
    PDFOptionalContentConfiguration m_defaultConfiguration;
    std::vector<PDFOptionalContentConfiguration> m_configurations;
    std::map<PDFObjectReference, PDFOptionalContentGroup> m_optionalContentGroups;
};

}   // namespace pdf

#endif // PDFOPTIONALCONTENT_H
