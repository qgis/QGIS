/***************************************************************************
  qgsdatadefinedsizelegenddialog.cpp
  --------------------------------------
  Date                 : June 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatadefinedsizelegenddialog.h"

#include "qgsdatadefinedsizelegend.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgsmapcanvas.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbolselectordialog.h"
#include "qgsvectorlayer.h"

QgsDataDefinedSizeLegendDialog::QgsDataDefinedSizeLegendDialog( const QgsDataDefinedSizeLegend *ddsLegend, const QgsProperty &ddSize, QgsMarkerSymbol *overrideSymbol, QgsMapCanvas *canvas, QWidget *parent )
  : QDialog( parent )
  , mSizeProperty( ddSize )
  , mMapCanvas( canvas )
{
  setupUi( this );

  QgsMarkerSymbol *symbol = nullptr;

  if ( !ddsLegend )
  {
    radDisabled->setChecked( true );
  }
  else
  {
    if ( ddsLegend->legendType() == QgsDataDefinedSizeLegend::LegendSeparated )
      radSeparated->setChecked( true );
    else
      radCollapsed->setChecked( true );

    if ( ddsLegend->verticalAlignment() == QgsDataDefinedSizeLegend::AlignBottom )
      cboAlignSymbols->setCurrentIndex( 0 );
    else
      cboAlignSymbols->setCurrentIndex( 1 );

    symbol = ddsLegend->symbol();  // may be null (undefined)
  }

  if ( overrideSymbol )
  {
    symbol = overrideSymbol;   // takes ownership
    mOverrideSymbol = true;
  }

  if ( !symbol )
  {
    symbol = QgsMarkerSymbol::createSimple( QgsStringMap() );
  }
  mSourceSymbol.reset( symbol );

  btnChangeSymbol->setEnabled( !mOverrideSymbol );

  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mSourceSymbol.get(), btnChangeSymbol->iconSize() );
  btnChangeSymbol->setIcon( icon );

  // prepare layer and model to preview legend
  mPreviewLayer = new QgsVectorLayer( "Point?crs=EPSG:4326", "Preview", "memory" );
  mPreviewTree = new QgsLayerTree;
  mPreviewLayerNode = mPreviewTree->addLayer( mPreviewLayer );  // node owned by the tree
  mPreviewModel = new QgsLayerTreeModel( mPreviewTree );
  if ( canvas )
    mPreviewModel->setLegendMapViewData( canvas->mapUnitsPerPixel(), canvas->mapSettings().outputDpi(), canvas->scale() );
  viewLayerTree->setModel( mPreviewModel );

  connect( cboAlignSymbols, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [ = ] { updatePreview(); } );
  connect( radDisabled, &QRadioButton::clicked, this, &QgsDataDefinedSizeLegendDialog::updatePreview );
  connect( radSeparated, &QRadioButton::clicked, this, &QgsDataDefinedSizeLegendDialog::updatePreview );
  connect( radCollapsed, &QRadioButton::clicked, this, &QgsDataDefinedSizeLegendDialog::updatePreview );
  connect( btnChangeSymbol, &QPushButton::clicked, this, &QgsDataDefinedSizeLegendDialog::changeSymbol );
  updatePreview();
}

QgsDataDefinedSizeLegendDialog::~QgsDataDefinedSizeLegendDialog()
{
  delete mPreviewModel;
  delete mPreviewTree;
  delete mPreviewLayer;
}

QgsDataDefinedSizeLegend *QgsDataDefinedSizeLegendDialog::dataDefinedSizeLegend() const
{
  if ( radDisabled->isChecked() )
    return nullptr;

  QgsDataDefinedSizeLegend *ddsLegend = new QgsDataDefinedSizeLegend;
  ddsLegend->setLegendType( radSeparated->isChecked() ? QgsDataDefinedSizeLegend::LegendSeparated : QgsDataDefinedSizeLegend::LegendCollapsed );
  ddsLegend->setVerticalAlignment( cboAlignSymbols->currentIndex() == 0 ? QgsDataDefinedSizeLegend::AlignBottom : QgsDataDefinedSizeLegend::AlignCenter );
  if ( !mOverrideSymbol )
  {
    ddsLegend->setSymbol( mSourceSymbol->clone() );
  }
  return ddsLegend;
}

void QgsDataDefinedSizeLegendDialog::updatePreview()
{
  QgsMarkerSymbol *symbol = mSourceSymbol->clone();
  symbol->setDataDefinedSize( mSizeProperty );
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( symbol );
  r->setDataDefinedSizeLegend( dataDefinedSizeLegend() );
  mPreviewLayer->setRenderer( r );
  mPreviewModel->refreshLayerLegend( mPreviewLayerNode );
  viewLayerTree->expandAll();
}

void QgsDataDefinedSizeLegendDialog::changeSymbol()
{
  std::unique_ptr<QgsMarkerSymbol> newSymbol( mSourceSymbol->clone() );
  QgsSymbolWidgetContext context;
  if ( mMapCanvas )
    context.setMapCanvas( mMapCanvas );

  QgsExpressionContext ec;
  ec << QgsExpressionContextUtils::globalScope()
     << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
     << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mMapCanvas )
    ec << QgsExpressionContextUtils::mapSettingsScope( mMapCanvas->mapSettings() );
  context.setExpressionContext( &ec );

  std::unique_ptr<QgsVectorLayer> layer( new QgsVectorLayer( "Point", "tmp", "memory" ) );

  QgsSymbolSelectorDialog d( newSymbol.get(), QgsStyle::defaultStyle(), layer.get(), this );
  d.setContext( context );

  if ( d.exec() != QDialog::Accepted )
    return;

  mSourceSymbol.reset( newSymbol.release() );
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mSourceSymbol.get(), btnChangeSymbol->iconSize() );
  btnChangeSymbol->setIcon( icon );

  updatePreview();
}
