/***************************************************************************
                         qgscomposermapwidget.cpp
                         ------------------------
    begin                : May 26, 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgscomposermap.h"
#include "qgscomposermapgrid.h"
#include "qgscomposermapoverview.h"
#include "qgscomposermapwidget.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermapgridwidget.h"
#include "qgscomposition.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayerregistry.h"
#include "qgscomposershape.h"
#include "qgspaperitem.h"
#include "qgsproject.h"
#include "qgsmapthemecollection.h"
#include "qgsmapthemes.h"
#include "qgisgui.h"

#include <QMessageBox>

QgsComposerMapWidget::QgsComposerMapWidget( QgsComposerMap* composerMap )
    : QgsComposerItemBaseWidget( nullptr, composerMap )
    , mComposerMap( composerMap )
{
  setupUi( this );
  setPanelTitle( tr( "Map properties" ) );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, composerMap );
  mainLayout->addWidget( itemPropertiesWidget );

  mScaleLineEdit->setValidator( new QDoubleValidator( mScaleLineEdit ) );

  mXMinLineEdit->setValidator( new QDoubleValidator( mXMinLineEdit ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( mXMaxLineEdit ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( mYMinLineEdit ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( mYMaxLineEdit ) );

  blockAllSignals( true );
  mPreviewModeComboBox->insertItem( 0, tr( "Cache" ) );
  mPreviewModeComboBox->insertItem( 1, tr( "Render" ) );
  mPreviewModeComboBox->insertItem( 2, tr( "Rectangle" ) );

  // follow preset combo
  mFollowVisibilityPresetCombo->setModel( new QStringListModel( mFollowVisibilityPresetCombo ) );
  connect( mFollowVisibilityPresetCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( followVisibilityPresetSelected( int ) ) );
  connect( QgsProject::instance()->mapThemeCollection(), SIGNAL( presetsChanged() ),
           this, SLOT( onPresetsChanged() ) );
  onPresetsChanged();

  // keep layers from preset button
  QMenu* menuKeepLayers = new QMenu( this );
  mLayerListFromPresetButton->setMenu( menuKeepLayers );
  mLayerListFromPresetButton->setIcon( QgsApplication::getThemeIcon( "/mActionShowAllLayers.svg" ) );
  mLayerListFromPresetButton->setToolTip( tr( "Set layer list from a map theme" ) );
  connect( menuKeepLayers, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowKeepLayersVisibilityPresetsMenu() ) );

  if ( composerMap )
  {
    mLabel->setText( tr( "Map %1" ).arg( composerMap->id() ) );

    connect( composerMap, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );

    QgsAtlasComposition* atlas = atlasComposition();
    if ( atlas )
    {
      connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
               this, SLOT( atlasLayerChanged( QgsVectorLayer* ) ) );
      connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( compositionAtlasToggled( bool ) ) );

      compositionAtlasToggled( atlas->enabled() );
    }

    mOverviewFrameMapComboBox->setComposition( composerMap->composition() );
    mOverviewFrameMapComboBox->setItemType( QgsComposerItem::ComposerMap );
    mOverviewFrameMapComboBox->setExceptedItemList( QList< QgsComposerItem* >() << composerMap );
    connect( mOverviewFrameMapComboBox, SIGNAL( itemChanged( QgsComposerItem* ) ), this, SLOT( overviewMapChanged( QgsComposerItem* ) ) );
  }

  updateGuiElements();
  loadGridEntries();
  loadOverviewEntries();
  populateDataDefinedButtons();
  blockAllSignals( false );
}

QgsComposerMapWidget::~QgsComposerMapWidget()
{
}

void QgsComposerMapWidget::populateDataDefinedButtons()
{
  registerDataDefinedButton( mScaleDDBtn, QgsComposerObject::MapScale,
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mMapRotationDDBtn, QgsComposerObject::MapRotation,
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mXMinDDBtn, QgsComposerObject::MapXMin,
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mYMinDDBtn, QgsComposerObject::MapYMin,
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mXMaxDDBtn, QgsComposerObject::MapXMax,
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mYMaxDDBtn, QgsComposerObject::MapYMax,
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mAtlasMarginDDBtn, QgsComposerObject::MapAtlasMargin,
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mStylePresetsDDBtn, QgsComposerObject::MapStylePreset,
                             QgsDataDefinedButton::String, tr( "string matching a style preset name" ) );
  registerDataDefinedButton( mLayersDDBtn, QgsComposerObject::MapLayers,
                             QgsDataDefinedButton::String, tr( "list of map layer names separated by | characters" ) );
}

void QgsComposerMapWidget::compositionAtlasToggled( bool atlasEnabled )
{
  if ( atlasEnabled &&
       mComposerMap && mComposerMap->composition() && mComposerMap->composition()->atlasComposition().coverageLayer()
       && mComposerMap->composition()->atlasComposition().coverageLayer()->wkbType() != QgsWkbTypes::NoGeometry )
  {
    mAtlasCheckBox->setEnabled( true );
  }
  else
  {
    mAtlasCheckBox->setEnabled( false );
    mAtlasCheckBox->setChecked( false );
  }
}

void QgsComposerMapWidget::aboutToShowKeepLayersVisibilityPresetsMenu()
{
  // this menu is for the case when setting "keep layers" and "keep layer styles"
  // and the preset configuration is copied. The preset is not followed further.

  QMenu* menu = qobject_cast<QMenu*>( sender() );
  if ( !menu )
    return;

  menu->clear();
  Q_FOREACH ( const QString& presetName, QgsProject::instance()->mapThemeCollection()->presets() )
  {
    menu->addAction( presetName, this, SLOT( keepLayersVisibilityPresetSelected() ) );
  }

  if ( menu->actions().isEmpty() )
    menu->addAction( tr( "No presets defined" ) )->setEnabled( false );
}

void QgsComposerMapWidget::followVisibilityPresetSelected( int currentIndex )
{
  if ( !mComposerMap )
    return;

  if ( currentIndex == -1 )
    return;  // doing combo box model reset

  QString presetName;
  if ( currentIndex != 0 )
  {
    presetName = mFollowVisibilityPresetCombo->currentText();
  }

  if ( presetName == mComposerMap->followVisibilityPresetName() )
    return;

  mFollowVisibilityPresetCheckBox->setChecked( true );
  mComposerMap->setFollowVisibilityPresetName( presetName );

  mComposerMap->cache();
  mComposerMap->update();
}

void QgsComposerMapWidget::keepLayersVisibilityPresetSelected()
{
  QAction* action = qobject_cast<QAction*>( sender() );
  if ( !action )
    return;

  QString presetName = action->text();
  QStringList lst = QgsMapThemes::instance()->orderedPresetVisibleLayers( presetName );
  if ( mComposerMap )
  {
    mKeepLayerListCheckBox->setChecked( true );
    mComposerMap->setLayerSet( lst );

    mKeepLayerStylesCheckBox->setChecked( true );

    mComposerMap->setLayerStyleOverrides( QgsProject::instance()->mapThemeCollection()->presetStyleOverrides( presetName ) );

    mComposerMap->cache();
    mComposerMap->update();
  }
}

void QgsComposerMapWidget::onPresetsChanged()
{
  if ( QStringListModel* model = qobject_cast<QStringListModel*>( mFollowVisibilityPresetCombo->model() ) )
  {
    QStringList lst;
    lst.append( tr( "(none)" ) );
    lst += QgsProject::instance()->mapThemeCollection()->presets();
    model->setStringList( lst );

    // select the previously selected item again
    int presetModelIndex = mFollowVisibilityPresetCombo->findText( mComposerMap->followVisibilityPresetName() );
    mFollowVisibilityPresetCombo->blockSignals( true );
    mFollowVisibilityPresetCombo->setCurrentIndex( presetModelIndex != -1 ? presetModelIndex : 0 ); // 0 == none
    mFollowVisibilityPresetCombo->blockSignals( false );
  }
}

void QgsComposerMapWidget::updateOverviewFrameStyleFromWidget()
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  QgsSymbolSelectorWidget* w = qobject_cast<QgsSymbolSelectorWidget*>( sender() );
  overview->setFrameSymbol( dynamic_cast< QgsFillSymbol* >( w->symbol()->clone() ) );
  mComposerMap->update();
}

void QgsComposerMapWidget::cleanUpOverviewFrameStyleSelector( QgsPanelWidget* container )
{
  QgsSymbolSelectorWidget* w = qobject_cast<QgsSymbolSelectorWidget*>( container );
  if ( !w )
    return;

  delete w->symbol();

  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  updateOverviewFrameSymbolMarker( overview );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAtlasCheckBox_toggled( bool checked )
{
  if ( !mComposerMap )
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
    QgsComposition* composition = mComposerMap->composition();
    if ( composition )
    {
      toggleAtlasScalingOptionsByLayerType();
    }
  }

  // disable predefined scales if none are defined
  if ( !hasPredefinedScales() )
  {
    mAtlasPredefinedScaleRadio->setEnabled( false );
  }

  mComposerMap->setAtlasDriven( checked );
  updateMapForAtlas();
}

void QgsComposerMapWidget::updateMapForAtlas()
{
  //update map if in atlas preview mode
  QgsComposition* composition = mComposerMap->composition();
  if ( !composition )
  {
    return;
  }
  if ( composition->atlasMode() == QgsComposition::AtlasOff )
  {
    return;
  }

  if ( mComposerMap->atlasDriven() )
  {
    //update atlas based extent for map
    QgsAtlasComposition* atlas = &composition->atlasComposition();
    //prepareMap causes a redraw
    atlas->prepareMap( mComposerMap );
  }
  else
  {
    //redraw map
    mComposerMap->cache();
    mComposerMap->update();
  }
}

void QgsComposerMapWidget::on_mAtlasMarginRadio_toggled( bool checked )
{
  mAtlasMarginSpinBox->setEnabled( checked );

  if ( checked && mComposerMap )
  {
    mComposerMap->setAtlasScalingMode( QgsComposerMap::Auto );
    updateMapForAtlas();
  }
}

void QgsComposerMapWidget::on_mAtlasMarginSpinBox_valueChanged( int value )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->setAtlasMargin( value / 100. );
  updateMapForAtlas();
}

void QgsComposerMapWidget::on_mAtlasFixedScaleRadio_toggled( bool checked )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( checked )
  {
    mComposerMap->setAtlasScalingMode( QgsComposerMap::Fixed );
    updateMapForAtlas();
  }
}

void QgsComposerMapWidget::on_mAtlasPredefinedScaleRadio_toggled( bool checked )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( hasPredefinedScales() )
  {
    if ( checked )
    {
      mComposerMap->setAtlasScalingMode( QgsComposerMap::Predefined );
      updateMapForAtlas();
    }
  }
  else
  {
    // restore to fixed scale if no predefined scales exist
    mAtlasFixedScaleRadio->blockSignals( true );
    mAtlasFixedScaleRadio->setChecked( true );
    mAtlasFixedScaleRadio->blockSignals( false );
    mComposerMap->setAtlasScalingMode( QgsComposerMap::Fixed );
  }
}

void QgsComposerMapWidget::on_mPreviewModeComboBox_activated( int i )
{
  Q_UNUSED( i );

  if ( !mComposerMap )
  {
    return;
  }

  if ( mComposerMap->isDrawing() )
  {
    return;
  }

  QString comboText = mPreviewModeComboBox->currentText();
  if ( comboText == tr( "Cache" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Cache );
    mUpdatePreviewButton->setEnabled( true );
  }
  else if ( comboText == tr( "Render" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Render );
    mUpdatePreviewButton->setEnabled( true );
  }
  else if ( comboText == tr( "Rectangle" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Rectangle );
    mUpdatePreviewButton->setEnabled( false );
  }

  mComposerMap->cache();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mScaleLineEdit_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }

  bool conversionSuccess;
  double scaleDenominator = mScaleLineEdit->text().toDouble( &conversionSuccess );

  if ( !conversionSuccess )
  {
    return;
  }

  if ( qRound( scaleDenominator ) == qRound( mComposerMap->scale() ) )
    return;

  mComposerMap->beginCommand( tr( "Map scale changed" ) );
  mComposerMap->setNewScale( scaleDenominator );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mMapRotationSpinBox_valueChanged( double value )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Map rotation changed" ), QgsComposerMergeCommand::ComposerMapRotation );
  mComposerMap->setMapRotation( value );
  mComposerMap->endCommand();
  mComposerMap->cache();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mSetToMapCanvasExtentButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsRectangle newExtent = mComposerMap->composition()->mapSettings().visibleExtent();

  mComposerMap->beginCommand( tr( "Map extent changed" ) );
  mComposerMap->zoomToExtent( newExtent );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mViewExtentInCanvasButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsRectangle currentMapExtent = *( mComposerMap->currentMapExtent() );

  if ( !currentMapExtent.isEmpty() )
  {
    QgisApp::instance()->mapCanvas()->setExtent( currentMapExtent );
    QgisApp::instance()->mapCanvas()->refresh();
  }
}

void QgsComposerMapWidget::on_mXMinLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mXMaxLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mYMinLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mYMaxLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::setGuiElementValues()
{
  mScaleLineEdit->blockSignals( true );
  mPreviewModeComboBox->blockSignals( true );

  updateGuiElements();

  mScaleLineEdit->blockSignals( false );
  mPreviewModeComboBox->blockSignals( false );
}

void QgsComposerMapWidget::updateGuiElements()
{
  if ( !mComposerMap )
  {
    return;
  }

  blockAllSignals( true );

  //width, height, scale
  double scale = mComposerMap->scale();

  //round scale to an appropriate number of decimal places
  if ( scale >= 10 )
  {
    //round scale to integer if it's greater than 10
    mScaleLineEdit->setText( QString::number( mComposerMap->scale(), 'f', 0 ) );
  }
  else if ( scale >= 1 )
  {
    //don't round scale if it's less than 10, instead use 4 decimal places
    mScaleLineEdit->setText( QString::number( mComposerMap->scale(), 'f', 4 ) );
  }
  else
  {
    //if scale < 1 then use 10 decimal places
    mScaleLineEdit->setText( QString::number( mComposerMap->scale(), 'f', 10 ) );
  }

  //preview mode
  QgsComposerMap::PreviewMode previewMode = mComposerMap->previewMode();
  int index = -1;
  if ( previewMode == QgsComposerMap::Cache )
  {
    index = mPreviewModeComboBox->findText( tr( "Cache" ) );
    mUpdatePreviewButton->setEnabled( true );
  }
  else if ( previewMode == QgsComposerMap::Render )
  {
    index = mPreviewModeComboBox->findText( tr( "Render" ) );
    mUpdatePreviewButton->setEnabled( true );
  }
  else if ( previewMode == QgsComposerMap::Rectangle )
  {
    index = mPreviewModeComboBox->findText( tr( "Rectangle" ) );
    mUpdatePreviewButton->setEnabled( false );
  }
  if ( index != -1 )
  {
    mPreviewModeComboBox->setCurrentIndex( index );
  }

  //composer map extent
  QgsRectangle composerMapExtent = *( mComposerMap->currentMapExtent() );
  mXMinLineEdit->setText( QString::number( composerMapExtent.xMinimum(), 'f', 3 ) );
  mXMaxLineEdit->setText( QString::number( composerMapExtent.xMaximum(), 'f', 3 ) );
  mYMinLineEdit->setText( QString::number( composerMapExtent.yMinimum(), 'f', 3 ) );
  mYMaxLineEdit->setText( QString::number( composerMapExtent.yMaximum(), 'f', 3 ) );

  mMapRotationSpinBox->setValue( mComposerMap->mapRotation( QgsComposerObject::OriginalValue ) );

  // follow preset check box
  mFollowVisibilityPresetCheckBox->setCheckState(
    mComposerMap->followVisibilityPreset() ? Qt::Checked : Qt::Unchecked );
  int presetModelIndex = mFollowVisibilityPresetCombo->findText( mComposerMap->followVisibilityPresetName() );
  mFollowVisibilityPresetCombo->setCurrentIndex( presetModelIndex != -1 ? presetModelIndex : 0 ); // 0 == none

  //keep layer list check box
  if ( mComposerMap->keepLayerSet() )
  {
    mKeepLayerListCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mKeepLayerListCheckBox->setCheckState( Qt::Unchecked );
  }

  mKeepLayerStylesCheckBox->setEnabled( mComposerMap->keepLayerSet() );
  mKeepLayerStylesCheckBox->setCheckState( mComposerMap->keepLayerStyles() ? Qt::Checked : Qt::Unchecked );

  //draw canvas items
  if ( mComposerMap->drawCanvasItems() )
  {
    mDrawCanvasItemsCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mDrawCanvasItemsCheckBox->setCheckState( Qt::Unchecked );
  }

  //atlas controls
  mAtlasCheckBox->setChecked( mComposerMap->atlasDriven() );
  mAtlasMarginSpinBox->setValue( static_cast<int>( mComposerMap->atlasMargin( QgsComposerObject::OriginalValue ) * 100 ) );

  mAtlasFixedScaleRadio->setEnabled( mComposerMap->atlasDriven() );
  mAtlasFixedScaleRadio->setChecked( mComposerMap->atlasScalingMode() == QgsComposerMap::Fixed );
  mAtlasMarginSpinBox->setEnabled( mComposerMap->atlasScalingMode() == QgsComposerMap::Auto );
  mAtlasMarginRadio->setEnabled( mComposerMap->atlasDriven() );
  mAtlasMarginRadio->setChecked( mComposerMap->atlasScalingMode() == QgsComposerMap::Auto );
  mAtlasPredefinedScaleRadio->setEnabled( mComposerMap->atlasDriven() );
  mAtlasPredefinedScaleRadio->setChecked( mComposerMap->atlasScalingMode() == QgsComposerMap::Predefined );

  if ( mComposerMap->atlasDriven() )
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

void QgsComposerMapWidget::toggleAtlasScalingOptionsByLayerType()
{
  if ( !mComposerMap )
  {
    return;
  }

  //get atlas coverage layer
  QgsVectorLayer* coverageLayer = atlasCoverageLayer();
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
}

void QgsComposerMapWidget::updateComposerExtentFromGui()
{
  if ( !mComposerMap )
  {
    return;
  }

  double xmin, ymin, xmax, ymax;
  bool conversionSuccess;

  xmin = mXMinLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;
  xmax = mXMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymin = mYMinLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymax = mYMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;

  QgsRectangle newExtent( xmin, ymin, xmax, ymax );
  mComposerMap->beginCommand( tr( "Map extent changed" ) );
  mComposerMap->setNewExtent( newExtent );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::blockAllSignals( bool b )
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

void QgsComposerMapWidget::handleChangedFrameDisplay( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::DisplayMode mode )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame divisions changed" ) );
  grid->setFrameDivisions( mode, border );
  mComposerMap->endCommand();
  mComposerMap->updateBoundingRect();
}

void QgsComposerMapWidget::handleChangedAnnotationDisplay( QgsComposerMapGrid::BorderSide border, const QString &text )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation display changed" ) );
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

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mUpdatePreviewButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( mComposerMap->isDrawing() )
  {
    return;
  }

  mUpdatePreviewButton->setEnabled( false ); //prevent crashes because of many button clicks

  mComposerMap->setCacheUpdated( false );
  mComposerMap->cache();
  mComposerMap->update();

  mUpdatePreviewButton->setEnabled( true );
}

void QgsComposerMapWidget::on_mFollowVisibilityPresetCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mComposerMap->setFollowVisibilityPreset( true );

    // mutually exclusive with keeping custom layer list
    mKeepLayerListCheckBox->setCheckState( Qt::Unchecked );
    mKeepLayerStylesCheckBox->setCheckState( Qt::Unchecked );

    mComposerMap->cache();
    mComposerMap->update();
  }
  else
  {
    mComposerMap->setFollowVisibilityPreset( false );
  }
}

void QgsComposerMapWidget::on_mKeepLayerListCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mComposerMap->storeCurrentLayerSet();
    mComposerMap->setKeepLayerSet( true );

    // mutually exclusive with following a preset
    mFollowVisibilityPresetCheckBox->setCheckState( Qt::Unchecked );
  }
  else
  {
    QStringList emptyLayerSet;
    mComposerMap->setLayerSet( emptyLayerSet );
    mComposerMap->setKeepLayerSet( false );

    mKeepLayerStylesCheckBox->setChecked( Qt::Unchecked );
  }

  mKeepLayerStylesCheckBox->setEnabled( state == Qt::Checked );
}

void QgsComposerMapWidget::on_mKeepLayerStylesCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mComposerMap->storeCurrentLayerStyles();
    mComposerMap->setKeepLayerStyles( true );
  }
  else
  {
    mComposerMap->setLayerStyleOverrides( QMap<QString, QString>() );
    mComposerMap->setKeepLayerStyles( false );
  }
}

void QgsComposerMapWidget::on_mDrawCanvasItemsCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Canvas items toggled" ) );
  mComposerMap->setDrawCanvasItems( state == Qt::Checked );
  mUpdatePreviewButton->setEnabled( false ); //prevent crashes because of many button clicks
  mComposerMap->setCacheUpdated( false );
  mComposerMap->cache();
  mComposerMap->update();
  mUpdatePreviewButton->setEnabled( true );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::addPageToToolbox( QWidget* widget, const QString& name )
{
  Q_UNUSED( name );
  //TODO : wrap the widget in a collapsibleGroupBox to be more consistent with previous implementation
  mainLayout->addWidget( widget );
}

void QgsComposerMapWidget::insertAnnotationPositionEntries( QComboBox* c )
{
  c->insertItem( 0, tr( "Inside frame" ) );
  c->insertItem( 1, tr( "Outside frame" ) );
}

void QgsComposerMapWidget::insertAnnotationDirectionEntries( QComboBox* c )
{
  c->addItem( tr( "Horizontal" ), QgsComposerMapGrid::Horizontal );
  c->addItem( tr( "Vertical ascending" ), QgsComposerMapGrid::Vertical );
  c->addItem( tr( "Vertical descending" ), QgsComposerMapGrid::VerticalDescending );
}

void QgsComposerMapWidget::initFrameDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( display ) );
}

void QgsComposerMapWidget::initAnnotationDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display )
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

void QgsComposerMapWidget::handleChangedAnnotationPosition( QgsComposerMapGrid::BorderSide border, const QString& text )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation position changed" ) );
  if ( text == tr( "Inside frame" ) )
  {
    grid->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, border );
  }
  else //Outside frame
  {
    grid->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, border );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::handleChangedAnnotationDirection( QgsComposerMapGrid::BorderSide border, QgsComposerMapGrid::AnnotationDirection direction )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed annotation direction" ) );
  grid->setAnnotationDirection( direction, border );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::insertFrameDisplayEntries( QComboBox *c )
{
  c->addItem( tr( "All" ), QgsComposerMapGrid::ShowAll );
  c->addItem( tr( "Latitude/Y only" ), QgsComposerMapGrid::LatitudeOnly );
  c->addItem( tr( "Longitude/X only" ), QgsComposerMapGrid::LongitudeOnly );
}

void QgsComposerMapWidget::insertAnnotationDisplayEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Show all" ) );
  c->insertItem( 1, tr( "Show latitude only" ) );
  c->insertItem( 2, tr( "Show longitude only" ) );
  c->insertItem( 3, tr( "Disabled" ) );
}

void QgsComposerMapWidget::initAnnotationPositionBox( QComboBox* c, QgsComposerMapGrid::AnnotationPosition pos )
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

void QgsComposerMapWidget::initAnnotationDirectionBox( QComboBox* c, QgsComposerMapGrid::AnnotationDirection dir )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( dir ) );
}

void QgsComposerMapWidget::atlasLayerChanged( QgsVectorLayer* layer )
{
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
}

bool QgsComposerMapWidget::hasPredefinedScales() const
{
  // first look at project's scales
  QStringList scales( QgsProject::instance()->readListEntry( "Scales", "/ScalesList" ) );
  bool hasProjectScales( QgsProject::instance()->readBoolEntry( "Scales", "/useProjectScales" ) );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    QSettings settings;
    QString scalesStr( settings.value( "Map/scales", PROJECT_SCALES ).toString() );
    QStringList myScalesList = scalesStr.split( ',' );
    return !myScalesList.isEmpty() && myScalesList[0] != "";
  }
  return true;
}

void QgsComposerMapWidget::on_mAddGridPushButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QString itemName = tr( "Grid %1" ).arg( mComposerMap->grids()->size() + 1 );
  QgsComposerMapGrid* grid = new QgsComposerMapGrid( itemName, mComposerMap );
  mComposerMap->beginCommand( tr( "Add map grid" ) );
  mComposerMap->grids()->addGrid( grid );
  mComposerMap->endCommand();
  mComposerMap->updateBoundingRect();
  mComposerMap->update();

  addGridListItem( grid->id(), grid->name() );
  mGridListWidget->setCurrentRow( 0 );
  on_mGridListWidget_currentItemChanged( mGridListWidget->currentItem(), nullptr );
}

void QgsComposerMapWidget::on_mRemoveGridPushButton_clicked()
{
  QListWidgetItem* item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  mComposerMap->grids()->removeGrid( item->data( Qt::UserRole ).toString() );
  QListWidgetItem* delItem = mGridListWidget->takeItem( mGridListWidget->row( item ) );
  delete delItem;
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mGridUpButton_clicked()
{
  QListWidgetItem* item = mGridListWidget->currentItem();
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
  mComposerMap->grids()->moveGridUp( item->data( Qt::UserRole ).toString() );
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mGridDownButton_clicked()
{
  QListWidgetItem* item = mGridListWidget->currentItem();
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
  mComposerMap->grids()->moveGridDown( item->data( Qt::UserRole ).toString() );
  mComposerMap->update();
}

QgsComposerMapGrid* QgsComposerMapWidget::currentGrid()
{
  if ( !mComposerMap )
  {
    return nullptr;
  }

  QListWidgetItem* item = mGridListWidget->currentItem();
  if ( !item )
  {
    return nullptr;
  }

  return mComposerMap->grids()->grid( item->data( Qt::UserRole ).toString() );
}

void QgsComposerMapWidget::on_mGridListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous )
{
  Q_UNUSED( previous );
  if ( !current )
  {
    mDrawGridCheckBox->setEnabled( false );
    mDrawGridCheckBox->setChecked( false );
    mGridPropertiesButton->setEnabled( false );
    return;
  }

  mDrawGridCheckBox->setEnabled( true );
  mDrawGridCheckBox->setChecked( currentGrid()->enabled() );
  mGridPropertiesButton->setEnabled( currentGrid()->enabled() );

  mDrawGridCheckBox->setText( QString( tr( "Draw \"%1\" grid" ) ).arg( currentGrid()->name() ) );
}

void QgsComposerMapWidget::on_mGridListWidget_itemChanged( QListWidgetItem* item )
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsComposerMapGrid* grid = mComposerMap->grids()->grid( item->data( Qt::UserRole ).toString() );
  if ( !grid )
  {
    return;
  }

  grid->setName( item->text() );
  if ( item->isSelected() )
  {
    //update check box title if item is current item
    mDrawGridCheckBox->setText( QString( tr( "Draw \"%1\" grid" ) ).arg( grid->name() ) );
  }
}

void QgsComposerMapWidget::on_mGridPropertiesButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  QgsComposerMapGridWidget* w = new QgsComposerMapGridWidget( grid, mComposerMap );
  openPanel( w );
}

QListWidgetItem* QgsComposerMapWidget::addGridListItem( const QString& id, const QString& name )
{
  QListWidgetItem* item = new QListWidgetItem( name, nullptr );
  item->setData( Qt::UserRole, id );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mGridListWidget->insertItem( 0, item );
  return item;
}

void QgsComposerMapWidget::loadGridEntries()
{
  //save selection
  QSet<QString> selectedIds;
  QList<QListWidgetItem*> itemSelection = mGridListWidget->selectedItems();
  QList<QListWidgetItem*>::const_iterator sIt = itemSelection.constBegin();
  for ( ; sIt != itemSelection.constEnd(); ++sIt )
  {
    selectedIds.insert(( *sIt )->data( Qt::UserRole ).toString() );
  }

  mGridListWidget->clear();
  if ( !mComposerMap )
  {
    return;
  }

  //load all composer grids into list widget
  QList< QgsComposerMapGrid* > grids = mComposerMap->grids()->asList();
  QList< QgsComposerMapGrid* >::const_iterator gridIt = grids.constBegin();
  for ( ; gridIt != grids.constEnd(); ++gridIt )
  {
    QListWidgetItem* item = addGridListItem(( *gridIt )->id(), ( *gridIt )->name() );
    if ( selectedIds.contains(( *gridIt )->id() ) )
    {
      item->setSelected( true );
      mGridListWidget->setCurrentItem( item );
    }
  }

  if ( mGridListWidget->currentItem() )
  {
    on_mGridListWidget_currentItemChanged( mGridListWidget->currentItem(), nullptr );
  }
  else
  {
    on_mGridListWidget_currentItemChanged( nullptr, nullptr );
  }
}

void QgsComposerMapWidget::on_mDrawGridCheckBox_toggled( bool state )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mGridPropertiesButton->setEnabled( state );

  mComposerMap->beginCommand( tr( "Grid checkbox toggled" ) );
  if ( state )
  {
    grid->setEnabled( true );
  }
  else
  {
    grid->setEnabled( false );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAddOverviewPushButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QString itemName = tr( "Overview %1" ).arg( mComposerMap->overviews()->size() + 1 );
  QgsComposerMapOverview* overview = new QgsComposerMapOverview( itemName, mComposerMap );
  mComposerMap->beginCommand( tr( "Add map overview" ) );
  mComposerMap->overviews()->addOverview( overview );
  mComposerMap->endCommand();
  mComposerMap->update();

  addOverviewListItem( overview->id(), overview->name() );
  mOverviewListWidget->setCurrentRow( 0 );
}

void QgsComposerMapWidget::on_mRemoveOverviewPushButton_clicked()
{
  QListWidgetItem* item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  mComposerMap->overviews()->removeOverview( item->data( Qt::UserRole ).toString() );
  QListWidgetItem* delItem = mOverviewListWidget->takeItem( mOverviewListWidget->row( item ) );
  delete delItem;
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mOverviewUpButton_clicked()
{
  QListWidgetItem* item = mOverviewListWidget->currentItem();
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
  mComposerMap->overviews()->moveOverviewUp( item->data( Qt::UserRole ).toString() );
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mOverviewDownButton_clicked()
{
  QListWidgetItem* item = mOverviewListWidget->currentItem();
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
  mComposerMap->overviews()->moveOverviewDown( item->data( Qt::UserRole ).toString() );
  mComposerMap->update();
}

QgsComposerMapOverview* QgsComposerMapWidget::currentOverview()
{
  if ( !mComposerMap )
  {
    return nullptr;
  }

  QListWidgetItem* item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return nullptr;
  }

  return mComposerMap->overviews()->overview( item->data( Qt::UserRole ).toString() );
}

void QgsComposerMapWidget::on_mOverviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous )
{
  Q_UNUSED( previous );
  if ( !current )
  {
    mOverviewCheckBox->setEnabled( false );
    return;
  }

  mOverviewCheckBox->setEnabled( true );
  setOverviewItems( mComposerMap->overviews()->constOverview( current->data( Qt::UserRole ).toString() ) );
}

void QgsComposerMapWidget::on_mOverviewListWidget_itemChanged( QListWidgetItem* item )
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsComposerMapOverview* overview = mComposerMap->overviews()->overview( item->data( Qt::UserRole ).toString() );
  if ( !overview )
  {
    return;
  }

  overview->setName( item->text() );
  if ( item->isSelected() )
  {
    //update check box title if item is current item
    mOverviewCheckBox->setTitle( QString( tr( "Draw \"%1\" overview" ) ).arg( overview->name() ) );
  }
}

void QgsComposerMapWidget::setOverviewItemsEnabled( bool enabled )
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

void QgsComposerMapWidget::blockOverviewItemsSignals( bool block )
{
  //grid
  mOverviewFrameMapComboBox->blockSignals( block );
  mOverviewFrameStyleButton->blockSignals( block );
  mOverviewBlendModeComboBox->blockSignals( block );
  mOverviewInvertCheckbox->blockSignals( block );
  mOverviewCenterCheckbox->blockSignals( block );
}

void QgsComposerMapWidget::setOverviewItems( const QgsComposerMapOverview* overview )
{
  if ( !overview )
  {
    return;
  }

  blockOverviewItemsSignals( true );

  mOverviewCheckBox->setTitle( QString( tr( "Draw \"%1\" overview" ) ).arg( overview->name() ) );
  mOverviewCheckBox->setChecked( overview->enabled() );

  //overview frame
  mOverviewFrameMapComboBox->setItem( mComposerMap->composition()->getComposerMapById( overview->frameMapId() ) );
  //overview frame blending mode
  mOverviewBlendModeComboBox->setBlendMode( overview->blendMode() );
  //overview inverted
  mOverviewInvertCheckbox->setChecked( overview->inverted() );
  //center overview
  mOverviewCenterCheckbox->setChecked( overview->centered() );

  //frame style
  updateOverviewFrameSymbolMarker( overview );

  blockOverviewItemsSignals( false );
}

void QgsComposerMapWidget::updateOverviewFrameSymbolMarker( const QgsComposerMapOverview* overview )
{
  if ( overview )
  {
    QgsFillSymbol* nonConstSymbol = const_cast<QgsFillSymbol*>( overview->frameSymbol() ); //bad
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( nonConstSymbol, mOverviewFrameStyleButton->iconSize() );
    mOverviewFrameStyleButton->setIcon( icon );
  }
}

QListWidgetItem* QgsComposerMapWidget::addOverviewListItem( const QString& id, const QString& name )
{
  QListWidgetItem* item = new QListWidgetItem( name, nullptr );
  item->setData( Qt::UserRole, id );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mOverviewListWidget->insertItem( 0, item );
  return item;
}

void QgsComposerMapWidget::loadOverviewEntries()
{
  //save selection
  QSet<QString> selectedIds;
  QList<QListWidgetItem*> itemSelection = mOverviewListWidget->selectedItems();
  QList<QListWidgetItem*>::const_iterator sIt = itemSelection.constBegin();
  for ( ; sIt != itemSelection.constEnd(); ++sIt )
  {
    selectedIds.insert(( *sIt )->data( Qt::UserRole ).toString() );
  }

  mOverviewListWidget->clear();
  if ( !mComposerMap )
  {
    return;
  }

  //load all composer overviews into list widget
  QList< QgsComposerMapOverview* > overviews = mComposerMap->overviews()->asList();
  QList< QgsComposerMapOverview* >::const_iterator overviewIt = overviews.constBegin();
  for ( ; overviewIt != overviews.constEnd(); ++overviewIt )
  {
    QListWidgetItem* item = addOverviewListItem(( *overviewIt )->id(), ( *overviewIt )->name() );
    if ( selectedIds.contains(( *overviewIt )->id() ) )
    {
      item->setSelected( true );
      mOverviewListWidget->setCurrentItem( item );
    }
  }

  if ( mOverviewListWidget->currentItem() )
  {
    on_mOverviewListWidget_currentItemChanged( mOverviewListWidget->currentItem(), nullptr );
  }
  else
  {
    on_mOverviewListWidget_currentItemChanged( nullptr, nullptr );
  }
}

void QgsComposerMapWidget::on_mOverviewCheckBox_toggled( bool state )
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Overview checkbox toggled" ) );
  if ( state )
  {
    overview->setEnabled( true );
  }
  else
  {
    overview->setEnabled( false );
  }
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::overviewMapChanged( QgsComposerItem* item )
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  QgsComposerMap* map = dynamic_cast< QgsComposerMap* >( item );
  if ( !map )
    return;

  mComposerMap->beginCommand( tr( "Overview map changed" ) );
  overview->setFrameMap( map->id() );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOverviewFrameStyleButton_clicked()
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  // use the atlas coverage layer, if any
  QgsVectorLayer* coverageLayer = atlasCoverageLayer();

  QgsFillSymbol* newSymbol = static_cast<QgsFillSymbol*>( overview->frameSymbol()->clone() );
  QgsExpressionContext context = mComposerMap->createExpressionContext();

  QgsSymbolSelectorWidget* d = new QgsSymbolSelectorWidget( newSymbol, QgsStyle::defaultStyle(), coverageLayer, nullptr );
  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  d->setContext( symbolContext );

  connect( d, SIGNAL( widgetChanged() ), this, SLOT( updateOverviewFrameStyleFromWidget() ) );
  connect( d, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( cleanUpOverviewFrameStyleSelector( QgsPanelWidget* ) ) );
  openPanel( d );
  mComposerMap->beginCommand( tr( "Overview frame style changed" ) );
}

void QgsComposerMapWidget::on_mOverviewBlendModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Overview blend mode changed" ) );
  overview->setBlendMode( mOverviewBlendModeComboBox->blendMode() );
  mComposerMap->update();
  mComposerMap->endCommand();
}
void QgsComposerMapWidget::on_mOverviewInvertCheckbox_toggled( bool state )
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Overview inverted toggled" ) );
  overview->setInverted( state );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOverviewCenterCheckbox_toggled( bool state )
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Overview centered toggled" ) );
  overview->setCentered( state );
  mComposerMap->update();
  mComposerMap->endCommand();
}
