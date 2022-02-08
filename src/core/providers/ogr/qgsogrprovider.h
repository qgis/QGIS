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

#include "qgsvectordataprovider.h"
#include "qgsogrproviderutils.h"

#include <gdal.h>

class QgsOgrLayer;
class QgsOgrTransaction;
class QgsProviderSublayerDetails;

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * \class QgsOgrProvider
 * \brief Data provider for OGR datasources
  */
class QgsOgrProvider final: public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    //! Convert a vector layer to a vector file
    static Qgis::VectorExportResult createEmptyLayer(
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
                             const QgsDataProvider::ProviderOptions &providerOptions,
                             QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

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

    /**
     * Gets the number of sublayer in the OGR datasource.
     * layer_styles is not counted.
     * \since QGIS 3.16
     */
    uint subLayerCount() const override;
    QStringList subLayers() const override;
    QgsLayerMetadata layerMetadata() const override;
    QString storageType() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QString subsetString() const override;
    bool supportsSubsetString() const override { return true; }
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    QgsWkbTypes::Type wkbType() const override;
    virtual size_t layerCount() const;
    long long featureCount() const override;
    QgsFields fields() const override;
    QgsRectangle extent() const override;
    QVariant defaultValue( int fieldId ) const override;
    QString defaultValueClause( int fieldIndex ) const override;
    bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const override;
    void updateExtents() override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    bool createSpatialIndex() override;
    bool createAttributeIndex( int field ) override;
    QgsVectorDataProvider::Capabilities capabilities() const override;
    QgsAttributeList pkAttributeIndexes() const override { return mPrimaryKeyAttrs; }
    void setEncoding( const QString &e ) override;
    bool enterUpdateMode() override { return _enterUpdateMode(); }
    bool leaveUpdateMode() override;
    bool isSaveAndLoadStyleToDatabaseSupported() const override;
    bool isDeleteStyleFromDatabaseSupported() const override;
    QString fileVectorFilters() const override;
    bool isValid() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet< QVariant > uniqueValues( int index, int limit = -1 ) const override;
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = nullptr ) const override;
    QgsFeatureSource::SpatialIndexPresence hasSpatialIndex() const override;
    Qgis::VectorLayerTypeFlags vectorLayerTypeFlags() const override;

    QString name() const override;
    static QString providerKey();
    QString description() const override;
    QgsTransaction *transaction() const override;
    bool doesStrictFeatureTypeCheck() const override;
    QgsFeatureRenderer *createRenderer( const QVariantMap &configuration = QVariantMap() ) const override;

    //! Returns OGR geometry type
    static OGRwkbGeometryType getOgrGeomType( const QString &driverName, OGRLayerH ogrLayer );

    QString layerName() const { return mLayerName; }

    QString filePath() const { return mFilePath; }

    QString authCfg() const { return mAuthCfg; }

    int layerIndex() const { return mLayerIndex; }

    QByteArray quotedIdentifier( const QByteArray &field ) const;

  protected:
    //! Loads fields from input file to member attributeFields
    void loadFields();

    //! Loads metadata for the layer
    void loadMetadata();

    //! Find out the number of features of the whole layer
    void recalculateFeatureCount() const;

    //! Tell OGR, which fields to fetch in nextFeature/featureAtId (ie. which not to ignore)
    void setRelevantFields( bool fetchGeometry, const QgsAttributeList &fetchAttributes ) const;

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

    static QString createIndexName( QString tableName, QString field );

    //! Starts a transaction if possible and return true in that case
    bool startTransaction();

    //! Commits a transaction
    bool commitTransaction();

    //! Rolls back a transaction
    bool rollbackTransaction();

    //! Does the real job of settings the subset string and adds an argument to disable update capabilities
    bool _setSubsetString( const QString &theSQL, bool updateFeatureCount = true, bool updateCapabilities = true, bool hasExistingRef = true );

    QList< QgsProviderSublayerDetails > _subLayers( Qgis::SublayerQueryFlags flags ) const;

    QgsFields mAttributeFields;

    //! Map of field index to default value
    QMap<int, QString> mDefaultValues;

    bool mFirstFieldIsFid = false;
    mutable std::unique_ptr< OGREnvelope > mExtent;
    bool mForceRecomputeExtent = false;

    QList<int> mPrimaryKeyAttrs;

    /**
     * This member variable receives the same value as extent_
     * in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak.
    */
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

    QgsLayerMetadata mLayerMetadata;

    //! path to filename
    QString mFilePath;

    //! Authentication configuration
    QString mAuthCfg;

    //! layer name
    QString mLayerName;

    //! layer index
    int mLayerIndex = 0;

    //! open options
    QStringList mOpenOptions;

    //! was a sub layer requested?
    bool mIsSubLayer = false;

    /**
     * Optional geometry type for layers with multiple geometries,
     * otherwise wkbUnknown. This type is always flatten (2D) and single, it means
     * that 2D, 25D, single and multi types are mixed in one sublayer.
    */
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

    //! Whether the next call to featureCount() should refresh the feature count
    mutable bool mRefreshFeatureCount = true;

    mutable long long mFeaturesCounted = static_cast< long long >( Qgis::FeatureCountState::Uncounted );

    mutable QList<QgsProviderSublayerDetails> mSubLayerList;

    //! Converts \a value from json QVariant to QString
    QString jsonStringValue( const QVariant &value ) const;

    //! The \a incrementalFeatureId will generally be -1, except for a few OGR drivers where QGIS will pass on a value when OGR doesn't set it
    bool addFeaturePrivate( QgsFeature &f, QgsFeatureSink::Flags flags, QgsFeatureId incrementalFeatureId = -1 );

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

    QgsVectorDataProvider::Capabilities mCapabilities = QgsVectorDataProvider::Capabilities();

    bool doInitialActionsForEdition();

    bool addAttributeOGRLevel( const QgsField &field, bool &ignoreErrorOut );

    QgsOgrTransaction *mTransaction = nullptr;

    void setTransaction( QgsTransaction *transaction ) override;

    /**
    * Invalidates and reopens the file and resets the feature count
    * E.g. in case a shapefile is replaced, the old file will be closed
    * and the new file will be opened.
    */
    void reloadProviderData() override;

    //! Invalidate GDAL /vsicurl/ RAM cache for mFilePath
    void invalidateNetworkCache();
};

///@endcond
#endif // QGSOGRPROVIDER_H
