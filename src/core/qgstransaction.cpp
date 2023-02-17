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
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"
#include "qgsmessagelog.h"
#include <QUuid>

QgsTransaction *QgsTransaction::create( const QString &connString, const QString &providerKey )
{
  return QgsProviderRegistry::instance()->createTransaction( providerKey, connString );
}

QgsTransaction *QgsTransaction::create( const QSet<QgsVectorLayer *> &layers )
{
  if ( layers.isEmpty() )
    return nullptr;

  QgsVectorLayer *firstLayer = *layers.constBegin();

  const QString connStr = connectionString( firstLayer->source() );
  const QString providerKey = firstLayer->providerType();
  std::unique_ptr<QgsTransaction> transaction( QgsTransaction::create( connStr, providerKey ) );
  if ( transaction )
  {
    for ( QgsVectorLayer *layer : layers )
    {
      if ( !transaction->addLayer( layer, false ) )
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

QString QgsTransaction::connectionString() const
{
  return mConnString;
}

// For the needs of the OGR provider with GeoPackage datasources, remove
// any reference to layers in the connection string
QString QgsTransaction::removeLayerIdOrName( const QString &str )
{
  QString res( str );

  for ( int i = 0; i < 2; i++ )
  {
    const int pos = res.indexOf( i == 0 ? QLatin1String( "|layername=" ) :  QLatin1String( "|layerid=" ) );
    if ( pos >= 0 )
    {
      const int end = res.indexOf( '|', pos + 1 );
      if ( end >= 0 )
      {
        res = res.mid( 0, pos ) + res.mid( end );
      }
      else
      {
        res = res.mid( 0, pos );
      }
    }
  }
  return res;
}

///@cond PRIVATE
QString QgsTransaction::connectionString( const QString &layerUri )
{
  QString connString = QgsDataSourceUri( layerUri ).connectionInfo( false );
  // In the case of a OGR datasource, connectionInfo() will return an empty
  // string. In that case, use the layer->source() itself, and strip any
  // reference to layers from it.
  if ( connString.isEmpty() )
  {
    connString = removeLayerIdOrName( layerUri );
  }
  return connString;
}
///@endcond

bool QgsTransaction::addLayer( QgsVectorLayer *layer, bool addLayersInEditMode )
{
  if ( !layer )
    return false;

  if ( ! addLayersInEditMode
       && layer->isEditable() )
    return false;

  //test if provider supports transactions
  if ( !supportsTransaction( layer ) )
    return false;

  if ( layer->dataProvider()->transaction() )
    return false;

  //connection string not compatible

  if ( connectionString( layer->source() ) != mConnString )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't start transaction because connection string for layer %1 : '%2' does not match '%3'" ).arg(
                   layer->id(), connectionString( layer->source() ), mConnString ) );
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
  //test if provider supports transactions
  if ( !layer->dataProvider() || ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::TransactionSupport ) == 0 )
    return false;

  return true;
}

void QgsTransaction::onLayerDeleted()
{
  mLayers.remove( static_cast<QgsVectorLayer *>( sender() ) );
}

void QgsTransaction::setLayerTransactionIds( QgsTransaction *transaction )
{
  const auto constMLayers = mLayers;
  for ( QgsVectorLayer *vl : constMLayers )
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
  {
    return mSavepoints.top();
  }

  const QString name( QStringLiteral( "qgis" ) + ( QUuid::createUuid().toString().mid( 1, 24 ).replace( '-', QString() ) ) );
  return createSavepoint( name, error );
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
  // Rolling back always dirties the previous savepoint because
  // the status of the DB has changed between the previous savepoint and the
  // one we are rolling back to.
  mLastSavePointIsDirty = true;
  return executeSql( QStringLiteral( "ROLLBACK TO SAVEPOINT %1" ).arg( QgsExpression::quotedColumnRef( name ) ), error );
}

void QgsTransaction::dirtyLastSavePoint()
{
  mLastSavePointIsDirty = true;
}
