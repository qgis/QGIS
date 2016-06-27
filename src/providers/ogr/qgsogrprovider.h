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

#include "QTextCodec"

#include "qgsrectangle.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayerimport.h"

class QgsField;
class QgsVectorLayerImport;

class QgsOgrFeatureIterator;

#include <ogr_api.h>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x)   (x).toUtf8().constData()
#define TO8F(x)  (x).toUtf8().constData()
#define FROM8(x) QString::fromUtf8(x)
#else
#define TO8(x)   (x).toLocal8Bit().constData()
#define TO8F(x)  QFile::encodeName( x ).constData()
#define FROM8(x) QString::fromLocal8Bit(x)
#endif

/**
  \class QgsOgrProvider
  \brief Data provider for ESRI shapefiles
  */
class QgsOgrProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    /** Convert a vector layer to a vector file */
    static QgsVectorLayerImport::ImportError createEmptyLayer(
      const QString& uri,
      const QgsFields &fields,
      QGis::WkbType wkbType,
      const QgsCoordinateReferenceSystem *srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    /**
     * Constructor of the vector provider
     * @param uri  uniform resource locator (URI) for a dataset
     */
    explicit QgsOgrProvider( QString const & uri = "" );

    /**
     * Destructor
     */
    virtual ~QgsOgrProvider();

    virtual QgsAbstractFeatureSource* featureSource() const override;

    virtual QgsCoordinateReferenceSystem crs() override;

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     */
    virtual QStringList subLayers() const override;

    /**
     *   Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const override;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

    /** Accessor for sql where clause used to limit dataset */
    virtual QString subsetString() override;

    virtual bool supportsSubsetString() override { return true; }

    /** Mutator for sql where clause used to limit dataset size */
    virtual bool setSubsetString( const QString& theSQL, bool updateFeatureCount = true ) override;

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const override;

    /** Return the number of layers for the current data source
     *
     * @note
     *
     * Should this be subLayerCount() instead?
     */
    virtual size_t layerCount() const;

    /**
     * Get the number of features in the layer
     */
    virtual long featureCount() const override;

    /**
     * Get the field information for the layer
     */
    virtual const QgsFields & fields() const override;

    /** Return the extent for this data layer
     */
    virtual QgsRectangle extent() override;

    /** Update the extents
     */
    virtual void updateExtents() override;

    /** Writes a list of features to the file*/
    virtual bool addFeatures( QgsFeatureList & flist ) override;

    /** Deletes a feature*/
    virtual bool deleteFeatures( const QgsFeatureIds & id ) override;

    virtual bool addAttributes( const QList<QgsField> &attributes ) override;
    virtual bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    virtual bool renameAttributes( const QgsFieldNameMap& renamedAttributes ) override;

    /** Changes attribute values of existing features */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

    /** Changes existing geometries*/
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    /** Tries to create a .qix index file for faster access if only a subset of the features is required
     @return true in case of success*/
    virtual bool createSpatialIndex() override;

    /** Create an attribute index on the datasource*/
    virtual bool createAttributeIndex( int field ) override;

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on whether
        a spatial filter is active on this provider, so it may
        be prudent to check this value per intended operation.
        See the OGRLayer::TestCapability API for details.
      */
    virtual int capabilities() const override;

    virtual void setEncoding( const QString& e ) override;

    virtual bool enterUpdateMode() override;

    virtual bool leaveUpdateMode() override;

    /** Return vector file filter string
     *
     * Returns a string suitable for a QFileDialog of vector file formats
     * supported by the data provider.  Naturally this will be an empty string
     * for those data providers that do not deal with plain files, such as
     * databases and servers.
     *
     * @note It'd be nice to eventually be raster/vector neutral.
     */
    /* virtual */
    QString fileVectorFilters() const override;
    /** Return a string containing the available database drivers */
    QString databaseDrivers() const;
    /** Return a string containing the available directory drivers */
    QString protocolDrivers() const;
    /** Return a string containing the available protocol drivers */
    QString directoryDrivers() const;

    /** Returns true if this is a valid shapefile
     */
    bool isValid() override;

    /** Returns the minimum value of an attribute
     *  @param index the index of the attribute
     */
    QVariant minimumValue( int index ) override;

    /** Returns the maximum value of an attribute
     *  @param index the index of the attribute
     */
    QVariant maximumValue( int index ) override;

    /** Return the unique values of an attribute
     *  @param index the index of the attribute
     *  @param values reference to the list of unique values
     */
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues, int limit = -1 ) override;

    /** Return a provider name
     *
     * Essentially just returns the provider key.  Should be used to build file
     * dialogs so that providers can be shown with their supported types. Thus
     * if more than one provider supports a given format, the user is able to
     * select a specific provider to open that file.
     *
     * @note
     *
     * Instead of being pure virtual, might be better to generalize this
     * behavior and presume that none of the sub-classes are going to do
     * anything strange with regards to their name or description?
     *
     */
    QString name() const override;


    /** Return description
     *
     * Return a terse string describing what the provider is.
     *
     * @note
     *
     * Instead of being pure virtual, might be better to generalize this
     * behavior and presume that none of the sub-classes are going to do
     * anything strange with regards to their name or description?
     *
     */
    QString description() const override;

    /** Returns true if the provider is strict about the type of inserted features
     * (e.g. no multipolygon in a polygon layer)
     */
    virtual bool doesStrictFeatureTypeCheck() const override;

    /** Return OGR geometry type */
    static OGRwkbGeometryType getOgrGeomType( OGRLayerH ogrLayer );

    /** Get single flatten geometry type */
    static OGRwkbGeometryType ogrWkbSingleFlatten( OGRwkbGeometryType type );

    QString layerName() const { return mLayerName; }

    QString filePath() const { return mFilePath; }

    int layerIndex() const { return mLayerIndex; }

    QTextCodec* textEncoding() { return mEncoding; }

    QByteArray quotedIdentifier( QByteArray field ) const;

    /**
     * A forced reload invalidates the underlying connection.
     * E.g. in case a shapefile is replaced, the old file will be closed
     * and the new file will be opened.
     */
    void forceReload() override;

    /** Closes and re-open the datasource */
    void reloadData() override;

  protected:
    /** Loads fields from input file to member attributeFields */
    void loadFields();

    /** Find out the number of features of the whole layer */
    void recalculateFeatureCount();

    /** Tell OGR, which fields to fetch in nextFeature/featureAtId (ie. which not to ignore) */
    void setRelevantFields( OGRLayerH ogrLayer, bool fetchGeometry, const QgsAttributeList& fetchAttributes );

    /** Convert a QgsField to work with OGR */
    static bool convertField( QgsField &field, const QTextCodec &encoding );

    /** Clean shapefile from features which are marked as deleted */
    void repack();

    enum OpenMode
    {
      OpenModeInitial,
      OpenModeSameAsCurrent,
      OpenModeForceReadOnly,
      OpenModeForceUpdate,
    };

    void open( OpenMode mode );
    void close();

  private:
    unsigned char *getGeometryPointer( OGRFeatureH fet );
    QString ogrWkbGeometryTypeName( OGRwkbGeometryType type ) const;
    OGRwkbGeometryType ogrWkbGeometryTypeFromName( const QString& typeName ) const;
    QgsFields mAttributeFields;
    bool mFirstFieldIsFid;
    OGRDataSourceH ogrDataSource;
    OGREnvelope* mExtent;

    /** This member variable receives the same value as extent_
     in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak*/
    QgsRectangle mExtentRect;
    OGRLayerH ogrLayer;
    OGRLayerH ogrOrigLayer;

    //! path to filename
    QString mFilePath;

    //! layer name
    QString mLayerName;

    //! layer index
    int mLayerIndex;

    //! was a sub layer requested?
    bool mIsSubLayer;

    /** Optional geometry type for layers with multiple geometries,
     *  otherwise wkbUnknown. This type is always flatten (2D) and single, it means
     *  that 2D, 25D, single and multi types are mixed in one sublayer */
    OGRwkbGeometryType mOgrGeometryTypeFilter;

    //! current spatial filter
    QgsRectangle mFetchRect;

    //! String used to define a subset of the layer
    QString mSubsetString;

    // OGR Driver that was actually used to open the layer
    OGRSFDriverH ogrDriver;

    // Friendly name of the OGR Driver that was actually used to open the layer
    QString ogrDriverName;

    bool mValid;

    OGRwkbGeometryType mOGRGeomType;
    long mFeaturesCounted;

    mutable QStringList mSubLayerList;

    /** Adds one feature*/
    bool addFeature( QgsFeature& f );
    /** Deletes one feature*/
    bool deleteFeature( QgsFeatureId id );

    /** Calls OGR_L_SyncToDisk and recreates the spatial index if present*/
    bool syncToDisc();

    OGRLayerH setSubsetString( OGRLayerH layer, OGRDataSourceH ds );

    friend class QgsOgrFeatureSource;

    /** Whether the file is opened in write mode*/
    bool mWriteAccess;

    /** Whether the file can potentially be opened in write mode (but not necessarily currently) */
    bool mWriteAccessPossible;

    /** Whether the open mode of the datasource changes w.r.t calls to enterUpdateMode() / leaveUpdateMode() */
    bool mDynamicWriteAccess;

    bool mShapefileMayBeCorrupted;

    /** Converts the geometry to the layer type if necessary. Takes ownership of the passed geometry */
    OGRGeometryH ConvertGeometryIfNecessary( OGRGeometryH );

    int mUpdateModeStackDepth;

    void computeCapabilities();

    int mCapabilities;

    bool doInitialActionsForEdition();
};


class QgsOgrProviderUtils
{
  public:
    static void setRelevantFields( OGRLayerH ogrLayer, int fieldCount, bool fetchGeometry, const QgsAttributeList &fetchAttributes, bool firstAttrIsFid );
    static OGRLayerH setSubsetString( OGRLayerH layer, OGRDataSourceH ds, QTextCodec* encoding, const QString& subsetString );
    static QByteArray quotedIdentifier( QByteArray field, const QString& ogrDriverName );

    /** Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant& value );

    static OGRDataSourceH OGROpenWrapper( const char* pszPath, bool bUpdate, OGRSFDriverH *phDriver );
};
