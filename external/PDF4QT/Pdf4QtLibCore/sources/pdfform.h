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

#ifndef PDFFORM_H
#define PDFFORM_H

#include "pdfobject.h"
#include "pdfdocument.h"
#include "pdfannotation.h"
#include "pdfsignaturehandler.h"
#include "pdfxfaengine.h"

#include <QTextLayout>

#include <optional>

namespace pdf
{
class PDFFormField;
class PDFFormManager;
class PDFObjectStorage;
class PDFModifiedDocument;
class PDFDocumentModifier;

using PDFFormFieldPointer = QSharedPointer<PDFFormField>;
using PDFFormFields = std::vector<PDFFormFieldPointer>;
using PDFWidgetToFormFieldMapping = std::map<PDFObjectReference, PDFFormField*>;

/// A simple proxy to the widget annotation
class PDFFormWidget
{
public:
    explicit inline PDFFormWidget() = default;
    explicit inline PDFFormWidget(PDFObjectReference page, PDFObjectReference widget, PDFFormField* parentField, PDFAnnotationAdditionalActions actions);

    PDFObjectReference getPage() const { return m_page; }
    PDFObjectReference getWidget() const { return m_widget; }
    PDFFormField* getParent() const { return m_parentField; }
    const PDFAction* getAction(PDFAnnotationAdditionalActions::Action action) const { return m_actions.getAction(action); }

    /// Parses form widget from the object reference. If some error occurs
    /// then empty object is returned, no exception is thrown.
    /// \param storage Storage
    /// \param reference Widget reference
    /// \param parentField Parent field
    static PDFFormWidget parse(const PDFObjectStorage* storage, PDFObjectReference reference, PDFFormField* parentField);

private:
    PDFObjectReference m_page;
    PDFObjectReference m_widget;
    PDFFormField* m_parentField;
    PDFAnnotationAdditionalActions m_actions;
};

using PDFFormWidgets = std::vector<PDFFormWidget>;

/// Form field is terminal or non-terminal field (non-terminal fields
/// have children), fields represents various interactive widgets, such as
/// checks, radio buttons, text edits etc., which are editable and user
/// can interact with them.
class PDF4QTLIBCORESHARED_EXPORT PDFFormField
{
public:
    explicit inline PDFFormField() = default;
    virtual ~PDFFormField() = default;

    enum class FieldType
    {
        Invalid,
        Button,
        Text,
        Choice,
        Signature
    };

    enum NameType
    {
        Partial,            ///< Partial name for this field
        UserCaption,        ///< Name used in GUI (for example, in message boxes)
        FullyQualified,     ///< Fully qualified name (according to the PDF specification 1.7)
        Export,             ///< Name for export
        NameTypeEnd
    };

    using FieldNames = std::array<QString, NameTypeEnd>;

    enum FieldFlag
    {
        None = 0,

        /// Field is read only, it doesn't respond on mouse clicks (if it is a button),
        /// associated widget annotation will not interact with the user. Field can't
        /// change it's value. Mainly used for calculable fields.
        ReadOnly = 1 << 0,

        /// If set, field is required, when submitting form by submit action. If submit
        /// action is triggered, then all fields with this flag must have a value.
        Required = 1 << 1,

        /// If set, field value must not be exported by submit form action.
        NoExport = 1 << 2,

        /// Text fields only. If set, then text can span multiple lines. Otherwise,
        /// text is restricted to single line.
        Multiline = 1 << 12,

        /// Text fields only. If set, field is intended to display text edit, which
        /// can edit passwords. Password characters should not be echoed to the screen
        /// and value of this field should not be stored in PDF file.
        Password = 1 << 13,

        /// Only for radio buttons. If set, at least one radio button is checked.
        /// If user clicks on checked radio button, it is not checked off. Otherwise
        /// user can uncheck checked radio button (so no button is selected).
        NoToggleToOff = 1 << 14,

        /// Valid only for buttons which have PushButton flag unset. If Radio flag is set,
        /// then widget is radio button, otherwise widget is push button.
        Radio = 1 << 15,

        /// Valid only for buttons. If set, button is push button.
        PushButton = 1 << 16,

        /// For choice fields only. If set, choice field is combo box.
        /// If clear, it is a list box.
        Combo = 1 << 17,

        /// For choice fields combo boxes only. If set, combo box is editable,
        /// i.e. user can enter custom text. If this flag is cleared, then combo box
        /// is not editable and user can only select items from the drop down list.
        Edit = 1 << 18,

        /// For choice fields only. If set, the field option's items should be sorted
        /// alphabetically, but not by the viewer application, but by author of the form.
        /// Viewer application should respect Opt array and display items in that order.
        Sort = 1 << 19,

        /// Text fields only. Text field is used to select file path, whose contents
        /// should be submitted as the value of the field.
        FileSelect = 1 << 20,

        /// For choice fields only. If set, then user can select multiple choices
        /// simultaneously, if clear, then only one item should be selected at the time.
        MultiSelect = 1 << 21,

        /// Text fields only. If set, texts entered in this field should not be spell checked.
        DoNotSpellcheck = 1 << 22,

        /// Text fields only. Allow only so much text, which fits field's area. If field's area is filled,
        /// then do not allow the user to store more text in the field.
        DoNotScroll = 1 << 23,

        /// Text fields only. If set, then MaxLen entry and annotation rectangle is used
        /// to divide space equally for each character. Text is laid out into those spaces.
        Comb = 1 << 24,

        /// Valid only for radio buttons. Radio buttons in a group of radio buttons,
        /// which have same value for 'On' state, will turn On and Off in unison, if one
        /// is checked, all are checked. If clear, radio buttons are mutually exclusive.
        RadiosInUnison = 1 << 25,

        /// Text fields only. Value of this field should be a rich text.
        RichText = 1 << 25,

        /// Choice fields only. If set, then when user selects choice by mouse,
        /// data is immediately commited. Otherwise data are commited only, when
        /// widget lose focus.
        CommitOnSelectionChange = 1 << 26
    };

    Q_DECLARE_FLAGS(FieldFlags, FieldFlag)

    PDFObjectReference getSelfReference() const { return m_selfReference; }
    FieldType getFieldType() const { return m_fieldType; }
    const PDFFormField* getParentField() const { return m_parentField; }
    const PDFFormFields& getChildFields() const { return m_childFields; }
    const PDFFormWidgets& getWidgets() const { return m_widgets; }
    const QString& getName(NameType nameType) const { return m_fieldNames.at(nameType); }
    FieldFlags getFlags() const { return m_fieldFlags; }
    const PDFObject& getValue() const { return m_value; }
    const PDFObject& getDefaultValue() const { return m_defaultValue; }

    /// Fills widget to form field mapping
    /// \param mapping Form field mapping
    void fillWidgetToFormFieldMapping(PDFWidgetToFormFieldMapping& mapping);

    /// Reloads value from object storage. Actually stored value is lost.
    virtual void reloadValue(const PDFObjectStorage* storage, PDFObject parentValue);

    /// Applies function to this form field and all its descendants,
    /// in pre-order (first application is to the parent, following
    /// calls to apply for children).
    /// \param functor Functor to apply
    void apply(const std::function<void(const PDFFormField*)>& functor) const;

    /// Applies function to this form field and all its descendants,
    /// in pre-order (first application is to the parent, following
    /// calls to apply for children).
    /// \param functor Functor to apply
    void modify(const std::function<void(PDFFormField*)>& functor);

    /// Returns action by type. If action is not found, nullptr is returned
    /// \param action Action type
    const PDFAction* getAction(PDFAnnotationAdditionalActions::Action action) const { return m_additionalActions.getAction(action); }

    /// Returns container of actions
    const PDFAnnotationAdditionalActions& getActions() const { return m_additionalActions; }

    /// Parses form field from the object reference. If some error occurs
    /// then null pointer is returned, no exception is thrown.
    /// \param storage Storage
    /// \param reference Field reference
    /// \param parentField Parent field (or nullptr, if it is root field)
    static PDFFormFieldPointer parse(const PDFObjectStorage* storage, PDFObjectReference reference, PDFFormField* parentField);

    struct SetValueParameters
    {
        enum class Scope
        {
            User,       ///< Changed value comes from user input
            Internal    ///< Value is changed by some program operation (for example, calculation)
        };

        PDFObject value;
        PDFObjectReference invokingWidget;
        PDFFormField* invokingFormField = nullptr;
        PDFDocumentModifier* modifier = nullptr;
        PDFFormManager* formManager = nullptr;
        Scope scope = Scope::User;

        // Choice list box field only
        PDFInteger listboxTopIndex= 0;
        std::vector<PDFInteger> listboxChoices;
    };

    /// Sets value to the form field. If value has been correctly
    /// set, then true is returned, otherwise false is returned.
    /// This function also verifies, if value can be set (i.e. form field
    /// is editable, and value is valid).
    /// \param parameters Parameters
    virtual bool setValue(const SetValueParameters& parameters);

    struct ResetValueParameters
    {
        PDFDocumentModifier* modifier = nullptr;
        PDFFormManager* formManager = nullptr;
    };

    /// Resets value to the field's default value. Widget annotation
    /// appearances are also updated.
    /// \param parameters Parameters
    virtual void resetValue(const ResetValueParameters& parameters);

protected:
    PDFObjectReference m_selfReference;
    FieldType m_fieldType = FieldType::Invalid;
    PDFFormField* m_parentField = nullptr;
    PDFFormFields m_childFields;
    PDFFormWidgets m_widgets;
    FieldNames m_fieldNames;
    FieldFlags m_fieldFlags = None;
    PDFObject m_value;
    PDFObject m_defaultValue;
    PDFAnnotationAdditionalActions m_additionalActions;
};

/// Represents pushbutton, checkbox and radio button (which is distinguished
/// by flags).
class PDF4QTLIBCORESHARED_EXPORT PDFFormFieldButton : public PDFFormField
{
public:
    explicit inline PDFFormFieldButton() = default;

    enum class ButtonType
    {
        PushButton,
        RadioButton,
        CheckBox
    };

    /// Determines button type from form field's flags
    ButtonType getButtonType() const;

    const QStringList& getOptions() const { return m_options; }

    /// Returns appearance state, which corresponds to the checked
    /// state of checkbox or radio button. If error occurs, then
    /// empty byte array is returned.
    /// \param formManager Form manager
    /// \param widget Widget
    static QByteArray getOnAppearanceState(const PDFFormManager* formManager, const PDFFormWidget* widget);

    /// Returns appearance state, which corresponds to the unchecked
    /// state of checkbox or radio button. If error occurs, then
    /// empty byte array is returned.
    /// \param formManager Form manager
    /// \param widget Widget
    static QByteArray getOffAppearanceState(const PDFFormManager* formManager, const PDFFormWidget* widget);

    virtual bool setValue(const SetValueParameters& parameters) override;
    virtual void resetValue(const ResetValueParameters& parameters) override;

private:
    friend PDFFormFieldPointer PDFFormField::parse(const PDFObjectStorage* storage, PDFObjectReference reference, PDFFormField* parentField);

    /// List of export names of 'On' state for radio buttons. In widget annotation's appearance
    /// dictionaries, state names are computer generated numbers (for example /1, /3, ...),
    /// which are indices to this string list. This allows to distinguish between
    /// different widget annotations, even if they have same value in m_options array.
    QStringList m_options;
};

/// Represents single line, or multiline text field
class PDF4QTLIBCORESHARED_EXPORT PDFFormFieldText : public PDFFormField
{
public:
    explicit inline PDFFormFieldText() = default;

    PDFInteger getTextMaximalLength() const { return m_maxLength; }
    const QByteArray& getDefaultAppearance() const { return m_defaultAppearance; }
    Qt::Alignment getAlignment() const { return m_alignment; }
    const QString& getRichTextDefaultStyle() const { return m_defaultStyle; }
    const QString& getRichTextValue() const { return m_richTextValue; }

    virtual bool setValue(const SetValueParameters& parameters) override;
    virtual void resetValue(const ResetValueParameters& parameters) override;

private:
    friend PDFFormFieldPointer PDFFormField::parse(const PDFObjectStorage* storage, PDFObjectReference reference, PDFFormField* parentField);

    /// Maximal length of text in the field. If zero,
    /// no maximal length is specified.
    PDFInteger m_maxLength = 0;

    /// Default appearance
    QByteArray m_defaultAppearance;

    /// Text field alignment
    Qt::Alignment m_alignment = Qt::Alignment();

    /// Default style
    QString m_defaultStyle;

    /// Rich text value
    QString m_richTextValue;
};

class PDF4QTLIBCORESHARED_EXPORT PDFFormFieldChoice : public PDFFormField
{
    using BaseClass = PDFFormField;

public:
    explicit inline PDFFormFieldChoice() = default;

    bool isComboBox() const { return m_fieldFlags.testFlag(Combo); }
    bool isEditableComboBox() const { return m_fieldFlags.testFlag(Edit); }
    bool isListBox() const { return !isComboBox(); }

    struct Option
    {
        QString exportString;
        QString userString;
    };

    using Options = std::vector<Option>;

    const Options& getOptions() const { return m_options; }
    PDFInteger getTopIndex() const { return m_topIndex; }
    const PDFObject& getSelection() const { return m_selection; }

    virtual bool setValue(const SetValueParameters& parameters) override;
    virtual void resetValue(const ResetValueParameters& parameters) override;
    virtual void reloadValue(const PDFObjectStorage* storage, PDFObject parentValue) override;

private:
    friend PDFFormFieldPointer PDFFormField::parse(const PDFObjectStorage* storage, PDFObjectReference reference, PDFFormField* parentField);

    Options m_options;
    PDFInteger m_topIndex;
    PDFObject m_selection;
};

class PDF4QTLIBCORESHARED_EXPORT PDFFormFieldSignature : public PDFFormField
{
public:
    explicit inline PDFFormFieldSignature() = default;

    const PDFSignature& getSignature() const { return m_signature; }

private:
    friend PDFFormFieldPointer PDFFormField::parse(const PDFObjectStorage* storage, PDFObjectReference reference, PDFFormField* parentField);

    PDFSignature m_signature;
};

/// This class represents interactive form. Interactive form fields can span multiple
/// document pages. So this object represents all interactive form fields in the document.
/// Fields forms tree-like structure, where leafs are usually widgets. Fields include
/// ordinary widgets, such as buttons, check boxes, combo boxes and text fields, and one
/// special - signature field, which represents digital signature.
class PDF4QTLIBCORESHARED_EXPORT PDFForm
{
public:
    explicit inline PDFForm() = default;

    enum class FormType
    {
        None,
        AcroForm,
        XFAForm
    };

    enum SignatureFlag
    {
        None            = 0x0000,
        SignatureExists = 0x0001, ///< If set, at least one signature exists in the document
        AppendOnly      = 0x0002, ///< If set, signature may be invalidated during rewrite
    };
    Q_DECLARE_FLAGS(SignatureFlags, SignatureFlag)

    FormType getFormType() const { return m_formType; }
    const PDFFormFields& getFormFields() const { return m_formFields; }
    bool isAppearanceUpdateNeeded() const { return m_needAppearances; }
    SignatureFlags getSignatureFlags() const { return m_signatureFlags; }
    const std::vector<PDFObjectReference>& getCalculationOrder() const { return m_calculationOrder; }
    const PDFObject& getResources() const { return m_resources; }
    const std::optional<QByteArray>& getDefaultAppearance() const { return m_defaultAppearance; }
    const std::optional<PDFInteger>& getQuadding() const { return m_quadding; }
    const PDFObject& getXFA() const { return m_xfa; }

    Qt::Alignment getDefaultAlignment() const;

    bool isAcroForm() const { return getFormType() == PDFForm::FormType::AcroForm; }
    bool isXFAForm() const { return getFormType() == PDFForm::FormType::XFAForm; }

    /// Returns form field for widget. If widget doesn't have attached form field,
    /// then nullptr is returned.
    /// \param widget Widget annotation
    const PDFFormField* getFormFieldForWidget(PDFObjectReference widget) const;

    /// Returns form field for widget. If widget doesn't have attached form field,
    /// then nullptr is returned.
    /// \param widget Widget annotation
    PDFFormField* getFormFieldForWidget(PDFObjectReference widget);

    /// Applies function to all form fields present in the form,
    /// in pre-order (first application is to the parent, following
    /// calls to apply for children).
    /// \param functor Functor to apply
    void apply(const std::function<void(const PDFFormField*)>& functor) const;

    /// Parses form from the object. If some error occurs
    /// then empty form is returned, no exception is thrown.
    /// \param document Document
    /// \param reference Field reference
    static PDFForm parse(const PDFDocument* document, PDFObject object);

private:
    void updateWidgetToFormFieldMapping();

    FormType m_formType = FormType::None;
    PDFFormFields m_formFields;
    bool m_needAppearances = false;
    SignatureFlags m_signatureFlags = None;
    std::vector<PDFObjectReference> m_calculationOrder;
    PDFObject m_resources;
    std::optional<QByteArray> m_defaultAppearance;
    std::optional<PDFInteger> m_quadding;
    PDFObject m_xfa;
    PDFWidgetToFormFieldMapping m_widgetToFormField;
};

/// Form manager. Manages all form widgets functionality - triggers actions,
/// edits fields, updates annotation appearances, etc. Valid pointer to annotation
/// manager is requirement.
class PDF4QTLIBCORESHARED_EXPORT PDFFormManager : public QObject
{
    Q_OBJECT

private:
    using BaseClass = QObject;

public:
    explicit PDFFormManager(QObject* parent);
    virtual ~PDFFormManager() override;

    enum FormAppearanceFlag
    {
        None                    = 0x0000,
        HighlightFields         = 0x0001,
        HighlightRequiredFields = 0x0002,
    };
    Q_DECLARE_FLAGS(FormAppearanceFlags, FormAppearanceFlag)

    bool hasForm() const { return hasAcroForm() || hasXFAForm(); }
    bool hasAcroForm() const { return m_form.getFormType() == PDFForm::FormType::AcroForm; }
    bool hasXFAForm() const { return m_form.getFormType() == PDFForm::FormType::XFAForm; }

    const PDFForm* getForm() const { return &m_form; }

    /// Returns form field for widget. If widget doesn't have attached form field,
    /// then nullptr is returned.
    /// \param widget Widget annotation
    const PDFFormField* getFormFieldForWidget(PDFObjectReference widget) const { return m_form.getFormFieldForWidget(widget); }

    /// Returns form field for widget. If widget doesn't have attached form field,
    /// then nullptr is returned.
    /// \param widget Widget annotation
    PDFFormField* getFormFieldForWidget(PDFObjectReference widget) { return m_form.getFormFieldForWidget(widget); }

    const PDFDocument* getDocument() const;
    void setDocument(const PDFModifiedDocument& document);

    FormAppearanceFlags getAppearanceFlags() const;
    void setAppearanceFlags(FormAppearanceFlags flags);

    /// Returns true, if form field has text (for example, it is a text box,
    /// or editable combo box)
    /// \param widgetAnnotation Widget annotation
    bool hasFormFieldWidgetText(PDFObjectReference widgetAnnotation) const;

    /// Returns all form field widgets. This function is not simple getter,
    /// call can be expensive, because all form fields are accessed.
    PDFFormWidgets getWidgets() const;

    /// Applies function to all form fields present in the form,
    /// in pre-order (first application is to the parent, following
    /// calls to apply for children).
    /// \param functor Functor to apply
    void apply(const std::function<void(const PDFFormField*)>& functor) const;

    /// Applies function to all form fields present in the form,
    /// in pre-order (first application is to the parent, following
    /// calls to apply for children).
    /// \param functor Functor to apply
    void modify(const std::function<void(PDFFormField*)>& functor) const;

    /// Tries to find appropriate action and returns it. If action is not found,
    /// then nullptr is returned.
    /// \param actionType Action to be performed
    /// \param widget Form field widget
    const PDFAction* getAction(PDFAnnotationAdditionalActions::Action actionType, const PDFFormWidget* widget);

    /// Returns default form apperance flags
    static constexpr FormAppearanceFlags getDefaultApperanceFlags() { return FormAppearanceFlags(HighlightFields | HighlightRequiredFields); }

    /// Tries to set value to the form field
    void setFormFieldValue(PDFFormField::SetValueParameters parameters);

    /// Get widget rectangle (from annotation)
    QRectF getWidgetRectangle(const PDFFormWidget& widget) const;

    /// Is committing data disabled?
    bool isCommitDisabled() const { return m_isCommitDisabled; }

    /// Performs reset action. Action must be valid pointer. Fields are resetted
    /// according to the criteria specified in reset action.
    /// \param action Reset action
    void performResetAction(const PDFActionResetForm* action);

    /// Draws XFA form, or does nothing, if XFA form is not present
    /// \param pagePointToDevicePointMatrix Page point to device point matrix
    /// \param page Page
    /// \param errors Error list (for reporting rendering errors)
    /// \param painter Painter
    void drawXFAForm(const QTransform& pagePointToDevicePointMatrix,
                     const PDFPage* page,
                     QList<PDFRenderError>& errors,
                     QPainter* painter);

    /// Performs paging, when XFA form needs to change page count and size.
    /// If some change needs to be done, then signal \p documentModified
    /// is emitted.
    void performPaging();

    /// Returns true, if widget is focused.
    /// \param widget Widget annotation reference
    virtual bool isFocused(PDFObjectReference widget) const;

    /// Is editor draw enabled?
    virtual bool isEditorDrawEnabled(const PDFObjectReference& reference) const;
    virtual bool isEditorDrawEnabled(const PDFFormField* formField) const;

    /// Draw form field widget using given parameters. It is used, when
    /// we want to draw editor contents on the painter using parameters.
    /// Parameter \p edit decides, if editor is drawn, or static contents
    /// based on field value is drawn.
    /// \param parameters Parameters
    /// \param edit Draw editor or static contents
    virtual void drawFormField(const PDFFormField* formField, AnnotationDrawParameters& parameters, bool edit) const;

protected:
    virtual void updateFieldValues();
    virtual void onDocumentReset() { }

signals:
    void actionTriggered(const pdf::PDFAction* action);
    void documentModified(pdf::PDFModifiedDocument document);

private:
    const PDFDocument* m_document;
    FormAppearanceFlags m_flags;
    PDFForm m_form;
    bool m_isCommitDisabled;

    PDFXFAEngine m_xfaEngine;
};

}   // namespace pdf

#endif // PDFFORM_H
