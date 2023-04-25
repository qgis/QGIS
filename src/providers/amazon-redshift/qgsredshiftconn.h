/***************************************************************************
   qgsredshiftconn.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTCONN_H
#define QGSREDSHIFTCONN_H

#include <QMap>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QVector>

#include "qgis.h"
#include "qgsconfig.h"
#include "qgsdatasourceuri.h"
#include "qgsvectordataprovider.h"
#include "qgswkbtypes.h"

extern "C"
{
#include <libpq-fe.h>
}

class QgsField;

//! Spatial column types
enum QgsRedshiftGeometryColumnType
{
  SctNone,
  SctGeometry,
  SctGeography
};

//! Schema properties structure
struct QgsRedshiftSchemaProperty
{
  QString name;
  QString description;
  QString owner;
};

//! Layer Property structure
struct QgsRedshiftLayerProperty
{
  // Redshift layer properties
  QList<Qgis::WkbType> types;
  QString databaseName;
  QString schemaName;
  QString tableName;
  QString geometryColName;
  QgsRedshiftGeometryColumnType geometryColType;
  QStringList pkCols;
  QList<int> srids;
  unsigned int nSpCols;
  QString sql;
  QString relKind;
  bool isView = false;
  bool isMaterializedView = false;
  QString tableComment;

  // TODO: rename this !
  int size() const
  {
    Q_ASSERT( types.size() == srids.size() );
    return types.size();
  }

  QString defaultName() const
  {
    QString n = tableName;
    if ( nSpCols > 1 )
      n += '.' + geometryColName;
    return n;
  }

  QgsRedshiftLayerProperty at( int i ) const
  {
    QgsRedshiftLayerProperty property;

    Q_ASSERT( i >= 0 && i < size() );

    property.types << types[i];
    property.srids << srids[i];
    property.databaseName = databaseName;
    property.schemaName = schemaName;
    property.tableName = tableName;
    property.geometryColName = geometryColName;
    property.geometryColType = geometryColType;
    property.pkCols = pkCols;
    property.nSpCols = nSpCols;
    property.sql = sql;
    property.relKind = relKind;
    property.isView = isView;
    property.isMaterializedView = isMaterializedView;
    property.tableComment = tableComment;

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
    for ( int srid : constSrids )
    {
      if ( !sridString.isEmpty() )
        sridString += '|';
      sridString += QString::number( srid );
    }

    QString columnInfo;
    if ( databaseName.isEmpty() )
    {
      columnInfo = QStringLiteral( "%1.%2.%3" ).arg( schemaName, tableName,
                   geometryColName );
    }
    else
    {
      columnInfo = QStringLiteral( "%1.%2.%3.%4" ).arg( databaseName,
                   schemaName, tableName, geometryColName );
    }
    return QStringLiteral( "%1 type=%2 srid=%3 pkCols=%4 sql=%5 nSpCols=%6" )
           .arg( columnInfo, typeString, sridString,
                 pkCols.join( QStringLiteral( "|" ) ), sql )
           .arg( nSpCols );
  }
#endif
};

class QgsRedshiftResult
{
  public:
    explicit QgsRedshiftResult( PGresult *result = nullptr ) : mRes( result )
    {
    }
    ~QgsRedshiftResult();

    QgsRedshiftResult &operator=( PGresult *result );
    QgsRedshiftResult &operator=( const QgsRedshiftResult &src );

    QgsRedshiftResult( const QgsRedshiftResult &rh ) = delete;

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

    PGresult *result() const
    {
      return mRes;
    }

  private:
    PGresult *mRes = nullptr;
};

class QgsPoolRedshiftConn
{
    class QgsRedshiftConn *mConn;

  public:
    QgsPoolRedshiftConn( const QString &connInfo );
    ~QgsPoolRedshiftConn();

    class QgsRedshiftConn *get() const
    {
        return mConn;
    }
};

class QgsRedshiftConn : public QObject
{
    Q_OBJECT

  public:
    static const int SINGLE_NODE_MAX_FETCH_SIZE = 1000;
    static const int MULTI_NODE_MAX_FETCH_SIZE = 2000;

    /*
     * \param shared allow using a shared connection. Should never be
     *        called from a thread other than the main one.
     *        An assertion guards against such programmatic error.
     */
    static QgsRedshiftConn *connectDb( const QString &connInfo, bool readOnly, bool shared = true );

    void ref();
    void unref();

    //! Get spatial version string
    QString spatialVersion() const;

    //! encode wkb in hex
    bool useWkbHex() const
    {
      return mUseWkbHex;
    }

    //! 'True' if cluster supports Geography datatype
    bool supportsGeography() const;

    //! run a query and free result buffer
    bool PQexecNR( const QString &query );

    //! cursor handling
    bool openCursor( const QString &cursorName, const QString &declare,
                     bool isExternalDatabase = false );
    bool closeCursor( const QString &cursorName );

    QString uniqueCursorName();

    //
    // libpq wrapper
    //

    // run a query and check for errors, thread-safe
    PGresult *PQexec( const QString &query, bool logError = true, bool retry = true ) const;
    int PQCancel();
    void PQfinish();
    QString PQerrorMessage() const;
    int PQstatus() const;
    PGresult *PQprepare( const QString &stmtName, const QString &query, int nParams, const Oid *paramTypes );
    PGresult *PQexecPrepared( const QString &stmtName, const QStringList &params );

    /**
     * PQsendQuery is used for asynchronous queries (with PQgetResult)
     * Thread safety must be ensured by the caller by calling
     * QgsRedshiftConn::lock() and QgsRedshiftConn::unlock()
     */
    int PQsendQuery( const QString &query );

    /**
     * PQgetResult is used for asynchronous queries (with PQsendQuery)
     * Thread safety must be ensured by the caller by calling
     * QgsRedshiftConn::lock() and QgsRedshiftConn::unlock()
     */
    PGresult *PQgetResult();

    bool begin();
    bool commit();
    bool rollback();

    // cancel running query
    bool cancel();

    /**
     * Double quote an identifier for placement in a SQL string.
     */
    static QString quotedIdentifier( const QString &ident );

    /**
     * Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant &value );

    /**
     * Find whether a given view is a materialized view
     */
    bool isMaterializedView( const QString &schemaName, const QString &viewName );

    /**
     * Gets the list of supported layers
     * \param layers list to store layers in
     * contained in the geometry_columns metatable
     * \param searchPublicOnly
     * \param allowGeometrylessTables
     * \param schema restrict layers to layers within specified schema
     * \returns true if layers were fetched successfully
     */
    bool supportedLayers( QVector<QgsRedshiftLayerProperty> &layers, bool searchPublicOnly = true,
                          bool allowGeometrylessTables = false, QString schema = QString() );

    /**
     * Gets the list of database schemas
     * \param schemas list to store schemas in
     * \returns true if schemas where fetched successfully
     * \since QGIS 2.7
     */
    bool getSchemas( QList<QgsRedshiftSchemaProperty> &schemas );

    /**
     * Determine type and srid of a layer from data (possibly estimated)
     */
    void retrieveLayerTypes( QgsRedshiftLayerProperty &layerProperty, bool useEstimatedMetadata );

    /**
     * Determine type and srid of a vector of layers from data (possibly
     * estimated)
     */
    void retrieveLayerTypes( QVector<QgsRedshiftLayerProperty *> &layerProperties, bool useEstimatedMetadata );

    /**
     * Search for a possible private key column, add all available columns for
     * the user to choose. Returns 'false' if a SELECT query fails for the
     * datashare.
     * \param fromTable the external table complete location
     * <database>.<schema>.<table>
     * \param pkCols reference to the layerProperties columns list.
     * \returns true if the SELECT query succeded false otherwise.
     */
    bool setPkColumns( const QString &fromTable, QStringList &pkCols );

    /**
     * Gets information about the external (datashared) spatial tables contained
     * in the meta table. Populates mLayersSupported with available
     * entries.
     * \param meta_table can be either svv_geometry_columns or geography_columns
     */
    void getDatashareTablesInfo( const QString &metaTableName,
                                 const int tableType,
                                 const QString &currentDatabaseName );

    /**
     * Gets information about the spatial tables
     * contained in the geometry_columns metatable
     * \param searchPublicOnly
     * \param allowGeometrylessTables
     * \param schema restrict tables to those within specified schema
     * \returns true if tables were successfully queried
     */
    bool getTableInfo( bool searchPublicOnly, bool allowGeometrylessTables, QString schema = QString() );

    QString fieldExpressionForWhereClause( const QgsField &fld, QVariant::Type valueType = QVariant::LastType,
                                           QString expr = "%1" );

    QString fieldExpression( const QgsField &fld, QString expr = "%1" );

    QString connInfo() const
    {
      return mConnInfo;
    }

    /**
     * Returns a list of supported native types for this connection.
     * \since QGIS 3.16
     */
    QList<QgsVectorDataProvider::NativeType> nativeTypes();

    /**
     * Returns the underlying database.
     *
     * \since QGIS 3.0
     */
    QString currentDatabase() const;

    /*
     * Query the number of nodes on the cluster and set the cluster
     * type accordingly.
     */
    void setClusterType();

    int getFetchLimit() const;

    static const int GEOM_TYPE_SELECT_LIMIT;

    static QString displayStringForWkbType( Qgis::WkbType wkbType );
    static QString displayStringForGeomType( QgsRedshiftGeometryColumnType geomType );

    static QString redshiftWkbTypeName( Qgis::WkbType wkbType );
    static void redshiftWkbType( Qgis::WkbType wkbType, QString &geometryType, int &dim );

    static QString spatialTypeFilter( QString geomCol, Qgis::WkbType wkbType, bool castToGeometry );

    static Qgis::WkbType wkbTypeFromGeomType( Qgis::GeometryType geomType );

    static QStringList connectionList();
    static QString selectedConnection();
    static void setSelectedConnection( const QString &connName );
    static QgsDataSourceUri connUri( const QString &connName );
    static bool publicSchemaOnly( const QString &connName );
    static bool dontResolveType( const QString &connName );
    static bool useEstimatedMetadata( const QString &connName );
    static bool allowGeometrylessTables( const QString &connName );
    static bool allowProjectsInDatabase( const QString &connName );
    static void deleteConnection( const QString &connName );

    /**
     * A connection needs to be locked when it uses transactions, see
     * QgsRedshiftConn::{begin,commit,rollback}
     */
    void lock()
    {
      mLock.lock();
    }
    void unlock()
    {
      mLock.unlock();
    }

  private:
    QgsRedshiftConn( const QString &conninfo, bool readOnly, bool shared );
    ~QgsRedshiftConn() override;

    int mRef;
    int mOpenCursors;
    PGconn *mConn = nullptr;
    QString mConnInfo;

    // Default timeout
    static const int DEFAULT_TIMEOUT = 30;

    //! PROJ capability
    mutable bool mProjAvailable;

    //! Spatial version string
    mutable QString mSpatialVersionInfo;

    //! variable that stores whether we already got the spatial version
    mutable bool mGotSpatialVersion;

    //! Spatial version number
    mutable int mSpatialVersion;

    //! Supports geography
    mutable bool mSupportsGeography;

    //! encode wkb in hex
    mutable bool mUseWkbHex;

    bool mReadOnly;

    QStringList supportedSpatialTypes() const;

    static QMap<QString, QgsRedshiftConn *> sConnectionsRW;
    static QMap<QString, QgsRedshiftConn *> sConnectionsRO;

    //! List of the supported layers
    QVector<QgsRedshiftLayerProperty> mLayersSupported;

    int mNextCursorId;

    //! Whether the connection is shared by more providers
    // (must not be if going to be used in worker threads)
    bool mShared;

    mutable QMutex mLock;

    const QHash<QString, bool> mStandardKeyColumns =
    {
      std::pair<QString, bool>( "id", true ),
      std::pair<QString, bool>( "fid", true ),
      std::pair<QString, bool>( "gid", true )
    };

    //! If we cannot determine cluster type, use the lower fetch limit (the
    //  limit for single node)
    bool mIsSingleNode = true;
    QString mDatabaseName;
};

// clazy:excludeall=qstring-allocations

#endif // QGSREDSHIFTCONN_H
