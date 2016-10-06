/***************************************************************************
    qgspostgrestransaction.cpp  -  Transaction support for PostgreSQL/PostGIS layers
                             -------------------
    begin                : Jan 8, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgrestransaction.h"
#include "qgspostgresconn.h"
#include "qgslogger.h"
#include "qgis.h"

QgsPostgresTransaction::QgsPostgresTransaction( const QString &connString )
    : QgsTransaction( connString )
    , mConn( nullptr )
{

}

bool QgsPostgresTransaction::beginTransaction( QString &error, int statementTimeout )
{
  mConn = QgsPostgresConn::connectDb( mConnString, false /*readonly*/, false /*shared*/, true /*transaction*/ );

  return executeSql( QString( "SET statement_timeout = %1" ).arg( statementTimeout * 1000 ), error )
         && executeSql( "BEGIN TRANSACTION", error );
}

bool QgsPostgresTransaction::commitTransaction( QString &error )
{
  if ( executeSql( "COMMIT TRANSACTION", error ) )
  {
    mConn->unref();
    mConn = nullptr;
    return true;
  }
  return false;
}

bool QgsPostgresTransaction::rollbackTransaction( QString &error )
{
  if ( executeSql( "ROLLBACK TRANSACTION", error ) )
  {
    mConn->unref();
    mConn = nullptr;
    return true;
  }
  return false;
}

bool QgsPostgresTransaction::executeSql( const QString &sql, QString &errorMsg )
{
  if ( !mConn )
  {
    return false;
  }

  QgsDebugMsg( QString( "Transaction sql: %1" ).arg( sql ) );
  mConn->lock();
  QgsPostgresResult r( mConn->PQexec( sql, true ) );
  mConn->unlock();
  if ( r.PQresultStatus() != PGRES_COMMAND_OK )
  {
    errorMsg = QString( "Status %1 (%2)" ).arg( r.PQresultStatus() ).arg( r.PQresultErrorMessage() );
    QgsDebugMsg( errorMsg );
    return false;
  }
  QgsDebugMsg( QString( "Status %1 (OK)" ).arg( r.PQresultStatus() ) );
  return true;
}
