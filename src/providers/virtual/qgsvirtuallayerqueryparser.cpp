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

#include <QRegExp>
#include <QtDebug>

namespace QgsVirtualLayerQueryParser
{

  QStringList referencedTables( const QString& query )
  {
    QStringList tables;

    //
    // open an empty in-memory sqlite database and execute the query
    // sqlite will return an error for each missing table
    // this way we know the list of tables referenced by the query
    QgsScopedSqlite db( ":memory:", /*withExtension=*/ false );

    const QString noSuchError = "no such table: ";

    while ( true )
    {
      char *errMsg = nullptr;
      int r = sqlite3_exec( db.get(), query.toLocal8Bit().constData(), nullptr, nullptr, &errMsg );
      QString err = errMsg;
      if ( r && err.startsWith( noSuchError ) )
      {
        QString tableName = err.mid( noSuchError.size() );
        tables << tableName;

        // create a dummy table to skip this error
        QString createStr = QString( "CREATE TABLE \"%1\" (id int)" ).arg( tableName.replace( "\"", "\"\"" ) );
        ( void )sqlite3_exec( db.get(), createStr.toLocal8Bit().constData(), nullptr, NULL, NULL );
      }
      else
      {
        // no error, or another error
        break;
      }
    }
    return tables;
  }

  QMap<QString, ColumnDef> columnCommentDefinitions( const QString& query )
  {
    QMap<QString, ColumnDef> defs;

    // look for special comments in SQL
    // a column name followed by /*:type*/
    QRegExp rx( "([a-zA-Z_\x80-\xFF][a-zA-Z0-9_\x80-\xFF]*)\\s*/\\*:(int|real|text|((?:multi)?(?:point|linestring|polygon)):(\\d+))\\s*\\*/", Qt::CaseInsensitive );
    int pos = 0;

    while (( pos = rx.indexIn( query, pos ) ) != -1 )
    {
      QString column = rx.cap( 1 );
      QString type = rx.cap( 2 );
      ColumnDef def;
      def.setName( column );
      if ( type == "int" )
        def.setScalarType( QVariant::Int );
      else if ( type == "real" )
        def.setScalarType( QVariant::Double );
      else if ( type == "text" )
        def.setScalarType( QVariant::String );
      else
      {
        // there should be 2 more captures
        def.setGeometry( QgsWKBTypes::parseType( rx.cap( 3 ) ) );
        def.setSrid( static_cast<QgsWKBTypes::Type>( rx.cap( 4 ).toLong() ) );
      }
      defs[column] = def;

      pos += rx.matchedLength();
    }
    return defs;
  }

// set the type of the column type, given its text representation
  void setColumnDefType( const QString& columnType, ColumnDef& d )
  {
    // geometry type
    QRegExp geometryTypeRx( "\\(([0-9]+),([0-9]+)\\)" );

    // see qgsvirtuallayersqlitemodule for possible declared types
    // the type returned by PRAGMA table_info will be either
    // the type declared by one of the virtual tables
    // or null
    if ( columnType == "int" )
      d.setScalarType( QVariant::Int );
    else if ( columnType == "real" )
      d.setScalarType( QVariant::Double );
    else if ( columnType == "text" )
      d.setScalarType( QVariant::String );
    else if ( columnType.startsWith( "geometry" ) )
    {
      // parse the geometry type and srid
      // geometry(type,srid)
      int pos = geometryTypeRx.indexIn( columnType, 0 );
      if ( pos != -1 )
      {
        QgsWKBTypes::Type type = static_cast<QgsWKBTypes::Type>( geometryTypeRx.cap( 1 ).toInt() );
        long srid = geometryTypeRx.cap( 2 ).toLong();
        d.setGeometry( type );
        d.setSrid( srid );
      }
    }
  }

  ColumnDef geometryDefinitionFromVirtualTable( sqlite3* db, const QString& tableName )
  {
    ColumnDef d;
    Sqlite::Query q( db, QString( "PRAGMA table_info(%1)" ).arg( tableName ) );
    while ( q.step() == SQLITE_ROW )
    {
      QString columnName = q.columnText( 1 );
      QString columnType = q.columnText( 2 );
      if ( ! columnType.startsWith( "geometry" ) )
        continue;

      d.setName( columnName );

      setColumnDefType( columnType, d );

      break;
    }
    return d;
  }

  TableDef columnDefinitionsFromQuery( sqlite3* db, const QString& query )
  {
    // get column types defined by comments
    QMap<QString, ColumnDef> definedColumns = columnCommentDefinitions( query );

    // create a view to detect column names and types, using PRAGMA table_info
    QString viewStr = "CREATE TEMP VIEW _tview AS " + query;
    Sqlite::Query::exec( db, viewStr );

    QStringList columns;
    QVector<int> undefinedColumns;
    TableDef tableDef;
    {
      Sqlite::Query q( db, "PRAGMA table_info(_tview)" );
      int columnNumber = 0;
      while ( q.step() == SQLITE_ROW )
      {
        QString columnName = q.columnText( 1 );

        columns << columnName;

        QString columnType = q.columnText( 2 );

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
      QString qs = "SELECT ";
      for ( int i = 0; i < undefinedColumns.size(); i++ )
      {
        qs += "\"" + columns[undefinedColumns[i]] + "\"";
        if ( i != undefinedColumns.size() - 1 )
          qs += ", ";
      }
      qs += " FROM _tview LIMIT 1";
      qWarning() << qs;

      Sqlite::Query q( db, qs );
      if ( q.step() == SQLITE_ROW )
      {
        for ( int i = 0; i < undefinedColumns.size(); i++ )
        {
          int colIdx = undefinedColumns[i];
          int type = q.columnType( i );
          switch ( type )
          {
            case SQLITE_INTEGER:
              tableDef[colIdx].setScalarType( QVariant::Int );
              break;
            case SQLITE_FLOAT:
              tableDef[colIdx].setScalarType( QVariant::Double );
              break;
            case SQLITE_BLOB:
            {
              // might be a geometry, parse the type
              QByteArray ba( q.columnBlob( i ) );
              QPair<QgsWKBTypes::Type, long> p( spatialiteBlobGeometryType( ba.constData(), ba.size() ) );
              if ( p.first != QgsWKBTypes::NoGeometry )
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

  TableDef tableDefinitionFromVirtualTable( sqlite3* db, const QString& tableName )
  {
    TableDef td;
    Sqlite::Query q( db, QString( "PRAGMA table_info(%1)" ).arg( tableName ) );
    while ( q.step() == SQLITE_ROW )
    {
      ColumnDef d;
      QString columnName = q.columnText( 1 );
      QString columnType = q.columnText( 2 );

      d.setName( columnName );
      setColumnDefType( columnType, d );

      td << d;
    }
    return td;
  }

} // namespace
