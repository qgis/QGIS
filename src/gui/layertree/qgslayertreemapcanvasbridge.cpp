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

QgsLayerTreeMapCanvasBridge::QgsLayerTreeMapCanvasBridge( QgsLayerTreeGroup *root, QgsMapCanvas *canvas, QObject* parent )
    : QObject( parent )
    , mRoot( root )
    , mCanvas( canvas )
    , mPendingCanvasUpdate( false )
    , mHasCustomLayerOrder( false )
    , mAutoSetupOnFirstLayer( true )
    , mAutoEnableCrsTransform( true )
    , mLastLayerCount( root->findLayers().count() )
{
  connect( root, SIGNAL( addedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeAddedChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( root, SIGNAL( customPropertyChanged( QgsLayerTreeNode*, QString ) ), this, SLOT( nodeCustomPropertyChanged( QgsLayerTreeNode*, QString ) ) );
  connect( root, SIGNAL( removedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeRemovedChildren() ) );
  connect( root, SIGNAL( visibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ), this, SLOT( nodeVisibilityChanged() ) );

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

  foreach ( QgsLayerTreeNode* child, node->children() )
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
  QList<QgsMapCanvasLayer> layers;

  if ( mHasCustomLayerOrder )
  {
    foreach ( QString layerId, mCustomLayerOrder )
    {
      QgsLayerTreeLayer* nodeLayer = mRoot->findLayer( layerId );
      if ( nodeLayer )
        layers << QgsMapCanvasLayer( nodeLayer->layer(), nodeLayer->isVisible() == Qt::Checked, nodeLayer->customProperty( "overview", 0 ).toInt() );
    }
  }
  else
    setCanvasLayers( mRoot, layers );

  QList<QgsLayerTreeLayer*> layerNodes = mRoot->findLayers();
  int currentLayerCount = layerNodes.count();
  bool firstLayers = mAutoSetupOnFirstLayer && mLastLayerCount == 0 && currentLayerCount != 0;

  if ( firstLayers )
  {
    // also setup destination CRS and map units if the OTF projections are not yet enabled
    if ( !mCanvas->mapSettings().hasCrsTransformEnabled() )
    {
      foreach ( QgsLayerTreeLayer* layerNode, layerNodes )
      {
        if ( layerNode->layer() &&
             (
               qobject_cast<QgsVectorLayer *>( layerNode->layer() ) == 0 ||
               qobject_cast<QgsVectorLayer *>( layerNode->layer() )->geometryType() != QGis::NoGeometry
             )
           )
        {
          mCanvas->setDestinationCrs( layerNode->layer()->crs() );
          mCanvas->setMapUnits( layerNode->layer()->crs().mapUnits() );
          break;
        }
      }
    }
  }

  mCanvas->setLayerSet( layers );

  if ( firstLayers )
  {
    // if we are moving from zero to non-zero layers, let's zoom to those data
    mCanvas->zoomToFullExtent();
  }

  if ( !mFirstCRS.isValid() )
  {
    // find out what is the first used CRS in case we may need to turn on OTF projections later
    foreach ( QgsLayerTreeLayer* layerNode, layerNodes )
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
    foreach ( QgsLayerTreeLayer* layerNode, layerNodes )
    {
      if ( layerNode->layer() && layerNode->layer()->crs().isValid() && layerNode->layer()->crs() != mFirstCRS )
      {
        mCanvas->setDestinationCrs( mFirstCRS );
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

  QDomElement elem = doc.documentElement().firstChildElement( "layer-tree-canvas" );
  if ( elem.isNull() )
  {
    bool oldEnabled;
    QStringList oldOrder;
    if ( QgsLayerTreeUtils::readOldLegendLayerOrder( doc.documentElement().firstChildElement( "legend" ), oldEnabled, oldOrder ) )
    {
      setHasCustomLayerOrder( oldEnabled );
      setCustomLayerOrder( oldOrder );
    }
    return;
  }

  QDomElement customOrderElem = elem.firstChildElement( "custom-order" );
  if ( !customOrderElem.isNull() )
  {
    QStringList order;
    QDomElement itemElem = customOrderElem.firstChildElement( "item" );
    while ( !itemElem.isNull() )
    {
      order.append( itemElem.text() );
      itemElem = itemElem.nextSiblingElement( "item" );
    }

    setHasCustomLayerOrder( customOrderElem.attribute( "enabled", 0 ).toInt() );
    setCustomLayerOrder( order );
  }
}

void QgsLayerTreeMapCanvasBridge::writeProject( QDomDocument& doc )
{
  QDomElement elem = doc.createElement( "layer-tree-canvas" );
  QDomElement customOrderElem = doc.createElement( "custom-order" );
  customOrderElem.setAttribute( "enabled", mHasCustomLayerOrder ? 1 : 0 );

  foreach ( QString layerId, mCustomLayerOrder )
  {
    QDomElement itemElem = doc.createElement( "item" );
    itemElem.appendChild( doc.createTextNode( layerId ) );
    customOrderElem.appendChild( itemElem );
  }
  elem.appendChild( customOrderElem );

  doc.documentElement().appendChild( elem );
}

void QgsLayerTreeMapCanvasBridge::setCanvasLayers( QgsLayerTreeNode *node, QList<QgsMapCanvasLayer> &layers )
{
  if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
    layers << QgsMapCanvasLayer( nodeLayer->layer(), nodeLayer->isVisible() == Qt::Checked, nodeLayer->customProperty( "overview", 0 ).toInt() );
  }

  foreach ( QgsLayerTreeNode* child, node->children() )
    setCanvasLayers( child, layers );
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
      foreach ( QgsLayerTreeLayer* nodeL, QgsLayerTree::toGroup( child )->findLayers() )
        layerIds << nodeL->layerId();
    }
  }

  foreach ( QString layerId, layerIds )
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

void QgsLayerTreeMapCanvasBridge::nodeCustomPropertyChanged( QgsLayerTreeNode* node, QString key )
{
  Q_UNUSED( node );
  if ( key == "overview" )
    deferredSetCanvasLayers();
}

