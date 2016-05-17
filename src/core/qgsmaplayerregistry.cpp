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

//
// Main class begins now...
//

QgsMapLayerRegistry::QgsMapLayerRegistry( QObject *parent ) : QObject( parent )
{
  // constructor does nothing
}

QgsMapLayerRegistry::~QgsMapLayerRegistry()
{
  removeAllMapLayers();
}

// get the layer count (number of registered layers)
int QgsMapLayerRegistry::count()
{
  return mMapLayers.size();
}

QgsMapLayer * QgsMapLayerRegistry::mapLayer( const QString& theLayerId )
{
  return mMapLayers.value( theLayerId );
}

QList<QgsMapLayer *> QgsMapLayerRegistry::mapLayersByName( const QString& layerName )
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

//introduced in 1.8
QList<QgsMapLayer *> QgsMapLayerRegistry::addMapLayers(
  const QList<QgsMapLayer *>& theMapLayers,
  bool addToLegend,
  bool takeOwnership )
{
  QList<QgsMapLayer *> myResultList;
  for ( int i = 0; i < theMapLayers.size(); ++i )
  {
    QgsMapLayer * myLayer = theMapLayers.at( i );
    if ( !myLayer || !myLayer->isValid() )
    {
      QgsDebugMsg( "cannot add invalid layers" );
      continue;
    }
    //check the layer is not already registered!
    if ( !mMapLayers.contains( myLayer->id() ) )
    {
      mMapLayers[myLayer->id()] = myLayer;
      myResultList << mMapLayers[myLayer->id()];
      if ( takeOwnership )
        mOwnedLayers << myLayer;
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
} // QgsMapLayerRegistry::addMapLayers

//this is just a thin wrapper for addMapLayers
QgsMapLayer *
QgsMapLayerRegistry::addMapLayer( QgsMapLayer* theMapLayer,
                                  bool addToLegend,
                                  bool takeOwnership )
{
  QList<QgsMapLayer *> addedLayers;
  addedLayers = addMapLayers( QList<QgsMapLayer*>() << theMapLayer, addToLegend, takeOwnership );
  return addedLayers.isEmpty() ? nullptr : addedLayers[0];
}


//introduced in 1.8
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
  QStringList layerIds;

  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    if ( layer )
      layerIds << layer->id();
  }

  emit layersWillBeRemoved( layerIds );
  emit layersWillBeRemoved( layers );

  Q_FOREACH ( QgsMapLayer* lyr, layers )
  {
    if ( !lyr )
      continue;

    QString myId( lyr->id() );
    if ( mOwnedLayers.contains( lyr ) )
    {
      emit layerWillBeRemoved( myId );
      emit layerWillBeRemoved( lyr );
      delete lyr;
      mOwnedLayers.remove( lyr );
    }
    mMapLayers.remove( myId );
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
  // now let all canvas observers know to clear themselves,
  // and then consequently any of their map legends
  removeMapLayers( mMapLayers.keys() );
  mMapLayers.clear();
} // QgsMapLayerRegistry::removeAllMapLayers()

void QgsMapLayerRegistry::reloadAllLayers()
{
  QMap<QString, QgsMapLayer *>::iterator it;
  for ( it = mMapLayers.begin(); it != mMapLayers.end() ; ++it )
  {
    QgsMapLayer* layer = it.value();
    if ( layer )
    {
      layer->reload();
    }
  }
}

QMap<QString, QgsMapLayer*> QgsMapLayerRegistry::mapLayers()
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
