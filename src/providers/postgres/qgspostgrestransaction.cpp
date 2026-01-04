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

#include "qgis.h"
#include "qgslogger.h"
#include "qgspostgresconn.h"

#include "moc_qgspostgrestransaction.cpp"

QgsPostgresTransaction::QgsPostgresTransaction( const QString &connString )
  : QgsTransaction( connString )

{
}

bool QgsPostgresTransaction::beginTransaction( QString &error, int statementTimeout )
{
  mConn = QgsPostgresConn::connectDb( mConnString, false /*readonly*/, false /*shared*/, true /*transaction*/ );

  return executeSql( u"SET statement_timeout = %1"_s.arg( statementTimeout * 1000 ), error )
         && executeSql( u"BEGIN TRANSACTION"_s, error );
}

bool QgsPostgresTransaction::commitTransaction( QString &error )
{
  if ( executeSql( u"COMMIT TRANSACTION"_s, error ) )
  {
    mConn->unref();
    mConn = nullptr;
    return true;
  }
  return false;
}

bool QgsPostgresTransaction::rollbackTransaction( QString &error )
{
  if ( executeSql( u"ROLLBACK TRANSACTION"_s, error ) )
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

  QgsDebugMsgLevel( u"Transaction sql: %1"_s.arg( sql ), 2 );
  QgsPostgresResult r( mConn->LoggedPQexec( "QgsPostgresTransaction", sql ) );
  if ( r.PQresultStatus() == PGRES_BAD_RESPONSE || r.PQresultStatus() == PGRES_FATAL_ERROR )
  {
    errorMsg = u"Status %1 (%2)"_s.arg( r.PQresultStatus() ).arg( r.PQresultErrorMessage() );
    QgsDebugError( errorMsg );

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

  QgsDebugMsgLevel( u"Status %1 (OK)"_s.arg( r.PQresultStatus() ), 2 );
  return true;
}
