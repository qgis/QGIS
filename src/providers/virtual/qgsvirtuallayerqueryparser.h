/***************************************************************************
   qgsvirtuallayerqueryparser.h : SQL query parser utility functions
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

#ifndef QGSVIRTUALLAYER_QUERY_PARSER_H
#define QGSVIRTUALLAYER_QUERY_PARSER_H

#include <qgis.h>
#include <qgswkbtypes.h>
#include <qgsvectorlayer.h>

namespace QgsVirtualLayerQueryParser
{

  //!
  //! Return the list of tables referenced in the SQL query
  QStringList referencedTables( const QString& q );

  /**
   * Type used to define a column
   *
   * It can hold a name and a type.
   * The type can be a 'scalar' type (int, double, string) or a geometry type (WKB) and an SRID
   */
  class ColumnDef
  {
    public:
      ColumnDef()
          : mType( QVariant::Invalid )
          , mWkbType( QgsWKBTypes::Unknown )
          , mSrid( -1 )
      {}
      ColumnDef( const QString& name, QgsWKBTypes::Type aWkbType, long aSrid )
          : mName( name )
          , mType( QVariant::UserType )
          , mWkbType( aWkbType )
          , mSrid( aSrid )
      {}
      ColumnDef( const QString& name, QVariant::Type aType )
          : mName( name )
          , mType( aType )
          , mWkbType( QgsWKBTypes::NoGeometry )
          , mSrid( -1 )
      {}

      QString name() const { return mName; }
      void setName( QString name ) { mName = name; }

      bool isGeometry() const { return mType == QVariant::UserType; }
      void setGeometry( QgsWKBTypes::Type wkbType ) { mType = QVariant::UserType; mWkbType = wkbType; }
      long srid() const { return mSrid; }
      void setSrid( long srid ) { mSrid = srid; }

      void setScalarType( QVariant::Type t ) { mType = t; mWkbType = QgsWKBTypes::NoGeometry; }
      QVariant::Type scalarType() const { return mType; }
      QgsWKBTypes::Type wkbType() const { return mWkbType; }

    private:
      QString mName;
      QVariant::Type mType;
      QgsWKBTypes::Type mWkbType;
      long mSrid;
  };

  //!
  //! Type used by the parser to type a query. It is slightly different from a QgsVirtualLayerDefinition since more than one geometry column can be represented
  typedef QList<ColumnDef> TableDef;

  //! Get the column names and types that can be deduced from the query, using SQLite introspection
  //! Special comments can also be used in the query to type columns
  //! Comments should be set after the name of the column and are introduced by "/*:"
  //! For instance 'SELECT t+1 /*:int*/ FROM table' will type the column 't' as integer
  //! A geometry column can also be set by specifying a type and an SRID
  //! For instance 'SELECT t, GeomFromText('POINT(0 0)',4326) as geom /*:point:4326*/
  TableDef columnDefinitionsFromQuery( sqlite3* db, const QString& query );

  //! Get the column types of a virtual table
  TableDef tableDefinitionFromVirtualTable( sqlite3* db, const QString& tableName );

}

#endif
