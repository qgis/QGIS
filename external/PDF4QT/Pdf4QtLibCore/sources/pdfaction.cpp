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

#include "pdfaction.h"
#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfencoding.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFActionPtr PDFAction::parse(const PDFObjectStorage* storage, PDFObject object)
{
    std::set<PDFObjectReference> usedReferences;
    return parseImpl(storage, qMove(object), usedReferences);
}

void PDFAction::apply(const std::function<void (const PDFAction*)>& callback)
{
    callback(this);

    for (const PDFActionPtr& nextAction : m_nextActions)
    {
        nextAction->apply(callback);
    }
}

std::vector<const PDFAction*> PDFAction::getActionList() const
{
    std::vector<const PDFAction*> result;
    fillActionList(result);
    return result;
}

PDFActionPtr PDFAction::parseImpl(const PDFObjectStorage* storage, PDFObject object, std::set<PDFObjectReference>& usedReferences)
{
    if (object.isReference())
    {
        PDFObjectReference reference = object.getReference();
        if (usedReferences.count(reference))
        {
            throw PDFException(PDFTranslationContext::tr("Circular dependence in actions found."));
        }
        usedReferences.insert(reference);
        object = storage->getObjectByReference(reference);
    }

    if (object.isNull())
    {
        return PDFActionPtr();
    }

    if (!object.isDictionary())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid action."));
    }

    PDFDocumentDataLoaderDecorator loader(storage);
    const PDFDictionary* dictionary = object.getDictionary();
    QByteArray name = loader.readNameFromDictionary(dictionary, "S");

    if (name == "GoTo") // Goto action
    {
        PDFDestination destination = PDFDestination::parse(storage, dictionary->get("D"));
        PDFDestination structureDestination = PDFDestination::parse(storage, dictionary->get("SD"));
        return PDFActionPtr(new PDFActionGoTo(qMove(destination), qMove(structureDestination)));
    }
    else if (name == "GoToR")
    {
        PDFDestination destination = PDFDestination::parse(storage, dictionary->get("D"));
        PDFDestination structureDestination = PDFDestination::parse(storage, dictionary->get("SD"));
        PDFFileSpecification fileSpecification = PDFFileSpecification::parse(storage, dictionary->get("F"));
        return PDFActionPtr(new PDFActionGoToR(qMove(destination), qMove(structureDestination), qMove(fileSpecification), loader.readBooleanFromDictionary(dictionary, "NewWindow", false)));
    }
    else if (name == "GoToE")
    {
        PDFDestination destination = PDFDestination::parse(storage, dictionary->get("D"));
        PDFFileSpecification fileSpecification = PDFFileSpecification::parse(storage, dictionary->get("F"));
        return PDFActionPtr(new PDFActionGoToE(qMove(destination), qMove(fileSpecification), loader.readBooleanFromDictionary(dictionary, "NewWindow", false), storage->getObject(dictionary->get("T"))));
    }
    else if (name == "GoToDp")
    {
        PDFObjectReference documentPart = loader.readReferenceFromDictionary(dictionary, "Dp");
        return PDFActionPtr(new PDFActionGoToDp(documentPart));
    }
    else if (name == "Launch")
    {
        PDFFileSpecification fileSpecification = PDFFileSpecification::parse(storage, dictionary->get("F"));
        const bool newWindow = loader.readBooleanFromDictionary(dictionary, "NewWindow", false);
        PDFActionLaunch::Win win;

        const PDFObject& winDictionaryObject = storage->getObject(dictionary->get("Win"));
        if (winDictionaryObject.isDictionary())
        {
            const PDFDictionary* winDictionary = winDictionaryObject.getDictionary();
            win.file = loader.readStringFromDictionary(winDictionary, "F");
            win.directory = loader.readStringFromDictionary(winDictionary, "D");
            win.operation = loader.readStringFromDictionary(winDictionary, "O");
            win.parameters = loader.readStringFromDictionary(winDictionary, "P");
        }

        return PDFActionPtr(new PDFActionLaunch(qMove(fileSpecification), newWindow, qMove(win)));
    }
    else if (name == "Thread")
    {
        PDFFileSpecification fileSpecification = PDFFileSpecification::parse(storage, dictionary->get("F"));
        PDFActionThread::Thread thread;
        PDFActionThread::Bead bead;

        const PDFObject& threadObject = dictionary->get("D");
        if (threadObject.isReference())
        {
            thread = threadObject.getReference();
        }
        else if (threadObject.isInt())
        {
            thread = threadObject.getInteger();
        }
        else if (threadObject.isString())
        {
            thread = PDFEncoding::convertTextString(threadObject.getString());
        }
        const PDFObject& beadObject = dictionary->get("B");
        if (beadObject.isReference())
        {
            bead = beadObject.getReference();
        }
        else if (beadObject.isInt())
        {
            bead = beadObject.getInteger();
        }

        return PDFActionPtr(new PDFActionThread(qMove(fileSpecification), qMove(thread), qMove(bead)));
    }
    else if (name == "URI")
    {
        return PDFActionPtr(new PDFActionURI(loader.readStringFromDictionary(dictionary, "URI"), loader.readBooleanFromDictionary(dictionary, "IsMap", false)));
    }
    else if (name == "Sound")
    {
        const PDFReal volume = loader.readNumberFromDictionary(dictionary, "Volume", 1.0);
        const bool isSynchronous = loader.readBooleanFromDictionary(dictionary, "Synchronous", false);
        const bool isRepeat = loader.readBooleanFromDictionary(dictionary, "Repeat", false);
        const bool isMix = loader.readBooleanFromDictionary(dictionary, "Mix", false);
        return PDFActionPtr(new PDFActionSound(PDFSound::parse(storage, dictionary->get("Sound")), volume, isSynchronous, isRepeat, isMix));
    }
    else if (name == "Movie")
    {
        constexpr const std::array<std::pair<const char*, PDFActionMovie::Operation>, 4> operations = {
            std::pair<const char*, PDFActionMovie::Operation>{ "Play", PDFActionMovie::Operation::Play },
            std::pair<const char*, PDFActionMovie::Operation>{ "Stop", PDFActionMovie::Operation::Stop },
            std::pair<const char*, PDFActionMovie::Operation>{ "Pause", PDFActionMovie::Operation::Pause },
            std::pair<const char*, PDFActionMovie::Operation>{ "Resume", PDFActionMovie::Operation::Resume }
        };

        // Jakub Melka: parse the movie action
        PDFObject annotationObject = dictionary->get("Annotation");
        PDFObjectReference annotation = annotationObject.isReference() ? annotationObject.getReference() : PDFObjectReference();
        QString title = loader.readTextStringFromDictionary(dictionary, "T", QString());
        PDFActionMovie::Operation operation = loader.readEnumByName(dictionary->get("Operation"), operations.cbegin(), operations.cend(), PDFActionMovie::Operation::Play);

        return PDFActionPtr(new PDFActionMovie(annotation, qMove(title), operation));
    }
    else if (name == "Hide")
    {
        std::vector<PDFObjectReference> annotations;
        std::vector<QString> fieldNames;

        const PDFObject& objectT = dictionary->get("T");
        if (objectT.isReference())
        {
            annotations = { objectT.getReference() };
        }
        else if (objectT.isString())
        {
            fieldNames = { loader.readTextString(objectT, QString()) };
        }
        else if (objectT.isArray())
        {
            const PDFArray* items = objectT.getArray();
            for (size_t i = 0; i < items->getCount(); ++i)
            {
                const PDFObject& itemObject = items->getItem(i);
                if (itemObject.isReference())
                {
                    annotations.push_back(itemObject.getReference());
                }
                else if (itemObject.isString())
                {
                    fieldNames.push_back(loader.readTextString(itemObject, QString()));
                }
            }
        }

        const bool hide = loader.readBooleanFromDictionary(dictionary, "H", true);
        return PDFActionPtr(new PDFActionHide(qMove(annotations), qMove(fieldNames), hide));
    }
    else if (name == "Named")
    {
        constexpr const std::array<std::pair<const char*, PDFActionNamed::NamedActionType>, 4> types = {
            std::pair<const char*, PDFActionNamed::NamedActionType>{ "NextPage", PDFActionNamed::NamedActionType::NextPage },
            std::pair<const char*, PDFActionNamed::NamedActionType>{ "PrevPage", PDFActionNamed::NamedActionType::PrevPage },
            std::pair<const char*, PDFActionNamed::NamedActionType>{ "FirstPage", PDFActionNamed::NamedActionType::FirstPage },
            std::pair<const char*, PDFActionNamed::NamedActionType>{ "LastPage", PDFActionNamed::NamedActionType::LastPage }
        };

        QByteArray actionNamed = loader.readNameFromDictionary(dictionary, "N");
        PDFActionNamed::NamedActionType actionType = loader.readEnumByName(dictionary->get("N"), types.cbegin(), types.cend(), PDFActionNamed::NamedActionType::Custom);
        return PDFActionPtr(new PDFActionNamed(actionType, qMove(actionNamed)));
    }
    else if (name == "SetOCGState")
    {
        const bool isRadioButtonsPreserved = loader.readBooleanFromDictionary(dictionary, "PreserveRB", true);
        PDFActionSetOCGState::StateChangeItems items;

        PDFObject stateArrayObject = storage->getObject(dictionary->get("State"));
        if (stateArrayObject.isArray())
        {
            constexpr const std::array<std::pair<const char*, PDFActionSetOCGState::SwitchType>, 3> types = {
                std::pair<const char*, PDFActionSetOCGState::SwitchType>{ "ON", PDFActionSetOCGState::SwitchType::ON },
                std::pair<const char*, PDFActionSetOCGState::SwitchType>{ "OFF", PDFActionSetOCGState::SwitchType::OFF },
                std::pair<const char*, PDFActionSetOCGState::SwitchType>{ "Toggle", PDFActionSetOCGState::SwitchType::Toggle }
            };

            PDFActionSetOCGState::SwitchType switchType = PDFActionSetOCGState::SwitchType::ON;
            const PDFArray* stateArray = stateArrayObject.getArray();
            items.reserve(stateArray->getCount());
            for (size_t i = 0; i < stateArray->getCount(); ++i)
            {
                const PDFObject& item = stateArray->getItem(i);
                if (item.isName())
                {
                    switchType = loader.readEnumByName(item, types.cbegin(), types.cend(), PDFActionSetOCGState::SwitchType::ON);
                }
                else if (item.isReference())
                {
                    items.emplace_back(switchType, item.getReference());
                }
            }
        }

        return PDFActionPtr(new PDFActionSetOCGState(qMove(items), isRadioButtonsPreserved));
    }
    else if (name == "Rendition")
    {
        PDFObject annotationObject = dictionary->get("AN");
        std::optional<PDFRendition> rendition;
        PDFObjectReference annotation = annotationObject.isReference() ? annotationObject.getReference() : PDFObjectReference();
        PDFActionRendition::Operation operation = static_cast<PDFActionRendition::Operation>(loader.readIntegerFromDictionary(dictionary, "OP", 4));
        QString javascript;

        if (dictionary->hasKey("R"))
        {
            rendition = PDFRendition::parse(storage, dictionary->get("R"));
        }
        PDFObject javascriptObject = storage->getObject(dictionary->get("JS"));
        if (javascriptObject.isString())
        {
            javascript = PDFEncoding::convertTextString(javascriptObject.getString());
        }
        else if (javascriptObject.isStream())
        {
            javascript = PDFEncoding::convertTextString(storage->getDecodedStream(javascriptObject.getStream()));
        }

        return PDFActionPtr(new PDFActionRendition(qMove(rendition), annotation, operation, qMove(javascript)));
    }
    else if (name == "Trans")
    {
        return PDFActionPtr(new PDFActionTransition(PDFPageTransition::parse(storage, dictionary->get("Trans"))));
    }
    else if (name == "GoTo3DView")
    {
        return PDFActionPtr(new PDFActionGoTo3DView(dictionary->get("TA"), dictionary->get("V")));
    }
    else if (name == "JavaScript")
    {
        QByteArray textJavaScript;
        const PDFObject& javaScriptObject = storage->getObject(dictionary->get("JS"));
        if (javaScriptObject.isString())
        {
            textJavaScript = javaScriptObject.getString();
        }
        else if (javaScriptObject.isStream())
        {
            textJavaScript = storage->getDecodedStream(javaScriptObject.getStream());
        }
        return PDFActionPtr(new PDFActionJavaScript(PDFEncoding::convertTextString(textJavaScript)));
    }
    else if (name == "RichMediaExecute")
    {
        PDFObjectReference richMediaAnnotation = loader.readReferenceFromDictionary(dictionary, "TA");
        PDFObjectReference richMediaInstance = loader.readReferenceFromDictionary(dictionary, "TI");

        QString command;
        PDFObject arguments;

        if (const PDFDictionary* commandDictionary = storage->getDictionaryFromObject(dictionary->get("CMD")))
        {
            command = loader.readTextStringFromDictionary(commandDictionary, "C", QString());
            arguments = commandDictionary->get("A");
        }

        return PDFActionPtr(new PDFActionRichMediaExecute(richMediaAnnotation, richMediaInstance, qMove(command), qMove(arguments)));
    }
    else if (name == "SubmitForm")
    {
        PDFFormAction::FieldScope fieldScope = PDFFormAction::FieldScope::All;
        PDFFileSpecification url = PDFFileSpecification::parse(storage, dictionary->get("F"));
        PDFFormAction::FieldList fieldList = PDFFormAction::parseFieldList(storage, dictionary->get("Fields"), fieldScope);
        PDFActionSubmitForm::SubmitFlags flags = static_cast<PDFActionSubmitForm::SubmitFlag>(loader.readIntegerFromDictionary(dictionary, "Flags", 0));
        QByteArray charset = loader.readStringFromDictionary(dictionary, "CharSet");

        if (fieldScope == PDFFormAction::FieldScope::Include &&
            flags.testFlag(PDFActionSubmitForm::IncludeExclude))
        {
            fieldScope = PDFFormAction::FieldScope::Exclude;
        }

        return PDFActionPtr(new PDFActionSubmitForm(fieldScope, qMove(fieldList), qMove(url), qMove(charset), flags));
    }
    else if (name == "ResetForm")
    {
        PDFFormAction::FieldScope fieldScope = PDFFormAction::FieldScope::All;
        PDFFormAction::FieldList fieldList = PDFFormAction::parseFieldList(storage, dictionary->get("Fields"), fieldScope);
        PDFActionResetForm::ResetFlags flags = static_cast<PDFActionResetForm::ResetFlag>(loader.readIntegerFromDictionary(dictionary, "Flags", 0));

        if (fieldScope == PDFFormAction::FieldScope::Include &&
            flags.testFlag(PDFActionResetForm::IncludeExclude))
        {
            fieldScope = PDFFormAction::FieldScope::Exclude;
        }

        return PDFActionPtr(new PDFActionResetForm(fieldScope, qMove(fieldList), flags));
    }
    else if (name == "ImportData")
    {
        PDFFileSpecification file = PDFFileSpecification::parse(storage, dictionary->get("F"));
        return PDFActionPtr(new PDFActionImportDataForm(qMove(file)));
    }

    return PDFActionPtr();
}

void PDFAction::fillActionList(std::vector<const PDFAction*>& actionList) const
{
    actionList.push_back(this);

    for (const PDFActionPtr& actionPointer : m_nextActions)
    {
        if (actionPointer)
        {
            actionPointer->fillActionList(actionList);
        }
    }
}

void PDFAction::cloneActionList(const PDFAction* sourceAction)
{
    if (sourceAction)
    {
        for (const auto& action : sourceAction->m_nextActions)
        {
            m_nextActions.push_back(action->clone());
        }
    }
}

PDFDestination PDFDestination::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFDestination result;
    object = storage->getObject(object);

    if (object.isName() || object.isString())
    {
        QByteArray name = object.getString();
        result.m_destinationType = DestinationType::Named;
        result.m_name = name;
    }
    else if (object.isArray())
    {
        const PDFArray* array = object.getArray();
        if (array->getCount() < 2)
        {
            return result;
        }

        PDFDocumentDataLoaderDecorator loader(storage);

        // First parse page number/page index
        PDFObject pageNumberObject = array->getItem(0);
        if (pageNumberObject.isReference())
        {
            result.m_pageReference = pageNumberObject.getReference();
        }
        else if (pageNumberObject.isInt())
        {
            result.m_pageIndex = pageNumberObject.getInteger();
        }
        else
        {
            return result;
        }

        QByteArray name = loader.readName(array->getItem(1));

        size_t currentIndex = 2;
        auto readNumber = [&]()
        {
            if (currentIndex < array->getCount())
            {
                return loader.readNumber(array->getItem(currentIndex++), 0.0);
            }
            return 0.0;
        };

        if (name == "XYZ")
        {
            result.m_destinationType = DestinationType::XYZ;
            result.m_left = readNumber();
            result.m_top = readNumber();
            result.m_zoom = readNumber();
        }
        else if (name == "Fit")
        {
            result.m_destinationType = DestinationType::Fit;
        }
        else if (name == "FitH")
        {
            result.m_destinationType = DestinationType::FitH;
            result.m_top = readNumber();
        }
        else if (name == "FitV")
        {
            result.m_destinationType = DestinationType::FitV;
            result.m_left = readNumber();
        }
        else if (name == "FitR")
        {
            result.m_destinationType = DestinationType::FitR;
            result.m_left = readNumber();
            result.m_bottom = readNumber();
            result.m_right = readNumber();
            result.m_top = readNumber();
        }
        else if (name == "FitB")
        {
            result.m_destinationType = DestinationType::FitB;
        }
        else if (name == "FitBH")
        {
            result.m_destinationType = DestinationType::FitBH;
            result.m_top = readNumber();
        }
        else if (name == "FitBV")
        {
            result.m_destinationType = DestinationType::FitBV;
            result.m_left = readNumber();
        }
        else
        {
            return result;
        }
    }

    return result;
}

void PDFDestination::setDestinationType(DestinationType destinationType)
{
    m_destinationType = destinationType;
}

void PDFDestination::setLeft(PDFReal left)
{
    m_left = left;
}

void PDFDestination::setTop(PDFReal top)
{
    m_top = top;
}

void PDFDestination::setRight(PDFReal right)
{
    m_right = right;
}

void PDFDestination::setBottom(PDFReal bottom)
{
    m_bottom = bottom;
}

void PDFDestination::setZoom(PDFReal zoom)
{
    m_zoom = zoom;
}

void PDFDestination::setName(const QByteArray& name)
{
    m_name = name;
}

void PDFDestination::setPageReference(PDFObjectReference pageReference)
{
    m_pageReference = pageReference;
}

void PDFDestination::setPageIndex(PDFInteger pageIndex)
{
    m_pageIndex = pageIndex;
}

PDFDestination PDFDestination::createXYZ(PDFObjectReference page, PDFReal left, PDFReal top, PDFReal zoom)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::XYZ);
    result.setPageReference(page);
    result.setLeft(left);
    result.setTop(top);
    result.setZoom(zoom);
    return result;
}

PDFDestination PDFDestination::createFit(PDFObjectReference page)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::Fit);
    result.setPageReference(page);
    return result;
}

PDFDestination PDFDestination::createFitH(PDFObjectReference page, PDFReal top)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::FitH);
    result.setPageReference(page);
    result.setTop(top);
    return result;
}

PDFDestination PDFDestination::createFitV(PDFObjectReference page, PDFReal left)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::FitV);
    result.setPageReference(page);
    result.setLeft(left);
    return result;
}

PDFDestination PDFDestination::createFitR(PDFObjectReference page, PDFReal left, PDFReal top, PDFReal right, PDFReal bottom)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::FitR);
    result.setPageReference(page);
    result.setLeft(left);
    result.setTop(top);
    result.setRight(right);
    result.setBottom(bottom);
    return result;
}

PDFDestination PDFDestination::createFitB(PDFObjectReference page)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::FitB);
    result.setPageReference(page);
    return result;
}

PDFDestination PDFDestination::createFitBH(PDFObjectReference page, PDFReal top)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::FitBH);
    result.setPageReference(page);
    result.setTop(top);
    return result;
}

PDFDestination PDFDestination::createFitBV(PDFObjectReference page, PDFReal left)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::FitBV);
    result.setPageReference(page);
    result.setLeft(left);
    return result;
}

PDFDestination PDFDestination::createNamed(const QByteArray& name)
{
    PDFDestination result;
    result.setDestinationType(DestinationType::Named);
    result.setName(name);
    return result;
}

bool PDFDestination::hasLeft() const
{
    switch (m_destinationType)
    {
        case DestinationType::XYZ:
        case DestinationType::FitV:
        case DestinationType::FitBV:
        case DestinationType::FitR:
            return true;

        default:
            break;
    }

    return false;
}

bool PDFDestination::hasTop() const
{
    switch (m_destinationType)
    {
        case DestinationType::XYZ:
        case DestinationType::FitH:
        case DestinationType::FitBH:
        case DestinationType::FitR:
            return true;

        default:
            break;
    }

    return false;
}

bool PDFDestination::hasRight() const
{
    switch (m_destinationType)
    {
        case DestinationType::FitR:
            return true;

        default:
            break;
    }

    return false;
}

bool PDFDestination::hasBottom() const
{
    switch (m_destinationType)
    {
        case DestinationType::FitR:
            return true;

        default:
            break;
    }

    return false;
}

bool PDFDestination::hasZoom() const
{
    switch (m_destinationType)
    {
        case DestinationType::XYZ:
            return true;

        default:
            break;
    }

    return false;
}

PDFFormAction::FieldList PDFFormAction::parseFieldList(const PDFObjectStorage* storage, PDFObject object, FieldScope& fieldScope)
{
    FieldList result;

    object = storage->getObject(object);
    if (object.isArray())
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        const PDFArray* fieldsArray = object.getArray();
        for (size_t i = 0, count = fieldsArray->getCount(); i < count; ++i)
        {
            PDFObject fieldObject = fieldsArray->getItem(i);
            if (fieldObject.isReference())
            {
                result.fieldReferences.push_back(fieldObject.getReference());
            }
            else if (fieldObject.isString())
            {
                result.qualifiedNames.push_back(loader.readTextString(fieldObject, QString()));
            }
        }
    }

    if (!result.isEmpty())
    {
        fieldScope = FieldScope::Include;
    }

    return result;
}

QString PDFActionURI::getURIString() const
{
    return QString::fromUtf8(m_URI);
}

void PDFActionURI::setURI(const QByteArray& newURI)
{
    m_URI = newURI;
}

void PDFActionURI::setIsMap(bool newIsMap)
{
    m_isMap = newIsMap;
}

void PDFActionGoTo::setDestination(const PDFDestination& destination)
{
    m_destination = destination;
}

void PDFActionGoTo::setStructureDestination(const PDFDestination& structureDestination)
{
    m_structureDestination = structureDestination;
}

PDFActionPtr PDFActionGoTo::clone() const
{
    PDFActionGoTo* clonedAction = new PDFActionGoTo(getDestination(), getStructureDestination());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionGoToR::clone() const
{
    PDFActionGoToR* clonedAction = new PDFActionGoToR(getDestination(),
                                                      getStructureDestination(),
                                                      getFileSpecification(),
                                                      isNewWindow());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionGoToE::clone() const
{
    PDFActionGoToE* clonedAction = new PDFActionGoToE(getDestination(),
                                                      getFileSpecification(),
                                                      isNewWindow(),
                                                      getTarget());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionGoToDp::clone() const
{
    PDFActionGoToDp* clonedAction = new PDFActionGoToDp(getDocumentPart());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionLaunch::clone() const
{
    PDFActionLaunch* clonedAction = new PDFActionLaunch(getFileSpecification(), isNewWindow(), getWinSpecification());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionThread::clone() const
{
    PDFActionThread* clonedAction = new PDFActionThread(getFileSpecification(), getThread(), getBead());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionURI::clone() const
{
    PDFActionURI* clonedAction = new PDFActionURI(getURI(), isMap());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionSound::clone() const
{
    PDFActionSound* clonedAction = new PDFActionSound(*getSound(), getVolume(), isSynchronous(), isRepeat(), isMix());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionMovie::clone() const
{
    PDFActionMovie* clonedAction = new PDFActionMovie(getAnnotation(), getTitle(), getOperation());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionHide::clone() const
{
    PDFActionHide* clonedAction = new PDFActionHide(getAnnotations(), getFieldNames(), isHide());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionNamed::clone() const
{
    PDFActionNamed* clonedAction = new PDFActionNamed(getNamedActionType(), getCustomNamedAction());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionSetOCGState::clone() const
{
    PDFActionSetOCGState* clonedAction = new PDFActionSetOCGState(getStateChangeItems(), isRadioButtonsPreserved());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionTransition::clone() const
{
    PDFActionTransition* clonedAction = new PDFActionTransition(getTransition());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionGoTo3DView::clone() const
{
    PDFActionGoTo3DView* clonedAction = new PDFActionGoTo3DView(getAnnotation(), getView());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionJavaScript::clone() const
{
    PDFActionJavaScript* clonedAction = new PDFActionJavaScript(getJavaScript());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionRichMediaExecute::clone() const
{
    PDFActionRichMediaExecute* clonedAction = new PDFActionRichMediaExecute(getRichMediaAnnotation(),
                                                                            getRichMediaInstance(),
                                                                            getCommand(),
                                                                            getArguments());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionSubmitForm::clone() const
{
    PDFActionSubmitForm* clonedAction = new PDFActionSubmitForm(getFieldScope(), getFieldList(), getUrl(), getCharset(), getFlags());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionResetForm::clone() const
{
    PDFActionResetForm* clonedAction = new PDFActionResetForm(getFieldScope(), getFieldList(), getFlags());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionImportDataForm::clone() const
{
    PDFActionImportDataForm* clonedAction = new PDFActionImportDataForm(getFile());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

PDFActionPtr PDFActionRendition::clone() const
{
    PDFActionRendition* clonedAction = new PDFActionRendition(m_rendition, getAnnotation(), getOperation(), getJavaScript());
    clonedAction->cloneActionList(this);
    return PDFActionPtr(clonedAction);
}

}   // namespace pdf
