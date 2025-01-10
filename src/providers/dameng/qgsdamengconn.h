/***************************************************************************
  qgsdamengconn.h  -  connection class to Dameng/DAMENG
                             -------------------
    begin                : 2011/01/28
    copyright            : ( C ) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   ( at your option ) any later version.                                   *
  *                                                                         *
  ***************************************************************************/

#ifndef QGSDAMENGCONN_H
#define QGSDAMENGCONN_H

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
#include "qgsdamengdatabase.h"
#include "qgsdbquerylog_p.h"

class QgsField;

//! Schema properties structure
struct QgsDamengSchemaProperty
{
  QString name;
  QString description;
  QString owner;
};

//! Layer Property structure
// TODO: Fill to Dameng/DAMENG specifications
struct QgsDamengLayerProperty
{
  // Dameng/DAMENG layer properties
  QList<Qgis::WkbType>          types;
  QString                       schemaName;
  QString                       tableName;
  QString                       geometryColName;
  QgsDamengGeometryColumnType   geometryColType;
  QStringList                   pkCols;
  QList<int>                    srids;
  unsigned int                  nSpCols;
  QString                       sql;
  QString                       relKind;
  bool                          isView = false;
  bool                          isMaterializedView = false;
  QString                       tableComment;

  // TODO: rename this !
  int size() const { Q_ASSERT( types.size() == srids.size() ); return types.size(); }

  QString defaultName() const
  {
    QString n = tableName;
    if ( nSpCols > 1 ) n += '.' + geometryColName;
    return n;
  }

  QgsDamengLayerProperty at( int i ) const
  {
    QgsDamengLayerProperty property;

    Q_ASSERT( i >= 0 && i < size() );

    property.types << types[i];
    property.srids << srids[i];
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
      if (!typeString.isEmpty())
        typeString += '|';
      typeString += QString::number( type );
    }
    QString sridString;
    const auto constSrids = srids;
    for ( const int srid : constSrids )
    {
      if (!sridString.isEmpty())
        sridString += '|';
      sridString += QString::number( srid );
    }

    return QStringLiteral("%1.%2.%3 type=%4 srid=%5 pkCols=%6 sql=%7 nSpCols=%8")
      .arg( schemaName,
        tableName,
        geometryColName,
        typeString,
        sridString,
        pkCols.join( QLatin1Char('|')),
        sql )
      .arg( nSpCols );
  }
#endif
};

class QgsDamengResult
{
  public:
    explicit QgsDamengResult( QgsDMResult *result = nullptr ) : mRes( result ) {}
    ~QgsDamengResult();

    QgsDamengResult &operator=( QgsDMResult *result );
    QgsDamengResult &operator=( const QgsDamengResult &src );

    QgsDamengResult( const QgsDamengResult &rh ) = delete;


    ExecStatusType DMresultStatus();
    QString DMresultErrorMessage();

    QgsDMResult *result() const { return mRes; }

    // 封装后
    int DMntuples();
    QString DMgetvalue( int col );
    bool DMgetisnull( int col );
    int DMgetlength( int col, int type = DSQL_C_TYPE_INVALID );

    int DMnfields();
    udint4 DMftype( int col );
    QString DMftypeName( int col, udint4 type );
    QString DMfname( int col );
    sdint4 DMftable( int col );

  private:
    QgsDMResult *mRes;
};

//! Wraps acquireConnection() and releaseConnection() from a QgsDamengConnPool.
// This can be used for creating std::shared_ptr<QgsPoolDamengConn>.
class QgsPoolDamengConn
{
    class QgsDamengConn *mDmConn;
  public:
    QgsPoolDamengConn( const QString &connInfo );
    ~QgsPoolDamengConn();

    class QgsDamengConn *get() const { return mDmConn; }
};

class QgsDamengConn : public QObject
{
    Q_OBJECT

  public:
    /*
     * \param shared allow using a shared connection. Should never be
     *        called from a thread other than the main one.
     *        An assertion guards against such programmatic error.
     */
    static QgsDamengConn *connectDb( const QString &connInfo, bool readOnly, bool shared = true, bool transaction = false, bool allowRequestCredentials = true );
    QString UserTrans;

    QgsDamengConn( const QString &conninfo, bool readOnly, bool shared, bool transaction, bool allowRequestCredentials = true );
    ~QgsDamengConn() override;

    void ref();
    void unref();

    DmConn *DMconnect( const QString &ipaddr, const QString &port, const QString &user, const QString &pwd );//连接数据库
    DmConn *dmConnection() { return mConn; }
    bool getEstimatedMetadata() { return mUseEstimatedMetadata; }

    /** Returns the underlying database.*/
    QString currentDatabase() const;

    int DMserverVersion() const;
    QString dmSpatialVersion() const; //! Gets dmSpatial version string
    int dmVersion() const { return mDamengVersion; }  //! Dameng version

    bool hasGEOS() const; //! Gets status of GEOS capability
    bool hasTopology() const; //! Gets status of topology capability

    //! run a query and free result buffer
    bool DMexecNR( const QString &query, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );

    QgsDMResult *DMexec( const QString &query, bool logError = true, bool acquireRows = false, const QString &originatorClass = QString(), const QString &queryOrigin = QString() ) const;
    bool DMCancel();
    QgsDMResult *DMgetResult();
    ExecStatusType DMprepare( const QString &query, int nParams, const udint4 *paramTypes, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );
    QgsDMResult *DMexecPrepared( const QByteArray &wkb, const QStringList &params, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );

    void DMfinish();
    QString DMconnErrorMessage() const;
    int DMconnStatus() const;

    bool begin();
    bool commit();
    bool rollback();

    // cancel running query
    bool cancel();

    static QString quotedIdentifier( const QString &ident );/** Double quote("") */
    static QString quotedValue( const QVariant &value );/** Quote a value */

    /**
     * Gets the list of supported layers
     * \param layers list to store layers in
     * \param searchSysdbaOnly
     * \param allowGeometrylessTables
     * \param schema restrict layers to layers within specified schema
     * \returns true if layers were fetched successfully
     */
    bool supportedLayers( QVector<QgsDamengLayerProperty> &layers, bool searchSysdbaOnly = true, bool allowGeometrylessTables = false, const QString &schema = QString() );
    
    /**
     * Get the information about a supported layer
     * \param schema
     * \param table
     * \returns TRUE if the table was found
     */
    bool supportedLayer( QgsDamengLayerProperty &layerProperty, const QString &schema, const QString &table );
    
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
    bool supportedLayersPrivate( QVector<QgsDamengLayerProperty> &layers, bool searchSysdbaOnly = true, bool allowGeometrylessTables = false, const QString &schema = QString(), const QString &table = QString() );

    /** Determine type and srid of a ( vector )layer from data ( possibly estimated ) */
    void retrieveLayerTypes( QgsDamengLayerProperty &layerProperty, bool useEstimatedMetadata );
    void retrieveLayerTypes( QVector<QgsDamengLayerProperty*> &layerProperties, bool useEstimatedMetadata );

    /**
     * Gets the list of database schemas
     * \param schemas list to store schemas in
     * \returns true if schemas where fetched successfully
     * \since QGIS 2.7
     */
    bool getSchemas( QList<QgsDamengSchemaProperty> &schemas );

    /**
     * Gets information about the spatial tables
     * \param searchSysdbaOnly
     * \param allowGeometrylessTables
     * \param schema restrict tables to those within specified schema
     * \returns true if tables were successfully queried
     */
    bool getTableInfo( bool searchSysdbaOnly, bool allowGeometrylessTables, const QString &schema = QString(), const QString &name = QString() );

    QString fieldExpressionForWhereClause( const QgsField &fld, QMetaType::Type valueType = QMetaType::Type::UnknownType, QString expr = "%1");

    QString fieldExpression( const QgsField &fld, QString expr = "%1");

    QString connInfo() const { return mConnInfo; }

    /**
     * Returns a list of supported native types for this connection.
     */
    QList<QgsVectorDataProvider::NativeType> nativeTypes();

    static const int GEOM_TYPE_SELECT_LIMIT;

    static QString displayStringForWkbType( Qgis::WkbType wkbType );
    static QString displayStringForGeomType( QgsDamengGeometryColumnType geomType );
    static Qgis::WkbType wkbTypeFromDmSpatial( const QString &dbType );

    static QString dmSpatialWkbTypeName( Qgis::WkbType wkbType );
    static int dmSpatialWkbTypeDim( Qgis::WkbType wkbType );
    static void dmSpatialWkbType( Qgis::WkbType wkbType, QString &geometryType, int &dim );

    static QString dmSpatialTypeFilter( QString geomCol, Qgis::WkbType wkbType, bool castToGeometry );

    static Qgis::WkbType wkbTypeFromGeomType( Qgis::GeometryType geomType );
    static Qgis::WkbType wkbTypeFromOgcWkbType( unsigned int ogcWkbType );

    static QStringList connectionList();
    static QString selectedConnection();
    static void setSelectedConnection( const QString &connName );
    static QgsDataSourceUri connUri( const QString &connName );
    static bool sysdbaSchemaOnly( const QString &connName );
    static bool dontResolveType( const QString &connName );
    static bool useEstimatedMetadata( const QString &connName );
    static bool allowGeometrylessTables( const QString &connName );
    static bool allowProjectsInDatabase( const QString &connName );
    static void deleteConnection( const QString &connName );

    //! A connection needs to be locked when it uses transactions, see QgsDamengConn::{begin,commit,rollback}
    void lock() { mLock.lock(); }
    void unlock() { mLock.unlock(); }
    
    QgsCoordinateReferenceSystem sridToCrs( int srsId );

  private:

    int mRef;
    DmConn *mConn = nullptr;
    QString mConnInfo;
    bool mUseEstimatedMetadata = false;

    mutable bool mGeosAvailable;  //! GEOS capability
    mutable bool mProjAvailable;  //! PROJ capability
    mutable bool mTopologyAvailable;  //! Topology capability

    mutable int mDamengVersion; //! Dameng version
    mutable bool mGotDmSpatialVersion;  //! DAMENG Spatial version
    mutable QString mDmSpatialVersionInfo;  //! DAMENG Spatial version string


    bool mReadOnly;
    void setEstimatedMetadata(bool EstimatedMetadata) { mUseEstimatedMetadata = EstimatedMetadata; };

    QStringList supportedSpatialTypes() const;

    static QMap< QString, QgsDamengConn*> sConnectionsRW;
    static QMap< QString, QgsDamengConn*> sConnectionsRO;

    //! List of the supported layers
    QVector<QgsDamengLayerProperty> mLayersSupported;

    /**
     * Flag indicating whether data from binary cursors must undergo an
     * endian conversion prior to use
     * \note
     *
     * XXX Umm, it'd be helpful to know what we're swapping from and to.
     * XXX Presumably this means swapping from big-endian ( network ) byte order
     * XXX to little-endian; but the inverse transaction is possible, too, and
     * XXX that's not reflected in this variable
     */
    bool mSwapEndian;

    int mNextCursorId;

    bool mShared; //!< Whether the connection is shared by more providers ( must not be if going to be used in worker threads )

    bool mTransaction;

#if QT_VERSION < QT_VERSION_CHECK( 5, 14, 0 )
    mutable QMutex mLock{ QMutex::Recursive };
#else
    mutable QRecursiveMutex mLock;
#endif
};

// clazy:excludeall=qstring-allocations

#endif
