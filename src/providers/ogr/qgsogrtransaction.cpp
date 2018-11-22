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
#include "qgsogrprovider.h"
#include "qgslogger.h"
#include "qgis.h"

QgsOgrTransaction::QgsOgrTransaction( const QString &connString, QgsOgrDatasetSharedPtr ds )
  : QgsTransaction( connString ), mSharedDS( ds )

{
  Q_ASSERT( mSharedDS );
}

bool QgsOgrTransaction::beginTransaction( QString &error, int /* statementTimeout */ )
{
  return executeSql( QStringLiteral( "BEGIN" ), error );
}

bool QgsOgrTransaction::commitTransaction( QString &error )
{
  return executeSql( QStringLiteral( "COMMIT" ), error );
}

bool QgsOgrTransaction::rollbackTransaction( QString &error )
{
  return executeSql( QStringLiteral( "ROLLBACK" ), error );
}

bool QgsOgrTransaction::executeSql( const QString &sql, QString &errorMsg, bool isDirty, const QString &name )
{

  QString err;
  if ( isDirty )
  {
    createSavepoint( err );
  }

  QgsDebugMsg( QStringLiteral( "Transaction sql: %1" ).arg( sql ) );
  if ( !mSharedDS->executeSQLNoReturn( sql ) )
  {
    errorMsg = CPLGetLastErrorMsg();
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

  QgsDebugMsg( QStringLiteral( "... ok" ) );
  return true;
}
