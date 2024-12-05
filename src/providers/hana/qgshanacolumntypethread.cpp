/***************************************************************************
   qgshanacolumntypethread.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanacolumntypethread.h"
#include "moc_qgshanacolumntypethread.cpp"
#include "qgshanaconnection.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaexception.h"
#include "qgshanautils.h"
#include "qgslogger.h"

QgsHanaColumnTypeThread::QgsHanaColumnTypeThread( const QString &connName, const QgsDataSourceUri &uri, bool allowGeometrylessTables, bool userTablesOnly )
  : mConnectionName( connName )
  , mUri( uri )
  , mAllowGeometrylessTables( allowGeometrylessTables )
  , mUserTablesOnly( userTablesOnly )
{
  // Make sure the meta type is registered only once
  static int initialized = qRegisterMetaType<QgsHanaLayerProperty>( "QgsHanaLayerProperty" );
  Q_UNUSED( initialized )
}

void QgsHanaColumnTypeThread::run()
{
  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
  {
    mErrorMessage = tr( "Connection failed: %1" ).arg( mUri.connectionInfo( false ) );
    return;
  }

  emit progressMessage( tr( "Retrieving tables of %1." ).arg( mConnectionName ) );

  try
  {
    QVector<QgsHanaLayerProperty> layerProperties = conn->getLayers(
      mUri.schema(),
      mAllowGeometrylessTables,
      mUserTablesOnly
    );

    if ( layerProperties.isEmpty() )
      return;

    const int totalLayers = layerProperties.size();

    emit progress( 0, totalLayers );

    for ( int i = 0; i < totalLayers; ++i )
    {
      if ( isInterruptionRequested() )
        break;

      QgsHanaLayerProperty &layerProperty = layerProperties[i];
      emit progress( i, totalLayers );
      emit progressMessage( tr( "Scanning column %1.%2.%3…" )
                              .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ) );
      conn->readLayerInfo( layerProperty );

      if ( layerProperty.isValid )
        emit setLayerType( layerProperty );
    }
  }
  catch ( const QgsHanaException &ex )
  {
    mErrorMessage = ex.what();
  }

  emit progress( 0, 0 );
  if ( isInterruptionRequested() )
    emit progressMessage( tr( "Table retrieval stopped." ) );
  else
    emit progressMessage( tr( "Table retrieval finished." ) );
}
