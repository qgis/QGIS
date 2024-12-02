/***************************************************************************
 qgscolumntypetask.cpp - lookup oracle geometry type and srid in a thread
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

#include "qgsoraclecolumntypetask.h"
#include "moc_qgsoraclecolumntypetask.cpp"
#include "qgslogger.h"
#include "qgsoracleconnpool.h"

#include <QMetaType>

QgsOracleColumnTypeTask::QgsOracleColumnTypeTask( const QString &name, const QString &limitToSchema, bool useEstimatedMetadata, bool allowGeometrylessTables )
  : QgsTask( tr( "Scanning tables for %1" ).arg( name ) )
  , mName( name )
  , mSchema( limitToSchema )
  , mUseEstimatedMetadata( useEstimatedMetadata )
  , mAllowGeometrylessTables( allowGeometrylessTables )
{
  qRegisterMetaType<QgsOracleLayerProperty>( "QgsOracleLayerProperty" );
}

bool QgsOracleColumnTypeTask::run()
{
  QString conninfo = QgsOracleConn::toPoolName( QgsOracleConn::connUri( mName ) );
  QgsOracleConn *conn = QgsOracleConnPool::instance()->acquireConnection( conninfo );
  if ( !conn )
  {
    QgsDebugError( "Connection failed - " + conninfo );
    return false;
  }

  emit progressMessage( tr( "Retrieving tables of %1…" ).arg( mName ) );
  QVector<QgsOracleLayerProperty> layerProperties;
  if ( !conn->supportedLayers( layerProperties, mSchema, QgsOracleConn::geometryColumnsOnly( mName ), QgsOracleConn::userTablesOnly( mName ), mAllowGeometrylessTables ) || layerProperties.isEmpty() )
  {
    return false;
  }

  int i = 0, n = layerProperties.size();
  for ( QVector<QgsOracleLayerProperty>::iterator it = layerProperties.begin(),
                                                  end = layerProperties.end();
        it != end; ++it )
  {
    QgsOracleLayerProperty &layerProperty = *it;
    if ( !isCanceled() )
    {
      setProgress( ( i * 100. ) / n );
      emit progressMessage( tr( "Scanning column %1.%2.%3…" )
                              .arg( layerProperty.ownerName, layerProperty.tableName, layerProperty.geometryColName ) );
      conn->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata, QgsOracleConn::onlyExistingTypes( mName ) );
    }

    if ( isCanceled() )
    {
      layerProperty.types.clear();
      layerProperty.srids.clear();
    }

    // Now tell the layer list dialog box...
    emit setLayerType( layerProperty );
  }

  // store the list for later use (cache)
  if ( !isCanceled() )
    mLayerProperties = layerProperties;

  setProgress( 100 );
  emit progressMessage( tr( "Table retrieval finished." ) );

  QgsOracleConnPool::instance()->releaseConnection( conn );

  return true;
}
