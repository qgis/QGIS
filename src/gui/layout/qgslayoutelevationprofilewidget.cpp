/***************************************************************************
                         qgslayoutelevationprofilewidget.cpp
                         ----------------------
    begin                : January 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgslayoutelevationprofilewidget.h"
#include "qgslayoutitemelevationprofile.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemregistry.h"
#include "qgsplot.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsvectorlayer.h"
#include "qgsnumericformatselectorwidget.h"
#include "qgslayout.h"
#include "qgslayertree.h"
#include "qgslayertreeregistrybridge.h"
#include "qgselevationprofilelayertreeview.h"

QgsLayoutElevationProfileWidget::QgsLayoutElevationProfileWidget( QgsLayoutItemElevationProfile *profile )
  : QgsLayoutItemBaseWidget( nullptr, profile )
  , mProfile( profile )
  , mLayerTree( new QgsLayerTree() )
  , mLayerTreeBridge( new QgsLayerTreeRegistryBridge( mLayerTree.get(), mProfile->layout() ? mProfile->layout()->project() : QgsProject::instance(), this ) )
{
  Q_ASSERT( mProfile );

  setupUi( this );
  setPanelTitle( tr( "Elevation Profile Properties" ) );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, profile );
  mainLayout->addWidget( mItemPropertiesWidget );

  connect( mLayerTree.get(), &QgsLayerTree::layerOrderChanged, this, &QgsLayoutElevationProfileWidget::updateItemLayers );
  connect( mLayerTree.get(), &QgsLayerTreeGroup::visibilityChanged, this, &QgsLayoutElevationProfileWidget::updateItemLayers );

  mSpinMinDistance->setClearValue( 0 );
  connect( mSpinMinDistance, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Minimum Distance" ), QgsLayoutItem::UndoElevationProfileMinimumDistance );
    mProfile->plot()->setXMinimum( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mSpinMaxDistance, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Maximum Distance" ), QgsLayoutItem::UndoElevationProfileMaximumDistance );
    mProfile->plot()->setXMaximum( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinMinElevation->setClearValue( 0 );
  connect( mSpinMinElevation, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Minimum Elevation" ), QgsLayoutItem::UndoElevationProfileMinimumElevation );
    mProfile->plot()->setYMinimum( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mSpinMaxElevation, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Maximum Elevation" ), QgsLayoutItem::UndoElevationProfileMaximumElevation );
    mProfile->plot()->setYMaximum( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mDistanceAxisMajorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mDistanceAxisMajorLinesSymbolButton, &QgsSymbolButton::changed, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Major Gridlines" ), QgsLayoutItem::UndoElevationProfileDistanceMajorGridlines );
    mProfile->plot()->xAxis().setGridMajorSymbol( mDistanceAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mDistanceAxisMajorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMajorSymbol() );

  mDistanceAxisMinorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mDistanceAxisMinorLinesSymbolButton, &QgsSymbolButton::changed, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Minor Gridlines" ), QgsLayoutItem::UndoElevationProfileDistanceMinorGridlines );
    mProfile->plot()->xAxis().setGridMinorSymbol( mDistanceAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mDistanceAxisMinorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMinorSymbol() );

  connect( mDistanceAxisMajorIntervalSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Major Gridlines" ), QgsLayoutItem::UndoElevationProfileDistanceMajorGridlines );
    mProfile->plot()->xAxis().setGridIntervalMajor( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mDistanceAxisMinorIntervalSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Minor Gridlines" ), QgsLayoutItem::UndoElevationProfileDistanceMinorGridlines );
    mProfile->plot()->xAxis().setGridIntervalMinor( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mDistanceAxisLabelIntervalSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Label" ), QgsLayoutItem::UndoElevationProfileDistanceLabels );
    mProfile->plot()->xAxis().setLabelInterval( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mElevationAxisMajorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mElevationAxisMajorLinesSymbolButton, &QgsSymbolButton::changed, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Major Gridlines" ), QgsLayoutItem::UndoElevationProfileElevationMajorGridlines );
    mProfile->plot()->yAxis().setGridMajorSymbol( mElevationAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mElevationAxisMajorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMajorSymbol() );

  mElevationAxisMinorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mElevationAxisMinorLinesSymbolButton, &QgsSymbolButton::changed, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Minor Gridlines" ), QgsLayoutItem::UndoElevationProfileElevationMinorGridlines );
    mProfile->plot()->yAxis().setGridMinorSymbol( mElevationAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mElevationAxisMinorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMinorSymbol() );

  connect( mElevationAxisLabelIntervalSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Label" ), QgsLayoutItem::UndoElevationProfileElevationLabels );
    mProfile->plot()->yAxis().setLabelInterval( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mElevationAxisMajorIntervalSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Major Gridlines" ), QgsLayoutItem::UndoElevationProfileElevationMajorGridlines );
    mProfile->plot()->yAxis().setGridIntervalMajor( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mElevationAxisMinorIntervalSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Minor Gridlines" ), QgsLayoutItem::UndoElevationProfileElevationMinorGridlines );
    mProfile->plot()->yAxis().setGridIntervalMinor( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mChartBackgroundSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  connect( mChartBackgroundSymbolButton, &QgsSymbolButton::changed, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Background" ), QgsLayoutItem::UndoElevationProfileChartBackground );
    mProfile->plot()->setChartBackgroundSymbol( mChartBackgroundSymbolButton->clonedSymbol<QgsFillSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mChartBackgroundSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::chartBackgroundSymbol() );

  mChartBorderSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  connect( mChartBorderSymbolButton, &QgsSymbolButton::changed, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Border" ), QgsLayoutItem::UndoElevationProfileChartBorder );
    mProfile->plot()->setChartBorderSymbol( mChartBorderSymbolButton->clonedSymbol<QgsFillSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mChartBorderSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::chartBorderSymbol() );

  connect( mDistanceAxisLabelFormatButton, &QPushButton::clicked, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Distance Number Format" ) );
    widget->setFormat( mProfile->plot()->xAxis().numericFormat() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [ = ]
    {
      mProfile->beginCommand( tr( "Change Profile Chart Distance Format" ), QgsLayoutItem::UndoElevationProfileDistanceFormat );
      mProfile->plot()->xAxis().setNumericFormat( widget->format() );
      mProfile->invalidateCache();
      mProfile->endCommand();
      mProfile->update();
    } );
    openPanel( widget );
  } );

  connect( mElevationAxisLabelFormatButton, &QPushButton::clicked, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Elevation Number Format" ) );
    widget->setFormat( mProfile->plot()->yAxis().numericFormat() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [ = ]
    {
      mProfile->beginCommand( tr( "Change Profile Chart Elevation Format" ), QgsLayoutItem::UndoElevationProfileElevationFormat );
      mProfile->plot()->yAxis().setNumericFormat( widget->format() );
      mProfile->invalidateCache();
      mProfile->endCommand();
      mProfile->update();
    } );
    openPanel( widget );
  } );

  mDistanceAxisLabelFontButton->setDialogTitle( tr( "Distance Label Font" ) );
  mElevationAxisLabelFontButton->setDialogTitle( tr( "Elevation Label Font" ) );
  mDistanceAxisLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );
  mElevationAxisLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );

  connect( mDistanceAxisLabelFontButton, &QgsFontButton::changed, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Font" ), QgsLayoutItem::UndoElevationProfileDistanceFont );
    mProfile->plot()->xAxis().setTextFormat( mDistanceAxisLabelFontButton->textFormat() );
    mProfile->invalidateCache();
    mProfile->endCommand();
    mProfile->update();
  } );

  connect( mElevationAxisLabelFontButton, &QgsFontButton::changed, this, [ = ]
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Font" ), QgsLayoutItem::UndoElevationProfileElevationFont );
    mProfile->plot()->yAxis().setTextFormat( mElevationAxisLabelFontButton->textFormat() );
    mProfile->invalidateCache();
    mProfile->endCommand();
    mProfile->update();
  } );

  mSpinLeftMargin->setClearValue( 0 );
  connect( mSpinLeftMargin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Left Margin" ), QgsLayoutItem::UndoMarginLeft );
    QgsMargins margins = mProfile->plot()->margins();
    margins.setLeft( value );
    mProfile->plot()->setMargins( margins );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinRightMargin->setClearValue( 0 );
  connect( mSpinRightMargin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Right Margin" ), QgsLayoutItem::UndoMarginRight );
    QgsMargins margins = mProfile->plot()->margins();
    margins.setRight( value );
    mProfile->plot()->setMargins( margins );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinTopMargin->setClearValue( 0 );
  connect( mSpinTopMargin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Top Margin" ), QgsLayoutItem::UndoMarginTop );
    QgsMargins margins = mProfile->plot()->margins();
    margins.setTop( value );
    mProfile->plot()->setMargins( margins );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinBottomMargin->setClearValue( 0 );
  connect( mSpinBottomMargin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Bottom Margin" ), QgsLayoutItem::UndoMarginBottom );
    QgsMargins margins = mProfile->plot()->margins();
    margins.setBottom( value );
    mProfile->plot()->setMargins( margins );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  registerDataDefinedButton( mDDBtnMinDistance, QgsLayoutObject::ElevationProfileMinimumDistance );
  registerDataDefinedButton( mDDBtnMaxDistance, QgsLayoutObject::ElevationProfileMaximumDistance );
  registerDataDefinedButton( mDDBtnMinElevation, QgsLayoutObject::ElevationProfileMinimumElevation );
  registerDataDefinedButton( mDDBtnMaxElevation, QgsLayoutObject::ElevationProfileMaximumElevation );
  registerDataDefinedButton( mDDBtnDistanceMajorInterval, QgsLayoutObject::ElevationProfileDistanceMajorInterval );
  registerDataDefinedButton( mDDBtnDistanceMinorInterval, QgsLayoutObject::ElevationProfileDistanceMinorInterval );
  registerDataDefinedButton( mDDBtnDistanceLabelInterval, QgsLayoutObject::ElevationProfileDistanceLabelInterval );
  registerDataDefinedButton( mDDBtnElevationMajorInterval, QgsLayoutObject::ElevationProfileElevationMajorInterval );
  registerDataDefinedButton( mDDBtnElevationMinorInterval, QgsLayoutObject::ElevationProfileElevationMinorInterval );
  registerDataDefinedButton( mDDBtnElevationLabelInterval, QgsLayoutObject::ElevationProfileElevationLabelInterval );
  registerDataDefinedButton( mDDBtnLeftMargin, QgsLayoutObject::MarginLeft );
  registerDataDefinedButton( mDDBtnRightMargin, QgsLayoutObject::MarginRight );
  registerDataDefinedButton( mDDBtnTopMargin, QgsLayoutObject::MarginTop );
  registerDataDefinedButton( mDDBtnBottomMargin, QgsLayoutObject::MarginBottom );

  mLayerTreeView = new QgsElevationProfileLayerTreeView( mLayerTree.get() );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->addWidget( mLayerTreeView );
  mTreeViewContainer->setLayout( vl );

  mBlockChanges++;
  mLayerTreeView->populateInitialLayers( mProfile->layout() && mProfile->layout()->project() ? mProfile->layout()->project() : QgsProject::instance() );
  mBlockChanges--;

  setGuiElementValues();

  mDistanceAxisMajorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
  mDistanceAxisMinorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
  mElevationAxisMajorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
  mElevationAxisMinorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
  mChartBackgroundSymbolButton->registerExpressionContextGenerator( mProfile );
  mChartBorderSymbolButton->registerExpressionContextGenerator( mProfile );
  mDistanceAxisLabelFontButton->registerExpressionContextGenerator( mProfile );
  mElevationAxisLabelFontButton->registerExpressionContextGenerator( mProfile );

  mDistanceAxisMajorLinesSymbolButton->setLayer( coverageLayer() );
  mDistanceAxisMinorLinesSymbolButton->setLayer( coverageLayer() );
  mElevationAxisMajorLinesSymbolButton->setLayer( coverageLayer() );
  mElevationAxisMinorLinesSymbolButton->setLayer( coverageLayer() );
  mDistanceAxisLabelFontButton->setLayer( coverageLayer() );
  mElevationAxisLabelFontButton->setLayer( coverageLayer() );
  mChartBackgroundSymbolButton->setLayer( coverageLayer() );
  mChartBorderSymbolButton->setLayer( coverageLayer() );

  if ( mProfile->layout() )
  {
    connect( &mProfile->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, this, [ = ]( QgsVectorLayer * layer )
    {
      mDistanceAxisMajorLinesSymbolButton->setLayer( layer );
      mDistanceAxisMinorLinesSymbolButton->setLayer( layer );
      mElevationAxisMajorLinesSymbolButton->setLayer( layer );
      mElevationAxisMinorLinesSymbolButton->setLayer( layer );
      mDistanceAxisLabelFontButton->setLayer( layer );
      mElevationAxisLabelFontButton->setLayer( layer );
      mChartBackgroundSymbolButton->setLayer( layer );
      mChartBorderSymbolButton->setLayer( layer );
    } );
  }
}

QgsLayoutElevationProfileWidget::~QgsLayoutElevationProfileWidget() = default;

void QgsLayoutElevationProfileWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

QgsExpressionContext QgsLayoutElevationProfileWidget::createExpressionContext() const
{
  return mProfile->createExpressionContext();
}

bool QgsLayoutElevationProfileWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutElevationProfile )
    return false;

  if ( mProfile )
  {
    disconnect( mProfile, &QgsLayoutObject::changed, this, &QgsLayoutElevationProfileWidget::setGuiElementValues );
  }

  mProfile = qobject_cast< QgsLayoutItemElevationProfile * >( item );
  mItemPropertiesWidget->setItem( mProfile );

  if ( mProfile )
  {
    connect( mProfile, &QgsLayoutObject::changed, this, &QgsLayoutElevationProfileWidget::setGuiElementValues );
    mDistanceAxisMajorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
    mDistanceAxisMinorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
    mElevationAxisMajorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
    mElevationAxisMinorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
    mDistanceAxisLabelFontButton->registerExpressionContextGenerator( mProfile );
    mElevationAxisLabelFontButton->registerExpressionContextGenerator( mProfile );
    mChartBackgroundSymbolButton->registerExpressionContextGenerator( mProfile );
    mChartBorderSymbolButton->registerExpressionContextGenerator( mProfile );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutElevationProfileWidget::setGuiElementValues()
{
  mBlockChanges++;

  mSpinMinDistance->setValue( mProfile->plot()->xMinimum() );
  mSpinMaxDistance->setValue( mProfile->plot()->xMaximum() );
  mSpinMinElevation->setValue( mProfile->plot()->yMinimum() );
  mSpinMaxElevation->setValue( mProfile->plot()->yMaximum() );

  if ( mProfile->plot()->xAxis().gridMajorSymbol() )
    mDistanceAxisMajorLinesSymbolButton->setSymbol( mProfile->plot()->xAxis().gridMajorSymbol()->clone() );
  if ( mProfile->plot()->xAxis().gridMinorSymbol() )
    mDistanceAxisMinorLinesSymbolButton->setSymbol( mProfile->plot()->xAxis().gridMinorSymbol()->clone() );
  if ( mProfile->plot()->yAxis().gridMajorSymbol() )
    mElevationAxisMajorLinesSymbolButton->setSymbol( mProfile->plot()->yAxis().gridMajorSymbol()->clone() );
  if ( mProfile->plot()->yAxis().gridMajorSymbol() )
    mElevationAxisMinorLinesSymbolButton->setSymbol( mProfile->plot()->yAxis().gridMinorSymbol()->clone() );

  mDistanceAxisLabelFontButton->setTextFormat( mProfile->plot()->xAxis().textFormat() );
  mElevationAxisLabelFontButton->setTextFormat( mProfile->plot()->yAxis().textFormat() );

  mDistanceAxisMajorIntervalSpin->setValue( mProfile->plot()->xAxis().gridIntervalMajor() );
  mDistanceAxisMinorIntervalSpin->setValue( mProfile->plot()->xAxis().gridIntervalMinor() );
  mDistanceAxisLabelIntervalSpin->setValue( mProfile->plot()->xAxis().labelInterval() );

  mElevationAxisMajorIntervalSpin->setValue( mProfile->plot()->yAxis().gridIntervalMajor() );
  mElevationAxisMinorIntervalSpin->setValue( mProfile->plot()->yAxis().gridIntervalMinor() );
  mElevationAxisLabelIntervalSpin->setValue( mProfile->plot()->yAxis().labelInterval() );

  if ( mProfile->plot()->chartBackgroundSymbol() )
    mChartBackgroundSymbolButton->setSymbol( mProfile->plot()->chartBackgroundSymbol()->clone() );
  if ( mProfile->plot()->chartBorderSymbol() )
    mChartBorderSymbolButton->setSymbol( mProfile->plot()->chartBorderSymbol()->clone() );

  mSpinLeftMargin->setValue( mProfile->plot()->margins().left() );
  mSpinRightMargin->setValue( mProfile->plot()->margins().right() );
  mSpinTopMargin->setValue( mProfile->plot()->margins().top() );
  mSpinBottomMargin->setValue( mProfile->plot()->margins().bottom() );

  const QList<QgsLayerTreeLayer *>  layers = mLayerTree->findLayers();
  for ( QgsLayerTreeLayer *layer : layers )
  {
    layer->setItemVisibilityChecked( mProfile->layers().contains( layer->layer() ) );
  }
  mLayerTree->reorderGroupLayers( mProfile->layers() );

  updateDataDefinedButton( mDDBtnMinDistance );
  updateDataDefinedButton( mDDBtnMaxDistance );
  updateDataDefinedButton( mDDBtnMinElevation );
  updateDataDefinedButton( mDDBtnMaxElevation );
  updateDataDefinedButton( mDDBtnDistanceMajorInterval );
  updateDataDefinedButton( mDDBtnDistanceMinorInterval );
  updateDataDefinedButton( mDDBtnDistanceLabelInterval );
  updateDataDefinedButton( mDDBtnElevationMajorInterval );
  updateDataDefinedButton( mDDBtnElevationMinorInterval );
  updateDataDefinedButton( mDDBtnElevationLabelInterval );
  updateDataDefinedButton( mDDBtnLeftMargin );
  updateDataDefinedButton( mDDBtnRightMargin );
  updateDataDefinedButton( mDDBtnTopMargin );
  updateDataDefinedButton( mDDBtnBottomMargin );

  mBlockChanges--;
}

void QgsLayoutElevationProfileWidget::updateItemLayers()
{
  if ( mBlockChanges )
    return;

  QList<QgsMapLayer *> layers;
  const QList< QgsMapLayer * > layerOrder = mLayerTree->layerOrder();
  layers.reserve( layerOrder.size() );
  for ( QgsMapLayer *layer : layerOrder )
  {
    if ( mLayerTree->findLayer( layer )->isVisible() )
      layers << layer;
  }

  mProfile->setLayers( layers );
  mProfile->update();
}
