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
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"
#include "qgsmessagelog.h"
#include <QUuid>

typedef QgsTransaction *createTransaction_t( const QString &connString );

QgsTransaction *QgsTransaction::create( const QString &connString, const QString &providerKey )
{
  std::unique_ptr< QLibrary > lib( QgsProviderRegistry::instance()->createProviderLibrary( providerKey ) );
  if ( !lib )
    return nullptr;

  createTransaction_t *createTransaction = reinterpret_cast< createTransaction_t * >( cast_to_fptr( lib->resolve( "createTransaction" ) ) );
  if ( !createTransaction )
    return nullptr;

  QgsTransaction *ts = createTransaction( connString );

  return ts;
}

QgsTransaction *QgsTransaction::create( const QSet<QgsVectorLayer *> &layers )
{
  if ( layers.isEmpty() )
    return nullptr;

  QgsVectorLayer *firstLayer = *layers.constBegin();

  QString connStr = QgsDataSourceUri( firstLayer->source() ).connectionInfo( false );
  QString providerKey = firstLayer->dataProvider()->name();
  std::unique_ptr<QgsTransaction> transaction( QgsTransaction::create( connStr, providerKey ) );
  if ( transaction )
  {
    for ( QgsVectorLayer *layer : layers )
    {
      if ( !transaction->addLayer( layer ) )
      {
        transaction.reset();
        break;
      }
    }
  }
  return transaction.release();
}


QgsTransaction::QgsTransaction( const QString &connString )
  : mConnString( connString )
  , mTransactionActive( false )
  , mLastSavePointIsDirty( true )
{
}

QgsTransaction::~QgsTransaction()
{
  setLayerTransactionIds( nullptr );
}

bool QgsTransaction::addLayer( QgsVectorLayer *layer )
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
  if ( QgsDataSourceUri( layer->source() ).connectionInfo( false ) != mConnString )
  {
    QgsDebugMsg( QString( "Couldn't start transaction because connection string for layer %1 : '%2' does not match '%3'" ).arg(
                   layer->id(), QgsDataSourceUri( layer->source() ).connectionInfo( false ), mConnString ) );
    return false;
  }

  connect( this, &QgsTransaction::afterRollback, layer->dataProvider(), &QgsVectorDataProvider::dataChanged );
  connect( layer, &QgsVectorLayer::destroyed, this, &QgsTransaction::onLayerDeleted );
  mLayers.insert( layer );

  if ( mTransactionActive )
    layer->dataProvider()->setTransaction( this );

  return true;
}

bool QgsTransaction::begin( QString &errorMsg, int statementTimeout )
{
  if ( mTransactionActive )
    return false;

  //Set all layers to direct edit mode
  if ( !beginTransaction( errorMsg, statementTimeout ) )
    return false;

  setLayerTransactionIds( this );
  mTransactionActive = true;
  mSavepoints.clear();
  return true;
}

bool QgsTransaction::commit( QString &errorMsg )
{
  if ( !mTransactionActive )
    return false;

  if ( !commitTransaction( errorMsg ) )
    return false;

  setLayerTransactionIds( nullptr );
  mTransactionActive = false;
  mSavepoints.clear();
  return true;
}

bool QgsTransaction::rollback( QString &errorMsg )
{
  if ( !mTransactionActive )
    return false;

  if ( !rollbackTransaction( errorMsg ) )
    return false;

  setLayerTransactionIds( nullptr );
  mTransactionActive = false;
  mSavepoints.clear();

  emit afterRollback();

  return true;
}

bool QgsTransaction::supportsTransaction( const QgsVectorLayer *layer )
{
  std::unique_ptr< QLibrary > lib( QgsProviderRegistry::instance()->createProviderLibrary( layer->providerType() ) );
  if ( !lib )
    return false;

  return lib->resolve( "createTransaction" );
}

void QgsTransaction::onLayerDeleted()
{
  mLayers.remove( static_cast<QgsVectorLayer *>( sender() ) );
}

void QgsTransaction::setLayerTransactionIds( QgsTransaction *transaction )
{
  Q_FOREACH ( QgsVectorLayer *vl, mLayers )
  {
    if ( vl->dataProvider() )
    {
      vl->dataProvider()->setTransaction( transaction );
    }
  }
}

QString QgsTransaction::createSavepoint( QString &error SIP_OUT )
{
  if ( !mTransactionActive )
    return QString();

  if ( !mLastSavePointIsDirty && !mSavepoints.isEmpty() )
    return mSavepoints.top();

  const QString name( QUuid::createUuid().toString() );

  if ( !executeSql( QStringLiteral( "SAVEPOINT %1" ).arg( QgsExpression::quotedColumnRef( name ) ), error ) )
  {
    QgsMessageLog::logMessage( tr( "Could not create savepoint (%1)" ).arg( error ) );
    return QString();
  }

  mSavepoints.push( name );
  mLastSavePointIsDirty = false;
  return name;
}

QString QgsTransaction::createSavepoint( const QString &savePointId, QString &error SIP_OUT )
{
  if ( !mTransactionActive )
    return QString();

  if ( !executeSql( QStringLiteral( "SAVEPOINT %1" ).arg( QgsExpression::quotedColumnRef( savePointId ) ), error ) )
  {
    QgsMessageLog::logMessage( tr( "Could not create savepoint (%1)" ).arg( error ) );
    return QString();
  }

  mSavepoints.push( savePointId );
  mLastSavePointIsDirty = false;
  return savePointId;
}

bool QgsTransaction::rollbackToSavepoint( const QString &name, QString &error SIP_OUT )
{
  if ( !mTransactionActive )
    return false;

  const int idx = mSavepoints.indexOf( name );

  if ( idx == -1 )
    return false;

  mSavepoints.resize( idx );
  mLastSavePointIsDirty = false;
  return executeSql( QStringLiteral( "ROLLBACK TO SAVEPOINT %1" ).arg( QgsExpression::quotedColumnRef( name ) ), error );
}

void QgsTransaction::dirtyLastSavePoint()
{
  mLastSavePointIsDirty = true;
}
