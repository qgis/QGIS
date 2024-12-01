/***************************************************************************
 qgscolumntypethread.cpp - lookup postgres geometry type and srid in a thread
                              -------------------
begin                : 3.1.2012
copyright            : (C) 2012 by Juergen E. Fischer
email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolumntypethread.h"
#include "moc_qgscolumntypethread.cpp"
#include "qgspostgresconnpool.h"
#include "qgslogger.h"

#include <QMetaType>
#include <climits>

QgsGeomColumnTypeThread::QgsGeomColumnTypeThread( const QString &name, bool useEstimatedMetaData, bool allowGeometrylessTables )
  : mName( name )
  , mUseEstimatedMetadata( useEstimatedMetaData )
  , mAllowGeometrylessTables( allowGeometrylessTables )
  , mStopped( false )
{
  qRegisterMetaType<QgsPostgresLayerProperty>( "QgsPostgresLayerProperty" );
}

void QgsGeomColumnTypeThread::stop()
{
  if ( !mConn )
    return;

  mConn->cancel();
  mStopped = true;
}

void QgsGeomColumnTypeThread::run()
{
  QgsDataSourceUri uri = QgsPostgresConn::connUri( mName );
  mConn = QgsPostgresConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !mConn )
  {
    QgsDebugError( "Connection failed - " + uri.connectionInfo( false ) );
    return;
  }

  mStopped = false;

  bool dontResolveType = QgsPostgresConn::dontResolveType( mName );

  emit progressMessage( tr( "Retrieving tables of %1…" ).arg( mName ) );
  QVector<QgsPostgresLayerProperty> layerProperties;
  if ( !mConn->supportedLayers( layerProperties, QgsPostgresConn::geometryColumnsOnly( mName ), QgsPostgresConn::publicSchemaOnly( mName ), mAllowGeometrylessTables ) || layerProperties.isEmpty() )
  {
    QgsPostgresConnPool::instance()->releaseConnection( mConn );
    mConn = nullptr;
    return;
  }

  int totalLayers = layerProperties.length();
  int addedLayers = 0;

  emit progress( addedLayers, totalLayers );

  QVector<QgsPostgresLayerProperty *> unrestrictedLayers;

  for ( auto &layerProperty : layerProperties )
  {
    if ( !layerProperty.geometryColName.isNull() && ( layerProperty.types.value( 0, Qgis::WkbType::Unknown ) == Qgis::WkbType::Unknown || layerProperty.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) )
    {
      unrestrictedLayers << &layerProperty;
    }
  }

  if ( mStopped )
  {
    emit progress( 0, 0 );
    emit progressMessage( tr( "Table retrieval stopped." ) );
    QgsPostgresConnPool::instance()->releaseConnection( mConn );
    mConn = nullptr;
    return;
  }

  if ( !dontResolveType )
  {
    mConn->retrieveLayerTypes( unrestrictedLayers, mUseEstimatedMetadata );
  }

  for ( const auto &layerProperty : layerProperties )
  {
    // Tell the layer list dialog box about the completed layers
    emit setLayerType( layerProperty );
    emit progress( ++addedLayers, totalLayers );
  }

  emit progress( 0, 0 );
  emit progressMessage( mStopped ? tr( "Table retrieval stopped." ) : tr( "Table retrieval finished." ) );

  QgsPostgresConnPool::instance()->releaseConnection( mConn );
  mConn = nullptr;
}
