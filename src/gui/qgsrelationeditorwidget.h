/***************************************************************************
    qgsrelationeditorwidget.h
     --------------------------------------
    Date                 : 17.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONEDITOR_H
#define QGSRELATIONEDITOR_H

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

#ifdef SIP_RUN
% ModuleHeaderCode
// fix to allow compilation with sip that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsrelationeditorwidget.h>
% End
#endif


/// @cond PRIVATE
#ifndef SIP_RUN

/**
 * This class is used to filter the current vector layer selection to features matching the given request.
 * Relation editor widget use it in order to get selected feature for the current relation.
 */
class QgsFilteredSelectionManager : public QgsVectorLayerSelectionManager
{
    Q_OBJECT

  public:
    QgsFilteredSelectionManager( QgsVectorLayer *layer, const QgsFeatureRequest &request, QObject *parent = nullptr );

    const QgsFeatureIds &selectedFeatureIds() const override;
    int selectedFeatureCount() override;

  private slots:

    void onSelectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect ) override;

  private:

    QgsFeatureRequest mRequest;
    QgsFeatureIds mSelectedFeatureIds;
};
#endif
/// @endcond


/**
 * \ingroup gui
 * \class QgsRelationEditorWidget
 */
class GUI_EXPORT QgsRelationEditorWidget : public QgsCollapsibleGroupBox
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsRelationEditorWidget *>( sipCpp ) )
      sipType = sipType_QgsRelationEditorWidget;
    else
      sipType = NULL;
    SIP_END
#endif



    Q_OBJECT
    Q_PROPERTY( QgsDualView::ViewMode viewMode READ viewMode WRITE setViewMode )
    Q_PROPERTY( bool showLabel READ showLabel WRITE setShowLabel )

  public:

    /**
     * \param parent parent widget
     */
    QgsRelationEditorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Define the view mode for the dual view
    void setViewMode( QgsDualView::ViewMode mode );

    //! Gets the view mode for the dual view
    QgsDualView::ViewMode viewMode() {return mViewMode;}

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
     * \since QGIS 3.14
     */
    QgsAttributeEditorContext editorContext( ) const;

    /**
     * The feature selection manager is responsible for the selected features
     * which are currently being edited.
     */
    QgsIFeatureSelectionManager *featureSelectionManager();

    /**
     * Defines if a title label should be shown for this widget.
     *
     * \since QGIS 2.18
     */
    bool showLabel() const;

    /**
     * Defines if a title label should be shown for this widget.
     *
     * \since QGIS 2.18
     */
    void setShowLabel( bool showLabel );

    /**
     * Determines if the "link feature" button should be shown
     *
     * \since QGIS 2.18
     */
    bool showLinkButton() const;

    /**
     * Determines if the "link feature" button should be shown
     *
     * \since QGIS 2.18
     */
    void setShowLinkButton( bool showLinkButton );

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * \since QGIS 2.18
     */
    bool showUnlinkButton() const;

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * \since QGIS 2.18
     */
    void setShowUnlinkButton( bool showUnlinkButton );

    /**
     * Determines if the "Save child layer edits" button should be shown
     *
     * \since QGIS 3.14
     */
    void setShowSaveChildEditsButton( bool showChildEdits );

    /**
     * Determines if the "Save child layer edits" button should be shown
     *
     * \since QGIS 3.14
     */
    bool showSaveChildEditsButton() const;

    /**
     * Returns the widget's current feature
     *
     * \since QGIS 3.14
     */
    QgsFeature feature() const;

  public slots:

    /**
     * Called when an \a attribute value in the parent widget has changed to \a newValue
     *
     * \since QGIS 3.14
     */
    void parentFormValueChanged( const QString &attribute, const QVariant &newValue );

  private slots:
    void setViewMode( int mode ) {setViewMode( static_cast<QgsDualView::ViewMode>( mode ) );}
    void updateButtons();

    void addFeature( const QgsGeometry &geometry = QgsGeometry() );
    void addFeatureGeometry();
    void duplicateFeature();
    void linkFeature();
    void deleteFeature( QgsFeatureId featureid = QgsFeatureId() );
    void deleteSelectedFeatures();
    void unlinkFeature( QgsFeatureId featureid = QgsFeatureId() );
    void unlinkSelectedFeatures();
    void zoomToSelectedFeatures();
    void saveEdits();
    void toggleEditing( bool state );
    void onCollapsedStateChanged( bool collapsed );
    void showContextMenu( QgsActionMenu *menu, QgsFeatureId fid );
    void mapToolDeactivated();
    void onKeyPressed( QKeyEvent *e );
    void onDigitizingCompleted( const QgsFeature &feature );
    void onLinkFeatureDlgAccepted();

  private:
    void updateUi();
    void initDualView( QgsVectorLayer *layer, const QgsFeatureRequest &request );
    void setMapTool( QgsMapTool *mapTool );
    void unsetMapTool();
    void updateTitle();

    QgsDualView *mDualView = nullptr;
    QPointer<QgsMessageBarItem> mMessageBarItem;
    QgsDualView::ViewMode mViewMode = QgsDualView::AttributeEditor;
    QgsVectorLayerSelectionManager *mFeatureSelectionMgr = nullptr;
    QgsAttributeEditorContext mEditorContext;
    QgsRelation mRelation;
    QgsRelation mNmRelation;
    QgsFeature mFeature;

    QToolButton *mToggleEditingButton = nullptr;
    QToolButton *mSaveEditsButton = nullptr;
    QToolButton *mAddFeatureButton = nullptr;
    QToolButton *mDuplicateFeatureButton = nullptr;
    QToolButton *mDeleteFeatureButton = nullptr;
    QToolButton *mLinkFeatureButton = nullptr;
    QToolButton *mUnlinkFeatureButton = nullptr;
    QToolButton *mZoomToFeatureButton = nullptr;
    QToolButton *mFormViewButton = nullptr;
    QToolButton *mTableViewButton = nullptr;
    QToolButton *mAddFeatureGeometryButton = nullptr;
    QGridLayout *mRelationLayout = nullptr;
    QObjectUniquePtr<QgsMapToolDigitizeFeature> mMapToolDigitize;
    QButtonGroup *mViewModeButtonGroup = nullptr;

    bool mShowLabel = true;
    bool mVisible = false;

    /**
     * Deletes the features
     * \param featureids features to delete
     * \since QGIS 3.00
     */
    void deleteFeatures( const QgsFeatureIds &featureids );

    /**
     * Unlinks the features
     * \param featureids features to unlink
     * \since QGIS 3.00
     */
    void unlinkFeatures( const QgsFeatureIds &featureids );
};

#endif // QGSRELATIONEDITOR_H
