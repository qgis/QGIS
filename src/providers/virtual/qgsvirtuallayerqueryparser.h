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

#include "qgis.h"
#include "qgswkbtypes.h"

struct sqlite3;

namespace QgsVirtualLayerQueryParser
{

  /**
   * Returns the list of tables referenced in the SQL query
   */
  QStringList referencedTables( const QString &q );

  /**
   * Type used to define a column
   *
   * It can hold a name and a type.
   * The type can be a 'scalar' type (int, double, string) or a geometry type (WKB) and an SRID
   */
  class ColumnDef
  {
    public:
      ColumnDef() = default;
      ColumnDef( const QString &name, Qgis::WkbType aWkbType, long aSrid )
        : mName( name )
        , mType( QMetaType::Type::User )
        , mWkbType( aWkbType )
        , mSrid( aSrid )
      {}
      ColumnDef( const QString &name, QMetaType::Type aType )
        : mName( name )
        , mType( aType )
        , mWkbType( Qgis::WkbType::NoGeometry )
      {}

      QString name() const { return mName; }
      void setName( const QString &name ) { mName = name; }

      bool isGeometry() const { return mType == QMetaType::Type::User; }
      void setGeometry( Qgis::WkbType wkbType )
      {
        mType = QMetaType::Type::User;
        mWkbType = wkbType;
      }
      long srid() const { return mSrid; }
      void setSrid( long srid ) { mSrid = srid; }

      void setScalarType( QMetaType::Type t )
      {
        mType = t;
        mWkbType = Qgis::WkbType::NoGeometry;
      }
      QMetaType::Type scalarType() const { return mType; }
      Qgis::WkbType wkbType() const { return mWkbType; }

    private:
      QString mName;
      QMetaType::Type mType = QMetaType::Type::UnknownType;
      Qgis::WkbType mWkbType = Qgis::WkbType::Unknown;
      long mSrid = -1;
  };

  /**
   * Type used by the parser to type a query. It is slightly different from a QgsVirtualLayerDefinition since more than one geometry column can be represented
   */
  typedef QList<ColumnDef> TableDef;

  /**
   * Gets the column names and types that can be deduced from the query, using SQLite introspection
   * Special comments can also be used in the query to type columns
   * Comments should be set after the name of the column and are introduced by "/\htmlonly\endhtmlonly*:"
   * For instance 'SELECT t+1 /\htmlonly\endhtmlonly*:int*\htmlonly\endhtmlonly/ FROM table' will type the column 't' as integer
   * A geometry column can also be set by specifying a type and an SRID
   * For instance 'SELECT t, GeomFromText('POINT(0 0)',4326) as geom /\htmlonly\endhtmlonly*:point:4326*\htmlonly\endhtmlonly/
   */
  TableDef columnDefinitionsFromQuery( sqlite3 *db, const QString &query );

  //! Gets the column types of a virtual table
  TableDef tableDefinitionFromVirtualTable( sqlite3 *db, const QString &tableName );

} // namespace QgsVirtualLayerQueryParser

#endif
