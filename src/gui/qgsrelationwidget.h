/***************************************************************************
                         qgsrelationwidget.h
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

#ifndef QGSRELATIONWIDGET_H
#define QGSRELATIONWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QGridLayout>
#include "qobjectuniqueptr.h"

#include "qobjectuniqueptr.h"
#include "qgsattributeeditorcontext.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsdualview.h"
#include "qgsrelation.h"
#include "qgsvectorlayerselectionmanager.h"
#include "qgis_gui.h"

class QgsFeature;
class QgsVectorLayer;
class QgsVectorLayerTools;
class QgsMapTool;
class QgsMapToolDigitizeFeature;


/**
 * Base class to build new relation widgets.
 * \ingroup gui
 * \class QgsRelationWidget
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsRelationWidget : public QWidget
{
    Q_OBJECT

  public:


    /**
     * Constructor
     */
    QgsRelationWidget( const QVariantMap &config, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a relation and the \a feature
     */
    void setRelationFeature( const QgsRelation &relation, const QgsFeature &feature );

    /**
     * Set the relation(s) for this widget
     * If only one relation is set, it will act as a simple 1:N relation widget
     * If both relations are set, it will act as an N:M relation widget
     * inserting and deleting entries on the intermediate table as required.
     *
     * \param relation    Relation referencing the edited table
     * \param nmrelation  Optional reference from the referencing table to a 3rd N:M table
     */
    void setRelations( const QgsRelation &relation, const QgsRelation &nmrelation );

    /**
     * Sets the \a feature being edited and updates the UI unless \a update is set to FALSE
     */
    void setFeature( const QgsFeature &feature, bool update = true );

    /**
     * Sets the editor \a context
     * \note if context cadDockWidget is null, it won't be possible to digitize
     * the geometry of a referencing feature from this widget
     */
    void setEditorContext( const QgsAttributeEditorContext &context );

    /**
     * Returns the attribute editor context.
     */
    QgsAttributeEditorContext editorContext( ) const;

    /**
     * The feature selection manager is responsible for the selected features
     * which are currently being edited.
     */
    QgsIFeatureSelectionManager *featureSelectionManager();

    /**
     * Defines if a title label should be shown for this widget.
     */
    bool showLabel() const;

    /**
     * Defines if a title label should be shown for this widget.
     */
    void setShowLabel( bool showLabel );

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
     */
    QString label() const;

    /**
     * Sets \a label for this element
     * If it's empty it takes the relation id as label
     */
    void setLabel( const QString &label = QString() );

    /**
     * Returns the widget's current feature
     */
    QgsFeature feature() const;

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
     * Adds a new feature with given \a geometry
     */
    void addFeature( const QgsGeometry &geometry = QgsGeometry() );

    /**
     * Delete a feature with given \a fid
     */
    void deleteFeature( QgsFeatureId fid = QgsFeatureId() );

    /**
     * Deletes the currently selected features
     */
    void deleteSelectedFeatures();

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
     * Unlinks the selected features from the relation
     */
    void unlinkSelectedFeatures();

    /**
     * Duplicates a feature
     */
    void duplicateFeature();

    /**
     * Zooms to the selected features
     */
    void zoomToSelectedFeatures();

  protected:

    QgsVectorLayerSelectionManager *mFeatureSelectionMgr = nullptr;
    QgsAttributeEditorContext mEditorContext;
    QgsRelation mRelation;
    QgsRelation mNmRelation;
    QgsFeature mFeature;

    bool mShowLabel = true;
    bool mLayerInSameTransactionGroup = false;

    bool mForceSuppressFormPopup = false;
    QVariant mNmRelationId;
    QString mLabel;

    /**
     * Updates the title contents to reflect the current state of the widget
     */
    void updateTitle();

    /**
     * Deletes the features with \a fids
     */
    void deleteFeatures( const QgsFeatureIds &fids );

    /**
     * Unlinks the features with \a fids
     */
    void unlinkFeatures( const QgsFeatureIds &fids );

  private:
    virtual void updateUi() {};
    virtual void setTitle( const QString &title ) { Q_UNUSED( title ); };
    virtual void beforeSetRelationFeature( const QgsRelation &newRelation, const QgsFeature &newFeature ) { Q_UNUSED( newRelation ); Q_UNUSED( newFeature ); };
    virtual void afterSetRelationFeature() {};
    virtual void beforeSetRelations( const QgsRelation &newRelation, const QgsRelation &newNmRelation ) { Q_UNUSED( newRelation ); Q_UNUSED( newNmRelation ); };
    virtual void afterSetRelations() {};
};

#endif // QGSRELATIONWIDGET_H
