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
#include "qgsdiagramrendererv2.h"


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


void QgsMapLayerLegendUtils::setLegendNodeOrder( QgsLayerTreeLayer* nodeLayer, const QList<int>& order )
{
  QStringList orderStr;
  foreach ( int id, order )
    orderStr << QString::number( id );
  QString str = orderStr.isEmpty() ? "empty" : orderStr.join( "," );

  nodeLayer->setCustomProperty( "legend/node-order", str );
}

static int _originalLegendNodeCount( QgsLayerTreeLayer* nodeLayer )
{
  // this is not particularly efficient way of finding out number of legend nodes
  QList<QgsLayerTreeModelLegendNode*> lst = nodeLayer->layer()->legend()->createLayerTreeModelLegendNodes( nodeLayer );
  int numNodes = lst.count();
  qDeleteAll( lst );
  return numNodes;
}

static QList<int> _makeNodeOrder( QgsLayerTreeLayer* nodeLayer )
{
  if ( !nodeLayer->layer() || !nodeLayer->layer()->legend() )
  {
    QgsDebugMsg( "Legend node order manipulation is invalid without existing legend" );
    return QList<int>();
  }

  int numNodes = _originalLegendNodeCount( nodeLayer );

  QList<int> order;
  for ( int i = 0; i < numNodes; ++i )
    order << i;
  return order;
}

QList<int> QgsMapLayerLegendUtils::legendNodeOrder( QgsLayerTreeLayer* nodeLayer )
{
  QString orderStr = nodeLayer->customProperty( "legend/node-order" ).toString();

  if ( orderStr.isEmpty() )
    return _makeNodeOrder( nodeLayer );

  if ( orderStr == "empty" )
    return QList<int>();

  int numNodes = _originalLegendNodeCount( nodeLayer );

  QList<int> lst;
  foreach ( QString item, orderStr.split( "," ) )
  {
    bool ok;
    int id = item.toInt( &ok );
    if ( !ok || id < 0 || id >= numNodes )
      return _makeNodeOrder( nodeLayer );

    lst << id;
  }

  return lst;
}

bool QgsMapLayerLegendUtils::hasLegendNodeOrder( QgsLayerTreeLayer* nodeLayer )
{
  return nodeLayer->customProperties().contains( "legend/node-order" );
}

void QgsMapLayerLegendUtils::setLegendNodeUserLabel( QgsLayerTreeLayer* nodeLayer, int originalIndex, const QString& newLabel )
{
  nodeLayer->setCustomProperty( "legend/label-" + QString::number( originalIndex ), newLabel );
}

QString QgsMapLayerLegendUtils::legendNodeUserLabel( QgsLayerTreeLayer* nodeLayer, int originalIndex )
{
  return nodeLayer->customProperty( "legend/label-" + QString::number( originalIndex ) ).toString();
}

bool QgsMapLayerLegendUtils::hasLegendNodeUserLabel( QgsLayerTreeLayer* nodeLayer, int originalIndex )
{
  return nodeLayer->customProperties().contains( "legend/label-" + QString::number( originalIndex ) );
}


void QgsMapLayerLegendUtils::applyLayerNodeProperties( QgsLayerTreeLayer* nodeLayer, QList<QgsLayerTreeModelLegendNode*>& nodes )
{
  // handle user labels
  int i = 0;
  foreach ( QgsLayerTreeModelLegendNode* legendNode, nodes )
  {
    QString userLabel = QgsMapLayerLegendUtils::legendNodeUserLabel( nodeLayer, i++ );
    if ( !userLabel.isNull() )
      legendNode->setUserLabel( userLabel );
  }

  // handle user order of nodes
  if ( QgsMapLayerLegendUtils::hasLegendNodeOrder( nodeLayer ) )
  {
    QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( nodeLayer );

    QList<QgsLayerTreeModelLegendNode*> newOrder;
    QSet<int> usedIndices;
    foreach ( int idx, order )
    {
      if ( usedIndices.contains( idx ) )
      {
        QgsDebugMsg( "invalid node order. ignoring." );
        return;
      }

      newOrder << nodes[idx];
      usedIndices << idx;
    }

    // delete unused nodes
    for ( int i = 0; i < nodes.count(); ++i )
    {
      if ( !usedIndices.contains( i ) )
        delete nodes[i];
    }

    nodes = newOrder;
  }

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
    QgsSymbolV2LegendNode * n = new QgsSymbolV2LegendNode( nodeLayer, i );
    nodes.append( n );
  }

  if ( nodes.count() == 1 && nodes[0]->data( Qt::EditRole ).toString().isEmpty() )
    nodes[0]->setEmbeddedInParent( true );


  if ( mLayer->diagramsEnabled() )
  {
    foreach ( QgsLayerTreeModelLegendNode * i, mLayer->diagramRenderer()->legendItems( nodeLayer ) )
    {
      nodes.append( i );
    }
  }


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
    nodes << new QgsWMSLegendNode( nodeLayer );
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

