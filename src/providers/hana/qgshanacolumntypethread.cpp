/***************************************************************************
   qgshanacolumntypethread.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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
#include "qgshanaconnection.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include <mutex>

QgsHanaColumnTypeThread::QgsHanaColumnTypeThread( const QString &connName, const QgsDataSourceUri &uri, bool allowGeometrylessTables, bool userTablesOnly )
  : mConnectionName( connName )
  , mUri( uri )
  , mAllowGeometrylessTables( allowGeometrylessTables )
  , mUserTablesOnly( userTablesOnly )
  , mStopped( false )
{
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    qRegisterMetaType<QgsHanaLayerProperty>( "QgsHanaLayerProperty" );
  } );
}

void QgsHanaColumnTypeThread::stop()
{
  mStopped = true;
}

void QgsHanaColumnTypeThread::run()
{
  mStopped = false;

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
  {
    QgsDebugMsg( "Connection failed - " + conn->connInfo() );
    mStopped = true;
    return;
  }

  emit progressMessage( tr( "Retrieving tables of %1 ." ).arg( mConnectionName ) );
  QVector<QgsHanaLayerProperty> layerProperties = conn->getLayers(
        mUri.schema(),
        mAllowGeometrylessTables,
        mUserTablesOnly );

  if ( layerProperties.isEmpty() )
  {
    QgsMessageLog::logMessage(
      QObject::tr( "Unable to get list of spatially enabled tables from the database" ), tr( "HANA" ) );
    return;
  }

  const int totalLayers = layerProperties.size();

  emit progress( 0, totalLayers );

  for ( int i = 0; i < totalLayers; ++i )
  {
    if ( mStopped )
      break;

    QgsHanaLayerProperty &layerProperty = layerProperties[i];
    emit progress( i, totalLayers );
    emit progressMessage( tr( "Scanning column %1.%2.%3…" )
                          .arg( layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName ) );
    conn->readLayerInfo( layerProperty );

    emit setLayerType( layerProperty );
  }

  emit progress( 0, 0 );
  if ( mStopped )
    emit progressMessage( tr( "Table retrieval stopped." ) );
  else
    emit progressMessage( tr( "Table retrieval finished." ) );
}
