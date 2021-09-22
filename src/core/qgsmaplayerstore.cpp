/***************************************************************************
                         qgsmaplayerstore.cpp
                         --------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerstore.h"
#include "qgsmaplayer.h"
#include "qgslogger.h"
#include <QList>

QgsMapLayerStore::QgsMapLayerStore( QObject *parent )
  : QObject( parent )
{}

QgsMapLayerStore::~QgsMapLayerStore()
{
  removeAllMapLayers();
}

int QgsMapLayerStore::count() const
{
  return mMapLayers.size();
}

int QgsMapLayerStore::validCount() const
{
  int i = 0;
  const QList<QgsMapLayer *> cLayers = mMapLayers.values();
  for ( const auto l : cLayers )
  {
    if ( l->isValid() )
      i++;
  }
  return i;
}

QgsMapLayer *QgsMapLayerStore::mapLayer( const QString &layerId ) const
{
  return mMapLayers.value( layerId );
}

QList<QgsMapLayer *> QgsMapLayerStore::mapLayersByName( const QString &layerName ) const
{
  QList<QgsMapLayer *> myResultList;
  const auto constMMapLayers = mMapLayers;
  for ( QgsMapLayer *layer : constMMapLayers )
  {
    if ( layer->name() == layerName )
    {
      myResultList << layer;
    }
  }
  return myResultList;
}

QList<QgsMapLayer *> QgsMapLayerStore::addMapLayers( const QList<QgsMapLayer *> &layers, bool takeOwnership )
{
  QList<QgsMapLayer *> myResultList;
  const auto constLayers = layers;
  for ( QgsMapLayer *myLayer : constLayers )
  {
    if ( !myLayer )
    {
      QgsDebugMsg( QStringLiteral( "Cannot add null layers" ) );
      continue;
    }
    // If the layer is already in the store but its validity has flipped to TRUE reset data source
    if ( mMapLayers.contains( myLayer->id() ) && ! mMapLayers[myLayer->id()]->isValid() && myLayer->isValid() && myLayer->dataProvider() )
    {
      mMapLayers[myLayer->id()]->setDataSource( myLayer->dataProvider()->dataSourceUri(), myLayer->name(), myLayer->providerType(), QgsDataProvider::ProviderOptions() );
    }
    //check the layer is not already registered!
    if ( !mMapLayers.contains( myLayer->id() ) )
    {
      mMapLayers[myLayer->id()] = myLayer;
      myResultList << mMapLayers[myLayer->id()];
      if ( takeOwnership )
      {
        myLayer->setParent( this );
      }
      connect( myLayer, &QObject::destroyed, this, &QgsMapLayerStore::onMapLayerDeleted );
      emit layerWasAdded( myLayer );
    }
  }
  if ( !myResultList.isEmpty() )
  {
    emit layersAdded( myResultList );
  }
  return myResultList;
}

QgsMapLayer *
QgsMapLayerStore::addMapLayer( QgsMapLayer *layer, bool takeOwnership )
{
  QList<QgsMapLayer *> addedLayers;
  addedLayers = addMapLayers( QList<QgsMapLayer *>() << layer, takeOwnership );
  return addedLayers.isEmpty() ? nullptr : addedLayers[0];
}

void QgsMapLayerStore::removeMapLayers( const QStringList &layerIds )
{
  QList<QgsMapLayer *> layers;
  const auto constLayerIds = layerIds;
  for ( const QString &myId : constLayerIds )
  {
    layers << mMapLayers.value( myId );
  }

  removeMapLayers( layers );
}

void QgsMapLayerStore::removeMapLayers( const QList<QgsMapLayer *> &layers )
{
  if ( layers.isEmpty() )
    return;

  QStringList layerIds;
  QList<QgsMapLayer *> layerList;

  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    // check layer and the store contains it
    if ( layer && mMapLayers.contains( layer->id() ) )
    {
      layerIds << layer->id();
      layerList << layer;
    }
  }

  if ( layerIds.isEmpty() )
    return;

  emit layersWillBeRemoved( layerIds );
  emit layersWillBeRemoved( layerList );

  const auto constLayerList = layerList;
  for ( QgsMapLayer *lyr : constLayerList )
  {
    const QString myId( lyr->id() );
    emit layerWillBeRemoved( myId );
    emit layerWillBeRemoved( lyr );
    mMapLayers.remove( myId );
    if ( lyr->parent() == this )
    {
      delete lyr;
    }
    emit layerRemoved( myId );
  }

  emit layersRemoved( layerIds );
}

void QgsMapLayerStore::removeMapLayer( const QString &layerId )
{
  removeMapLayers( QList<QgsMapLayer *>() << mMapLayers.value( layerId ) );
}

void QgsMapLayerStore::removeMapLayer( QgsMapLayer *layer )
{
  if ( layer )
    removeMapLayers( QList<QgsMapLayer *>() << layer );
}

QgsMapLayer *QgsMapLayerStore::takeMapLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return nullptr;

  if ( mMapLayers.contains( layer->id() ) )
  {
    emit layersWillBeRemoved( QStringList() << layer->id() );
    emit layersWillBeRemoved( QList<QgsMapLayer *>() << layer );
    emit layerWillBeRemoved( layer->id() );
    emit layerWillBeRemoved( layer );

    mMapLayers.remove( layer->id() );
    layer->setParent( nullptr );
    emit layerRemoved( layer->id() );
    emit layersRemoved( QStringList() << layer->id() );
    return layer;
  }
  return nullptr; //don't return layer - it wasn't owned and accordingly we aren't transferring ownership
}

void QgsMapLayerStore::removeAllMapLayers()
{
  emit allLayersRemoved();
  // now let all observers know to clear themselves,
  // and then consequently any of their map legends
  removeMapLayers( mMapLayers.keys() );
  mMapLayers.clear();
}

void QgsMapLayerStore::transferLayersFromStore( QgsMapLayerStore *other )
{
  if ( !other || other == this )
    return;

  Q_ASSERT_X( other->thread() == thread(), "QgsMapLayerStore::transferLayersFromStore", "Cannot transfer layers from store with different thread affinity" );

  const QMap<QString, QgsMapLayer *> otherLayers = other->mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator it = otherLayers.constBegin();
  for ( ; it != otherLayers.constEnd(); ++it )
  {
    QgsMapLayer *layer = other->takeMapLayer( it.value() );
    if ( layer )
      addMapLayer( layer );
  }
}

void QgsMapLayerStore::onMapLayerDeleted( QObject *obj )
{
  const QString id = mMapLayers.key( static_cast<QgsMapLayer *>( obj ) );

  if ( !id.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Map layer deleted without unregistering! %1" ).arg( id ) );
    mMapLayers.remove( id );
  }
}

QMap<QString, QgsMapLayer *> QgsMapLayerStore::mapLayers() const
{
  return mMapLayers;
}

QMap<QString, QgsMapLayer *> QgsMapLayerStore::validMapLayers() const
{
  QMap<QString, QgsMapLayer *> validLayers;
  for ( const auto &id : mMapLayers.keys() )
  {
    if ( mMapLayers[id]->isValid() )
      validLayers[id] = mMapLayers[id];
  }
  return validLayers;
}
