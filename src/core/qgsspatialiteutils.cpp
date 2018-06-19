/***************************************************************************
                          qgsspatialiteutils.cpp
                           -------------------
    begin                : Nov, 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsspatialiteutils.h"
#include "qgslogger.h"

#include <sqlite3.h>
#include <spatialite.h>

int spatialite_database_unique_ptr::open( const QString &path )
{
  auto &deleter = get_deleter();
  deleter.mSpatialiteContext = spatialite_alloc_connection();

  sqlite3 *database = nullptr;
  int result = sqlite3_open( path.toUtf8(), &database );
  std::unique_ptr< sqlite3, QgsSpatialiteCloser>::reset( database );

  if ( result == SQLITE_OK )
    spatialite_init_ex( database, deleter.mSpatialiteContext, 0 );

  return result;
}

void spatialite_database_unique_ptr::reset()
{
  std::unique_ptr< sqlite3, QgsSpatialiteCloser>::reset();
}

int spatialite_database_unique_ptr::open_v2( const QString &path, int flags, const char *zVfs )
{
  auto &deleter = get_deleter();
  deleter.mSpatialiteContext = spatialite_alloc_connection();

  sqlite3 *database = nullptr;
  int result = sqlite3_open_v2( path.toUtf8(), &database, flags, zVfs );
  std::unique_ptr< sqlite3, QgsSpatialiteCloser>::reset( database );

  if ( result == SQLITE_OK )
    spatialite_init_ex( database, deleter.mSpatialiteContext, 0 );

  return result;
}

QString spatialite_database_unique_ptr::errorMessage() const
{
  return QString( sqlite3_errmsg( get() ) );
}

sqlite3_statement_unique_ptr spatialite_database_unique_ptr::prepare( const QString &sql, int &resultCode )
{
  sqlite3_stmt *preparedStatement = nullptr;
  const char *tail = nullptr;
  const QByteArray sqlUtf8 = sql.toUtf8();
  resultCode = sqlite3_prepare( get(), sqlUtf8, sqlUtf8.length(), &preparedStatement, &tail );
  sqlite3_statement_unique_ptr s;
  s.reset( preparedStatement );
  return s;
}

void QgsSpatialiteCloser::operator()( sqlite3 *handle )
{
  int res = sqlite3_close_v2( handle );
  if ( res != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "sqlite3_close_v2() failed: %1" ).arg( res ) );
  }

  spatialite_cleanup_ex( mSpatialiteContext );
  mSpatialiteContext = nullptr;
}
