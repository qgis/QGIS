/***************************************************************************
  qgsmaplayerlegend.cpp
  --------------------------------------
  Date                 : July 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerlegend.h"

#include <QSettings>

#include "qgslayertree.h"
#include "qgslayertreemodellegendnode.h"
#include "qgspluginlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"


QgsMapLayerLegend::QgsMapLayerLegend( QObject *parent ) :
    QObject( parent )
{
}

QgsMapLayerLegend* QgsMapLayerLegend::defaultVectorLegend( QgsVectorLayer* vl )
{
  return new QgsDefaultVectorLayerLegend( vl );
}

QgsMapLayerLegend* QgsMapLayerLegend::defaultRasterLegend( QgsRasterLayer* rl )
{
  return new QgsDefaultRasterLayerLegend( rl );
}

QgsMapLayerLegend* QgsMapLayerLegend::defaultPluginLegend( QgsPluginLayer* pl )
{
  return new QgsDefaultPluginLayerLegend( pl );
}


// -------------------------------------------------------------------------


QgsDefaultVectorLayerLegend::QgsDefaultVectorLayerLegend( QgsVectorLayer* vl )
    : mLayer( vl )
{
  connect( mLayer, SIGNAL( rendererChanged() ), this, SIGNAL( itemsChanged() ) );
}

QList<QgsLayerTreeModelLegendNode*> QgsDefaultVectorLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode*> nodes;

  QgsFeatureRendererV2* r = mLayer->rendererV2();
  if ( !r )
    return nodes;

  if ( nodeLayer->customProperty( "showFeatureCount", 0 ).toBool() )
    mLayer->countSymbolFeatures();

  QSettings settings;
  if ( settings.value( "/qgis/showLegendClassifiers", false ).toBool() && !r->legendClassificationAttribute().isEmpty() )
  {
    nodes.append( new QgsSimpleLegendNode( nodeLayer, r->legendClassificationAttribute() ) );
  }

  foreach ( const QgsLegendSymbolItemV2& i, r->legendSymbolItemsV2() )
  {
    nodes.append( new QgsSymbolV2LegendNode( nodeLayer, i ) );
  }

  if ( nodes.count() == 1 && nodes[0]->data( Qt::EditRole ).toString().isEmpty() )
    nodes[0]->setEmbeddedInParent( true );

  return nodes;
}


// -------------------------------------------------------------------------


QgsDefaultRasterLayerLegend::QgsDefaultRasterLayerLegend( QgsRasterLayer* rl )
    : mLayer( rl )
{
  connect( mLayer, SIGNAL( rendererChanged() ), this, SIGNAL( itemsChanged() ) );
}

QList<QgsLayerTreeModelLegendNode*> QgsDefaultRasterLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode*> nodes;

  // temporary solution for WMS. Ideally should be done with a delegate.
  if ( mLayer->providerType() == "wms" )
  {
    QImage legendGraphic = mLayer->dataProvider()->getLegendGraphic();
    if ( !legendGraphic.isNull() )
    {
      QgsDebugMsg( QString( "downloaded legend with dimension width:" ) + QString::number( legendGraphic.width() ) + QString( " and Height:" ) + QString::number( legendGraphic.height() ) );
      nodes << new QgsImageLegendNode( nodeLayer, legendGraphic );
    }
  }

  QgsLegendColorList rasterItemList = mLayer->legendSymbologyItems();
  if ( rasterItemList.count() == 0 )
    return nodes;

  // Paletted raster may have many colors, for example UInt16 may have 65536 colors
  // and it is very slow, so we limit max count
  int count = 0;
  int max_count = 1000;

  for ( QgsLegendColorList::const_iterator itemIt = rasterItemList.constBegin();
        itemIt != rasterItemList.constEnd(); ++itemIt, ++count )
  {
    nodes << new QgsRasterSymbolLegendNode( nodeLayer, itemIt->second, itemIt->first );

    if ( count == max_count )
    {
      QString label = tr( "following %1 items\nnot displayed" ).arg( rasterItemList.size() - max_count );
      nodes << new QgsSimpleLegendNode( nodeLayer, label );
      break;
    }
  }

  return nodes;
}


// -------------------------------------------------------------------------


QgsDefaultPluginLayerLegend::QgsDefaultPluginLayerLegend( QgsPluginLayer* pl )
    : mLayer( pl )
{
}

QList<QgsLayerTreeModelLegendNode*> QgsDefaultPluginLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode*> nodes;

  QSize iconSize( 16, 16 );
  QgsLegendSymbologyList symbologyList = mLayer->legendSymbologyItems( iconSize );

  if ( symbologyList.count() == 0 )
    return nodes;

  typedef QPair<QString, QPixmap> XY;
  foreach ( XY item, symbologyList )
  {
    nodes << new QgsSimpleLegendNode( nodeLayer, item.first, QIcon( item.second ) );
  }

  return nodes;
}

