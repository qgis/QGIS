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

{

}

bool QgsPostgresTransaction::beginTransaction( QString &error, int statementTimeout )
{
  mConn = QgsPostgresConn::connectDb( mConnString, false /*readonly*/, false /*shared*/, true /*transaction*/ );

  return executeSql( QStringLiteral( "SET statement_timeout = %1" ).arg( statementTimeout * 1000 ), error )
         && executeSql( QStringLiteral( "BEGIN TRANSACTION" ), error );
}

bool QgsPostgresTransaction::commitTransaction( QString &error )
{
  if ( executeSql( QStringLiteral( "COMMIT TRANSACTION" ), error ) )
  {
    mConn->unref();
    mConn = nullptr;
    return true;
  }
  return false;
}

bool QgsPostgresTransaction::rollbackTransaction( QString &error )
{
  if ( executeSql( QStringLiteral( "ROLLBACK TRANSACTION" ), error ) )
  {
    mConn->unref();
    mConn = nullptr;
    return true;
  }
  return false;
}

bool QgsPostgresTransaction::executeSql( const QString &sql, QString &errorMsg, bool isDirty, const QString &name )
{
  if ( !mConn )
  {
    errorMsg = tr( "Connection to the database not available" );
    return false;
  }

  QString err;
  if ( isDirty )
  {
    createSavepoint( err );
  }

  QgsDebugMsg( QStringLiteral( "Transaction sql: %1" ).arg( sql ) );
  QgsPostgresResult r( mConn->LoggedPQexec( "QgsPostgresTransaction", sql ) );
  if ( r.PQresultStatus() == PGRES_BAD_RESPONSE ||
       r.PQresultStatus() == PGRES_FATAL_ERROR )
  {
    errorMsg = QStringLiteral( "Status %1 (%2)" ).arg( r.PQresultStatus() ).arg( r.PQresultErrorMessage() );
    QgsDebugMsg( errorMsg );

    if ( isDirty )
    {
      rollbackToSavepoint( savePoints().last(), err );
    }

    return false;
  }

  if ( isDirty )
  {
    dirtyLastSavePoint();
    emit dirtied( sql, name );
  }

  QgsDebugMsg( QStringLiteral( "Status %1 (OK)" ).arg( r.PQresultStatus() ) );
  return true;
}
