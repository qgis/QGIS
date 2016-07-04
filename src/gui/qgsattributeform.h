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
#include "qgsvectorlayer.h"
#include "qgsattributeeditorcontext.h"

#include <QWidget>
#include <QLabel>
#include <QDialogButtonBox>

class QgsAttributeFormInterface;
class QgsAttributeFormEditorWidget;
class QgsMessageBar;
class QgsMessageBarItem;
class QgsWidgetWrapper;

/** \ingroup gui
 * \class QgsAttributeForm
 */
class GUI_EXPORT QgsAttributeForm : public QWidget
{
    Q_OBJECT

  public:

    //! Form modes
    enum Mode
    {
      SingleEditMode, /*!< Single edit mode, for editing a single feature */
      AddFeatureMode, /*!< Add feature mode, for setting attributes for a new feature. In this mode the dialog will be editable even with an invalid feature and
      will add a new feature when the form is accepted. */
      MultiEditMode, /*!< Multi edit mode, for editing fields of multiple features at once */
      SearchMode, /*!< Form values are used for searching/filtering the layer */
    };

    //! Filter types
    enum FilterType
    {
      ReplaceFilter, /*!< Filter should replace any existing filter */
      FilterAnd, /*!< Filter should be combined using "AND" */
      FilterOr, /*!< Filter should be combined using "OR" */
    };

    explicit QgsAttributeForm( QgsVectorLayer* vl, const QgsFeature &feature = QgsFeature(),
                               const QgsAttributeEditorContext& context = QgsAttributeEditorContext(), QWidget *parent = nullptr );
    ~QgsAttributeForm();

    const QgsFeature& feature() { return mFeature; }

    /**
     * Hides the button box (Ok/Cancel) and enables auto-commit
     * @note set Embed in QgsAttributeEditorContext in constructor instead
     */
    // TODO QGIS 3.0 - make private
    void hideButtonBox();

    /**
     * Shows the button box (Ok/Cancel) and disables auto-commit
     * @note set Embed in QgsAttributeEditorContext in constructor instead
     */
    // TODO QGIS 3.0 - make private
    void showButtonBox();

    /**
     * Disconnects the button box (Ok/Cancel) from the accept/resetValues slots
     * If this method is called, you have to create these connections from outside
     */
    // TODO QGIS 3.0 - make private
    void disconnectButtonBox();

    /**
     * Takes ownership
     * @param iface
     */
    void addInterface( QgsAttributeFormInterface* iface );

    /**
     * Returns the layer for which this form is shown
     *
     * @return  Layer
     */
    QgsVectorLayer* layer() { return mLayer; }

    /**
     * Returns if the form is currently in editable mode.
     *
     * @return Editable mode of this form
     */
    bool editable();

    /** Returns the current mode of the form.
     * @note added in QGIS 2.16
     * @see setMode()
     */
    Mode mode() const { return mMode; }

    /** Sets the current mode of the form.
     * @param mode form mode
     * @note added in QGIS 2.16
     * @see mode()
     */
    void setMode( Mode mode );

    /**
     * Toggles the form mode between edit feature and add feature.
     * If set to true, the dialog will be editable even with an invalid feature.
     * If set to true, the dialog will add a new feature when the form is accepted.
     *
     * @param isAddDialog If set to true, turn this dialog into an add feature dialog.
     * @deprecated use setMode() instead
     */
    Q_DECL_DEPRECATED void setIsAddDialog( bool isAddDialog );

    /**
     * Sets the edit command message (Undo) that will be used when the dialog is accepted
     *
     * @param message The message
     */
    void setEditCommandMessage( const QString& message ) { mEditCommandMessage = message; }

    /**
     * Intercepts keypress on custom form (escape should not close it)
     *
     * @param object   The object for which the event has been sent
     * @param event    The event which is being filtered
     *
     * @return         true if the event has been handled (key was ESC)
     */
    bool eventFilter( QObject* object, QEvent* event ) override;

    /** Sets all feature IDs which are to be edited if the form is in multiedit mode
     * @param fids feature ID list
     * @note added in QGIS 2.16
     */
    void setMultiEditFeatureIds( const QgsFeatureIds& fids );

    /** Sets the message bar to display feedback from the form in. This is used in the search/filter
     * mode to display the count of selected features.
     * @param messageBar target message bar
     * @note added in QGIS 2.16
     */
    void setMessageBar( QgsMessageBar* messageBar );

  signals:
    /**
     * Notifies about changes of attributes
     *
     * @param attribute The name of the attribute that changed.
     * @param value     The new value of the attribute.
     */
    void attributeChanged( const QString& attribute, const QVariant& value );

    /**
     * Will be emitted before the feature is saved. Use this signal to perform sanity checks.
     * You can set the parameter ok to false to notify the form that you don't want it to be saved.
     * If you want the form to be saved, leave the parameter untouched.
     *
     * @param ok  Set this parameter to false if you don't want the form to be saved
     * @note not available  in python bindings
     */
    void beforeSave( bool& ok );

    /**
     * Is emitted, when a feature is changed or added
     */
    void featureSaved( const QgsFeature& feature );

    /** Is emitted when a filter expression is set using the form.
     * @param expression filter expression
     * @param type filter type
     * @note added in QGIS 2.16
     */
    void filterExpressionSet( const QString& expression, QgsAttributeForm::FilterType type );

    /** Emitted when the form changes mode.
     * @param mode new mode
     */
    void modeChanged( QgsAttributeForm::Mode mode );

    /** Emitted when the user selects the close option from the form's button bar.
     * @note added in QGIS 2.16
     */
    void closed();

  public slots:
    /**
     * Call this to change the content of a given attribute. Will update the editor(s) related to this field.
     *
     * @param field The field to change
     * @param value The new value
     */
    void changeAttribute( const QString& field, const QVariant& value );

    /**
     * Update all editors to correspond to a different feature.
     *
     * @param feature The feature which will be represented by the form
     */
    void setFeature( const QgsFeature& feature );

    /**
     * Save all the values from the editors to the layer.
     *
     * @return True if successful
     */
    bool save();

    /**
     * Alias for save()
     *
     * @deprecated
     */
    Q_DECL_DEPRECATED void accept() { save(); }

    /**
     * Alias for resetValues()
     *
     * @deprecated
     */
    Q_DECL_DEPRECATED void reject() { resetValues(); }

    /**
     * Sets all values to the values of the current feature
     */
    void resetValues();

    /** Resets the search/filter form values.
     * @note added in QGIS 2.16
     */
    void resetSearch();

    /**
     * reload current feature
     */
    void refreshFeature();

  private slots:
    void onAttributeChanged( const QVariant& value );
    void onAttributeAdded( int idx );
    void onAttributeDeleted( int idx );
    void onUpdatedFields();
    void onConstraintStatusChanged( const QString& constraint,
                                    const QString &description, const QString& err, bool ok );
    void preventFeatureRefresh();
    void synchronizeEnabledState();
    void layerSelectionChanged();

    //! Save multi edit changes
    bool saveMultiEdits();
    void resetMultiEdit( bool promptToSave = false );
    void multiEditMessageClicked( const QString& link );

    void filterAndTriggered();
    void filterOrTriggered();
    void filterTriggered();

    void searchSetSelection();
    void searchAddToSelection();
    void searchRemoveFromSelection();
    void searchIntersectSelection();

  private:
    void init();

    void cleanPython();

    void initPython();

    struct WidgetInfo
    {
      WidgetInfo()
          : widget( nullptr )
          , labelOnTop( false )
          , labelAlignRight( false )
      {}

      QWidget* widget;
      QString labelText;
      bool labelOnTop;
      bool labelAlignRight;
    };

    WidgetInfo createWidgetFromDef( const QgsAttributeEditorElement* widgetDef, QWidget* parent, QgsVectorLayer* vl, QgsAttributeEditorContext& context );

    void addWidgetWrapper( QgsEditorWidgetWrapper* eww );

    /**
     * Creates widget wrappers for all suitable widgets found.
     * Called once maximally.
     */
    void createWrappers();
    void afterWidgetInit();

    void scanForEqualAttributes( QgsFeatureIterator& fit, QSet< int >& mixedValueFields, QHash< int, QVariant >& fieldSharedValues ) const;

    //! Save single feature or add feature edits
    bool saveEdits();

    int messageTimeout();
    void clearMultiEditMessages();
    void pushSelectedFeaturesMessage();
    void runSearchSelect( QgsVectorLayer::SelectBehaviour behaviour );

    QString createFilterExpression() const;

    //! constraints management
    void updateAllConstaints();
    void updateConstraints( QgsEditorWidgetWrapper *w );
    bool currentFormFeature( QgsFeature &feature );
    bool currentFormValidConstraints( QStringList &invalidFields, QStringList &descriptions );
    void constraintDependencies( QgsEditorWidgetWrapper *w, QList<QgsEditorWidgetWrapper*> &wDeps );
    void clearInvalidConstraintsMessage();
    void displayInvalidConstraintMessage( const QStringList &invalidFields,
                                          const QStringList &description );

    QgsVectorLayer* mLayer;
    QgsFeature mFeature;
    QgsMessageBar* mMessageBar;
    bool mOwnsMessageBar;
    QgsMessageBarItem* mMultiEditUnsavedMessageBarItem;
    QgsMessageBarItem* mMultiEditMessageBarItem;
    QLabel* mInvalidConstraintMessage;
    QList<QgsWidgetWrapper*> mWidgets;
    QgsAttributeEditorContext mContext;
    QDialogButtonBox* mButtonBox;
    QWidget* mSearchButtonBox;
    QList<QgsAttributeFormInterface*> mInterfaces;
    QMap< int, QgsAttributeFormEditorWidget* > mFormEditorWidgets;

    // Variables below are used for python
    static int sFormCounter;
    int mFormNr;
    QString mPyFormVarName;

    //! Set to true while saving to prevent recursive saves
    bool mIsSaving;

    //! Flag to prevent refreshFeature() to change mFeature
    bool mPreventFeatureRefresh;

    //! Set to true while setting feature to prevent attributeChanged signal
    bool mIsSettingFeature;
    bool mIsSettingMultiEditFeatures;

    QgsFeatureIds mMultiEditFeatureIds;
    bool mUnsavedMultiEditChanges;

    QString mEditCommandMessage;

    Mode mMode;

    //! Backlinks widgets to buddies.
    QMap<QWidget*, QLabel*> mBuddyMap;

    friend class TestQgsDualView;
    friend class TestQgsAttributeForm;
};

#endif // QGSATTRIBUTEFORM_H

