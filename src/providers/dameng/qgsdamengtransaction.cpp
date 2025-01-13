/***************************************************************************
    qgsdamengtransaction.cpp  -  Transaction support for Dameng layers
                             -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengdatabase.h"
#include "qgsdamengtransaction.h"
#include "qgsdamengconn.h"
#include "qgslogger.h"
#include "qgis.h"

QgsDamengTransaction::QgsDamengTransaction( const QString &connString )
  : QgsTransaction( connString )
{

}

QgsDamengTransaction::~QgsDamengTransaction() = default;

QString QgsDamengTransaction::createSavepoint( const QString &savePointId, QString &error )
{
  if ( !mTransactionActive )
    return QString();

  if ( !executeSql( QStringLiteral( "SAVEPOINT %1" ).arg( QgsExpression::quotedColumnRef( savePointId ) ), error ) )
  {
    QgsDebugMsgLevel( tr( "Could not create savepoint (%1)" ).arg( error ), 2 );
    return QString();
  }

  mSavepoints.push( savePointId );
  mLastSavePointIsDirty = false;
  return savePointId;
}


bool QgsDamengTransaction::rollbackToSavepoint( const QString &name, QString &error )
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
  return executeSql( QStringLiteral( "ROLLBACK TO SAVEPOINT %1" ).arg( QgsExpression::quotedColumnRef( name ) ), error );
}


bool QgsDamengTransaction::beginTransaction( QString &error, int statementTimeout )
{
  Q_UNUSED( statementTimeout )
  mConn = QgsDamengConn::connectDb( mConnString, false /*readonly*/, false /*shared*/, true /*transaction*/ );

  if ( !mConn )
  {
    error = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( mConn->dmConnection()->dmDriver->beginTransaction() )
  {
    return true;
  }

  error = mConn->dmConnection()->dmDriver->getConnMsg();
  return false;
}

bool QgsDamengTransaction::commitTransaction( QString &error )
{
  if ( mConn->dmConnection()->dmDriver->commitTransaction() )
  {
    return true;
  }

  error = mConn->dmConnection()->dmDriver->getConnMsg();
  return false;
}

bool QgsDamengTransaction::rollbackTransaction( QString &error )
{
  if ( mConn->dmConnection()->dmDriver->rollbackTransaction() )
  {
    return true;
  }

  error = mConn->dmConnection()->dmDriver->getConnMsg();
  return false;
}

bool QgsDamengTransaction::executeSql( const QString &sql, QString &errorMsg, bool isDirty, const QString &name )
{
  if ( !mConn )
  {
    errorMsg = tr( "Connection to the database not available" );
    return false;
  }

  QString err;
  if ( isDirty )
  {
    QgsTransaction::createSavepoint( err );
    if ( !err.isEmpty() )
    {
      return false;
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "Transaction sql: %1" ).arg( sql ), 2 );
  QgsDamengResult r( mConn->DMexec( sql, true ) );
  if ( r.DMresultStatus() == DmResFatalError )
  {
    errorMsg = QStringLiteral( "Status %1 (%2)" ).arg( r.DMresultStatus() ).arg( r.DMresultErrorMessage() );
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

  QgsDebugMsgLevel( QStringLiteral( "Status %1 (OK)" ).arg( r.DMresultStatus() ), 2 );
  return true;
}
