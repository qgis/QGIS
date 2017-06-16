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
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgsvectorlayer.h"

QgsDataDefinedSizeLegendDialog::QgsDataDefinedSizeLegendDialog( const QgsDataDefinedSizeLegend *ddsLegend, QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

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
  }

  // prepare default source symbol - it should be hopefully replaced in setSourceSymbol() later
  mSourceSymbol.reset( static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );
  QgsProperty property = QgsProperty::fromValue( 1 );
  property.setTransformer( new QgsSizeScaleTransformer( QgsSizeScaleTransformer::Linear, 0, 1, 0, 20 ) );
  mSourceSymbol->setDataDefinedSize( property );

  // prepare layer and model to preview legend
  mPreviewLayer = new QgsVectorLayer( "Point?crs=EPSG:4326", "Preview", "memory" );
  mPreviewTree = new QgsLayerTree;
  mPreviewLayerNode = mPreviewTree->addLayer( mPreviewLayer );  // node owned by the tree
  mPreviewModel = new QgsLayerTreeModel( mPreviewTree );
  viewLayerTree->setModel( mPreviewModel );

  connect( cboAlignSymbols, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [ = ] { updatePreview(); } );
  connect( radDisabled, &QRadioButton::clicked, this, &QgsDataDefinedSizeLegendDialog::updatePreview );
  connect( radSeparated, &QRadioButton::clicked, this, &QgsDataDefinedSizeLegendDialog::updatePreview );
  connect( radCollapsed, &QRadioButton::clicked, this, &QgsDataDefinedSizeLegendDialog::updatePreview );
  updatePreview();
}

QgsDataDefinedSizeLegendDialog::~QgsDataDefinedSizeLegendDialog()
{
  delete mPreviewModel;
  delete mPreviewTree;
  delete mPreviewLayer;
}

void QgsDataDefinedSizeLegendDialog::setSourceSymbol( QgsMarkerSymbol *symbol )
{
  mSourceSymbol.reset( symbol );
  updatePreview();
}

void QgsDataDefinedSizeLegendDialog::setLegendMapViewData( double mapUnitsPerPixel, int dpi, double scale )
{
  mPreviewModel->setLegendMapViewData( mapUnitsPerPixel, dpi, scale );
  updatePreview();
}

QgsDataDefinedSizeLegend *QgsDataDefinedSizeLegendDialog::dataDefinedSizeLegend() const
{
  if ( radDisabled->isChecked() )
    return nullptr;

  QgsDataDefinedSizeLegend *ddsLegend = new QgsDataDefinedSizeLegend;
  ddsLegend->setLegendType( radSeparated->isChecked() ? QgsDataDefinedSizeLegend::LegendSeparated : QgsDataDefinedSizeLegend::LegendCollapsed );
  ddsLegend->setVerticalAlignment( cboAlignSymbols->currentIndex() == 0 ? QgsDataDefinedSizeLegend::AlignBottom : QgsDataDefinedSizeLegend::AlignCenter );
  return ddsLegend;
}

void QgsDataDefinedSizeLegendDialog::updatePreview()
{
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( mSourceSymbol->clone() );
  r->setDataDefinedSizeLegend( dataDefinedSizeLegend() );
  mPreviewLayer->setRenderer( r );
  mPreviewModel->refreshLayerLegend( mPreviewLayerNode );
  viewLayerTree->expandAll();
}
