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

#include "qgssettings.h"
#include "qgslayertree.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsmeshlayer.h"
#include "qgspluginlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsdiagramrenderer.h"
#include "qgssymbollayerutils.h"

QgsMapLayerLegend::QgsMapLayerLegend( QObject *parent )
  : QObject( parent )
{
}

void QgsMapLayerLegend::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( elem )
  Q_UNUSED( context )
}

QDomElement QgsMapLayerLegend::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( doc )
  Q_UNUSED( context )
  return QDomElement();
}

QgsMapLayerLegend *QgsMapLayerLegend::defaultVectorLegend( QgsVectorLayer *vl )
{
  return new QgsDefaultVectorLayerLegend( vl );
}

QgsMapLayerLegend *QgsMapLayerLegend::defaultRasterLegend( QgsRasterLayer *rl )
{
  return new QgsDefaultRasterLayerLegend( rl );
}

QgsMapLayerLegend *QgsMapLayerLegend::defaultMeshLegend( QgsMeshLayer *ml )
{
  return new QgsDefaultMeshLayerLegend( ml );
}

// -------------------------------------------------------------------------


void QgsMapLayerLegendUtils::setLegendNodeOrder( QgsLayerTreeLayer *nodeLayer, const QList<int> &order )
{
  QStringList orderStr;
  const auto constOrder = order;
  for ( int id : constOrder )
    orderStr << QString::number( id );
  QString str = orderStr.isEmpty() ? QStringLiteral( "empty" ) : orderStr.join( QStringLiteral( "," ) );

  nodeLayer->setCustomProperty( QStringLiteral( "legend/node-order" ), str );
}

static int _originalLegendNodeCount( QgsLayerTreeLayer *nodeLayer )
{
  // this is not particularly efficient way of finding out number of legend nodes
  QList<QgsLayerTreeModelLegendNode *> lst = nodeLayer->layer()->legend()->createLayerTreeModelLegendNodes( nodeLayer );
  int numNodes = lst.count();
  qDeleteAll( lst );
  return numNodes;
}

static QList<int> _makeNodeOrder( QgsLayerTreeLayer *nodeLayer )
{
  if ( !nodeLayer->layer() || !nodeLayer->layer()->legend() )
  {
    QgsDebugMsg( QStringLiteral( "Legend node order manipulation is invalid without existing legend" ) );
    return QList<int>();
  }

  int numNodes = _originalLegendNodeCount( nodeLayer );

  QList<int> order;
  order.reserve( numNodes );
  for ( int i = 0; i < numNodes; ++i )
    order << i;
  return order;
}

QList<int> QgsMapLayerLegendUtils::legendNodeOrder( QgsLayerTreeLayer *nodeLayer )
{
  QString orderStr = nodeLayer->customProperty( QStringLiteral( "legend/node-order" ) ).toString();

  if ( orderStr.isEmpty() )
    return _makeNodeOrder( nodeLayer );

  if ( orderStr == QLatin1String( "empty" ) )
    return QList<int>();

  int numNodes = _originalLegendNodeCount( nodeLayer );

  QList<int> lst;
  const auto constSplit = orderStr.split( ',' );
  for ( const QString &item : constSplit )
  {
    bool ok;
    int id = item.toInt( &ok );
    if ( !ok || id < 0 || id >= numNodes )
      return _makeNodeOrder( nodeLayer );

    lst << id;
  }

  return lst;
}

bool QgsMapLayerLegendUtils::hasLegendNodeOrder( QgsLayerTreeLayer *nodeLayer )
{
  return nodeLayer->customProperties().contains( QStringLiteral( "legend/node-order" ) );
}

void QgsMapLayerLegendUtils::setLegendNodeUserLabel( QgsLayerTreeLayer *nodeLayer, int originalIndex, const QString &newLabel )
{
  nodeLayer->setCustomProperty( "legend/label-" + QString::number( originalIndex ), newLabel );
}

QString QgsMapLayerLegendUtils::legendNodeUserLabel( QgsLayerTreeLayer *nodeLayer, int originalIndex )
{
  return nodeLayer->customProperty( "legend/label-" + QString::number( originalIndex ) ).toString();
}

bool QgsMapLayerLegendUtils::hasLegendNodeUserLabel( QgsLayerTreeLayer *nodeLayer, int originalIndex )
{
  return nodeLayer->customProperties().contains( "legend/label-" + QString::number( originalIndex ) );
}

void QgsMapLayerLegendUtils::setLegendNodePatchShape( QgsLayerTreeLayer *nodeLayer, int originalIndex, const QgsLegendPatchShape &shape )
{
  QDomDocument patchDoc;
  QDomElement patchElem = patchDoc.createElement( QStringLiteral( "patch" ) );
  shape.writeXml( patchElem, patchDoc, QgsReadWriteContext() );
  patchDoc.appendChild( patchElem );
  nodeLayer->setCustomProperty( "legend/patch-shape-" + QString::number( originalIndex ), patchDoc.toString() );
}

QgsLegendPatchShape QgsMapLayerLegendUtils::legendNodePatchShape( QgsLayerTreeLayer *nodeLayer, int originalIndex )
{
  QString patchDef = nodeLayer->customProperty( "legend/patch-shape-" + QString::number( originalIndex ) ).toString();
  if ( patchDef.isEmpty() )
    return QgsLegendPatchShape();

  QDomDocument doc( QStringLiteral( "patch" ) );
  doc.setContent( patchDef );
  QgsLegendPatchShape shape;
  shape.readXml( doc.documentElement(), QgsReadWriteContext() );
  return shape;
}

void QgsMapLayerLegendUtils::setLegendNodeSymbolSize( QgsLayerTreeLayer *nodeLayer, int originalIndex, QSizeF size )
{
  if ( size.isValid() )
    nodeLayer->setCustomProperty( "legend/symbol-size-" + QString::number( originalIndex ), QgsSymbolLayerUtils::encodeSize( size ) );
  else
    nodeLayer->removeCustomProperty( "legend/symbol-size-" + QString::number( originalIndex ) );
}

QSizeF QgsMapLayerLegendUtils::legendNodeSymbolSize( QgsLayerTreeLayer *nodeLayer, int originalIndex )
{
  const QString size = nodeLayer->customProperty( "legend/symbol-size-" + QString::number( originalIndex ) ).toString();
  if ( size.isEmpty() )
    return QSizeF();
  else
    return QgsSymbolLayerUtils::decodeSize( size );
}

void QgsMapLayerLegendUtils::applyLayerNodeProperties( QgsLayerTreeLayer *nodeLayer, QList<QgsLayerTreeModelLegendNode *> &nodes )
{
  // handle user labels
  int i = 0;
  const auto constNodes = nodes;
  for ( QgsLayerTreeModelLegendNode *legendNode : constNodes )
  {
    QString userLabel = QgsMapLayerLegendUtils::legendNodeUserLabel( nodeLayer, i );
    if ( !userLabel.isNull() )
      legendNode->setUserLabel( userLabel );

    if ( QgsSymbolLegendNode *symbolNode = dynamic_cast< QgsSymbolLegendNode * >( legendNode ) )
    {
      const QgsLegendPatchShape shape = QgsMapLayerLegendUtils::legendNodePatchShape( nodeLayer, i );
      symbolNode->setPatchShape( shape );
    }

    const QSizeF userSize = QgsMapLayerLegendUtils::legendNodeSymbolSize( nodeLayer, i );
    if ( userSize.isValid() )
    {
      legendNode->setUserPatchSize( userSize );
    }

    i++;
  }

  // handle user order of nodes
  if ( QgsMapLayerLegendUtils::hasLegendNodeOrder( nodeLayer ) )
  {
    QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( nodeLayer );

    QList<QgsLayerTreeModelLegendNode *> newOrder;
    QSet<int> usedIndices;
    const auto constOrder = order;
    for ( int idx : constOrder )
    {
      if ( usedIndices.contains( idx ) )
      {
        QgsDebugMsg( QStringLiteral( "invalid node order. ignoring." ) );
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


QgsDefaultVectorLayerLegend::QgsDefaultVectorLayerLegend( QgsVectorLayer *vl )
  : mLayer( vl )
{
  connect( mLayer, &QgsMapLayer::rendererChanged, this, &QgsMapLayerLegend::itemsChanged );
}

QList<QgsLayerTreeModelLegendNode *> QgsDefaultVectorLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  QgsFeatureRenderer *r = mLayer->renderer();
  if ( !r )
    return nodes;

  if ( nodeLayer->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toBool() )
    mLayer->countSymbolFeatures();

  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/showLegendClassifiers" ), false ).toBool() && !r->legendClassificationAttribute().isEmpty() )
  {
    nodes.append( new QgsSimpleLegendNode( nodeLayer, r->legendClassificationAttribute() ) );
  }

  const auto constLegendSymbolItems = r->legendSymbolItems();
  for ( const QgsLegendSymbolItem &i : constLegendSymbolItems )
  {
    if ( i.dataDefinedSizeLegendSettings() )
      nodes << new QgsDataDefinedSizeLegendNode( nodeLayer, *i.dataDefinedSizeLegendSettings() );
    else
    {
      QgsSymbolLegendNode *legendNode = new QgsSymbolLegendNode( nodeLayer, i );
      if ( mTextOnSymbolEnabled && mTextOnSymbolContent.contains( i.ruleKey() ) )
      {
        legendNode->setTextOnSymbolLabel( mTextOnSymbolContent.value( i.ruleKey() ) );
        legendNode->setTextOnSymbolTextFormat( mTextOnSymbolTextFormat );
      }
      nodes << legendNode;
    }
  }

  if ( nodes.count() == 1 && nodes[0]->data( Qt::EditRole ).toString().isEmpty() )
    nodes[0]->setEmbeddedInParent( true );


  if ( mLayer->diagramsEnabled() )
  {
    const auto constLegendItems = mLayer->diagramRenderer()->legendItems( nodeLayer );
    for ( QgsLayerTreeModelLegendNode *i : constLegendItems )
    {
      nodes.append( i );
    }
  }


  return nodes;
}

void QgsDefaultVectorLayerLegend::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mTextOnSymbolEnabled = false;
  mTextOnSymbolTextFormat = QgsTextFormat();
  mTextOnSymbolContent.clear();

  QDomElement tosElem = elem.firstChildElement( QStringLiteral( "text-on-symbol" ) );
  if ( !tosElem.isNull() )
  {
    mTextOnSymbolEnabled = true;
    QDomElement tosFormatElem = tosElem.firstChildElement( QStringLiteral( "text-style" ) );
    mTextOnSymbolTextFormat.readXml( tosFormatElem, context );
    QDomElement tosContentElem = tosElem.firstChildElement( QStringLiteral( "content" ) );
    QDomElement tosContentItemElem = tosContentElem.firstChildElement( QStringLiteral( "item" ) );
    while ( !tosContentItemElem.isNull() )
    {
      mTextOnSymbolContent.insert( tosContentItemElem.attribute( QStringLiteral( "key" ) ), tosContentItemElem.attribute( QStringLiteral( "value" ) ) );
      tosContentItemElem = tosContentItemElem.nextSiblingElement( QStringLiteral( "item" ) );
    }
  }
}

QDomElement QgsDefaultVectorLayerLegend::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "legend" ) );
  elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "default-vector" ) );

  if ( mTextOnSymbolEnabled )
  {
    QDomElement tosElem = doc.createElement( QStringLiteral( "text-on-symbol" ) );
    QDomElement tosFormatElem = mTextOnSymbolTextFormat.writeXml( doc, context );
    tosElem.appendChild( tosFormatElem );
    QDomElement tosContentElem = doc.createElement( QStringLiteral( "content" ) );
    for ( auto it = mTextOnSymbolContent.constBegin(); it != mTextOnSymbolContent.constEnd(); ++it )
    {
      QDomElement tosContentItemElem = doc.createElement( QStringLiteral( "item" ) );
      tosContentItemElem.setAttribute( QStringLiteral( "key" ), it.key() );
      tosContentItemElem.setAttribute( QStringLiteral( "value" ), it.value() );
      tosContentElem.appendChild( tosContentItemElem );
    }
    tosElem.appendChild( tosContentElem );
    elem.appendChild( tosElem );
  }

  return elem;
}


// -------------------------------------------------------------------------


QgsDefaultRasterLayerLegend::QgsDefaultRasterLayerLegend( QgsRasterLayer *rl )
  : mLayer( rl )
{
  connect( mLayer, &QgsMapLayer::rendererChanged, this, &QgsMapLayerLegend::itemsChanged );
}

QList<QgsLayerTreeModelLegendNode *> QgsDefaultRasterLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  // temporary solution for WMS. Ideally should be done with a delegate.
  if ( mLayer->dataProvider() && mLayer->dataProvider()->supportsLegendGraphic() )
  {
    nodes << new QgsWmsLegendNode( nodeLayer );
  }

  QgsLegendColorList rasterItemList = mLayer->legendSymbologyItems();
  if ( rasterItemList.isEmpty() )
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

QgsDefaultMeshLayerLegend::QgsDefaultMeshLayerLegend( QgsMeshLayer *ml )
  : mLayer( ml )
{
  connect( mLayer, &QgsMapLayer::rendererChanged, this, &QgsMapLayerLegend::itemsChanged );
}

QList<QgsLayerTreeModelLegendNode *> QgsDefaultMeshLayerLegend::createLayerTreeModelLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  QgsMeshDataProvider *provider = mLayer->dataProvider();
  if ( !provider )
    return nodes;

  QgsMeshRendererSettings rendererSettings = mLayer->rendererSettings();

  int indexScalar = rendererSettings.activeScalarDatasetGroup();
  int indexVector = rendererSettings.activeVectorDatasetGroup();

  QString name;
  if ( indexScalar > -1 && indexVector > -1 && indexScalar != indexVector )
    name = QString( "%1 / %2" ).arg( provider->datasetGroupMetadata( indexScalar ).name(), provider->datasetGroupMetadata( indexVector ).name() );
  else if ( indexScalar > -1 )
    name = provider->datasetGroupMetadata( indexScalar ).name();
  else if ( indexVector > -1 )
    name = provider->datasetGroupMetadata( indexVector ).name();
  else
  {
    // neither contours nor vectors get rendered - no legend needed
    return nodes;
  }

  nodes << new QgsSimpleLegendNode( nodeLayer, name );

  if ( indexScalar > -1 )
  {
    QgsMeshRendererScalarSettings settings = rendererSettings.scalarSettings( indexScalar );
    QgsLegendColorList items;
    settings.colorRampShader().legendSymbologyItems( items );
    for ( const QPair< QString, QColor > &item : qgis::as_const( items ) )
    {
      nodes << new QgsRasterSymbolLegendNode( nodeLayer, item.second, item.first );
    }
  }

  return nodes;
}
