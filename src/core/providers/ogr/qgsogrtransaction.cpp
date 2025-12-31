/***************************************************************************
    QgsOgrTransaction.cpp -  Transaction support for OGR layers
                             -------------------
    begin                : June 13, 2018
    copyright            : (C) 2018 by Even Rouault
    email                : even.rouault @ spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrtransaction.h"

#include "moc_qgsogrtransaction.cpp"

///@cond PRIVATE

#include "qgsogrprovider.h"
#include "qgslogger.h"
#include "qgis.h"

QgsOgrTransaction::QgsOgrTransaction( const QString &connString, QgsOgrDatasetSharedPtr ds )
  : QgsTransaction( connString )
  , mSharedDS( std::move( ds ) )
{
  Q_ASSERT( mSharedDS );
}

bool QgsOgrTransaction::beginTransaction( QString &error, int /* statementTimeout */ )
{
  GDALDriverH hDriver = GDALGetDatasetDriver( mSharedDS.get()->mDs->hDS );
  const QString driverName = GDALGetDriverShortName( hDriver );
  if ( driverName == "GPKG"_L1 || driverName == "SQLite"_L1 )
  {
    QString fkDeferError;
    if ( ! executeSql( u"PRAGMA defer_foreign_keys = ON"_s, fkDeferError ) )
    {
      QgsDebugError( u"Error setting PRAGMA defer_foreign_keys = ON: %1"_s.arg( fkDeferError ) );
    }
  }
  return executeSql( u"BEGIN"_s, error );
}

bool QgsOgrTransaction::commitTransaction( QString &error )
{
  return executeSql( u"COMMIT"_s, error );
}

bool QgsOgrTransaction::rollbackTransaction( QString &error )
{
  return executeSql( u"ROLLBACK"_s, error );
}

bool QgsOgrTransaction::executeSql( const QString &sql, QString &errorMsg, bool isDirty, const QString &name )
{

  QString err;
  if ( isDirty )
  {
    createSavepoint( err );
  }

  QgsDebugMsgLevel( u"Transaction sql: %1"_s.arg( sql ), 2 );
  if ( !mSharedDS->executeSQLNoReturn( sql ) )
  {
    errorMsg = CPLGetLastErrorMsg();
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

  QgsDebugMsgLevel( u"... ok"_s, 2 );
  return true;
}

///@endcond
