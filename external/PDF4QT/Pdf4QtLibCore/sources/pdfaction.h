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

#ifndef PDFACTION_H
#define PDFACTION_H

#include "pdfglobal.h"
#include "pdfobject.h"
#include "pdffile.h"
#include "pdfmultimedia.h"
#include "pdfpagetransition.h"

#include <QSharedPointer>

#include <set>
#include <variant>
#include <functional>

namespace pdf
{
class PDFAction;
class PDFDocument;
class PDFObjectStorage;

enum class ActionType
{
    GoTo,
    GoToR,
    GoToE,
    GoToDp,
    Launch,
    Thread,
    URI,
    Sound,
    Movie,
    Hide,
    Named,
    SetOCGState,
    Rendition,
    Transition,
    GoTo3DView,
    JavaScript,
    SubmitForm,
    ResetForm,
    ImportDataForm,
    RichMediaExecute
};

enum class DestinationType
{
    Invalid,
    Named,
    XYZ,
    Fit,
    FitH,
    FitV,
    FitR,
    FitB,
    FitBH,
    FitBV
};

/// Destination to the specific location of the document. Destination can also be 'Named' type,
/// in this case, destination in name tree is found and used. Important note: because structure
/// destination has almost exactly same syntax as page destination, it should be checked,
/// if indirect reference returned by function \p getPageReference references really page,
/// or some structure element.
class PDF4QTLIBCORESHARED_EXPORT PDFDestination
{
public:
    explicit inline PDFDestination() = default;

    DestinationType getDestinationType() const { return m_destinationType; }
    PDFReal getLeft() const { return m_left; }
    PDFReal getTop() const { return m_top; }
    PDFReal getRight() const { return m_right; }
    PDFReal getBottom() const { return m_bottom; }
    PDFReal getZoom() const { return m_zoom; }
    const QByteArray& getName() const { return m_name; }
    PDFObjectReference getPageReference() const { return m_pageReference; }
    PDFInteger getPageIndex() const { return m_pageIndex; }

    /// Parses the destination from the object. If object contains invalid destination,
    /// then empty destination is returned. If object is empty, empty destination is returned.
    /// \param storage Object storage
    /// \param object Destination object
    static PDFDestination parse(const PDFObjectStorage* storage, PDFObject object);

    void setDestinationType(DestinationType destinationType);
    void setLeft(PDFReal left);
    void setTop(PDFReal top);
    void setRight(PDFReal right);
    void setBottom(PDFReal bottom);
    void setZoom(PDFReal zoom);
    void setName(const QByteArray& name);
    void setPageReference(PDFObjectReference pageReference);
    void setPageIndex(PDFInteger pageIndex);

    static PDFDestination createXYZ(PDFObjectReference page, PDFReal left, PDFReal top, PDFReal zoom);
    static PDFDestination createFit(PDFObjectReference page);
    static PDFDestination createFitH(PDFObjectReference page, PDFReal top);
    static PDFDestination createFitV(PDFObjectReference page, PDFReal left);
    static PDFDestination createFitR(PDFObjectReference page, PDFReal left, PDFReal top, PDFReal right, PDFReal bottom);
    static PDFDestination createFitB(PDFObjectReference page);
    static PDFDestination createFitBH(PDFObjectReference page, PDFReal top);
    static PDFDestination createFitBV(PDFObjectReference page, PDFReal left);
    static PDFDestination createNamed(const QByteArray& name);

    bool hasLeft() const;
    bool hasTop() const;
    bool hasRight() const;
    bool hasBottom() const;
    bool hasZoom() const;

    bool isValid() const { return m_destinationType != DestinationType::Invalid; }
    bool isNamedDestination() const { return m_destinationType == DestinationType::Named; }

private:
    DestinationType m_destinationType = DestinationType::Invalid;
    PDFReal m_left = 0.0;
    PDFReal m_top = 0.0;
    PDFReal m_right = 0.0;
    PDFReal m_bottom = 0.0;
    PDFReal m_zoom = 0.0;
    QByteArray m_name;
    PDFObjectReference m_pageReference;
    PDFInteger m_pageIndex = 0;
};

using PDFActionPtr = QSharedPointer<PDFAction>;

/// Base class for action types.
class PDF4QTLIBCORESHARED_EXPORT PDFAction
{
public:
    explicit PDFAction() = default;
    virtual ~PDFAction() = default;

    /// Returns type of the action.
    virtual ActionType getType() const = 0;

    /// Returns container with next actions
    const std::vector<PDFActionPtr>& getNextActions() const { return m_nextActions; }

    /// Tries to parse the action. If serious error occurs, then exception is thrown.
    /// If \p object is null object, then nullptr is returned.
    /// \param storage Object storage
    /// \param object Object containing the action
    static PDFActionPtr parse(const PDFObjectStorage* storage, PDFObject object);

    /// Calls the lambda function with action as parameter, then following
    /// the 'Next' entry, as described by PDF 1.7 specification.
    void apply(const std::function<void(const PDFAction* action)>& callback);

    /// Returns list of actions to be executed
    std::vector<const PDFAction*> getActionList() const;

    /// Clone action
    virtual PDFActionPtr clone() const = 0;

protected:
    void cloneActionList(const PDFAction* sourceAction);

private:
    static PDFActionPtr parseImpl(const PDFObjectStorage* storage, PDFObject object, std::set<PDFObjectReference>& usedReferences);

    void fillActionList(std::vector<const PDFAction*>& actionList) const;

    std::vector<PDFActionPtr> m_nextActions;
};

/// Regular go-to action. Can contain also structure destinations, both regular page destination
/// and structure destination are present, because if structure destination fails, then
/// page destination can be used as fallback resolution.
class PDF4QTLIBCORESHARED_EXPORT PDFActionGoTo : public PDFAction
{
public:
    explicit inline PDFActionGoTo(PDFDestination destination, PDFDestination structureDestination) :
        m_destination(qMove(destination)), m_structureDestination(qMove(structureDestination)) { }

    virtual ActionType getType() const override { return ActionType::GoTo; }

    const PDFDestination& getDestination() const { return m_destination; }
    const PDFDestination& getStructureDestination() const { return m_structureDestination; }

    void setDestination(const PDFDestination& destination);
    void setStructureDestination(const PDFDestination& structureDestination);

    virtual PDFActionPtr clone() const override;

private:
    PDFDestination m_destination;
    PDFDestination m_structureDestination;
};

class PDFActionGoToR : public PDFAction
{
public:
    explicit inline PDFActionGoToR(PDFDestination destination, PDFDestination structureDestination, PDFFileSpecification fileSpecification, bool newWindow) :
        m_destination(qMove(destination)),
        m_structureDestination(qMove(structureDestination)),
        m_fileSpecification(qMove(fileSpecification)),
        m_newWindow(newWindow)
    {

    }

    virtual ActionType getType() const override { return ActionType::GoToR; }

    const PDFDestination& getDestination() const { return m_destination; }
    const PDFDestination& getStructureDestination() const { return m_structureDestination; }
    const PDFFileSpecification& getFileSpecification() const { return m_fileSpecification; }
    bool isNewWindow() const { return m_newWindow; }

    virtual PDFActionPtr clone() const override;

private:
    PDFDestination m_destination;
    PDFDestination m_structureDestination;
    PDFFileSpecification m_fileSpecification;
    bool m_newWindow = false;
};

class PDFActionGoToE : public PDFAction
{
public:
    explicit inline PDFActionGoToE(PDFDestination destination, PDFFileSpecification fileSpecification, bool newWindow, const PDFObject& target) :
        m_destination(qMove(destination)),
        m_fileSpecification(qMove(fileSpecification)),
        m_newWindow(newWindow),
        m_target(target)
    {

    }

    virtual ActionType getType() const override { return ActionType::GoToE; }

    const PDFDestination& getDestination() const { return m_destination; }
    const PDFFileSpecification& getFileSpecification() const { return m_fileSpecification; }
    bool isNewWindow() const { return m_newWindow; }
    const PDFObject& getTarget() const { return m_target; }

    virtual PDFActionPtr clone() const override;

private:
    PDFDestination m_destination;
    PDFFileSpecification m_fileSpecification;
    bool m_newWindow = false;
    PDFObject m_target;
};

/// Go to document part
class PDFActionGoToDp : public PDFAction
{
public:
    explicit inline PDFActionGoToDp(PDFObjectReference documentPart) :
        m_documentPart(documentPart) { }

    virtual ActionType getType() const override { return ActionType::GoToDp; }

    PDFObjectReference getDocumentPart() const { return m_documentPart; }

    virtual PDFActionPtr clone() const override;

private:
    PDFObjectReference m_documentPart;
};

class PDFActionLaunch : public PDFAction
{
public:

    /// Specification of launched application (if not file specification is attached)
    struct Win
    {
        QByteArray file;
        QByteArray directory;
        QByteArray operation;
        QByteArray parameters;
    };

    explicit inline PDFActionLaunch(PDFFileSpecification fileSpecification, bool newWindow, Win win) :
        m_fileSpecification(qMove(fileSpecification)),
        m_newWindow(newWindow),
        m_win(qMove(win))
    {

    }

    virtual ActionType getType() const override { return ActionType::Launch; }

    const PDFFileSpecification& getFileSpecification() const { return m_fileSpecification; }
    const Win& getWinSpecification() const { return m_win; }
    bool isNewWindow() const { return m_newWindow; }

    virtual PDFActionPtr clone() const override;

private:
    PDFFileSpecification m_fileSpecification;
    bool m_newWindow = false;
    Win m_win;
};

class PDFActionThread : public PDFAction
{
public:
    using Thread = std::variant<typename std::monostate, PDFObjectReference, PDFInteger, QString>;
    using Bead = std::variant<typename std::monostate, PDFObjectReference, PDFInteger>;

    explicit inline PDFActionThread(PDFFileSpecification fileSpecification, Thread thread, Bead bead) :
        m_fileSpecification(qMove(fileSpecification)),
        m_thread(qMove(thread)),
        m_bead(qMove(bead))
    {

    }

    virtual ActionType getType() const override { return ActionType::Thread; }

    const PDFFileSpecification& getFileSpecification() const { return m_fileSpecification; }
    const Thread& getThread() const { return m_thread; }
    const Bead& getBead() const { return m_bead; }

    virtual PDFActionPtr clone() const override;

private:
    PDFFileSpecification m_fileSpecification;
    Thread m_thread;
    Bead m_bead;
};

class PDFActionURI : public PDFAction
{
public:
    explicit inline PDFActionURI(QByteArray URI, bool isMap) :
        m_URI(qMove(URI)),
        m_isMap(isMap)
    {

    }

    virtual ActionType getType() const override { return ActionType::URI; }

    const QByteArray& getURI() const { return m_URI; }
    bool isMap() const { return m_isMap; }

    /// Returns URI as string in unicode. If pdf document conforms
    /// to PDF specification, URI is UTF-8 encoded string.
    QString getURIString() const;

    virtual PDFActionPtr clone() const override;

    void setURI(const QByteArray& newURI);
    void setIsMap(bool newIsMap);

private:
    QByteArray m_URI;
    bool m_isMap;
};

class PDFActionSound : public PDFAction
{
public:
    explicit inline PDFActionSound(PDFSound sound, PDFReal volume, bool isSynchronous, bool isRepeat, bool isMix) :
        m_sound(qMove(sound)),
        m_volume(qMove(volume)),
        m_isSynchronous(isSynchronous),
        m_isRepeat(isRepeat),
        m_isMix(isMix)
    {

    }

    virtual ActionType getType() const override { return ActionType::Sound; }

    const PDFSound* getSound() const { return &m_sound; }
    PDFReal getVolume() const { return m_volume; }
    bool isSynchronous() const { return m_isSynchronous; }
    bool isRepeat() const { return m_isRepeat; }
    bool isMix() const { return m_isMix; }

    virtual PDFActionPtr clone() const override;

private:
    PDFSound m_sound;
    PDFReal m_volume;
    bool m_isSynchronous;
    bool m_isRepeat;
    bool m_isMix;
};

class PDFActionMovie : public PDFAction
{
public:
    enum class Operation
    {
        Play,
        Stop,
        Pause,
        Resume
    };

    explicit inline PDFActionMovie(PDFObjectReference annotation, QString title, Operation operation) :
        m_annotation(qMove(annotation)),
        m_title(qMove(title)),
        m_operation(operation)
    {

    }

    virtual ActionType getType() const override { return ActionType::Movie; }

    PDFObjectReference getAnnotation() const { return m_annotation; }
    const QString& getTitle() const { return m_title; }
    Operation getOperation() const { return m_operation; }

    virtual PDFActionPtr clone() const override;

private:
    PDFObjectReference m_annotation;
    QString m_title;
    Operation m_operation;
};

class PDFActionHide : public PDFAction
{
public:
    explicit inline PDFActionHide(std::vector<PDFObjectReference>&& annotations, std::vector<QString>&& fieldNames, bool hide) :
        m_annotations(qMove(annotations)),
        m_fieldNames(qMove(fieldNames)),
        m_hide(hide)
    {

    }

    explicit inline PDFActionHide(const std::vector<PDFObjectReference>& annotations, const std::vector<QString>& fieldNames, bool hide) :
        m_annotations(annotations),
        m_fieldNames(fieldNames),
        m_hide(hide)
    {

    }

    virtual ActionType getType() const override { return ActionType::Hide; }

    const std::vector<PDFObjectReference>& getAnnotations() const { return m_annotations; }
    const std::vector<QString>& getFieldNames() const { return m_fieldNames; }
    bool isHide() const { return m_hide; }

    virtual PDFActionPtr clone() const override;

private:
    std::vector<PDFObjectReference> m_annotations;
    std::vector<QString> m_fieldNames;
    bool m_hide;
};

class PDFActionNamed : public PDFAction
{
public:
    enum class NamedActionType
    {
        Custom,
        NextPage,
        PrevPage,
        FirstPage,
        LastPage
    };

    explicit inline PDFActionNamed(NamedActionType namedActionType, QByteArray&& customNamedAction) :
        m_namedActionType(namedActionType),
        m_customNamedAction(qMove(customNamedAction))
    {

    }

    explicit inline PDFActionNamed(NamedActionType namedActionType, const QByteArray& customNamedAction) :
        m_namedActionType(namedActionType),
        m_customNamedAction(customNamedAction)
    {

    }

    virtual ActionType getType() const override { return ActionType::Named; }

    NamedActionType getNamedActionType() const { return m_namedActionType; }
    const QByteArray& getCustomNamedAction() const { return m_customNamedAction; }

    virtual PDFActionPtr clone() const override;

private:
    NamedActionType m_namedActionType;
    QByteArray m_customNamedAction;
};

class PDFActionSetOCGState : public PDFAction
{
public:

    enum class SwitchType
    {
        ON = 0,
        OFF = 1,
        Toggle = 2
    };

    using StateChangeItem = std::pair<SwitchType, PDFObjectReference>;
    using StateChangeItems = std::vector<StateChangeItem>;

    explicit inline PDFActionSetOCGState(StateChangeItems stateChangeItems, bool isRadioButtonsPreserved) :
        m_items(qMove(stateChangeItems)),
        m_isRadioButtonsPreserved(isRadioButtonsPreserved)
    {

    }

    virtual ActionType getType() const override { return ActionType::SetOCGState; }

    const StateChangeItems& getStateChangeItems() const { return m_items; }
    bool isRadioButtonsPreserved() const { return m_isRadioButtonsPreserved; }

    virtual PDFActionPtr clone() const override;

private:
    StateChangeItems m_items;
    bool m_isRadioButtonsPreserved;
};

class PDFActionRendition : public PDFAction
{
public:

    enum class Operation
    {
        PlayAndAssociate = 0,
        Stop = 1,
        Pause = 2,
        Resume = 3,
        Play = 4
    };

    explicit inline PDFActionRendition(std::optional<PDFRendition> rendition, PDFObjectReference annotation, Operation operation, QString javascript) :
        m_rendition(qMove(rendition)),
        m_annotation(annotation),
        m_operation(operation),
        m_javascript(qMove(javascript))
    {

    }

    virtual ActionType getType() const override { return ActionType::Rendition; }

    const PDFRendition* getRendition() const { return m_rendition.has_value() ? &m_rendition.value() : nullptr; }
    PDFObjectReference getAnnotation() const { return m_annotation; }
    Operation getOperation() const { return m_operation; }
    const QString& getJavaScript() const { return m_javascript; }

    virtual PDFActionPtr clone() const override;

private:
    std::optional<PDFRendition> m_rendition;
    PDFObjectReference m_annotation;
    Operation m_operation;
    QString m_javascript;
};

class PDFActionTransition : public PDFAction
{
public:
    explicit inline PDFActionTransition(PDFPageTransition transition) :
        m_transition(qMove(transition))
    {

    }

    virtual ActionType getType() const override { return ActionType::Transition; }

    const PDFPageTransition& getTransition() const { return m_transition; }

    virtual PDFActionPtr clone() const override;

private:
    PDFPageTransition m_transition;
};

class PDFActionGoTo3DView : public PDFAction
{
public:
    explicit PDFActionGoTo3DView(PDFObject annotation, PDFObject view) :
        m_annotation(qMove(annotation)),
        m_view(qMove(view))
    {

    }

    virtual ActionType getType() const override { return ActionType::GoTo3DView; }

    const PDFObject& getAnnotation() const { return m_annotation; }
    const PDFObject& getView() const { return m_view; }

    virtual PDFActionPtr clone() const override;

private:
    PDFObject m_annotation;
    PDFObject m_view;
};

class PDFActionJavaScript : public PDFAction
{
public:
    explicit PDFActionJavaScript(const QString& javaScript) :
        m_javaScript(javaScript)
    {

    }

    virtual ActionType getType() const override { return ActionType::JavaScript; }

    const QString& getJavaScript() const { return m_javaScript; }

    virtual PDFActionPtr clone() const override;

private:
    QString m_javaScript;
};

class PDFActionRichMediaExecute : public PDFAction
{
public:
    explicit PDFActionRichMediaExecute(PDFObjectReference richMediaAnnotation,
                                       PDFObjectReference richMediaInstance,
                                       QString command,
                                       PDFObject arguments) :
        m_richMediaAnnotation(richMediaAnnotation),
        m_richMediaInstance(richMediaInstance),
        m_command(qMove(command)),
        m_arguments(qMove(arguments))
    {

    }

    virtual ActionType getType() const override { return ActionType::RichMediaExecute; }

    PDFObjectReference getRichMediaAnnotation() const { return m_richMediaAnnotation; }
    PDFObjectReference getRichMediaInstance() const { return m_richMediaInstance; }
    QString getCommand() const { return m_command; }
    PDFObject getArguments() const { return m_arguments; }

    virtual PDFActionPtr clone() const override;

private:
    PDFObjectReference m_richMediaAnnotation;
    PDFObjectReference m_richMediaInstance;
    QString m_command;
    PDFObject m_arguments;
};


class PDFFormAction : public PDFAction
{
public:
    enum FieldScope
    {
        All,        ///< Perform action for all form fields
        Include,    ///< Perform action only on fields listed in the list
        Exclude     ///< Perform action on all fields except those in the list
    };

    struct FieldList
    {
        std::vector<PDFObjectReference> fieldReferences;
        QStringList qualifiedNames;

        bool isEmpty() const { return fieldReferences.empty() && qualifiedNames.isEmpty(); }
    };

    explicit inline PDFFormAction(FieldScope fieldScope, FieldList fieldList) :
        m_fieldScope(fieldScope),
        m_fieldList(qMove(fieldList))
    {

    }

    FieldScope getFieldScope() const { return m_fieldScope; }
    const FieldList& getFieldList() const { return m_fieldList; }

    /// Parses the field list from the object. If object contains invalid field list,
    /// then empty field list is returned. If object is empty, empty field list is returned.
    /// \param storage Object storage
    /// \param object Field list array object
    /// \param[out] fieldScope Result field scope
    static FieldList parseFieldList(const PDFObjectStorage* storage, PDFObject object, FieldScope& fieldScope);

protected:
    FieldScope m_fieldScope = FieldScope::All;
    FieldList m_fieldList;
};

class PDFActionSubmitForm : public PDFFormAction
{
public:
    enum SubmitFlag
    {
        None = 0,
        IncludeExclude = 1 << 0,
        IncludeNoValueFields = 1 << 1,
        ExportFormat = 1 << 2,
        GetMethod = 1 << 3,
        SubmitCoordinates = 1 << 4,
        XFDF = 1 << 5,
        IncludeAppendSaves = 1 << 6,
        IncludeAnnotations = 1 << 7,
        SubmitPDF = 1 << 8,
        CanonicalFormat = 1 << 9,
        ExclNonUseAnnots = 1 << 10,
        ExclFKey = 1 << 11,
        EmbedForm = 1 << 13
    };
    Q_DECLARE_FLAGS(SubmitFlags, SubmitFlag)

    explicit inline PDFActionSubmitForm(FieldScope fieldScope, FieldList fieldList, PDFFileSpecification url, QByteArray charset, SubmitFlags flags) :
        PDFFormAction(fieldScope, qMove(fieldList)),
        m_url(qMove(url)),
        m_charset(qMove(charset)),
        m_flags(flags)
    {

    }

    virtual ActionType getType() const override { return ActionType::SubmitForm; }

    const PDFFileSpecification& getUrl() const { return m_url; }
    const QByteArray& getCharset() const { return m_charset; }
    SubmitFlags getFlags() const { return m_flags; }

    virtual PDFActionPtr clone() const override;

private:
    PDFFileSpecification m_url;
    QByteArray m_charset;
    SubmitFlags m_flags = None;
};

class PDFActionResetForm : public PDFFormAction
{
public:
    enum ResetFlag
    {
        None = 0,
        IncludeExclude = 1 << 0,
    };
    Q_DECLARE_FLAGS(ResetFlags, ResetFlag)


    explicit inline PDFActionResetForm(FieldScope fieldScope, FieldList fieldList, ResetFlags flags) :
        PDFFormAction(fieldScope, qMove(fieldList)),
        m_flags(flags)
    {

    }

    virtual ActionType getType() const override { return ActionType::ResetForm; }

    ResetFlags getFlags() const { return m_flags; }

    virtual PDFActionPtr clone() const override;

private:
    ResetFlags m_flags = None;
};

class PDFActionImportDataForm : public PDFAction
{
public:

    explicit inline PDFActionImportDataForm(PDFFileSpecification file) :
        m_file(qMove(file))
    {

    }

    virtual ActionType getType() const override { return ActionType::ImportDataForm; }

    const PDFFileSpecification& getFile() const { return m_file; }

    virtual PDFActionPtr clone() const override;

private:
    PDFFileSpecification m_file;
};

}   // namespace pdf

#endif // PDFACTION_H
