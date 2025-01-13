/***************************************************************************
  qgsdamengcolumntypethread.cpp - lookup dameng geometry type and srid in a thread
                              -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengcolumntypethread.h"
#include "qgsdamengconnpool.h"
#include "qgslogger.h"

#include <QMetaType>
#include <climits>

QgsDamengGeomColumnTypeThread::QgsDamengGeomColumnTypeThread( const QString &name, bool useEstimatedMetaData, bool allowGeometrylessTables )
  : mName( name )
  , mUseEstimatedMetadata( useEstimatedMetaData )
  , mAllowGeometrylessTables( allowGeometrylessTables )
  , mStopped( false )
{
  qRegisterMetaType<QgsDamengLayerProperty>( "QgsDamengLayerProperty" );
}

void QgsDamengGeomColumnTypeThread::stop()
{
  if ( !mConn )
    return;

  mConn->cancel();
  mStopped = true;
}

void QgsDamengGeomColumnTypeThread::run()
{
  QgsDataSourceUri uri = QgsDamengConn::connUri( mName );
  mConn = QgsDamengConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !mConn )
  {
    QgsDebugError( "Connection failed - " + uri.connectionInfo( false ) );
    return;
  }

  mStopped = false;

  bool dontResolveType = QgsDamengConn::dontResolveType( mName );

  emit progressMessage( tr( "Retrieving tables of %1…" ).arg( mName ) );
  QVector<QgsDamengLayerProperty> layerProperties;
  if ( !mConn->supportedLayers( layerProperties, QgsDamengConn::sysdbaSchemaOnly( mName ), mAllowGeometrylessTables ) || layerProperties.isEmpty() )
  {
    QgsDamengConnPool::instance()->releaseConnection( mConn );
    mConn = nullptr;
    return;
  }

  int totalLayers = layerProperties.length();
  int addedLayers = 0;

  emit progress( addedLayers, totalLayers );

  QVector<QgsDamengLayerProperty *> unrestrictedLayers;

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
    QgsDamengConnPool::instance()->releaseConnection( mConn );
    mConn = nullptr;
    return;
  }

  if ( ! dontResolveType )
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

  QgsDamengConnPool::instance()->releaseConnection( mConn );
  mConn = nullptr;
}
