/***************************************************************************
    qgsoracletransaction.cpp
    -------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoracletransaction.h"
///@cond PRIVATE

#include "qgslogger.h"
#include "qgis.h"
#include "qgsoracleconn.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"

QgsOracleTransaction::QgsOracleTransaction( const QString &connString )
  : QgsTransaction( connString )

{

}

QgsOracleTransaction::~QgsOracleTransaction()
{
  if ( mConn )
    mConn->unref();
}

bool QgsOracleTransaction::beginTransaction( QString &, int /* statementTimeout */ )
{
  mConn = QgsOracleConn::connectDb( mConnString, true /*transaction*/ );

  return true;
}

bool QgsOracleTransaction::commitTransaction( QString &error )
{
  if ( executeSql( QStringLiteral( "COMMIT" ), error ) )
  {
    mConn->disconnect();
    mConn = nullptr;
    return true;
  }
  return false;
}

bool QgsOracleTransaction::rollbackTransaction( QString &error )
{
  if ( executeSql( QStringLiteral( "ROLLBACK" ), error ) )
  {
    mConn->disconnect();
    mConn = nullptr;
    return true;
  }
  return false;
}

bool QgsOracleTransaction::executeSql( const QString &sql, QString &errorMsg, bool isDirty, const QString &name )
{
  QString err;
  if ( isDirty )
  {
    createSavepoint( err );
  }

  QgsDebugMsgLevel( QStringLiteral( "Transaction sql: %1" ).arg( sql ), 2 );

  QgsDatabaseQueryLogWrapper logWrapper { sql, mConnString, QStringLiteral( "oracle" ), QStringLiteral( "QgsOracleConn" ), QGS_QUERY_LOG_ORIGIN };
  const bool res = mConn->exec( sql, true, &errorMsg );
  if ( ! errorMsg.isEmpty() )
  {
    logWrapper.setError( errorMsg );
  }
  if ( !res )
  {
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

  return true;
}

///@endcond
