/***************************************************************************
                         qgslayoutmapwidget.cpp
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutmapwidget.h"
#include "qgssettingsregistrycore.h"
#include "qgslayoutitemmap.h"
#include "qgsproject.h"
#include "qgsmapthemecollection.h"
#include "qgslayout.h"
#include "qgslayertree.h"
#include "qgsmapcanvas.h"
#include "qgslayoutmapgridwidget.h"
#include "qgslayoutundostack.h"
#include "qgslayoutatlas.h"
#include "qgslayoutdesignerinterface.h"
#include "qgsguiutils.h"
#include "qgsbookmarkmodel.h"
#include "qgsreferencedgeometry.h"
#include "qgsprojectviewsettings.h"
#include "qgsmaplayermodel.h"
#include "qgsfillsymbol.h"

#include <QMenu>
#include <QMessageBox>
#include <QStringListModel>

QgsLayoutMapWidget::QgsLayoutMapWidget( QgsLayoutItemMap *item, QgsMapCanvas *mapCanvas )
  : QgsLayoutItemBaseWidget( nullptr, item )
  , mMapItem( item )
  , mMapCanvas( mapCanvas )
{
  Q_ASSERT( mMapItem );

  setupUi( this );
  connect( mScaleLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutMapWidget::mScaleLineEdit_editingFinished );
  connect( mActionSetToCanvasExtent, &QAction::triggered, this, &QgsLayoutMapWidget::setToMapCanvasExtent );
  connect( mActionViewExtentInCanvas, &QAction::triggered, this, &QgsLayoutMapWidget::viewExtentInCanvas );
  connect( mActionSetToCanvasScale, &QAction::triggered, this, &QgsLayoutMapWidget::setToMapCanvasScale );
  connect( mActionViewScaleInCanvas, &QAction::triggered, this, &QgsLayoutMapWidget::viewScaleInCanvas );
  connect( mActionUpdatePreview, &QAction::triggered, this, &QgsLayoutMapWidget::updatePreview );
  connect( mFollowVisibilityPresetCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMapWidget::mFollowVisibilityPresetCheckBox_stateChanged );
  connect( mKeepLayerListCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMapWidget::mKeepLayerListCheckBox_stateChanged );
  connect( mKeepLayerStylesCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMapWidget::mKeepLayerStylesCheckBox_stateChanged );
  connect( mDrawCanvasItemsCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMapWidget::mDrawCanvasItemsCheckBox_stateChanged );
  connect( mOverviewBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapWidget::mOverviewBlendModeComboBox_currentIndexChanged );
  connect( mOverviewInvertCheckbox, &QCheckBox::toggled, this, &QgsLayoutMapWidget::mOverviewInvertCheckbox_toggled );
  connect( mOverviewCenterCheckbox, &QCheckBox::toggled, this, &QgsLayoutMapWidget::mOverviewCenterCheckbox_toggled );
  connect( mOverviewPositionComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapWidget::overviewStackingChanged );
  connect( mOverviewStackingLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsLayoutMapWidget::overviewStackingLayerChanged );
  connect( mXMinLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutMapWidget::mXMinLineEdit_editingFinished );
  connect( mXMaxLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutMapWidget::mXMaxLineEdit_editingFinished );
  connect( mYMinLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutMapWidget::mYMinLineEdit_editingFinished );
  connect( mYMaxLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutMapWidget::mYMaxLineEdit_editingFinished );
  connect( mAtlasMarginRadio, &QRadioButton::toggled, this, &QgsLayoutMapWidget::mAtlasMarginRadio_toggled );
  connect( mAtlasCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutMapWidget::mAtlasCheckBox_toggled );
  connect( mAtlasMarginSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutMapWidget::mAtlasMarginSpinBox_valueChanged );
  connect( mAtlasFixedScaleRadio, &QRadioButton::toggled, this, &QgsLayoutMapWidget::mAtlasFixedScaleRadio_toggled );
  connect( mAtlasPredefinedScaleRadio, &QRadioButton::toggled, this, &QgsLayoutMapWidget::mAtlasPredefinedScaleRadio_toggled );
  connect( mAddGridPushButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mAddGridPushButton_clicked );
  connect( mRemoveGridPushButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mRemoveGridPushButton_clicked );
  connect( mGridUpButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mGridUpButton_clicked );
  connect( mGridDownButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mGridDownButton_clicked );
  connect( mGridListWidget, &QListWidget::currentItemChanged, this, &QgsLayoutMapWidget::mGridListWidget_currentItemChanged );
  connect( mGridListWidget, &QListWidget::itemChanged, this, &QgsLayoutMapWidget::mGridListWidget_itemChanged );
  connect( mGridPropertiesButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mGridPropertiesButton_clicked );
  connect( mAddOverviewPushButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mAddOverviewPushButton_clicked );
  connect( mRemoveOverviewPushButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mRemoveOverviewPushButton_clicked );
  connect( mOverviewUpButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mOverviewUpButton_clicked );
  connect( mOverviewDownButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mOverviewDownButton_clicked );
  connect( mOverviewCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutMapWidget::mOverviewCheckBox_toggled );
  connect( mOverviewListWidget, &QListWidget::currentItemChanged, this, &QgsLayoutMapWidget::mOverviewListWidget_currentItemChanged );
  connect( mOverviewListWidget, &QListWidget::itemChanged, this, &QgsLayoutMapWidget::mOverviewListWidget_itemChanged );
  connect( mActionLabelSettings, &QAction::triggered, this, &QgsLayoutMapWidget::showLabelSettings );
  connect( mActionClipSettings, &QAction::triggered, this, &QgsLayoutMapWidget::showClipSettings );

  connect( mTemporalCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutMapWidget::mTemporalCheckBox_toggled );
  connect( mStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsLayoutMapWidget::updateTemporalExtent );
  connect( mEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsLayoutMapWidget::updateTemporalExtent );

  mZLowerSpin->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mZUpperSpin->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  connect( mElevationRangeCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutMapWidget::mElevationRangeCheckBox_toggled );
  connect( mZLowerSpin, qOverload< double >( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutMapWidget::updateZRange );
  connect( mZUpperSpin, qOverload< double >( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutMapWidget::updateZRange );

  mStartDateTime->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTime->maximumDateTime() );
  mEndDateTime->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTime->maximumDateTime() );
  mStartDateTime->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mEndDateTime->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  connect( mActionMoveContent, &QAction::triggered, this, &QgsLayoutMapWidget::switchToMoveContentTool );
  setPanelTitle( tr( "Map Properties" ) );
  mMapRotationSpinBox->setClearValue( 0 );

  mDockToolbar->setIconSize( QgsGuiUtils::iconSize( true ) );

  mBookmarkMenu = new QMenu( this );
  QToolButton *btnBookmarks = new QToolButton( this );
  btnBookmarks->setAutoRaise( true );
  btnBookmarks->setToolTip( tr( "Bookmarks" ) );
  btnBookmarks->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowBookmarks.svg" ) ) );
  btnBookmarks->setPopupMode( QToolButton::InstantPopup );
  btnBookmarks->setMenu( mBookmarkMenu );

  mDockToolbar->insertWidget( mActionMoveContent, btnBookmarks );
  connect( mBookmarkMenu, &QMenu::aboutToShow, this, &QgsLayoutMapWidget::aboutToShowBookmarkMenu );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, item );
  mainLayout->addWidget( mItemPropertiesWidget );

  mScaleLineEdit->setValidator( new QDoubleValidator( mScaleLineEdit ) );

  mXMinLineEdit->setValidator( new QDoubleValidator( mXMinLineEdit ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( mXMaxLineEdit ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( mYMinLineEdit ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( mYMaxLineEdit ) );

  mOverviewPositionComboBox->addItem( tr( "Below Map" ), QgsLayoutItemMapItem::StackBelowMap );
  mOverviewPositionComboBox->addItem( tr( "Below Map Layer" ), QgsLayoutItemMapItem::StackBelowMapLayer );
  mOverviewPositionComboBox->addItem( tr( "Above Map Layer" ), QgsLayoutItemMapItem::StackAboveMapLayer );
  mOverviewPositionComboBox->addItem( tr( "Below Map Labels" ), QgsLayoutItemMapItem::StackBelowMapLabels );
  mOverviewPositionComboBox->addItem( tr( "Above Map Labels" ), QgsLayoutItemMapItem::StackAboveMapLabels );

  blockAllSignals( true );

  mCrsSelector->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  mCrsSelector->setNotSetText( tr( "Use Project CRS" ) );
  mCrsSelector->setDialogTitle( tr( "Map Item CRS" ) );

  mOverviewFrameStyleButton->setSymbolType( Qgis::SymbolType::Fill );

  // follow preset combo
  mFollowVisibilityPresetCombo->setModel( new QStringListModel( mFollowVisibilityPresetCombo ) );
  connect( mFollowVisibilityPresetCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapWidget::followVisibilityPresetSelected );
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemesChanged,
           this, &QgsLayoutMapWidget::onMapThemesChanged );
  onMapThemesChanged();

  // keep layers from preset button
  QMenu *menuKeepLayers = new QMenu( this );
  mLayerListFromPresetButton->setMenu( menuKeepLayers );
  mLayerListFromPresetButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ) );
  mLayerListFromPresetButton->setToolTip( tr( "Set layer list from a map theme" ) );
  connect( menuKeepLayers, &QMenu::aboutToShow, this, &QgsLayoutMapWidget::aboutToShowKeepLayersVisibilityPresetsMenu );

  connect( item, &QgsLayoutObject::changed, this, &QgsLayoutMapWidget::updateGuiElements );

  connect( &item->layout()->reportContext(), &QgsLayoutReportContext::layerChanged,
           this, &QgsLayoutMapWidget::atlasLayerChanged );
  if ( QgsLayoutAtlas *atlas = layoutAtlas() )
  {
    connect( atlas, &QgsLayoutAtlas::toggled, this, &QgsLayoutMapWidget::compositionAtlasToggled );
    compositionAtlasToggled( atlas->enabled() );
  }

  mOverviewFrameMapComboBox->setCurrentLayout( item->layout() );
  mOverviewFrameMapComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
  mOverviewFrameStyleButton->registerExpressionContextGenerator( item );

  connect( mOverviewFrameMapComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutMapWidget::overviewMapChanged );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsLayoutMapWidget::mapCrsChanged );
  connect( mOverviewFrameStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutMapWidget::overviewSymbolChanged );

  mOverviewFrameStyleButton->registerExpressionContextGenerator( item );
  mOverviewFrameStyleButton->setLayer( coverageLayer() );
  if ( item->layout() )
  {
    connect( &item->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mOverviewFrameStyleButton, &QgsSymbolButton::setLayer );
  }


  registerDataDefinedButton( mScaleDDBtn, QgsLayoutObject::DataDefinedProperty::MapScale );
  registerDataDefinedButton( mMapRotationDDBtn, QgsLayoutObject::DataDefinedProperty::MapRotation );
  registerDataDefinedButton( mXMinDDBtn, QgsLayoutObject::DataDefinedProperty::MapXMin );
  registerDataDefinedButton( mYMinDDBtn, QgsLayoutObject::DataDefinedProperty::MapYMin );
  registerDataDefinedButton( mXMaxDDBtn, QgsLayoutObject::DataDefinedProperty::MapXMax );
  registerDataDefinedButton( mYMaxDDBtn, QgsLayoutObject::DataDefinedProperty::MapYMax );
  registerDataDefinedButton( mAtlasMarginDDBtn, QgsLayoutObject::DataDefinedProperty::MapAtlasMargin );
  registerDataDefinedButton( mStylePresetsDDBtn, QgsLayoutObject::DataDefinedProperty::MapStylePreset );
  registerDataDefinedButton( mLayersDDBtn, QgsLayoutObject::DataDefinedProperty::MapLayers );
  registerDataDefinedButton( mCRSDDBtn, QgsLayoutObject::DataDefinedProperty::MapCrs );
  registerDataDefinedButton( mStartDateTimeDDBtn, QgsLayoutObject::DataDefinedProperty::StartDateTime );
  registerDataDefinedButton( mEndDateTimeDDBtn, QgsLayoutObject::DataDefinedProperty::EndDateTime );
  registerDataDefinedButton( mZLowerDDBtn, QgsLayoutObject::DataDefinedProperty::MapZRangeLower );
  registerDataDefinedButton( mZUpperDDBtn, QgsLayoutObject::DataDefinedProperty::MapZRangeUpper );

  updateGuiElements();
  loadGridEntries();
  loadOverviewEntries();

  connect( mMapRotationSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutMapWidget::rotationChanged );
  connect( mMapItem, &QgsLayoutItemMap::extentChanged, mItemPropertiesWidget, &QgsLayoutItemPropertiesWidget::updateVariables );
  connect( mMapItem, &QgsLayoutItemMap::mapRotationChanged, mItemPropertiesWidget, &QgsLayoutItemPropertiesWidget::updateVariables );

  blockAllSignals( false );
}

void QgsLayoutMapWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

void QgsLayoutMapWidget::setReportTypeString( const QString &string )
{
  mReportTypeString = string;
  mAtlasCheckBox->setTitle( tr( "Controlled by %1" ).arg( string == tr( "atlas" ) ? tr( "Atlas" ) : tr( "Report" ) ) );
  mAtlasPredefinedScaleRadio->setToolTip( tr( "Use one of the predefined scales of the project where the %1 feature best fits." ).arg( string ) );

  if ( mClipWidget )
    mClipWidget->setReportTypeString( string );
  if ( mLabelWidget )
    mLabelWidget->setReportTypeString( string );
}

void QgsLayoutMapWidget::setDesignerInterface( QgsLayoutDesignerInterface *iface )
{
  mInterface = iface;
  QgsLayoutItemBaseWidget::setDesignerInterface( iface );
}

bool QgsLayoutMapWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutMap )
    return false;

  if ( mMapItem )
  {
    disconnect( mMapItem, &QgsLayoutObject::changed, this, &QgsLayoutMapWidget::updateGuiElements );
    disconnect( mMapItem, &QgsLayoutItemMap::extentChanged, mItemPropertiesWidget, &QgsLayoutItemPropertiesWidget::updateVariables );
    disconnect( mMapItem, &QgsLayoutItemMap::mapRotationChanged, mItemPropertiesWidget, &QgsLayoutItemPropertiesWidget::updateVariables );
  }

  mMapItem = qobject_cast< QgsLayoutItemMap * >( item );
  mItemPropertiesWidget->setItem( mMapItem );
  if ( mLabelWidget )
    mLabelWidget->setItem( mMapItem );
  if ( mClipWidget )
    mClipWidget->setItem( mMapItem );

  if ( mMapItem )
  {
    connect( mMapItem, &QgsLayoutObject::changed, this, &QgsLayoutMapWidget::updateGuiElements );
    connect( mMapItem, &QgsLayoutItemMap::extentChanged, mItemPropertiesWidget, &QgsLayoutItemPropertiesWidget::updateVariables );
    connect( mMapItem, &QgsLayoutItemMap::mapRotationChanged, mItemPropertiesWidget, &QgsLayoutItemPropertiesWidget::updateVariables );
    mOverviewFrameStyleButton->registerExpressionContextGenerator( mMapItem );
  }

  updateGuiElements();

  return true;
}

void QgsLayoutMapWidget::populateDataDefinedButtons()
{
  updateDataDefinedButton( mScaleDDBtn );
  updateDataDefinedButton( mMapRotationDDBtn );
  updateDataDefinedButton( mXMinDDBtn );
  updateDataDefinedButton( mYMinDDBtn );
  updateDataDefinedButton( mXMaxDDBtn );
  updateDataDefinedButton( mYMaxDDBtn );
  updateDataDefinedButton( mAtlasMarginDDBtn );
  updateDataDefinedButton( mStylePresetsDDBtn );
  updateDataDefinedButton( mLayersDDBtn );
  updateDataDefinedButton( mCRSDDBtn );
  updateDataDefinedButton( mStartDateTimeDDBtn );
  updateDataDefinedButton( mEndDateTimeDDBtn );
}

void QgsLayoutMapWidget::compositionAtlasToggled( bool atlasEnabled )
{
  if ( atlasEnabled &&
       mMapItem && mMapItem->layout() && mMapItem->layout()->reportContext().layer()
       && mMapItem->layout()->reportContext().layer()->wkbType() != Qgis::WkbType::NoGeometry )
  {
    mAtlasCheckBox->setEnabled( true );
  }
  else
  {
    mAtlasCheckBox->setEnabled( false );
    mAtlasCheckBox->setChecked( false );
  }
}

void QgsLayoutMapWidget::aboutToShowKeepLayersVisibilityPresetsMenu()
{
  // this menu is for the case when setting "keep layers" and "keep layer styles"
  // and the preset configuration is copied. The preset is not followed further.

  QMenu *menu = qobject_cast<QMenu *>( sender() );
  if ( !menu )
    return;

  menu->clear();
  const auto constMapThemes = QgsProject::instance()->mapThemeCollection()->mapThemes();
  for ( const QString &presetName : constMapThemes )
  {
    menu->addAction( presetName, this, &QgsLayoutMapWidget::keepLayersVisibilityPresetSelected );
  }

  if ( menu->actions().isEmpty() )
    menu->addAction( tr( "No presets defined" ) )->setEnabled( false );
}

void QgsLayoutMapWidget::followVisibilityPresetSelected( int currentIndex )
{
  if ( !mMapItem )
    return;

  if ( mBlockThemeComboChanges != 0 )
    return;

  if ( currentIndex == -1 )
    return;  // doing combo box model reset

  QString presetName;
  if ( currentIndex != 0 )
  {
    presetName = mFollowVisibilityPresetCombo->currentText();
  }

  if ( presetName == mMapItem->followVisibilityPresetName() )
    return;

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Preset" ) );
  mFollowVisibilityPresetCheckBox->setChecked( true );
  mMapItem->setFollowVisibilityPresetName( presetName );
  mMapItem->layout()->undoStack()->endCommand();

  mMapItem->invalidateCache();
}

void QgsLayoutMapWidget::keepLayersVisibilityPresetSelected()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  const QString presetName = action->text();
  const QList<QgsMapLayer *> lst = orderedPresetVisibleLayers( presetName );
  if ( mMapItem )
  {
    mKeepLayerListCheckBox->setChecked( true );
    mMapItem->setLayers( lst );

    mKeepLayerStylesCheckBox->setChecked( true );

    mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Preset" ) );
    mMapItem->setLayerStyleOverrides( QgsProject::instance()->mapThemeCollection()->mapThemeStyleOverrides( presetName ) );
    mMapItem->layout()->undoStack()->endCommand();

    mMapItem->invalidateCache();
  }
}

void QgsLayoutMapWidget::onMapThemesChanged()
{
  if ( QStringListModel *model = qobject_cast<QStringListModel *>( mFollowVisibilityPresetCombo->model() ) )
  {
    mBlockThemeComboChanges++;
    QStringList lst;
    lst.append( tr( "(none)" ) );
    lst += QgsProject::instance()->mapThemeCollection()->mapThemes();
    model->setStringList( lst );

    // select the previously selected item again
    const int presetModelIndex = mFollowVisibilityPresetCombo->findText( mMapItem->followVisibilityPresetName() );
    mFollowVisibilityPresetCombo->blockSignals( true );
    mFollowVisibilityPresetCombo->setCurrentIndex( presetModelIndex != -1 ? presetModelIndex : 0 ); // 0 == none
    mFollowVisibilityPresetCombo->blockSignals( false );
    mBlockThemeComboChanges--;
  }
}

void QgsLayoutMapWidget::mapCrsChanged( const QgsCoordinateReferenceSystem &crs )
{
  if ( !mMapItem )
  {
    return;
  }

  if ( mMapItem->presetCrs() == crs )
    return;

  // try to reproject to maintain extent
  const QgsCoordinateReferenceSystem oldCrs = mMapItem->crs();

  bool updateExtent = false;
  QgsRectangle newExtent;
  try
  {
    QgsCoordinateTransform xForm( oldCrs, crs.isValid() ? crs : QgsProject::instance()->crs(), QgsProject::instance() );
    xForm.setBallparkTransformsAreAppropriate( true );
    const QgsRectangle prevExtent = mMapItem->extent();
    newExtent = xForm.transformBoundingBox( prevExtent );
    updateExtent = true;
  }
  catch ( QgsCsException & )
  {
    //transform failed, don't update extent
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map CRS" ) );
  mMapItem->setCrs( crs );
  if ( updateExtent )
    mMapItem->zoomToExtent( newExtent );
  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->invalidateCache();
}

void QgsLayoutMapWidget::overviewSymbolChanged()
{
  QgsLayoutItemMapOverview *overview = currentOverview();
  if ( !overview )
    return;

  mMapItem->beginCommand( tr( "Change Overview Style" ), QgsLayoutItem::UndoOverviewStyle );
  overview->setFrameSymbol( mOverviewFrameStyleButton->clonedSymbol<QgsFillSymbol>() );
  mMapItem->endCommand();
  mMapItem->invalidateCache();
}

void QgsLayoutMapWidget::showLabelSettings()
{
  mLabelWidget = new QgsLayoutMapLabelingWidget( mMapItem );

  if ( !mReportTypeString.isEmpty() )
    mLabelWidget->setReportTypeString( mReportTypeString );

  openPanel( mLabelWidget );
}

void QgsLayoutMapWidget::showClipSettings()
{
  mClipWidget = new QgsLayoutMapClippingWidget( mMapItem );
  if ( !mReportTypeString.isEmpty() )
    mClipWidget->setReportTypeString( mReportTypeString );
  openPanel( mClipWidget );
}

void QgsLayoutMapWidget::switchToMoveContentTool()
{
  if ( mInterface )
    mInterface->activateTool( QgsLayoutDesignerInterface::ToolMoveItemContent );
}

void QgsLayoutMapWidget::aboutToShowBookmarkMenu()
{
  mBookmarkMenu->clear();

  // query the bookmarks now? or once during widget creation... Hmm. Either way, there's potentially a
  // delay if there's LOTS of bookmarks. Let's avoid the cost until bookmarks are actually required.
  if ( !mBookmarkModel )
    mBookmarkModel = new QgsBookmarkManagerProxyModel( QgsApplication::bookmarkManager(), QgsProject::instance()->bookmarkManager(), this );

  QMap< QString, QMenu * > groupMenus;
  for ( int i = 0; i < mBookmarkModel->rowCount(); ++i )
  {
    const QString group = mBookmarkModel->data( mBookmarkModel->index( i, 0 ), static_cast< int >( QgsBookmarkManagerModel::CustomRole::Group ) ).toString();
    QMenu *destMenu = mBookmarkMenu;
    if ( !group.isEmpty() )
    {
      destMenu = groupMenus.value( group );
      if ( !destMenu )
      {
        destMenu = new QMenu( group, mBookmarkMenu );
        groupMenus[ group ] = destMenu;
      }
    }
    QAction *action = new QAction( mBookmarkModel->data( mBookmarkModel->index( i, 0 ), static_cast< int >( QgsBookmarkManagerModel::CustomRole::Name ) ).toString(), mBookmarkMenu );
    const QgsReferencedRectangle extent = mBookmarkModel->data( mBookmarkModel->index( i, 0 ), static_cast< int >( QgsBookmarkManagerModel::CustomRole::Extent ) ).value< QgsReferencedRectangle >();
    connect( action, &QAction::triggered, this, [ = ]
    {
      if ( !mMapItem )
      {
        return;
      }

      QgsRectangle newExtent = extent;

      //transform?
      if ( extent.crs() != mMapItem->crs() )
      {
        try
        {
          QgsCoordinateTransform xForm( extent.crs(), mMapItem->crs(), QgsProject::instance() );
          xForm.setBallparkTransformsAreAppropriate( true );
          newExtent = xForm.transformBoundingBox( newExtent );
        }
        catch ( QgsCsException & )
        {
          //transform failed, better not proceed
          return;
        }
      }

      mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Extent" ) );
      mMapItem->zoomToExtent( newExtent );
      mMapItem->layout()->undoStack()->endCommand();
    } );
    destMenu->addAction( action );
  }

  QStringList groupKeys = groupMenus.keys();
  groupKeys.sort( Qt::CaseInsensitive );
  for ( int i = 0; i < groupKeys.count(); ++i )
  {
    if ( mBookmarkMenu->actions().value( i ) )
      mBookmarkMenu->insertMenu( mBookmarkMenu->actions().at( i ), groupMenus.value( groupKeys.at( i ) ) );
    else
      mBookmarkMenu->addMenu( groupMenus.value( groupKeys.at( i ) ) );
  }
}

void QgsLayoutMapWidget::mTemporalCheckBox_toggled( bool checked )
{
  if ( !mMapItem )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Toggle Temporal Range" ) );
  mMapItem->setIsTemporal( checked );
  mMapItem->layout()->undoStack()->endCommand();

  if ( checked )
  {
    whileBlocking( mStartDateTime )->setDateTime( mMapItem->temporalRange().begin() );
    whileBlocking( mEndDateTime )->setDateTime( mMapItem->temporalRange().end() );
  }

  updatePreview();
}

void QgsLayoutMapWidget::updateTemporalExtent()
{
  if ( !mMapItem )
  {
    return;
  }

  const QDateTime begin = mStartDateTime->dateTime();
  const QDateTime end = mEndDateTime->dateTime();
  const QgsDateTimeRange range = QgsDateTimeRange( begin, end, true, begin == end );

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Set Temporal Range" ) );
  mMapItem->setTemporalRange( range );
  mMapItem->layout()->undoStack()->endCommand();

  updatePreview();
}

void QgsLayoutMapWidget::mElevationRangeCheckBox_toggled( bool checked )
{
  if ( !mMapItem )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Toggle Z Range" ) );
  mMapItem->setZRangeEnabled( checked );
  mMapItem->layout()->undoStack()->endCommand();

  updatePreview();
}

void QgsLayoutMapWidget::updateZRange()
{
  if ( !mMapItem )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Set Z Range" ) );
  double zLower = mZLowerSpin->value();
  if ( zLower == mZLowerSpin->clearValue() )
    zLower = std::numeric_limits< double >::lowest();
  double zUpper = mZUpperSpin->value();
  if ( zUpper == mZUpperSpin->clearValue() )
    zUpper = std::numeric_limits< double >::max();

  mMapItem->setZRange( QgsDoubleRange( zLower, zUpper ) );
  mMapItem->layout()->undoStack()->endCommand();

  updatePreview();
}

void QgsLayoutMapWidget::mAtlasCheckBox_toggled( bool checked )
{
  if ( !mMapItem )
  {
    return;
  }

  mAtlasFixedScaleRadio->setEnabled( checked );
  mAtlasMarginRadio->setEnabled( checked );

  if ( mAtlasMarginRadio->isEnabled() && mAtlasMarginRadio->isChecked() )
  {
    mAtlasMarginSpinBox->setEnabled( true );
  }
  else
  {
    mAtlasMarginSpinBox->setEnabled( false );
  }

  mAtlasPredefinedScaleRadio->setEnabled( checked );

  if ( checked )
  {
    //check atlas coverage layer type
    if ( mMapItem->layout() )
    {
      toggleAtlasScalingOptionsByLayerType();
    }
  }

  // disable predefined scales if none are defined
  if ( !hasPredefinedScales() )
  {
    mAtlasPredefinedScaleRadio->setEnabled( false );
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Set Atlas Driven" ) );
  mMapItem->setAtlasDriven( checked );
  mMapItem->layout()->undoStack()->endCommand();
  updateMapForAtlas();
}

void QgsLayoutMapWidget::updateMapForAtlas()
{
  //update map if in atlas preview mode
  if ( mMapItem->atlasDriven() )
  {
    mMapItem->refresh();
  }
  else
  {
    //redraw map
    mMapItem->invalidateCache();
  }
}

void QgsLayoutMapWidget::mAtlasMarginRadio_toggled( bool checked )
{
  mAtlasMarginSpinBox->setEnabled( checked );

  if ( checked && mMapItem )
  {
    mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Atlas Mode" ) );
    mMapItem->setAtlasScalingMode( QgsLayoutItemMap::Auto );
    mMapItem->layout()->undoStack()->endCommand();
    updateMapForAtlas();
  }
}

void QgsLayoutMapWidget::mAtlasMarginSpinBox_valueChanged( int value )
{
  if ( !mMapItem )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Atlas Margin" ), QgsLayoutItem::UndoAtlasMargin );
  mMapItem->setAtlasMargin( value / 100. );
  mMapItem->layout()->undoStack()->endCommand();
  updateMapForAtlas();
}

void QgsLayoutMapWidget::mAtlasFixedScaleRadio_toggled( bool checked )
{
  if ( !mMapItem )
  {
    return;
  }

  if ( checked )
  {
    mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Atlas Mode" ) );
    mMapItem->setAtlasScalingMode( QgsLayoutItemMap::Fixed );
    mMapItem->layout()->undoStack()->endCommand();
    updateMapForAtlas();
  }
}

void QgsLayoutMapWidget::mAtlasPredefinedScaleRadio_toggled( bool checked )
{
  if ( !mMapItem )
  {
    return;
  }

  if ( hasPredefinedScales() )
  {
    if ( checked )
    {
      mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Atlas Scales" ) );
      mMapItem->setAtlasScalingMode( QgsLayoutItemMap::Predefined );
      mMapItem->layout()->undoStack()->endCommand();
      updateMapForAtlas();
    }
  }
  else
  {
    // restore to fixed scale if no predefined scales exist
    whileBlocking( mAtlasFixedScaleRadio )->setChecked( true );
    mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Atlas Mode" ) );
    mMapItem->setAtlasScalingMode( QgsLayoutItemMap::Fixed );
    mMapItem->layout()->undoStack()->endCommand();
  }
}

void QgsLayoutMapWidget::mScaleLineEdit_editingFinished()
{
  if ( !mMapItem )
  {
    return;
  }

  bool conversionSuccess = false;
  const double scaleDenominator = QLocale().toDouble( mScaleLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
  {
    return;
  }

  if ( qgsDoubleNear( scaleDenominator, mMapItem->scale() ) )
    return;

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Scale" ) );
  mMapItem->setScale( scaleDenominator );
  mMapItem->layout()->undoStack()->endCommand();
}

void QgsLayoutMapWidget::rotationChanged( double value )
{
  if ( !mMapItem )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Rotation" ), QgsLayoutItem::UndoMapRotation );
  mMapItem->setMapRotation( value );
  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->invalidateCache();
}

void QgsLayoutMapWidget::setToMapCanvasExtent()
{
  if ( !mMapItem )
  {
    return;
  }

  QgsRectangle newExtent = mMapCanvas->mapSettings().visibleExtent();

  //transform?
  if ( mMapCanvas->mapSettings().destinationCrs()
       != mMapItem->crs() )
  {
    try
    {
      QgsCoordinateTransform xForm( mMapCanvas->mapSettings().destinationCrs(),
                                    mMapItem->crs(), QgsProject::instance() );
      xForm.setBallparkTransformsAreAppropriate( true );
      newExtent = xForm.transformBoundingBox( newExtent );
    }
    catch ( QgsCsException & )
    {
      //transform failed, better not proceed
      return;
    }
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Extent" ) );
  mMapItem->zoomToExtent( newExtent );
  mMapItem->layout()->undoStack()->endCommand();
}

void QgsLayoutMapWidget::setToMapCanvasScale()
{
  if ( !mMapItem )
  {
    return;
  }

  const double newScale = mMapCanvas->scale();

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Scale" ) );
  mMapItem->setScale( newScale );
  mMapItem->layout()->undoStack()->endCommand();
}

void QgsLayoutMapWidget::viewExtentInCanvas()
{
  if ( !mMapItem )
  {
    return;
  }

  const QgsRectangle currentMapExtent = mMapItem->extent();

  if ( !currentMapExtent.isEmpty() )
  {
    try
    {
      mMapCanvas->setReferencedExtent( QgsReferencedRectangle( currentMapExtent, mMapItem->crs() ) );
    }
    catch ( QgsCsException & )
    {
      //transform failed, better not proceed
      return;
    }
    mMapCanvas->refresh();
  }
}

void QgsLayoutMapWidget::viewScaleInCanvas()
{
  if ( !mMapItem )
  {
    return;
  }

  const double currentScale = mMapItem->scale();
  mMapCanvas->zoomScale( currentScale, true );
}

void QgsLayoutMapWidget::mXMinLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsLayoutMapWidget::mXMaxLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsLayoutMapWidget::mYMinLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsLayoutMapWidget::mYMaxLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsLayoutMapWidget::updateGuiElements()
{
  if ( !mMapItem )
  {
    return;
  }

  blockAllSignals( true );
  mLabel->setText( mMapItem->displayName() );

  whileBlocking( mCrsSelector )->setCrs( mMapItem->presetCrs() );

  //width, height, scale
  const double scale = mMapItem->scale();

  //round scale to an appropriate number of decimal places
  if ( scale >= 10000 )
  {
    //round scale to integer if it's greater than 10000
    mScaleLineEdit->setText( QLocale().toString( mMapItem->scale(), 'f', 0 ) );
  }
  else if ( scale >= 10 )
  {
    mScaleLineEdit->setText( QLocale().toString( mMapItem->scale(), 'f', 3 ) );
  }
  else if ( scale >= 1 )
  {
    //don't round scale if it's less than 10, instead use 4 decimal places
    mScaleLineEdit->setText( QLocale().toString( mMapItem->scale(), 'f', 4 ) );
  }
  else
  {
    //if scale < 1 then use 10 decimal places
    mScaleLineEdit->setText( QLocale().toString( mMapItem->scale(), 'f', 10 ) );
  }

  //composer map extent
  const QgsRectangle composerMapExtent = mMapItem->extent();
  mXMinLineEdit->setText( QLocale().toString( composerMapExtent.xMinimum(), 'f', 3 ) );
  mXMaxLineEdit->setText( QLocale().toString( composerMapExtent.xMaximum(), 'f', 3 ) );
  mYMinLineEdit->setText( QLocale().toString( composerMapExtent.yMinimum(), 'f', 3 ) );
  mYMaxLineEdit->setText( QLocale().toString( composerMapExtent.yMaximum(), 'f', 3 ) );

  mMapRotationSpinBox->setValue( mMapItem->mapRotation( QgsLayoutObject::OriginalValue ) );

  // follow preset checkbox
  mFollowVisibilityPresetCheckBox->setCheckState(
    mMapItem->followVisibilityPreset() ? Qt::Checked : Qt::Unchecked );
  const int presetModelIndex = mFollowVisibilityPresetCombo->findText( mMapItem->followVisibilityPresetName() );
  mFollowVisibilityPresetCombo->setCurrentIndex( presetModelIndex != -1 ? presetModelIndex : 0 ); // 0 == none

  //keep layer list checkbox
  if ( mMapItem->keepLayerSet() )
  {
    mKeepLayerListCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mKeepLayerListCheckBox->setCheckState( Qt::Unchecked );
  }

  mKeepLayerStylesCheckBox->setEnabled( mMapItem->keepLayerSet() );
  mKeepLayerStylesCheckBox->setCheckState( mMapItem->keepLayerStyles() ? Qt::Checked : Qt::Unchecked );

  //draw canvas items
  if ( mMapItem->drawAnnotations() )
  {
    mDrawCanvasItemsCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mDrawCanvasItemsCheckBox->setCheckState( Qt::Unchecked );
  }

  //atlas controls
  mAtlasCheckBox->setChecked( mMapItem->atlasDriven() );
  mAtlasMarginSpinBox->setValue( static_cast<int>( mMapItem->atlasMargin( QgsLayoutObject::OriginalValue ) * 100 ) );

  mAtlasFixedScaleRadio->setEnabled( mMapItem->atlasDriven() );
  mAtlasFixedScaleRadio->setChecked( mMapItem->atlasScalingMode() == QgsLayoutItemMap::Fixed );
  mAtlasMarginSpinBox->setEnabled( mMapItem->atlasScalingMode() == QgsLayoutItemMap::Auto );
  mAtlasMarginRadio->setEnabled( mMapItem->atlasDriven() );
  mAtlasMarginRadio->setChecked( mMapItem->atlasScalingMode() == QgsLayoutItemMap::Auto );
  mAtlasPredefinedScaleRadio->setEnabled( mMapItem->atlasDriven() );
  mAtlasPredefinedScaleRadio->setChecked( mMapItem->atlasScalingMode() == QgsLayoutItemMap::Predefined );

  if ( mMapItem->atlasDriven() )
  {
    toggleAtlasScalingOptionsByLayerType();
  }
  // disable predefined scales if none are defined
  if ( !hasPredefinedScales() )
  {
    mAtlasPredefinedScaleRadio->setEnabled( false );
  }

  mTemporalCheckBox->setChecked( mMapItem->isTemporal() );
  mTemporalCheckBox->setCollapsed( !mMapItem->isTemporal() );
  mStartDateTime->setEnabled( mMapItem->isTemporal() );
  mEndDateTime->setEnabled( mMapItem->isTemporal() );
  if ( mMapItem->isTemporal() )
  {
    mStartDateTime->setDateTime( mMapItem->temporalRange().begin() );
    mEndDateTime->setDateTime( mMapItem->temporalRange().end() );
  }

  whileBlocking( mElevationRangeCheckBox )->setChecked( mMapItem->zRangeEnabled() );
  mElevationRangeCheckBox->setCollapsed( !mMapItem->zRangeEnabled() );
  if ( mMapItem->zRange().lower() != std::numeric_limits< double >::lowest() )
    whileBlocking( mZLowerSpin )->setValue( mMapItem->zRange().lower() );
  else
    whileBlocking( mZLowerSpin )->clear();
  if ( mMapItem->zRange().upper() != std::numeric_limits< double >::max() )
    whileBlocking( mZUpperSpin )->setValue( mMapItem->zRange().upper() );
  else
    whileBlocking( mZUpperSpin )->clear();

  populateDataDefinedButtons();
  loadGridEntries();
  loadOverviewEntries();

  blockAllSignals( false );
}

void QgsLayoutMapWidget::toggleAtlasScalingOptionsByLayerType()
{
  if ( !mMapItem )
  {
    return;
  }

  //get atlas coverage layer
  QgsVectorLayer *layer = coverageLayer();
  if ( !layer )
  {
    return;
  }

  if ( QgsWkbTypes::geometryType( layer->wkbType() ) == Qgis::GeometryType::Point )
  {
    //For point layers buffer setting makes no sense, so set "fixed scale" on and disable margin control
    if ( mMapItem->atlasScalingMode() == QgsLayoutItemMap::Auto )
      mAtlasFixedScaleRadio->setChecked( true );
    mAtlasMarginRadio->setEnabled( false );
  }
  else
  {
    //Not a point layer, so enable changes to fixed scale control
    mAtlasMarginRadio->setEnabled( true );
  }
}

void QgsLayoutMapWidget::updateComposerExtentFromGui()
{
  if ( !mMapItem )
  {
    return;
  }

  double xmin, ymin, xmax, ymax;
  bool conversionSuccess;

  xmin = QLocale().toDouble( mXMinLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
    return;
  xmax = QLocale().toDouble( mXMaxLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymin = QLocale().toDouble( mYMinLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymax = QLocale().toDouble( mYMaxLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
    return;

  const QgsRectangle newExtent( xmin, ymin, xmax, ymax );

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Extent" ) );
  mMapItem->setExtent( newExtent );
  mMapItem->layout()->undoStack()->endCommand();
}

void QgsLayoutMapWidget::blockAllSignals( bool b )
{
  mScaleLineEdit->blockSignals( b );
  mXMinLineEdit->blockSignals( b );
  mXMaxLineEdit->blockSignals( b );
  mYMinLineEdit->blockSignals( b );
  mYMaxLineEdit->blockSignals( b );
  mDrawCanvasItemsCheckBox->blockSignals( b );
  mOverviewFrameMapComboBox->blockSignals( b );
  mOverviewFrameStyleButton->blockSignals( b );
  mOverviewBlendModeComboBox->blockSignals( b );
  mOverviewInvertCheckbox->blockSignals( b );
  mOverviewCenterCheckbox->blockSignals( b );
  mAtlasCheckBox->blockSignals( b );
  mAtlasMarginSpinBox->blockSignals( b );
  mAtlasFixedScaleRadio->blockSignals( b );
  mAtlasMarginRadio->blockSignals( b );
  mFollowVisibilityPresetCheckBox->blockSignals( b );
  mFollowVisibilityPresetCombo->blockSignals( b );
  mKeepLayerListCheckBox->blockSignals( b );
  mKeepLayerStylesCheckBox->blockSignals( b );
  mActionSetToCanvasExtent->blockSignals( b );
  mActionUpdatePreview->blockSignals( b );
  mTemporalCheckBox->blockSignals( b );
  mStartDateTime->blockSignals( b );
  mEndDateTime->blockSignals( b );

  blockOverviewItemsSignals( b );
}

void QgsLayoutMapWidget::updatePreview()
{
  if ( !mMapItem )
  {
    return;
  }
  mMapItem->refresh();
}

void QgsLayoutMapWidget::mFollowVisibilityPresetCheckBox_stateChanged( int state )
{
  if ( !mMapItem )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Preset" ) );
  if ( state == Qt::Checked )
  {
    mMapItem->setFollowVisibilityPreset( true );

    // mutually exclusive with keeping custom layer list
    mKeepLayerListCheckBox->setCheckState( Qt::Unchecked );
    mKeepLayerStylesCheckBox->setCheckState( Qt::Unchecked );

    mMapItem->invalidateCache();
  }
  else
  {
    mMapItem->setFollowVisibilityPreset( false );
  }
  mMapItem->layout()->undoStack()->endCommand();
}

void QgsLayoutMapWidget::mKeepLayerListCheckBox_stateChanged( int state )
{
  if ( !mMapItem )
  {
    return;
  }

  // update map
  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Map Preset Changed" ) );
  storeCurrentLayerSet();
  mMapItem->setKeepLayerSet( state == Qt::Checked );
  if ( state == Qt::Unchecked )
  {
    mMapItem->setLayers( QList< QgsMapLayer * >() );
  }
  mMapItem->layout()->undoStack()->endCommand();

  // update gui
  if ( state == Qt::Checked )
  {
    // mutually exclusive with following a preset
    mFollowVisibilityPresetCheckBox->setCheckState( Qt::Unchecked );
  }
  else
  {
    mKeepLayerStylesCheckBox->setChecked( Qt::Unchecked );
    mMapItem->invalidateCache();
  }

  mKeepLayerStylesCheckBox->setEnabled( state == Qt::Checked );
}

void QgsLayoutMapWidget::mKeepLayerStylesCheckBox_stateChanged( int state )
{
  if ( !mMapItem )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Map Preset" ) );
  if ( state == Qt::Checked )
  {
    mMapItem->storeCurrentLayerStyles();
    mMapItem->setKeepLayerStyles( true );
  }
  else
  {
    mMapItem->setLayerStyleOverrides( QMap<QString, QString>() );
    mMapItem->setKeepLayerStyles( false );
  }
  mMapItem->layout()->undoStack()->endCommand();
}

void QgsLayoutMapWidget::mDrawCanvasItemsCheckBox_stateChanged( int state )
{
  if ( !mMapItem )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Toggle Map Item" ) );
  mMapItem->setDrawAnnotations( state == Qt::Checked );
  mMapItem->invalidateCache();
  mMapItem->layout()->undoStack()->endCommand();
}

void QgsLayoutMapWidget::atlasLayerChanged( QgsVectorLayer *layer )
{
  if ( !layer || layer->wkbType() == Qgis::WkbType::NoGeometry )
  {
    //geometryless layer, disable atlas control
    mAtlasCheckBox->setChecked( false );
    mAtlasCheckBox->setEnabled( false );
    return;
  }
  else
  {
    mAtlasCheckBox->setEnabled( true );
  }

  // enable or disable fixed scale control based on layer type
  if ( mAtlasCheckBox->isChecked() )
    toggleAtlasScalingOptionsByLayerType();
}

bool QgsLayoutMapWidget::hasPredefinedScales() const
{
  // first look at project's scales
  const QVector< double > scales( QgsProject::instance()->viewSettings()->mapScales() );
  const bool hasProjectScales( QgsProject::instance()->viewSettings()->useProjectScales() );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    const QgsSettings settings;
    const QStringList scales = QgsSettingsRegistryCore::settingsMapScales->value();
    return !scales.isEmpty() && !scales[0].isEmpty();
  }
  return true;
}

void QgsLayoutMapWidget::mAddGridPushButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }

  const QString itemName = tr( "Grid %1" ).arg( mMapItem->grids()->size() + 1 );
  QgsLayoutItemMapGrid *grid = new QgsLayoutItemMapGrid( itemName, mMapItem );
  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Add Map Grid" ) );
  mMapItem->grids()->addGrid( grid );
  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->updateBoundingRect();
  mMapItem->update();

  addGridListItem( grid->id(), grid->name() );
  mGridListWidget->setCurrentRow( 0 );
  mGridListWidget_currentItemChanged( mGridListWidget->currentItem(), nullptr );
}

void QgsLayoutMapWidget::mRemoveGridPushButton_clicked()
{
  QListWidgetItem *item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Remove Grid" ) );
  mMapItem->grids()->removeGrid( item->data( Qt::UserRole ).toString() );
  QListWidgetItem *delItem = mGridListWidget->takeItem( mGridListWidget->row( item ) );
  delete delItem;
  mMapItem->endCommand();
  mMapItem->updateBoundingRect();
  mMapItem->update();
}

void QgsLayoutMapWidget::mGridUpButton_clicked()
{
  QListWidgetItem *item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  const int row = mGridListWidget->row( item );
  if ( row < 1 )
  {
    return;
  }
  mGridListWidget->takeItem( row );
  mGridListWidget->insertItem( row - 1, item );
  mGridListWidget->setCurrentItem( item );
  mMapItem->beginCommand( tr( "Move Grid Up" ) );
  mMapItem->grids()->moveGridUp( item->data( Qt::UserRole ).toString() );
  mMapItem->endCommand();
  mMapItem->update();
}

void QgsLayoutMapWidget::mGridDownButton_clicked()
{
  QListWidgetItem *item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  const int row = mGridListWidget->row( item );
  if ( mGridListWidget->count() <= row )
  {
    return;
  }
  mGridListWidget->takeItem( row );
  mGridListWidget->insertItem( row + 1, item );
  mGridListWidget->setCurrentItem( item );
  mMapItem->beginCommand( tr( "Move Grid Down" ) );
  mMapItem->grids()->moveGridDown( item->data( Qt::UserRole ).toString() );
  mMapItem->endCommand();
  mMapItem->update();
}

QgsLayoutItemMapGrid *QgsLayoutMapWidget::currentGrid()
{
  if ( !mMapItem )
  {
    return nullptr;
  }

  QListWidgetItem *item = mGridListWidget->currentItem();
  if ( !item )
  {
    return nullptr;
  }
  return mMapItem->grids()->grid( item->data( Qt::UserRole ).toString() );
}

void QgsLayoutMapWidget::mGridListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem * )
{
  mGridPropertiesButton->setEnabled( static_cast< bool >( current ) );
}

void QgsLayoutMapWidget::mGridListWidget_itemChanged( QListWidgetItem *item )
{
  if ( !mMapItem )
  {
    return;
  }

  QgsLayoutItemMapGrid *grid = mMapItem->grids()->grid( item->data( Qt::UserRole ).toString() );
  if ( !grid )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Rename Grid" ) );
  grid->setName( item->text() );
  mMapItem->endCommand();
}

void QgsLayoutMapWidget::mGridPropertiesButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }
  QgsLayoutItemMapGrid *grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  QgsLayoutMapGridWidget *w = new QgsLayoutMapGridWidget( grid, mMapItem );
  w->setDesignerInterface( mInterface );
  openPanel( w );
}

QListWidgetItem *QgsLayoutMapWidget::addGridListItem( const QString &id, const QString &name )
{
  QListWidgetItem *item = new QListWidgetItem( name, nullptr );
  item->setData( Qt::UserRole, id );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mGridListWidget->insertItem( 0, item );
  return item;
}

void QgsLayoutMapWidget::loadGridEntries()
{
  //save selection
  QSet<QString> selectedIds;
  const QList<QListWidgetItem *> itemSelection = mGridListWidget->selectedItems();
  QList<QListWidgetItem *>::const_iterator sIt = itemSelection.constBegin();
  for ( ; sIt != itemSelection.constEnd(); ++sIt )
  {
    selectedIds.insert( ( *sIt )->data( Qt::UserRole ).toString() );
  }

  mGridListWidget->clear();
  if ( !mMapItem )
  {
    return;
  }
  //load all composer grids into list widget
  const QList< QgsLayoutItemMapGrid * > grids = mMapItem->grids()->asList();
  QList< QgsLayoutItemMapGrid * >::const_iterator gridIt = grids.constBegin();
  for ( ; gridIt != grids.constEnd(); ++gridIt )
  {
    QListWidgetItem *item = addGridListItem( ( *gridIt )->id(), ( *gridIt )->name() );
    if ( selectedIds.contains( ( *gridIt )->id() ) )
    {
      item->setSelected( true );
      mGridListWidget->setCurrentItem( item );
    }
  }

  if ( mGridListWidget->currentItem() )
  {
    mGridListWidget_currentItemChanged( mGridListWidget->currentItem(), nullptr );
  }
  else
  {
    mGridListWidget_currentItemChanged( nullptr, nullptr );
  }
}

void QgsLayoutMapWidget::mAddOverviewPushButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }

  const QString itemName = tr( "Overview %1" ).arg( mMapItem->overviews()->size() + 1 );
  QgsLayoutItemMapOverview *overview = new QgsLayoutItemMapOverview( itemName, mMapItem );
  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Add Map Overview" ) );
  mMapItem->overviews()->addOverview( overview );
  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->invalidateCache();

  addOverviewListItem( overview->id(), overview->name() );

  mOverviewListWidget->setCurrentRow( 0 );
}

void QgsLayoutMapWidget::mRemoveOverviewPushButton_clicked()
{
  QListWidgetItem *item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }
  mMapItem->beginCommand( tr( "Remove Map Overview" ) );
  mMapItem->overviews()->removeOverview( item->data( Qt::UserRole ).toString() );
  mMapItem->endCommand();
  QListWidgetItem *delItem = mOverviewListWidget->takeItem( mOverviewListWidget->row( item ) );
  delete delItem;
  mMapItem->invalidateCache();
}

void QgsLayoutMapWidget::mOverviewUpButton_clicked()
{
  QListWidgetItem *item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  const int row = mOverviewListWidget->row( item );
  if ( row < 1 )
  {
    return;
  }
  mOverviewListWidget->takeItem( row );
  mOverviewListWidget->insertItem( row - 1, item );
  mOverviewListWidget->setCurrentItem( item );
  mMapItem->beginCommand( tr( "Move Overview Up" ) );
  mMapItem->overviews()->moveOverviewUp( item->data( Qt::UserRole ).toString() );
  mMapItem->endCommand();
  mMapItem->invalidateCache();
}

void QgsLayoutMapWidget::mOverviewDownButton_clicked()
{
  QListWidgetItem *item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  const int row = mOverviewListWidget->row( item );
  if ( mOverviewListWidget->count() <= row )
  {
    return;
  }
  mOverviewListWidget->takeItem( row );
  mOverviewListWidget->insertItem( row + 1, item );
  mOverviewListWidget->setCurrentItem( item );
  mMapItem->beginCommand( tr( "Move Overview Down" ) );
  mMapItem->overviews()->moveOverviewDown( item->data( Qt::UserRole ).toString() );
  mMapItem->endCommand();
  mMapItem->invalidateCache();
}

QgsLayoutItemMapOverview *QgsLayoutMapWidget::currentOverview()
{
  if ( !mMapItem )
  {
    return nullptr;
  }

  QListWidgetItem *item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return nullptr;
  }

  return mMapItem->overviews()->overview( item->data( Qt::UserRole ).toString() );
}

void QgsLayoutMapWidget::mOverviewListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous )
{
  Q_UNUSED( previous )
  if ( !current )
  {
    mOverviewCheckBox->setEnabled( false );
    return;
  }

  mOverviewCheckBox->setEnabled( true );
  setOverviewItems( mMapItem->overviews()->overview( current->data( Qt::UserRole ).toString() ) );
}

void QgsLayoutMapWidget::mOverviewListWidget_itemChanged( QListWidgetItem *item )
{
  if ( !mMapItem )
  {
    return;
  }

  QgsLayoutItemMapOverview *overview = mMapItem->overviews()->overview( item->data( Qt::UserRole ).toString() );
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( QStringLiteral( "Rename Overview" ) );
  overview->setName( item->text() );
  mMapItem->endCommand();
  if ( item->isSelected() )
  {
    //update checkbox title if item is current item
    mOverviewCheckBox->setTitle( tr( "Draw \"%1\" overview" ).arg( overview->name() ) );
  }
}

void QgsLayoutMapWidget::setOverviewItemsEnabled( bool enabled )
{
  mOverviewFrameMapLabel->setEnabled( enabled );
  mOverviewFrameMapComboBox->setEnabled( enabled );
  mOverviewFrameStyleLabel->setEnabled( enabled );
  mOverviewFrameStyleButton->setEnabled( enabled );
  mOverviewBlendModeLabel->setEnabled( enabled );
  mOverviewBlendModeComboBox->setEnabled( enabled );
  mOverviewInvertCheckbox->setEnabled( enabled );
  mOverviewCenterCheckbox->setEnabled( enabled );
  mOverviewPositionComboBox->setEnabled( enabled );

  const QgsLayoutItemMapItem::StackingPosition currentStackingPos = static_cast< QgsLayoutItemMapItem::StackingPosition >( mOverviewPositionComboBox->currentData().toInt() );
  mOverviewStackingLayerComboBox->setEnabled( enabled && ( currentStackingPos == QgsLayoutItemMapItem::StackAboveMapLayer || currentStackingPos == QgsLayoutItemMapItem::StackBelowMapLayer ) );
}

void QgsLayoutMapWidget::blockOverviewItemsSignals( const bool block )
{
  mOverviewFrameMapComboBox->blockSignals( block );
  mOverviewFrameStyleButton->blockSignals( block );
  mOverviewBlendModeComboBox->blockSignals( block );
  mOverviewInvertCheckbox->blockSignals( block );
  mOverviewCenterCheckbox->blockSignals( block );
  mOverviewPositionComboBox->blockSignals( block );
  mOverviewStackingLayerComboBox->blockSignals( block );
}

void QgsLayoutMapWidget::setOverviewItems( QgsLayoutItemMapOverview *overview )
{
  if ( !overview )
  {
    return;
  }

  blockOverviewItemsSignals( true );

  mOverviewCheckBox->setTitle( tr( "Draw \"%1\" overview" ).arg( overview->name() ) );
  mOverviewCheckBox->setChecked( overview->enabled() );

  //overview frame
  mOverviewFrameMapComboBox->setItem( overview->linkedMap() );

  //overview frame blending mode
  mOverviewBlendModeComboBox->setBlendMode( overview->blendMode() );
  //overview inverted
  mOverviewInvertCheckbox->setChecked( overview->inverted() );
  //center overview
  mOverviewCenterCheckbox->setChecked( overview->centered() );

  mOverviewPositionComboBox->setCurrentIndex( mOverviewPositionComboBox->findData( overview->stackingPosition() ) );
  mOverviewStackingLayerComboBox->setLayer( overview->stackingLayer() );
  mOverviewStackingLayerComboBox->setEnabled( mOverviewPositionComboBox->isEnabled() && ( overview->stackingPosition() == QgsLayoutItemMapItem::StackAboveMapLayer
      || overview->stackingPosition() == QgsLayoutItemMapItem::StackBelowMapLayer ) );

  mOverviewFrameStyleButton->setSymbol( overview->frameSymbol()->clone() );

  blockOverviewItemsSignals( false );
}

void QgsLayoutMapWidget::storeCurrentLayerSet()
{
  if ( !mMapItem )
    return;

  QList<QgsMapLayer *> layers = mMapCanvas->mapSettings().layers();

  mMapItem->setLayers( layers );

  if ( mMapItem->keepLayerStyles() )
  {
    // also store styles associated with the layers
    mMapItem->storeCurrentLayerStyles();
  }
}

QList<QgsMapLayer *> QgsLayoutMapWidget::orderedPresetVisibleLayers( const QString &name ) const
{
  const QStringList visibleIds = QgsProject::instance()->mapThemeCollection()->mapThemeVisibleLayerIds( name );

  // also make sure to order the layers according to map canvas order
  QList<QgsMapLayer *> lst;
  const auto constLayerOrder = QgsProject::instance()->layerTreeRoot()->layerOrder();
  for ( QgsMapLayer *layer : constLayerOrder )
  {
    if ( visibleIds.contains( layer->id() ) )
    {
      lst << layer;
    }
  }
  return lst;
}

QListWidgetItem *QgsLayoutMapWidget::addOverviewListItem( const QString &id, const QString &name )
{
  QListWidgetItem *item = new QListWidgetItem( name, nullptr );
  item->setData( Qt::UserRole, id );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mOverviewListWidget->insertItem( 0, item );
  return item;
}

void QgsLayoutMapWidget::loadOverviewEntries()
{
  //save selection
  QSet<QString> selectedIds;
  const QList<QListWidgetItem *> itemSelection = mOverviewListWidget->selectedItems();
  QList<QListWidgetItem *>::const_iterator sIt = itemSelection.constBegin();
  for ( ; sIt != itemSelection.constEnd(); ++sIt )
  {
    selectedIds.insert( ( *sIt )->data( Qt::UserRole ).toString() );
  }

  mOverviewListWidget->clear();
  if ( !mMapItem )
  {
    return;
  }

  mOverviewFrameMapComboBox->setExceptedItemList( QList< QgsLayoutItem * >() << mMapItem );

  //load all composer overviews into list widget
  const QList< QgsLayoutItemMapOverview * > overviews = mMapItem->overviews()->asList();
  QList< QgsLayoutItemMapOverview * >::const_iterator overviewIt = overviews.constBegin();
  for ( ; overviewIt != overviews.constEnd(); ++overviewIt )
  {
    QListWidgetItem *item = addOverviewListItem( ( *overviewIt )->id(), ( *overviewIt )->name() );
    if ( selectedIds.contains( ( *overviewIt )->id() ) )
    {
      item->setSelected( true );
      mOverviewListWidget->setCurrentItem( item );
    }
  }

  if ( mOverviewListWidget->currentItem() )
  {
    mOverviewListWidget_currentItemChanged( mOverviewListWidget->currentItem(), nullptr );
  }
  else
  {
    mOverviewListWidget_currentItemChanged( nullptr, nullptr );
  }
}

void QgsLayoutMapWidget::mOverviewCheckBox_toggled( bool state )
{
  QgsLayoutItemMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Overview Display Toggled" ) );
  overview->setEnabled( state );
  mMapItem->invalidateCache();
  mMapItem->layout()->undoStack()->endCommand();
}

void QgsLayoutMapWidget::overviewMapChanged( QgsLayoutItem *item )
{
  QgsLayoutItemMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  QgsLayoutItemMap *map = dynamic_cast< QgsLayoutItemMap * >( item );
  if ( !map )
    return;

  mMapItem->beginCommand( tr( "Change Overview Map" ) );
  overview->setLinkedMap( map );
  mMapItem->invalidateCache();
  mMapItem->endCommand();
}

void QgsLayoutMapWidget::mOverviewBlendModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )
  QgsLayoutItemMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Change Overview Blend Mode" ) );
  overview->setBlendMode( mOverviewBlendModeComboBox->blendMode() );
  mMapItem->invalidateCache();
  mMapItem->endCommand();
}

void QgsLayoutMapWidget::mOverviewInvertCheckbox_toggled( bool state )
{
  QgsLayoutItemMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Toggle Overview Inverted" ) );
  overview->setInverted( state );
  mMapItem->invalidateCache();
  mMapItem->endCommand();
}

void QgsLayoutMapWidget::mOverviewCenterCheckbox_toggled( bool state )
{
  QgsLayoutItemMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Toggle Overview Centered" ) );
  overview->setCentered( state );
  mMapItem->invalidateCache();
  mMapItem->endCommand();
}

void QgsLayoutMapWidget::overviewStackingChanged( int )
{
  QgsLayoutItemMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Change Overview Position" ) );
  overview->setStackingPosition( static_cast< QgsLayoutItemMapItem::StackingPosition >( mOverviewPositionComboBox->currentData().toInt() ) );
  mMapItem->invalidateCache();
  mMapItem->endCommand();

  switch ( overview->stackingPosition() )
  {
    case QgsLayoutItemMapItem::StackAboveMapLabels:
    case QgsLayoutItemMapItem::StackBelowMap:
    case QgsLayoutItemMapItem::StackBelowMapLabels:
      mOverviewStackingLayerComboBox->setEnabled( false );
      break;

    case QgsLayoutItemMapItem::StackAboveMapLayer:
    case QgsLayoutItemMapItem::StackBelowMapLayer:
      mOverviewStackingLayerComboBox->setEnabled( true );
      break;
  }
}

void QgsLayoutMapWidget::overviewStackingLayerChanged( QgsMapLayer *layer )
{
  QgsLayoutItemMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Change Overview Position" ) );
  overview->setStackingLayer( layer );
  mMapItem->invalidateCache();
  mMapItem->endCommand();
}

//
// QgsLayoutMapLabelingWidget
//

QgsLayoutMapLabelingWidget::QgsLayoutMapLabelingWidget( QgsLayoutItemMap *map )
  : QgsLayoutItemBaseWidget( nullptr, map )
  , mMapItem( map )
{
  setupUi( this );
  setPanelTitle( tr( "Label Settings" ) );

  mLabelBoundarySpinBox->setClearValue( 0 );
  mLabelBoundarySpinBox->setShowClearButton( true );

  mLabelBoundaryUnitsCombo->linkToWidget( mLabelBoundarySpinBox );
  mLabelBoundaryUnitsCombo->setConverter( &mMapItem->layout()->renderContext().measurementConverter() );

  connect( mLabelBoundaryUnitsCombo, &QgsLayoutUnitsComboBox::unitChanged, this, &QgsLayoutMapLabelingWidget::labelMarginUnitsChanged );
  connect( mLabelBoundarySpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapLabelingWidget::labelMarginChanged );
  connect( mShowPartialLabelsCheckBox, &QCheckBox::toggled, this, &QgsLayoutMapLabelingWidget::showPartialsToggled );
  connect( mShowUnplacedCheckBox, &QCheckBox::toggled, this, &QgsLayoutMapLabelingWidget::showUnplacedToggled );

  registerDataDefinedButton( mLabelMarginDDBtn, QgsLayoutObject::DataDefinedProperty::MapLabelMargin );

  setNewItem( map );
}

bool QgsLayoutMapLabelingWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutMap )
    return false;

  if ( mMapItem )
  {
    disconnect( mMapItem, &QgsLayoutObject::changed, this, &QgsLayoutMapLabelingWidget::updateGuiElements );
  }

  mMapItem = qobject_cast< QgsLayoutItemMap * >( item );

  if ( mMapItem )
  {
    connect( mMapItem, &QgsLayoutObject::changed, this, &QgsLayoutMapLabelingWidget::updateGuiElements );
  }

  updateGuiElements();

  return true;
}

void QgsLayoutMapLabelingWidget::updateGuiElements()
{
  whileBlocking( mLabelBoundarySpinBox )->setValue( mMapItem->labelMargin().length() );
  whileBlocking( mLabelBoundaryUnitsCombo )->setUnit( mMapItem->labelMargin().units() );
  whileBlocking( mShowPartialLabelsCheckBox )->setChecked( mMapItem->mapFlags() & QgsLayoutItemMap::ShowPartialLabels );
  whileBlocking( mShowUnplacedCheckBox )->setChecked( mMapItem->mapFlags() & QgsLayoutItemMap::ShowUnplacedLabels );

  if ( mBlockingItemsListView->model() )
  {
    QAbstractItemModel *oldModel = mBlockingItemsListView->model();
    mBlockingItemsListView->setModel( nullptr );
    oldModel->deleteLater();
  }

  QgsLayoutMapItemBlocksLabelsModel *model = new QgsLayoutMapItemBlocksLabelsModel( mMapItem, mMapItem->layout()->itemsModel(), mBlockingItemsListView );
  mBlockingItemsListView->setModel( model );

  updateDataDefinedButton( mLabelMarginDDBtn );
}

void QgsLayoutMapLabelingWidget::labelMarginChanged( double val )
{
  if ( !mMapItem )
    return;

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Label Margin" ), QgsLayoutItem::UndoMapLabelMargin );
  mMapItem->setLabelMargin( QgsLayoutMeasurement( val, mLabelBoundaryUnitsCombo->unit() ) );
  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->invalidateCache();
}

void QgsLayoutMapLabelingWidget::labelMarginUnitsChanged()
{
  if ( !mMapItem )
    return;

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Label Margin" ), QgsLayoutItem::UndoMapLabelMargin );
  mMapItem->setLabelMargin( QgsLayoutMeasurement( mLabelBoundarySpinBox->value(), mLabelBoundaryUnitsCombo->unit() ) );
  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->invalidateCache();
}

void QgsLayoutMapLabelingWidget::showPartialsToggled( bool checked )
{
  if ( !mMapItem )
    return;

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Label Visibility" ) );
  QgsLayoutItemMap::MapItemFlags flags = mMapItem->mapFlags();
  if ( checked )
    flags |= QgsLayoutItemMap::ShowPartialLabels;
  else
    flags &= ~QgsLayoutItemMap::ShowPartialLabels;
  mMapItem->setMapFlags( flags );
  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->invalidateCache();
}

void QgsLayoutMapLabelingWidget::showUnplacedToggled( bool checked )
{
  if ( !mMapItem )
    return;

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Label Visibility" ) );
  QgsLayoutItemMap::MapItemFlags flags = mMapItem->mapFlags();
  if ( checked )
    flags |= QgsLayoutItemMap::ShowUnplacedLabels;
  else
    flags &= ~QgsLayoutItemMap::ShowUnplacedLabels;
  mMapItem->setMapFlags( flags );
  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->invalidateCache();
}

QgsLayoutMapItemBlocksLabelsModel::QgsLayoutMapItemBlocksLabelsModel( QgsLayoutItemMap *map, QgsLayoutModel *layoutModel, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mLayoutModel( layoutModel )
  , mMapItem( map )
{
  setSourceModel( layoutModel );
}

int QgsLayoutMapItemBlocksLabelsModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QVariant QgsLayoutMapItemBlocksLabelsModel::data( const QModelIndex &i, int role ) const
{
  if ( !i.isValid() )
    return QVariant();

  if ( i.column() != 0 )
    return QVariant();

  const QModelIndex sourceIndex = mapToSource( index( i.row(), QgsLayoutModel::ItemId, i.parent() ) );

  QgsLayoutItem *item = mLayoutModel->itemFromIndex( mapToSource( i ) );
  if ( !item )
  {
    return QVariant();
  }

  switch ( role )
  {
    case Qt::CheckStateRole:
      switch ( i.column() )
      {
        case 0:
          return mMapItem ? ( mMapItem->isLabelBlockingItem( item ) ? Qt::Checked : Qt::Unchecked ) : Qt::Unchecked;
        default:
          return QVariant();
      }

    default:
      return mLayoutModel->data( sourceIndex, role );
  }
}

bool QgsLayoutMapItemBlocksLabelsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( role )

  if ( !index.isValid() )
    return false;

  QgsLayoutItem *item = mLayoutModel->itemFromIndex( mapToSource( index ) );
  if ( !item || !mMapItem )
  {
    return false;
  }

  mMapItem->layout()->undoStack()->beginCommand( mMapItem, tr( "Change Label Blocking Items" ) );

  if ( value.toBool() )
  {
    mMapItem->addLabelBlockingItem( item );
  }
  else
  {
    mMapItem->removeLabelBlockingItem( item );
  }
  emit dataChanged( index, index, QVector<int>() << role );

  mMapItem->layout()->undoStack()->endCommand();
  mMapItem->invalidateCache();

  return true;
}

Qt::ItemFlags QgsLayoutMapItemBlocksLabelsModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QSortFilterProxyModel::flags( index );

  if ( ! index.isValid() )
  {
    return flags ;
  }

  switch ( index.column() )
  {
    case 0:
      return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    default:
      return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
}

bool QgsLayoutMapItemBlocksLabelsModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QgsLayoutItem *item = mLayoutModel->itemFromIndex( mLayoutModel->index( source_row, 0, source_parent ) );
  if ( !item || item == mMapItem )
  {
    return false;
  }

  return true;
}



//
// QgsLayoutMapClippingWidget
//

QgsLayoutMapClippingWidget::QgsLayoutMapClippingWidget( QgsLayoutItemMap *map )
  : QgsLayoutItemBaseWidget( nullptr, map )
  , mMapItem( map )
{
  setupUi( this );
  setPanelTitle( tr( "Clipping Settings" ) );

  mLayerModel = new QgsMapLayerModel( this );
  mLayerModel->setItemsCheckable( true );
  mLayersTreeView->setModel( mLayerModel );

  mAtlasClippingTypeComboBox->addItem( tr( "Clip During Render Only" ), static_cast< int >( QgsMapClippingRegion::FeatureClippingType::ClipPainterOnly ) );
  mAtlasClippingTypeComboBox->addItem( tr( "Clip Feature Before Render" ), static_cast< int >( QgsMapClippingRegion::FeatureClippingType::ClipToIntersection ) );
  mAtlasClippingTypeComboBox->addItem( tr( "Render Intersecting Features Unchanged" ), static_cast< int >( QgsMapClippingRegion::FeatureClippingType::NoClipping ) );

  for ( int i = 0; i < mAtlasClippingTypeComboBox->count(); ++i )
  {
    mItemClippingTypeComboBox->addItem( mAtlasClippingTypeComboBox->itemText( i ), mAtlasClippingTypeComboBox->itemData( i ) );
  }

  mClipItemComboBox->setCurrentLayout( map->layout() );
  mClipItemComboBox->setItemFlags( QgsLayoutItem::FlagProvidesClipPath );

  connect( mRadioClipSelectedLayers, &QRadioButton::toggled, this, &QgsLayoutMapClippingWidget::toggleLayersSelectionGui );
  mLayersTreeView->setEnabled( false );
  mSelectAllButton->setEnabled( false );
  mDeselectAllButton->setEnabled( false );
  mInvertSelectionButton->setEnabled( false );
  mRadioClipAllLayers->setChecked( true );

  connect( mClipToAtlasCheckBox, &QGroupBox::toggled, this, [ = ]( bool active )
  {
    if ( !mBlockUpdates )
    {
      mMapItem->beginCommand( tr( "Toggle Atlas Clipping" ) );
      mMapItem->atlasClippingSettings()->setEnabled( active );
      mMapItem->endCommand();
    }
  } );
  connect( mForceLabelsInsideCheckBox, &QCheckBox::toggled, this, [ = ]( bool active )
  {
    if ( !mBlockUpdates )
    {
      mMapItem->beginCommand( tr( "Change Atlas Clipping Label Behavior" ) );
      mMapItem->atlasClippingSettings()->setForceLabelsInsideFeature( active );
      mMapItem->endCommand();
    }
  } );
  connect( mAtlasClippingTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    if ( !mBlockUpdates )
    {
      mMapItem->beginCommand( tr( "Change Atlas Clipping Behavior" ) );
      mMapItem->atlasClippingSettings()->setFeatureClippingType( static_cast< QgsMapClippingRegion::FeatureClippingType >( mAtlasClippingTypeComboBox->currentData().toInt() ) );
      mMapItem->endCommand();
    }
  } );

  connect( mRadioClipSelectedLayers, &QCheckBox::toggled, this, [ = ]( bool active )
  {
    if ( active && !mBlockUpdates )
    {
      mBlockUpdates = true;
      mMapItem->beginCommand( tr( "Change Atlas Clipping Layers" ) );
      mMapItem->atlasClippingSettings()->setRestrictToLayers( true );
      mMapItem->endCommand();
      mBlockUpdates = false;
    }
  } );
  // layers selection buttons
  connect( mSelectAllButton, &QPushButton::clicked, this, &QgsLayoutMapClippingWidget::selectAll );
  connect( mDeselectAllButton, &QPushButton::clicked, this, &QgsLayoutMapClippingWidget::deselectAll );
  connect( mInvertSelectionButton, &QPushButton::clicked, this, &QgsLayoutMapClippingWidget::invertSelection );

  connect( mRadioClipAllLayers, &QCheckBox::toggled, this, [ = ]( bool active )
  {
    if ( active && !mBlockUpdates )
    {
      mBlockUpdates = true;
      mMapItem->beginCommand( tr( "Change Atlas Clipping Layers" ) );
      mMapItem->atlasClippingSettings()->setRestrictToLayers( false );
      mMapItem->endCommand();
      mBlockUpdates = false;
    }
  } );
  connect( mLayerModel, &QgsMapLayerModel::dataChanged, this, [ = ]( const QModelIndex &, const QModelIndex &, const QVector<int> &roles = QVector<int>() )
  {
    if ( !roles.contains( Qt::CheckStateRole ) )
      return;

    if ( !mBlockUpdates )
    {
      mBlockUpdates = true;
      mMapItem->beginCommand( tr( "Change Atlas Clipping Layers" ) );
      mMapItem->atlasClippingSettings()->setLayersToClip( mLayerModel->layersChecked() );
      mMapItem->endCommand();
      mBlockUpdates = false;
    }
  } );

  // item clipping widgets

  connect( mClipToItemCheckBox, &QGroupBox::toggled, this, [ = ]( bool active )
  {
    if ( !mBlockUpdates )
    {
      mMapItem->beginCommand( tr( "Toggle Map Clipping" ) );
      mMapItem->itemClippingSettings()->setEnabled( active );
      mMapItem->endCommand();
    }
  } );
  connect( mItemClippingTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    if ( !mBlockUpdates )
    {
      mMapItem->beginCommand( tr( "Change Map Clipping Behavior" ) );
      mMapItem->itemClippingSettings()->setFeatureClippingType( static_cast< QgsMapClippingRegion::FeatureClippingType >( mItemClippingTypeComboBox->currentData().toInt() ) );
      mMapItem->endCommand();
    }
  } );
  connect( mForceLabelsInsideItemCheckBox, &QCheckBox::toggled, this, [ = ]( bool active )
  {
    if ( !mBlockUpdates )
    {
      mMapItem->beginCommand( tr( "Change Map Clipping Label Behavior" ) );
      mMapItem->itemClippingSettings()->setForceLabelsInsideClipPath( active );
      mMapItem->endCommand();
    }
  } );
  connect( mClipItemComboBox, &QgsLayoutItemComboBox::itemChanged, this, [ = ]( QgsLayoutItem * item )
  {
    if ( !mBlockUpdates )
    {
      mMapItem->beginCommand( tr( "Change Map Clipping Item" ) );
      mMapItem->itemClippingSettings()->setSourceItem( item );
      mMapItem->endCommand();
    }
  } );

  setNewItem( map );

  connect( &map->layout()->reportContext(), &QgsLayoutReportContext::layerChanged,
           this, &QgsLayoutMapClippingWidget::atlasLayerChanged );
  if ( QgsLayoutAtlas *atlas = layoutAtlas() )
  {
    connect( atlas, &QgsLayoutAtlas::toggled, this, &QgsLayoutMapClippingWidget::atlasToggled );
    atlasToggled( atlas->enabled() );
  }
}

void QgsLayoutMapClippingWidget::setReportTypeString( const QString &string )
{
  mClipToAtlasCheckBox->setTitle( tr( "Clip to %1 feature" ).arg( string ) );
  mClipToAtlasLabel->setText( tr( "<b>When enabled, map layers will be automatically clipped to the boundary of the current %1 feature.</b>" ).arg( string ) );
  mForceLabelsInsideCheckBox->setText( tr( "Force labels inside %1 feature" ).arg( string ) );
}

bool QgsLayoutMapClippingWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutMap )
    return false;

  if ( mMapItem )
  {
    disconnect( mMapItem, &QgsLayoutObject::changed, this, &QgsLayoutMapClippingWidget::updateGuiElements );
  }

  mMapItem = qobject_cast< QgsLayoutItemMap * >( item );

  if ( mMapItem )
  {
    connect( mMapItem, &QgsLayoutObject::changed, this, &QgsLayoutMapClippingWidget::updateGuiElements );
  }

  updateGuiElements();

  return true;
}

void QgsLayoutMapClippingWidget::updateGuiElements()
{
  if ( mBlockUpdates )
    return;

  mBlockUpdates = true;
  mClipToAtlasCheckBox->setChecked( mMapItem->atlasClippingSettings()->enabled() );
  mAtlasClippingTypeComboBox->setCurrentIndex( mAtlasClippingTypeComboBox->findData( static_cast< int >( mMapItem->atlasClippingSettings()->featureClippingType() ) ) );
  mForceLabelsInsideCheckBox->setChecked( mMapItem->atlasClippingSettings()->forceLabelsInsideFeature() );

  mRadioClipAllLayers->setChecked( !mMapItem->atlasClippingSettings()->restrictToLayers() );
  mRadioClipSelectedLayers->setChecked( mMapItem->atlasClippingSettings()->restrictToLayers() );
  mLayerModel->setLayersChecked( mMapItem->atlasClippingSettings()->layersToClip() );

  mClipToItemCheckBox->setChecked( mMapItem->itemClippingSettings()->enabled() );
  mItemClippingTypeComboBox->setCurrentIndex( mItemClippingTypeComboBox->findData( static_cast< int >( mMapItem->itemClippingSettings()->featureClippingType() ) ) );
  mForceLabelsInsideItemCheckBox->setChecked( mMapItem->itemClippingSettings()->forceLabelsInsideClipPath() );
  mClipItemComboBox->setItem( mMapItem->itemClippingSettings()->sourceItem() );

  mBlockUpdates = false;
}

void QgsLayoutMapClippingWidget::atlasLayerChanged( QgsVectorLayer *layer )
{
  if ( !layer || layer->geometryType() != Qgis::GeometryType::Polygon )
  {
    //non-polygon layer, disable atlas control
    mClipToAtlasCheckBox->setChecked( false );
    mClipToAtlasCheckBox->setEnabled( false );
    return;
  }
  else
  {
    mClipToAtlasCheckBox->setEnabled( true );
  }
}

void QgsLayoutMapClippingWidget::atlasToggled( bool atlasEnabled )
{
  if ( atlasEnabled &&
       mMapItem && mMapItem->layout() && mMapItem->layout()->reportContext().layer()
       && mMapItem->layout()->reportContext().layer()->geometryType() == Qgis::GeometryType::Polygon )
  {
    mClipToAtlasCheckBox->setEnabled( true );
  }
  else
  {
    mClipToAtlasCheckBox->setEnabled( false );
    mClipToAtlasCheckBox->setChecked( false );
  }
}

void QgsLayoutMapClippingWidget::invertSelection()
{
  for ( int i = 0; i < mLayerModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mLayerModel->index( i, 0 );
    Qt::CheckState currentState = Qt::CheckState( mLayerModel->data( index, Qt::CheckStateRole ).toInt() );
    Qt::CheckState newState = ( currentState == Qt::Checked ) ? Qt::Unchecked : Qt::Checked;
    mLayerModel->setData( index, newState, Qt::CheckStateRole );
  }
}

void QgsLayoutMapClippingWidget::selectAll()
{
  for ( int i = 0; i < mLayerModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mLayerModel->index( i, 0 );
    mLayerModel->setData( index, Qt::Checked, Qt::CheckStateRole );
  }
}

void QgsLayoutMapClippingWidget::deselectAll()
{
  for ( int i = 0; i < mLayerModel->rowCount( QModelIndex() ); i++ )
  {
    QModelIndex index = mLayerModel->index( i, 0 );
    mLayerModel->setData( index, Qt::Unchecked, Qt::CheckStateRole );
  }
}

void QgsLayoutMapClippingWidget::toggleLayersSelectionGui( bool toggled )
{
  mLayersTreeView->setEnabled( toggled );
  mSelectAllButton->setEnabled( toggled );
  mDeselectAllButton->setEnabled( toggled );
  mInvertSelectionButton->setEnabled( toggled );
}
