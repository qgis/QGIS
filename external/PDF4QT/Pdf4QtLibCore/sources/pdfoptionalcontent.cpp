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


#include "pdfoptionalcontent.h"
#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFOptionalContentProperties PDFOptionalContentProperties::create(const PDFDocument* document, const PDFObject& object)
{
    PDFOptionalContentProperties properties;

    const PDFObject& dereferencedObject = document->getObject(object);
    if (dereferencedObject.isDictionary())
    {
        const PDFDictionary* dictionary = dereferencedObject.getDictionary();
        PDFDocumentDataLoaderDecorator loader(document);
        properties.m_allOptionalContentGroups = loader.readReferenceArrayFromDictionary(dictionary, "OCGs");

        for (const PDFObjectReference& reference : properties.m_allOptionalContentGroups)
        {
            const PDFObject& currentObject = document->getStorage().getObject(reference);
            if (!currentObject.isNull())
            {
                properties.m_optionalContentGroups[reference] = PDFOptionalContentGroup::create(document, currentObject);
            }
        }

        if (dictionary->hasKey("D"))
        {
            properties.m_defaultConfiguration = PDFOptionalContentConfiguration::create(document, dictionary->get("D"));
        }

        if (dictionary->hasKey("Configs"))
        {
            const PDFObject& configsObject = document->getObject(dictionary->get("Configs"));
            if (configsObject.isArray())
            {
                const PDFArray* configsArray = configsObject.getArray();
                properties.m_configurations.reserve(configsArray->getCount());

                for (size_t i = 0, count = configsArray->getCount(); i < count; ++i)
                {
                    properties.m_configurations.emplace_back(PDFOptionalContentConfiguration::create(document, configsArray->getItem(i)));
                }
            }
            else if (!configsObject.isNull())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid optional content properties."));
            }
        }
    }
    else if (!dereferencedObject.isNull())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid optional content properties."));
    }

    return properties;
}

PDFOptionalContentConfiguration PDFOptionalContentConfiguration::create(const PDFDocument* document, const PDFObject& object)
{
    PDFOptionalContentConfiguration configuration;

    const PDFObject& dereferencedObject = document->getObject(object);
    if (dereferencedObject.isDictionary())
    {
        const PDFDictionary* dictionary = dereferencedObject.getDictionary();
        PDFDocumentDataLoaderDecorator loader(document);
        configuration.m_name = loader.readTextStringFromDictionary(dictionary, "Name", QString());
        configuration.m_creator = loader.readTextStringFromDictionary(dictionary, "Creator", QString());

        constexpr const std::array<std::pair<const char*, BaseState>, 3> baseStateEnumValues = {
            std::pair<const char*, BaseState>{ "ON", BaseState::ON },
            std::pair<const char*, BaseState>{ "OFF", BaseState::OFF },
            std::pair<const char*, BaseState>{ "Unchanged", BaseState::Unchanged }
        };
        configuration.m_baseState = loader.readEnumByName(dictionary->get("BaseState"), baseStateEnumValues.cbegin(), baseStateEnumValues.cend(), BaseState::ON);
        configuration.m_OnArray = loader.readReferenceArrayFromDictionary(dictionary, "ON");
        configuration.m_OffArray = loader.readReferenceArrayFromDictionary(dictionary, "OFF");

        if (dictionary->hasKey("Intent"))
        {
            const PDFObject& nameOrNames = document->getObject(dictionary->get("Intent"));

            if (nameOrNames.isName())
            {
                configuration.m_intents = { loader.readName(nameOrNames) };
            }
            else if (nameOrNames.isArray())
            {
                configuration.m_intents = loader.readNameArray(nameOrNames);
            }
            else if (!nameOrNames.isNull())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid optional content configuration."));
            }
        }

        if (dictionary->hasKey("AS"))
        {
            const PDFObject& asArrayObject = document->getObject(dictionary->get("AS"));
            if (asArrayObject.isArray())
            {
                const PDFArray* asArray = asArrayObject.getArray();
                configuration.m_usageApplications.reserve(asArray->getCount());

                for (size_t i = 0, count = asArray->getCount(); i < count; ++i)
                {
                    configuration.m_usageApplications.emplace_back(createUsageApplication(document, asArray->getItem(i)));
                }
            }
            else if (!asArrayObject.isNull())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid optional content configuration."));
            }
        }

        configuration.m_order = document->getObject(dictionary->get("Order"));
        if (!configuration.m_order.isArray() && !configuration.m_order.isNull())
        {
            throw PDFException(PDFTranslationContext::tr("Invalid optional content configuration."));
        }

        constexpr const std::array<std::pair<const char*, ListMode>, 3> listModeEnumValues = {
            std::pair<const char*, ListMode>{ "AllPages", ListMode::AllPages },
            std::pair<const char*, ListMode>{ "VisiblePages", ListMode::VisiblePages }
        };
        configuration.m_listMode = loader.readEnumByName(dictionary->get("ListMode"), listModeEnumValues.cbegin(), listModeEnumValues.cend(), ListMode::AllPages);

        if (dictionary->hasKey("RBGroups"))
        {
            const PDFObject& rbGroupsObject = document->getObject(dictionary->get("RBGroups"));
            if (rbGroupsObject.isArray())
            {
                const PDFArray* rbGroupsArray = rbGroupsObject.getArray();
                configuration.m_radioButtonGroups.reserve(rbGroupsArray->getCount());

                for (size_t i = 0, count = rbGroupsArray->getCount(); i < count; ++i)
                {
                    configuration.m_radioButtonGroups.emplace_back(loader.readReferenceArray(rbGroupsArray->getItem(i)));
                }
            }
            else if (!rbGroupsObject.isNull())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid optional content configuration."));
            }
        }

        configuration.m_locked = loader.readReferenceArrayFromDictionary(dictionary, "Locked");
    }

    return configuration;
}

OCUsage PDFOptionalContentConfiguration::getUsageFromName(const QByteArray& name)
{
    if (name == "View")
    {
        return OCUsage::View;
    }
    else if (name == "Print")
    {
        return OCUsage::Print;
    }
    else if (name == "Export")
    {
        return OCUsage::Export;
    }

    return OCUsage::Invalid;
}

PDFOptionalContentConfiguration::UsageApplication PDFOptionalContentConfiguration::createUsageApplication(const PDFDocument* document, const PDFObject& object)
{
    UsageApplication result;

    const PDFObject& dereferencedObject = document->getObject(object);
    if (dereferencedObject.isDictionary())
    {
        PDFDocumentDataLoaderDecorator loader(document);
        const PDFDictionary* dictionary = dereferencedObject.getDictionary();
        result.event = loader.readNameFromDictionary(dictionary, "Event");
        result.optionalContentGroups = loader.readReferenceArrayFromDictionary(dictionary, "OCGs");
        result.categories = loader.readNameArrayFromDictionary(dictionary, "Category");
    }

    return result;
}

PDFOptionalContentGroup::PDFOptionalContentGroup() :
    m_languagePreferred(false),
    m_usageZoomMin(0),
    m_usageZoomMax(std::numeric_limits<PDFReal>::infinity()),
    m_usagePrintState(OCState::Unknown),
    m_usageViewState(OCState::Unknown),
    m_usageExportState(OCState::Unknown)
{

}

PDFOptionalContentGroup PDFOptionalContentGroup::create(const PDFDocument* document, const PDFObject& object)
{
    PDFOptionalContentGroup result;

    const PDFObject& dereferencedObject = document->getObject(object);
    if (!dereferencedObject.isDictionary())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid optional content group."));
    }

    PDFDocumentDataLoaderDecorator loader(document);

    const PDFDictionary* dictionary = dereferencedObject.getDictionary();
    result.m_name = loader.readTextStringFromDictionary(dictionary, "Name", QString());

    if (dictionary->hasKey("Intent"))
    {
        const PDFObject& nameOrNames = document->getObject(dictionary->get("Intent"));

        if (nameOrNames.isName())
        {
            result.m_intents = { loader.readName(nameOrNames) };
        }
        else if (nameOrNames.isArray())
        {
            result.m_intents = loader.readNameArray(nameOrNames);
        }
        else if (!nameOrNames.isNull())
        {
            throw PDFException(PDFTranslationContext::tr("Invalid optional content group."));
        }
    }

    const PDFObject& usageDictionaryObject = dictionary->get("Usage");
    if (usageDictionaryObject.isDictionary())
    {
        const PDFDictionary* usageDictionary = usageDictionaryObject.getDictionary();

        if (const PDFDictionary* creatorDictionary = document->getDictionaryFromObject(usageDictionary->get("CreatorInfo")))
        {
            result.m_creator = loader.readTextStringFromDictionary(creatorDictionary, "Creator", QString());
            result.m_subtype = loader.readNameFromDictionary(creatorDictionary, "Subtype");
        }
        result.m_creatorInfo = document->getObject(usageDictionary->get("CreatorInfo"));

        if (const PDFDictionary* languageDictionary = document->getDictionaryFromObject(usageDictionary->get("Language")))
        {
            result.m_language = loader.readTextStringFromDictionary(languageDictionary, "Lang", QString());
            result.m_languagePreferred = loader.readNameFromDictionary(languageDictionary, "Preferred") == "ON";
        }

        if (const PDFDictionary* zoomDictionary = document->getDictionaryFromObject(usageDictionary->get("Zoom")))
        {
            result.m_usageZoomMin = loader.readNumberFromDictionary(zoomDictionary, "min", result.m_usageZoomMin);
            result.m_usageZoomMax = loader.readNumberFromDictionary(zoomDictionary, "max", result.m_usageZoomMax);
        }

        auto readState = [document, usageDictionary, &loader](const char* dictionaryKey, const char* key) -> OCState
        {
            const PDFObject& stateDictionaryObject = document->getObject(usageDictionary->get(dictionaryKey));
            if (stateDictionaryObject.isDictionary())
            {
                const PDFDictionary* stateDictionary = stateDictionaryObject.getDictionary();
                QByteArray stateName = loader.readNameFromDictionary(stateDictionary, key);

                if (stateName == "ON")
                {
                    return OCState::ON;
                }
                if (stateName == "OFF")
                {
                    return OCState::OFF;
                }
            }

            return OCState::Unknown;
        };

        result.m_usageViewState = readState("View", "ViewState");
        result.m_usagePrintState = readState("Print", "PrintState");
        result.m_usageExportState = readState("Export", "ExportState");

        if (const PDFDictionary* userDictionary = document->getDictionaryFromObject(usageDictionary->get("User")))
        {
            result.m_userType = loader.readNameFromDictionary(userDictionary, "Type");

            PDFObject namesObject = document->getObject(userDictionary->get("Name"));
            if (namesObject.isArray())
            {
                result.m_userNames = loader.readTextStringList(userDictionary->get("Name"));
            }
            else
            {
                QString name = loader.readStringFromDictionary(userDictionary, "Name");
                if (!name.isEmpty())
                {
                    result.m_userNames.append(qMove(name));
                }
            }
        }

        result.m_pageElement = usageDictionary->get("PageElement");
    }

    return result;
}

OCState PDFOptionalContentGroup::getUsageState(OCUsage usage) const
{
    switch (usage)
    {
        case OCUsage::View:
            return getUsageViewState();

        case OCUsage::Print:
            return getUsagePrintState();

        case OCUsage::Export:
            return getUsageExportState();

        case OCUsage::Invalid:
            break;

        default:
            break;
    }

    return OCState::Unknown;
}

PDFOptionalContentActivity::PDFOptionalContentActivity(const PDFDocument* document, OCUsage usage, QObject* parent) :
    QObject(parent),
    m_document(document),
    m_properties(document->getCatalog()->getOptionalContentProperties()),
    m_usage(usage)
{
    if (m_properties->isValid())
    {
        for (const PDFObjectReference& reference : m_properties->getAllOptionalContentGroups())
        {
            m_states[reference] = OCState::Unknown;
        }

        applyConfiguration(m_properties->getDefaultConfiguration());
    }
}

OCState PDFOptionalContentActivity::getState(PDFObjectReference ocg) const
{
    auto it = m_states.find(ocg);
    if (it != m_states.cend())
    {
        return it->second;
    }

    return OCState::Unknown;
}

void PDFOptionalContentActivity::setDocument(const PDFDocument* document)
{
    if (m_document != document)
    {
        Q_ASSERT(document);
        m_document = document;
        m_properties = document->getCatalog()->getOptionalContentProperties();
    }
}

void PDFOptionalContentActivity::setState(PDFObjectReference ocg, OCState state, bool preserveRadioButtons)
{
    auto it = m_states.find(ocg);
    if (it != m_states.cend() && it->second != state)
    {
        // We are changing the state. If new state is ON, then we must check radio button groups.
        if (state == OCState::ON && preserveRadioButtons)
        {
            for (const std::vector<PDFObjectReference>& radioButtonGroup : m_properties->getDefaultConfiguration().getRadioButtonGroups())
            {
                if (std::find(radioButtonGroup.cbegin(), radioButtonGroup.cend(), ocg) != radioButtonGroup.cend())
                {
                    // We must set all states of this radio button group to OFF
                    for (const PDFObjectReference& ocgRadioButtonGroup : radioButtonGroup)
                    {
                        setState(ocgRadioButtonGroup, OCState::OFF);
                    }
                }
            }
        }

        it->second = state;
        Q_EMIT optionalContentGroupStateChanged(ocg, state);
    }
}

void PDFOptionalContentActivity::applyConfiguration(const PDFOptionalContentConfiguration& configuration)
{
    // Step 1: Apply base state to all states
    if (configuration.getBaseState() != PDFOptionalContentConfiguration::BaseState::Unchanged)
    {
        const OCState newState = (configuration.getBaseState() == PDFOptionalContentConfiguration::BaseState::ON) ? OCState::ON : OCState::OFF;
        for (auto& item : m_states)
        {
            item.second = newState;
        }
    }

    auto setOCGState = [this](PDFObjectReference ocg, OCState state)
    {
        auto it = m_states.find(ocg);
        if (it != m_states.cend())
        {
            it->second = state;
        }
    };

    // Step 2: Process 'ON' entry
    for (PDFObjectReference ocg : configuration.getOnArray())
    {
        setOCGState(ocg, OCState::ON);
    }

    // Step 3: Process 'OFF' entry
    for (PDFObjectReference ocg : configuration.getOffArray())
    {
        setOCGState(ocg, OCState::OFF);
    }

    // Step 4: Apply usage
    for (const PDFOptionalContentConfiguration::UsageApplication& usageApplication : configuration.getUsageApplications())
    {
        // We will use usage from the events name. We ignore category, as it should duplicate the events name.
        const OCUsage usage = PDFOptionalContentConfiguration::getUsageFromName(usageApplication.event);

        if (usage == m_usage)
        {
            for (PDFObjectReference ocg : usageApplication.optionalContentGroups)
            {
                if (!m_properties->hasOptionalContentGroup(ocg))
                {
                    continue;
                }

                const PDFOptionalContentGroup& optionalContentGroup = m_properties->getOptionalContentGroup(ocg);
                const OCState newState = optionalContentGroup.getUsageState(usage);
                setOCGState(ocg, newState);
            }
        }
    }
}

PDFOptionalContentMembershipObject PDFOptionalContentMembershipObject::create(const PDFDocument* document, const PDFObject& object)
{
    PDFOptionalContentMembershipObject result;
    const PDFObject& dereferencedObject = document->getObject(object);
    if (dereferencedObject.isDictionary())
    {
        const PDFDictionary* dictionary = dereferencedObject.getDictionary();
        if (dictionary->hasKey("VE"))
        {
            // Parse visibility expression

            std::set<PDFObjectReference> usedReferences;
            std::function<std::unique_ptr<Node>(const PDFObject&)> parseNode = [document, &parseNode, &usedReferences](const PDFObject& nodeObject) -> std::unique_ptr<Node>
            {
                const PDFObject& dereferencedNodeObject = document->getObject(nodeObject);
                if (dereferencedNodeObject.isArray())
                {
                    // It is probably array. We must check, if we doesn't have cyclic reference.
                    if (nodeObject.isReference())
                    {
                        if (usedReferences.count(nodeObject.getReference()))
                        {
                            throw PDFException(PDFTranslationContext::tr("Cyclic reference error in optional content visibility expression."));
                        }
                        else
                        {
                            usedReferences.insert(nodeObject.getReference());
                        }
                    }

                    // Check the array
                    const PDFArray* array = dereferencedNodeObject.getArray();
                    if (array->getCount() < 2)
                    {
                        throw PDFException(PDFTranslationContext::tr("Invalid optional content visibility expression."));
                    }

                    // Read the operator
                    const PDFObject& dereferencedNameObject = document->getObject(array->getItem(0));
                    QByteArray operatorName;
                    if (dereferencedNameObject.isName())
                    {
                        operatorName = dereferencedNameObject.getString();
                    }

                    Operator operatorType = Operator::And;
                    if (operatorName == "And")
                    {
                        operatorType = Operator::And;
                    } else if (operatorName == "Or")
                    {
                        operatorType = Operator::Or;
                    }
                    else if (operatorName == "Not")
                    {
                        operatorType = Operator::Not;
                    }
                    else
                    {
                        throw PDFException(PDFTranslationContext::tr("Invalid optional content visibility expression."));
                    }

                    // Read the operands
                    std::vector<std::unique_ptr<Node>> operands;
                    operands.reserve(array->getCount());
                    for (size_t i = 1, count = array->getCount(); i < count; ++i)
                    {
                        operands.push_back(parseNode(array->getItem(i)));
                    }

                    return std::unique_ptr<Node>(new OperatorNode(operatorType, qMove(operands)));
                }
                else if (nodeObject.isReference())
                {
                    // Treat is as an optional content group
                    return std::unique_ptr<Node>(new OptionalContentGroupNode(nodeObject.getReference()));
                }
                else
                {
                    // Something strange occured - either we should have an array, or we should have a reference to the OCG
                    throw PDFException(PDFTranslationContext::tr("Invalid optional content visibility expression."));
                }
            };

            result.m_expression = parseNode(dictionary->get("VE"));
        }
        else
        {
            // First, scan all optional content groups
            PDFDocumentDataLoaderDecorator loader(document);
            std::vector<PDFObjectReference> ocgs;

            PDFObject singleOCG = dictionary->get("OCGs");
            if (singleOCG.isReference())
            {
                ocgs = { singleOCG.getReference() };
            }
            else
            {
                ocgs = loader.readReferenceArrayFromDictionary(dictionary, "OCGs");
            }

            if (!ocgs.empty())
            {
                auto createOperatorOnOcgs = [&ocgs](Operator operator_)
                {
                    std::vector<std::unique_ptr<Node>> operands;
                    operands.reserve(ocgs.size());
                    for (PDFObjectReference reference : ocgs)
                    {
                        operands.push_back(std::unique_ptr<Node>(new OptionalContentGroupNode(reference)));
                    }
                    return std::unique_ptr<Node>(new OperatorNode(operator_, qMove(operands)));
                };

                // Parse 'P' mode
                QByteArray type = loader.readNameFromDictionary(dictionary, "P");
                if (type == "AllOn")
                {
                    // All of entries in OCGS are turned on
                    result.m_expression = createOperatorOnOcgs(Operator::And);
                }
                else if (type == "AnyOn")
                {
                    // Any of entries in OCGS is turned on
                    result.m_expression = createOperatorOnOcgs(Operator::Or);
                }
                else if (type == "AnyOff")
                {
                    // Any of entries are turned off. It is negation of 'AllOn'.
                    std::vector<std::unique_ptr<Node>> subexpression;
                    subexpression.push_back(createOperatorOnOcgs(Operator::And));
                    result.m_expression = std::unique_ptr<Node>(new OperatorNode(Operator::Not, qMove(subexpression)));
                }
                else if (type == "AllOff")
                {
                    // All of entries are turned off. It is negation of 'AnyOn'
                    std::vector<std::unique_ptr<Node>> subexpression;
                    subexpression.push_back(createOperatorOnOcgs(Operator::Or));
                    result.m_expression = std::unique_ptr<Node>(new OperatorNode(Operator::Not, qMove(subexpression)));
                }
                else
                {
					// Default value is AnyOn according to the PDF reference
					result.m_expression = createOperatorOnOcgs(Operator::Or);
                }
            }
        }
    }

    return result;
}

OCState PDFOptionalContentMembershipObject::evaluate(const PDFOptionalContentActivity* activity) const
{
    return m_expression ? m_expression->evaluate(activity) : OCState::Unknown;
}

OCState PDFOptionalContentMembershipObject::OptionalContentGroupNode::evaluate(const PDFOptionalContentActivity* activity) const
{
    return activity->getState(m_optionalContentGroup);
}

OCState PDFOptionalContentMembershipObject::OperatorNode::evaluate(const PDFOptionalContentActivity* activity) const
{
    OCState result = OCState::Unknown;

    switch (m_operator)
    {
        case Operator::And:
        {
            for (const auto& child : m_children)
            {
                result = result & child->evaluate(activity);
            }
            break;
        }

        case Operator::Or:
        {
            for (const auto& child : m_children)
            {
                result = result | child->evaluate(activity);
            }
            break;
        }

        case Operator::Not:
        {
            // We must handle case, when we have zero or more expressions (Not operator requires exactly one).
            // If this case occurs, then we return Unknown state.
            if (m_children.size() == 1)
            {
                OCState childState = m_children.front()->evaluate(activity);

                switch (childState)
                {
                    case OCState::ON:
                        result = OCState::OFF;
                        break;

                    case OCState::OFF:
                        result = OCState::ON;
                        break;

                    default:
                        Q_ASSERT(result == OCState::Unknown);
                        break;
                }
            }
            break;
        }

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return result;
}

}   // namespace pdf
