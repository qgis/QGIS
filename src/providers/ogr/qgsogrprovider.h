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
#include "qgsvectorlayerimport.h"
#include "qgsogrproviderutils.h"

class QgsField;
class QgsVectorLayerImport;

class QgsOgrFeatureIterator;

#include <ogr_api.h>

/**
  \class QgsOgrProvider
  \brief Data provider for ESRI shapefiles
  */
class QgsOgrProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    //! Convert a vector layer to a vector file
    static QgsVectorLayerImport::ImportError createEmptyLayer(
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
    explicit QgsOgrProvider( QString const &uri = "" );

    virtual ~QgsOgrProvider();

    virtual QgsAbstractFeatureSource *featureSource() const override;

    virtual QgsCoordinateReferenceSystem crs() const override;

    /** Retrieves list of valid sublayers of the Dataset, using different Gdal versions (compile,runtime) pre 2.0 and after
     * Sub-layers handled by this provider, in order from bottom to top
     * \returns subLayers list of valid sublayers of the Dataset
     * \note
     * Terminolgy: Databases: 'table_name(geometry_column_name)' OGR: 'layer_name(feature_name)'
     * Only in cases where the table/layer has more than 1 geometry column/feature
     * - must the geometry_column/feature name be supplied
     * - in such cases retrieving a layer by name must be used, using the 'layer_name(feature_name)' syntax.
     * In cases where the table/layer has one 1 geometry column/feature
     * - must also retrieve a layer by name  using the 'layer_name' syntax.
     * \see QgsOgrProviderUtils::OGRGetLayerNameWrapper()
     * For non-Database-drivers, such as GML,KML
     * - duplicate layer-names can exist
     * - in such cases retrieving a layer by index must be used.
     * \see QgsOgrProviderUtils::OGRGetLayerIndexWrapper()
     * - Layer without geometries can exist, but will be ignored
     * - Layer with a defined geometry, but is empty (no rows) , will be loaded so that new geometries can be added
     *
     * Logic changes:
     * For non-Database-drivers, such as GML,KML
     * - duplicate layer-names can exist, therefore all layers must be listed to check againt duplicate Layer-Names
     *-- the skipping-logic, when searching for a specific layer has been removed.
     * \see OGRGetLayerWrapper()
     * For cases where the geometry type could not be reliably determined
     * - all the features of the Layer where checked until something was found
     * -- with Gdal 2.* (where more than 1 geometry can exist for a Layer),
     * -- also with KML where different types of geometries can exist
     * --> the first found setting would be taken, making a LINESTRING to become a POLYGON and thus causing confusion
     * - this has been replaced by calling the existing 'getOgrGeomType', which will determine the geometry type correctly
     * \see getOgrGeomType()
     * the promotion of CurvePolygons to Polygons, CompoundCurves to LineStrings and CircularStrings
     * - was not documented in a way that was understable and removed
     * -- if this is not being delt with correctly, should be added inside 'getOgrGeomType'
     * OGRGetLayerWrapper will now call this function during the first call in QgsOgrProvider::open
     * \see OGRGetLayerWrapper()
     * \see open()
     *
     */
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

    /** Extent OGR Layer
     * \note
     * For Layers that contain more than one Geometry
     * - OGR_L_GetExtentEx must be used with the value of mGeometryIndex
     * -> otherwise the extent of the first Geometry will be returned
     * - if mGeometryIndex < 0: OGR_L_GetExtent will be used
     * \see mGeometryIndex
     * \returns mExtentRect QgsRectangle
     */
    virtual QgsRectangle extent() const override;
    QVariant defaultValue( int fieldId ) const override;
    virtual void updateExtents() override;
    virtual bool addFeatures( QgsFeatureList &flist ) override;
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
    virtual bool enterUpdateMode() override;
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
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues, int limit = -1 ) const override;
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
    QString geometryName() const { return mGeometryName; }
    QString SubLayerString() const { return mSubLayerString; }

    QString filePath() const { return mFilePath; }

    int layerIndex() const { return mLayerIndex; }
    int geometryIndex() const { return mGeometryIndex; }

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
    void setRelevantFields( OGRLayerH ogrLayer, bool fetchGeometry, const QgsAttributeList &fetchAttributes );

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
    };

    /** Load OGR Layer
     * \param mode  OpenMode
     * \returns geomType OGRwkbGeometryType
     * \note
     *
     * Up to Gdal 2.* each geometry was 1 layer and could be retrieved using an index
     * Starting with Gdal 2.0, each table is one layer, that can contain more than 1 geometry
     *
     * OGRGetLayerWrapper will be used to determine how to open the Layer correctly
     * \see OGRGetLayerWrapper()
     *
     * If Layer has been successfully loaded, the used values will be stored in 'mSubLayerString'
     * - when called a second time, OGRGetLayerWrapper will avoid - otherwise needed, checks
     * \see mSubLayerString
     * \see QgsOgrProviderUtils::OGRGetLayerNameWrapper()
     * \see QgsOgrProviderUtils::OGRGetLayerIndexWrapper()
     */
    void open( OpenMode mode );
    void close();

  private:
    unsigned char *getGeometryPointer( OGRFeatureH fet );
    QString ogrWkbGeometryTypeName( OGRwkbGeometryType type ) const;

    //! Starts a transaction if possible and return true in that case
    bool startTransaction();

    //! Commits a transaction
    bool commitTransaction();

    QgsFields mAttributeFields;

    //! Map of field index to default value
    QMap<int, QString> mDefaultValues;

    bool mFirstFieldIsFid;
    OGRDataSourceH ogrDataSource;
    //! Driver name
    QString mDriverName;
    mutable OGREnvelope *mExtent;
    bool mForceRecomputeExtent;

    /** This member variable receives the same value as extent_
     in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak*/
    mutable QgsRectangle mExtentRect;
    OGRLayerH ogrLayer;
    OGRLayerH ogrOrigLayer;

    //! ogr get type
    QgsOgrProviderUtils::OgrGetType mOgrGetType;

    //! path to filename
    QString mFilePath;

    //! layer name
    QString mLayerName;
    //! geometry name
    QString mGeometryName;

    //! layer index
    int mLayerIndex;

    //! geometry field index as returned from OGR_FD_GetGeomFieldIndex
    int mGeometryIndex;

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

    /** SubLayerList entry that was actually used to open the layer
     * \note
     *
     * After OGRGetLayerWrapper has successfully loaded a Layer
     * an adapted version of mSubLayerList entry that was used to open the layer will be stored
     * \see OGRGetLayerWrapper()
     *
     * Sample of a duplicate Layer-name created by 'subLayers':
     * '3:Directions from 423, Taiwan, 台中市東勢區慶福里 to 423, Taiwan, Taichung City, Dongshi District, 中45鄉道:0:3:LineString25D:1'
     * '5:Directions from 423, Taiwan, 台中市東勢區慶福里 to 423, Taiwan, Taichung City, Dongshi District, 中45鄉道:0:3:LineString25D:1'
     * \see subLayers()
     *
     * When the User has selected '5', OGRGetLayerIndexWrapper will be called for Layer 5
     * \see QgsOgrProviderUtils::OGRGetLayerIndexWrapper()
     * - 'mSubLayerString' will contained the unchanged mSubLayerList entry of '5'
     *
     * Sample of a unique Layer-name created by 'subLayers' as the only layer:
     * When the User has selected '5', OGRGetLayerIndexWrapper will be called for Layer 5
     * '0:berlin_ortsteile_segmente:634:LineString'
     *
     * Since this is the only layer and the name has benn found in the list created by 'subLayers', OGRGetLayerNameWrapper will be called:
     * \see QgsOgrProviderUtils::OGRGetLayerNameWrapper()
     * - 'mSubLayerString' will contained the mSubLayerList entry, replacing '0' with '-1'
     * '-1:berlin_ortsteile_segmente:634:LineString:0'
     *
     * Sample of multiple unique Layer-names created by 'subLayers' as the only layer:
     * When the User has selected '3', OGRGetLayerNameWrapper will be called for Layer 3
     * '3:berlin_street_geometries(soldner_polygon):11751:MultiPolygon:0'
     *
     * OGRGetLayerIndexWrapper will be called for Layer 3
     * \see QgsOgrProviderUtils::OGRGetLayerIndexWrapper()
     * - 'mSubLayerString' will contained the unchanged mSubLayerList entry of '3'
     *
     * When compiled and running with Gdal 2.*, OGRGetLayerNameWrapper will be called:
     * \see QgsOgrProviderUtils::OGRGetLayerNameWrapper()
     * - 'mSubLayerString' will contained the mSubLayerList entry, replacing '3' with '-1'
     * '-1:berlin_street_geometries(soldner_polygon):11751:MultiPolygon:0'
     *
     * \since QGIS 3.0
    */
    QString mSubLayerString;

    // OGR Driver that was actually used to open the layer
    OGRSFDriverH ogrDriver;

    // Friendly name of the OGR Driver that was actually used to open the layer
    QString ogrDriverName;

    bool mValid;

    OGRwkbGeometryType mOGRGeomType;
    long mFeaturesCounted;

    mutable QStringList mSubLayerList;

    //! Adds one feature
    bool addFeature( QgsFeature &f );
    //! Deletes one feature
    bool deleteFeature( QgsFeatureId id );

    //! Calls OGR_L_SyncToDisk and recreates the spatial index if present
    bool syncToDisc();

    OGRLayerH setSubsetString( OGRLayerH layer, OGRDataSourceH ds );

    friend class QgsOgrFeatureSource;

    //! Whether the file is opened in write mode
    bool mWriteAccess;

    //! Whether the file can potentially be opened in write mode (but not necessarily currently)
    bool mWriteAccessPossible;

    //! Whether the open mode of the datasource changes w.r.t calls to enterUpdateMode() / leaveUpdateMode()
    bool mDynamicWriteAccess;

    bool mShapefileMayBeCorrupted;

    //! Converts the geometry to the layer type if necessary. Takes ownership of the passed geometry
    OGRGeometryH ConvertGeometryIfNecessary( OGRGeometryH );

    int mUpdateModeStackDepth;

    void computeCapabilities();

    QgsVectorDataProvider::Capabilities mCapabilities;

    bool doInitialActionsForEdition();

    /** Retrieves the requested Layer, using different Gdal versions (compile,runtime) pre 2.0 and after
     * \param ogrDataSource Data Source
     * \param mLayerName name of Layer
     * \param mLayerIndex index of Layer
     * \returns OGRLayerH if found
     * \note
     *
     * QgsOgrProvider should always use this function to open a layer
     * this will be called 2 times during QgsOgrProvider::open()
     * \see open()
     *
     * If this is being called for the first time
     * 'subLayers' will be called to determinee which Layers exist for this Dataset
     * \see subLayers()
     *
     * If the given Layer-Name is found and is unique, OGRGetLayerNameWrapper will be called
     * - if the loading failed, an attempt will be made to load the layer with OGRGetLayerIndexWrapper
     * \see QgsOgrProviderUtils::OGRGetLayerNameWrapper()
     *
     * If the given Layer-Name is found and is not unique, OGRGetLayerIndexWrapper will be called
     * - checking is done that the given index is valid, if not will be replaced by the value returned by 'subLayers'
     * -- this may happen if the id is stored in a project and the Layer has been externaly changed. OGR may return a different id.
     * \see QgsOgrProviderUtils::OGRGetLayerIndexWrapper()
     *
     * If Layer has been successfully loaded, the used values will be stored in 'mSubLayerString'
     * - if 'OGRGetLayerNameWrapper' was used to successfully load the Layer, the index value in 'mSubLayerString' will be set to '-1'
     * \see mSubLayerString
     *
     * After OGRGetLayerWrapper, setDataSourceUri will store the values found by OGR for the Layer-Name and Index
     * - which is read by 'QgsVectorLayer' and used in the project file
     * \see QgsOgrProvider::setSubsetString
     * \see QgsVectorLayer::setDataProvider
     *
     * If this is being called after the initial calling
     * - 'mSubLayerString' will be read: field 0=layer-id, field 1: layer-name
     * \see QgsOgrProviderUtils::OGRGetSubLayerStringWrapper
     *
     * Since this function cannot be called from another class (QgsOgrProvider being 'const')
     * - 'mSubLayerString' should be read to determine which QgsOgrProviderUtils function should be called.
     * \see QgsOgrFeatureIterator::QgsOgrFeatureIterator
     * \since QGIS 3.0
     */
    OGRLayerH OGRGetLayerWrapper( OGRDataSourceH ogrDataSource, QString sLayerName, QString sGeometryName, long lLayerIndex );
};

#endif // QGSOGRPROVIDER_H
