/***************************************************************************
  qgsmssqltransaction.cpp - QgsMssqlTransaction
 ---------------------
 begin                : 11.3.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqltransaction.h"

#include "qgsmssqldatabase.h"

#include "qgsexpression.h"
#include "qgsmessagelog.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>


QgsMssqlTransaction::QgsMssqlTransaction( const QString &connString )
  : QgsTransaction( connString )
{
}

QgsMssqlTransaction::~QgsMssqlTransaction() = default;


bool QgsMssqlTransaction::executeSql( const QString &sql, QString &error, bool isDirty, const QString &name )
{
  QSqlDatabase &database = mConn->db();

  if ( !database.isValid() )
    return false;
  if ( !database.isOpen() )
    return false;

  if ( isDirty )
  {
    QgsTransaction::createSavepoint( error );
    if ( ! error.isEmpty() )
    {
      return false;
    }
  }

  QSqlQuery query( database );
  if ( !query.exec( sql ) )
  {
    if ( isDirty )
      rollbackToSavepoint( savePoints().last(), error );

    QString msg = tr( "MSSQL query failed: %1" ).arg( query.lastError().text() );
    if ( error.isEmpty() )
      error = msg;
    else
      error = QStringLiteral( "%1\n%2" ).arg( error, msg );

    return false;
  }

  if ( isDirty )
  {
    dirtyLastSavePoint();
    emit dirtied( sql, name );
  }

  return true;
}


QString QgsMssqlTransaction::createSavepoint( const QString &savePointId, QString &error )
{
  if ( !mTransactionActive )
    return QString();

  if ( !executeSql( QStringLiteral( "SAVE TRAN %1" ).arg( QgsExpression::quotedColumnRef( savePointId ) ), error ) )
  {
    QgsMessageLog::logMessage( tr( "Could not create savepoint (%1)" ).arg( error ) );
    return QString();
  }

  mSavepoints.push( savePointId );
  mLastSavePointIsDirty = false;
  return savePointId;
}


bool QgsMssqlTransaction::rollbackToSavepoint( const QString &name, QString &error )
{
  if ( !mTransactionActive )
    return false;

  const int idx = mSavepoints.indexOf( name );

  if ( idx == -1 )
    return false;

  mSavepoints.resize( idx );
  // Rolling back always dirties the previous savepoint because
  // the status of the DB has changed between the previous savepoint and the
  // one we are rolling back to.
  mLastSavePointIsDirty = true;
  return executeSql( QStringLiteral( "ROLLBACK TRANSACTION %1" ).arg( QgsExpression::quotedColumnRef( name ) ), error );
}


bool QgsMssqlTransaction::beginTransaction( QString &error, int /*statementTimeout*/ )
{
  mConn = QgsMssqlDatabase::connectDb( mConnString, true /*transaction*/ );

  if ( !mConn->isValid() )
  {
    error = mConn->errorText();
    return false;
  }

  if ( !mConn->db().transaction() )
  {
    error = mConn->errorText();
    return false;
  }

  return true;
}


bool QgsMssqlTransaction::commitTransaction( QString &error )
{
  if ( mConn->db().commit() )
  {
    return true;
  }

  error = mConn->db().lastError().text();
  return false;
}


bool QgsMssqlTransaction::rollbackTransaction( QString &error )
{
  if ( mConn->db().rollback() )
  {
    return true;
  }

  error = mConn->db().lastError().text();
  return false;
}
