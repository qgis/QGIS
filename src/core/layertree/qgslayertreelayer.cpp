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


QgsLayerTreeLayer::QgsLayerTreeLayer( QgsMapLayer* layer )
  : QgsLayerTreeNode( NodeLayer )
  , mLayerId( layer->id() )
  , mLayer( nullptr )
  , mVisible( Qt::Checked )
{
  Q_ASSERT( QgsMapLayerRegistry::instance()->mapLayer( mLayerId ) == layer );
  attachToLayer();
}

QgsLayerTreeLayer::QgsLayerTreeLayer( const QString& layerId, const QString& name )
  : QgsLayerTreeNode( NodeLayer )
  , mLayerId( layerId )
  , mLayerName( name )
  , mLayer( nullptr )
  , mVisible( Qt::Checked )
{
  attachToLayer();
}

QgsLayerTreeLayer::QgsLayerTreeLayer( const QgsLayerTreeLayer& other )
  : QgsLayerTreeNode( other )
  , mLayerId( other.mLayerId )
  , mLayerName( other.mLayerName )
  , mLayer( nullptr )
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
    connect( l, SIGNAL( nameChanged() ), this, SLOT( layerNameChanged() ) );
    // make sure we are notified if the layer is removed
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( registryLayersWillBeRemoved( QStringList ) ) );
  }
  else
  {
    if ( mLayerName.isEmpty() )
      mLayerName = QStringLiteral( "(?)" );
    // wait for the layer to be eventually loaded
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( registryLayersAdded( QList<QgsMapLayer*> ) ) );
  }
}

QString QgsLayerTreeLayer::name() const
{
  return mLayer ? mLayer->name() : mLayerName;
}

void QgsLayerTreeLayer::setName( const QString& n )
{
  if ( mLayer )
  {
    if ( mLayer->name() == n )
      return;
    mLayer->setName( n );
    // no need to emit signal: we will be notified from layer's nameChanged() signal
  }
  else
  {
    if ( mLayerName == n )
      return;
    mLayerName = n;
    emit nameChanged( this, n );
  }
}

void QgsLayerTreeLayer::setVisible( Qt::CheckState state )
{
  if ( mVisible == state )
    return;

  mVisible = state;
  emit visibilityChanged( this, state );
}

QgsLayerTreeLayer* QgsLayerTreeLayer::readXml( QDomElement& element )
{
  if ( element.tagName() != QLatin1String( "layer-tree-layer" ) )
    return nullptr;

  QString layerID = element.attribute( QStringLiteral( "id" ) );
  QString layerName = element.attribute( QStringLiteral( "name" ) );
  Qt::CheckState checked = QgsLayerTreeUtils::checkStateFromXml( element.attribute( QStringLiteral( "checked" ) ) );
  bool isExpanded = ( element.attribute( QStringLiteral( "expanded" ), QStringLiteral( "1" ) ) == QLatin1String( "1" ) );

  QgsLayerTreeLayer* nodeLayer = nullptr;

  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );

  if ( layer )
    nodeLayer = new QgsLayerTreeLayer( layer );
  else
    nodeLayer = new QgsLayerTreeLayer( layerID, layerName );

  nodeLayer->readCommonXml( element );

  nodeLayer->setVisible( checked );
  nodeLayer->setExpanded( isExpanded );
  return nodeLayer;
}

void QgsLayerTreeLayer::writeXml( QDomElement& parentElement )
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement( QStringLiteral( "layer-tree-layer" ) );
  elem.setAttribute( QStringLiteral( "id" ), mLayerId );
  elem.setAttribute( QStringLiteral( "name" ), name() );
  elem.setAttribute( QStringLiteral( "checked" ), QgsLayerTreeUtils::checkStateToXml( mVisible ) );
  elem.setAttribute( QStringLiteral( "expanded" ), mExpanded ? "1" : "0" );

  writeCommonXml( elem );

  parentElement.appendChild( elem );
}

QString QgsLayerTreeLayer::dump() const
{
  return QStringLiteral( "LAYER: %1 visible=%2 expanded=%3 id=%4\n" ).arg( name() ).arg( mVisible ).arg( mExpanded ).arg( layerId() );
}

QgsLayerTreeLayer* QgsLayerTreeLayer::clone() const
{
  return new QgsLayerTreeLayer( *this );
}

void QgsLayerTreeLayer::registryLayersAdded( const QList<QgsMapLayer*>& layers )
{
  Q_FOREACH ( QgsMapLayer* l, layers )
  {
    if ( l->id() == mLayerId )
    {
      disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( registryLayersAdded( QList<QgsMapLayer*> ) ) );
      attachToLayer();
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

    mLayer = nullptr;
  }
}

void QgsLayerTreeLayer::layerNameChanged()
{
  Q_ASSERT( mLayer );
  emit nameChanged( this, mLayer->name() );
}
