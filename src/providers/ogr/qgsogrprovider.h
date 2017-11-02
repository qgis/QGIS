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
     * \param uri  uniform resource locator (URI) for a dataset
     */
    explicit QgsOgrProvider( QString const &uri = QString() );

    virtual ~QgsOgrProvider();

    /**
     * Get the data source specification. This may be a path or database
     * connection string
     * \param expandAuthConfig Whether to expand any assigned authentication configuration
     * \returns data source specification
     * \note The default authentication configuration expansion is FALSE. This keeps credentials
     * out of layer data source URIs and project files. Expansion should be specifically done
     * only when needed within a provider
     */
    QString dataSourceUri( bool expandAuthConfig = false ) const override;


    virtual QgsAbstractFeatureSource *featureSource() const override;

    virtual QgsCoordinateReferenceSystem crs() const override;
    virtual QStringList subLayers() const override;
    virtual QString storageType() const override;
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    virtual QString subsetString() const override;
    virtual bool supportsSubsetString() const override { return true; }
    virtual bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    virtual QgsWkbTypes::Type wkbType() const override;
    virtual size_t layerCount() const;
    virtual long featureCount() const override;
    virtual QgsFields fields() const override;
    virtual QgsRectangle extent() const override;
    QVariant defaultValue( int fieldId ) const override;
    virtual void updateExtents() override;
    virtual bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = 0 ) override;
    virtual bool deleteFeatures( const QgsFeatureIds &id ) override;
    virtual bool addAttributes( const QList<QgsField> &attributes ) override;
    virtual bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    virtual bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    virtual bool createSpatialIndex() override;
    virtual bool createAttributeIndex( int field ) override;
    virtual QgsVectorDataProvider::Capabilities capabilities() const override;
    virtual void setEncoding( const QString &e ) override;
    virtual bool enterUpdateMode() override { return _enterUpdateMode(); }
    virtual bool leaveUpdateMode() override;
    virtual bool isSaveAndLoadStyleToDatabaseSupported() const override;
    QString fileVectorFilters() const override;
    //! Return a string containing the available database drivers
    QString databaseDrivers() const;
    //! Return a string containing the available directory drivers
    QString protocolDrivers() const;
    //! Return a string containing the available protocol drivers
    QString directoryDrivers() const;

    bool isValid() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    virtual QSet< QVariant > uniqueValues( int index, int limit = -1 ) const override;
    virtual QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
        QgsFeedback *feedback = nullptr ) const override;

    QString name() const override;
    QString description() const override;
    virtual bool doesStrictFeatureTypeCheck() const override;

    //! Return OGR geometry type
    static OGRwkbGeometryType getOgrGeomType( OGRLayerH ogrLayer );

    //! Get single flatten geometry type
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

    void addSubLayerDetailsToSubLayerList( int i, QgsOgrLayer *layer ) const;

    QgsFields mAttributeFields;

    //! Map of field index to default value
    QMap<int, QString> mDefaultValues;

    bool mFirstFieldIsFid = false;
    mutable OGREnvelope *mExtent = nullptr;
    bool mForceRecomputeExtent = false;

    /**
     * This member variable receives the same value as extent_
     in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak*/
    mutable QgsRectangle mExtentRect;

    //! Current working layer (might be a SQL result layer if mSubsetString is set)
    QgsOgrLayer *mOgrLayer = nullptr;

    //! Original layer (not a SQL result layer)
    QgsOgrLayer *mOgrOrigLayer = nullptr;

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

    bool mValid = false;

    OGRwkbGeometryType mOGRGeomType = wkbUnknown;
    long mFeaturesCounted = QgsVectorDataProvider::Uncounted;

    mutable QStringList mSubLayerList;

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

    QgsVectorDataProvider::Capabilities mCapabilities;

    bool doInitialActionsForEdition();

#ifndef QT_NO_NETWORKPROXY
    void setupProxy();
#endif

};

/**
  \class QgsOgrProviderUtils
  \brief Utility class with static methods
  */
class QgsOgrProviderUtils
{
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

        DatasetWithLayers(): mutex( QMutex::Recursive ) {}
    };

    //! Global mutex for QgsOgrProviderUtils
    static QMutex globalMutex;

    //! Map dataset identification to a list of corresponding DatasetWithLayers*
    static QMap< DatasetIdentification, QList<DatasetWithLayers *> > mapSharedDS;

    //! Map a dataset name to the number of opened GDAL dataset objects on it (if opened with GDALOpenWrapper, only for GPKG)
    static QMap< QString, int > mapCountOpenedDS;

    //! Map a dataset handle to its update open mode (if opened with GDALOpenWrapper, only for GPKG)
    static QMap< GDALDatasetH, bool> mapDSHandleToUpdateMode;

    //! Map a dataset name to its last modified data
    static QMap< QString, QDateTime > mapDSNameToLastModifiedDate;

    static bool canUseOpenedDatasets( const QString &dsName );

  public:

    //! Inject credentials into the dsName (if any)
    static QString expandAuthConfig( const QString &dsName );

    static void setRelevantFields( OGRLayerH ogrLayer, int fieldCount, bool fetchGeometry, const QgsAttributeList &fetchAttributes, bool firstAttrIsFid );
    static OGRLayerH setSubsetString( OGRLayerH layer, GDALDatasetH ds, QTextCodec *encoding, const QString &subsetString, bool &origFidAdded );
    static QByteArray quotedIdentifier( QByteArray field, const QString &driverName );

    /**
     * Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant &value );

    //! Wrapper for GDALOpenEx() that does a few lower level actions. Should be strictly paired with GDALCloseWrapper()
    static GDALDatasetH GDALOpenWrapper( const char *pszPath, bool bUpdate, char **papszOpenOptionsIn, GDALDriverH *phDriver );

    //! Wrapper for GDALClose()
    static void GDALCloseWrapper( GDALDatasetH mhDS );

    //! Open a layer given by name, potentially reusing an existing GDALDatasetH if it doesn't already use that layer. release() should be called when done with the object
    static QgsOgrLayer *getLayer( const QString &dsName,
                                  const QString &layerName,
                                  QString &errCause );


    //! Open a layer given by name, potentially reusing an existing GDALDatasetH if it has been opened with the same (updateMode, options) tuple and doesn't already use that layer. release() should be called when done with the object
    static QgsOgrLayer *getLayer( const QString &dsName,
                                  bool updateMode,
                                  const QStringList &options,
                                  const QString &layerName,
                                  QString &errCause );

    //! Open a layer given by index, potentially reusing an existing GDALDatasetH if it doesn't already use that layer. release() should be called when done with the object
    static QgsOgrLayer *getLayer( const QString &dsName,
                                  int layerIndex,
                                  QString &errCause );

    //! Open a layer given by index, potentially reusing an existing GDALDatasetH if it has been opened with the same (updateMode, options) tuple and doesn't already use that layer. release() should be called when done with the object
    static QgsOgrLayer *getLayer( const QString &dsName,
                                  bool updateMode,
                                  const QStringList &options,
                                  int layerIndex,
                                  QString &errCause );

    //! Return a QgsOgrLayer* with a SQL result layer
    static QgsOgrLayer *getSqlLayer( QgsOgrLayer *baseLayer, OGRLayerH hSqlLayer, const QString &sql );

    //! Release a QgsOgrLayer*
    static void release( QgsOgrLayer *&layer );

    //! Make sure that the existing pool of opened datasets on dsName is not accessible for new getLayer() attempts
    static void invalidateCachedDatasets( const QString &dsName );

    //! Return the string to provide to QgsOgrConnPool::instance() methods
    static QString connectionPoolId( const QString &dataSourceURI );
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
    QString sql;
    QgsOgrProviderUtils::DatasetWithLayers *ds = nullptr;
    OGRLayerH hLayer = nullptr;
    QgsOgrFeatureDefn oFDefn;

    QgsOgrLayer();
    ~QgsOgrLayer() = default;

    static QgsOgrLayer *CreateForLayer(
      const QgsOgrProviderUtils::DatasetIdentification &ident,
      const QString &layerName,
      QgsOgrProviderUtils::DatasetWithLayers *ds,
      OGRLayerH hLayer );

    static QgsOgrLayer *CreateForSql(
      const QgsOgrProviderUtils::DatasetIdentification &ident,
      const QString &sql,
      QgsOgrProviderUtils::DatasetWithLayers *ds,
      OGRLayerH hLayer );

    QMutex &mutex() { return ds->mutex; }

  public:

    //! Return GDALDriverH object for current dataset
    GDALDriverH driver();

    //! Return driver name for current dataset
    QString driverName();

    //! Return current dataset name
    const QString &datasetName() const { return ident.dsName; }

    //! Return dataset open mode
    bool updateMode() const { return ident.updateMode; }

    //! Return dataset open options
    const QStringList &options() const { return ident.options; }

    //! Return layer name
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

    //! Return native GDALDatasetH object with the mutex to lock when using it
    GDALDatasetH getDatasetHandleAndMutex( QMutex *&mutex );

    //! Return native OGRLayerH object with the mutex to lock when using it
    OGRLayerH getHandleAndMutex( QMutex *&mutex );

    //! Wrapper of GDALDatasetReleaseResultSet( GDALDatasetExecuteSQL( ... ) )
    void ExecuteSQLNoReturn( const QByteArray &sql );

    //! Wrapper of GDALDatasetExecuteSQL(). Returned layer must be released with QgsOgrProviderUtils::release()
    QgsOgrLayer *ExecuteSQL( const QByteArray &sql );
};

// clazy:excludeall=qstring-allocations

#endif // QGSOGRPROVIDER_H
