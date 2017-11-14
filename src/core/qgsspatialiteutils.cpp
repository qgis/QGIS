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

#include <sqlite3.h>
#include <spatialite.h>

int spatialite_database_unique_ptr::open( const QString &path )
{

#if defined(SPATIALITE_HAS_INIT_EX)
  void *conn = spatialite_alloc_connection();
#else
  spatialite_init( 0 );
#endif

  sqlite3 *database = nullptr;
  int result = sqlite3_open( path.toUtf8(), &database );
  reset( database );

#if defined(SPATIALITE_HAS_INIT_EX)
  if ( result == SQLITE_OK )
    spatialite_init_ex( database, conn, 0 );
#endif

  return result;
}

int spatialite_database_unique_ptr::open_v2( const QString &path, int flags, const char *zVfs )
{
#if defined(SPATIALITE_HAS_INIT_EX)
  void *conn = spatialite_alloc_connection();
#else
  spatialite_init( 0 );
#endif

  sqlite3 *database = nullptr;
  int result = sqlite3_open_v2( path.toUtf8(), &database, flags, zVfs );
  reset( database );

#if defined(SPATIALITE_HAS_INIT_EX)
  if ( result == SQLITE_OK )
    spatialite_init_ex( database, conn, 0 );
#endif

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
  resultCode = sqlite3_prepare( get(), sql.toUtf8(), sql.toUtf8().length(), &preparedStatement, &tail );
  sqlite3_statement_unique_ptr s;
  s.reset( preparedStatement );
  return s;
}

void QgsSpatialiteCloser::operator()( sqlite3 *database )
{
  sqlite3_close( database );

#if defined(SPATIALITE_HAS_INIT_EX)
  spatialite_cleanup_ex( database );
#endif

}
