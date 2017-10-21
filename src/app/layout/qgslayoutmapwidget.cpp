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
#include "qgslayoutitemmap.h"
#include "qgsproject.h"
#include "qgsmapthemecollection.h"
#include "qgsmapthemes.h"
#include "qgslayout.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"

#include <QMenu>
#include <QMessageBox>

QgsLayoutMapWidget::QgsLayoutMapWidget( QgsLayoutItemMap *item )
  : QgsLayoutItemBaseWidget( nullptr, item )
  , mMapItem( item )
{
  setupUi( this );
  connect( mScaleLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutMapWidget::mScaleLineEdit_editingFinished );
  connect( mSetToMapCanvasExtentButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mSetToMapCanvasExtentButton_clicked );
  connect( mViewExtentInCanvasButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mViewExtentInCanvasButton_clicked );
  connect( mUpdatePreviewButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mUpdatePreviewButton_clicked );
  connect( mFollowVisibilityPresetCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMapWidget::mFollowVisibilityPresetCheckBox_stateChanged );
  connect( mKeepLayerListCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMapWidget::mKeepLayerListCheckBox_stateChanged );
  connect( mKeepLayerStylesCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMapWidget::mKeepLayerStylesCheckBox_stateChanged );
  connect( mDrawCanvasItemsCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMapWidget::mDrawCanvasItemsCheckBox_stateChanged );
  connect( mOverviewFrameStyleButton, &QPushButton::clicked, this, &QgsLayoutMapWidget::mOverviewFrameStyleButton_clicked );
  connect( mOverviewBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapWidget::mOverviewBlendModeComboBox_currentIndexChanged );
  connect( mOverviewInvertCheckbox, &QCheckBox::toggled, this, &QgsLayoutMapWidget::mOverviewInvertCheckbox_toggled );
  connect( mOverviewCenterCheckbox, &QCheckBox::toggled, this, &QgsLayoutMapWidget::mOverviewCenterCheckbox_toggled );
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
  connect( mDrawGridCheckBox, &QCheckBox::toggled, this, &QgsLayoutMapWidget::mDrawGridCheckBox_toggled );
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
  setPanelTitle( tr( "Map properties" ) );
  mMapRotationSpinBox->setClearValue( 0 );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, item );
  mainLayout->addWidget( mItemPropertiesWidget );

  mScaleLineEdit->setValidator( new QDoubleValidator( mScaleLineEdit ) );

  mXMinLineEdit->setValidator( new QDoubleValidator( mXMinLineEdit ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( mXMaxLineEdit ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( mYMinLineEdit ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( mYMaxLineEdit ) );

  blockAllSignals( true );

  mCrsSelector->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  mCrsSelector->setNotSetText( tr( "Use project CRS" ) );

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

  if ( item )
  {
    mLabel->setText( tr( "Map %1" ).arg( item->id() ) );
    connect( item, &QgsLayoutObject::changed, this, &QgsLayoutMapWidget::updateGuiElements );

#if 0 //TODO
    QgsAtlasComposition *atlas = atlasComposition();
    if ( atlas )
    {
      connect( atlas, &QgsAtlasComposition::coverageLayerChanged,
               this, &QgsLayoutMapWidget::atlasLayerChanged );
      connect( atlas, &QgsAtlasComposition::toggled, this, &QgsLayoutMapWidget::compositionAtlasToggled );

      compositionAtlasToggled( atlas->enabled() );
    }
    mOverviewFrameMapComboBox->setComposition( composerMap->composition() );
    mOverviewFrameMapComboBox->setItemType( QgsComposerItem::ComposerMap );
    mOverviewFrameMapComboBox->setExceptedItemList( QList< QgsComposerItem * >() << composerMap );
    connect( mOverviewFrameMapComboBox, &QgsComposerItemComboBox::itemChanged, this, &QgsLayoutMapWidget::overviewMapChanged );
#endif
  }

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsLayoutMapWidget::mapCrsChanged );

  registerDataDefinedButton( mScaleDDBtn, QgsLayoutObject::MapScale );
  registerDataDefinedButton( mMapRotationDDBtn, QgsLayoutObject::MapRotation );
  registerDataDefinedButton( mXMinDDBtn, QgsLayoutObject::MapXMin );
  registerDataDefinedButton( mYMinDDBtn, QgsLayoutObject::MapYMin );
  registerDataDefinedButton( mXMaxDDBtn, QgsLayoutObject::MapXMax );
  registerDataDefinedButton( mYMaxDDBtn, QgsLayoutObject::MapYMax );
  registerDataDefinedButton( mAtlasMarginDDBtn, QgsLayoutObject::MapAtlasMargin );
  registerDataDefinedButton( mStylePresetsDDBtn, QgsLayoutObject::MapStylePreset );
  registerDataDefinedButton( mLayersDDBtn, QgsLayoutObject::MapLayers );

  updateGuiElements();
  loadGridEntries();
  loadOverviewEntries();

  connect( mMapRotationSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutMapWidget::rotationChanged );

  blockAllSignals( false );
}

bool QgsLayoutMapWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutMap )
    return false;

  mMapItem = qobject_cast< QgsLayoutItemMap * >( item );
  mItemPropertiesWidget->setItem( mMapItem );

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
}

void QgsLayoutMapWidget::compositionAtlasToggled( bool atlasEnabled )
{
  Q_UNUSED( atlasEnabled );
#if 0 //TODO
  if ( atlasEnabled &&
       mMapItem && mMapItem->composition() && mMapItem->composition()->atlasComposition().coverageLayer()
       && mMapItem->composition()->atlasComposition().coverageLayer()->wkbType() != QgsWkbTypes::NoGeometry )
  {
    mAtlasCheckBox->setEnabled( true );
  }
  else
  {
    mAtlasCheckBox->setEnabled( false );
    mAtlasCheckBox->setChecked( false );
  }
#endif
}

void QgsLayoutMapWidget::aboutToShowKeepLayersVisibilityPresetsMenu()
{
  // this menu is for the case when setting "keep layers" and "keep layer styles"
  // and the preset configuration is copied. The preset is not followed further.

  QMenu *menu = qobject_cast<QMenu *>( sender() );
  if ( !menu )
    return;

  menu->clear();
  Q_FOREACH ( const QString &presetName, QgsProject::instance()->mapThemeCollection()->mapThemes() )
  {
    menu->addAction( presetName, this, SLOT( keepLayersVisibilityPresetSelected() ) );
  }

  if ( menu->actions().isEmpty() )
    menu->addAction( tr( "No presets defined" ) )->setEnabled( false );
}

void QgsLayoutMapWidget::followVisibilityPresetSelected( int currentIndex )
{
  if ( !mMapItem )
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

  QString presetName = action->text();
  QList<QgsMapLayer *> lst = QgsMapThemes::instance()->orderedPresetVisibleLayers( presetName );
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
    QStringList lst;
    lst.append( tr( "(none)" ) );
    lst += QgsProject::instance()->mapThemeCollection()->mapThemes();
    model->setStringList( lst );

    // select the previously selected item again
    int presetModelIndex = mFollowVisibilityPresetCombo->findText( mMapItem->followVisibilityPresetName() );
    mFollowVisibilityPresetCombo->blockSignals( true );
    mFollowVisibilityPresetCombo->setCurrentIndex( presetModelIndex != -1 ? presetModelIndex : 0 ); // 0 == none
    mFollowVisibilityPresetCombo->blockSignals( false );
  }
}

void QgsLayoutMapWidget::updateOverviewFrameStyleFromWidget()
{
#if 0 //TODO
  QgsComposerMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( sender() );
  overview->setFrameSymbol( dynamic_cast< QgsFillSymbol * >( w->symbol()->clone() ) );
  mMapItem->update();
#endif
}

void QgsLayoutMapWidget::cleanUpOverviewFrameStyleSelector( QgsPanelWidget *container )
{
  Q_UNUSED( container );
#if 0 //TODO
  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( container );
  if ( !w )
    return;

  delete w->symbol();

  QgsComposerMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  updateOverviewFrameSymbolMarker( overview );
  mMapItem->endCommand();
#endif
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
  QgsCoordinateReferenceSystem oldCrs = mMapItem->crs();

  bool updateExtent = false;
  QgsRectangle newExtent;
  try
  {
    QgsCoordinateTransform xForm( oldCrs, crs.isValid() ? crs : QgsProject::instance()->crs() );
    QgsRectangle prevExtent = mMapItem->extent();
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
#if 0 //TODO
  //update map if in atlas preview mode
  QgsComposition *composition = mMapItem->composition();
  if ( !composition )
  {
    return;
  }
  if ( composition->atlasMode() == QgsComposition::AtlasOff )
  {
    return;
  }

  if ( mMapItem->atlasDriven() )
  {
    //update atlas based extent for map
    QgsAtlasComposition *atlas = &composition->atlasComposition();
    //prepareMap causes a redraw
    atlas->prepareMap( mMapItem );
  }
  else
  {
    //redraw map
    mMapItem->invalidateCache();
  }
#endif
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

  bool conversionSuccess;
  double scaleDenominator = mScaleLineEdit->text().toDouble( &conversionSuccess );

  if ( !conversionSuccess )
  {
    return;
  }

  if ( std::round( scaleDenominator ) == std::round( mMapItem->scale() ) )
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

void QgsLayoutMapWidget::mSetToMapCanvasExtentButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }

  QgsRectangle newExtent = QgisApp::instance()->mapCanvas()->mapSettings().visibleExtent();

  //transform?
  if ( QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs()
       != mMapItem->crs() )
  {
    try
    {
      QgsCoordinateTransform xForm( QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs(),
                                    mMapItem->crs() );
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

void QgsLayoutMapWidget::mViewExtentInCanvasButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }

  QgsRectangle currentMapExtent = mMapItem->extent();

  if ( !currentMapExtent.isEmpty() )
  {
    //transform?
    if ( QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs()
         != mMapItem->crs() )
    {
      try
      {
        QgsCoordinateTransform xForm( mMapItem->crs(),
                                      QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs() );
        currentMapExtent = xForm.transformBoundingBox( currentMapExtent );
      }
      catch ( QgsCsException & )
      {
        //transform failed, better not proceed
        return;
      }
    }

    QgisApp::instance()->mapCanvas()->setExtent( currentMapExtent );
    QgisApp::instance()->mapCanvas()->refresh();
  }
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

  whileBlocking( mCrsSelector )->setCrs( mMapItem->presetCrs() );

  //width, height, scale
  double scale = mMapItem->scale();

  //round scale to an appropriate number of decimal places
  if ( scale >= 10 )
  {
    //round scale to integer if it's greater than 10
    mScaleLineEdit->setText( QString::number( mMapItem->scale(), 'f', 0 ) );
  }
  else if ( scale >= 1 )
  {
    //don't round scale if it's less than 10, instead use 4 decimal places
    mScaleLineEdit->setText( QString::number( mMapItem->scale(), 'f', 4 ) );
  }
  else
  {
    //if scale < 1 then use 10 decimal places
    mScaleLineEdit->setText( QString::number( mMapItem->scale(), 'f', 10 ) );
  }

  //composer map extent
  QgsRectangle composerMapExtent = mMapItem->extent();
  mXMinLineEdit->setText( QString::number( composerMapExtent.xMinimum(), 'f', 3 ) );
  mXMaxLineEdit->setText( QString::number( composerMapExtent.xMaximum(), 'f', 3 ) );
  mYMinLineEdit->setText( QString::number( composerMapExtent.yMinimum(), 'f', 3 ) );
  mYMaxLineEdit->setText( QString::number( composerMapExtent.yMaximum(), 'f', 3 ) );

  mMapRotationSpinBox->setValue( mMapItem->mapRotation( QgsLayoutObject::OriginalValue ) );

  // follow preset checkbox
  mFollowVisibilityPresetCheckBox->setCheckState(
    mMapItem->followVisibilityPreset() ? Qt::Checked : Qt::Unchecked );
  int presetModelIndex = mFollowVisibilityPresetCombo->findText( mMapItem->followVisibilityPresetName() );
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

#if 0 //TODO
  //get atlas coverage layer
  QgsVectorLayer *coverageLayer = atlasCoverageLayer();
  if ( !coverageLayer )
  {
    return;
  }

  switch ( coverageLayer->wkbType() )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::Point25D:
    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiPoint25D:
      //For point layers buffer setting makes no sense, so set "fixed scale" on and disable margin control
      mAtlasFixedScaleRadio->setChecked( true );
      mAtlasMarginRadio->setEnabled( false );
      mAtlasPredefinedScaleRadio->setEnabled( false );
      break;
    default:
      //Not a point layer, so enable changes to fixed scale control
      mAtlasMarginRadio->setEnabled( true );
      mAtlasPredefinedScaleRadio->setEnabled( true );
  }
#endif
}

void QgsLayoutMapWidget::updateComposerExtentFromGui()
{
  if ( !mMapItem )
  {
    return;
  }

  double xmin, ymin, xmax, ymax;
  bool conversionSuccess;

  xmin = QLocale::system().toDouble( mXMinLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
    return;
  xmax = QLocale::system().toDouble( mXMaxLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymin = QLocale::system().toDouble( mYMinLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymax = QLocale::system().toDouble( mYMaxLineEdit->text(), &conversionSuccess );
  if ( !conversionSuccess )
    return;

  QgsRectangle newExtent( xmin, ymin, xmax, ymax );

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
  mSetToMapCanvasExtentButton->blockSignals( b );
  mUpdatePreviewButton->blockSignals( b );

  blockOverviewItemsSignals( b );
}

#if 0 //TODO
void QgsLayoutMapWidget::handleChangedFrameDisplay( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::DisplayMode mode )
{
  QgsComposerMapGrid *grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Frame divisions changed" ) );
  grid->setFrameDivisions( mode, border );
  mMapItem->endCommand();
  mMapItem->updateBoundingRect();
}

void QgsLayoutMapWidget::handleChangedAnnotationDisplay( QgsComposerMapGrid::BorderSide border, const QString &text )
{
  QgsComposerMapGrid *grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Annotation display changed" ) );
  if ( text == tr( "Show all" ) )
  {
    grid->setAnnotationDisplay( QgsComposerMapGrid::ShowAll, border );
  }
  else if ( text == tr( "Show latitude only" ) )
  {
    grid->setAnnotationDisplay( QgsComposerMapGrid::LatitudeOnly, border );
  }
  else if ( text == tr( "Show longitude only" ) )
  {
    grid->setAnnotationDisplay( QgsComposerMapGrid::LongitudeOnly, border );
  }
  else //disabled
  {
    grid->setAnnotationDisplay( QgsComposerMapGrid::HideAll, border );
  }

  mMapItem->updateBoundingRect();
  mMapItem->update();
  mMapItem->endCommand();
}
#endif

void QgsLayoutMapWidget::mUpdatePreviewButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }
  mMapItem->invalidateCache();
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

void QgsLayoutMapWidget::insertAnnotationPositionEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Inside frame" ) );
  c->insertItem( 1, tr( "Outside frame" ) );
}

void QgsLayoutMapWidget::insertAnnotationDirectionEntries( QComboBox *c )
{
#if 0 //TODO
  c->addItem( tr( "Horizontal" ), QgsComposerMapGrid::Horizontal );
  c->addItem( tr( "Vertical ascending" ), QgsComposerMapGrid::Vertical );
  c->addItem( tr( "Vertical descending" ), QgsComposerMapGrid::VerticalDescending );
#endif
}
#if 0 //TODO
void QgsLayoutMapWidget::initFrameDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( display ) );
}

void QgsLayoutMapWidget::initAnnotationDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }

  if ( display == QgsComposerMapGrid::ShowAll )
  {
    c->setCurrentIndex( c->findText( tr( "Show all" ) ) );
  }
  else if ( display == QgsComposerMapGrid::LatitudeOnly )
  {
    c->setCurrentIndex( c->findText( tr( "Show latitude only" ) ) );
  }
  else if ( display == QgsComposerMapGrid::LongitudeOnly )
  {
    c->setCurrentIndex( c->findText( tr( "Show longitude only" ) ) );
  }
  else
  {
    c->setCurrentIndex( c->findText( tr( "Disabled" ) ) );
  }
}

void QgsLayoutMapWidget::handleChangedAnnotationPosition( QgsComposerMapGrid::BorderSide border, const QString &text )
{
  QgsComposerMapGrid *grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Annotation position changed" ) );
  if ( text == tr( "Inside frame" ) )
  {
    grid->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, border );
  }
  else //Outside frame
  {
    grid->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, border );
  }

  mMapItem->updateBoundingRect();
  mMapItem->update();
  mMapItem->endCommand();
}

void QgsLayoutMapWidget::handleChangedAnnotationDirection( QgsComposerMapGrid::BorderSide border, QgsComposerMapGrid::AnnotationDirection direction )
{
  QgsComposerMapGrid *grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Changed annotation direction" ) );
  grid->setAnnotationDirection( direction, border );
  mMapItem->updateBoundingRect();
  mMapItem->update();
  mMapItem->endCommand();
}

#endif
void QgsLayoutMapWidget::insertFrameDisplayEntries( QComboBox *c )
{
#if 0 //TODO
  c->addItem( tr( "All" ), QgsComposerMapGrid::ShowAll );
  c->addItem( tr( "Latitude/Y only" ), QgsComposerMapGrid::LatitudeOnly );
  c->addItem( tr( "Longitude/X only" ), QgsComposerMapGrid::LongitudeOnly );
#endif
}

void QgsLayoutMapWidget::insertAnnotationDisplayEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Show all" ) );
  c->insertItem( 1, tr( "Show latitude only" ) );
  c->insertItem( 2, tr( "Show longitude only" ) );
  c->insertItem( 3, tr( "Disabled" ) );
}

#if 0 //TODO
void QgsLayoutMapWidget::initAnnotationPositionBox( QComboBox *c, QgsComposerMapGrid::AnnotationPosition pos )
{
  if ( !c )
  {
    return;
  }

  if ( pos == QgsComposerMapGrid::InsideMapFrame )
  {
    c->setCurrentIndex( c->findText( tr( "Inside frame" ) ) );
  }
  else
  {
    c->setCurrentIndex( c->findText( tr( "Outside frame" ) ) );
  }
}

void QgsLayoutMapWidget::initAnnotationDirectionBox( QComboBox *c, QgsComposerMapGrid::AnnotationDirection dir )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( dir ) );
}

#endif

void QgsLayoutMapWidget::atlasLayerChanged( QgsVectorLayer *layer )
{
#if 0 //TODO
  if ( !layer || layer->wkbType() == QgsWkbTypes::NoGeometry )
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
#endif
}

bool QgsLayoutMapWidget::hasPredefinedScales() const
{
  // first look at project's scales
  QStringList scales( QgsProject::instance()->readListEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ) ) );
  bool hasProjectScales( QgsProject::instance()->readBoolEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ) ) );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    QgsSettings settings;
    QString scalesStr( settings.value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString() );
    QStringList myScalesList = scalesStr.split( ',' );
    return !myScalesList.isEmpty() && !myScalesList[0].isEmpty();
  }
  return true;
}

void QgsLayoutMapWidget::mAddGridPushButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }
#if 0 //TODO
  QString itemName = tr( "Grid %1" ).arg( mMapItem->grids()->size() + 1 );
  QgsComposerMapGrid *grid = new QgsComposerMapGrid( itemName, mMapItem );
  mMapItem->beginCommand( tr( "Add map grid" ) );
  mMapItem->grids()->addGrid( grid );
  mMapItem->endCommand();
  mMapItem->updateBoundingRect();
  mMapItem->update();

  addGridListItem( grid->id(), grid->name() );
  mGridListWidget->setCurrentRow( 0 );
  mGridListWidget_currentItemChanged( mGridListWidget->currentItem(), nullptr );
#endif
}

void QgsLayoutMapWidget::mRemoveGridPushButton_clicked()
{
  QListWidgetItem *item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

#if 0 //TODO
  mMapItem->grids()->removeGrid( item->data( Qt::UserRole ).toString() );
  QListWidgetItem *delItem = mGridListWidget->takeItem( mGridListWidget->row( item ) );
  delete delItem;
  mMapItem->updateBoundingRect();
  mMapItem->update();
#endif
}

void QgsLayoutMapWidget::mGridUpButton_clicked()
{
  QListWidgetItem *item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  int row = mGridListWidget->row( item );
  if ( row < 1 )
  {
    return;
  }
  mGridListWidget->takeItem( row );
  mGridListWidget->insertItem( row - 1, item );
  mGridListWidget->setCurrentItem( item );
#if 0 //TODO
  mMapItem->grids()->moveGridUp( item->data( Qt::UserRole ).toString() );
#endif
  mMapItem->update();
}

void QgsLayoutMapWidget::mGridDownButton_clicked()
{
  QListWidgetItem *item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  int row = mGridListWidget->row( item );
  if ( mGridListWidget->count() <= row )
  {
    return;
  }
  mGridListWidget->takeItem( row );
  mGridListWidget->insertItem( row + 1, item );
  mGridListWidget->setCurrentItem( item );
#if 0 //TODO
  mMapItem->grids()->moveGridDown( item->data( Qt::UserRole ).toString() );
#endif
  mMapItem->update();
}

QgsLayoutMapGrid *QgsLayoutMapWidget::currentGrid()
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
#if 0 //TODO
  return mMapItem->grids()->grid( item->data( Qt::UserRole ).toString() );
#endif
  return nullptr;
}

void QgsLayoutMapWidget::mGridListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous )
{
  Q_UNUSED( previous );
  if ( !current )
  {
    mDrawGridCheckBox->setEnabled( false );
    mDrawGridCheckBox->setChecked( false );
    mGridPropertiesButton->setEnabled( false );
    return;
  }

#if 0 //TODO
  mDrawGridCheckBox->setEnabled( true );
  mDrawGridCheckBox->setChecked( currentGrid()->enabled() );
  mGridPropertiesButton->setEnabled( currentGrid()->enabled() );

  mDrawGridCheckBox->setText( QString( tr( "Draw \"%1\" grid" ) ).arg( currentGrid()->name() ) );
#endif
}

void QgsLayoutMapWidget::mGridListWidget_itemChanged( QListWidgetItem *item )
{
  if ( !mMapItem )
  {
    return;
  }

#if 0 //TODO
  QgsComposerMapGrid *grid = mMapItem->grids()->grid( item->data( Qt::UserRole ).toString() );
  if ( !grid )
  {
    return;
  }

  grid->setName( item->text() );
  if ( item->isSelected() )
  {
    //update checkbox title if item is current item
    mDrawGridCheckBox->setText( QString( tr( "Draw \"%1\" grid" ) ).arg( grid->name() ) );
  }
#endif
}

void QgsLayoutMapWidget::mGridPropertiesButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }
#if 0 //TODO
  QgsComposerMapGrid *grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  QgsComposerMapGridWidget *w = new QgsComposerMapGridWidget( grid, mMapItem );
  openPanel( w );
#endif
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
  QList<QListWidgetItem *> itemSelection = mGridListWidget->selectedItems();
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
#if 0 //TODO
  //load all composer grids into list widget
  QList< QgsComposerMapGrid * > grids = mMapItem->grids()->asList();
  QList< QgsComposerMapGrid * >::const_iterator gridIt = grids.constBegin();
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
#endif
}

void QgsLayoutMapWidget::mDrawGridCheckBox_toggled( bool state )
{
#if 0 //TODO
  QgsComposerMapGrid *grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mGridPropertiesButton->setEnabled( state );

  mMapItem->beginCommand( tr( "Grid checkbox toggled" ) );
  if ( state )
  {
    grid->setEnabled( true );
  }
  else
  {
    grid->setEnabled( false );
  }
  mMapItem->updateBoundingRect();
  mMapItem->update();
  mMapItem->endCommand();
#endif
}

void QgsLayoutMapWidget::mAddOverviewPushButton_clicked()
{
  if ( !mMapItem )
  {
    return;
  }

#if 0 //TODO
  QString itemName = tr( "Overview %1" ).arg( mMapItem->overviews()->size() + 1 );
  QgsComposerMapOverview *overview = new QgsComposerMapOverview( itemName, mMapItem );
  mMapItem->beginCommand( tr( "Add map overview" ) );
  mMapItem->overviews()->addOverview( overview );
  mMapItem->endCommand();
  mMapItem->update();

  addOverviewListItem( overview->id(), overview->name() );
#endif
  mOverviewListWidget->setCurrentRow( 0 );
}

void QgsLayoutMapWidget::mRemoveOverviewPushButton_clicked()
{
  QListWidgetItem *item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }
#if 0 //TODO
  mMapItem->overviews()->removeOverview( item->data( Qt::UserRole ).toString() );
  QListWidgetItem *delItem = mOverviewListWidget->takeItem( mOverviewListWidget->row( item ) );
  delete delItem;
  mMapItem->update();
#endif
}

void QgsLayoutMapWidget::mOverviewUpButton_clicked()
{
  QListWidgetItem *item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  int row = mOverviewListWidget->row( item );
  if ( row < 1 )
  {
    return;
  }
  mOverviewListWidget->takeItem( row );
  mOverviewListWidget->insertItem( row - 1, item );
  mOverviewListWidget->setCurrentItem( item );
#if 0 //TODO
  mMapItem->overviews()->moveOverviewUp( item->data( Qt::UserRole ).toString() );
#endif
  mMapItem->update();
}

void QgsLayoutMapWidget::mOverviewDownButton_clicked()
{
  QListWidgetItem *item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  int row = mOverviewListWidget->row( item );
  if ( mOverviewListWidget->count() <= row )
  {
    return;
  }
  mOverviewListWidget->takeItem( row );
  mOverviewListWidget->insertItem( row + 1, item );
  mOverviewListWidget->setCurrentItem( item );
#if 0 //TODO
  mMapItem->overviews()->moveOverviewDown( item->data( Qt::UserRole ).toString() );
#endif
  mMapItem->update();
}

QgsLayoutMapOverview *QgsLayoutMapWidget::currentOverview()
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

#if 0 //TODO
  return mMapItem->overviews()->overview( item->data( Qt::UserRole ).toString() );
#endif
  return nullptr;
}

void QgsLayoutMapWidget::mOverviewListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous )
{
  Q_UNUSED( previous );
  if ( !current )
  {
    mOverviewCheckBox->setEnabled( false );
    return;
  }

  mOverviewCheckBox->setEnabled( true );
#if 0 //TODO
  setOverviewItems( mMapItem->overviews()->constOverview( current->data( Qt::UserRole ).toString() ) );
#endif
}

void QgsLayoutMapWidget::mOverviewListWidget_itemChanged( QListWidgetItem *item )
{
  if ( !mMapItem )
  {
    return;
  }

#if 0 //TODO
  QgsComposerMapOverview *overview = mMapItem->overviews()->overview( item->data( Qt::UserRole ).toString() );
  if ( !overview )
  {
    return;
  }

  overview->setName( item->text() );
  if ( item->isSelected() )
  {
    //update checkbox title if item is current item
    mOverviewCheckBox->setTitle( QString( tr( "Draw \"%1\" overview" ) ).arg( overview->name() ) );
  }
#endif
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
}

void QgsLayoutMapWidget::blockOverviewItemsSignals( bool block )
{
  //grid
  mOverviewFrameMapComboBox->blockSignals( block );
  mOverviewFrameStyleButton->blockSignals( block );
  mOverviewBlendModeComboBox->blockSignals( block );
  mOverviewInvertCheckbox->blockSignals( block );
  mOverviewCenterCheckbox->blockSignals( block );
}

void QgsLayoutMapWidget::setOverviewItems( const QgsLayoutMapOverview *overview )
{
  if ( !overview )
  {
    return;
  }

  blockOverviewItemsSignals( true );

#if 0 //TODO
  mOverviewCheckBox->setTitle( QString( tr( "Draw \"%1\" overview" ) ).arg( overview->name() ) );
  mOverviewCheckBox->setChecked( overview->enabled() );

  //overview frame
  mOverviewFrameMapComboBox->setItem( mMapItem->composition()->getComposerMapById( overview->frameMapId() ) );
  //overview frame blending mode
  mOverviewBlendModeComboBox->setBlendMode( overview->blendMode() );
  //overview inverted
  mOverviewInvertCheckbox->setChecked( overview->inverted() );
  //center overview
  mOverviewCenterCheckbox->setChecked( overview->centered() );
#endif
  //frame style
  updateOverviewFrameSymbolMarker( overview );

  blockOverviewItemsSignals( false );
}

void QgsLayoutMapWidget::updateOverviewFrameSymbolMarker( const QgsLayoutMapOverview *overview )
{
#if 0 //TODO
  if ( overview )
  {
    QgsFillSymbol *nonConstSymbol = const_cast<QgsFillSymbol *>( overview->frameSymbol() ); //bad
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( nonConstSymbol, mOverviewFrameStyleButton->iconSize() );
    mOverviewFrameStyleButton->setIcon( icon );
  }
#endif
}

void QgsLayoutMapWidget::storeCurrentLayerSet()
{
  if ( !mMapItem )
    return;

  QList<QgsMapLayer *> layers = QgisApp::instance()->mapCanvas()->mapSettings().layers();
  mMapItem->setLayers( layers );

  if ( mMapItem->keepLayerStyles() )
  {
    // also store styles associated with the layers
    mMapItem->storeCurrentLayerStyles();
  }
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
  QList<QListWidgetItem *> itemSelection = mOverviewListWidget->selectedItems();
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

#if 0 //TODO
  //load all composer overviews into list widget
  QList< QgsComposerMapOverview * > overviews = mMapItem->overviews()->asList();
  QList< QgsComposerMapOverview * >::const_iterator overviewIt = overviews.constBegin();
  for ( ; overviewIt != overviews.constEnd(); ++overviewIt )
  {
    QListWidgetItem *item = addOverviewListItem( ( *overviewIt )->id(), ( *overviewIt )->name() );
    if ( selectedIds.contains( ( *overviewIt )->id() ) )
    {
      item->setSelected( true );
      mOverviewListWidget->setCurrentItem( item );
    }
  }
#endif

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
#if 0 //TODO
  QgsComposerMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Overview checkbox toggled" ) );
  if ( state )
  {
    overview->setEnabled( true );
  }
  else
  {
    overview->setEnabled( false );
  }
  mMapItem->update();
  mMapItem->endCommand();
#endif
}

void QgsLayoutMapWidget::overviewMapChanged( QgsComposerItem *item )
{
#if 0 //TODO
  QgsComposerMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  QgsComposerMap *map = dynamic_cast< QgsComposerMap * >( item );
  if ( !map )
    return;

  mMapItem->beginCommand( tr( "Overview map changed" ) );
  overview->setFrameMap( map->id() );
  mMapItem->update();
  mMapItem->endCommand();
#endif
}

void QgsLayoutMapWidget::mOverviewFrameStyleButton_clicked()
{
#if 0 //TODO
  QgsComposerMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  // use the atlas coverage layer, if any
  QgsVectorLayer *coverageLayer = atlasCoverageLayer();

  QgsFillSymbol *newSymbol = static_cast<QgsFillSymbol *>( overview->frameSymbol()->clone() );
  QgsExpressionContext context = mMapItem->createExpressionContext();

  QgsSymbolSelectorWidget *d = new QgsSymbolSelectorWidget( newSymbol, QgsStyle::defaultStyle(), coverageLayer, nullptr );
  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  d->setContext( symbolContext );

  connect( d, &QgsPanelWidget::widgetChanged, this, &QgsLayoutMapWidget::updateOverviewFrameStyleFromWidget );
  connect( d, &QgsPanelWidget::panelAccepted, this, &QgsLayoutMapWidget::cleanUpOverviewFrameStyleSelector );
  openPanel( d );
  mMapItem->beginCommand( tr( "Overview frame style changed" ) );
#endif
}

void QgsLayoutMapWidget::mOverviewBlendModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
#if 0 //TODO
  QgsComposerMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Overview blend mode changed" ) );
  overview->setBlendMode( mOverviewBlendModeComboBox->blendMode() );
  mMapItem->update();
  mMapItem->endCommand();
#endif
}

void QgsLayoutMapWidget::mOverviewInvertCheckbox_toggled( bool state )
{
#if 0 //TODO
  QgsComposerMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Overview inverted toggled" ) );
  overview->setInverted( state );
  mMapItem->update();
  mMapItem->endCommand();
#endif
}

void QgsLayoutMapWidget::mOverviewCenterCheckbox_toggled( bool state )
{
#if 0 //TODO
  QgsComposerMapOverview *overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mMapItem->beginCommand( tr( "Overview centered toggled" ) );
  overview->setCentered( state );
  mMapItem->update();
  mMapItem->endCommand();
#endif
}
