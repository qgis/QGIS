/***************************************************************************
 qgscolumntypethread.cpp - lookup oracle geometry type and srid in a thread
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

#include "qgsoraclecolumntypethread.h"
#include "qgslogger.h"

#include <QMetaType>

QgsOracleColumnTypeThread::QgsOracleColumnTypeThread( QString name, bool useEstimatedMetadata )
    : QThread()
    , mName( name )
    , mUseEstimatedMetadata( useEstimatedMetadata )
{
  qRegisterMetaType<QgsOracleLayerProperty>( "QgsOracleLayerProperty" );
}

void QgsOracleColumnTypeThread::stop()
{
  mStopped = true;
}

void QgsOracleColumnTypeThread::run()
{
  QgsDataSourceURI uri = QgsOracleConn::connUri( mName );
  QgsOracleConn *conn = QgsOracleConn::connectDb( uri.connectionInfo() );
  if ( !conn )
  {
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo() );
    return;
  }

  mStopped = false;

  emit progressMessage( tr( "Retrieving tables of %1..." ).arg( mName ) );
  QVector<QgsOracleLayerProperty> layerProperties;
  if ( !conn->supportedLayers( layerProperties,
                               QgsOracleConn::geometryColumnsOnly( mName ),
                               QgsOracleConn::userTablesOnly( mName ),
                               QgsOracleConn::allowGeometrylessTables( mName ) ) ||
       layerProperties.isEmpty() )
  {
    return;
  }

  int i = 0;
  foreach ( QgsOracleLayerProperty layerProperty, layerProperties )
  {
    if ( !mStopped )
    {
      emit progress( i++, layerProperties.size() );
      emit progressMessage( tr( "Scanning column %1.%2.%3..." )
                            .arg( layerProperty.ownerName )
                            .arg( layerProperty.tableName )
                            .arg( layerProperty.geometryColName ) );
      conn->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata );
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

  conn->disconnect();
}
