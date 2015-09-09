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
  {
    return 0;
  }

  createTransaction_t* createTransaction = ( createTransaction_t* ) cast_to_fptr( lib->resolve( "createTransaction" ) );
  if ( !createTransaction )
  {
    return 0;
  }

  QgsTransaction* ts = createTransaction( connString );

  delete lib;

  return ts;
}

QgsTransaction* QgsTransaction::create( const QStringList& layerIds )
{
  if ( layerIds.isEmpty() )
  {
    return 0;
  }

  QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerIds.first() ) );
  if ( !layer )
  {
    return 0;
  }

  QString connStr = QgsDataSourceURI( layer->source() ).connectionInfo();
  QString providerKey = layer->dataProvider()->name();
  QgsTransaction* ts = QgsTransaction::create( connStr, providerKey );
  if ( !ts )
  {
    return 0;
  }

  Q_FOREACH ( const QString& layerId, layerIds )
  {
    if ( !ts->addLayer( layerId ) )
    {
      delete ts;
      return 0;
    }
  }
  return ts;
}


QgsTransaction::QgsTransaction( const QString& connString )
    : mConnString( connString ), mTransactionActive( false )
{
}

QgsTransaction::~QgsTransaction()
{
  setLayerTransactionIds( 0 );
}

bool QgsTransaction::addLayer( const QString& layerId )
{
  if ( mTransactionActive )
  {
    return false;
  }

  QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  if ( !layer )
  {
    return false;
  }

  if ( layer->isEditable() )
  {
    return false;
  }

  //test if provider supports transactions
  if ( !layer->dataProvider() || ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::TransactionSupport ) == 0 )
  {
    return false;
  }

  if ( layer->dataProvider()->transaction() != 0 )
  {
    return false;
  }

  //connection string not compatible
  if ( QgsDataSourceURI( layer->source() ).connectionInfo() != mConnString )
  {
    return false;
  }

  mLayers.insert( layerId );
  return true;
}

bool QgsTransaction::begin( QString& errorMsg, int statementTimeout )
{
  if ( mTransactionActive )
  {
    return false;
  }

  //Set all layers to direct edit mode
  if ( !beginTransaction( errorMsg, statementTimeout ) )
  {
    return false;
  }

  setLayerTransactionIds( this );
  mTransactionActive = true;
  return true;
}

bool QgsTransaction::commit( QString& errorMsg )
{
  if ( !mTransactionActive )
  {
    return false;
  }

  Q_FOREACH ( const QString& layerid, mLayers )
  {
    QgsMapLayer* l = QgsMapLayerRegistry::instance()->mapLayer( layerid );
    if ( !l || l->isEditable() )
    {
      return false;
    }
  }

  if ( !commitTransaction( errorMsg ) )
  {
    return false;
  }

  setLayerTransactionIds( 0 );
  mTransactionActive = false;
  return true;
}

bool QgsTransaction::rollback( QString& errorMsg )
{
  if ( !mTransactionActive )
  {
    return false;
  }

  Q_FOREACH ( const QString& layerid, mLayers )
  {
    QgsMapLayer* l = QgsMapLayerRegistry::instance()->mapLayer( layerid );
    if ( !l || l->isEditable() )
    {
      return false;
    }
  }

  if ( !rollbackTransaction( errorMsg ) )
  {
    return false;
  }

  setLayerTransactionIds( 0 );
  mTransactionActive = false;
  return true;
}

void QgsTransaction::setLayerTransactionIds( QgsTransaction* transaction )
{
  Q_FOREACH ( const QString& layerid, mLayers )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerid ) );
    if ( vl && vl->dataProvider() )
    {
      vl->dataProvider()->setTransaction( transaction );
    }
  }
}
