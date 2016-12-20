/***************************************************************************
  qgslayertreemapcanvasbridge.cpp
  --------------------------------------
  Date                 : May 2014
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

#include "qgslayertreemapcanvasbridge.h"

#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsproject.h"

QgsLayerTreeMapCanvasBridge::QgsLayerTreeMapCanvasBridge( QgsLayerTreeGroup *root, QgsMapCanvas *canvas, QObject* parent )
    : QObject( parent )
    , mRoot( root )
    , mCanvas( canvas )
    , mOverviewCanvas( nullptr )
    , mPendingCanvasUpdate( false )
    , mHasCustomLayerOrder( false )
    , mAutoSetupOnFirstLayer( true )
    , mAutoEnableCrsTransform( true )
    , mLastLayerCount( !root->findLayers().isEmpty() )
{
  connect( root, SIGNAL( addedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeAddedChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( root, SIGNAL( customPropertyChanged( QgsLayerTreeNode*, QString ) ), this, SLOT( nodeCustomPropertyChanged( QgsLayerTreeNode*, QString ) ) );
  connect( root, SIGNAL( removedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeRemovedChildren() ) );
  connect( root, &QgsLayerTreeNode::visibilityChanged, this, &QgsLayerTreeMapCanvasBridge::nodeVisibilityChanged );

  setCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::clear()
{
  setHasCustomLayerOrder( false );
  setCustomLayerOrder( defaultLayerOrder() );
}

QStringList QgsLayerTreeMapCanvasBridge::defaultLayerOrder() const
{
  QStringList order;
  defaultLayerOrder( mRoot, order );
  return order;
}

void QgsLayerTreeMapCanvasBridge::defaultLayerOrder( QgsLayerTreeNode* node, QStringList& order ) const
{
  if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
    order << nodeLayer->layerId();
  }

  Q_FOREACH ( QgsLayerTreeNode* child, node->children() )
    defaultLayerOrder( child, order );
}


void QgsLayerTreeMapCanvasBridge::setHasCustomLayerOrder( bool state )
{
  if ( mHasCustomLayerOrder == state )
    return;

  mHasCustomLayerOrder = state;
  emit hasCustomLayerOrderChanged( mHasCustomLayerOrder );

  deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::setCustomLayerOrder( const QStringList& order )
{
  if ( mCustomLayerOrder == order )
    return;

  // verify that the new order is correct
  QStringList defOrder( defaultLayerOrder() );
  QStringList newOrder( order );
  QStringList sortedNewOrder( order );
  qSort( defOrder );
  qSort( sortedNewOrder );

  if ( defOrder.size() < sortedNewOrder.size() )
  {
    // might contain bad layers, but also duplicates
    QSet<QString> ids( defOrder.toSet() );

    for ( int i = 0; i < sortedNewOrder.size(); i++ )
    {
      if ( !ids.contains( sortedNewOrder[i] ) )
      {
        newOrder.removeAll( sortedNewOrder[i] );
        sortedNewOrder.removeAt( i-- );
      }
    }
  }

  if ( defOrder != sortedNewOrder )
    return; // must be permutation of the default order

  mCustomLayerOrder = newOrder;
  emit customLayerOrderChanged( mCustomLayerOrder );

  if ( mHasCustomLayerOrder )
    deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::setCanvasLayers()
{
  QList<QgsMapLayer*> canvasLayers, overviewLayers;

  if ( mHasCustomLayerOrder )
  {
    Q_FOREACH ( const QString& layerId, mCustomLayerOrder )
    {
      QgsLayerTreeLayer* nodeLayer = mRoot->findLayer( layerId );
      if ( nodeLayer )
      {
        if ( nodeLayer->isVisible() )
          canvasLayers << nodeLayer->layer();
        if ( nodeLayer->customProperty( QStringLiteral( "overview" ), 0 ).toInt() )
          overviewLayers << nodeLayer->layer();
      }
    }
  }
  else
    setCanvasLayers( mRoot, canvasLayers, overviewLayers );

  QList<QgsLayerTreeLayer*> layerNodes = mRoot->findLayers();
  int currentLayerCount = layerNodes.count();
  bool firstLayers = mAutoSetupOnFirstLayer && mLastLayerCount == 0 && currentLayerCount != 0;

  if ( firstLayers )
  {
    // also setup destination CRS and map units if the OTF projections are not yet enabled
    if ( !mCanvas->mapSettings().hasCrsTransformEnabled() )
    {
      Q_FOREACH ( QgsLayerTreeLayer* layerNode, layerNodes )
      {
        if ( !layerNode->layer() )
          continue;

        if ( layerNode->layer()->isSpatial() )
        {
          mCanvas->setDestinationCrs( layerNode->layer()->crs() );
          QgsProject::instance()->setCrs( layerNode->layer()->crs() );
          mCanvas->setMapUnits( layerNode->layer()->crs().mapUnits() );
          break;
        }
      }
    }
  }

  mCanvas->setLayers( canvasLayers );
  if ( mOverviewCanvas )
    mOverviewCanvas->setLayers( overviewLayers );

  if ( firstLayers )
  {
    // if we are moving from zero to non-zero layers, let's zoom to those data
    mCanvas->zoomToFullExtent();
  }

  if ( !mFirstCRS.isValid() )
  {
    // find out what is the first used CRS in case we may need to turn on OTF projections later
    Q_FOREACH ( QgsLayerTreeLayer* layerNode, layerNodes )
    {
      if ( layerNode->layer() && layerNode->layer()->crs().isValid() )
      {
        mFirstCRS = layerNode->layer()->crs();
        break;
      }
    }
  }

  if ( mAutoEnableCrsTransform && mFirstCRS.isValid() && !mCanvas->mapSettings().hasCrsTransformEnabled() )
  {
    // check whether all layers still have the same CRS
    Q_FOREACH ( QgsLayerTreeLayer* layerNode, layerNodes )
    {
      if ( layerNode->layer() && layerNode->layer()->crs().isValid() && layerNode->layer()->crs() != mFirstCRS )
      {
        mCanvas->setDestinationCrs( mFirstCRS );
        QgsProject::instance()->setCrs( mFirstCRS );
        mCanvas->setCrsTransformEnabled( true );
        break;
      }
    }
  }

  mLastLayerCount = currentLayerCount;
  if ( currentLayerCount == 0 )
    mFirstCRS = QgsCoordinateReferenceSystem();

  mPendingCanvasUpdate = false;
}

void QgsLayerTreeMapCanvasBridge::readProject( const QDomDocument& doc )
{
  mFirstCRS = QgsCoordinateReferenceSystem(); // invalidate on project load

  QDomElement elem = doc.documentElement().firstChildElement( QStringLiteral( "layer-tree-canvas" ) );
  if ( elem.isNull() )
  {
    bool oldEnabled;
    QStringList oldOrder;
    if ( QgsLayerTreeUtils::readOldLegendLayerOrder( doc.documentElement().firstChildElement( QStringLiteral( "legend" ) ), oldEnabled, oldOrder ) )
    {
      setHasCustomLayerOrder( oldEnabled );
      setCustomLayerOrder( oldOrder );
    }
    return;
  }

  QDomElement customOrderElem = elem.firstChildElement( QStringLiteral( "custom-order" ) );
  if ( !customOrderElem.isNull() )
  {
    QStringList order;
    QDomElement itemElem = customOrderElem.firstChildElement( QStringLiteral( "item" ) );
    while ( !itemElem.isNull() )
    {
      order.append( itemElem.text() );
      itemElem = itemElem.nextSiblingElement( QStringLiteral( "item" ) );
    }

    setHasCustomLayerOrder( customOrderElem.attribute( QStringLiteral( "enabled" ), QString() ).toInt() );
    setCustomLayerOrder( order );
  }
}

void QgsLayerTreeMapCanvasBridge::writeProject( QDomDocument& doc )
{
  QDomElement elem = doc.createElement( QStringLiteral( "layer-tree-canvas" ) );
  QDomElement customOrderElem = doc.createElement( QStringLiteral( "custom-order" ) );
  customOrderElem.setAttribute( QStringLiteral( "enabled" ), mHasCustomLayerOrder ? 1 : 0 );

  Q_FOREACH ( const QString& layerId, mCustomLayerOrder )
  {
    QDomElement itemElem = doc.createElement( QStringLiteral( "item" ) );
    itemElem.appendChild( doc.createTextNode( layerId ) );
    customOrderElem.appendChild( itemElem );
  }
  elem.appendChild( customOrderElem );

  doc.documentElement().appendChild( elem );
}

void QgsLayerTreeMapCanvasBridge::setCanvasLayers( QgsLayerTreeNode *node, QList<QgsMapLayer*> &canvasLayers, QList<QgsMapLayer*>& overviewLayers )
{
  if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
    if ( nodeLayer->isVisible() )
      canvasLayers << nodeLayer->layer();
    if ( nodeLayer->customProperty( QStringLiteral( "overview" ), 0 ).toInt() )
      overviewLayers << nodeLayer->layer();
  }

  Q_FOREACH ( QgsLayerTreeNode* child, node->children() )
    setCanvasLayers( child, canvasLayers, overviewLayers );
}

void QgsLayerTreeMapCanvasBridge::deferredSetCanvasLayers()
{
  if ( mPendingCanvasUpdate )
    return;

  mPendingCanvasUpdate = true;
  QMetaObject::invokeMethod( this, "setCanvasLayers", Qt::QueuedConnection );
}

void QgsLayerTreeMapCanvasBridge::nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );

  // collect layer IDs that have been added in order to put them into custom layer order
  QStringList layerIds;
  QList<QgsLayerTreeNode*> children = node->children();
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode* child = children.at( i );
    if ( QgsLayerTree::isLayer( child ) )
    {
      layerIds << QgsLayerTree::toLayer( child )->layerId();
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      Q_FOREACH ( QgsLayerTreeLayer* nodeL, QgsLayerTree::toGroup( child )->findLayers() )
        layerIds << nodeL->layerId();
    }
  }

  Q_FOREACH ( const QString& layerId, layerIds )
  {
    if ( !mCustomLayerOrder.contains( layerId ) )
      mCustomLayerOrder.append( layerId );
  }

  emit customLayerOrderChanged( mCustomLayerOrder );

  deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::nodeRemovedChildren()
{
  // no need to disconnect from removed nodes as they are deleted

  // check whether the layers are still there, if not, remove them from the layer order!
  QList<int> toRemove;
  for ( int i = 0; i < mCustomLayerOrder.count(); ++i )
  {
    QgsLayerTreeLayer* node = mRoot->findLayer( mCustomLayerOrder[i] );
    if ( !node )
      toRemove << i;
  }
  for ( int i = toRemove.count() - 1; i >= 0; --i )
    mCustomLayerOrder.removeAt( toRemove[i] );
  emit customLayerOrderChanged( mCustomLayerOrder );

  deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::nodeVisibilityChanged()
{
  deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::nodeCustomPropertyChanged( QgsLayerTreeNode* node, const QString& key )
{
  Q_UNUSED( node );
  if ( key == QLatin1String( "overview" ) )
    deferredSetCanvasLayers();
}

