/***************************************************************************
                         qgsabstractrelationeditorwidget.h
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTRELATIONEDITORWIDGET_H
#define QGSABSTRACTRELATIONEDITORWIDGET_H

#include <QWidget>
#include "qobjectuniqueptr.h"

#include "qgsattributeeditorcontext.h"
#include "qgsrelation.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

#ifdef SIP_RUN
// this is needed for the "convert to subclass" code below to compile
% ModuleHeaderCode
#include "qgsrelationeditorwidget.h"
% End
#endif

class QgsFeature;
class QgsVectorLayer;

/**
 * Base class to build new relation widgets.
 * \ingroup gui
 * \class QgsAbstractRelationEditorWidget
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsAbstractRelationEditorWidget : public QWidget
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsRelationEditorWidget *>( sipCpp ) )
      sipType = sipType_QgsRelationEditorWidget;
    else
      sipType = 0;
    SIP_END
#endif

    Q_OBJECT

  public:


    /**
     * Constructor
     */
    QgsAbstractRelationEditorWidget( const QVariantMap &config, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a relation and the \a feature
     */
    void setRelationFeature( const QgsRelation &relation, const QgsFeature &feature );

    /**
     * Sets the relation(s) for this widget
     * If only one relation is set, it will act as a simple 1:N relation widget
     * If both relations are set, it will act as an N:M relation widget
     * inserting and deleting entries on the intermediate table as required.
     *
     * \param relation    Relation referencing the edited table
     * \param nmrelation  Optional reference from the referencing table to a 3rd N:M table
     */
    void setRelations( const QgsRelation &relation, const QgsRelation &nmrelation );

    /**
     * Returns the relation
     * \since QGIS 3.18
     */
    QgsRelation relation() const {return mRelation;}

    /**
     * Returns the nm relation
     * \since QGIS 3.18
     */
    QgsRelation nmRelation() const {return mNmRelation;}

    /**
     * Sets the \a feature being edited and updates the UI unless \a update is set to FALSE
     */
    void setFeature( const QgsFeature &feature, bool update = true );

    /**
     * Set multiple feature to edit simultaneously.
     * \param fids Multiple Id of features to edit
     * \since QGIS 3.24
     */
    void setMultiEditFeatureIds( const QgsFeatureIds &fids );

    /**
     * Sets the editor \a context
     * \note if context cadDockWidget is null, it won't be possible to digitize
     * the geometry of a referencing feature from this widget
     */
    virtual void setEditorContext( const QgsAttributeEditorContext &context );

    /**
     * Returns the attribute editor context.
     */
    QgsAttributeEditorContext editorContext( ) const;

    /**
     * Defines if a title label should be shown for this widget.
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED bool showLabel() const SIP_DEPRECATED;

    /**
     * Defines if a title label should be shown for this widget.
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED void setShowLabel( bool showLabel ) SIP_DEPRECATED;

    /**
    * Determines the relation id of the second relation involved in an N:M relation.
    */
    QVariant nmRelationId() const;

    /**
     * Sets \a nmRelationId for the relation id of the second relation involved in an N:M relation.
     * If it's empty, then it's considered as a 1:M relationship.
     */
    void setNmRelationId( const QVariant &nmRelationId = QVariant() );

    /**
     * Determines the label of this element
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED QString label() const SIP_DEPRECATED;

    /**
     * Sets \a label for this element
     * If it's empty it takes the relation id as label
     */
    void setLabel( const QString &label = QString() );

    /**
     * Returns the widget's current feature
     * If the widget is in multiedit mode only the first is returned
     * \see features
     */
    QgsFeature feature() const;

    /**
     * Returns the widget's current features
     * \since QGIS 3.24
     */
    QList<QgsFeature> features() const;

    /**
       * Determines the force suppress form popup status that is configured for this widget
       */
    bool forceSuppressFormPopup() const;

    /**
     * Sets force suppress form popup status with \a forceSuppressFormPopup
     * configured for this widget
     */
    void setForceSuppressFormPopup( bool forceSuppressFormPopup );

    /**
     * Returns the widget configuration
     */
    virtual QVariantMap config() const = 0;

    /**
     * Defines the widget configuration
     */
    virtual void setConfig( const QVariantMap &config ) = 0;

    /**
     * Returns true if editing multiple features at a time
     * \since QGIS 3.24
     */
    bool multiEditModeActive() const;

  signals:

    /**
     * Emit this signal, whenever the related features changed.
     * This happens for example when related features are added, removed,
     * linked or unlinked.
     *
     * \since QGIS 3.22
     */
    void relatedFeaturesChanged();

  public slots:

    /**
     * Called when an \a attribute value in the parent widget has changed to \a newValue
     */
    virtual void parentFormValueChanged( const QString &attribute, const QVariant &newValue ) = 0;

  protected slots:

    /**
     * Toggles editing state of the widget
     */
    void toggleEditing( bool state );

    /**
     * Saves the current modifications in the relation
     */
    void saveEdits();

    /**
     * Adds new features with given \a geometry
     * Returns the Id of added features \since QGIS 3.24
     */
    QgsFeatureIds addFeature( const QgsGeometry &geometry = QgsGeometry() );

    /**
     * Delete a feature with given \a fid
     */
    void deleteFeature( QgsFeatureId fid = QgsFeatureId() );

    /**
     * Links a new feature to the relation
     */
    void linkFeature();

    /**
     * Called when the link feature dialog is confirmed by the user
     */
    void onLinkFeatureDlgAccepted();

    /**
     * Unlinks a feature with given \a fid
     */
    void unlinkFeature( QgsFeatureId fid = QgsFeatureId() );

    /**
     * Duplicates a feature
     */
    void duplicateFeature( const QgsFeatureId &fid );

    /**
     * Duplicates features
     */
    void duplicateFeatures( const QgsFeatureIds &fids );

  protected:

    QgsAttributeEditorContext mEditorContext;
    QgsRelation mRelation;
    QgsRelation mNmRelation;
    QgsFeatureList mFeatureList;

    bool mLayerInSameTransactionGroup = false;

    bool mForceSuppressFormPopup = false;

    /**
     * Refresh the UI when the widget becomes visible
     */
    void showEvent( QShowEvent * );

    /**
     * Updates the title contents to reflect the current state of the widget
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED void updateTitle() SIP_DEPRECATED;

    /**
     * Deletes the features with \a fids
     */
    void deleteFeatures( const QgsFeatureIds &fids );

    /**
     * Unlinks the features with \a fids
     */
    void unlinkFeatures( const QgsFeatureIds &fids );

    // Following virtual methods need to be protected so they can be overridden in bindings

    /**
     * A hook called every time the state of the relation editor widget has changed via calling its `set*` methods or slots,
     * e.g. changed relation, added feature, etc.
     * Should be used to refresh the UI regarding the new data.
     * Check QgsRealationEditorWidget as an example.
     */
    virtual void updateUi();

    /**
     * Sets the title of the widget, if it is wrapped within a QgsCollapsibleGroupBox
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED virtual void setTitle( const QString &title ) SIP_DEPRECATED;

    /**
     * A hook called right before setRelationFeature() is executed. Used to update the UI once setting the relation feature is done.
     * Check QgsRealationEditorWidget as an example.
     */
    virtual void beforeSetRelationFeature( const QgsRelation &newRelation, const QgsFeature &newFeature );

    /**
     * A hook called right after setRelationFeature() is executed, but before updateUi() is called. Used to update the UI once setting the relation feature is done.
     * Check QgsRealationEditorWidget as an example.
     */
    virtual void afterSetRelationFeature();

    /**
     * A hook called right before setRelations() is executed. Used to manipulate UI once setting the relations is done.
     * Check QgsRealationEditorWidget as an example.
     */
    virtual void beforeSetRelations( const QgsRelation &newRelation, const QgsRelation &newNmRelation );

    /**
     * A hook called right after setRelations() is executed, but before updateUi() is called. Used to update the UI once setting the relations is done.
     * Check QgsRealationEditorWidget as an example.
     */
    virtual void afterSetRelations();
};


/**
 * \ingroup gui
 * \brief This class should be subclassed for every configurable relation widget type.
 *
 * It implements the GUI configuration widget and transforms this to/from a configuration.
 *
 * It will only be instantiated by {\see QgsAbstractRelationEditorWidgetFactory}
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsAbstractRelationEditorConfigWidget : public QWidget
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsRelationEditorConfigWidget *>( sipCpp ) )
      sipType = sipType_QgsRelationEditorConfigWidget;
    else
      sipType = 0;
    SIP_END
#endif

    Q_OBJECT
  public:

    /**
     * Create a new configuration widget
     *
     * \param relation    The relation for which the configuration dialog will be created
     * \param parent      A parent widget
     */
    explicit QgsAbstractRelationEditorConfigWidget( const QgsRelation &relation, QWidget *parent SIP_TRANSFERTHIS );

    /**
     * \brief Create a configuration from the current GUI state
     *
     * \returns A widget configuration
     */
    virtual QVariantMap config() = 0;

    /**
     * \brief Update the configuration widget to represent the given configuration.
     *
     * \param config The configuration which should be represented by this widget
     */
    virtual void setConfig( const QVariantMap &config ) = 0;

    /**
     * Returns the layer for which this configuration widget applies
     *
     * \returns The layer
     */
    QgsVectorLayer *layer();

    /**
     * Returns the relation for which this configuration widget applies
     *
     * \returns The relation
     */
    QgsRelation relation() const;

    /**
     * \brief Set the nm relation for this widget.
     *
     * \param nmRelation The nm relation
     */
    virtual void setNmRelation( const QgsRelation &nmRelation );

    /**
     * Returns the nm relation for which this configuration widget applies
     *
     * \returns The nm relation
     */
    virtual QgsRelation nmRelation() const;

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsRelation mRelation;
    QgsRelation mNmRelation;
};


///////////////////////////////


/**
 * Factory class for creating relation widgets and their corresponding config widgets
 * \ingroup gui
 * \class QgsAbstractRelationEditorWidgetFactory
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsAbstractRelationEditorWidgetFactory
{
  public:

    /**
     * Creates a new relation widget factory with given \a name
     */
    QgsAbstractRelationEditorWidgetFactory();

    virtual ~QgsAbstractRelationEditorWidgetFactory() = default;

    /**
     * Returns the machine readable identifier name of this widget type
     */
    virtual QString type() const = 0;

    /**
     * Returns the human readable identifier name of this widget type
     */
    virtual QString name() const = 0;

    /**
     * Override this in your implementation.
     * Create a new relation widget. Call QgsEditorWidgetRegistry::create()
     * instead of calling this method directly.
     *
     * \param config   The widget configuration to build the widget with
     * \param parent   The parent for the wrapper class and any created widget.
     *
     * \returns         A new widget wrapper
     */
    virtual QgsAbstractRelationEditorWidget *create( const QVariantMap &config, QWidget *parent = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Override this in your implementation.
     * Create a new configuration widget for this widget type.
     *
     * \param relation The relation for which the widget will be created
     * \param parent   The parent widget of the created config widget
     *
     * \returns         A configuration widget
     */
    virtual QgsAbstractRelationEditorConfigWidget *configWidget( const QgsRelation &relation, QWidget *parent ) const = 0 SIP_FACTORY;
};

#endif // QGSABSTRACTRELATIONEDITORWIDGET_H
