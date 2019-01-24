/***************************************************************************
    qgsattributeform.h
     --------------------------------------
    Date                 : 3.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEFORM_H
#define QGSATTRIBUTEFORM_H

#include "qgsfeature.h"
#include "qgis_sip.h"
#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetwrapper.h"

#include <QWidget>
#include <QSvgWidget>
#include <QLabel>
#include <QDialogButtonBox>
#include "qgis_gui.h"


class QgsAttributeFormInterface;
class QgsAttributeFormEditorWidget;
class QgsMessageBar;
class QgsMessageBarItem;
class QgsWidgetWrapper;
class QgsTabWidget;
class QgsAttributeFormWidget;

/**
 * \ingroup gui
 * \class QgsAttributeForm
 */
class GUI_EXPORT QgsAttributeForm : public QWidget
{
    Q_OBJECT

  public:

    //! Form modes \deprecated Use QgsAttributeEditorContext::Mode instead.
    enum Mode
    {
      SingleEditMode, //!< Single edit mode, for editing a single feature
      AddFeatureMode, /*!< Add feature mode, for setting attributes for a new feature. In this mode the dialog will be editable even with an invalid feature and
      will add a new feature when the form is accepted. */
      MultiEditMode, //!< Multi edit mode, for editing fields of multiple features at once
      SearchMode, //!< Form values are used for searching/filtering the layer
      AggregateSearchMode, //!< Form is in aggregate search mode, show each widget in this mode \since QGIS 3.0
      IdentifyMode //!< Identify the feature \since QGIS 3.0
    };

    //! Filter types
    enum FilterType
    {
      ReplaceFilter, //!< Filter should replace any existing filter
      FilterAnd, //!< Filter should be combined using "AND"
      FilterOr, //!< Filter should be combined using "OR"
    };

    explicit QgsAttributeForm( QgsVectorLayer *vl,
                               const QgsFeature &feature = QgsFeature(),
                               const QgsAttributeEditorContext &context = QgsAttributeEditorContext(),
                               QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsAttributeForm() override;

    const QgsFeature &feature() { return mFeature; }

    /**
     * Hides the button box (OK/Cancel) and enables auto-commit
     * \note set Embed in QgsAttributeEditorContext in constructor instead
     */
    // TODO QGIS 3.0 - make private
    void hideButtonBox();

    /**
     * Shows the button box (OK/Cancel) and disables auto-commit
     * \note set Embed in QgsAttributeEditorContext in constructor instead
     */
    // TODO QGIS 3.0 - make private
    void showButtonBox();

    /**
     * Disconnects the button box (OK/Cancel) from the accept/resetValues slots
     * If this method is called, you have to create these connections from outside
     */
    // TODO QGIS 3.0 - make private
    void disconnectButtonBox();

    /**
     * Takes ownership
     * \param iface
     */
    void addInterface( QgsAttributeFormInterface *iface SIP_TRANSFER );

    /**
     * Returns the layer for which this form is shown
     *
     * \returns  Layer
     */
    QgsVectorLayer *layer() { return mLayer; }

    /**
     * Returns if the form is currently in editable mode.
     *
     * \returns Editable mode of this form
     */
    bool editable();

    /**
     * Returns the current mode of the form.
     * \see setMode()
     * \since QGIS 2.16
     */
    QgsAttributeEditorContext::Mode mode() const { return mMode; }

    /**
     * Sets the current mode of the form.
     * \param mode form mode
     * \see mode()
     * \since QGIS 2.16
     */
    void setMode( QgsAttributeEditorContext::Mode mode );

    /**
     * Sets the edit command message (Undo) that will be used when the dialog is accepted
     *
     * \param message The message
     */
    void setEditCommandMessage( const QString &message ) { mEditCommandMessage = message; }

    /**
     * Intercepts keypress on custom form (escape should not close it)
     *
     * \param object   The object for which the event has been sent
     * \param event    The event which is being filtered
     *
     * \returns         true if the event has been handled (key was ESC)
     */
    bool eventFilter( QObject *object, QEvent *event ) override;

    /**
     * Sets all feature IDs which are to be edited if the form is in multiedit mode
     * \param fids feature ID list
     * \since QGIS 2.16
     */
    void setMultiEditFeatureIds( const QgsFeatureIds &fids );

    /**
     * Sets the message bar to display feedback from the form in. This is used in the search/filter
     * mode to display the count of selected features.
     * \param messageBar target message bar
     * \since QGIS 2.16
     */
    void setMessageBar( QgsMessageBar *messageBar );

    /**
     * The aggregate filter is only useful if the form is in AggregateFilter mode.
     * In this case it will return a combined expression according to the chosen filters
     * on all attribute widgets.
     *
     * \since QGIS 3.0
     */
    QString aggregateFilter() const;

  signals:

    /**
     * Notifies about changes of attributes, this signal is not emitted when the value is set
     * back to the original one.
     *
     * \param attribute The name of the attribute that changed.
     * \param value     The new value of the attribute.
     * \deprecated since 3.0
     */
    Q_DECL_DEPRECATED void attributeChanged( const QString &attribute, const QVariant &value ) SIP_DEPRECATED;

    /**
     * Notifies about changes of attributes
     *
     * \param attribute The name of the attribute that changed.
     * \param value     The new value of the attribute.
     * \param attributeChanged If true, it corresponds to an actual change of the feature attribute
     * \since QGIS 3.0.1
     */
    void widgetValueChanged( const QString &attribute, const QVariant &value, bool attributeChanged );

    /**
     * Will be emitted before the feature is saved. Use this signal to perform sanity checks.
     * You can set the parameter ok to false to notify the form that you don't want it to be saved.
     * If you want the form to be saved, leave the parameter untouched.
     *
     * \param ok  Set this parameter to false if you don't want the form to be saved
     * \note not available  in Python bindings
     */
    void beforeSave( bool &ok ) SIP_SKIP;

    /**
     * Is emitted, when a feature is changed or added
     */
    void featureSaved( const QgsFeature &feature );

    /**
     * Is emitted when a filter expression is set using the form.
     * \param expression filter expression
     * \param type filter type
     * \since QGIS 2.16
     */
    void filterExpressionSet( const QString &expression, QgsAttributeForm::FilterType type );

    /**
     * Emitted when the form changes mode.
     * \param mode new mode
     */
    void modeChanged( QgsAttributeEditorContext::Mode mode );

    /**
     * Emitted when the user selects the close option from the form's button bar.
     * \since QGIS 2.16
     */
    void closed();

    /**
     * Emitted when the user chooses to zoom to a filtered set of features.
     * \since QGIS 3.0
     */
    void zoomToFeatures( const QString &filter );

    /**
     * Emitted when the user chooses to flash a filtered set of features.
     * \since QGIS 3.0
     */
    void flashFeatures( const QString &filter );

  public slots:

    /**
     * Call this to change the content of a given attribute. Will update the editor(s) related to this field.
     *
     * \param field The field to change
     * \param value The new value
     * \param hintText A hint text for non existent joined features
     */
    void changeAttribute( const QString &field, const QVariant &value, const QString &hintText = QString() );

    /**
     * Update all editors to correspond to a different feature.
     *
     * \param feature The feature which will be represented by the form
     */
    void setFeature( const QgsFeature &feature );

    /**
     * Save all the values from the editors to the layer.
     *
     * \returns True if successful
     */
    bool save();

    /**
     * Sets all values to the values of the current feature
     */
    void resetValues();

    /**
     * Resets the search/filter form values.
     * \since QGIS 2.16
     */
    void resetSearch();

    /**
     * reload current feature
     */
    void refreshFeature();

  private slots:
    void onAttributeChanged( const QVariant &value );
    void onAttributeAdded( int idx );
    void onAttributeDeleted( int idx );
    void onUpdatedFields();
    void onConstraintStatusChanged( const QString &constraint,
                                    const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result );
    void preventFeatureRefresh();
    void synchronizeEnabledState();
    void layerSelectionChanged();

    //! Save multi edit changes
    bool saveMultiEdits();
    void resetMultiEdit( bool promptToSave = false );
    void multiEditMessageClicked( const QString &link );

    void filterAndTriggered();
    void filterOrTriggered();
    void filterTriggered();

    void searchZoomTo();
    void searchFlash();
    void searchSetSelection();
    void searchAddToSelection();
    void searchRemoveFromSelection();
    void searchIntersectSelection();

  private:
    void init();

    void cleanPython();

    void initPython();

    void updateJoinedFields( const QgsEditorWidgetWrapper &eww );

    bool fieldIsEditable( int fieldIndex ) const;

    bool fieldIsEditable( const QgsVectorLayer &layer, int fieldIndex, QgsFeatureId fid ) const;

    struct WidgetInfo
    {
      QWidget *widget = nullptr;
      QString labelText;
      QString toolTip;
      QString hint;
      bool labelOnTop = false;
      bool labelAlignRight = false;
      bool showLabel = true;
    };

    WidgetInfo createWidgetFromDef( const QgsAttributeEditorElement *widgetDef, QWidget *parent, QgsVectorLayer *vl, QgsAttributeEditorContext &context );

    void addWidgetWrapper( QgsEditorWidgetWrapper *eww );

    /**
     * Creates widget wrappers for all suitable widgets found.
     * Called once maximally.
     */
    void createWrappers();
    void afterWidgetInit();

    void scanForEqualAttributes( QgsFeatureIterator &fit, QSet< int > &mixedValueFields, QHash< int, QVariant > &fieldSharedValues ) const;

    //! Save single feature or add feature edits
    bool saveEdits();

    int messageTimeout();
    void clearMultiEditMessages();
    void pushSelectedFeaturesMessage();
    void runSearchSelect( QgsVectorLayer::SelectBehavior behavior );

    QString createFilterExpression() const;

    //! constraints management
    void updateAllConstraints();
    void updateConstraints( QgsEditorWidgetWrapper *w );
    void updateContainersVisibility();
    void updateConstraint( const QgsFeature &ft, QgsEditorWidgetWrapper *eww );
    bool currentFormFeature( QgsFeature &feature );
    bool currentFormValidConstraints( QStringList &invalidFields, QStringList &descriptions );
    QList<QgsEditorWidgetWrapper *> constraintDependencies( QgsEditorWidgetWrapper *w );

    QgsVectorLayer *mLayer = nullptr;
    QgsFeature mFeature;
    QgsMessageBar *mMessageBar = nullptr;
    bool mOwnsMessageBar;
    QgsMessageBarItem *mMultiEditUnsavedMessageBarItem = nullptr;
    QgsMessageBarItem *mMultiEditMessageBarItem = nullptr;
    QList<QgsWidgetWrapper *> mWidgets;
    QgsAttributeEditorContext mContext;
    QDialogButtonBox *mButtonBox = nullptr;
    QWidget *mSearchButtonBox = nullptr;
    QList<QgsAttributeFormInterface *> mInterfaces;
    QMap< int, QgsAttributeFormEditorWidget * > mFormEditorWidgets;
    QList< QgsAttributeFormWidget *> mFormWidgets;
    QgsExpressionContext mExpressionContext;
    QMap<const QgsVectorLayerJoinInfo *, QgsFeature> mJoinedFeatures;
    bool mValuesInitialized = false;
    bool mDirty = false;
    bool mIsSettingFeature = false;

    struct ContainerInformation
    {
      ContainerInformation( QgsTabWidget *tabWidget, QWidget *widget, const QgsExpression &expression )
        : tabWidget( tabWidget )
        , widget( widget )
        , expression( expression )
        , isVisible( true )
      {}

      ContainerInformation( QWidget *widget, const QgsExpression &expression )
        : widget( widget )
        , expression( expression )
        , isVisible( true )
      {}

      QgsTabWidget *tabWidget = nullptr;
      QWidget *widget = nullptr;
      QgsExpression expression;
      bool isVisible;

      void apply( QgsExpressionContext *expressionContext );
    };

    void registerContainerInformation( ContainerInformation *info );

    void updateIcon( QgsEditorWidgetWrapper *eww );

    void reloadIcon( const QString &file, const QString &tooltip, QSvgWidget *sw );

    // Contains information about tabs and groupboxes, their visibility state visibility conditions
    QVector<ContainerInformation *> mContainerVisibilityInformation;
    QMap<QString, QVector<ContainerInformation *> > mContainerInformationDependency;

    // Variables below are used for Python
    static int sFormCounter;
    int mFormNr;
    QString mPyFormVarName;

    //! Sets to true while saving to prevent recursive saves
    bool mIsSaving;

    //! Flag to prevent refreshFeature() to change mFeature
    bool mPreventFeatureRefresh;

    bool mIsSettingMultiEditFeatures;

    QgsFeatureIds mMultiEditFeatureIds;
    bool mUnsavedMultiEditChanges;

    QString mEditCommandMessage;

    QgsAttributeEditorContext::Mode mMode;

    QMap<QWidget *, QSvgWidget *> mIconMap;

    friend class TestQgsDualView;
    friend class TestQgsAttributeForm;
};

#endif // QGSATTRIBUTEFORM_H

