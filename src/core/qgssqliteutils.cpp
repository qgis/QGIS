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
#include <QSet>

// Temporary solution until GDAL Unique support is available
#include <regex>
#include <sstream>
#include <algorithm>
// end temporary

void QgsSqlite3Closer::operator()( sqlite3 *database ) const
{
  sqlite3_close_v2( database );
}

void QgsSqlite3StatementFinalizer::operator()( sqlite3_stmt *statement ) const
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

QByteArray sqlite3_statement_unique_ptr::columnAsBlob( int column ) const
{
  const void *blob = sqlite3_column_blob( get(), column );
  const int size = sqlite3_column_bytes( get(), column );
  return QByteArray( reinterpret_cast<const char *>( blob ), size );
}

qlonglong sqlite3_statement_unique_ptr::columnAsInt64( int column ) const
{
  return sqlite3_column_int64( get(), column );
}

int sqlite3_database_unique_ptr::open( const QString &path )
{
  sqlite3 *database = nullptr;
  const int result = sqlite3_open( path.toUtf8(), &database );
  reset( database );
  return result;
}

int sqlite3_database_unique_ptr::open_v2( const QString &path, int flags, const char *zVfs )
{
  sqlite3 *database = nullptr;
  const int result = sqlite3_open_v2( path.toUtf8(), &database, flags, zVfs );
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

  const int ret = sqlite3_exec( get(), sql.toUtf8(), nullptr, nullptr, &errMsg );

  if ( errMsg )
  {
    errorMessage = QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
  }

  return ret;
}

QSet<QString> QgsSqliteUtils::uniqueFields( sqlite3 *connection, const QString &tableName, QString &errorMessage )
{
  QSet<QString> uniqueFieldsResults;
  char *zErrMsg = 0;
  std::vector<std::string> rows;
  const QByteArray tableNameUtf8 = tableName.toUtf8();
  QString sql = qgs_sqlite3_mprintf( "select sql from sqlite_master "
                                     "where type='table' and name='%q'", tableNameUtf8.constData() );
  auto cb = [ ](
              void *data /* Data provided in the 4th argument of sqlite3_exec() */,
              int /* The number of columns in row */,
              char **argv /* An array of strings representing fields in the row */,
              char ** /* An array of strings representing column names */ ) -> int
  {
    static_cast<std::vector<std::string>*>( data )->push_back( argv[0] );
    return 0;
  };

  int rc = sqlite3_exec( connection, sql.toUtf8(), cb, ( void * )&rows, &zErrMsg );
  if ( rc != SQLITE_OK )
  {
    errorMessage = zErrMsg;
    sqlite3_free( zErrMsg );
    return uniqueFieldsResults;
  }

  // Match identifiers with " or ` or no delimiter (and no spaces).
  std::smatch uniqueFieldMatch;
  static const std::regex sFieldIdentifierRe { R"raw(\s*(["`]([^"`]+)["`])|(([^\s]+)\s).*)raw" };
  for ( auto tableDefinition : rows )
  {
    tableDefinition = tableDefinition.substr( tableDefinition.find( '(' ), tableDefinition.rfind( ')' ) );
    std::stringstream tableDefinitionStream { tableDefinition };
    while ( tableDefinitionStream.good() )
    {
      std::string fieldStr;
      std::getline( tableDefinitionStream, fieldStr, ',' );
      std::string upperCaseFieldStr { fieldStr };
      std::transform( upperCaseFieldStr.begin(), upperCaseFieldStr.end(), upperCaseFieldStr.begin(), ::toupper );
      if ( upperCaseFieldStr.find( "UNIQUE" ) != std::string::npos )
      {
        if ( std::regex_search( fieldStr, uniqueFieldMatch, sFieldIdentifierRe ) )
        {
          const std::string quoted { uniqueFieldMatch.str( 2 ) };
          uniqueFieldsResults.insert( QString::fromStdString( quoted.length() ? quoted :  uniqueFieldMatch.str( 4 ) ) );
        }
      }
    }
  }
  rows.clear();

  // Search indexes:
  sql = qgs_sqlite3_mprintf( "SELECT sql FROM sqlite_master WHERE type='index' AND"
                             " tbl_name='%q' AND sql LIKE 'CREATE UNIQUE INDEX%%'", tableNameUtf8.constData() );
  rc = sqlite3_exec( connection, sql.toUtf8(), cb, ( void * )&rows, &zErrMsg );
  if ( rc != SQLITE_OK )
  {
    errorMessage = zErrMsg;
    sqlite3_free( zErrMsg );
    return uniqueFieldsResults;
  }

  if ( rows.size() > 0 )
  {
    static const std::regex sFieldIndexIdentifierRe { R"raw(\(\s*[`"]?([^",`\)]+)["`]?\s*\))raw" };
    for ( auto indexDefinition : rows )
    {
      std::string upperCaseIndexDefinition { indexDefinition };
      std::transform( upperCaseIndexDefinition.begin(), upperCaseIndexDefinition.end(), upperCaseIndexDefinition.begin(), ::toupper );
      if ( upperCaseIndexDefinition.find( "UNIQUE" ) != std::string::npos )
      {
        indexDefinition = indexDefinition.substr( indexDefinition.find( '(' ), indexDefinition.rfind( ')' ) );
        if ( std::regex_search( indexDefinition, uniqueFieldMatch, sFieldIndexIdentifierRe ) )
        {
          uniqueFieldsResults.insert( QString::fromStdString( uniqueFieldMatch.str( 1 ) ) );
        }
      }
    }
  }
  return uniqueFieldsResults;
}

long long QgsSqliteUtils::nextSequenceValue( sqlite3 *connection, const QString &tableName, QString errorMessage )
{
  long long result { -1 };
  sqlite3_database_unique_ptr dsPtr;
  dsPtr.reset( connection );
  const QString quotedTableName { QgsSqliteUtils::quotedValue( tableName ) };

  int resultCode;
  sqlite3_statement_unique_ptr stmt { dsPtr.prepare( QStringLiteral( "SELECT seq FROM sqlite_sequence WHERE name = %1" )
                                      .arg( quotedTableName ), resultCode )};
  if ( resultCode == SQLITE_OK )
  {
    stmt.step();
    result = sqlite3_column_int64( stmt.get(), 0 );
    // Try to create the sequence in case this is an empty layer
    if ( sqlite3_column_count( stmt.get() ) == 0 )
    {
      dsPtr.exec( QStringLiteral( "INSERT INTO sqlite_sequence (name, seq) VALUES (%1, 1)" ).arg( quotedTableName ), errorMessage );
      if ( errorMessage.isEmpty() )
      {
        result = 1;
      }
      else
      {
        errorMessage = QObject::tr( "Error retrieving default value for %1" ).arg( tableName );
      }
    }
    else // increment
    {
      if ( dsPtr.exec( QStringLiteral( "UPDATE sqlite_sequence SET seq = %1 WHERE name = %2" )
                       .arg( QString::number( ++result ), quotedTableName ),
                       errorMessage ) != SQLITE_OK )
      {
        errorMessage = QObject::tr( "Error retrieving default value for %1" ).arg( tableName );
        result = -1;
      }
    }
  }

  dsPtr.release();
  return result;
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

QString qgs_sqlite3_mprintf( const char *format, ... )
{
  va_list ap;
  va_start( ap, format );
  char *c_str = sqlite3_vmprintf( format, ap );
  va_end( ap );
  QString res( QString::fromUtf8( c_str ) );
  sqlite3_free( c_str );
  return res;
}
