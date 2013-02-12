/***************************************************************************
  qgsoracleconn.h  -  connection class to Oracle
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLECONN_H
#define QGSORACLECONN_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QThread>

#include "qgis.h"
#include "qgsdatasourceuri.h"

#include <QSqlDatabase>
#include <QSqlQuery>

class QgsField;

// Oracle layer properties
struct QgsOracleLayerProperty
{
  QList<QGis::WkbType> types;
  QList<int>           srids;
  QString              ownerName;
  QString              tableName;
  QString              geometryColName;
  bool                 isView;
  QStringList          pkCols;
  QString              sql;

  int size() { Q_ASSERT( types.size() == srids.size() ); return types.size(); }

  QgsOracleLayerProperty at( int i )
  {
    QgsOracleLayerProperty property;

    Q_ASSERT( i >= 0 && i < size() );

    property.types << types.at( i );
    property.srids << srids.at( i );
    property.ownerName       = ownerName;
    property.tableName       = tableName;
    property.geometryColName = geometryColName;
    property.isView          = isView;
    property.pkCols          = pkCols;
    property.sql             = sql;

    return property;
  }

#if QGISDEBUG
  QString toString()
  {
    QString typeString;
    foreach ( QGis::WkbType type, types )
    {
      if ( !typeString.isEmpty() )
        typeString += "|";
      typeString += QString::number( type );
    }
    QString sridString;
    foreach ( int srid, srids )
    {
      if ( !sridString.isEmpty() )
        sridString += "|";
      sridString += QString::number( srid );
    }

    return QString( "%1.%2.%3 type=%4 srid=%5 view=%6%7 sql=%8" )
           .arg( ownerName )
           .arg( tableName )
           .arg( geometryColName )
           .arg( typeString )
           .arg( sridString )
           .arg( isView ? "yes" : "no" )
           .arg( isView ? QString( " pk=%1" ).arg( pkCols.join( "|" ) ) : "" )
           .arg( sql );
  }
#endif
};

class QgsOracleConn : public QThread
{
    Q_OBJECT;

  public:
    static QgsOracleConn *connectDb( QgsDataSourceURI uri );
    void disconnect();

    /** Double quote a Oracle identifier for placement in a SQL string.
     */
    static QString quotedIdentifier( QString ident );

    /** Quote a value for placement in a SQL string.
     */
    static QString quotedValue( QVariant value );

    //! Get the list of usable to check
    bool supportedLayers( QVector<QgsOracleLayerProperty> &layers,
                          bool geometryTablesOnly,
                          bool userTablesOnly = true,
                          bool allowGeometrylessTables = false );

    void retrieveLayerTypes( QgsOracleLayerProperty &layerProperty, bool useEstimatedMetadata );

    /** Gets information about the spatial tables */
    bool tableInfo( bool geometryTablesOnly, bool userTablesOnly, bool allowGeometrylessTables );

    /** get primary key candidates (all int4 columns) */
    QStringList pkCandidates( QString ownerName, QString viewName );

    QString fieldExpression( const QgsField &fld );

    QString connInfo();

    QString currentUser();

    bool hasSpatial();

    static const int sGeomTypeSelectLimit;

    static QString displayStringForWkbType( QGis::WkbType wkbType );
    static QGis::WkbType wkbTypeFromDatabase( int gtype );

    static QString databaseTypeFilter( QString alias, QString geomCol, QGis::WkbType wkbType );

    static QGis::WkbType wkbTypeFromGeomType( QGis::GeometryType geomType );

    static QStringList connectionList();
    static QString selectedConnection();
    static void setSelectedConnection( QString theConnName );
    static QgsDataSourceURI connUri( QString theConnName );
    static bool userTablesOnly( QString theConnName );
    static bool geometryColumnsOnly( QString theConnName );
    static bool allowGeometrylessTables( QString theConnName );
    static bool estimatedMetadata( QString theConnName );
    static void deleteConnection( QString theConnName );
    static QString databaseName( QString database, QString host, QString port );

    operator QSqlDatabase() { return mDatabase; }

  private:
    QgsOracleConn( QgsDataSourceURI uri );
    ~QgsOracleConn();

    bool exec( QSqlQuery &qry, QString sql );

    //! reference count
    int mRef;

    QString mCurrentUser;

    //! has spatial
    int mHasSpatial;

    QSqlDatabase mDatabase;
    QSqlQuery mQuery;

    //! List of the supported layers
    QVector<QgsOracleLayerProperty> mLayersSupported;

    static QMap<QString, QgsOracleConn *> sConnections;
    static int snConnections;
};

#endif
