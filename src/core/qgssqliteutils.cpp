/***************************************************************************
                          qgssqliteutils.cpp
                           -------------------
    begin                : Nov, 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgssqliteutils.h"

#include <sqlite3.h>

void QgsSqlite3Closer::operator()( sqlite3 *database )
{
  sqlite3_close( database );
}

void QgsSqlite3StatementFinalizer::operator()( sqlite3_stmt *statement )
{
  sqlite3_finalize( statement );
}

int sqlite3_statement_unique_ptr::step()
{
  return sqlite3_step( get() );
}

QString sqlite3_statement_unique_ptr::columnName( int column ) const
{
  return QString::fromUtf8( static_cast<const char *>( sqlite3_column_name( get(), column ) ) );
}

double sqlite3_statement_unique_ptr::columnAsDouble( int column ) const
{
  return sqlite3_column_double( get(), column );
}

int sqlite3_statement_unique_ptr::columnCount() const
{
  return sqlite3_column_count( get() );
}

QString sqlite3_statement_unique_ptr::columnAsText( int column ) const
{
  return QString::fromUtf8( reinterpret_cast<const char *>( sqlite3_column_text( get(), column ) ) );
}

qlonglong sqlite3_statement_unique_ptr::columnAsInt64( int column ) const
{
  return sqlite3_column_int64( get(), column );
}

int sqlite3_database_unique_ptr::open( const QString &path )
{
  sqlite3 *database = nullptr;
  int result = sqlite3_open( path.toUtf8(), &database );
  reset( database );
  return result;
}

int sqlite3_database_unique_ptr::open_v2( const QString &path, int flags, const char *zVfs )
{
  sqlite3 *database = nullptr;
  int result = sqlite3_open_v2( path.toUtf8(), &database, flags, zVfs );
  reset( database );
  return result;
}

QString sqlite3_database_unique_ptr::errorMessage() const
{
  return QString( sqlite3_errmsg( get() ) );
}

sqlite3_statement_unique_ptr sqlite3_database_unique_ptr::prepare( const QString &sql, int &resultCode ) const
{
  sqlite3_stmt *preparedStatement = nullptr;
  const char *tail = nullptr;
  resultCode = sqlite3_prepare( get(), sql.toUtf8(), sql.toUtf8().length(), &preparedStatement, &tail );
  sqlite3_statement_unique_ptr s;
  s.reset( preparedStatement );
  return s;
}
