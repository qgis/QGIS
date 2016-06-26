/***************************************************************************
    qgsrelationeditor.cpp
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
#include "qgsproject.h"
#include "qgstransactiongroup.h"

#include <QHBoxLayout>
#include <QLabel>

QgsRelationEditorWidget::QgsRelationEditorWidget( QWidget* parent )
    : QgsCollapsibleGroupBox( parent )
    , mViewMode( QgsDualView::AttributeEditor )
    , mVisible( false )
{
  QVBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->setContentsMargins( 0, 9, 0, 0 );
  setLayout( topLayout );

  // buttons
  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->setContentsMargins( 0, 0, 0, 0 );
  // toogle editing
  mToggleEditingButton = new QToolButton( this );
  mToggleEditingButton->setObjectName( "mToggleEditingButton" );
  mToggleEditingButton->setIcon( QgsApplication::getThemeIcon( "/mActionToggleEditing.svg" ) );
  mToggleEditingButton->setText( tr( "Toggle editing" ) );
  mToggleEditingButton->setEnabled( false );
  mToggleEditingButton->setCheckable( true );
  mToggleEditingButton->setToolTip( tr( "Toggle editing mode for child layer" ) );
  buttonLayout->addWidget( mToggleEditingButton );
  // save Edits
  mSaveEditsButton = new QToolButton( this );
  mSaveEditsButton->setIcon( QgsApplication::getThemeIcon( "/mActionSaveEdits.svg" ) );
  mSaveEditsButton->setText( tr( "Save child layer edits" ) );
  mSaveEditsButton->setToolTip( tr( "Save child layer edits" ) );
  mSaveEditsButton->setEnabled( true );
  buttonLayout->addWidget( mSaveEditsButton );
  // add feature
  mAddFeatureButton = new QToolButton( this );
  mAddFeatureButton->setIcon( QgsApplication::getThemeIcon( "/mActionNewTableRow.svg" ) );
  mAddFeatureButton->setText( tr( "Add child feature" ) );
  mAddFeatureButton->setToolTip( tr( "Add child feature" ) );
  mAddFeatureButton->setObjectName( "mAddFeatureButton" );
  buttonLayout->addWidget( mAddFeatureButton );
  // delete feature
  mDeleteFeatureButton = new QToolButton( this );
  mDeleteFeatureButton->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteSelected.svg" ) );
  mDeleteFeatureButton->setText( tr( "Delete child feature" ) );
  mDeleteFeatureButton->setToolTip( tr( "Delete child feature" ) );
  mDeleteFeatureButton->setObjectName( "mDeleteFeatureButton" );
  buttonLayout->addWidget( mDeleteFeatureButton );
  // link feature
  mLinkFeatureButton = new QToolButton( this );
  mLinkFeatureButton->setIcon( QgsApplication::getThemeIcon( "/mActionLink.svg" ) );
  mLinkFeatureButton->setText( tr( "Link existing features" ) );
  mLinkFeatureButton->setToolTip( tr( "Link existing child features" ) );
  mLinkFeatureButton->setObjectName( "mLinkFeatureButton" );
  buttonLayout->addWidget( mLinkFeatureButton );
  // unlink feature
  mUnlinkFeatureButton = new QToolButton( this );
  mUnlinkFeatureButton->setIcon( QgsApplication::getThemeIcon( "/mActionUnlink.svg" ) );
  mUnlinkFeatureButton->setText( tr( "Unlink feature" ) );
  mUnlinkFeatureButton->setToolTip( tr( "Unlink child feature" ) );
  mUnlinkFeatureButton->setObjectName( "mUnlinkFeatureButton" );
  buttonLayout->addWidget( mUnlinkFeatureButton );
  // spacer
  buttonLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ) );
  // form view
  mFormViewButton = new QToolButton( this );
  mFormViewButton->setText( tr( "Form view" ) );
  mFormViewButton->setToolTip( tr( "Switch to form view" ) );
  mFormViewButton->setIcon( QgsApplication::getThemeIcon( "/mActionPropertyItem.png" ) );
  mFormViewButton->setCheckable( true );
  mFormViewButton->setChecked( mViewMode == QgsDualView::AttributeEditor );
  buttonLayout->addWidget( mFormViewButton );
  // table view
  mTableViewButton = new QToolButton( this );
  mTableViewButton->setText( tr( "Table view" ) );
  mTableViewButton->setToolTip( tr( "Switch to table view" ) );
  mTableViewButton->setIcon( QgsApplication::getThemeIcon( "/mActionOpenTable.svg" ) );
  mTableViewButton->setCheckable( true );
  mTableViewButton->setChecked( mViewMode == QgsDualView::AttributeTable );
  buttonLayout->addWidget( mTableViewButton );
  // button group
  mViewModeButtonGroup = new QButtonGroup( this );
  mViewModeButtonGroup->addButton( mFormViewButton, QgsDualView::AttributeEditor );
  mViewModeButtonGroup->addButton( mTableViewButton, QgsDualView::AttributeTable );

  // add buttons layout
  topLayout->addLayout( buttonLayout );

  mRelationLayout = new QGridLayout();
  mRelationLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->addLayout( mRelationLayout );

  mDualView = new QgsDualView( this );
  mDualView->setView( mViewMode );
  mFeatureSelectionMgr = new QgsGenericFeatureSelectionManager( mDualView );
  mDualView->setFeatureSelectionManager( mFeatureSelectionMgr );

  mRelationLayout->addWidget( mDualView );

  connect( this, SIGNAL( collapsedStateChanged( bool ) ), this, SLOT( onCollapsedStateChanged( bool ) ) );
  connect( mViewModeButtonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( setViewMode( int ) ) );
  connect( mToggleEditingButton, SIGNAL( clicked( bool ) ), this, SLOT( toggleEditing( bool ) ) );
  connect( mSaveEditsButton, SIGNAL( clicked() ), this, SLOT( saveEdits() ) );
  connect( mAddFeatureButton, SIGNAL( clicked() ), this, SLOT( addFeature() ) );
  connect( mDeleteFeatureButton, SIGNAL( clicked() ), this, SLOT( deleteFeature() ) );
  connect( mLinkFeatureButton, SIGNAL( clicked() ), this, SLOT( linkFeature() ) );
  connect( mUnlinkFeatureButton, SIGNAL( clicked() ), this, SLOT( unlinkFeature() ) );
  connect( mFeatureSelectionMgr, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SLOT( updateButtons() ) );

  // Set initial state for add/remove etc. buttons
  updateButtons();
}

void QgsRelationEditorWidget::setRelationFeature( const QgsRelation& relation, const QgsFeature& feature )
{
  if ( mRelation.isValid() )
  {
    disconnect( mRelation.referencingLayer(), SIGNAL( editingStarted() ), this, SLOT( updateButtons() ) );
    disconnect( mRelation.referencingLayer(), SIGNAL( editingStopped() ), this, SLOT( updateButtons() ) );
  }

  mRelation = relation;
  mFeature = feature;

  connect( mRelation.referencingLayer(), SIGNAL( editingStarted() ), this, SLOT( updateButtons() ) );
  connect( mRelation.referencingLayer(), SIGNAL( editingStopped() ), this, SLOT( updateButtons() ) );

  setTitle( relation.name() );

  QgsVectorLayer* lyr = relation.referencingLayer();

  bool canChangeAttributes = lyr->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  if ( canChangeAttributes && !lyr->readOnly() )
  {
    mToggleEditingButton->setEnabled( true );
    updateButtons();
  }
  else
  {
    mToggleEditingButton->setEnabled( false );
  }

  setObjectName( mRelation.name() );
  loadState();

  // If not yet initialized, it is not (yet) visible, so we don't load it to be faster (lazy loading)
  // If it is already initialized, it has been set visible before and the currently shown feature is changing
  // and the widget needs updating

  if ( mVisible )
  {
    QgsFeatureRequest myRequest = mRelation.getRelatedFeaturesRequest( mFeature );

    mDualView->init( mRelation.referencingLayer(), nullptr, myRequest, mEditorContext );
  }
}

void QgsRelationEditorWidget::setRelations( const QgsRelation& relation, const QgsRelation& nmrelation )
{
  if ( mRelation.isValid() )
  {
    disconnect( mRelation.referencingLayer(), SIGNAL( editingStarted() ), this, SLOT( updateButtons() ) );
    disconnect( mRelation.referencingLayer(), SIGNAL( editingStopped() ), this, SLOT( updateButtons() ) );
  }

  if ( mNmRelation.isValid() )
  {
    disconnect( mNmRelation.referencedLayer(), SIGNAL( editingStarted() ), this, SLOT( updateButtons() ) );
    disconnect( mNmRelation.referencedLayer(), SIGNAL( editingStopped() ), this, SLOT( updateButtons() ) );
  }

  mRelation = relation;
  mNmRelation = nmrelation;

  if ( !mRelation.isValid() )
    return;

  mToggleEditingButton->setVisible( true );

  Q_FOREACH ( QgsTransactionGroup* tg, QgsProject::instance()->transactionGroups().values() )
  {
    if ( tg->layers().contains( mRelation.referencingLayer() ) )
    {
      mToggleEditingButton->setVisible( false );
      mSaveEditsButton->setVisible( false );
    }
  }

  connect( mRelation.referencingLayer(), SIGNAL( editingStarted() ), this, SLOT( updateButtons() ) );
  connect( mRelation.referencingLayer(), SIGNAL( editingStopped() ), this, SLOT( updateButtons() ) );

  if ( mNmRelation.isValid() )
  {
    connect( mNmRelation.referencingLayer(), SIGNAL( editingStarted() ), this, SLOT( updateButtons() ) );
    connect( mNmRelation.referencingLayer(), SIGNAL( editingStopped() ), this, SLOT( updateButtons() ) );
  }

  setTitle( relation.name() );

  QgsVectorLayer* lyr = relation.referencingLayer();

  bool canChangeAttributes = lyr->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  if ( canChangeAttributes && !lyr->readOnly() )
  {
    mToggleEditingButton->setEnabled( true );
    updateButtons();
  }
  else
  {
    mToggleEditingButton->setEnabled( false );
  }

  setObjectName( mRelation.name() );
  loadState();

  updateUi();
}

void QgsRelationEditorWidget::setEditorContext( const QgsAttributeEditorContext & context )
{
  mEditorContext = context;
}

QgsIFeatureSelectionManager* QgsRelationEditorWidget::featureSelectionManager()
{
  return mFeatureSelectionMgr;
}

void QgsRelationEditorWidget::setViewMode( QgsDualView::ViewMode mode )
{
  mDualView->setView( mode );
  mViewMode = mode;
}


void QgsRelationEditorWidget::setFeature( const QgsFeature& feature )
{
  mFeature = feature;

  updateUi();
}

void QgsRelationEditorWidget::updateButtons()
{
  bool editable = false;
  bool linkable = false;
  bool selectionNotEmpty = mFeatureSelectionMgr->selectedFeatureCount();

  if ( mRelation.isValid() )
  {
    editable = mRelation.referencingLayer()->isEditable();
    linkable = mRelation.referencingLayer()->isEditable();
  }

  if ( mNmRelation.isValid() )
  {
    editable = mNmRelation.referencedLayer()->isEditable();
  }

  mAddFeatureButton->setEnabled( editable );
  mLinkFeatureButton->setEnabled( linkable );
  mDeleteFeatureButton->setEnabled( editable && selectionNotEmpty );
  mUnlinkFeatureButton->setEnabled( linkable && selectionNotEmpty );
  mToggleEditingButton->setChecked( editable );
  mSaveEditsButton->setEnabled( editable );
}

void QgsRelationEditorWidget::addFeature()
{
  QgsAttributeMap keyAttrs;

  const QgsVectorLayerTools* vlTools = mEditorContext.vectorLayerTools();

  if ( mNmRelation.isValid() )
  {
    // n:m Relation: first let the user create a new feature on the other table
    // and autocreate a new linking feature.
    QgsFeature f;
    if ( vlTools->addFeature( mNmRelation.referencedLayer(), QgsAttributeMap(), QgsGeometry(), &f ) )
    {
      QgsFeature flink( mRelation.referencingLayer()->fields() ); // Linking feature

      flink.setAttribute( mRelation.fieldPairs().at( 0 ).first, mFeature.attribute( mRelation.fieldPairs().at( 0 ).second ) );
      flink.setAttribute( mNmRelation.referencingFields().at( 0 ), f.attribute( mNmRelation.referencedFields().at( 0 ) ) );

      mRelation.referencingLayer()->addFeature( flink );

      updateUi();
    }
  }
  else
  {
    QgsFields fields = mRelation.referencingLayer()->fields();

    Q_FOREACH ( const QgsRelation::FieldPair& fieldPair, mRelation.fieldPairs() )
    {
      keyAttrs.insert( fields.indexFromName( fieldPair.referencingField() ), mFeature.attribute( fieldPair.referencedField() ) );
    }

    vlTools->addFeature( mDualView->masterModel()->layer(), keyAttrs );
  }
}

void QgsRelationEditorWidget::linkFeature()
{
  QgsVectorLayer* layer;

  if ( mNmRelation.isValid() )
    layer = mNmRelation.referencedLayer();
  else
    layer = mRelation.referencingLayer();

  QgsFeatureSelectionDlg selectionDlg( layer, mEditorContext , this );

  if ( selectionDlg.exec() )
  {
    if ( mNmRelation.isValid() )
    {
      QgsFeatureIterator it = mNmRelation.referencedLayer()->getFeatures(
                                QgsFeatureRequest()
                                .setFilterFids( selectionDlg.selectedFeatures() )
                                .setSubsetOfAttributes( mNmRelation.referencedFields() ) );

      QgsFeature relatedFeature;

      QgsFeatureList newFeatures;
      QgsFeature linkFeature( mRelation.referencingLayer()->fields() );

      Q_FOREACH ( const QgsRelation::FieldPair& fieldPair, mRelation.fieldPairs() )
      {
        linkFeature.setAttribute( fieldPair.first, mFeature.attribute( fieldPair.second ) );
      }

      while ( it.nextFeature( relatedFeature ) )
      {
        Q_FOREACH ( const QgsRelation::FieldPair& fieldPair, mNmRelation.fieldPairs() )
        {
          linkFeature.setAttribute( fieldPair.first, relatedFeature.attribute( fieldPair.second ) );
        }

        newFeatures << linkFeature;
      }

      mRelation.referencingLayer()->addFeatures( newFeatures );

      updateUi();
    }
    else
    {
      QMap<int, QVariant> keys;
      Q_FOREACH ( const QgsRelation::FieldPair& fieldPair, mRelation.fieldPairs() )
      {
        int idx = mRelation.referencingLayer()->fieldNameIndex( fieldPair.referencingField() );
        QVariant val = mFeature.attribute( fieldPair.referencedField() );
        keys.insert( idx, val );
      }

      Q_FOREACH ( QgsFeatureId fid, selectionDlg.selectedFeatures() )
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
}

void QgsRelationEditorWidget::deleteFeature()
{
  QgsVectorLayer* layer;

  if ( mNmRelation.isValid() )
    // So far we expect the database to take care of cleaning up the linking table or restricting
    // TODO: add more options for the behavior here
    layer = mNmRelation.referencedLayer();
  else
    layer = mRelation.referencingLayer();
  QgsDebugMsg( QString( "Delete %1" ).arg( mFeatureSelectionMgr->selectedFeaturesIds().size() ) );
  layer->deleteFeatures( mFeatureSelectionMgr->selectedFeaturesIds() );
}

void QgsRelationEditorWidget::unlinkFeature()
{
  if ( mNmRelation.isValid() )
  {
    QgsFeatureIterator selectedIterator = mNmRelation.referencedLayer()->getFeatures(
                                            QgsFeatureRequest()
                                            .setFilterFids( mFeatureSelectionMgr->selectedFeaturesIds() )
                                            .setSubsetOfAttributes( mNmRelation.referencedFields() ) );

    QgsFeature f;

    QStringList filters;

    while ( selectedIterator.nextFeature( f ) )
    {
      filters << '(' + mNmRelation.getRelatedFeaturesRequest( f ).filterExpression()->expression() + ')';
    }

    QString filter = QString( "(%1) AND (%2)" ).arg(
                       mRelation.getRelatedFeaturesRequest( mFeature ).filterExpression()->expression(),
                       filters.join( " OR " ) );

    QgsFeatureIterator linkedIterator = mRelation.referencingLayer()->getFeatures( QgsFeatureRequest()
                                        .setSubsetOfAttributes( QgsAttributeList() )
                                        .setFilterExpression( filter ) );

    QgsFeatureIds fids;

    while ( linkedIterator.nextFeature( f ) )
      fids << f.id();

    mRelation.referencingLayer()->deleteFeatures( fids );

    updateUi();
  }
  else
  {
    QMap<int, QgsField> keyFields;
    Q_FOREACH ( const QgsRelation::FieldPair& fieldPair, mRelation.fieldPairs() )
    {
      int idx = mRelation.referencingLayer()->fieldNameIndex( fieldPair.referencingField() );
      if ( idx < 0 )
      {
        QgsDebugMsg( QString( "referencing field %1 not found" ).arg( fieldPair.referencingField() ) );
        return;
      }
      QgsField fld = mRelation.referencingLayer()->fields().at( idx );
      keyFields.insert( idx, fld );
    }

    Q_FOREACH ( QgsFeatureId fid, mFeatureSelectionMgr->selectedFeaturesIds() )
    {
      QMapIterator<int, QgsField> it( keyFields );
      while ( it.hasNext() )
      {
        it.next();
        mRelation.referencingLayer()->changeAttributeValue( fid, it.key(), QVariant( it.value().type() ) );
      }
    }
  }
}

void QgsRelationEditorWidget::toggleEditing( bool state )
{
  if ( state )
  {
    mEditorContext.vectorLayerTools()->startEditing( mRelation.referencingLayer() );
    if ( mNmRelation.isValid() )
      mEditorContext.vectorLayerTools()->startEditing( mNmRelation.referencedLayer() );
  }
  else
  {
    mEditorContext.vectorLayerTools()->stopEditing( mRelation.referencingLayer() );
    if ( mNmRelation.isValid() )
      mEditorContext.vectorLayerTools()->stopEditing( mNmRelation.referencedLayer() );
  }
}

void QgsRelationEditorWidget::saveEdits()
{
  mEditorContext.vectorLayerTools()->saveEdits( mRelation.referencingLayer() );
  if ( mNmRelation.isValid() )
    mEditorContext.vectorLayerTools()->saveEdits( mNmRelation.referencedLayer() );
}

void QgsRelationEditorWidget::onCollapsedStateChanged( bool collapsed )
{

  if ( !collapsed )
  {
    mVisible = true;
    updateUi();
  }
}

void QgsRelationEditorWidget::updateUi()
{
  // If not yet initialized, it is not (yet) visible, so we don't load it to be faster (lazy loading)
  // If it is already initialized, it has been set visible before and the currently shown feature is changing
  // and the widget needs updating

  if ( mVisible )
  {
    QgsFeatureRequest myRequest = mRelation.getRelatedFeaturesRequest( mFeature );

    if ( mNmRelation.isValid() )
    {
      QgsFeatureIterator it = mRelation.referencingLayer()->getFeatures( myRequest );

      QgsFeature fet;

      QStringList filters;

      while ( it.nextFeature( fet ) )
      {
        QString filter = mNmRelation.getReferencedFeatureRequest( fet ).filterExpression()->expression();
        filters << filter.prepend( '(' ).append( ')' );
      }

      QgsFeatureRequest nmRequest;

      nmRequest.setFilterExpression( filters.join( " OR " ) );

      mDualView->init( mNmRelation.referencedLayer(), nullptr, nmRequest, mEditorContext );
    }
    else
    {
      mDualView->init( mRelation.referencingLayer(), nullptr, myRequest, mEditorContext );
    }
  }
}
