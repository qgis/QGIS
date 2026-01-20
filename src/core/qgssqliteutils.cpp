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

#include <cstdarg>
#include <sqlite3.h>

#include "qgsvariantutils.h"

#include <QSet>
#include <QVariant>

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
  char *zErrMsg = nullptr;
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

  // this is MESSY -- this function does not have ownership of connection, so this is a HACK:
  // we intentionally .release() at the end of the function accordingly -- be careful if adding additional return paths!!
  dsPtr.reset( connection );

  const QString quotedTableName { QgsSqliteUtils::quotedValue( tableName ) };

  int resultCode = 0;
  sqlite3_statement_unique_ptr stmt { dsPtr.prepare( u"SELECT seq FROM sqlite_sequence WHERE name = %1"_s
                                      .arg( quotedTableName ), resultCode )};
  if ( resultCode == SQLITE_OK && stmt.step() )
  {
    result = sqlite3_column_int64( stmt.get(), 0 );
    // Try to create the sequence in case this is an empty layer
    if ( sqlite3_column_count( stmt.get() ) == 0 )
    {
      dsPtr.exec( u"INSERT INTO sqlite_sequence (name, seq) VALUES (%1, 1)"_s.arg( quotedTableName ), errorMessage );
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
      if ( dsPtr.exec( u"UPDATE sqlite_sequence SET seq = %1 WHERE name = %2"_s
                       .arg( QString::number( ++result ), quotedTableName ),
                       errorMessage ) != SQLITE_OK )
      {
        errorMessage = QObject::tr( "Error retrieving default value for %1" ).arg( tableName );
        result = -1;
      }
    }
  }

  // INTENTIONAL HACK -- see above
  ( void )dsPtr.release();
  return result;
}

QString QgsSqliteUtils::quotedString( const QString &value )
{
  if ( value.isNull() )
    return u"NULL"_s;

  QString v = value;
  v.replace( '\'', "''"_L1 );
  return v.prepend( '\'' ).append( '\'' );
}

QString QgsSqliteUtils::quotedIdentifier( const QString &identifier )
{
  QString id( identifier );
  id.replace( '\"', "\"\""_L1 );
  return id.prepend( '\"' ).append( '\"' );
}

QString QgsSqliteUtils::quotedValue( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
    return u"NULL"_s;

  switch ( value.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::Double:
      return value.toString();

    case QMetaType::Type::Bool:
      //SQLite has no boolean literals
      return value.toBool() ? u"1"_s : u"0"_s;

    default:
    case QMetaType::Type::QString:
      QString v = value.toString();
      // https://www.sqlite.org/lang_expr.html :
      // """A string constant is formed by enclosing the string in single quotes (').
      // A single quote within the string can be encoded by putting two single quotes
      // in a row - as in Pascal. C-style escapes using the backslash character are not supported because they are not standard SQL. """
      return v.replace( '\'', "''"_L1 ).prepend( '\'' ).append( '\'' );
  }
}

QStringList QgsSqliteUtils::systemTables()
{
  return QStringList() << u"ElementaryGeometries"_s << u"SpatialIndex"_s
         << u"geom_cols_ref_sys"_s << u"geometry_columns"_s
         << u"geometry_columns_auth"_s << u"geometry_columns_field_infos"_s
         << u"geometry_columns_statistics"_s << u"geometry_columns_time"_s
         << u"layer_params"_s << u"layer_statistics"_s << u"layer_sub_classes"_s
         << u"layer_table_layout"_s << u"pattern_bitmaps"_s << u"project_defs"_s
         << u"raster_pyramids"_s << u"spatial_ref_sys"_s << u"spatial_ref_sys_all"_s
         << u"spatial_ref_sys_aux"_s << u"spatialite_history"_s << u"sql_statements_log"_s
         << u"sqlite_sequence"_s << u"sqlite_stat1"_s << u"sqlite_stat2"_s
         << u"symbol_bitmaps"_s << u"tableprefix_metadata"_s << u"tableprefix_rasters"_s
         << u"vector_layers"_s << u"vector_layers_auth"_s << u"vector_layers_field_infos"_s
         << u"vector_layers_statistics"_s << u"views_geometry_columns"_s
         << u"views_geometry_columns_auth"_s << u"views_geometry_columns_field_infos"_s
         << u"views_geometry_columns_statistics"_s << u"views_layer_statistics"_s
         << u"virts_geometry_columns"_s << u"virts_geometry_columns_auth"_s
         << u"virts_geometry_columns_field_infos"_s << u"virts_geometry_columns_statistics"_s
         << u"virts_layer_statistics"_s
         // Additional tables to be hidden
         << u"all_buckets_objects"_s << u"byfoot"_s << u"byfoot_data"_s
         << u"data_licenses"_s << u"ISO_metadata"_s << u"ISO_metadata_reference"_s
         << u"ISO_metadata_view"_s << u"KNN2"_s << u"KNN"_s
         << u"networks"_s << u"raster_coverages"_s << u"raster_coverages_keyword"_s
         << u"raster_coverages_ref_sys"_s << u"raster_coverages_srid"_s
         << u"rl2map_configurations"_s << u"rl2map_configurations_view"_s
         // SE_ (Styled Elements)
         << u"SE_external_graphics"_s << u"SE_external_graphics_view"_s
         << u"SE_fonts"_s << u"SE_fonts_view"_s << u"SE_group_styles"_s
         << u"SE_group_styles_view"_s << u"SE_raster_styled_layers"_s
         << u"SE_raster_styled_layers_view"_s << u"SE_raster_styles"_s
         << u"SE_raster_styles_view"_s << u"SE_styled_group_refs"_s
         << u"SE_styled_group_styles"_s << u"SE_styled_groups"_s
         << u"SE_styled_groups_view"_s << u"SE_vector_styled_layers"_s
         << u"SE_vector_styled_layers_view"_s << u"SE_vector_styles"_s
         << u"SE_vector_styles_view"_s
         << u"sqlite_stat3"_s << u"stored_procedures"_s << u"stored_variables"_s
         << u"topologies"_s << u"vector_coverages"_s << u"vector_coverages_keyword"_s
         << u"vector_coverages_ref_sys"_s << u"vector_coverages_srid"_s
         // WMS
         << u"wms_getcapabilities"_s << u"wms_getmap"_s << u"wms_ref_sys"_s
         << u"wms_settings"_s;
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
