/***************************************************************************
  qgslayertree
 ---------------------
 begin                : 22.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertree.h"
#include "qgsmaplayerlistutils.h"
#include "qgsvectorlayer.h"

QgsLayerTree::QgsLayerTree()
{
  connect( this, &QgsLayerTree::addedChildren, this, &QgsLayerTree::nodeAddedChildren );
  connect( this, &QgsLayerTree::removedChildren, this, &QgsLayerTree::nodeRemovedChildren );
}

QgsLayerTree::QgsLayerTree( const QgsLayerTree &other )
  : QgsLayerTreeGroup( other )
  , mCustomLayerOrder( other.mCustomLayerOrder )
  , mHasCustomLayerOrder( other.mHasCustomLayerOrder )
{
  connect( this, &QgsLayerTree::addedChildren, this, &QgsLayerTree::nodeAddedChildren );
  connect( this, &QgsLayerTree::removedChildren, this, &QgsLayerTree::nodeRemovedChildren );
}

QList<QgsMapLayer *> QgsLayerTree::customLayerOrder() const
{
  return _qgis_listQPointerToRaw( mCustomLayerOrder );
}

void QgsLayerTree::setCustomLayerOrder( const QList<QgsMapLayer *> &customLayerOrder )
{
  QgsWeakMapLayerPointerList newOrder = _qgis_listRawToQPointer( customLayerOrder );

  if ( newOrder == mCustomLayerOrder )
    return;

  mCustomLayerOrder = newOrder;
  emit customLayerOrderChanged();

  if ( mHasCustomLayerOrder )
    emit layerOrderChanged();
}

void QgsLayerTree::setCustomLayerOrder( const QStringList &customLayerOrder )
{
  QList<QgsMapLayer *> layers;

  for ( const auto &layerId : customLayerOrder )
  {
    QgsLayerTreeLayer *nodeLayer = findLayer( layerId );
    if ( nodeLayer )
    {
      // configuration from 2.x projects might have non spatial layers
      QgsMapLayer *layer = nodeLayer->layer();
      if ( !layer || !layer->isSpatial() )
      {
        continue;
      }
      layers.append( layer );
    }
  }
  setCustomLayerOrder( layers );
}

QList<QgsMapLayer *> QgsLayerTree::layerOrder() const
{
  if ( mHasCustomLayerOrder )
  {
    return customLayerOrder();
  }
  else
  {
    QList<QgsMapLayer *> layers;
    const QList< QgsLayerTreeLayer * > foundLayers = findLayers();
    for ( const auto &treeLayer : foundLayers )
    {
      QgsMapLayer *layer = treeLayer->layer();
      if ( !layer || !layer->isSpatial() )
      {
        continue;
      }
      layers.append( layer );
    }
    return layers;
  }
}

bool QgsLayerTree::hasCustomLayerOrder() const
{
  return mHasCustomLayerOrder;
}

void QgsLayerTree::setHasCustomLayerOrder( bool hasCustomLayerOrder )
{
  if ( hasCustomLayerOrder == mHasCustomLayerOrder )
    return;

  mHasCustomLayerOrder = hasCustomLayerOrder;

  emit hasCustomLayerOrderChanged( hasCustomLayerOrder );
  emit layerOrderChanged();
}

QgsLayerTree *QgsLayerTree::readXml( QDomElement &element, const QgsReadWriteContext &context )
{
  QgsLayerTree *tree = new QgsLayerTree();

  tree->readCommonXml( element );

  tree->readChildrenFromXml( element, context );

  return tree;
}

void QgsLayerTree::writeXml( QDomElement &parentElement, const QgsReadWriteContext &context )
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement( QStringLiteral( "layer-tree-group" ) );

  writeCommonXml( elem );

  for ( QgsLayerTreeNode *node : qgis::as_const( mChildren ) )
    node->writeXml( elem, context );

  QDomElement customOrderElem = doc.createElement( QStringLiteral( "custom-order" ) );
  customOrderElem.setAttribute( QStringLiteral( "enabled" ), mHasCustomLayerOrder ? 1 : 0 );
  elem.appendChild( customOrderElem );

  for ( QgsMapLayer *layer : qgis::as_const( mCustomLayerOrder ) )
  {
    // Safety belt, see https://github.com/qgis/QGIS/issues/26975
    // Crash when deleting an item from the layout legend
    if ( ! layer )
      continue;
    QDomElement layerElem = doc.createElement( QStringLiteral( "item" ) );
    layerElem.appendChild( doc.createTextNode( layer->id() ) );
    customOrderElem.appendChild( layerElem );
  }

  elem.appendChild( customOrderElem );

  parentElement.appendChild( elem );
}

QgsLayerTree *QgsLayerTree::clone() const
{
  return new QgsLayerTree( *this );
}

void QgsLayerTree::clear()
{
  removeAllChildren();
  setHasCustomLayerOrder( false );
  setCustomLayerOrder( QStringList() );
}

void QgsLayerTree::nodeAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );

  // collect layer IDs that have been added in order to put them into custom layer order
  QList<QgsMapLayer *> layers;

  QList<QgsLayerTreeNode *> children = node->children();
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode *child = children.at( i );
    if ( QgsLayerTree::isLayer( child ) )
    {
      layers << QgsLayerTree::toLayer( child )->layer();
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      const auto nodeLayers = QgsLayerTree::toGroup( child )->findLayers();
      for ( QgsLayerTreeLayer *nodeL : nodeLayers )
        layers << nodeL->layer();
    }
  }

  for ( QgsMapLayer *layer : qgis::as_const( layers ) )
  {
    if ( !mCustomLayerOrder.contains( layer ) && layer )
      mCustomLayerOrder.append( layer );
  }

  emit customLayerOrderChanged();
  emit layerOrderChanged();
}

void QgsLayerTree::nodeRemovedChildren()
{
  QList<QgsMapLayer *> layers = customLayerOrder();
  auto layer = layers.begin();

  while ( layer != layers.end() )
  {
    if ( !findLayer( *layer ) )
      layer = layers.erase( layer );
    else
      ++layer;
  }

  // we need to ensure that the customLayerOrderChanged signal is ALWAYS raised
  // here, since that order HAS changed due to removal of the child!
  // setCustomLayerOrder will only emit this signal when the layers list
  // at this stage is different to the stored customer layer order. If this
  // isn't the case (i.e. the lists ARE the same) then manually emit the
  // signal
  const bool emitSignal = _qgis_listRawToQPointer( layers ) == mCustomLayerOrder;

  setCustomLayerOrder( layers );
  if ( emitSignal )
    emit customLayerOrderChanged();

  emit layerOrderChanged();
}

void QgsLayerTree::addMissingLayers()
{
  bool changed = false;

  const QList< QgsLayerTreeLayer * > layers = findLayers();
  for ( const auto layer : layers )
  {
    if ( !mCustomLayerOrder.contains( layer->layer() ) &&
         layer->layer() && layer->layer()->isSpatial() )
    {
      mCustomLayerOrder.append( layer->layer() );
      changed = true;
    }
  }

  if ( changed )
  {
    emit customLayerOrderChanged();
    if ( mHasCustomLayerOrder )
      emit layerOrderChanged();
  }
}

void QgsLayerTree::readLayerOrderFromXml( const QDomElement &elem )
{
  QStringList order;

  QDomElement customOrderElem = elem.firstChildElement( QStringLiteral( "custom-order" ) );
  if ( !customOrderElem.isNull() )
  {
    setHasCustomLayerOrder( customOrderElem.attribute( QStringLiteral( "enabled" ) ).toInt() );

    QDomElement itemElem = customOrderElem.firstChildElement( QStringLiteral( "item" ) );
    while ( !itemElem.isNull() )
    {
      order.append( itemElem.text() );
      itemElem = itemElem.nextSiblingElement( QStringLiteral( "item" ) );
    }
  }

  setCustomLayerOrder( order );
  addMissingLayers();
}
