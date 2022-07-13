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
#include "qgsvectordataprovider.h"
#include "qgsdbquerylog.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMutex>

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

  bool operator==( const QgsOracleLayerProperty &other ) const
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
    const auto constTypes = types;
    for ( QgsWkbTypes::Type type : constTypes )
    {
      if ( !typeString.isEmpty() )
        typeString += "|";
      typeString += QString::number( type );
    }
    QString sridString;
    const auto constSrids = srids;
    for ( int srid : constSrids )
    {
      if ( !sridString.isEmpty() )
        sridString += "|";
      sridString += QString::number( srid );
    }

    return QString( "%1.%2.%3 type=%4 srid=%5 view=%6%7 sql=%8" )
           .arg( ownerName,
                 tableName,
                 geometryColName,
                 typeString,
                 sridString,
                 isView ? "yes" : "no",
                 isView ? QString( " pk=%1" ).arg( pkCols.join( "|" ) ) : "",
                 sql );
  }
#endif
};


#include "qgsconfig.h"
constexpr int sOracleConQueryLogFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );
#define LoggedExec(_class, query) execLogged( query, true, nullptr, _class, QString(QString( __FILE__ ).mid( sOracleConQueryLogFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")") )
#define LoggedExecPrivate(_class, query, sql, params ) execLogged( query, sql, params, _class, QString(QString( __FILE__ ).mid( sOracleConQueryLogFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")") )


/**
 * Wraps acquireConnection() and releaseConnection() from a QgsOracleConnPool.
 * This can be used to ensure a connection is correctly released when scope ends
 */
class QgsPoolOracleConn
{
    class QgsOracleConn *mConn;
  public:
    QgsPoolOracleConn( const QString &connInfo );
    ~QgsPoolOracleConn();

    class QgsOracleConn *get() const { return mConn; }
};


class QgsOracleConn : public QObject
{
    Q_OBJECT
  public:
    static QgsOracleConn *connectDb( const QgsDataSourceUri &uri, bool transaction );
    void disconnect();

    /**
     * Try to reconnect to database after timeout
     */
    void reconnect();

    void ref() { ++mRef; }
    void unref();

    //! A connection needs to be locked when it uses transactions, see QgsOracleConn::{begin,commit,rollback}
    void lock() { mLock.lock(); }
    void unlock() { mLock.unlock(); }

    /**
     * Double quote a Oracle identifier for placement in a SQL string.
     */
    static QString quotedIdentifier( QString ident );

    /**
     * Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant &value, QVariant::Type type = QVariant::Invalid );

    bool exec( const QString &query, bool logError = true, QString *errorMessage = nullptr );
    bool execLogged( const QString &sql, bool logError = true, QString *errorMessage = nullptr, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );

    bool begin( QSqlDatabase &db );
    bool commit( QSqlDatabase &db );
    bool rollback( QSqlDatabase &db );

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

    /**
     * \returns Oracle database major version, -1 if an error occurred
     */
    int version();

    /**
     * Returns a list of supported native types for this connection.
     * \since QGIS 3.18
     */
    QList<QgsVectorDataProvider::NativeType> nativeTypes();

    /**
     * Returns spatial index name for column \a geometryColumn in table \a tableName from
     * schema/user \a ownerName.
     * Returns an empty string if there is no spatial index
     * \a isValid is updated with TRUE if the returned index is valid
     * \since QGIS 3.18
     */
    QString getSpatialIndexName( const QString &ownerName, const QString &tableName, const QString &geometryColumn, bool &isValid );

    /**
     * Create a spatial index for for column \a geometryColumn in table \a tableName from
     * schema/user \a ownerName.
     * Returns created index name. An empty string is returned if the creation has failed.
     * \note We assume that the sdo_geom_metadata table is already correctly populated before creating
     * the index. If not, the index creation would failed.
     */
    QString createSpatialIndex( const QString &ownerName, const QString &tableName, const QString &geometryColumn );

    /**
     * Returns list of defined primary keys for \a tableName table in \a ownerName schema/user
     */
    QStringList getPrimaryKeys( const QString &ownerName, const QString &tableName );

    static const int sGeomTypeSelectLimit;

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
    static bool allowProjectsInDatabase( const QString &connName );
    static bool estimatedMetadata( const QString &connName );
    static bool onlyExistingTypes( const QString &connName );
    static void deleteConnection( const QString &connName );
    static QString databaseName( const QString &database, const QString &host, const QString &port );
    static QString toPoolName( const QgsDataSourceUri &uri );

    operator QSqlDatabase() { return mDatabase; }

    static QString getLastExecutedQuery( const QSqlQuery &query );

  private:
    explicit QgsOracleConn( QgsDataSourceUri uri, bool transaction );
    ~QgsOracleConn() override;

    bool exec( QSqlQuery &qry, const QString &sql, const QVariantList &params );
    bool execLogged( QSqlQuery &qry, const QString &sql, const QVariantList &params, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );

    //! reference count
    int mRef;

    QString mCurrentUser;

    //! has spatial
    int mHasSpatial;

    QSqlDatabase mDatabase;
    QSqlQuery mQuery;

    //! List of the supported layers
    QVector<QgsOracleLayerProperty> mLayersSupported;

    mutable QMutex mLock;
    bool mTransaction = false;
    int mSavePointId = 1;

    static QMap<QString, QgsOracleConn *> sConnections;
    static int snConnections;
    static QMap<QString, QDateTime> sBrokenConnections;

    // Connection URI string representation for query logger
    QString mConnInfo;
};

#endif
