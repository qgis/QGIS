/***************************************************************************
    qgsrelationeditor.cpp
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

#include "qgsrelationeditorwidget.h"

#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeatureselectiondlg.h"
#include "qgsgenericfeatureselectionmanager.h"
#include "qgsrelation.h"
#include "qgsvectorlayertools.h"

#include <QHBoxLayout>
#include <QLabel>

QgsRelationEditorWidget::QgsRelationEditorWidget( QWidget* parent )
    : QgsCollapsibleGroupBox( parent )
    , mViewMode( QgsDualView::AttributeEditor )
    , mEditorContext( QgsAttributeEditorContext() )
    , mRelation( QgsRelation() )
    , mFeature( QgsFeature() )
{
  QVBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->setContentsMargins( 0, 9, 0, 0 );
  setLayout( topLayout );

  // buttons
  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->setContentsMargins( 0, 0, 0, 0 );
  // toogle editing
  mToggleEditingButton = new QToolButton( this );
  QAction* toggleEditingAction = new QAction( QgsApplication::getThemeIcon( "/mActionToggleEditing.svg" ), tr( "Toggle editing" ), this );
  mToggleEditingButton->addAction( toggleEditingAction );
  mToggleEditingButton->setDefaultAction( toggleEditingAction );
  buttonLayout->addWidget( mToggleEditingButton );
  // add feature
  mAddFeatureButton = new QToolButton( this );
  QAction* addFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionAdd.svg" ), tr( "Add feature" ), this );
  mAddFeatureButton->addAction( addFeatureAction );
  mAddFeatureButton->setDefaultAction( addFeatureAction );
  buttonLayout->addWidget( mAddFeatureButton );
  // delete feature
  mDeleteFeatureButton = new QToolButton( this );
  QAction* deleteFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionRemove.svg" ), tr( "Delete feature" ), this );
  mDeleteFeatureButton->addAction( deleteFeatureAction );
  mDeleteFeatureButton->setDefaultAction( deleteFeatureAction );
  buttonLayout->addWidget( mDeleteFeatureButton );
  // link feature
  mLinkFeatureButton = new QToolButton( this );
  QAction* linkFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionLink.svg" ), tr( "Link feature" ), this );
  mLinkFeatureButton->addAction( linkFeatureAction );
  mLinkFeatureButton->setDefaultAction( linkFeatureAction );
  buttonLayout->addWidget( mLinkFeatureButton );
  // unlink feature
  mUnlinkFeatureButton = new QToolButton( this );
  QAction* unlinkFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionUnlink.svg" ), tr( "Unlink feature" ), this );
  mUnlinkFeatureButton->addAction( unlinkFeatureAction );
  mUnlinkFeatureButton->setDefaultAction( unlinkFeatureAction );
  buttonLayout->addWidget( mUnlinkFeatureButton );
  // spacer
  buttonLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ) );
  // form view
  mFormViewButton = new QToolButton( this );
  QAction* formViewAction = new QAction( QgsApplication::getThemeIcon( "/mActionPropertyItem.png" ), tr( "Form view" ), this );
  mFormViewButton->addAction( formViewAction );
  mFormViewButton->setDefaultAction( formViewAction );
  mFormViewButton->setCheckable( true );
  mFormViewButton->setChecked( mViewMode == QgsDualView::AttributeEditor );
  buttonLayout->addWidget( mFormViewButton );
  // table view
  mTableViewButton = new QToolButton( this );
  QAction* tableViewAction = new QAction( QgsApplication::getThemeIcon( "/mActionOpenTable.png" ), tr( "Table view" ), this );
  mTableViewButton->addAction( tableViewAction );
  mTableViewButton->setDefaultAction( tableViewAction );
  mTableViewButton->setCheckable( true );
  mTableViewButton->setChecked( mViewMode == QgsDualView::AttributeTable );
  buttonLayout->addWidget( mTableViewButton );
  // button group
  mViewModeButtonGroup = new QButtonGroup( this );
  mViewModeButtonGroup->addButton( mFormViewButton );
  mViewModeButtonGroup->addButton( mTableViewButton );
  mViewModeButtonGroup->setId( mTableViewButton, QgsDualView::AttributeTable );
  mViewModeButtonGroup->setId( mFormViewButton, QgsDualView::AttributeEditor );

  // add buttons layout
  topLayout->addLayout( buttonLayout );

  // Set initial state for add/remove etc. buttons
  referencingLayerEditingToggled();

  mRelationLayout = new QGridLayout();
  mRelationLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->addLayout( mRelationLayout );

  mDualView = new QgsDualView( this );
  mDualView->setView( mViewMode );
  mFeatureSelectionMgr = new QgsGenericFeatureSelectionManager( mDualView );
  mRelationLayout->addWidget( mDualView );

  connect( this, SIGNAL( collapsedStateChanged( bool ) ), this, SLOT( onCollapsedStateChanged( bool ) ) );
  connect( mViewModeButtonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( setViewMode( int ) ) );
}

void QgsRelationEditorWidget::setRelationFeature( const QgsRelation& relation, const QgsFeature& feature, const QgsAttributeEditorContext& context )
{
  if ( mRelation.isValid() )
  {
    disconnect( mRelation.referencingLayer(), SIGNAL( editingStarted() ), this, SLOT( referencingLayerEditingToggled() ) );
    disconnect( mRelation.referencingLayer(), SIGNAL( editingStopped() ), this, SLOT( referencingLayerEditingToggled() ) );
  }

  mRelation = relation;
  mFeature = feature;
  mEditorContext = context;

  connect( mRelation.referencingLayer(), SIGNAL( editingStarted() ), this, SLOT( referencingLayerEditingToggled() ) );
  connect( mRelation.referencingLayer(), SIGNAL( editingStopped() ), this, SLOT( referencingLayerEditingToggled() ) );

  setTitle( relation.name() );

  QgsVectorLayer* lyr = relation.referencingLayer();

  bool canChangeAttributes = lyr->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  mToggleEditingButton->setEnabled( canChangeAttributes && !lyr->isReadOnly() );


  mDualView->setFeatureSelectionManager( mFeatureSelectionMgr );

  QgsFeatureRequest myRequest = relation.getRelatedFeaturesRequest( feature );

  mDualView->init( relation.referencingLayer(), NULL, myRequest, context );
}

void QgsRelationEditorWidget::setViewMode( QgsDualView::ViewMode mode )
{
  mDualView->setView( mode );
  mViewMode = mode;
  mTableViewButton->setChecked( mViewMode == QgsDualView::AttributeTable );
  mFormViewButton->setChecked( mViewMode == QgsDualView::AttributeEditor );
}

void QgsRelationEditorWidget::onCollapsedStateChanged( bool state )
{
  if ( state && !mDualView->masterModel() )
  {
    // TODO: Lazy init dual view if collapsed on init
  }
}

void QgsRelationEditorWidget::referencingLayerEditingToggled()
{
  bool editable = false;
  if ( mRelation.isValid() )
  {
    editable = mRelation.referencingLayer()->isEditable();
  }

  mAddFeatureButton->setEnabled( editable );
  mLinkFeatureButton->setEnabled( editable );
  mDeleteFeatureButton->setEnabled( editable );
  mUnlinkFeatureButton->setEnabled( editable );
  mToggleEditingButton->setChecked( editable );
}

void QgsRelationEditorWidget::on_mAddFeatureButton_clicked()
{
  QgsAttributeMap keyAttrs;

  QgsFields fields = mRelation.referencingLayer()->pendingFields();

  foreach ( QgsRelation::FieldPair fieldPair, mRelation.fieldPairs() )
  {
    keyAttrs.insert( fields.indexFromName( fieldPair.referencingField() ), mFeature.attribute( fieldPair.referencedField() ) );
  }

  mEditorContext.vectorLayerTools()->addFeature( mDualView->masterModel()->layer(), keyAttrs );
}

void QgsRelationEditorWidget::on_mLinkFeatureButton_clicked()
{
  QgsFeatureSelectionDlg selectionDlg( mRelation.referencingLayer(), this );

  if ( selectionDlg.exec() )
  {
    QMap<int, QVariant> keys;
    foreach ( const QgsRelation::FieldPair fieldPair, mRelation.fieldPairs() )
    {
      int idx = mRelation.referencingLayer()->fieldNameIndex( fieldPair.referencingField() );
      QVariant val = mFeature.attribute( fieldPair.referencedField() );
      keys.insert( idx, val );
    }

    foreach ( QgsFeatureId fid, selectionDlg.selectedFeatures() )
    {
      QMapIterator<int, QVariant> it( keys );
      while ( it.hasNext() )
      {
        it.next();
        mRelation.referencingLayer()->changeAttributeValue( fid, it.key(), it.value() );
      }
    }
  }
}

void QgsRelationEditorWidget::on_mDeleteFeatureButton_clicked()
{
  foreach ( QgsFeatureId fid, mFeatureSelectionMgr->selectedFeaturesIds() )
  {
    mRelation.referencingLayer()->deleteFeature( fid );
  }
}

void QgsRelationEditorWidget::on_mUnlinkFeatureButton_clicked()
{
  QMap<int, QgsField> keyFields;
  foreach ( const QgsRelation::FieldPair fieldPair, mRelation.fieldPairs() )
  {
    int idx = mRelation.referencingLayer()->fieldNameIndex( fieldPair.referencingField() );
    QgsField fld = mRelation.referencingLayer()->pendingFields().at( idx );
    keyFields.insert( idx, fld );
  }

  foreach ( QgsFeatureId fid, mFeatureSelectionMgr->selectedFeaturesIds() )
  {
    QMapIterator<int, QgsField> it( keyFields );
    while ( it.hasNext() )
    {
      it.next();
      mRelation.referencingLayer()->changeAttributeValue( fid, it.key(), QVariant( it.value().type() ) );
    }
  }
}

void QgsRelationEditorWidget::on_mToggleEditingButton_toggled( bool state )
{
  if ( state )
  {
    mEditorContext.vectorLayerTools()->startEditing( mRelation.referencingLayer() );
  }
  else
  {
    mEditorContext.vectorLayerTools()->stopEditing( mRelation.referencingLayer() );
  }
}
