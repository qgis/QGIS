/***************************************************************************
   qgsvirtuallayerqueryparser.cpp : SQL query parser utility functions
begin                : Jan 2016
copyright            : (C) 2016 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtuallayerqueryparser.h"
#include "qgsvirtuallayersqlitehelper.h"
#include "qgsvirtuallayerblob.h"
#include "qgslogger.h"

#include "sqlite3.h"

#include <QRegularExpression>
#include <QtDebug>

namespace QgsVirtualLayerQueryParser
{

  QStringList referencedTables( const QString &query )
  {
    QStringList tables;

    //
    // open an empty in-memory sqlite database and execute the query
    // sqlite will return an error for each missing table
    // this way we know the list of tables referenced by the query
    const QgsScopedSqlite db( QStringLiteral( ":memory:" ), /*withExtension=*/ false );

    const QString noSuchError = QStringLiteral( "no such table: " );

    while ( true )
    {
      char *errMsg = nullptr;
      const int r = sqlite3_exec( db.get(), query.toUtf8().constData(), nullptr, nullptr, &errMsg );
      QString err;
      if ( r != SQLITE_OK )
      {
        err = QString::fromUtf8( errMsg );
        sqlite3_free( errMsg );
      }
      if ( r && err.startsWith( noSuchError ) )
      {
        QString tableName = err.mid( noSuchError.size() );
        tables << tableName;

        // create a dummy table to skip this error
        const QString createStr = QStringLiteral( "CREATE TABLE \"%1\" (id int)" ).arg( tableName.replace( QLatin1String( "\"" ), QLatin1String( "\"\"" ) ) );
        const int createRes = sqlite3_exec( db.get(), createStr.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( createRes != SQLITE_OK )
        {
          err = QString::fromUtf8( errMsg );
          sqlite3_free( errMsg );
          QgsDebugMsg( QStringLiteral( "Could not create temporary table for virtual layer: %1" ).arg( err ) );
          break;
        }
      }
      else
      {
        // no error, or another error
        break;
      }
    }
    return tables;
  }

  QMap<QString, ColumnDef> columnCommentDefinitions( const QString &query )
  {
    QMap<QString, ColumnDef> defs;

    // look for special comments in SQL
    // a column name followed by /*:type*/
    const thread_local QRegularExpression rx( "([a-zA-Z_\x80-\xFF][a-zA-Z0-9_\x80-\xFF]*)\\s*/\\*:(int|real|text|((?:multi)?(?:point|linestring|polygon)):(\\d+))\\s*\\*/", QRegularExpression::CaseInsensitiveOption );
    int pos = 0;

    QRegularExpressionMatch match = rx.match( query, pos );
    while ( match.hasMatch() )
    {
      const QString column = match.captured( 1 );
      const QString type = match.captured( 2 );
      ColumnDef def;
      def.setName( column );
      if ( type == QLatin1String( "int" ) )
        def.setScalarType( QVariant::LongLong );
      else if ( type == QLatin1String( "real" ) )
        def.setScalarType( QVariant::Double );
      else if ( type == QLatin1String( "text" ) )
        def.setScalarType( QVariant::String );
      else
      {
        // there should be 2 more captures
        def.setGeometry( QgsWkbTypes::parseType( match.captured( 3 ) ) );
        def.setSrid( static_cast<QgsWkbTypes::Type>( match.captured( 4 ).toLong() ) );
      }
      defs[column] = def;

      pos += match.capturedLength();
      match = rx.match( query, pos );
    }
    return defs;
  }

// set the type of the column type, given its text representation
  void setColumnDefType( const QString &columnType, ColumnDef &d )
  {
    // geometry type
    const thread_local QRegularExpression geometryTypeRx( "\\(([0-9]+),([0-9]+)\\)" );

    // see qgsvirtuallayersqlitemodule for possible declared types
    // the type returned by PRAGMA table_info will be either
    // the type declared by one of the virtual tables
    // or null
    if ( columnType.compare( QLatin1String( "int" ), Qt::CaseInsensitive ) == 0 )
      d.setScalarType( QVariant::LongLong );
    else if ( columnType.compare( QLatin1String( "real" ), Qt::CaseInsensitive ) == 0 )
      d.setScalarType( QVariant::Double );
    else if ( columnType.compare( QLatin1String( "text" ), Qt::CaseInsensitive ) == 0 )
      d.setScalarType( QVariant::String );
    else if ( columnType.startsWith( QLatin1String( "geometry" ), Qt::CaseInsensitive ) )
    {
      // parse the geometry type and srid
      // geometry(type,srid)
      const QRegularExpressionMatch match = geometryTypeRx.match( columnType );
      if ( match.hasMatch() )
      {
        const QgsWkbTypes::Type type = static_cast<QgsWkbTypes::Type>( match.captured( 1 ).toLong() );
        const long srid = match.captured( 2 ).toLong();
        d.setGeometry( type );
        d.setSrid( srid );
      }
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Unknown column type %1" ).arg( columnType ) );
    }
  }

  ColumnDef geometryDefinitionFromVirtualTable( sqlite3 *db, const QString &tableName )
  {
    ColumnDef d;
    Sqlite::Query q( db, QStringLiteral( "PRAGMA table_info(%1)" ).arg( tableName ) );
    while ( q.step() == SQLITE_ROW )
    {
      const QString columnName = q.columnText( 1 );
      const QString columnType = q.columnText( 2 );
      if ( ! columnType.startsWith( QLatin1String( "geometry" ) ) )
        continue;

      d.setName( columnName );

      setColumnDefType( columnType, d );

      break;
    }
    return d;
  }

  TableDef columnDefinitionsFromQuery( sqlite3 *db, const QString &query )
  {
    // get column types defined by comments
    QMap<QString, ColumnDef> definedColumns = columnCommentDefinitions( query );

    // create a view to detect column names and types, using PRAGMA table_info
    const QString viewStr = "CREATE TEMP VIEW _tview AS " + query;
    Sqlite::Query::exec( db, viewStr );

    QStringList columns;
    QVector<int> undefinedColumns;
    TableDef tableDef;
    {
      Sqlite::Query q( db, QStringLiteral( "PRAGMA table_info(_tview)" ) );
      int columnNumber = 0;
      while ( q.step() == SQLITE_ROW )
      {
        const QString columnName = q.columnText( 1 );

        columns << columnName;

        const QString columnType = q.columnText( 2 );

        // column type defined by comments
        if ( definedColumns.contains( columnName ) )
        {
          tableDef << definedColumns[columnName];
        }
        else
        {
          ColumnDef d;
          d.setName( columnName );

          setColumnDefType( columnType, d );

          if ( d.scalarType() == QVariant::Invalid )
          {
            // else no type is defined
            undefinedColumns << columnNumber;
          }

          tableDef << d;
        }

        columnNumber++;
      }
    }

    if ( undefinedColumns.size() == 0 )
      return tableDef;

    // get the first row to introspect types
    {
      QString qs = QStringLiteral( "SELECT " );
      for ( int i = 0; i < undefinedColumns.size(); i++ )
      {
        qs += "\"" + columns[undefinedColumns[i]] + "\"";
        if ( i != undefinedColumns.size() - 1 )
          qs += QLatin1String( ", " );
      }
      qs += QLatin1String( " FROM _tview LIMIT 1" );

      Sqlite::Query q( db, qs );
      if ( q.step() == SQLITE_ROW )
      {
        for ( int i = 0; i < undefinedColumns.size(); i++ )
        {
          const int colIdx = undefinedColumns[i];
          const int type = q.columnType( i );
          switch ( type )
          {
            case SQLITE_INTEGER:
              tableDef[colIdx].setScalarType( QVariant::LongLong );
              break;
            case SQLITE_FLOAT:
              tableDef[colIdx].setScalarType( QVariant::Double );
              break;
            case SQLITE_BLOB:
            {
              // might be a geometry, parse the type
              const QByteArray ba( q.columnBlob( i ) );
              const QPair<QgsWkbTypes::Type, long> p( spatialiteBlobGeometryType( ba.constData(), ba.size() ) );
              if ( p.first != QgsWkbTypes::NoGeometry )
              {
                tableDef[colIdx].setGeometry( p.first );
                tableDef[colIdx].setSrid( p.second );
              }
              else
              {
                // interpret it as a string
                tableDef[colIdx].setScalarType( QVariant::String );
              }
            }
            break;
            case SQLITE_TEXT:
            default:
              tableDef[colIdx].setScalarType( QVariant::String );
              break;
          };
        }
      }
    }
    return tableDef;
  }

  TableDef tableDefinitionFromVirtualTable( sqlite3 *db, const QString &tableName )
  {
    TableDef td;
    Sqlite::Query q( db, QStringLiteral( "PRAGMA table_info(%1)" ).arg( tableName ) );
    while ( q.step() == SQLITE_ROW )
    {
      ColumnDef d;
      const QString columnName = q.columnText( 1 );
      const QString columnType = q.columnText( 2 );

      d.setName( columnName );
      setColumnDefType( columnType, d );

      td << d;
    }
    return td;
  }

} // namespace
