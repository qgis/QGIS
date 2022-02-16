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
#include "qgsiconutils.h"
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
#include <QTreeWidget>

/// @cond PRIVATE
///
QgsFilteredSelectionManager::QgsFilteredSelectionManager( QgsVectorLayer *layer, const QgsFeatureRequest &request, QObject *parent )
  : QgsVectorLayerSelectionManager( layer, parent )
  , mRequest( request )
{
  if ( ! layer )
    return;

  for ( const auto fid : layer->selectedFeatureIds() )
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
    for ( const auto fid : deselected )
      mSelectedFeatureIds.remove( fid );
  }

  for ( const auto fid : selected )
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
  , mShowFirstFeature( config.value( QStringLiteral( "show_first_feature" ), true ).toBool() )
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
  mDuplicateFeatureButton->setText( tr( "Duplicate Child Feature(s)" ) );
  mDuplicateFeatureButton->setToolTip( tr( "Duplicate selected child feature(s)" ) );
  mDuplicateFeatureButton->setObjectName( QStringLiteral( "mDuplicateFeatureButton" ) );
  buttonLayout->addWidget( mDuplicateFeatureButton );
  // delete feature
  mDeleteFeatureButton = new QToolButton( this );
  mDeleteFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelectedFeatures.svg" ) ) );
  mDeleteFeatureButton->setText( tr( "Delete Child Feature(s)" ) );
  mDeleteFeatureButton->setToolTip( tr( "Delete selected child feature(s)" ) );
  mDeleteFeatureButton->setObjectName( QStringLiteral( "mDeleteFeatureButton" ) );
  buttonLayout->addWidget( mDeleteFeatureButton );
  // link feature
  mLinkFeatureButton = new QToolButton( this );
  mLinkFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLink.svg" ) ) );
  mLinkFeatureButton->setText( tr( "Link Existing Feature(s)" ) );
  mLinkFeatureButton->setToolTip( tr( "Link existing child feature(s)" ) );
  mLinkFeatureButton->setObjectName( QStringLiteral( "mLinkFeatureButton" ) );
  buttonLayout->addWidget( mLinkFeatureButton );
  // unlink feature
  mUnlinkFeatureButton = new QToolButton( this );
  mUnlinkFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUnlink.svg" ) ) );
  mUnlinkFeatureButton->setText( tr( "Unlink Feature(s)" ) );
  mUnlinkFeatureButton->setToolTip( tr( "Unlink selected child feature(s)" ) );
  mUnlinkFeatureButton->setObjectName( QStringLiteral( "mUnlinkFeatureButton" ) );
  buttonLayout->addWidget( mUnlinkFeatureButton );
  // zoom to linked feature
  mZoomToFeatureButton = new QToolButton( this );
  mZoomToFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToSelected.svg" ) ) );
  mZoomToFeatureButton->setText( tr( "Zoom To Feature(s)" ) );
  mZoomToFeatureButton->setToolTip( tr( "Zoom to selected child feature(s)" ) );
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
  // multiedit info label
  mMultiEditInfoLabel = new QLabel( this );
  buttonLayout->addWidget( mMultiEditInfoLabel );

  // add buttons layout
  rootLayout->addLayout( buttonLayout );

  // add stacked widget
  mStackedWidget = new QStackedWidget( this );

  // add dual view (single feature content)
  mDualView = new QgsDualView( this );
  mDualView->setView( mViewMode );
  connect( mDualView, &QgsDualView::showContextMenuExternally, this, &QgsRelationEditorWidget::showContextMenu );

  // add multi feature editing page
  mMultiEditStackedWidgetPage = new QWidget( this );
  {
    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    vBoxLayout->setContentsMargins( 0, 0, 0, 0 );

    mMultiEditTreeWidget = new QTreeWidget( this );
    mMultiEditTreeWidget->setHeaderHidden( true );
    mMultiEditTreeWidget->setSelectionMode( QTreeWidget::ExtendedSelection );
    vBoxLayout->addWidget( mMultiEditTreeWidget );

    mMultiEditStackedWidgetPage->setLayout( vBoxLayout );
  }
  mStackedWidget->addWidget( mMultiEditStackedWidgetPage );

  mStackedWidget->addWidget( mDualView );

  rootLayout->addWidget( mStackedWidget );

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  connect( mViewModeButtonGroup, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ),
           this, static_cast<void ( QgsRelationEditorWidget::* )( int )>( &QgsRelationEditorWidget::setViewMode ) );
#else
  connect( mViewModeButtonGroup, &QButtonGroup::idClicked,
           this, static_cast<void ( QgsRelationEditorWidget::* )( int )>( &QgsRelationEditorWidget::setViewMode ) );
#endif
  connect( mToggleEditingButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::toggleEditing );
  connect( mSaveEditsButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::saveEdits );
  connect( mAddFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::addFeature );
  connect( mAddFeatureGeometryButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::addFeatureGeometry );
  connect( mDuplicateFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::duplicateSelectedFeatures );
  connect( mDeleteFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::deleteSelectedFeatures );
  connect( mLinkFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::linkFeature );
  connect( mUnlinkFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::unlinkSelectedFeatures );
  connect( mZoomToFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::zoomToSelectedFeatures );
  connect( mMultiEditTreeWidget, &QTreeWidget::itemSelectionChanged, this, &QgsRelationEditorWidget::multiEditItemSelectionChanged );

  // Set initial state for add/remove etc. buttons
  updateButtons();

  setLayout( rootLayout );
}

void QgsRelationEditorWidget::initDualView( QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  if ( multiEditModeActive() )
  {
    QgsLogger::warning( tr( "Dual view should not be used in multiple edit mode" ) );
    return;
  }

  QgsAttributeEditorContext ctx { mEditorContext };
  ctx.setParentFormFeature( mFeatureList.first() );
  mDualView->init( layer, mEditorContext.mapCanvas(), request, ctx, true, mShowFirstFeature );
  mFeatureSelectionMgr = new QgsFilteredSelectionManager( layer, request, mDualView );
  mDualView->setFeatureSelectionManager( mFeatureSelectionMgr );

  connect( mFeatureSelectionMgr, &QgsIFeatureSelectionManager::selectionChanged, this, &QgsRelationEditorWidget::updateButtons );

  QIcon icon;
  QString text;
  if ( layer->geometryType() == QgsWkbTypes::PointGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePoint.svg" ) );
    text = tr( "Add Point Child Feature" );
  }
  else if ( layer->geometryType() == QgsWkbTypes::LineGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCaptureLine.svg" ) );
    text = tr( "Add Line Child Feature" );
  }
  else if ( layer->geometryType() == QgsWkbTypes::PolygonGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePolygon.svg" ) );
    text = tr( "Add Polygon Child Feature" );
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
  bool canAdd = false;
  bool canAddGeometry = false;
  bool canRemove = false;
  bool canEdit = false;
  bool canLink = false;
  bool canUnlink = false;
  bool spatial = false;

  if ( mRelation.isValid() )
  {
    toggleEditingButtonEnabled = mRelation.referencingLayer()->supportsEditing();
    canAdd = mRelation.referencingLayer()->isEditable();
    canAddGeometry = mRelation.referencingLayer()->isEditable();
    canRemove = mRelation.referencingLayer()->isEditable();
    canEdit = mRelation.referencingLayer()->isEditable();
    canLink = mRelation.referencingLayer()->isEditable();
    canUnlink = mRelation.referencingLayer()->isEditable();
    spatial = mRelation.referencingLayer()->isSpatial();
  }

  if ( mNmRelation.isValid() )
  {
    toggleEditingButtonEnabled |= mNmRelation.referencedLayer()->supportsEditing();
    canAdd = mNmRelation.referencedLayer()->isEditable();
    canAddGeometry = mNmRelation.referencedLayer()->isEditable();
    canRemove = mNmRelation.referencedLayer()->isEditable();
    canEdit = mNmRelation.referencedLayer()->isEditable();
    spatial = mNmRelation.referencedLayer()->isSpatial();
  }

  const bool selectionNotEmpty = mFeatureSelectionMgr ? mFeatureSelectionMgr->selectedFeatureCount() : false;
  if ( multiEditModeActive() )
  {
    const bool multieditLinkedChildSelected = ! selectedChildFeatureIds().isEmpty();

    canAddGeometry = false;

    canRemove = canRemove && multieditLinkedChildSelected;

    // In 1:n relations an element can be linked only to 1 feature
    canLink = canLink && mNmRelation.isValid();
    canUnlink = canUnlink && multieditLinkedChildSelected;
  }
  else
  {
    canRemove = canRemove && selectionNotEmpty;
    canUnlink = canUnlink && selectionNotEmpty;
  }

  mToggleEditingButton->setEnabled( toggleEditingButtonEnabled );
  mAddFeatureButton->setEnabled( canAdd );
  mAddFeatureGeometryButton->setEnabled( canAddGeometry );
  mDuplicateFeatureButton->setEnabled( canEdit && selectionNotEmpty );
  mLinkFeatureButton->setEnabled( canLink );
  mDeleteFeatureButton->setEnabled( canRemove );
  mUnlinkFeatureButton->setEnabled( canUnlink );
  mZoomToFeatureButton->setEnabled( selectionNotEmpty );
  mToggleEditingButton->setChecked( canEdit );
  mSaveEditsButton->setEnabled( canEdit || canLink || canUnlink );

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

void QgsRelationEditorWidget::addFeature()
{
  const QgsFeatureIds addedFeatures = QgsAbstractRelationEditorWidget::addFeature();

  if ( !multiEditModeActive() )
    return;

  mMultiEditTreeWidget->blockSignals( true );
  mMultiEdit1NJustAddedIds = addedFeatures;
  QTreeWidgetItemIterator treeWidgetItemIterator( mMultiEditTreeWidget );
  while ( *treeWidgetItemIterator )
  {
    if ( ( *treeWidgetItemIterator )->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureType ) ).toInt() != static_cast<int>( MultiEditFeatureType::Child ) )
    {
      ++treeWidgetItemIterator;
      continue;
    }

    if ( addedFeatures.contains( ( *treeWidgetItemIterator )->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureId ) ).toInt() ) )
      ( *treeWidgetItemIterator )->setSelected( true );

    ++treeWidgetItemIterator;
  }
  mMultiEditTreeWidget->blockSignals( false );

  updateUi();
  updateButtons();
}

void QgsRelationEditorWidget::addFeatureGeometry()
{
  if ( multiEditModeActive() )
  {
    QgsLogger::warning( tr( "Adding a geometry feature is not supported in multiple edit mode" ) );
    return;
  }

  QgsVectorLayer *layer = nullptr;
  if ( mNmRelation.isValid() )
    layer = mNmRelation.referencedLayer();
  else
    layer = mRelation.referencingLayer();

  mMapToolDigitize->setLayer( layer );

  // window is always on top, so we hide it to digitize without seeing it
  if ( window()->objectName() != QStringLiteral( "QgisApp" ) )
  {
    window()->setVisible( false );
  }
  setMapTool( mMapToolDigitize );

  connect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsRelationEditorWidget::onDigitizingCompleted );
  connect( mEditorContext.mapCanvas(), &QgsMapCanvas::keyPressed, this, &QgsRelationEditorWidget::onKeyPressed );

  if ( auto *lMainMessageBar = mEditorContext.mainMessageBar() )
  {
    const QString displayString = QgsVectorLayerUtils::getFeatureDisplayString( layer, mFeatureList.first() );

    const QString title = tr( "Create child feature for parent %1 \"%2\"" ).arg( mRelation.referencedLayer()->name(), displayString );
    const QString msg = tr( "Digitize the geometry for the new feature on layer %1. Press &lt;ESC&gt; to cancel." )
                        .arg( layer->name() );
    mMessageBarItem = QgsMessageBar::createMessage( title, msg, this );
    lMainMessageBar->pushItem( mMessageBarItem );
  }
}

void QgsRelationEditorWidget::onDigitizingCompleted( const QgsFeature &feature )
{
  QgsAbstractRelationEditorWidget::addFeature( feature.geometry() );

  unsetMapTool();
}

void QgsRelationEditorWidget::multiEditItemSelectionChanged()
{
  const QList<QTreeWidgetItem *> selectedItems = mMultiEditTreeWidget->selectedItems();

  // Select all items pointing to the same feature
  // but only if we are not deselecting.
  if ( selectedItems.size() == 1
       && mMultiEditPreviousSelectedItems.size() <= 1 )
  {
    if ( selectedItems.first()->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureType ) ).toInt() == static_cast<int>( MultiEditFeatureType::Child ) )
    {
      mMultiEditTreeWidget->blockSignals( true );

      const QgsFeatureId featureIdSelectedItem = selectedItems.first()->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureId ) ).toInt();

      QTreeWidgetItemIterator treeWidgetItemIterator( mMultiEditTreeWidget );
      while ( *treeWidgetItemIterator )
      {
        if ( ( *treeWidgetItemIterator )->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureType ) ).toInt() != static_cast<int>( MultiEditFeatureType::Child ) )
        {
          ++treeWidgetItemIterator;
          continue;
        }

        const QgsFeatureId featureIdCurrentItem = ( *treeWidgetItemIterator )->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureId ) ).toInt();
        if ( mNmRelation.isValid() )
        {
          if ( featureIdSelectedItem == featureIdCurrentItem )
            ( *treeWidgetItemIterator )->setSelected( true );
        }
        else
        {
          if ( ! mMultiEdit1NJustAddedIds.contains( featureIdSelectedItem ) )
            break;

          if ( mMultiEdit1NJustAddedIds.contains( featureIdCurrentItem ) )
            ( *treeWidgetItemIterator )->setSelected( true );
        }

        ++treeWidgetItemIterator;
      }
      mMultiEditTreeWidget->blockSignals( false );
    }
  }
  mMultiEditPreviousSelectedItems = selectedItems;
  updateButtons();
}

void QgsRelationEditorWidget::toggleEditing( bool state )
{
  QgsAbstractRelationEditorWidget::toggleEditing( state );

  updateButtons();
}

void QgsRelationEditorWidget::updateUi()
{
  if ( !mRelation.isValid() || mFeatureList.isEmpty() || !mFeatureList.first().isValid() )
    return;

  if ( !isVisible() )
    return;

  if ( multiEditModeActive() )
    updateUiMultiEdit();
  else
    updateUiSingleEdit();
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

QgsFeatureIds QgsRelationEditorWidget::selectedChildFeatureIds() const
{
  if ( multiEditModeActive() )
  {
    QgsFeatureIds featureIds;
    for ( QTreeWidgetItem *treeWidgetItem : mMultiEditTreeWidget->selectedItems() )
    {
      if ( static_cast<MultiEditFeatureType>( treeWidgetItem->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureType ) ).toInt() ) != MultiEditFeatureType::Child )
        continue;

      featureIds.insert( treeWidgetItem->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureId ) ).toLongLong() );
    }
    return featureIds;
  }
  else
    return mFeatureSelectionMgr->selectedFeatureIds();
}

void QgsRelationEditorWidget::updateUiSingleEdit()
{
  mFormViewButton->setVisible( true );
  mTableViewButton->setVisible( true );
  mMultiEditInfoLabel->setVisible( false );

  mStackedWidget->setCurrentWidget( mDualView );

  const QgsFeatureRequest request = mRelation.getRelatedFeaturesRequest( mFeatureList.first() );

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

void QgsRelationEditorWidget::updateUiMultiEdit()
{
  mFormViewButton->setVisible( false );
  mTableViewButton->setVisible( false );
  mMultiEditInfoLabel->setVisible( true );

  mStackedWidget->setCurrentWidget( mMultiEditStackedWidgetPage ) ;

  QList<QTreeWidgetItem *> parentTreeWidgetItems;

  QgsFeatureIds featureIdsMixedValues;
  QMultiMap<QTreeWidgetItem *, QgsFeatureId> multimapChildFeatures;

  mMultiEditTreeWidget->clear();
  for ( const QgsFeature &featureParent : std::as_const( mFeatureList ) )
  {
    QTreeWidgetItem *treeWidgetItem = createMultiEditTreeWidgetItem( featureParent, mRelation.referencedLayer(), MultiEditFeatureType::Parent );

    // Parent feature items are not selectable
    treeWidgetItem->setFlags( Qt::ItemIsEnabled );

    parentTreeWidgetItems.append( treeWidgetItem );

    // Get child features
    const QgsFeatureRequest request = relation().getRelatedFeaturesRequest( featureParent );
    QgsFeatureIterator featureIterator = mRelation.referencingLayer()->getFeatures( request );
    QgsFeature featureChild;
    while ( featureIterator.nextFeature( featureChild ) )
    {
      if ( mNmRelation.isValid() )
      {
        const QgsFeatureRequest requestFinalChild = mNmRelation.getReferencedFeatureRequest( featureChild );
        QgsFeatureIterator featureIteratorFinalChild = mNmRelation.referencedLayer()->getFeatures( requestFinalChild );
        QgsFeature featureChildChild;
        while ( featureIteratorFinalChild.nextFeature( featureChildChild ) )
        {
          QTreeWidgetItem *treeWidgetItemChild = createMultiEditTreeWidgetItem( featureChildChild, mNmRelation.referencedLayer(), MultiEditFeatureType::Child );

          treeWidgetItem->addChild( treeWidgetItemChild );

          featureIdsMixedValues.insert( featureChildChild.id() );
          multimapChildFeatures.insert( treeWidgetItem, featureChildChild.id() );
        }
      }
      else
      {
        QTreeWidgetItem *treeWidgetItemChild = createMultiEditTreeWidgetItem( featureChild, mRelation.referencingLayer(), MultiEditFeatureType::Child );
        treeWidgetItem->addChild( treeWidgetItemChild );

        featureIdsMixedValues.insert( featureChild.id() );
      }
    }

    mMultiEditTreeWidget->addTopLevelItem( treeWidgetItem );
    treeWidgetItem->setExpanded( true );
  }

  // Set mixed values indicator (Green or Orange)
  //
  // Green:
  //     n:m and 1:n: 0 child features available
  //     n:m with no mixed values
  // Orange:
  //     n:m with mixed values
  //     1:n always, including when we pseudo know that feature are related (just added feature)
  //
  // See https://github.com/qgis/QGIS/pull/45703
  //
  if ( mNmRelation.isValid() )
  {
    QgsFeatureIds::iterator iterator = featureIdsMixedValues.begin();
    while ( iterator != featureIdsMixedValues.end() )
    {
      bool mixedValues = false;
      for ( QTreeWidgetItem *parentTreeWidgetItem : parentTreeWidgetItems )
      {
        if ( ! multimapChildFeatures.values( parentTreeWidgetItem ).contains( *iterator ) )
        {
          mixedValues = true;
          break;
        }
      }

      if ( !mixedValues )
      {
        iterator = featureIdsMixedValues.erase( iterator );
        continue;
      }

      ++iterator;
    }
  }

  // Set multiedit info label
  if ( featureIdsMixedValues.isEmpty() )
  {
    QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "/multieditSameValues.svg" ) );
    mMultiEditInfoLabel->setPixmap( icon.pixmap( mMultiEditInfoLabel->height(),
                                    mMultiEditInfoLabel->height() ) );
    mMultiEditInfoLabel->setToolTip( tr( "All features in selection have equal relations" ) );
  }
  else
  {
    QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "/multieditMixedValues.svg" ) );
    mMultiEditInfoLabel->setPixmap( icon.pixmap( mMultiEditInfoLabel->height(),
                                    mMultiEditInfoLabel->height() ) );
    mMultiEditInfoLabel->setToolTip( tr( "Some features in selection have different relations" ) );

    // Set italic font for mixed values
    QFont fontItalic = mMultiEditTreeWidget->font();
    fontItalic.setItalic( true );
    for ( QTreeWidgetItem *parentTreeWidgetItem : parentTreeWidgetItems )
    {
      for ( int childIndex = 0; childIndex < parentTreeWidgetItem->childCount(); ++childIndex )
      {
        QTreeWidgetItem *childItem = parentTreeWidgetItem->child( childIndex );
        const QgsFeatureId featureIdCurrentItem = childItem->data( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureId ) ).toInt();
        if ( featureIdsMixedValues.contains( featureIdCurrentItem ) )
          childItem->setFont( 0, fontItalic );
      }
    }
  }
}

QTreeWidgetItem *QgsRelationEditorWidget::createMultiEditTreeWidgetItem( const QgsFeature &feature, QgsVectorLayer *layer, MultiEditFeatureType type )
{
  QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem();
  treeWidgetItem->setText( 0, QgsVectorLayerUtils::getFeatureDisplayString( layer, feature ) );
  treeWidgetItem->setIcon( 0, QgsIconUtils::iconForLayer( layer ) );
  treeWidgetItem->setData( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureType ), static_cast<int>( type ) );
  treeWidgetItem->setData( 0, static_cast<int>( MultiEditTreeWidgetRole::FeatureId ), feature.id() );
  return treeWidgetItem;
}

void QgsRelationEditorWidget::onKeyPressed( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    window()->setVisible( true );
    window()->raise();
    window()->activateWindow();
    unsetMapTool();
  }
}

void QgsRelationEditorWidget::mapToolDeactivated()
{
  if ( mEditorContext.mainMessageBar() && mMessageBarItem )
  {
    mEditorContext.mainMessageBar()->popWidget( mMessageBarItem );
  }
  mMessageBarItem = nullptr;
}

QVariantMap QgsRelationEditorWidget::config() const
{
  return QVariantMap( {{"buttons", qgsFlagValueToKeys( visibleButtons() )},
    {"show_first_feature", mShowFirstFeature}} );
}

void QgsRelationEditorWidget::setConfig( const QVariantMap &config )
{
  mButtonsVisibility = qgsFlagKeysToValue( config.value( QStringLiteral( "buttons" ) ).toString(), QgsRelationEditorWidget::Button::AllButtons );
  mShowFirstFeature = config.value( QStringLiteral( "show_first_feature" ), true ).toBool();
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
  if ( ! mRelation.isValid()
       || mFeatureList.isEmpty() )
  {
    updateButtons();
    return;
  }

  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );

  updateButtons();

  const QgsFeatureRequest myRequest = mRelation.getRelatedFeaturesRequest( mFeatureList.first() );
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
  const QgsFeatureIds selectedFids = selectedChildFeatureIds();
  unlinkFeatures( selectedFids );
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
  const QgsFeatureIds selectedFids = selectedChildFeatureIds();
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
    {"show_first_feature", mShowFirstFeature->isChecked()}
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
  mShowFirstFeature->setChecked( config.value( QStringLiteral( "show_first_feature" ), true ).toBool() );
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
  return QObject::tr( "Relation Editor" );
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
