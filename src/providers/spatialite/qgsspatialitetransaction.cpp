/***************************************************************************
  qgsspatialitetransaction.cpp - QgsSpatialiteTransaction

 ---------------------
 begin                : 30.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialitetransaction.h"

#include "qgslogger.h"

#include <QDebug>

#include "moc_qgsspatialitetransaction.cpp"

///@cond PRIVATE

QAtomicInt QgsSpatiaLiteTransaction::sSavepointId = 0;

QgsSpatiaLiteTransaction::QgsSpatiaLiteTransaction( const QString &connString, QgsSqliteHandle *sharedHandle )
  : QgsTransaction( connString )
  , mSharedHandle( sharedHandle )
{
  if ( mSharedHandle )
    mSqliteHandle = mSharedHandle->handle();
  mSavepointId = ++sSavepointId;
}

bool QgsSpatiaLiteTransaction::beginTransaction( QString &error, int /* statementTimeout */ )
{
  return executeSql( u"BEGIN"_s, error );
}

bool QgsSpatiaLiteTransaction::commitTransaction( QString &error )
{
  return executeSql( u"COMMIT"_s, error );
}

bool QgsSpatiaLiteTransaction::rollbackTransaction( QString &error )
{
  return executeSql( u"ROLLBACK"_s, error );
}

bool QgsSpatiaLiteTransaction::executeSql( const QString &sql, QString &errorMsg, bool isDirty, const QString &name )
{
  if ( !mSqliteHandle )
  {
    QgsDebugError( u"Spatialite handle is not set"_s );
    return false;
  }

  if ( isDirty )
  {
    createSavepoint( errorMsg );
    if ( !errorMsg.isEmpty() )
    {
      QgsDebugError( errorMsg );
      return false;
    }
  }

  char *errMsg = nullptr;
  if ( sqlite3_exec( mSqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
  {
    if ( isDirty )
    {
      rollbackToSavepoint( savePoints().last(), errorMsg );
    }
    errorMsg = u"%1\n%2"_s.arg( errMsg, errorMsg );
    QgsDebugError( errMsg );
    sqlite3_free( errMsg );
    return false;
  }

  if ( isDirty )
  {
    dirtyLastSavePoint();
    emit dirtied( sql, name );
  }

  QgsDebugMsgLevel( u"... ok"_s, 2 );
  return true;
}

sqlite3 *QgsSpatiaLiteTransaction::sqliteHandle() const
{
  return mSqliteHandle;
}
///@endcond
