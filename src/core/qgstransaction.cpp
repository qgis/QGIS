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

#include "qgstransaction.h"
#include "qgsmaplayerregistry.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"
#include "qgis.h"
#include <QLibrary>

typedef bool beginTransaction_t( const QString& id, const QString& connString, QString& error );
typedef bool commitTransaction_t( const QString& id, QString& error );
typedef bool rollbackTransaction_t( const QString& id, QString& error );
typedef QgsVectorDataProvider* executeTransactionSql_t( const QString& id, const QString& sql, QString& error );

QgsTransaction::QgsTransaction( const QString& connString, const QString& providerKey ): mId( QUuid::createUuid() ), mConnString( connString ), mProviderKey( providerKey )
{
}

QgsTransaction::~QgsTransaction()
{

}

bool QgsTransaction::addLayer( const QString& layerId )
{
  if ( !QgsMapLayerRegistry::instance()->mapLayer( layerId ) )
  {
    return false;
  }

  mLayers.insert( layerId );
  return true;
}

bool QgsTransaction::begin( QString& errorMsg )
{
  //Set all layers to direct edit mode

  QLibrary* lib = QgsProviderRegistry::instance()->providerLibrary( mProviderKey );
  if ( !lib )
  {
    return false;
  }

  beginTransaction_t* pBeginTransaction = ( beginTransaction_t* ) cast_to_fptr( lib->resolve( "beginTransaction" ) );
  delete lib;

  if ( !pBeginTransaction )
  {
    return false;
  }

  if ( !pBeginTransaction( mId.toString(), mConnString, errorMsg ) )
  {
    return false;
  }

  setLayerTransactionIds( mId.toString() );
  return true;
}

bool QgsTransaction::commit( QString& errorMsg )
{
  QLibrary* lib = QgsProviderRegistry::instance()->providerLibrary( mProviderKey );
  if ( !lib )
  {
    return false;
  }

  commitTransaction_t* pCommitTransaction = ( commitTransaction_t* ) cast_to_fptr( lib->resolve( "commitTransaction" ) );
  delete lib;

  if ( !pCommitTransaction )
  {
    return false;
  }

  if ( !pCommitTransaction( mId.toString(), errorMsg ) )
  {
    return false;
  }

  setLayerTransactionIds( QString() );
  return true;
}

bool QgsTransaction::rollback( QString& errorMsg )
{
  QLibrary* lib = QgsProviderRegistry::instance()->providerLibrary( mProviderKey );
  if ( !lib )
  {
    return false;
  }

  rollbackTransaction_t* pRollbackTransaction = ( rollbackTransaction_t* ) cast_to_fptr( lib->resolve( "rollbackTransaction" ) );
  delete lib;

  if ( !pRollbackTransaction )
  {
    return false;
  }

  if ( !pRollbackTransaction( mId.toString(), errorMsg ) )
  {
    return false;
  }

  setLayerTransactionIds( QString() );
  return true;
}

QgsVectorDataProvider* QgsTransaction::executeSql( const QString& sql, QString& errorMsg )
{
  QLibrary* lib = QgsProviderRegistry::instance()->providerLibrary( mProviderKey );
  if ( !lib )
  {
    return 0;
  }

  executeTransactionSql_t* pExecuteTransactionSql = ( executeTransactionSql_t* ) cast_to_fptr( lib->resolve( "executeTransactionSql" ) );
  delete lib;

  if ( !pExecuteTransactionSql )
  {
    return 0;
  }

  return pExecuteTransactionSql( mId.toString(), sql, errorMsg );
}

void QgsTransaction::setLayerTransactionIds( const QString& id )
{
  QSet<QString>::iterator layerIt = mLayers.begin();
  for ( ; layerIt != mLayers.end(); ++layerIt )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( *layerIt ) );
    if ( vl )
    {
      vl->setTransactionId( id );
    }
  }
}
