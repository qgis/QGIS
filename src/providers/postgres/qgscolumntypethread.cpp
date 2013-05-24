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
#include "qgslogger.h"

#include <QMetaType>

QgsGeomColumnTypeThread::QgsGeomColumnTypeThread( QString name, bool useEstimatedMetaData, bool allowGeometrylessTables )
    : QThread()
    , mConn( 0 )
    , mName( name )
    , mUseEstimatedMetadata( useEstimatedMetaData )
    , mAllowGeometrylessTables( allowGeometrylessTables )
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
  QgsDataSourceURI uri = QgsPostgresConn::connUri( mName );
  mConn = QgsPostgresConn::connectDb( uri.connectionInfo(), true );
  if ( !mConn )
  {
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo() );
    return;
  }

  mStopped = false;

  bool dontResolveType = QgsPostgresConn::dontResolveType( mName );

  emit progressMessage( tr( "Retrieving tables of %1..." ).arg( mName ) );
  QVector<QgsPostgresLayerProperty> layerProperties;
  if ( !mConn->supportedLayers( layerProperties,
                                QgsPostgresConn::geometryColumnsOnly( mName ),
                                QgsPostgresConn::publicSchemaOnly( mName ),
                                mAllowGeometrylessTables ) ||
       layerProperties.isEmpty() )
  {
    return;
  }

  int i = 0;
  foreach ( QgsPostgresLayerProperty layerProperty, layerProperties )
  {
    if ( !mStopped )
    {
      emit progress( i++, layerProperties.size() );
      emit progressMessage( tr( "Scanning column %1.%2.%3..." )
                            .arg( layerProperty.schemaName )
                            .arg( layerProperty.tableName )
                            .arg( layerProperty.geometryColName ) );

      if ( !layerProperty.geometryColName.isNull() &&
           ( layerProperty.types.value( 0, QGis::WKBUnknown ) == QGis::WKBUnknown ||
             layerProperty.srids.value( 0, 0 ) <= 0 ) )
      {
        if ( dontResolveType )
        {
          QgsDebugMsg( QString( "skipping column %1.%2 without type constraint" ).arg( layerProperty.schemaName ).arg( layerProperty.tableName ) );
          continue;
        }

        mConn->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata );
      }
    }

    if ( mStopped )
    {
      layerProperty.types.clear();
      layerProperty.srids.clear();
    }

    // Now tell the layer list dialog box...
    emit setLayerType( layerProperty );
  }

  emit progress( 0, 0 );
  emit progressMessage( tr( "Table retrieval finished." ) );

  mConn->disconnect();
  mConn = 0;
}
