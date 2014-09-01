/***************************************************************************
    qgsrelationeditor.h
     --------------------------------------
    Date                 : 17.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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

#include "qgsattributeeditorcontext.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsdualview.h"
#include "qgsrelation.h"

class QgsFeature;
class QgsGenericFeatureSelectionManager;
class QgsVectorLayer;
class QgsVectorLayerTools;

class GUI_EXPORT QgsRelationEditorWidget : public QgsCollapsibleGroupBox
{
    Q_OBJECT
    Q_PROPERTY( QString qgisRelation READ qgisRelation WRITE setQgisRelation )
    Q_PROPERTY( QgsDualView::ViewMode viewMode READ viewMode WRITE setViewMode )

  public:
    /**
     * @param parent parent widget
     */
    QgsRelationEditorWidget( QWidget* parent = NULL );

    //! Define the view mode for the dual view
    void setViewMode( QgsDualView::ViewMode mode );
    QgsDualView::ViewMode viewMode() {return mViewMode;}

    //! Defines the relation ID (from project relations)
    //! @note use a widget's property to keep compatibility with using basic widget instead of QgsRelationEditorWidget
    void setQgisRelation( QString qgisRelationId ) { mRelationId = qgisRelationId; }
    QString qgisRelation() { return mRelationId; }  //property( "qgisRelation" ).toString()

    void setRelationFeature( const QgsRelation& relation, const QgsFeature& feature );

    void setEditorContext( const QgsAttributeEditorContext& context );

  private slots:
    void setViewMode( int mode ) {setViewMode( static_cast<QgsDualView::ViewMode>( mode ) );}
    void referencingLayerEditingToggled();

    void addFeature();
    void linkFeature();
    void deleteFeature();
    void unlinkFeature();
    void toggleEditing( bool state );
    void onCollapsedStateChanged( bool collapsed );

  private:
    QgsDualView* mDualView;
    QgsDualView::ViewMode mViewMode;
    QgsGenericFeatureSelectionManager* mFeatureSelectionMgr;
    QgsAttributeEditorContext mEditorContext;
    QgsRelation mRelation;
    QString mRelationId;
    QgsFeature mFeature;

    QToolButton* mToggleEditingButton;
    QToolButton* mAddFeatureButton;
    QToolButton* mDeleteFeatureButton;
    QToolButton* mLinkFeatureButton;
    QToolButton* mUnlinkFeatureButton;
    QToolButton* mFormViewButton;
    QToolButton* mTableViewButton;
    QGridLayout* mRelationLayout;
    QButtonGroup* mViewModeButtonGroup;

    bool mInitialized;
};

#endif // QGSRELATIONEDITOR_H
