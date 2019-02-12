/***************************************************************************
            qgsogrprovider.h Data provider for ESRI shapefile format
                    Formerly known as qgsshapefileprovider.h
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRPROVIDER_H
#define QGSOGRPROVIDER_H

#include "QTextCodec"

#include "qgsrectangle.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayerexporter.h"

class QgsField;
class QgsVectorLayerExporter;

class QgsOgrFeatureIterator;

#include <gdal.h>

class QgsOgrLayer;
class QgsOgrTransaction;

/**
 * Releases a QgsOgrLayer
 */
struct QgsOgrLayerReleaser
{

  /**
   * Releases a QgsOgrLayer \a layer.
   */
  void operator()( QgsOgrLayer *layer );

};

/**
 * Scoped QgsOgrLayer.
 */
using QgsOgrLayerUniquePtr = std::unique_ptr< QgsOgrLayer, QgsOgrLayerReleaser>;

/**
  \class QgsOgrProvider
  \brief Data provider for OGR datasources
  */
class QgsOgrProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    //! Convert a vector layer to a vector file
    static QgsVectorLayerExporter::ExportError createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    /**
     * Constructor of the vector provider
     * \param uri uniform resource locator (URI) for a dataset
     * \param options generic data provider options
     */
    explicit QgsOgrProvider( QString const &uri,
                             const QgsDataProvider::ProviderOptions &options );

    ~QgsOgrProvider() override;

    /**
     * Gets the data source specification. This may be a path or database
     * connection string
     * \param expandAuthConfig Whether to expand any assigned authentication configuration
     * \returns data source specification
     * \note The default authentication configuration expansion is FALSE. This keeps credentials
     * out of layer data source URIs and project files. Expansion should be specifically done
     * only when needed within a provider
     */
    QString dataSourceUri( bool expandAuthConfig = false ) const override;

    QgsAbstractFeatureSource *featureSource() const override;

    QgsCoordinateReferenceSystem crs() const override;
    QStringList subLayers() const override;
    QStringList subLayersWithoutFeatureCount() const;
    QString storageType() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QString subsetString() const override;
    bool supportsSubsetString() const override { return true; }
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    QgsWkbTypes::Type wkbType() const override;
    virtual size_t layerCount() const;
    long featureCount() const override;
    QgsFields fields() const override;
    QgsRectangle extent() const override;
    QVariant defaultValue( int fieldId ) const override;
    QString defaultValueClause( int fieldIndex ) const override;
    bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const override;
    void updateExtents() override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = nullptr ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    bool createSpatialIndex() override;
    bool createAttributeIndex( int field ) override;
    QgsVectorDataProvider::Capabilities capabilities() const override;
    void setEncoding( const QString &e ) override;
    bool enterUpdateMode() override { return _enterUpdateMode(); }
    bool leaveUpdateMode() override;
    bool isSaveAndLoadStyleToDatabaseSupported() const override;
    bool isDeleteStyleFromDatabaseSupported() const override;
    QString fileVectorFilters() const override;
    //! Returns a string containing the available database drivers
    QString databaseDrivers() const;
    //! Returns a string containing the available directory drivers
    QString protocolDrivers() const;
    //! Returns a string containing the available protocol drivers
    QString directoryDrivers() const;

    bool isValid() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet< QVariant > uniqueValues( int index, int limit = -1 ) const override;
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = nullptr ) const override;

    QString name() const override;
    QString description() const override;
    QgsTransaction *transaction() const override;
    bool doesStrictFeatureTypeCheck() const override;

    //! Returns OGR geometry type
    static OGRwkbGeometryType getOgrGeomType( OGRLayerH ogrLayer );

    //! Gets single flatten geometry type
    static OGRwkbGeometryType ogrWkbSingleFlatten( OGRwkbGeometryType type );

    QString layerName() const { return mLayerName; }

    QString filePath() const { return mFilePath; }

    int layerIndex() const { return mLayerIndex; }

    QByteArray quotedIdentifier( const QByteArray &field ) const;

    /**
     * A forced reload invalidates the underlying connection.
     * E.g. in case a shapefile is replaced, the old file will be closed
     * and the new file will be opened.
     */
    void forceReload() override;
    void reloadData() override;

  protected:
    //! Loads fields from input file to member attributeFields
    void loadFields();

    //! Find out the number of features of the whole layer
    void recalculateFeatureCount();

    //! Tell OGR, which fields to fetch in nextFeature/featureAtId (ie. which not to ignore)
    void setRelevantFields( bool fetchGeometry, const QgsAttributeList &fetchAttributes );

    //! Convert a QgsField to work with OGR
    static bool convertField( QgsField &field, const QTextCodec &encoding );

    //! Clean shapefile from features which are marked as deleted
    void repack();

    //! Invalidate extent and optionally force its low level recomputation
    void invalidateCachedExtent( bool bForceRecomputeExtent );

    enum OpenMode
    {
      OpenModeInitial,
      OpenModeSameAsCurrent,
      OpenModeForceReadOnly,
      OpenModeForceUpdate,
      OpenModeForceUpdateRepackOff
    };

    void open( OpenMode mode );
    void close();

    bool _enterUpdateMode( bool implicit = false );

  private:
    unsigned char *getGeometryPointer( OGRFeatureH fet );
    QString ogrWkbGeometryTypeName( OGRwkbGeometryType type ) const;

    //! Starts a transaction if possible and return true in that case
    bool startTransaction();

    //! Commits a transaction
    bool commitTransaction();

    //! Does the real job of settings the subset string and adds an argument to disable update capabilities
    bool _setSubsetString( const QString &theSQL, bool updateFeatureCount = true, bool updateCapabilities = true, bool hasExistingRef = true );

    void addSubLayerDetailsToSubLayerList( int i, QgsOgrLayer *layer, bool withFeatureCount ) const;

    QStringList _subLayers( bool withFeatureCount ) const;

    QgsFields mAttributeFields;

    //! Map of field index to default value
    QMap<int, QString> mDefaultValues;

    bool mFirstFieldIsFid = false;
    mutable std::unique_ptr< OGREnvelope > mExtent;
    bool mForceRecomputeExtent = false;

    /**
     * This member variable receives the same value as extent_
     in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak*/
    mutable QgsRectangle mExtentRect;

    /**
     * Current working layer - will point to either mOgrSqlLayer or mOgrOrigLayer depending
     * on whether a subset string is set
     */
    QgsOgrLayer *mOgrLayer = nullptr;

    //! SQL result layer, used if a subset string is set
    QgsOgrLayerUniquePtr mOgrSqlLayer;

    //! Original layer (not a SQL result layer)
    QgsOgrLayerUniquePtr mOgrOrigLayer;

    //! path to filename
    QString mFilePath;

    //! layer name
    QString mLayerName;

    //! layer index
    int mLayerIndex = 0;

    //! was a sub layer requested?
    bool mIsSubLayer = false;

    /**
     * Optional geometry type for layers with multiple geometries,
     *  otherwise wkbUnknown. This type is always flatten (2D) and single, it means
     *  that 2D, 25D, single and multi types are mixed in one sublayer */
    OGRwkbGeometryType mOgrGeometryTypeFilter = wkbUnknown;

    //! current spatial filter
    QgsRectangle mFetchRect;

    //! String used to define a subset of the layer
    QString mSubsetString;

    // Friendly name of the GDAL Driver that was actually used to open the layer
    QString mGDALDriverName;

    //! Whether we can share the same dataset handle among different layers
    bool mShareSameDatasetAmongLayers = true;

    bool mValid = false;

    OGRwkbGeometryType mOGRGeomType = wkbUnknown;
    long mFeaturesCounted = QgsVectorDataProvider::Uncounted;

    mutable QStringList mSubLayerList;

    //! converts \a value from json QVariant to QString
    QString jsonStringValue( const QVariant &value ) const;

    bool addFeaturePrivate( QgsFeature &f, QgsFeatureSink::Flags flags );
    //! Deletes one feature
    bool deleteFeature( QgsFeatureId id );

    //! Calls OGR_L_SyncToDisk and recreates the spatial index if present
    bool syncToDisc();

    friend class QgsOgrFeatureSource;

    //! Whether the file is opened in write mode
    bool mWriteAccess = false;

    //! Whether the file can potentially be opened in write mode (but not necessarily currently)
    bool mWriteAccessPossible = false;

    //! Whether the open mode of the datasource changes w.r.t calls to enterUpdateMode() / leaveUpdateMode()
    bool mDynamicWriteAccess = false;

    bool mShapefileMayBeCorrupted = false;

    //! Converts the geometry to the layer type if necessary. Takes ownership of the passed geometry
    OGRGeometryH ConvertGeometryIfNecessary( OGRGeometryH );

    int mUpdateModeStackDepth = 0;

    bool mDeferRepack = false;

    void computeCapabilities();

    QgsVectorDataProvider::Capabilities mCapabilities = nullptr;

    bool doInitialActionsForEdition();

    bool addAttributeOGRLevel( const QgsField &field, bool &ignoreErrorOut );

#ifndef QT_NO_NETWORKPROXY
    void setupProxy();
#endif

    QgsOgrTransaction *mTransaction = nullptr;

    void setTransaction( QgsTransaction *transaction ) override;
};

class QgsOgrDataset;

/**
 * Scoped QgsOgrDataset.
 */
using QgsOgrDatasetSharedPtr = std::shared_ptr< QgsOgrDataset>;


/**
  \class QgsOgrProviderUtils
  \brief Utility class with static methods
  */
class QgsOgrProviderUtils
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
        QMutex         mutex;
        GDALDatasetH   hDS = nullptr;
        QMap<QString, QgsOgrLayer *>  setLayers;
        int            refCount = 0;
        bool           canBeShared = true;

        DatasetWithLayers(): mutex( QMutex::Recursive ) {}
    };

    //! Global mutex for QgsOgrProviderUtils
    static QMutex sGlobalMutex;

    //! Map dataset identification to a list of corresponding DatasetWithLayers*
    static QMap< DatasetIdentification, QList<DatasetWithLayers *> > sMapSharedDS;

    //! Map a dataset name to the number of opened GDAL dataset objects on it (if opened with GDALOpenWrapper, only for GPKG)
    static QMap< QString, int > sMapCountOpenedDS;

    //! Map a dataset handle to its update open mode (if opened with GDALOpenWrapper, only for GPKG)
    static QHash< GDALDatasetH, bool> sMapDSHandleToUpdateMode;

    //! Map a dataset name to its last modified data
    static QMap< QString, QDateTime > sMapDSNameToLastModifiedDate;

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
    static void releaseDataset( QgsOgrDataset *&ds );

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

    //! Whether a driver can share the same dataset handle among different layers
    static bool canDriverShareSameDatasetAmongLayers( const QString &driverName );
};


/**
  \class QgsOgrDataset
  \brief Wrap a GDALDatasetH object in a thread-safe way
  */
class QgsOgrDataset
{
    friend class QgsOgrProviderUtils;
    QgsOgrProviderUtils::DatasetIdentification mIdent;
    QgsOgrProviderUtils::DatasetWithLayers *mDs;

    QgsOgrDataset() = default;
    ~QgsOgrDataset() = default;

  public:

    static QgsOgrDatasetSharedPtr create( const QgsOgrProviderUtils::DatasetIdentification &ident,
                                          QgsOgrProviderUtils::DatasetWithLayers *ds );

    QMutex &mutex() { return mDs->mutex; }

    bool executeSQLNoReturn( const QString &sql );

    OGRLayerH getLayerFromNameOrIndex( const QString &layerName, int layerIndex );

    void releaseResultSet( OGRLayerH hSqlLayer );
};


/**
  \class QgsOgrFeatureDefn
  \brief Wrap a OGRFieldDefnH object in a thread-safe way
  */
class QgsOgrFeatureDefn
{
    friend class QgsOgrLayer;

    OGRFeatureDefnH hDefn = nullptr;
    QgsOgrLayer *layer = nullptr;

    QgsOgrFeatureDefn() = default;
    ~QgsOgrFeatureDefn() = default;

    OGRFeatureDefnH get();
    QMutex &mutex();

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
  \class QgsOgrLayer
  \brief Wrap a OGRLayerH object in a thread-safe way
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

    QMutex &mutex() { return ds->mutex; }

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

    //! Returns native GDALDatasetH object with the mutex to lock when using it
    GDALDatasetH getDatasetHandleAndMutex( QMutex *&mutex );

    //! Returns native OGRLayerH object with the mutex to lock when using it
    OGRLayerH getHandleAndMutex( QMutex *&mutex );

    //! Wrapper of GDALDatasetReleaseResultSet( GDALDatasetExecuteSQL( ... ) )
    void ExecuteSQLNoReturn( const QByteArray &sql );

    //! Wrapper of GDALDatasetExecuteSQL().
    QgsOgrLayerUniquePtr ExecuteSQL( const QByteArray &sql );
};


// clazy:excludeall=qstring-allocations

#endif // QGSOGRPROVIDER_H
