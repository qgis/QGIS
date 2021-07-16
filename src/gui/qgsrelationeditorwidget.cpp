/***************************************************************************
    qgsrelationeditorwidget.cpp
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
#include "qgsfeatureiterator.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeatureselectiondlg.h"
#include "qgsgenericfeatureselectionmanager.h"
#include "qgsrelation.h"
#include "qgsvectorlayertools.h"
#include "qgsproject.h"
#include "qgstransactiongroup.h"
#include "qgslogger.h"
#include "qgsvectorlayerutils.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayerselectionmanager.h"
#include "qgsmaptooldigitizefeature.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgscollapsiblegroupbox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

/// @cond PRIVATE
///
QgsFilteredSelectionManager::QgsFilteredSelectionManager( QgsVectorLayer *layer, const QgsFeatureRequest &request, QObject *parent )
  : QgsVectorLayerSelectionManager( layer, parent )
  , mRequest( request )
{
  if ( ! layer )
    return;

  for ( auto fid : layer->selectedFeatureIds() )
    if ( mRequest.acceptFeature( layer->getFeature( fid ) ) )
      mSelectedFeatureIds << fid;

  connect( layer, &QgsVectorLayer::selectionChanged, this, &QgsFilteredSelectionManager::onSelectionChanged );
}

const QgsFeatureIds &QgsFilteredSelectionManager::selectedFeatureIds() const


{
  return mSelectedFeatureIds;
}

int QgsFilteredSelectionManager::selectedFeatureCount()
{
  return mSelectedFeatureIds.count();
}

void QgsFilteredSelectionManager::onSelectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect )
{
  QgsFeatureIds lselected = selected;
  if ( clearAndSelect )
  {
    mSelectedFeatureIds.clear();
  }
  else
  {
    for ( auto fid : deselected )
      mSelectedFeatureIds.remove( fid );
  }

  for ( auto fid : selected )
    if ( mRequest.acceptFeature( layer()->getFeature( fid ) ) )
      mSelectedFeatureIds << fid;
    else
      lselected.remove( fid );

  emit selectionChanged( lselected, deselected, clearAndSelect );
}

/// @endcond

QgsRelationEditorWidget::QgsRelationEditorWidget( const QVariantMap &config, QWidget *parent )
  : QgsAbstractRelationEditorWidget( config, parent )
  , mButtonsVisibility( qgsFlagKeysToValue( config.value( QStringLiteral( "buttons" ) ).toString(), QgsRelationEditorWidget::Button::AllButtons ) )
{
  QVBoxLayout *rootLayout = new QVBoxLayout( this );
  rootLayout->setContentsMargins( 0, 9, 0, 0 );

  // buttons
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setContentsMargins( 0, 0, 0, 0 );
  // toggle editing
  mToggleEditingButton = new QToolButton( this );
  mToggleEditingButton->setObjectName( QStringLiteral( "mToggleEditingButton" ) );
  mToggleEditingButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
  mToggleEditingButton->setText( tr( "Toggle Editing" ) );
  mToggleEditingButton->setEnabled( false );
  mToggleEditingButton->setCheckable( true );
  mToggleEditingButton->setToolTip( tr( "Toggle editing mode for child layer" ) );
  buttonLayout->addWidget( mToggleEditingButton );
  // save Edits
  mSaveEditsButton = new QToolButton( this );
  mSaveEditsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveEdits.svg" ) ) );
  mSaveEditsButton->setText( tr( "Save Child Layer Edits" ) );
  mSaveEditsButton->setToolTip( tr( "Save child layer edits" ) );
  mSaveEditsButton->setEnabled( true );
  buttonLayout->addWidget( mSaveEditsButton );
  // add feature with geometry
  mAddFeatureGeometryButton = new QToolButton( this );
  mAddFeatureGeometryButton->setObjectName( QStringLiteral( "mAddFeatureGeometryButton" ) );
  buttonLayout->addWidget( mAddFeatureGeometryButton );
  // add feature
  mAddFeatureButton = new QToolButton( this );
  mAddFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewTableRow.svg" ) ) );
  mAddFeatureButton->setText( tr( "Add Child Feature" ) );
  mAddFeatureButton->setToolTip( tr( "Add child feature" ) );
  mAddFeatureButton->setObjectName( QStringLiteral( "mAddFeatureButton" ) );
  buttonLayout->addWidget( mAddFeatureButton );
  // duplicate feature
  mDuplicateFeatureButton = new QToolButton( this );
  mDuplicateFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDuplicateFeature.svg" ) ) );
  mDuplicateFeatureButton->setText( tr( "Duplicate Child Feature" ) );
  mDuplicateFeatureButton->setToolTip( tr( "Duplicate selected child feature" ) );
  mDuplicateFeatureButton->setObjectName( QStringLiteral( "mDuplicateFeatureButton" ) );
  buttonLayout->addWidget( mDuplicateFeatureButton );
  // delete feature
  mDeleteFeatureButton = new QToolButton( this );
  mDeleteFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ) );
  mDeleteFeatureButton->setText( tr( "Delete Child Feature" ) );
  mDeleteFeatureButton->setToolTip( tr( "Delete selected child feature" ) );
  mDeleteFeatureButton->setObjectName( QStringLiteral( "mDeleteFeatureButton" ) );
  buttonLayout->addWidget( mDeleteFeatureButton );
  // link feature
  mLinkFeatureButton = new QToolButton( this );
  mLinkFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLink.svg" ) ) );
  mLinkFeatureButton->setText( tr( "Link Existing Features" ) );
  mLinkFeatureButton->setToolTip( tr( "Link existing child features" ) );
  mLinkFeatureButton->setObjectName( QStringLiteral( "mLinkFeatureButton" ) );
  buttonLayout->addWidget( mLinkFeatureButton );
  // unlink feature
  mUnlinkFeatureButton = new QToolButton( this );
  mUnlinkFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUnlink.svg" ) ) );
  mUnlinkFeatureButton->setText( tr( "Unlink Feature" ) );
  mUnlinkFeatureButton->setToolTip( tr( "Unlink selected child feature" ) );
  mUnlinkFeatureButton->setObjectName( QStringLiteral( "mUnlinkFeatureButton" ) );
  buttonLayout->addWidget( mUnlinkFeatureButton );
  // zoom to linked feature
  mZoomToFeatureButton = new QToolButton( this );
  mZoomToFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToSelected.svg" ) ) );
  mZoomToFeatureButton->setText( tr( "Zoom To Feature" ) );
  mZoomToFeatureButton->setToolTip( tr( "Zoom to selected child feature" ) );
  mZoomToFeatureButton->setObjectName( QStringLiteral( "mZoomToFeatureButton" ) );
  buttonLayout->addWidget( mZoomToFeatureButton );
  // spacer
  buttonLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ) );
  // form view
  mFormViewButton = new QToolButton( this );
  mFormViewButton->setText( tr( "Form View" ) );
  mFormViewButton->setToolTip( tr( "Switch to form view" ) );
  mFormViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPropertyItem.svg" ) ) );
  mFormViewButton->setCheckable( true );
  mFormViewButton->setChecked( mViewMode == QgsDualView::AttributeEditor );
  buttonLayout->addWidget( mFormViewButton );
  // table view
  mTableViewButton = new QToolButton( this );
  mTableViewButton->setText( tr( "Table View" ) );
  mTableViewButton->setToolTip( tr( "Switch to table view" ) );
  mTableViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ) );
  mTableViewButton->setCheckable( true );
  mTableViewButton->setChecked( mViewMode == QgsDualView::AttributeTable );
  buttonLayout->addWidget( mTableViewButton );
  // button group
  mViewModeButtonGroup = new QButtonGroup( this );
  mViewModeButtonGroup->addButton( mFormViewButton, QgsDualView::AttributeEditor );
  mViewModeButtonGroup->addButton( mTableViewButton, QgsDualView::AttributeTable );

  // add buttons layout
  rootLayout->addLayout( buttonLayout );

  // add dual view
  QGridLayout *relationLayout = new QGridLayout();
  relationLayout->setContentsMargins( 0, 0, 0, 0 );
  mDualView = new QgsDualView( this );
  mDualView->setView( mViewMode );
  connect( mDualView, &QgsDualView::showContextMenuExternally, this, &QgsRelationEditorWidget::showContextMenu );
  relationLayout->addWidget( mDualView );
  rootLayout->addLayout( relationLayout );

  connect( mViewModeButtonGroup, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ),
           this, static_cast<void ( QgsRelationEditorWidget::* )( int )>( &QgsRelationEditorWidget::setViewMode ) );
  connect( mToggleEditingButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::toggleEditing );
  connect( mSaveEditsButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::saveEdits );
  connect( mAddFeatureButton, &QAbstractButton::clicked, this, [this]() { addFeature(); } );
  connect( mAddFeatureGeometryButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::addFeatureGeometry );
  connect( mDuplicateFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::duplicateSelectedFeatures );
  connect( mDeleteFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::deleteSelectedFeatures );
  connect( mLinkFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::linkFeature );
  connect( mUnlinkFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::unlinkSelectedFeatures );
  connect( mZoomToFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::zoomToSelectedFeatures );

  // Set initial state for add/remove etc. buttons
  updateButtons();
}

void QgsRelationEditorWidget::initDualView( QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  QgsAttributeEditorContext ctx { mEditorContext };
  ctx.setParentFormFeature( mFeature );
  mDualView->init( layer, mEditorContext.mapCanvas(), request, ctx );
  mFeatureSelectionMgr = new QgsFilteredSelectionManager( layer, request, mDualView );
  mDualView->setFeatureSelectionManager( mFeatureSelectionMgr );

  connect( mFeatureSelectionMgr, &QgsIFeatureSelectionManager::selectionChanged, this, &QgsRelationEditorWidget::updateButtons );

  QIcon icon;
  QString text;
  if ( layer->geometryType() == QgsWkbTypes::PointGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePoint.svg" ) );
    text = tr( "Add Point child Feature" );
  }
  else if ( layer->geometryType() == QgsWkbTypes::LineGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCaptureLine.svg" ) );
    text = tr( "Add Line child Feature" );
  }
  else if ( layer->geometryType() == QgsWkbTypes::PolygonGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePolygon.svg" ) );
    text = tr( "Add Polygon Feature" );
  }

  mAddFeatureGeometryButton->setIcon( icon );
  mAddFeatureGeometryButton->setText( text );
  mAddFeatureGeometryButton->setToolTip( text );

  updateButtons();
}

void QgsRelationEditorWidget::setEditorContext( const QgsAttributeEditorContext &context )
{
  mEditorContext = context;

  if ( context.mapCanvas() && context.cadDockWidget() )
  {
    mMapToolDigitize.reset( new QgsMapToolDigitizeFeature( context.mapCanvas(), context.cadDockWidget() ) );
    mMapToolDigitize->setButton( mAddFeatureGeometryButton );
  }

  updateButtons();
}

void QgsRelationEditorWidget::setViewMode( QgsDualView::ViewMode mode )
{
  mDualView->setView( mode );
  mViewMode = mode;
}

void QgsRelationEditorWidget::updateButtons()
{
  bool toggleEditingButtonEnabled = false;
  bool editable = false;
  bool linkable = false;
  bool spatial = false;
  bool selectionNotEmpty = mFeatureSelectionMgr ? mFeatureSelectionMgr->selectedFeatureCount() : false;

  if ( mRelation.isValid() )
  {
    toggleEditingButtonEnabled = mRelation.referencingLayer()->supportsEditing();
    editable = mRelation.referencingLayer()->isEditable();
    linkable = mRelation.referencingLayer()->isEditable();
    spatial = mRelation.referencingLayer()->isSpatial();
  }

  if ( mNmRelation.isValid() )
  {
    toggleEditingButtonEnabled |= mNmRelation.referencedLayer()->supportsEditing();
    editable = mNmRelation.referencedLayer()->isEditable();
    spatial = mNmRelation.referencedLayer()->isSpatial();
  }

  mToggleEditingButton->setEnabled( toggleEditingButtonEnabled );
  mAddFeatureButton->setEnabled( editable );
  mAddFeatureGeometryButton->setEnabled( editable );
  mDuplicateFeatureButton->setEnabled( editable && selectionNotEmpty );
  mLinkFeatureButton->setEnabled( linkable );
  mDeleteFeatureButton->setEnabled( editable && selectionNotEmpty );
  mUnlinkFeatureButton->setEnabled( linkable && selectionNotEmpty );
  mZoomToFeatureButton->setEnabled( selectionNotEmpty );
  mToggleEditingButton->setChecked( editable );
  mSaveEditsButton->setEnabled( editable || linkable );

  mToggleEditingButton->setVisible( !mLayerInSameTransactionGroup );

  mLinkFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsRelationEditorWidget::Button::Link ) );
  mUnlinkFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsRelationEditorWidget::Button::Unlink ) );
  mSaveEditsButton->setVisible( mButtonsVisibility.testFlag( QgsRelationEditorWidget::Button::SaveChildEdits ) && !mLayerInSameTransactionGroup );
  mAddFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsRelationEditorWidget::Button::AddChildFeature ) );
  mAddFeatureGeometryButton->setVisible( mButtonsVisibility.testFlag( QgsRelationEditorWidget::Button::AddChildFeature ) && mEditorContext.mapCanvas() && mEditorContext.cadDockWidget() && spatial );
  mDuplicateFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsRelationEditorWidget::Button::DuplicateChildFeature ) );
  mDeleteFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsRelationEditorWidget::Button::DeleteChildFeature ) );
  mZoomToFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsRelationEditorWidget::Button::ZoomToChildFeature ) && mEditorContext.mapCanvas() && spatial );
}

void QgsRelationEditorWidget::addFeatureGeometry()
{
  QgsVectorLayer *layer = nullptr;
  if ( mNmRelation.isValid() )
    layer = mNmRelation.referencedLayer();
  else
    layer = mRelation.referencingLayer();

  mMapToolDigitize->setLayer( layer );

  // window is always on top, so we hide it to digitize without seeing it
  window()->setVisible( false );
  setMapTool( mMapToolDigitize );

  connect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsRelationEditorWidget::onDigitizingCompleted );
  connect( mEditorContext.mapCanvas(), &QgsMapCanvas::keyPressed, this, &QgsRelationEditorWidget::onKeyPressed );

  if ( auto *lMainMessageBar = mEditorContext.mainMessageBar() )
  {
    QString displayString = QgsVectorLayerUtils::getFeatureDisplayString( layer, mFeature );

    QString title = tr( "Create child feature for parent %1 \"%2\"" ).arg( mRelation.referencedLayer()->name(), displayString );
    QString msg = tr( "Digitize the geometry for the new feature on layer %1. Press &lt;ESC&gt; to cancel." )
                  .arg( layer->name() );
    mMessageBarItem = QgsMessageBar::createMessage( title, msg, this );
    lMainMessageBar->pushItem( mMessageBarItem );
  }

}

void QgsRelationEditorWidget::onDigitizingCompleted( const QgsFeature &feature )
{
  addFeature( feature.geometry() );

  unsetMapTool();
}

void QgsRelationEditorWidget::toggleEditing( bool state )
{
  QgsAbstractRelationEditorWidget::toggleEditing( state );

  updateButtons();
}

void QgsRelationEditorWidget::updateUi()
{
  if ( !mRelation.isValid() || !mFeature.isValid() )
    return;

  if ( !isVisible() )
    return;

  QgsFeatureRequest request = mRelation.getRelatedFeaturesRequest( mFeature );

  if ( mNmRelation.isValid() )
  {
    QgsFeatureIterator it = mRelation.referencingLayer()->getFeatures( request );
    QgsFeature fet;
    QStringList filters;

    while ( it.nextFeature( fet ) )
    {
      QString filter = mNmRelation.getReferencedFeatureRequest( fet ).filterExpression()->expression();
      filters << filter.prepend( '(' ).append( ')' );
    }

    QgsFeatureRequest nmRequest;
    nmRequest.setFilterExpression( filters.join( QLatin1String( " OR " ) ) );

    initDualView( mNmRelation.referencedLayer(), nmRequest );
  }
  else if ( mRelation.referencingLayer() )
  {
    initDualView( mRelation.referencingLayer(), request );
  }
}

void QgsRelationEditorWidget::setVisibleButtons( const Buttons &buttons )
{
  mButtonsVisibility = buttons;
  updateButtons();
}

QgsRelationEditorWidget::Buttons QgsRelationEditorWidget::visibleButtons() const
{
  Buttons buttons;
  if ( mLinkFeatureButton->isVisible() )
    buttons |= Button::Link;
  if ( mUnlinkFeatureButton->isVisible() )
    buttons |= Button::Unlink;
  if ( mSaveEditsButton->isVisible() )
    buttons |= Button::SaveChildEdits;
  if ( mAddFeatureButton->isVisible() )
    buttons |= Button::AddChildFeature;
  if ( mDuplicateFeatureButton->isVisible() )
    buttons |= Button::DuplicateChildFeature;
  if ( mDeleteFeatureButton->isVisible() )
    buttons |= Button::DeleteChildFeature;
  if ( mZoomToFeatureButton->isVisible() )
    buttons |= Button::ZoomToChildFeature;
  return buttons;
}

void QgsRelationEditorWidget::parentFormValueChanged( const QString &attribute, const QVariant &newValue )
{
  mDualView->parentFormValueChanged( attribute, newValue );
}

void QgsRelationEditorWidget::showContextMenu( QgsActionMenu *menu, const QgsFeatureId fid )
{
  if ( mRelation.referencingLayer()->isEditable() )
  {
    QAction *qAction = nullptr;

    qAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ),  tr( "Delete Feature" ) );
    connect( qAction, &QAction::triggered, this, [this, fid]() { deleteFeature( fid ); } );

    qAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUnlink.svg" ) ),  tr( "Unlink Feature" ) );
    connect( qAction, &QAction::triggered, this, [this, fid]() { unlinkFeature( fid ); } );
  }
}

void QgsRelationEditorWidget::setMapTool( QgsMapTool *mapTool )
{
  QgsMapCanvas *mapCanvas = mEditorContext.mapCanvas();

  mapCanvas->setMapTool( mapTool );
  mapCanvas->window()->raise();
  mapCanvas->activateWindow();
  mapCanvas->setFocus();
  connect( mapTool, &QgsMapTool::deactivated, this, &QgsRelationEditorWidget::mapToolDeactivated );
}

void QgsRelationEditorWidget::unsetMapTool()
{
  QgsMapCanvas *mapCanvas = mEditorContext.mapCanvas();

  // this will call mapToolDeactivated
  mapCanvas->unsetMapTool( mMapToolDigitize );

  disconnect( mapCanvas, &QgsMapCanvas::keyPressed, this, &QgsRelationEditorWidget::onKeyPressed );
  disconnect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsRelationEditorWidget::onDigitizingCompleted );
}

void QgsRelationEditorWidget::onKeyPressed( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    unsetMapTool();
  }
}

void QgsRelationEditorWidget::mapToolDeactivated()
{
  window()->setVisible( true );
  window()->raise();
  window()->activateWindow();

  if ( mEditorContext.mainMessageBar() && mMessageBarItem )
  {
    mEditorContext.mainMessageBar()->popWidget( mMessageBarItem );
  }
  mMessageBarItem = nullptr;
}

QVariantMap QgsRelationEditorWidget::config() const
{
  return QVariantMap( {{"buttons", qgsFlagValueToKeys( visibleButtons() )}} );
}

void QgsRelationEditorWidget::setConfig( const QVariantMap &config )
{
  mButtonsVisibility = qgsFlagKeysToValue( config.value( QStringLiteral( "buttons" ) ).toString(), QgsRelationEditorWidget::Button::AllButtons );
  updateButtons();
}

void QgsRelationEditorWidget::beforeSetRelationFeature( const QgsRelation &newRelation, const QgsFeature &newFeature )
{
  Q_UNUSED( newRelation );
  Q_UNUSED( newFeature );

  if ( ! mRelation.isValid() )
    return;

  disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
  disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );
}

void QgsRelationEditorWidget::afterSetRelationFeature()
{
  if ( ! mRelation.isValid() )
  {
    updateButtons();
    return;
  }

  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );

  updateButtons();

  QgsFeatureRequest myRequest = mRelation.getRelatedFeaturesRequest( mFeature );
  initDualView( mRelation.referencingLayer(), myRequest );
}

void QgsRelationEditorWidget::beforeSetRelations( const QgsRelation &newRelation, const QgsRelation &newNmRelation )
{
  Q_UNUSED( newRelation );
  Q_UNUSED( newNmRelation );

  if ( mRelation.isValid() )
  {
    disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
    disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );
  }

  if ( mNmRelation.isValid() )
  {
    disconnect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
    disconnect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );
  }
}

void QgsRelationEditorWidget::afterSetRelations()
{
  if ( !mRelation.isValid() )
    return;

  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );

  if ( mNmRelation.isValid() )
  {
    connect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
    connect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );
  }

  updateButtons();
}

QgsIFeatureSelectionManager *QgsRelationEditorWidget::featureSelectionManager()
{
  return mFeatureSelectionMgr;
}

void QgsRelationEditorWidget::unlinkSelectedFeatures()
{
  unlinkFeatures( mFeatureSelectionMgr->selectedFeatureIds() );
}

void QgsRelationEditorWidget::duplicateFeature()
{
  duplicateFeatures( mFeatureSelectionMgr->selectedFeatureIds() );
}

void QgsRelationEditorWidget::duplicateSelectedFeatures()
{
  duplicateFeatures( mFeatureSelectionMgr->selectedFeatureIds() );
}

void QgsRelationEditorWidget::deleteSelectedFeatures()
{
  QgsFeatureIds selectedFids = mFeatureSelectionMgr->selectedFeatureIds();
  deleteFeatures( selectedFids );
}

void QgsRelationEditorWidget::zoomToSelectedFeatures()
{
  QgsMapCanvas *c = mEditorContext.mapCanvas();
  if ( !c )
    return;

  c->zoomToFeatureIds(
    mNmRelation.isValid() ? mNmRelation.referencedLayer() : mRelation.referencingLayer(),
    mFeatureSelectionMgr->selectedFeatureIds()
  );
}

///////////////////////////////////////////////////////////////////////////////


QgsRelationEditorConfigWidget::QgsRelationEditorConfigWidget( const QgsRelation &relation, QWidget *parent )
  : QgsAbstractRelationEditorConfigWidget( relation, parent )
{
  setupUi( this );
}

QVariantMap QgsRelationEditorConfigWidget::config()
{
  QgsRelationEditorWidget::Buttons buttons;
  buttons.setFlag( QgsRelationEditorWidget::Button::Link, mRelationShowLinkCheckBox->isChecked() );
  buttons.setFlag( QgsRelationEditorWidget::Button::Unlink, mRelationShowUnlinkCheckBox->isChecked() );
  buttons.setFlag( QgsRelationEditorWidget::Button::AddChildFeature, mRelationShowAddChildCheckBox->isChecked() );
  buttons.setFlag( QgsRelationEditorWidget::Button::DuplicateChildFeature, mRelationShowDuplicateChildFeatureCheckBox->isChecked() );
  buttons.setFlag( QgsRelationEditorWidget::Button::ZoomToChildFeature, mRelationShowZoomToFeatureCheckBox->isChecked() );
  buttons.setFlag( QgsRelationEditorWidget::Button::DeleteChildFeature, mRelationDeleteChildFeatureCheckBox->isChecked() );
  buttons.setFlag( QgsRelationEditorWidget::Button::SaveChildEdits, mRelationShowSaveChildEditsCheckBox->isChecked() );

  return QVariantMap(
  {
    {"buttons", qgsFlagValueToKeys( buttons )},
  } );
}

void QgsRelationEditorConfigWidget::setConfig( const QVariantMap &config )
{
  const QgsRelationEditorWidget::Buttons buttons = qgsFlagKeysToValue( config.value( QStringLiteral( "buttons" ) ).toString(), QgsRelationEditorWidget::Button::AllButtons );

  mRelationShowLinkCheckBox->setChecked( buttons.testFlag( QgsRelationEditorWidget::Button::Link ) );
  mRelationShowUnlinkCheckBox->setChecked( buttons.testFlag( QgsRelationEditorWidget::Button::Unlink ) );
  mRelationShowAddChildCheckBox->setChecked( buttons.testFlag( QgsRelationEditorWidget::Button::AddChildFeature ) );
  mRelationShowDuplicateChildFeatureCheckBox->setChecked( buttons.testFlag( QgsRelationEditorWidget::Button::DuplicateChildFeature ) );
  mRelationShowZoomToFeatureCheckBox->setChecked( buttons.testFlag( QgsRelationEditorWidget::Button::ZoomToChildFeature ) );
  mRelationDeleteChildFeatureCheckBox->setChecked( buttons.testFlag( QgsRelationEditorWidget::Button::DeleteChildFeature ) );
  mRelationShowSaveChildEditsCheckBox->setChecked( buttons.testFlag( QgsRelationEditorWidget::Button::SaveChildEdits ) );
}


///////////////////////////////////////////////////////////////////////////////


#ifndef SIP_RUN
QgsRelationEditorWidgetFactory::QgsRelationEditorWidgetFactory()
{

}

QString QgsRelationEditorWidgetFactory::type() const
{
  return QStringLiteral( "relation_editor" );
}

QString QgsRelationEditorWidgetFactory::name() const
{
  return QStringLiteral( "Relation Editor" );
}

QgsAbstractRelationEditorWidget *QgsRelationEditorWidgetFactory::create( const QVariantMap &config, QWidget *parent ) const
{
  return new QgsRelationEditorWidget( config, parent );
}

QgsAbstractRelationEditorConfigWidget *QgsRelationEditorWidgetFactory::configWidget( const QgsRelation &relation, QWidget *parent ) const
{
  return static_cast<QgsAbstractRelationEditorConfigWidget *>( new QgsRelationEditorConfigWidget( relation, parent ) );
}
#endif
