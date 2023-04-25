/***************************************************************************
   qgscolumntypethread.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgscolumntypethread.h"

#include <QMetaType>
#include <climits>

#include "qgslogger.h"
#include "qgsredshiftconnpool.h"

QgsGeomColumnTypeThread::QgsGeomColumnTypeThread( const QString &name, bool useEstimatedMetaData,
    bool allowGeometrylessTables )
  : mName( name ), mUseEstimatedMetadata( useEstimatedMetaData ), mAllowGeometrylessTables( allowGeometrylessTables ),
    mStopped( false )
{
  qRegisterMetaType<QgsRedshiftLayerProperty>( "QgsRedshiftLayerProperty" );
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
  QgsDataSourceUri uri = QgsRedshiftConn::connUri( mName );
  mConn = QgsRedshiftConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !mConn )
  {
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo( false ) );
    return;
  }

  mStopped = false;

  bool dontResolveType = QgsRedshiftConn::dontResolveType( mName );

  emit progressMessage( tr( "Retrieving tables of %1…" ).arg( mName ) );
  QVector<QgsRedshiftLayerProperty> layerProperties;
  if ( !mConn->supportedLayers( layerProperties, mAllowGeometrylessTables ) || layerProperties.isEmpty() )
  {
    QgsRedshiftConnPool::instance()->releaseConnection( mConn );
    mConn = nullptr;
    return;
  }

  int totalLayers = layerProperties.length();
  int addedLayers = 0;

  emit progress( addedLayers, totalLayers );

  QVector<QgsRedshiftLayerProperty *> unrestrictedLayers;

  for ( auto &layerProperty : layerProperties )
  {
    if ( !layerProperty.geometryColName.isNull() &&
         ( layerProperty.types.value( 0, Qgis::WkbType::Unknown ) == Qgis::WkbType::Unknown ||
           layerProperty.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) )
    {
      unrestrictedLayers << &layerProperty;
    }
  }

  if ( mStopped )
  {
    emit progress( 0, 0 );
    emit progressMessage( tr( "Table retrieval stopped." ) );
    QgsRedshiftConnPool::instance()->releaseConnection( mConn );
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

  QgsRedshiftConnPool::instance()->releaseConnection( mConn );
  mConn = nullptr;
}
