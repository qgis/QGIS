/***************************************************************************
                              qgstransaction.cpp
                              ------------------
  begin                : May 5, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLibrary>

#include "qgstransaction.h"
#include "qgsdatasourceuri.h"
#include "qgsmaplayerregistry.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

typedef QgsTransaction* createTransaction_t( const QString& connString );

QgsTransaction* QgsTransaction::create( const QString& connString, const QString& providerKey )
{

  QLibrary* lib = QgsProviderRegistry::instance()->providerLibrary( providerKey );
  if ( !lib )
    return nullptr;

  createTransaction_t* createTransaction = reinterpret_cast< createTransaction_t* >( cast_to_fptr( lib->resolve( "createTransaction" ) ) );
  if ( !createTransaction )
    return nullptr;

  QgsTransaction* ts = createTransaction( connString );

  delete lib;

  return ts;
}

QgsTransaction* QgsTransaction::create( const QStringList& layerIds )
{
  if ( layerIds.isEmpty() )
    return nullptr;

  QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerIds.first() ) );
  if ( !layer )
    return nullptr;

  QString connStr = QgsDataSourceURI( layer->source() ).connectionInfo( false );
  QString providerKey = layer->dataProvider()->name();
  QgsTransaction* ts = QgsTransaction::create( connStr, providerKey );
  if ( !ts )
    return nullptr;

  Q_FOREACH ( const QString& layerId, layerIds )
  {
    if ( !ts->addLayer( layerId ) )
    {
      delete ts;
      return nullptr;
    }
  }
  return ts;
}


QgsTransaction::QgsTransaction( const QString& connString )
    : mConnString( connString )
    , mTransactionActive( false )
{
}

QgsTransaction::~QgsTransaction()
{
  setLayerTransactionIds( nullptr );
}

bool QgsTransaction::addLayer( const QString& layerId )
{
  QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  return addLayer( layer );
}

bool QgsTransaction::addLayer( QgsVectorLayer* layer )
{
  if ( !layer )
    return false;

  if ( layer->isEditable() )
    return false;

  //test if provider supports transactions
  if ( !layer->dataProvider() || ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::TransactionSupport ) == 0 )
    return false;

  if ( layer->dataProvider()->transaction() )
    return false;

  //connection string not compatible
  if ( QgsDataSourceURI( layer->source() ).connectionInfo( false ) != mConnString )
  {
    QgsDebugMsg( QString( "Couldn't start transaction because connection string for layer %1 : '%2' does not match '%3'" ).arg(
                   layer->id(), QgsDataSourceURI( layer->source() ).connectionInfo( false ), mConnString ) );
    return false;
  }

  connect( this, SIGNAL( afterRollback() ), layer->dataProvider(), SIGNAL( dataChanged() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( onLayersDeleted( QStringList ) ) );
  mLayers.insert( layer );

  if ( mTransactionActive )
    layer->dataProvider()->setTransaction( this );

  return true;
}

bool QgsTransaction::begin( QString& errorMsg, int statementTimeout )
{
  if ( mTransactionActive )
    return false;

  //Set all layers to direct edit mode
  if ( !beginTransaction( errorMsg, statementTimeout ) )
    return false;

  setLayerTransactionIds( this );
  mTransactionActive = true;
  return true;
}

bool QgsTransaction::commit( QString& errorMsg )
{
  if ( !mTransactionActive )
    return false;

  if ( !commitTransaction( errorMsg ) )
    return false;

  setLayerTransactionIds( nullptr );
  mTransactionActive = false;
  return true;
}

bool QgsTransaction::rollback( QString& errorMsg )
{
  if ( !mTransactionActive )
    return false;

  if ( !rollbackTransaction( errorMsg ) )
    return false;

  setLayerTransactionIds( nullptr );
  mTransactionActive = false;

  emit afterRollback();

  return true;
}

bool QgsTransaction::supportsTransaction( const QgsVectorLayer* layer )
{
  QLibrary* lib = QgsProviderRegistry::instance()->providerLibrary( layer->providerType() );
  if ( !lib )
    return false;

  return lib->resolve( "createTransaction" );
}

void QgsTransaction::onLayersDeleted( const QStringList& layerids )
{
  Q_FOREACH ( const QString& layerid, layerids )
    Q_FOREACH ( QgsVectorLayer* l, mLayers )
      if ( l->id() == layerid )
        mLayers.remove( l );
}

void QgsTransaction::setLayerTransactionIds( QgsTransaction* transaction )
{
  Q_FOREACH ( QgsVectorLayer* vl, mLayers )
  {
    if ( vl->dataProvider() )
    {
      vl->dataProvider()->setTransaction( transaction );
    }
  }
}
