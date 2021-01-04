/***************************************************************************
                         qgsbasicrelationwidget.cpp
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

#include "qgsbasicrelationwidget.h"

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

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>


QgsBasicRelationWidget::QgsBasicRelationWidget( const QVariantMap &config, QWidget *parent )
  : QgsRelationWidget( config, parent )
  , mButtonsVisibility( qgsFlagKeysToValue( config.value( QStringLiteral( "buttons" ) ).toString(), QgsBasicRelationWidget::Button::AllButtons ) )
{
  QVBoxLayout *rootLayout = new QVBoxLayout( this );
  rootLayout->setContentsMargins( 0, 0, 0, 0 );

  mRootCollapsibleGroupBox = new QgsCollapsibleGroupBox( QString(), this );
  rootLayout->addWidget( mRootCollapsibleGroupBox );

  QVBoxLayout *topLayout = new QVBoxLayout( mRootCollapsibleGroupBox );
  topLayout->setContentsMargins( 0, 9, 0, 0 );

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
  mDuplicateFeatureButton->setToolTip( tr( "Duplicate child feature" ) );
  mDuplicateFeatureButton->setObjectName( QStringLiteral( "mDuplicateFeatureButton" ) );
  buttonLayout->addWidget( mDuplicateFeatureButton );
  // delete feature
  mDeleteFeatureButton = new QToolButton( this );
  mDeleteFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ) );
  mDeleteFeatureButton->setText( tr( "Delete Child Feature" ) );
  mDeleteFeatureButton->setToolTip( tr( "Delete child feature" ) );
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
  mUnlinkFeatureButton->setToolTip( tr( "Unlink child feature" ) );
  mUnlinkFeatureButton->setObjectName( QStringLiteral( "mUnlinkFeatureButton" ) );
  buttonLayout->addWidget( mUnlinkFeatureButton );
  // zoom to linked feature
  mZoomToFeatureButton = new QToolButton( this );
  mZoomToFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToSelected.svg" ) ) );
  mZoomToFeatureButton->setText( tr( "Zoom To Feature" ) );
  mZoomToFeatureButton->setToolTip( tr( "Zoom to child feature" ) );
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
  topLayout->addLayout( buttonLayout );

  mRelationLayout = new QGridLayout();
  mRelationLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->addLayout( mRelationLayout );

  mDualView = new QgsDualView( this );
  mDualView->setView( mViewMode );

  mRelationLayout->addWidget( mDualView );

  connect( mRootCollapsibleGroupBox, &QgsCollapsibleGroupBoxBasic::collapsedStateChanged, this, &QgsBasicRelationWidget::onCollapsedStateChanged );
  connect( mViewModeButtonGroup, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ),
           this, static_cast<void ( QgsBasicRelationWidget::* )( int )>( &QgsBasicRelationWidget::setViewMode ) );
  connect( mToggleEditingButton, &QAbstractButton::clicked, this, &QgsBasicRelationWidget::toggleEditing );
  connect( mSaveEditsButton, &QAbstractButton::clicked, this, &QgsBasicRelationWidget::saveEdits );
  connect( mAddFeatureButton, &QAbstractButton::clicked, this, [this]() { addFeature(); } );
  connect( mAddFeatureGeometryButton, &QAbstractButton::clicked, this, &QgsBasicRelationWidget::addFeatureGeometry );
  connect( mDuplicateFeatureButton, &QAbstractButton::clicked, this, &QgsBasicRelationWidget::duplicateFeature );
  connect( mDeleteFeatureButton, &QAbstractButton::clicked, this, &QgsBasicRelationWidget::deleteSelectedFeatures );
  connect( mLinkFeatureButton, &QAbstractButton::clicked, this, &QgsBasicRelationWidget::linkFeature );
  connect( mUnlinkFeatureButton, &QAbstractButton::clicked, this, &QgsBasicRelationWidget::unlinkSelectedFeatures );
  connect( mZoomToFeatureButton, &QAbstractButton::clicked, this, &QgsBasicRelationWidget::zoomToSelectedFeatures );

  connect( mDualView, &QgsDualView::showContextMenuExternally, this, &QgsBasicRelationWidget::showContextMenu );

  // Set initial state for add/remove etc. buttons
  updateButtons();
}

void QgsBasicRelationWidget::initDualView( QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  QgsAttributeEditorContext ctx { mEditorContext };
  ctx.setParentFormFeature( mFeature );
  mDualView->init( layer, mEditorContext.mapCanvas(), request, ctx );
  mFeatureSelectionMgr = new QgsFilteredSelectionManager( layer, request, mDualView );
  mDualView->setFeatureSelectionManager( mFeatureSelectionMgr );

  connect( mFeatureSelectionMgr, &QgsIFeatureSelectionManager::selectionChanged, this, &QgsBasicRelationWidget::updateButtons );

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

void QgsBasicRelationWidget::setEditorContext( const QgsAttributeEditorContext &context )
{
  mEditorContext = context;

  if ( context.mapCanvas() && context.cadDockWidget() )
  {
    mMapToolDigitize.reset( new QgsMapToolDigitizeFeature( context.mapCanvas(), context.cadDockWidget() ) );
    mMapToolDigitize->setButton( mAddFeatureGeometryButton );
  }

  updateButtons();
}

void QgsBasicRelationWidget::setViewMode( QgsDualView::ViewMode mode )
{
  mDualView->setView( mode );
  mViewMode = mode;
}

void QgsBasicRelationWidget::updateButtons()
{
  bool editable = false;
  bool linkable = false;
  bool spatial = false;
  bool selectionNotEmpty = mFeatureSelectionMgr ? mFeatureSelectionMgr->selectedFeatureCount() : false;

  if ( mRelation.isValid() )
  {
    editable = mRelation.referencingLayer()->isEditable();
    linkable = mRelation.referencingLayer()->isEditable();
    spatial = mRelation.referencingLayer()->isSpatial();
  }

  if ( mNmRelation.isValid() )
  {
    editable = mNmRelation.referencedLayer()->isEditable();
    spatial = mNmRelation.referencedLayer()->isSpatial();
  }

  mAddFeatureButton->setEnabled( editable );
  mAddFeatureGeometryButton->setEnabled( editable );
  mDuplicateFeatureButton->setEnabled( editable && selectionNotEmpty );
  mLinkFeatureButton->setEnabled( linkable );
  mDeleteFeatureButton->setEnabled( editable && selectionNotEmpty );
  mUnlinkFeatureButton->setEnabled( linkable && selectionNotEmpty );
  mZoomToFeatureButton->setEnabled( selectionNotEmpty );
  mToggleEditingButton->setChecked( editable );
  mSaveEditsButton->setEnabled( editable );

  mToggleEditingButton->setVisible( !mLayerInSameTransactionGroup );

  mLinkFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsBasicRelationWidget::Button::Link ) );
  mUnlinkFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsBasicRelationWidget::Button::Unlink ) );
  mSaveEditsButton->setVisible( mButtonsVisibility.testFlag( QgsBasicRelationWidget::Button::SaveChildEdits ) && !mLayerInSameTransactionGroup );
  mAddFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsBasicRelationWidget::Button::AddChildFeature ) );
  mAddFeatureGeometryButton->setVisible( mButtonsVisibility.testFlag( QgsBasicRelationWidget::Button::AddChildFeature ) && mEditorContext.mapCanvas() && mEditorContext.cadDockWidget() && spatial );
  mDuplicateFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsBasicRelationWidget::Button::DuplicateChildFeature ) );
  mDeleteFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsBasicRelationWidget::Button::DeleteChildFeature ) );
  mZoomToFeatureButton->setVisible( mButtonsVisibility.testFlag( QgsBasicRelationWidget::Button::ZoomToChildFeature ) && mEditorContext.mapCanvas() && spatial );
}

void QgsBasicRelationWidget::addFeatureGeometry()
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

  connect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsBasicRelationWidget::onDigitizingCompleted );
  connect( mEditorContext.mapCanvas(), &QgsMapCanvas::keyPressed, this, &QgsBasicRelationWidget::onKeyPressed );

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

void QgsBasicRelationWidget::onDigitizingCompleted( const QgsFeature &feature )
{
  addFeature( feature.geometry() );

  unsetMapTool();
}

void QgsBasicRelationWidget::toggleEditing( bool state )
{
  QgsRelationWidget::toggleEditing( state );

  updateButtons();
}

void QgsBasicRelationWidget::onCollapsedStateChanged( bool collapsed )
{
  if ( !collapsed )
  {
    mVisible = true;
    updateUi();
  }
}

void QgsBasicRelationWidget::updateUi()
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

      nmRequest.setFilterExpression( filters.join( QLatin1String( " OR " ) ) );

      initDualView( mNmRelation.referencedLayer(), nmRequest );
    }
    else if ( mRelation.referencingLayer() )
    {
      initDualView( mRelation.referencingLayer(), myRequest );
    }
  }
}

void QgsBasicRelationWidget::setVisibleButtons( const Buttons &buttons )
{
  mButtonsVisibility = buttons;
  updateButtons();
}

QgsBasicRelationWidget::Buttons QgsBasicRelationWidget::visibleButtons() const
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

void QgsBasicRelationWidget::parentFormValueChanged( const QString &attribute, const QVariant &newValue )
{
  mDualView->parentFormValueChanged( attribute, newValue );
}

void QgsBasicRelationWidget::showContextMenu( QgsActionMenu *menu, const QgsFeatureId fid )
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

void QgsBasicRelationWidget::setMapTool( QgsMapTool *mapTool )
{
  QgsMapCanvas *mapCanvas = mEditorContext.mapCanvas();

  mapCanvas->setMapTool( mapTool );
  mapCanvas->window()->raise();
  mapCanvas->activateWindow();
  mapCanvas->setFocus();
  connect( mapTool, &QgsMapTool::deactivated, this, &QgsBasicRelationWidget::mapToolDeactivated );
}

void QgsBasicRelationWidget::unsetMapTool()
{
  QgsMapCanvas *mapCanvas = mEditorContext.mapCanvas();

  // this will call mapToolDeactivated
  mapCanvas->unsetMapTool( mMapToolDigitize );

  disconnect( mapCanvas, &QgsMapCanvas::keyPressed, this, &QgsBasicRelationWidget::onKeyPressed );
  disconnect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsBasicRelationWidget::onDigitizingCompleted );
}

void QgsBasicRelationWidget::onKeyPressed( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    unsetMapTool();
  }
}

void QgsBasicRelationWidget::mapToolDeactivated()
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

QVariantMap QgsBasicRelationWidget::config() const
{
  return QVariantMap( {{"buttons", qgsFlagValueToKeys( visibleButtons() )}} );
}

void QgsBasicRelationWidget::setConfig( const QVariantMap &config )
{
  mButtonsVisibility = qgsFlagKeysToValue( config.value( QStringLiteral( "buttons" ) ).toString(), QgsBasicRelationWidget::Button::AllButtons );
  updateButtons();
}

void QgsBasicRelationWidget::setTitle( const QString &title )
{
  mRootCollapsibleGroupBox->setTitle( title );
}

void QgsBasicRelationWidget::beforeSetRelationFeature( const QgsRelation &newRelation, const QgsFeature &newFeature )
{
  Q_UNUSED( newRelation );
  Q_UNUSED( newFeature );

  if ( ! mRelation.isValid() )
    return;

  disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsBasicRelationWidget::updateButtons );
  disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsBasicRelationWidget::updateButtons );
}

void QgsBasicRelationWidget::afterSetRelationFeature()
{
  mToggleEditingButton->setEnabled( false );

  if ( ! mRelation.isValid() )
    return;

  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsBasicRelationWidget::updateButtons );
  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsBasicRelationWidget::updateButtons );

  QgsVectorLayer *vl = mRelation.referencingLayer();
  bool canChangeAttributes = vl->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  if ( canChangeAttributes && !vl->readOnly() )
  {
    mToggleEditingButton->setEnabled( true );
    updateButtons();
  }
  else
  {
    mToggleEditingButton->setEnabled( false );
  }

  // If not yet initialized, it is not (yet) visible, so we don't load it to be faster (lazy loading)
  // If it is already initialized, it has been set visible before and the currently shown feature is changing
  // and the widget needs updating

  if ( mVisible )
  {
    QgsFeatureRequest myRequest = mRelation.getRelatedFeaturesRequest( mFeature );
    initDualView( mRelation.referencingLayer(), myRequest );
  }
}

void QgsBasicRelationWidget::beforeSetRelations( const QgsRelation &newRelation, const QgsRelation &newNmRelation )
{
  Q_UNUSED( newRelation );
  Q_UNUSED( newNmRelation );

  if ( mRelation.isValid() )
  {
    disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsBasicRelationWidget::updateButtons );
    disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsBasicRelationWidget::updateButtons );
  }

  if ( mNmRelation.isValid() )
  {
    disconnect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStarted, this, &QgsBasicRelationWidget::updateButtons );
    disconnect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStopped, this, &QgsBasicRelationWidget::updateButtons );
  }
}

void QgsBasicRelationWidget::afterSetRelations()
{
  if ( !mRelation.isValid() )
    return;

  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsBasicRelationWidget::updateButtons );
  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsBasicRelationWidget::updateButtons );

  if ( mNmRelation.isValid() )
  {
    connect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStarted, this, &QgsBasicRelationWidget::updateButtons );
    connect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStopped, this, &QgsBasicRelationWidget::updateButtons );
  }

  QgsVectorLayer *vl = mRelation.referencingLayer();
  bool canChangeAttributes = vl->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  if ( canChangeAttributes && !vl->readOnly() )
  {
    mToggleEditingButton->setEnabled( true );
  }
  else
  {
    mToggleEditingButton->setEnabled( false );
  }

  updateButtons();
}
