/***************************************************************************
  qgspostgresconn.h  -  connection class to PostgreSQL/PostGIS
                             -------------------
    begin                : 2011/01/28
    copyright            : (C) 2011 by Juergen E. Fischer
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

#ifndef QGSPOSTGRESCONN_H
#define QGSPOSTGRESCONN_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>

#include "qgis.h"
#include "qgsdatasourceuri.h"

extern "C"
{
#include <libpq-fe.h>
}

class QgsField;

enum QgsPostgresGeometryColumnType
{
  sctNone,
  sctGeometry,
  sctGeography,
  sctTopoGeometry
};

/** Layer Property structure */
// TODO: Fill to Postgres/PostGIS specifications
struct QgsPostgresLayerProperty
{
  // Postgres/PostGIS layer properties
  QList<QGis::WkbType>          types;
  QString                       schemaName;
  QString                       tableName;
  QString                       geometryColName;
  QgsPostgresGeometryColumnType geometryColType;
  QStringList                   pkCols;
  QList<int>                    srids;
  QString                       sql;

  int size() const { Q_ASSERT( types.size() == srids.size() ); return types.size(); }

  QgsPostgresLayerProperty at( int i ) const
  {
    QgsPostgresLayerProperty property;

    Q_ASSERT( i >= 0 && i < size() );

    property.types << types[ i ];
    property.srids << srids[ i ];
    property.schemaName      = schemaName;
    property.tableName       = tableName;
    property.geometryColName = geometryColName;
    property.geometryColType = geometryColType;
    property.pkCols          = pkCols;
    property.sql             = sql;

    return property;
  }

#if QGISDEBUG
  QString toString() const
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

    return QString( "%1.%2.%3 type=%4 srid=%5 pkCols=%6 sql=%7" )
           .arg( schemaName )
           .arg( tableName )
           .arg( geometryColName )
           .arg( typeString )
           .arg( sridString )
           .arg( pkCols.join( "|" ) )
           .arg( sql );
  }
#endif
};

class QgsPostgresResult
{
  public:
    QgsPostgresResult( PGresult *theRes = 0 ) : mRes( theRes ) {}
    ~QgsPostgresResult();

    QgsPostgresResult &operator=( PGresult *theRes );
    QgsPostgresResult &operator=( const QgsPostgresResult &src );

    ExecStatusType PQresultStatus();
    QString PQresultErrorMessage();

    int PQntuples();
    QString PQgetvalue( int row, int col );
    bool PQgetisnull( int row, int col );

    int PQnfields();
    QString PQfname( int col );
    int PQftable( int col );
    int PQftype( int col );
    int PQftablecol( int col );
    Oid PQoidValue();

    PGresult *result() const { return mRes; }

  private:
    PGresult *mRes;
};

class QgsPostgresConn : public QObject
{
    Q_OBJECT;
  public:
    static QgsPostgresConn *connectDb( QString connInfo, bool readOnly );
    void disconnect();

    //! get postgis version string
    QString postgisVersion();

    //! get status of GEOS capability
    bool hasGEOS();

    //! get status of topology capability
    bool hasTopology();

    //! get status of GIST capability
    bool hasGIST();

    //! get status of PROJ4 capability
    bool hasPROJ();

    //! encode wkb in hex
    bool useWkbHex() { return mUseWkbHex; }

    //! major PostGIS version
    int majorVersion() { return mPostgisVersionMajor; }

    //! minor PostGIS version
    int minorVersion() { return mPostgisVersionMinor; }

    //! PostgreSQL version
    int pgVersion() { return mPostgresqlVersion; }

    //! run a query and free result buffer
    bool PQexecNR( QString query, bool retry = true );

    //! cursor handling
    bool openCursor( QString cursorName, QString declare );
    bool closeCursor( QString cursorName );

#if 0
    PGconn *pgConnection() { return mConn; }
#endif

    //
    // libpq wrapper
    //

    // run a query and check for errors
    PGresult *PQexec( QString query, bool logError = true );
    void PQfinish();
    QString PQerrorMessage();
    int PQsendQuery( QString query );
    int PQstatus();
    PGresult *PQgetResult();
    PGresult *PQprepare( QString stmtName, QString query, int nParams, const Oid *paramTypes );
    PGresult *PQexecPrepared( QString stmtName, const QStringList &params );

    // cancel running query
    bool cancel();

    /** Double quote a PostgreSQL identifier for placement in a SQL string.
     */
    static QString quotedIdentifier( QString ident );

    /** Quote a value for placement in a SQL string.
     */
    static QString quotedValue( QVariant value );

    //! Get the list of supported layers
    bool supportedLayers( QVector<QgsPostgresLayerProperty> &layers,
                          bool searchGeometryColumnsOnly = true,
                          bool searchPublicOnly = true,
                          bool allowGeometrylessTables = false );

    void retrieveLayerTypes( QgsPostgresLayerProperty &layerProperty, bool useEstimatedMetadata );

    /** Gets information about the spatial tables */
    bool getTableInfo( bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables );

    /** get primary key candidates (all int4 columns) */
    QStringList pkCandidates( QString schemaName, QString viewName );

    qint64 getBinaryInt( QgsPostgresResult &queryResult, int row, int col );

    QString fieldExpression( const QgsField &fld );

    QString connInfo() const { return mConnInfo; }

    static const int sGeomTypeSelectLimit;

    static QString displayStringForWkbType( QGis::WkbType wkbType );
    static QString displayStringForGeomType( QgsPostgresGeometryColumnType geomType );
    static QGis::WkbType wkbTypeFromPostgis( QString dbType );

    static QString postgisWkbTypeName( QGis::WkbType wkbType );
    static int postgisWkbTypeDim( QGis::WkbType wkbType );
    static void postgisWkbType( QGis::WkbType wkbType, QString &geometryType, int &dim );

    static QString postgisTypeFilter( QString geomCol, QGis::WkbType wkbType, bool isGeography );

    static QGis::WkbType wkbTypeFromGeomType( QGis::GeometryType geomType );

    static QStringList connectionList();
    static QString selectedConnection();
    static void setSelectedConnection( QString theConnName );
    static QgsDataSourceURI connUri( QString theConnName );
    static bool publicSchemaOnly( QString theConnName );
    static bool geometryColumnsOnly( QString theConnName );
    static bool dontResolveType( QString theConnName );
    static bool allowGeometrylessTables( QString theConnName );
    static void deleteConnection( QString theConnName );

  private:
    QgsPostgresConn( QString conninfo, bool readOnly );
    ~QgsPostgresConn();

    int mRef;
    int mOpenCursors;
    PGconn *mConn;
    QString mConnInfo;

    //! GEOS capability
    bool mGeosAvailable;

    //! Topology capability
    bool mTopologyAvailable;

    //! PostGIS version string
    QString mPostgisVersionInfo;

    //! Are mPostgisVersionMajor, mPostgisVersionMinor, mGeosAvailable, mGistAvailable, mProjAvailable, mTopologyAvailable valid?
    bool mGotPostgisVersion;

    //! PostgreSQL version
    int mPostgresqlVersion;

    //! PostGIS major version
    int mPostgisVersionMajor;

    //! PostGIS minor version
    int mPostgisVersionMinor;

    //! GIST capability
    bool mGistAvailable;

    //! PROJ4 capability
    bool mProjAvailable;

    //! encode wkb in hex
    bool mUseWkbHex;

    bool mReadOnly;

    static QMap<QString, QgsPostgresConn *> sConnectionsRW;
    static QMap<QString, QgsPostgresConn *> sConnectionsRO;

    //! List of the supported layers
    QVector<QgsPostgresLayerProperty> mLayersSupported;

    /**
     * Flag indicating whether data from binary cursors must undergo an
     * endian conversion prior to use
     @note

     XXX Umm, it'd be helpful to know what we're swapping from and to.
     XXX Presumably this means swapping from big-endian (network) byte order
     XXX to little-endian; but the inverse transaction is possible, too, and
     XXX that's not reflected in this variable
     */
    bool mSwapEndian;
    void deduceEndian();
};

#endif
