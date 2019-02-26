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
#include <cstdarg>
#include <QVariant>

void QgsSqlite3Closer::operator()( sqlite3 *database )
{
  sqlite3_close_v2( database );
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

int sqlite3_database_unique_ptr::exec( const QString &sql, QString &errorMessage ) const
{
  char *errMsg;

  int ret = sqlite3_exec( get(), sql.toUtf8(), nullptr, nullptr, &errMsg );

  if ( errMsg )
  {
    errorMessage = QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
  }

  return ret;
}

QString QgsSqliteUtils::quotedString( const QString &value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  QString v = value;
  v.replace( '\'', QLatin1String( "''" ) );
  return v.prepend( '\'' ).append( '\'' );
}

QString QgsSqliteUtils::quotedIdentifier( const QString &identifier )
{
  QString id( identifier );
  id.replace( '\"', QLatin1String( "\"\"" ) );
  return id.prepend( '\"' ).append( '\"' );
}

QString QgsSqliteUtils::quotedValue( const QVariant &value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      //SQLite has no boolean literals
      return value.toBool() ? QStringLiteral( "1" ) : QStringLiteral( "0" );

    default:
    case QVariant::String:
      QString v = value.toString();
      // https://www.sqlite.org/lang_expr.html :
      // """A string constant is formed by enclosing the string in single quotes (').
      // A single quote within the string can be encoded by putting two single quotes
      // in a row - as in Pascal. C-style escapes using the backslash character are not supported because they are not standard SQL. """
      return v.replace( '\'', QLatin1String( "''" ) ).prepend( '\'' ).append( '\'' );
  }
}

QStringList QgsSqliteUtils::systemTables()
{
  return QStringList() << QStringLiteral( "SpatialIndex" ) << QStringLiteral( "geom_cols_ref_sys" ) << QStringLiteral( "geometry_columns" )
         << QStringLiteral( "geometry_columns_auth" ) << QStringLiteral( "views_geometry_columns" ) << QStringLiteral( "virts_geometry_columns" )
         << QStringLiteral( "spatial_ref_sys" ) << QStringLiteral( "spatial_ref_sys_all" ) << QStringLiteral( "spatial_ref_sys_aux" )
         << QStringLiteral( "sqlite_sequence" ) << QStringLiteral( "tableprefix_metadata" ) << QStringLiteral( "tableprefix_rasters" )
         << QStringLiteral( "layer_params" ) << QStringLiteral( "layer_statistics" ) << QStringLiteral( "layer_sub_classes" )
         << QStringLiteral( "layer_table_layout" ) << QStringLiteral( "pattern_bitmaps" ) << QStringLiteral( "symbol_bitmaps" )
         << QStringLiteral( "project_defs" ) << QStringLiteral( "raster_pyramids" ) << QStringLiteral( "sqlite_stat1" ) << QStringLiteral( "sqlite_stat2" )
         << QStringLiteral( "spatialite_history" ) << QStringLiteral( "geometry_columns_field_infos" ) << QStringLiteral( "geometry_columns_statistics" )
         << QStringLiteral( "geometry_columns_time" ) << QStringLiteral( "sql_statements_log" ) << QStringLiteral( "vector_layers" )
         << QStringLiteral( "vector_layers_auth" ) << QStringLiteral( "vector_layers_field_infos" ) << QStringLiteral( "vector_layers_statistics" )
         << QStringLiteral( "views_geometry_columns_auth" ) << QStringLiteral( "views_geometry_columns_field_infos" )
         << QStringLiteral( "views_geometry_columns_statistics" ) << QStringLiteral( "virts_geometry_columns_auth" )
         << QStringLiteral( "virts_geometry_columns_field_infos" ) << QStringLiteral( "virts_geometry_columns_statistics" )
         << QStringLiteral( "virts_layer_statistics" ) << QStringLiteral( "views_layer_statistics" )
         << QStringLiteral( "ElementaryGeometries" );
}

QString QgsSqlite3Mprintf( const char *format, ... )
{
  va_list ap;
  va_start( ap, format );
  char *c_str = sqlite3_vmprintf( format, ap );
  va_end( ap );
  QString res( QString::fromUtf8( c_str ) );
  sqlite3_free( c_str );
  return res;
}
