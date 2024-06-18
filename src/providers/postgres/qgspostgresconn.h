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
#include <QMutex>

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgswkbtypes.h"
#include "qgsconfig.h"
#include "qgsvectordataprovider.h"
#include "qgsdbquerylog_p.h"

extern "C"
{
#include <libpq-fe.h>
}

class QgsField;

//! Spatial column types
enum QgsPostgresGeometryColumnType
{
  SctNone,
  SctGeometry,
  SctGeography,
  SctTopoGeometry,
  SctPcPatch,
  SctRaster
};

enum QgsPostgresPrimaryKeyType
{
  PktUnknown,
  PktInt,
  PktInt64,
  PktUint64,
  PktTid,
  PktOid,
  PktFidMap
};

//! Schema properties structure
struct QgsPostgresSchemaProperty
{
  QString name;
  QString description;
  QString owner;
};


//! Layer Property structure
// TODO: Fill to Postgres/PostGIS specifications
struct QgsPostgresLayerProperty
{
  // Postgres/PostGIS layer properties
  QList<Qgis::WkbType>          types;
  QString                       schemaName;
  QString                       tableName;
  QString                       geometryColName;
  QgsPostgresGeometryColumnType geometryColType;
  QStringList                   pkCols;
  QList<int>                    srids;
  unsigned int                  nSpCols;
  QString                       sql;
  Qgis::PostgresRelKind         relKind = Qgis::PostgresRelKind::Unknown;
  bool                          isRaster = false;
  QString                       tableComment;

  // TODO: rename this !
  int size() const { Q_ASSERT( types.size() == srids.size() ); return types.size(); }

  QString defaultName() const
  {
    QString n = tableName;
    if ( nSpCols > 1 ) n += '.' + geometryColName;
    return n;
  }

  QgsPostgresLayerProperty at( int i ) const
  {
    QgsPostgresLayerProperty property;

    Q_ASSERT( i >= 0 && i < size() );

    property.types << types[ i ];
    property.srids << srids[ i ];
    property.schemaName         = schemaName;
    property.tableName          = tableName;
    property.geometryColName    = geometryColName;
    property.geometryColType    = geometryColType;
    property.pkCols             = pkCols;
    property.nSpCols            = nSpCols;
    property.sql                = sql;
    property.relKind            = relKind;
    property.isRaster           = isRaster;
    property.tableComment       = tableComment;

    return property;
  }

#ifdef QGISDEBUG
  QString toString() const
  {
    QString typeString;
    const auto constTypes = types;
    for ( const Qgis::WkbType type : constTypes )
    {
      if ( !typeString.isEmpty() )
        typeString += '|';
      typeString += QString::number( static_cast< quint32>( type ) );
    }
    QString sridString;
    const auto constSrids = srids;
    for ( const int srid : constSrids )
    {
      if ( !sridString.isEmpty() )
        sridString += '|';
      sridString += QString::number( srid );
    }

    return QStringLiteral( "%1.%2.%3 type=%4 srid=%5 pkCols=%6 sql=%7 nSpCols=%8" )
           .arg( schemaName,
                 tableName,
                 geometryColName,
                 typeString,
                 sridString,
                 pkCols.join( QLatin1Char( '|' ) ),
                 sql )
           .arg( nSpCols );
  }
#endif
};

class QgsPostgresResult
{
  public:
    explicit QgsPostgresResult( PGresult *result = nullptr ) : mRes( result ) {}
    ~QgsPostgresResult();

    QgsPostgresResult &operator=( PGresult *result );
    QgsPostgresResult &operator=( const QgsPostgresResult &src );

    QgsPostgresResult( const QgsPostgresResult &rh ) = delete;

    ExecStatusType PQresultStatus();
    QString PQresultErrorMessage();

    int PQntuples();
    QString PQgetvalue( int row, int col );
    bool PQgetisnull( int row, int col );

    int PQnfields();
    QString PQfname( int col );
    Oid PQftable( int col );
    Oid PQftype( int col );
    int PQfmod( int col );
    int PQftablecol( int col );
    Oid PQoidValue();

    PGresult *result() const { return mRes; }

  private:
    PGresult *mRes = nullptr;

};

struct PGException
{
    explicit PGException( QgsPostgresResult &r )
      : mWhat( r.PQresultErrorMessage() )
    {}

    QString errorMessage() const
    {
      return mWhat;
    }

  private:
    QString mWhat;
};

//! Wraps acquireConnection() and releaseConnection() from a QgsPostgresConnPool.
// This can be used for creating std::shared_ptr<QgsPoolPostgresConn>.
class QgsPoolPostgresConn
{
    class QgsPostgresConn *mPgConn;
  public:
    QgsPoolPostgresConn( const QString &connInfo );
    ~QgsPoolPostgresConn();

    class QgsPostgresConn *get() const { return mPgConn; }
};

#include "qgsconfig.h"
constexpr int sPostgresConQueryLogFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );
#define QGS_QUERY_LOG_ORIGIN_PG_CON QString(QString( __FILE__ ).mid( sPostgresConQueryLogFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")")
#define LoggedPQexecNR(_class, query) PQexecNR( query, _class, QGS_QUERY_LOG_ORIGIN_PG_CON )
#define LoggedPQexec(_class, query) PQexec( query, true, true, _class, QGS_QUERY_LOG_ORIGIN_PG_CON )
#define LoggedPQexecNoLogError(_class, query ) PQexec( query, false, true, _class, QGS_QUERY_LOG_ORIGIN_PG_CON )

class QgsPostgresConn : public QObject
{
    Q_OBJECT

  public:

    /**
     * Get a new PostgreSQL connection
     *
     * \param connInfo the QgsDataSourceUri connection info with username / password
     * \param readOnly is the connection read only ?
     * \param shared allow using a shared connection if called from the same thread as the main one.
     * \param allowRequestCredentials allow credentials request through current QgsCredentials instance if the provided ones are not valid
     *
     * \returns the PostgreSQL connection
     */
    static QgsPostgresConn *connectDb( const QString &connInfo, bool readOnly, bool shared = true, bool transaction = false, bool allowRequestCredentials = true );

    /**
     * Get a new PostgreSQL connection
     *
     * \param uri the QgsDataSourceUri with username / password
     * \param readOnly is the connection read only ?
     * \param shared allow using a shared connection if called from the same thread as the main one.
     * \param transaction is the connection a transaction ?
     * \param allowRequestCredentials allow credentials request through current QgsCredentials instance if the provided ones are not valid
     *
     * \returns the PostgreSQL connection
     */
    static QgsPostgresConn *connectDb( const QgsDataSourceUri &uri, bool readOnly, bool shared = true, bool transaction = false, bool allowRequestCredentials = true );


    QgsPostgresConn( const QString &conninfo, bool readOnly, bool shared, bool transaction, bool allowRequestCredentials = true );
    ~QgsPostgresConn() override;

    void ref();
    void unref();

    /**
     * Returns the URI associated with the connection.
     */
    const QgsDataSourceUri &uri() const { return mUri; }

    //! Gets postgis version string
    QString postgisVersion() const;

    //! Gets status of GEOS capability
    bool hasGEOS() const;

    //! Gets status of topology capability
    bool hasTopology() const;

    //! Gets status of Pointcloud capability
    bool hasPointcloud() const;

    //! Gets status of Raster capability
    bool hasRaster() const;

    //! encode wkb in hex
    bool useWkbHex() const { return mUseWkbHex; }

    //! major PostGIS version
    int majorVersion() const { return mPostgisVersionMajor; }

    //! minor PostGIS version
    int minorVersion() const { return mPostgisVersionMinor; }

    //! PostgreSQL version
    int pgVersion() const { return mPostgresqlVersion; }

    /**
     * Sets the current user identifier of the current PostgreSQL session
     *
     * \param sessionRole the PostgreSQL ROLE for the session, it must be a role that the current session user is a member of.
     *
     * \returns TRUE if successful
     *
     * \since QGIS 3.28.0
     */
    bool setSessionRole( const QString &sessionRole );

    /**
     * Resets the current user identifier of the current PostgreSQL session
     * to the current session user identifier (user used to log in)
     *
     * \returns TRUE if successful
     *
     * \since QGIS 3.28.0
     */
    bool resetSessionRole();

    //! run a query and free result buffer
    bool PQexecNR( const QString &query, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );

    //! cursor handling
    bool openCursor( const QString &cursorName, const QString &declare );
    bool closeCursor( const QString &cursorName );

    QString uniqueCursorName();

    PGconn *pgConnection() { return mConn; }

    //
    // libpq wrapper
    //

    // run a query and check for errors, thread-safe
    PGresult *PQexec( const QString &query, bool logError = true, bool retry = true, const QString &originatorClass = QString(), const QString &queryOrigin = QString() ) const;
    int PQCancel();
    void PQfinish();
    QString PQerrorMessage() const;
    int PQstatus() const;
    PGresult *PQprepare( const QString &stmtName, const QString &query, int nParams, const Oid *paramTypes, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );
    PGresult *PQexecPrepared( const QString &stmtName, const QStringList &params, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );

    /**
     * PQsendQuery is used for asynchronous queries (with PQgetResult)
     * Thread safety must be ensured by the caller by calling QgsPostgresConn::lock() and QgsPostgresConn::unlock()
     */
    int PQsendQuery( const QString &query );

    /**
     * PQgetResult is used for asynchronous queries (with PQsendQuery)
     * Thread safety must be ensured by the caller by calling QgsPostgresConn::lock() and QgsPostgresConn::unlock()
     */
    PGresult *PQgetResult();

    bool begin();
    bool commit();
    bool rollback();


    // cancel running query
    bool cancel();

    /**
     * Double quote a PostgreSQL identifier for placement in a SQL string.
     */
    static QString quotedIdentifier( const QString &ident );

    /**
     * Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant &value );

    /**
     * Quote a json(b) value for placement in a SQL string.
     * \note a null value will be represented as a NULL and not as a json null.
     */
    static QString quotedJsonValue( const QVariant &value );

    /**
     * Returns the RelKind associated with a value from the relkind column
     * in pg_class.
     */
    static Qgis::PostgresRelKind relKindFromValue( const QString &value );

    /**
     * Gets the list of supported layers
     * \param layers list to store layers in
     * \param searchGeometryColumnsOnly only look for geometry columns which are
     * contained in the geometry_columns metatable
     * \param searchPublicOnly
     * \param allowGeometrylessTables
     * \param schema restrict layers to layers within specified schema
     * \returns true if layers were fetched successfully
     */
    bool supportedLayers( QVector<QgsPostgresLayerProperty> &layers,
                          bool searchGeometryColumnsOnly = true,
                          bool searchPublicOnly = true,
                          bool allowGeometrylessTables = false,
                          const QString &schema = QString() );

    /**
     * Get the information about a supported layer
     * \param schema
     * \param table
     * \returns TRUE if the table was found
     */
    bool supportedLayer( QgsPostgresLayerProperty &layerProperty, const QString &schema, const QString &table );

    /**
     * Gets the list of database schemas
     * \param schemas list to store schemas in
     * \returns true if schemas where fetched successfully
     */
    bool getSchemas( QList<QgsPostgresSchemaProperty> &schemas );

    /**
     * Determine type and srid of a layer from data (possibly estimated)
     */
    void retrieveLayerTypes( QgsPostgresLayerProperty &layerProperty, bool useEstimatedMetadata, QgsFeedback *feedback = nullptr );

    /**
     * Determine type and srid of a vector of layers from data (possibly estimated)
     */
    void retrieveLayerTypes( QVector<QgsPostgresLayerProperty *> &layerProperties, bool useEstimatedMetadata, QgsFeedback *feedback = nullptr );

    /**
     * Gets information about the spatial tables
     * \param searchGeometryColumnsOnly only look for geometry columns which are
     * contained in the geometry_columns metatable
     * \param searchPublicOnly
     * \param allowGeometrylessTables
     * \param schema restrict tables to those within specified schema
     * \param name restrict tables to those with specified name
     * \returns true if tables were successfully queried
     */
    bool getTableInfo( bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables,
                       const QString &schema = QString(), const QString &name = QString() );

    qint64 getBinaryInt( QgsPostgresResult &queryResult, int row, int col );

    QString fieldExpressionForWhereClause( const QgsField &fld, QMetaType::Type valueType = QMetaType::Type::UnknownType, QString expr = "%1" );

    QString fieldExpression( const QgsField &fld, QString expr = "%1" );

    QString connInfo() const { return mConnInfo; }

    /**
     * Returns a list of supported native types for this connection.
     * \since QGIS 3.16
     */
    QList<QgsVectorDataProvider::NativeType> nativeTypes();

    /**
     * Returns the underlying database.
     *
     */
    QString currentDatabase() const;

    static const int GEOM_TYPE_SELECT_LIMIT;

    static QString displayStringForWkbType( Qgis::WkbType wkbType );
    static QString displayStringForGeomType( QgsPostgresGeometryColumnType geomType );
    static Qgis::WkbType wkbTypeFromPostgis( const QString &dbType );

    static QString postgisWkbTypeName( Qgis::WkbType wkbType );
    static int postgisWkbTypeDim( Qgis::WkbType wkbType );
    static void postgisWkbType( Qgis::WkbType wkbType, QString &geometryType, int &dim );

    static QString postgisTypeFilter( QString geomCol, Qgis::WkbType wkbType, bool castToGeometry );

    static Qgis::WkbType wkbTypeFromGeomType( Qgis::GeometryType geomType );
    static Qgis::WkbType wkbTypeFromOgcWkbType( unsigned int ogcWkbType );

    static QStringList connectionList();
    static QString selectedConnection();
    static void setSelectedConnection( const QString &connName );
    static QgsDataSourceUri connUri( const QString &connName );
    static bool publicSchemaOnly( const QString &connName );
    static bool geometryColumnsOnly( const QString &connName );
    static bool dontResolveType( const QString &connName );
    static bool useEstimatedMetadata( const QString &connName );
    static bool allowGeometrylessTables( const QString &connName );
    static bool allowProjectsInDatabase( const QString &connName );
    static void deleteConnection( const QString &connName );
    static bool allowMetadataInDatabase( const QString &connName );

    //! A connection needs to be locked when it uses transactions, see QgsPostgresConn::{begin,commit,rollback}
    void lock() { mLock.lock(); }
    void unlock() { mLock.unlock(); }

    QgsCoordinateReferenceSystem sridToCrs( int srsId );

    int crsToSrid( const QgsCoordinateReferenceSystem &crs );

  private:

    int mRef;
    int mOpenCursors;
    PGconn *mConn = nullptr;
    QString mConnInfo;
    QgsDataSourceUri mUri;

    //! GEOS capability
    mutable bool mGeosAvailable;

    //! PROJ capability
    mutable bool mProjAvailable;

    //! Topology capability
    mutable bool mTopologyAvailable;

    //! PostGIS version string
    mutable QString mPostgisVersionInfo;

    //! Are mPostgisVersionMajor, mPostgisVersionMinor, mGeosAvailable, mTopologyAvailable valid?
    mutable bool mGotPostgisVersion;

    //! PostgreSQL version
    mutable int mPostgresqlVersion;

    //! PostGIS major version
    mutable int mPostgisVersionMajor;

    //! PostGIS minor version
    mutable int mPostgisVersionMinor;

    //! pointcloud support available
    mutable bool mPointcloudAvailable;

    //! raster support available
    mutable bool mRasterAvailable;

    //! encode wkb in hex
    mutable bool mUseWkbHex;

    bool mReadOnly;

    QStringList supportedSpatialTypes() const;

    static QMap<QString, QgsPostgresConn *> sConnectionsRW;
    static QMap<QString, QgsPostgresConn *> sConnectionsRO;

    //! Count number of spatial columns in a given relation
    void addColumnInfo( QgsPostgresLayerProperty &layerProperty, const QString &schemaName, const QString &viewName, bool fetchPkCandidates );

    /**
     * Gets the list of supported layers
     * \param layers list to store layers in
     * \param searchGeometryColumnsOnly only look for geometry columns which are
     * contained in the geometry_columns metatable
     * \param searchPublicOnly
     * \param allowGeometrylessTables
     * \param schema restrict layers to layers within specified schema
     * \param table restrict tables to those with specified table
     * \returns true if layers were fetched successfully
     */
    bool supportedLayersPrivate( QVector<QgsPostgresLayerProperty> &layers,
                                 bool searchGeometryColumnsOnly = true,
                                 bool searchPublicOnly = true,
                                 bool allowGeometrylessTables = false,
                                 const QString &schema = QString(),
                                 const QString &table = QString() );

    //! List of the supported layers
    QVector<QgsPostgresLayerProperty> mLayersSupported;

    /**
     * Flag indicating whether data from binary cursors must undergo an
     * endian conversion prior to use
     * \note
     *
     * XXX Umm, it'd be helpful to know what we're swapping from and to.
     * XXX Presumably this means swapping from big-endian (network) byte order
     * XXX to little-endian; but the inverse transaction is possible, too, and
     * XXX that's not reflected in this variable
     */
    bool mSwapEndian;
    void deduceEndian();

    static QAtomicInt sNextCursorId;

    bool mShared; //!< Whether the connection is shared by more providers (must not be if going to be used in worker threads)

    bool mTransaction;

    QString mCurrentSessionRole;

    mutable QRecursiveMutex mLock;

    /* Mutex protecting sCrsCache */
    QMutex mCrsCacheMutex;

    /* Cache of SRID to CRS */
    mutable QMap<int, QgsCoordinateReferenceSystem> mCrsCache;
};

// clazy:excludeall=qstring-allocations

#endif
