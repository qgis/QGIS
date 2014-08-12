/***************************************************************************
  qgslayertreelayer.cpp
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

#include "qgslayertreelayer.h"

#include "qgslayertreeutils.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"


QgsLayerTreeLayer::QgsLayerTreeLayer( QgsMapLayer *layer )
    : QgsLayerTreeNode( NodeLayer )
    , mLayerId( layer->id() )
    , mLayer( layer )
    , mVisible( Qt::Checked )
{
  Q_ASSERT( QgsMapLayerRegistry::instance()->mapLayer( mLayerId ) == layer );
}

QgsLayerTreeLayer::QgsLayerTreeLayer( QString layerId, QString name )
    : QgsLayerTreeNode( NodeLayer )
    , mLayerId( layerId )
    , mLayerName( name )
    , mLayer( 0 )
    , mVisible( Qt::Checked )
{
  attachToLayer();
}

QgsLayerTreeLayer::QgsLayerTreeLayer( const QgsLayerTreeLayer& other )
    : QgsLayerTreeNode( other )
    , mLayerId( other.mLayerId )
    , mLayerName( other.mLayerName )
    , mLayer( 0 )
    , mVisible( other.mVisible )
{
  attachToLayer();
}

void QgsLayerTreeLayer::attachToLayer()
{
  // layer is not necessarily already loaded
  QgsMapLayer* l = QgsMapLayerRegistry::instance()->mapLayer( mLayerId );
  if ( l )
  {
    mLayer = l;
    mLayerName = l->name();
    // make sure we are notified if the layer is removed
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( registryLayersWillBeRemoved( QStringList ) ) );
  }
  else
  {
    if ( mLayerName.isEmpty() )
      mLayerName = "(?)";
    // wait for the layer to be eventually loaded
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( registryLayersAdded( QList<QgsMapLayer*> ) ) );
  }
}


QString QgsLayerTreeLayer::layerName() const
{
  return mLayer ? mLayer->name() : mLayerName;
}

void QgsLayerTreeLayer::setLayerName( const QString& n )
{
  if ( mLayer )
    mLayer->setLayerName( n );
  else
    mLayerName = n;
}

void QgsLayerTreeLayer::setVisible( Qt::CheckState state )
{
  if ( mVisible == state )
    return;

  mVisible = state;
  emit visibilityChanged( this, state );
}

QgsLayerTreeLayer* QgsLayerTreeLayer::readXML( QDomElement& element )
{
  if ( element.tagName() != "layer-tree-layer" )
    return 0;

  QString layerID = element.attribute( "id" );
  QString layerName = element.attribute( "name" );
  Qt::CheckState checked = QgsLayerTreeUtils::checkStateFromXml( element.attribute( "checked" ) );
  bool isExpanded = ( element.attribute( "expanded", "1" ) == "1" );

  QgsLayerTreeLayer* nodeLayer = 0;

  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );

  if ( layer )
    nodeLayer = new QgsLayerTreeLayer( layer );
  else
    nodeLayer = new QgsLayerTreeLayer( layerID, layerName );

  nodeLayer->readCommonXML( element );

  nodeLayer->setVisible( checked );
  nodeLayer->setExpanded( isExpanded );
  return nodeLayer;
}

void QgsLayerTreeLayer::writeXML( QDomElement& parentElement )
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement( "layer-tree-layer" );
  elem.setAttribute( "id", mLayerId );
  elem.setAttribute( "name", layerName() );
  elem.setAttribute( "checked", QgsLayerTreeUtils::checkStateToXml( mVisible ) );
  elem.setAttribute( "expanded", mExpanded ? "1" : "0" );

  writeCommonXML( elem );

  parentElement.appendChild( elem );
}

QString QgsLayerTreeLayer::dump() const
{
  return QString( "LAYER: %1 visible=%2 expanded=%3 id=%4\n" ).arg( layerName() ).arg( mVisible ).arg( mExpanded ).arg( layerId() );
}

QgsLayerTreeNode* QgsLayerTreeLayer::clone() const
{
  return new QgsLayerTreeLayer( *this );
}

void QgsLayerTreeLayer::registryLayersAdded( QList<QgsMapLayer*> layers )
{
  foreach ( QgsMapLayer* l, layers )
  {
    if ( l->id() == mLayerId )
    {
      mLayer = l;
      disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( registryLayersAdded( QList<QgsMapLayer*> ) ) );
      emit layerLoaded();
      break;
    }
  }
}

void QgsLayerTreeLayer::registryLayersWillBeRemoved( const QStringList& layerIds )
{
  if ( layerIds.contains( mLayerId ) )
  {
    emit layerWillBeUnloaded();

    // stop listening to removal signals and start hoping that the layer may be added again
    disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( registryLayersWillBeRemoved( QStringList ) ) );
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( registryLayersAdded( QList<QgsMapLayer*> ) ) );

    mLayer = 0;
  }
}
