/***************************************************************************
                     qgsogrproviderutils.h
begin                : June 2021
copyright            : (C) 2021 by Nyall Dawson
email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRPROVIDERUTILS_H
#define QGSOGRPROVIDERUTILS_H

#include "qgis_core.h"
#include "qgswkbtypes.h"
#include "qgsvectordataprovider.h"

#include <gdal.h>

#include <QString>
#include <QStringList>
#include <QMap>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QMutex>
#else
#include <QRecursiveMutex>
#endif

class QgsOgrLayer;
class QgsCoordinateReferenceSystem;
class QgsProviderSublayerDetails;

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * Releases a QgsOgrLayer
 */
struct QgsOgrLayerReleaser
{

  /**
   * Releases a QgsOgrLayer \a layer.
   */
  void operator()( QgsOgrLayer *layer ) const;
};

/**
 * Scoped QgsOgrLayer.
 */
using QgsOgrLayerUniquePtr = std::unique_ptr< QgsOgrLayer, QgsOgrLayerReleaser>;

class QgsOgrDataset;

/**
 * Scoped QgsOgrDataset.
 */
using QgsOgrDatasetSharedPtr = std::shared_ptr< QgsOgrDataset>;

/**
 * \class QgsOgrProviderUtils
 * \brief Utility class with static methods
  */
class CORE_EXPORT QgsOgrProviderUtils
{
    friend class QgsOgrDataset;
    friend class QgsOgrLayer;

    //! Identifies a dataset by name, updateMode and options
    class DatasetIdentification
    {
        QString toString() const;

      public:
        QString dsName;
        bool    updateMode = false;
        QStringList options;
        DatasetIdentification() = default;

        bool operator< ( const DatasetIdentification &other ) const;
    };

    //! GDAL dataset objects and layers in use in it
    class DatasetWithLayers
    {
      public:
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QMutex mutex;
#else
        QRecursiveMutex mutex;
#endif
        GDALDatasetH    hDS = nullptr;
        QMap<QString, QgsOgrLayer *>  setLayers;
        int            refCount = 0;
        bool           canBeShared = true;

        DatasetWithLayers()
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
          : mutex( QMutex::Recursive )
#endif
        {}
    };

    //! Map dataset identification to a list of corresponding DatasetWithLayers*
    static QMap< DatasetIdentification, QList<DatasetWithLayers *> > sMapSharedDS;

    static bool canUseOpenedDatasets( const QString &dsName );

    static void releaseInternal( const DatasetIdentification &ident,
                                 DatasetWithLayers *ds,
                                 bool removeFromDatasetList );

    static DatasetWithLayers *createDatasetWithLayers(
      const QString &dsName,
      bool updateMode,
      const QStringList &options,
      const QString &layerName,
      const DatasetIdentification &ident,
      QgsOgrLayerUniquePtr &layer,
      QString &errCause );
  public:

    static QString fileVectorFilters();
    static QString databaseDrivers();
    static QString protocolDrivers();
    static QString directoryDrivers();
    static QStringList fileExtensions();
    static QStringList directoryExtensions();
    static QStringList wildcards();

    //! Whether the file is a local file.
    static bool IsLocalFile( const QString &path );

    /**
     * Creates an empty data source
     * \param uri location to store the file(s)
     * \param format data format (e.g. "ESRI Shapefile")
     * \param vectortype point/line/polygon or multitypes
     * \param attributes a list of name/type pairs for the initial attributes
     * \return TRUE in case of success
     */
    static bool createEmptyDataSource( const QString &uri,
                                       const QString &format,
                                       const QString &encoding,
                                       QgsWkbTypes::Type vectortype,
                                       const QList< QPair<QString, QString> > &attributes,
                                       const QgsCoordinateReferenceSystem &srs,
                                       QString &errorMessage );

    static bool deleteLayer( const QString &uri, QString &errCause );

    //! Inject credentials into the dsName (if any)
    static QString expandAuthConfig( const QString &dsName );

    static void setRelevantFields( OGRLayerH ogrLayer, int fieldCount,
                                   bool fetchGeometry,
                                   const QgsAttributeList &fetchAttributes,
                                   bool firstAttrIsFid,
                                   const QString &subsetString );

    /**
     * Sets a subset string for an OGR \a layer.
     * Might return either layer, or a new OGR SQL result layer
     */
    static OGRLayerH setSubsetString( OGRLayerH layer, GDALDatasetH ds, QTextCodec *encoding, const QString &subsetString );
    static QByteArray quotedIdentifier( QByteArray field, const QString &driverName );

    /**
     * Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant &value );

    //! Wrapper for GDALOpenEx() that does a few lower level actions. Should be strictly paired with GDALCloseWrapper()
    static GDALDatasetH GDALOpenWrapper( const char *pszPath, bool bUpdate, char **papszOpenOptionsIn, GDALDriverH *phDriver );

    //! Wrapper for GDALClose()
    static void GDALCloseWrapper( GDALDatasetH mhDS );

    //! Return a QgsOgrDataset wrapping an already opened GDALDataset. Typical use: by QgsOgrTransaction
    static QgsOgrDatasetSharedPtr getAlreadyOpenedDataset( const QString &dsName );

    //! Open a layer given by name, potentially reusing an existing GDALDatasetH if it doesn't already use that layer.
    static QgsOgrLayerUniquePtr getLayer( const QString &dsName,
                                          const QString &layerName,
                                          QString &errCause );


    //! Open a layer given by name, potentially reusing an existing GDALDatasetH if it has been opened with the same (updateMode, options) tuple and doesn't already use that layer.
    static QgsOgrLayerUniquePtr getLayer( const QString &dsName,
                                          bool updateMode,
                                          const QStringList &options,
                                          const QString &layerName,
                                          QString &errCause,
                                          bool checkModificationDateAgainstCache );

    //! Open a layer given by index, potentially reusing an existing GDALDatasetH if it doesn't already use that layer.
    static QgsOgrLayerUniquePtr getLayer( const QString &dsName,
                                          int layerIndex,
                                          QString &errCause );

    //! Open a layer given by index, potentially reusing an existing GDALDatasetH if it has been opened with the same (updateMode, options) tuple and doesn't already use that layer.
    static QgsOgrLayerUniquePtr getLayer( const QString &dsName,
                                          bool updateMode,
                                          const QStringList &options,
                                          int layerIndex,
                                          QString &errCause,
                                          bool checkModificationDateAgainstCache );

    //! Returns a QgsOgrLayer* with a SQL result layer
    static QgsOgrLayerUniquePtr getSqlLayer( QgsOgrLayer *baseLayer, OGRLayerH hSqlLayer, const QString &sql );

    //! Release a QgsOgrLayer*
    static void release( QgsOgrLayer *&layer );

    //! Release a QgsOgrDataset*
    static void releaseDataset( QgsOgrDataset *ds );

    //! Make sure that the existing pool of opened datasets on dsName is not accessible for new getLayer() attempts
    static void invalidateCachedDatasets( const QString &dsName );

    //! Returns the string to provide to QgsOgrConnPool::instance() methods
    static QString connectionPoolId( const QString &dataSourceURI, bool datasetSharedAmongLayers );

    //! Invalidate the cached last modified date of a dataset
    static void invalidateCachedLastModifiedDate( const QString &dsName );

    //! Converts a QGIS WKB type to the corresponding OGR wkb type
    static OGRwkbGeometryType ogrTypeFromQgisType( QgsWkbTypes::Type type );

    //! Converts a OGR WKB type to the corresponding QGIS wkb type
    static QgsWkbTypes::Type qgisTypeFromOgrType( OGRwkbGeometryType type );

    //! Conerts a string to an OGR WKB geometry type
    static OGRwkbGeometryType ogrWkbGeometryTypeFromName( const QString &typeName );

    //! Gets single flatten geometry type
    static OGRwkbGeometryType ogrWkbSingleFlatten( OGRwkbGeometryType type );

    //! Gets single flatten and linear geometry type
    static OGRwkbGeometryType ogrWkbSingleFlattenAndLinear( OGRwkbGeometryType type );

    static QString ogrWkbGeometryTypeName( OGRwkbGeometryType type );

    //! Resolves the geometry type for a feature, with special handling for some drivers
    static OGRwkbGeometryType resolveGeometryTypeForFeature( OGRFeatureH feature, const QString &driverName );

    static QString analyzeURI( QString const &uri,
                               bool &isSubLayer,
                               int &layerIndex,
                               QString &layerName,
                               QString &subsetString,
                               OGRwkbGeometryType &ogrGeometryTypeFilter,
                               QStringList &openOptions );

    //! Whether a driver can share the same dataset handle among different layers
    static bool canDriverShareSameDatasetAmongLayers( const QString &driverName );

    //! Whether a driver can share the same dataset handle among different layers
    static bool canDriverShareSameDatasetAmongLayers( const QString &driverName,
        bool updateMode,
        const QString &dsName );

    static QList<QgsProviderSublayerDetails> querySubLayerList( int i, QgsOgrLayer *layer, GDALDatasetH hDS, const QString &driverName, Qgis::SublayerQueryFlags flags,
        const QString &baseUri, bool hasSingleLayerOnly, QgsFeedback *feedback = nullptr );

    /**
     * Utility function to create and store a new DB connection
     * \param name is the translatable name of the managed layers (e.g. "GeoPackage")
     * \param extensions is a string with file extensions (e.g. "GeoPackage Database (*.gpkg *.GPKG)")
     * \param ogrDriverName the OGR/GDAL driver name (e.g. "GPKG")
     */
    static bool createConnection( const QString &name, const QString &extensions, const QString &ogrDriverName );

    /**
     * Utility function to store DB connections
     * \param path to the DB
     * \param ogrDriverName the OGR/GDAL driver name (e.g. "GPKG")
     */
    static bool saveConnection( const QString &path, const QString &ogrDriverName );
};


/**
 * \class QgsOgrDataset
 * \brief Wrap a GDALDatasetH object in a thread-safe way
  */
class QgsOgrDataset
{
    friend class QgsOgrProviderUtils;
    friend class QgsOgrTransaction;
    QgsOgrProviderUtils::DatasetIdentification mIdent;
    QgsOgrProviderUtils::DatasetWithLayers *mDs;

    QgsOgrDataset() = default;
    ~QgsOgrDataset() = default;

  public:

    static QgsOgrDatasetSharedPtr create( const QgsOgrProviderUtils::DatasetIdentification &ident,
                                          QgsOgrProviderUtils::DatasetWithLayers *ds );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex &mutex() { return mDs->mutex; }
#else
    QRecursiveMutex &mutex() { return mDs->mutex; }
#endif

    bool executeSQLNoReturn( const QString &sql );

    OGRLayerH getLayerFromNameOrIndex( const QString &layerName, int layerIndex );

    void releaseResultSet( OGRLayerH hSqlLayer );
};


/**
 * \class QgsOgrFeatureDefn
 * \brief Wrap a OGRFieldDefnH object in a thread-safe way
 */
class QgsOgrFeatureDefn
{
    friend class QgsOgrLayer;

    OGRFeatureDefnH hDefn = nullptr;
    QgsOgrLayer *layer = nullptr;

    QgsOgrFeatureDefn() = default;
    ~QgsOgrFeatureDefn() = default;

    OGRFeatureDefnH get();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex &mutex();
#else
    QRecursiveMutex &mutex();
#endif

  public:

    //! Wrapper of OGR_FD_GetFieldCount
    int GetFieldCount();

    //! Wrapper of OGR_FD_GetFieldDefn
    OGRFieldDefnH GetFieldDefn( int );

    //! Wrapper of OGR_FD_GetFieldIndex
    int GetFieldIndex( const QByteArray & );

    //! Wrapper of OGR_FD_GetGeomFieldCount
    int GetGeomFieldCount();

    //! Wrapper of OGR_FD_GetGeomFieldDefn
    OGRGeomFieldDefnH GetGeomFieldDefn( int idx );

    //! Wrapper of OGR_FD_GetGeomType
    OGRwkbGeometryType GetGeomType();

    //! Wrapper of OGR_F_Create
    OGRFeatureH CreateFeature();
};


/**
 * \class QgsOgrLayer
 * \brief Wrap a OGRLayerH object in a thread-safe way
  */
class QgsOgrLayer
{
    friend class QgsOgrFeatureDefn;
    friend class QgsOgrProviderUtils;

    QgsOgrProviderUtils::DatasetIdentification ident;
    bool isSqlLayer = false;
    QString layerName;
    QString sql; // not really used. Just set at QgsOgrLayer::CreateForLayer() time
    QgsOgrProviderUtils::DatasetWithLayers *ds = nullptr;
    OGRLayerH hLayer = nullptr;
    QgsOgrFeatureDefn oFDefn;

    QgsOgrLayer();
    ~QgsOgrLayer() = default;

    static QgsOgrLayerUniquePtr CreateForLayer(
      const QgsOgrProviderUtils::DatasetIdentification &ident,
      const QString &layerName,
      QgsOgrProviderUtils::DatasetWithLayers *ds,
      OGRLayerH hLayer );

    static QgsOgrLayerUniquePtr CreateForSql(
      const QgsOgrProviderUtils::DatasetIdentification &ident,
      const QString &sql,
      QgsOgrProviderUtils::DatasetWithLayers *ds,
      OGRLayerH hLayer );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex &mutex() { return ds->mutex; }
#else
    QRecursiveMutex &mutex() { return ds->mutex; }
#endif

  public:

    //! Returns GDALDriverH object for current dataset
    GDALDriverH driver();

    //! Returns driver name for current dataset
    QString driverName();

    //! Returns current dataset name
    const QString &datasetName() const { return ident.dsName; }

    //! Returns dataset open mode
    bool updateMode() const { return ident.updateMode; }

    //! Returns dataset open options
    const QStringList &options() const { return ident.options; }

    //! Returns layer name
    QByteArray name();

    //! Wrapper of OGR_L_GetLayerCount
    int GetLayerCount();

    //! Wrapper of OGR_L_GetLayerCount
    QByteArray GetFIDColumn();

    //! Wrapper of OGR_L_GetLayerCount
    OGRSpatialReferenceH GetSpatialRef();

    //! Wrapper of OGR_L_GetLayerCount
    void ResetReading();

    //! Wrapper of OGR_L_GetLayerCount
    OGRFeatureH GetNextFeature();

    //! Wrapper of OGR_L_GetLayerCount
    OGRFeatureH GetFeature( GIntBig fid );

    //! Wrapper of OGR_L_GetLayerCount
    QgsOgrFeatureDefn &GetLayerDefn();

    //! Wrapper of OGR_L_GetLayerCount
    GIntBig GetFeatureCount( bool force = false );

    //! Return an approximate feature count
    GIntBig GetApproxFeatureCount();

    //! Return an total feature count based on meta data from package container
    GIntBig GetTotalFeatureCountFromMetaData() const;

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr GetExtent( OGREnvelope *psExtent, bool bForce );

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr CreateFeature( OGRFeatureH hFeature );

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr SetFeature( OGRFeatureH hFeature );

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr DeleteFeature( GIntBig fid );

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr CreateField( OGRFieldDefnH hFieldDefn, bool bStrict );

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr DeleteField( int iField );

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr AlterFieldDefn( int iField, OGRFieldDefnH hNewFieldDefn, int flags );

    //! Wrapper of OGR_L_GetLayerCount
    int TestCapability( const char * );

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr StartTransaction();

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr CommitTransaction();

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr RollbackTransaction();

    //! Wrapper of OGR_L_GetLayerCount
    OGRErr SyncToDisk();

    //! Wrapper of OGR_L_GetLayerCount
    OGRGeometryH GetSpatialFilter();

    //! Wrapper of OGR_L_GetLayerCount
    void SetSpatialFilter( OGRGeometryH );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    //! Returns native GDALDatasetH object with the mutex to lock when using it
    GDALDatasetH getDatasetHandleAndMutex( QMutex *&mutex ) const;

    //! Returns native OGRLayerH object with the mutex to lock when using it
    OGRLayerH getHandleAndMutex( QMutex *&mutex ) const;
#else
    //! Returns native GDALDatasetH object with the mutex to lock when using it
    GDALDatasetH getDatasetHandleAndMutex( QRecursiveMutex *&mutex ) const;

    //! Returns native OGRLayerH object with the mutex to lock when using it
    OGRLayerH getHandleAndMutex( QRecursiveMutex *&mutex ) const;
#endif


    //! Wrapper of GDALDatasetReleaseResultSet( GDALDatasetExecuteSQL( ... ) )
    void ExecuteSQLNoReturn( const QByteArray &sql );

    //! Wrapper of GDALDatasetExecuteSQL().
    QgsOgrLayerUniquePtr ExecuteSQL( const QByteArray &sql );

    // Wrapper of GDALGetMetadataItem()
    QString GetMetadataItem( const QString &key, const QString &domain = QString() );
};

///@endcond

#endif // QGSOGRPROVIDERUTILS_H
