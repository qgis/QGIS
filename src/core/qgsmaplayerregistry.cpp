/***************************************************************************
 *  QgsMapLayerRegistry.cpp  -  Singleton class for tracking mMapLayers.
 *                         -------------------
 * begin                : Sun June 02 2004
 * copyright            : (C) 2004 by Tim Sutton
 * email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerregistry.h"
#include "qgsmaplayer.h"
#include "qgslogger.h"

//
// Static calls to enforce singleton behaviour
//
QgsMapLayerRegistry *QgsMapLayerRegistry::instance()
{
  static QgsMapLayerRegistry sInstance;
  return &sInstance;
}

QgsMapLayerRegistry::QgsMapLayerRegistry( QObject *parent )
    : QObject( parent )
{}

QgsMapLayerRegistry::~QgsMapLayerRegistry()
{
  removeAllMapLayers();
}

int QgsMapLayerRegistry::count() const
{
  return mMapLayers.size();
}

QgsMapLayer * QgsMapLayerRegistry::mapLayer( const QString& theLayerId ) const
{
  return mMapLayers.value( theLayerId );
}

QList<QgsMapLayer *> QgsMapLayerRegistry::mapLayersByName( const QString& layerName ) const
{
  QList<QgsMapLayer *> myResultList;
  Q_FOREACH ( QgsMapLayer* layer, mMapLayers )
  {
    if ( layer->name() == layerName )
    {
      myResultList << layer;
    }
  }
  return myResultList;
}

QList<QgsMapLayer *> QgsMapLayerRegistry::addMapLayers(
  const QList<QgsMapLayer *>& theMapLayers,
  bool addToLegend,
  bool takeOwnership )
{
  QList<QgsMapLayer *> myResultList;
  Q_FOREACH ( QgsMapLayer* myLayer, theMapLayers )
  {
    if ( !myLayer || !myLayer->isValid() )
    {
      QgsDebugMsg( "Cannot add invalid layers" );
      continue;
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
      connect( myLayer, SIGNAL( destroyed( QObject* ) ), this, SLOT( onMapLayerDeleted( QObject* ) ) );
      emit layerWasAdded( myLayer );
    }
  }
  if ( !myResultList.isEmpty() )
  {
    emit layersAdded( myResultList );

    if ( addToLegend )
      emit legendLayersAdded( myResultList );
  }
  return myResultList;
}

QgsMapLayer *
QgsMapLayerRegistry::addMapLayer( QgsMapLayer* theMapLayer,
                                  bool addToLegend,
                                  bool takeOwnership )
{
  QList<QgsMapLayer *> addedLayers;
  addedLayers = addMapLayers( QList<QgsMapLayer*>() << theMapLayer, addToLegend, takeOwnership );
  return addedLayers.isEmpty() ? nullptr : addedLayers[0];
}

void QgsMapLayerRegistry::removeMapLayers( const QStringList& theLayerIds )
{
  QList<QgsMapLayer*> layers;
  Q_FOREACH ( const QString &myId, theLayerIds )
  {
    layers << mMapLayers.value( myId );
  }

  removeMapLayers( layers );
}

void QgsMapLayerRegistry::removeMapLayers( const QList<QgsMapLayer*>& layers )
{
  if ( layers.isEmpty() )
    return;

  QStringList layerIds;
  QList<QgsMapLayer*> layerList;

  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    // check layer and the registry contains it
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

  Q_FOREACH ( QgsMapLayer* lyr, layerList )
  {
    QString myId( lyr->id() );
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

void QgsMapLayerRegistry::removeMapLayer( const QString& theLayerId )
{
  removeMapLayers( QList<QgsMapLayer*>() << mMapLayers.value( theLayerId ) );
}

void QgsMapLayerRegistry::removeMapLayer( QgsMapLayer* layer )
{
  if ( layer )
    removeMapLayers( QList<QgsMapLayer*>() << layer );
}

void QgsMapLayerRegistry::removeAllMapLayers()
{
  emit removeAll();
  // now let all observers know to clear themselves,
  // and then consequently any of their map legends
  removeMapLayers( mMapLayers.keys() );
  mMapLayers.clear();
}

void QgsMapLayerRegistry::reloadAllLayers()
{
  Q_FOREACH ( QgsMapLayer* layer, mMapLayers )
  {
    layer->reload();
  }
}

void QgsMapLayerRegistry::onMapLayerDeleted( QObject* obj )
{
  QString id = mMapLayers.key( static_cast<QgsMapLayer*>( obj ) );

  if ( !id.isNull() )
  {
    QgsDebugMsg( QString( "Map layer deleted without unregistering! %1" ).arg( id ) );
    mMapLayers.remove( id );
  }
}

QMap<QString, QgsMapLayer*> QgsMapLayerRegistry::mapLayers() const
{
  return mMapLayers;
}


#if 0
void QgsMapLayerRegistry::connectNotify( const char * signal )
{
  Q_UNUSED( signal );
  //QgsDebugMsg("QgsMapLayerRegistry connected to " + QString(signal));
} //  QgsMapLayerRegistry::connectNotify
#endif
