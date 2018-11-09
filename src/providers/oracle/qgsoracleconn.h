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
#include <QVariant>
#include <QDateTime>

#include "qgis.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"

#include <QSqlDatabase>
#include <QSqlQuery>

class QgsField;

// Oracle layer properties
struct QgsOracleLayerProperty
{
  QList<QgsWkbTypes::Type> types;
  QList<int>           srids;
  QString              ownerName;
  QString              tableName;
  QString              geometryColName;
  bool                 isView = false;
  QStringList          pkCols;
  QString              sql;

  QgsOracleLayerProperty() = default;

  int size() const { Q_ASSERT( types.size() == srids.size() ); return types.size(); }

  bool operator==( const QgsOracleLayerProperty &other )
  {
    return types == other.types && srids == other.srids && ownerName == other.ownerName &&
           tableName == other.tableName && geometryColName == other.geometryColName &&
           isView == other.isView && pkCols == other.pkCols && sql == other.sql;
  }

  QgsOracleLayerProperty at( int i ) const
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

#ifdef QGISDEBUG
  QString toString() const
  {
    QString typeString;
    Q_FOREACH ( QgsWkbTypes::Type type, types )
    {
      if ( !typeString.isEmpty() )
        typeString += "|";
      typeString += QString::number( type );
    }
    QString sridString;
    Q_FOREACH ( int srid, srids )
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

class QgsOracleConn : public QObject
{
    Q_OBJECT
  public:
    static QgsOracleConn *connectDb( const QgsDataSourceUri &uri );
    void disconnect();

    /**
     * Double quote a Oracle identifier for placement in a SQL string.
     */
    static QString quotedIdentifier( QString ident );

    /**
     * Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant &value, QVariant::Type type = QVariant::Invalid );

    /**
     * Gets the list of supported layers.
     *
     * If \a limitToSchema is specified, than only layers from the matching schema will be
     * returned.
     *
     */
    bool supportedLayers( QVector<QgsOracleLayerProperty> &layers,
                          const QString &limitToSchema,
                          bool geometryTablesOnly,
                          bool userTablesOnly = true,
                          bool allowGeometrylessTables = false );

    void retrieveLayerTypes( QgsOracleLayerProperty &layerProperty, bool useEstimatedMetadata, bool onlyExistingTypes );

    /**
     * Gets information about the spatial tables.
     *
     * If \a schema is specified, only tables from this schema will be retrieved.
     */
    bool tableInfo( const QString &schema, bool geometryTablesOnly, bool userTablesOnly, bool allowGeometrylessTables );

    //! Gets primary key candidates (all int4 columns)
    QStringList pkCandidates( const QString &ownerName, const QString &viewName );

    static QString fieldExpression( const QgsField &fld );

    QString connInfo();

    QString currentUser();

    bool hasSpatial();

    static const int sGeomTypeSelectLimit;

    static QString displayStringForWkbType( QgsWkbTypes::Type wkbType );
    static QgsWkbTypes::Type wkbTypeFromDatabase( int gtype );

    static QString databaseTypeFilter( const QString &alias, QString geomCol, QgsWkbTypes::Type wkbType );

    static QgsWkbTypes::Type wkbTypeFromGeomType( QgsWkbTypes::GeometryType geomType );

    static QStringList connectionList();
    static QString selectedConnection();
    static void setSelectedConnection( const QString &connName );
    static QgsDataSourceUri connUri( const QString &connName );
    static bool userTablesOnly( const QString &connName );
    static QString restrictToSchema( const QString &connName );
    static bool geometryColumnsOnly( const QString &connName );
    static bool allowGeometrylessTables( const QString &connName );
    static bool estimatedMetadata( const QString &connName );
    static bool onlyExistingTypes( const QString &connName );
    static void deleteConnection( const QString &connName );
    static QString databaseName( const QString &database, const QString &host, const QString &port );
    static QString toPoolName( const QgsDataSourceUri &uri );

    operator QSqlDatabase() { return mDatabase; }

  private:
    explicit QgsOracleConn( QgsDataSourceUri uri );
    ~QgsOracleConn() override;

    bool exec( QSqlQuery &qry, const QString &sql, const QVariantList &params );

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
    static QMap<QString, QDateTime> sBrokenConnections;
};

#endif
