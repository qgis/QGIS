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
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsrasterdataprovider.h"

QgsLayerTreeLayer::QgsLayerTreeLayer( QgsMapLayer *layer )
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

QgsLayerTreeLayer *QgsLayerTreeLayer::createLayerFromParams( const LayerMatchParams &source )
{
  QgsLayerTreeLayer* l = new QgsLayerTreeLayer( QString() );
  l->attachToSource( source );
  return l;
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
      mLayerName = "(?)";
    // wait for the layer to be eventually loaded
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( registryLayersAdded( QList<QgsMapLayer*> ) ) );
  }
}

bool QgsLayerTreeLayer::layerMatchesSource( QgsMapLayer* layer, const QgsLayerTreeLayer::LayerMatchParams &params ) const
{
  if ( layer->publicSource() != params.source ||
       layer->name() != params.name )
    return false;

  switch ( layer->type() )
  {
    case QgsMapLayer::VectorLayer:
    {
      QgsVectorLayer* vl = qobject_cast< QgsVectorLayer* >( layer );
      if ( vl->dataProvider()->name() != params.providerKey )
        return false;
      break;
    }
    case QgsMapLayer::RasterLayer:
    {
      QgsRasterLayer* rl = qobject_cast< QgsRasterLayer* >( layer );
      if ( rl->dataProvider()->name() != params.providerKey )
        return false;
      break;
    }
    case QgsMapLayer::PluginLayer:
      break;

  }
  return true;
}

QString QgsLayerTreeLayer::name() const
{
  return layerName();
}

void QgsLayerTreeLayer::setName( const QString& n )
{
  setLayerName( n );
}

void QgsLayerTreeLayer::attachToSource( const LayerMatchParams &source )
{
  // check if matching source already open
  bool foundMatch = false;
  Q_FOREACH ( QgsMapLayer* layer, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    if ( layerMatchesSource( layer, source ) )
    {
      // found a source! need to disconnect from layersAdded signal as original attachToLayer call
      // will have set this up
      disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( registryLayersAdded( QList<QgsMapLayer*> ) ) );
      mLayerId = layer->id();
      attachToLayer();
      emit layerLoaded();
      foundMatch = true;
      break;
    }
  }

  if ( !foundMatch )
    mLooseMatchParams = source; // no need to store source if match already made
}

QString QgsLayerTreeLayer::layerName() const
{
  return mLayer ? mLayer->name() : mLayerName;
}

void QgsLayerTreeLayer::setLayerName( const QString& n )
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

QgsLayerTreeLayer* QgsLayerTreeLayer::readXML( QDomElement& element , bool looseMatch )
{
  if ( element.tagName() != "layer-tree-layer" )
    return nullptr;

  QString layerID = element.attribute( "id" );
  QString layerName = element.attribute( "name" );

  QString source = element.attribute( "source" );
  QString providerKey = element.attribute( "providerKey" );

  Qt::CheckState checked = QgsLayerTreeUtils::checkStateFromXml( element.attribute( "checked" ) );
  bool isExpanded = ( element.attribute( "expanded", "1" ) == "1" );

  QgsLayerTreeLayer* nodeLayer = nullptr;

  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );

  if ( layer )
    nodeLayer = new QgsLayerTreeLayer( layer );
  else if ( looseMatch && !source.isEmpty() )
  {
    LayerMatchParams params;
    params.name = layerName;
    params.source = source;
    params.providerKey = providerKey;
    nodeLayer = QgsLayerTreeLayer::createLayerFromParams( params );
  }
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
  if ( mLayer )
  {
    elem.setAttribute( "source", mLayer->publicSource() );

    QString providerKey;
    switch ( mLayer->type() )
    {
      case QgsMapLayer::VectorLayer:
      {
        QgsVectorLayer* vl = qobject_cast< QgsVectorLayer* >( mLayer );
        providerKey = vl->dataProvider()->name();
        break;
      }
      case QgsMapLayer::RasterLayer:
      {
        QgsRasterLayer* rl = qobject_cast< QgsRasterLayer* >( mLayer );
        providerKey = rl->dataProvider()->name();
        break;
      }
      case QgsMapLayer::PluginLayer:
        break;
    }
    elem.setAttribute( "providerKey", providerKey );
  }

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

QgsLayerTreeLayer* QgsLayerTreeLayer::clone() const
{
  return new QgsLayerTreeLayer( *this );
}

void QgsLayerTreeLayer::registryLayersAdded( const QList<QgsMapLayer*>& layers )
{
  Q_FOREACH ( QgsMapLayer* l, layers )
  {
    if ( !mLooseMatchParams.source.isEmpty() && layerMatchesSource( l, mLooseMatchParams ) )
    {
      // we are loosely matching, and found a layer with a matching source.
      // Attach to this!
      mLayerId = l->id();
    }
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
