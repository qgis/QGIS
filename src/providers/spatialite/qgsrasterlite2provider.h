/***************************************************************************
     qgsrasterlite2provider.h
     --------------------------------------
    Date                 : 2017-08-29
    Copyright         : (C) 2017 by Mark Johnson, Berlin Germany
    Email                : mj10777 at googlemail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERLITE2PROVIDER_H
#define QGSRASTERLITE2PROVIDER_H

#include "qgsrasterdataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsspatialiteconnpool.h"

/**
  * \class QgsRasterLite2Provider
  * \brief Data provider for Spatialite RasterLite2 Raster layers.
  *  This provider implements the interface defined in the QgsDataProvider class
  *    to provide access to spatial data residing in a Spatialite enabled database.
  * \note
  *  Each Provider must be defined as an extra library in the CMakeLists.txt, when used as an extra library
  *  -> 'PROVIDER_KEY' and 'PROVIDER__DESCRIPTION' must be defined
  *  --> QGISEXTERN bool isProvider(), providerKey(),description() and Class Factory method must be defined
  * \since QGIS 3.0
 */
class QgsRasterLite2Provider : public QgsRasterDataProvider
{
    Q_OBJECT

  public:
    QgsRasterLite2Provider( const QString &uri );

    virtual ~ QgsRasterLite2Provider();

    void invalidateConnections( const QString &connection ) override;

    /**
     * Returns true if this is a valid layer. It is up to individual providers
     * to determine what constitutes a valid layer.
     * \note
     *  'mod_rasterlite2' must have been called
     * SpatialiteDbLayer must not be nullptr [possibly deleted]
     * \see isLayerValid
     * \since QGIS 3.0
     */
    bool isValid() const override { return isLayerValid(); }

    /**
     * Return a provider name
     *
     * Essentially just returns the provider key.  Should be used to build file
     * dialogs so that providers can be shown with their supported types. Thus
     * if more than one provider supports a given format, the user is able to
     * select a specific provider to open that file.
     * \since QGIS 3.0
     */
    QString name() const override;

    /**
     * Return description
     *
     * Return a terse string describing what the provider is.
     * \since QGIS 3.0
     */
    QString description() const override;

    static QgsRasterLite2Provider *createProvider( const QString &uri );

    /**
     * Returns the coordinate system for the data source.
     * If the provider isn't capable of returning its projection then an invalid
     * QgsCoordinateReferenceSystem will be returned.
     * \note
     *  readBlock parameter viewExtent will be that of this QgsCoordinateReferenceSystem
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem crs() const override { return mCrs; }

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     * \since QGIS 3.0
     */
    QStringList subLayers() const override { return mSubLayers; }

    /**
     * return the number of layers for the current data source
     * \since QGIS 3.0
     */
    uint subLayerCount() const override { return mSubLayers.size(); }

    /**
     * Sub-layer styles for each sub-layer handled by this provider,
     * in order from bottom to top
     *
     * Sub-layer styles are used to abstract the way the provider's source can symbolise
     * layers in some way at the server, before it serves them to the provider.
     * \since QGIS 3.0
     */
    QStringList subLayerStyles() const override;

    /**
     * Reorder the list of layer names to be rendered by this provider
     * (in order from bottom to top)
     * \note   layers must have been previously added.
     * \since QGIS 3.0
     */
    void setLayerOrder( const QStringList &layers ) override;

    /**
     * Set the visibility of the given sublayer name
     * \since QGIS 3.0
     */
    void setSubLayerVisibility( const QString &name, bool vis ) override;

    /**
     * Reloads the data from the source. Needs to be implemented by providers with data caches to
     * synchronize with changes in the data source
     * \since QGIS 3.0
     */
    void reloadData() override;

    /* Inherited from QgsRasterInterface */

    /**
     * Get number of bands
     *
     * \see mBandCount
     * \see setDbLayer
     * \see QgsRasterInterface::bandCount
     * \see SpatialiteDbLayer::getLayerNumBands
     * \since QGIS 3.0
     */
    int bandCount() const override { return mBandCount; }

    /**
     * RasterLite2 Raster-Layer tile height [QgsRasterInterface::xBlockSize]
     *  - based on // num_bands, tile_width, tile_height entries from raster_coverage table
     * \note
     *  - num_bands = mLayerBandsTileSize.x()
     *  - mLayerBandsTileSize.y() : not used
     *  - tile_width = mLayerBandsTileSize.width()
     *  - tile_height = mLayerBandsTileSize.height()
     * \see QgsRasterInterface::xBlockSize
     * \see SpatialiteDbLayer::getLayerTileWidth
     * \since QGIS 3.0
     */
    int xBlockSize() const override { return mXBlockSize; }

    /**
     * RasterLite2 Raster-Layer tile height [QgsRasterInterface::yBlockSize]
     *  - based on // num_bands, tile_width, tile_height entries from raster_coverage table
     * \note
     *  - num_bands = mLayerBandsTileSize.x()
     *  - mLayerBandsTileSize.y() : not used
     *  - tile_width = mLayerBandsTileSize.width()
     *  - tile_height = mLayerBandsTileSize.height()
     * \see QgsRasterInterface::yBlockSize
     * \see SpatialiteDbLayer::getLayerTileHeight
     * \since QGIS 3.0
     */
    int yBlockSize() const override { return mYBlockSize; }

    /**
     * RasterLite2 Raster-Layer Image Width [QgsRasterInterface::xSize() ]
     * \note
     *  - based on LayerExtent and Resolution from raster_coverage table
     * \see mWidth
     * \see setDbLayer
     * \see QgsRasterInterface::xSize
     * \see SpatialiteDbLayer::getLayerImageWidth
     * \since QGIS 3.0
     */
    int xSize() const override { return mWidth; }

    /**
     * RasterLite2 Raster-Layer Image Height [QgsRasterInterface::ySize() ]
     * \note
     *  - based on LayerExtent and Resolution from raster_coverage table
     * \see mHeight
     * \see setDbLayer
     * \see QgsRasterInterface::ySize
     * \see SpatialiteDbLayer::getLayerImageHeight
     * \since QGIS 3.0
     */
    int ySize() const override { return mHeight; }

    /**
     * Returns a bitmask containing the supported capabilities
     *
     * \since QGIS 3.0
     */
    int capabilities() const override;

    /* Inherited from QgsRasterDataProvider */

    /**
     * Return the extent for this data layer
     * \since QGIS 3.0
     */
    QgsRectangle extent() const override { return getLayerExtent(); }

    /**
     * \brief   Returns the caption error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     *
     * \since QGIS 3.0
     */
    QString lastErrorTitle() override { return mErrorTitle; }

    /**
     * \brief   Returns the verbose error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     *
     * \since QGIS 3.0
     */
    QString lastError() override { return mError; }
#if 1

    /**
     * Returns data type for the band specified by number
     * \since QGIS 3.0
     */
    Qgis::DataType dataType( int /*bandNo*/ ) const override { return getDbLayer()->getLayerRasterDataType(); }

    /**
     * Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType
     * \since QGIS 3.0
     */
    Qgis::DataType sourceDataType( int /*bandNo*/ ) const override { return getDbLayer()->getLayerRasterDataType(); }
#else

    /**
     * Returns data type for the band specified by number
     * \since QGIS 3.0
     */
    Qgis::DataType dataType( int /*bandNo*/ ) const override { return Qgis::ARGB32; }

    /**
     * Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType
     * \since QGIS 3.0
     */
    Qgis::DataType sourceDataType( int /*bandNo*/ ) const override { return Qgis::ARGB32; }
#endif

    /**
     * \brief helper function to create zero padded band names
     *  based on data-type and band number
     * \since QGIS 3.0
     */
    QString generateBandName( int bandNumber ) const override;

    /**
     * \brief Get band statistics.
     * \param bandNo The band (number).
     * \param stats Requested statistics
     * \param extent Extent used to calc statistics, if empty, whole raster extent is used.
     * \param sampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * \param feedback optional feedback object
     * \returns Band statistics.
     */
    bool hasStatistics( int bandNo,
                        int stats = QgsRasterBandStats::All,
                        const QgsRectangle &boundingBox = QgsRectangle(),
                        int sampleSize = 0 ) override;

    /**
     * \brief Get band statistics.
     * \param bandNo The band (number).
     * \param stats Requested statistics
     * \param extent Extent used to calc statistics, if empty, whole raster extent is used.
     * \param sampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * \param feedback optional feedback object
     * \returns Band statistics.
     */
    QgsRasterBandStats bandStatistics( int bandNo,
                                       int stats = QgsRasterBandStats::All,
                                       const QgsRectangle &boundingBox = QgsRectangle(),
                                       int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr ) override;

    /**
     * Clone itself, create deep copy
     * \since QGIS 3.0
     */
    QgsRasterInterface *clone() const override;

    /**
     * Get metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     * \since QGIS 3.0
     */
    QString metadata() override;

    /**
     * Returns whether the provider supplies a legend graphic
     * \since QGIS 3.0
     */
    bool supportsLegendGraphic() const override { return false; }

    /**
     * \brief Identify raster value(s) found on the point position. The context
     *         parameters extent, width and height are important to identify
     *         on the same zoom level as a displayed map and to do effective
     *         caching (WCS). If context params are not specified the highest
     *         resolution is used. capabilities() may be used to test if format
     *         is supported by provider. Values are set to 'no data' or empty string
     *         if point is outside data source extent.
     *
     * \note  The arbitraryness of the returned document is enforced by WMS standards
     *        up to at least v1.3.0
     * \param point coordinates in data source CRS
     * \param format result format
     * \param boundingBox context bounding box
     * \param width context width
     * \param height context height
     * \param dpi context dpi
     * \returns QgsRaster::IdentifyFormatValue: map of values for each band, keys are band numbers
     *         (from 1).
     *         QgsRaster::IdentifyFormatFeature: map of QgsRasterFeatureList for each sublayer
     *         (WMS) - TODO: it is not consistent with QgsRaster::IdentifyFormatValue.
     *         QgsRaster::IdentifyFormatHtml: map of HTML strings for each sublayer (WMS).
     *         Empty if failed or there are no results (TODO: better error reporting).
     */
    QgsRasterIdentifyResult identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &extent = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;

    /**
     * The class allowing to reuse the same sqlite handle for more layers
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    QgsSqliteHandle *getQSqliteHandle() const { return mQSqliteHandle; }

    /**
     * Retrieve SpatialiteDbInfo
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *getSpatialiteDbInfo() const { return mSpatialiteDbInfo; }

    /**
     * Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \since QGIS 3.0
     */
    bool isDbValid() const { if ( getSpatialiteDbInfo() ) return getSpatialiteDbInfo()->isDbValid(); else return false;}

    /**
     * Is the Layer valid
     * \note
     *  when false: the Layer should not be rendered
     * \see SpatialiteDbLayer::isLayerValid
     * \since QGIS 3.0
     */
    bool isLayerValid() const { if ( getDbLayer() ) return getDbLayer()->isLayerValid(); else return false;}

    /**
     * Is the read Database a Spatialite Database that contains a RasterLite2 Layer
     * - supported by QgsRasterLite2Provider
     * \note
     *  - RasterLite2 specific functions should not be called when false
     *  - For a Provider, this must always be active
     *  -> will be started through the calling this, if not already active
     * the Provider will fail if not active
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    bool isDbRasterLite2() const { return getSpatialiteDbInfo()->isDbRasterLite2(); }

    /**
     * Has 'mod_spatialite' or 'spatialite_init' been called for the QgsSpatialiteProvider and QgsRasterLite2Provider
     * \note
     *  - For a Provider, this must always be active
     *  -> will be started through the calling of dbHasSpatialite, if not already active
     * the Provider will fail if not active
     * \see setSqliteHandle
     * \see dbHasSpatialite
     * \see QgsSqliteHandle::isDbSpatialiteActive
     * \see SpatialiteDbInfo::isDbSpatialiteActive
     * \since QGIS 3.0
     */
    bool isDbSpatialiteActive() const { return getSpatialiteDbInfo()->isDbSpatialiteActive(); }

    /**
     * Has 'mod_rasterlite2' or 'rl2_init' been called for the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     *  - For a Provider, this must always be active
     *  -> will be started through the calling of dbHasSpatialite, if not already active
     * the Provider will fail if not active
    * \see QgsSqliteHandle::isDbSpatialiteActive
     * \since QGIS 3.0
     */
    bool isDbRasterLite2Active() const { return getSpatialiteDbInfo()->isDbRasterLite2Active(); }

    /**
     * If Spatialite/RasterLite2 can be used
     *  -  must be called before any Spatialite/Rasterlite2 functions are called
     * \note
     *   load Spatialite and RasterLite2-Drivers if not active
     * \returns true if Drivers are active
     * \see SpatialiteDbInfo::dbHasSpatialite
     * \since QGIS 3.0
     */
    bool dbHasSpatialite() const { return getSpatialiteDbInfo()->dbHasSpatialite( true, true ); }
    //!

    /**
     * The active Layer
     * - being read by the Provider
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *getDbLayer() const { return mDbLayer; }

    /**
     * The sqlite handler
     * - contained in the QgsSqliteHandle class being used by the layer
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    sqlite3 *dbSqliteHandle() const { return getSpatialiteDbInfo()->dbSqliteHandle(); }

    /**
     * The Database filename (with Path)
    * \returns mDatabaseFileName complete Path (without without symbolic links)
    * \since QGIS 3.0
    */
    QString getDatabaseFileName() const { return getSpatialiteDbInfo()->getDatabaseFileName(); }

    /**
     * The Spatialite internal Database structure being read
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see SpatialiteDbInfo::getSniffDatabaseType
    * \since QGIS 3.0
    */
    SpatialiteDbInfo::SpatialMetadata dbSpatialMetadata() const { return getSpatialiteDbInfo()->dbSpatialMetadata(); }

    /**
     * The Spatialite Version Driver being used
     * \note
     *  - returned from spatialite_version()
     * \see SpatialiteDbInfo::getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString dbSpatialiteVersionInfo() const { return getSpatialiteDbInfo()->dbSpatialiteVersionInfo(); }

    /**
     * The major Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see SpatialiteDbInfo::getSniffDatabaseType
     * \since QGIS 3.0
    */
    int dbSpatialiteVersionMajor() const { return getSpatialiteDbInfo()->dbSpatialiteVersionMajor(); }

    /**
     * The minor Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see SpatialiteDbInfo::getSniffDatabaseType
     * \since QGIS 3.0
    */
    int dbSpatialiteVersionMinor() const { return getSpatialiteDbInfo()->dbSpatialiteVersionMinor(); }

    /**
     * The revision Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see SpatialiteDbInfo::getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionRevision() const { return getSpatialiteDbInfo()->dbSpatialiteVersionRevision(); }

    /**
     * The Spatialite internal Database structure being read (as String)
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    *
    */
    QString dbSpatialMetadataString() const { return getSpatialiteDbInfo()->dbSpatialMetadataString(); }

    /**
     * The RasterLite2 Version Driver being used
     * \note
     *  - returned from RL2_Version()
     * \see SpatialiteDbInfo::dbHasRasterlite2
    * \since QGIS 3.0
    */
    QString dbRasterLite2VersionInfo() const { return getSpatialiteDbInfo()->dbRasterLite2VersionInfo(); }

    /**
     * The major RasterLite2 Version being used
     * \note
     *  - extracted from RL2_Version()
     * \see SpatialiteDbInfo::dbHasRasterlite2
     * \since QGIS 3.0
    */
    int dbRasterLite2VersionMajor() const { return getSpatialiteDbInfo()->dbRasterLite2VersionMajor(); }

    /**
     * The minor RasterLite2 Version being used
     * \note
     *  - extracted from RL2_Version()
     * \see SpatialiteDbInfo::dbHasRasterlite2
     * \since QGIS 3.0
    */
    int dbRasterLite2VersionMinor() const { return getSpatialiteDbInfo()->dbRasterLite2VersionMinor(); }

    /**
     * The revision RasterLite2 Version being used
     * \note
     *  - extracted from RL2_Version()
     * \see SpatialiteDbInfo::dbHasRasterlite2
    * \since QGIS 3.0
    */
    int dbRasterLite2VersionRevision() const { return getSpatialiteDbInfo()->dbRasterLite2VersionRevision(); }

    /**
     * Amount of RasterLite2 Raster-Coverages found in the Database
     * - from the raster_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 Raster-Coverages that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterCoveragesLayersCount() const { return getSpatialiteDbInfo()->dbRasterCoveragesLayersCount(); }

    /**
     * Layer-Name
     * \note
     *  Vector: Layer format: 'table_name(geometry_name)'
     *  Raster: Layer format: 'coverage_name'
     * \see SpatialiteDbLayer::getLayerName
     * \since QGIS 3.0
     */
    QString getLayerName() const { return mLayerName; }

    /**
     * Title of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite since 4.5.0: vector_coverages(title)
     *  - RasterLite2: raster_coverages(title)
     *  - GeoPackage: gpkg_contents(identifier)
     *  - MbTiles: metadata(name)
     *  - FdoOgr: none
     * \see SpatialiteDbLayer::getTitle
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QString getTitle() const { return mTitle; }

    /**
     * Abstract of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite since 4.5.0: vector_coverages(abstract)
     *  - RasterLite2: raster_coverages(abstract)
     *  - GeoPackage: gpkg_contents(description)
     *  - MbTiles: metadata(description)
     *  - FdoOgr: none
     * \see SpatialiteDbLayer::getAbstract
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QString getAbstract() const { return mAbstract; }

    /**
     * Copyright of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite since 4.5.0: vector_coverages(copyright)
     *  - RasterLite2: raster_coverages(copyright)
     *  - GeoPackage: none
     *  - MbTiles: none
     *  - FdoOgr: none
     * \see SpatialiteDbLayer::getCopyright
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QString getCopyright() const { return mCopyright; }

    /**
     * Srid of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite 4.0: vector_layers(srid) [geometry_columns(srid)]
     *  - RasterLite2: raster_coverages(srid)
     *  - GeoPackage: gpkg_contents(srs_id)
     *  - MbTiles: always 4326
     *  - FdoOgr: geometry_columns(srid)
     * \see SpatialiteDbLayer::getSrid
     * \since QGIS 3.0
     */
    int getSrid() const { return mSrid; }

    /**
     * The Srid of the Geometry or RasterLite2 Layer formatted as 'EPSG:nnnn'
     * \note
     *  - can be used to set the needed QgsCoordinateReferenceSystem
     * \see crs()
    * \since QGIS 3.0
    */
    QString getSridEpsg() const { return getDbLayer()->getSridEpsg(); }

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     * \note
     *  With UpdateLayerStatistics the Number of features will also be updated and retrieved
     * \param bUpdate force reading from Database
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see mLayerExtent
     * \see mNumberFeatures
     * \since QGIS 3.0
     */
    QgsRectangle getLayerExtent( bool bUpdate = false, bool bUpdateStatistics = false ) const { return getDbLayer()->getLayerExtent( bUpdate, bUpdateStatistics ); }

  protected:

    /**
     * Read block of data using give extent and size
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    void readBlock( int bandNo, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;

  private:

    /**
     * Returns true if this is a valid layer. It is up to individual providers
     * to determine what constitutes a valid layer.
     * \note
     * QGis must be compiled with RasterLite2 support (RASTERLITE2_VERSION_GE_*)
     * \since QGIS 3.0
     */
    bool mValid;

    /**
     * Name of the table with no schema
     * extracted from dataSourceUri()
     * \note
     * The Uri is based on the ogr version, which is NOT supported by QgsDataSourceUri
     * \since QGIS 3.0
     */
    QString mUriTableName;

    /**
     * DB full path
     * extracted from dataSourceUri()
     * \note
     * The Uri is based on the ogr version, which is NOT supported by QgsDataSourceUri
     * \since QGIS 3.0
     */
    QString mUriSqlitePath;

    /**
     * Returns the coordinate system for the data source.
     * If the provider isn't capable of returning its projection then an invalid
     * QgsCoordinateReferenceSystem will be returned.
     * created from value returned by getSridEpsg()
     * \see setDbLayer
     * \see SpatialiteDbLayer::getSridEpsg
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem mCrs;

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     * \see setLayerOrder
     * \see setSubLayerVisibility
     * \since QGIS 3.0
     */
    QStringList mSubLayers;

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     * \see setLayerOrder
     * \see setSubLayerVisibility
     * \since QGIS 3.0
     */
    QList<bool> mSubLayerVisibilities;

    /**
     * \brief   Returns the caption error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     *
     * \since QGIS 3.0
     */
    QString mErrorTitle;

    /**
     * \brief   Returns the verbose error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     *
     * \since QGIS 3.0
     */
    QString mError;

    /**
     * mCachedImage
     *
     * Not used
     * \since QGIS 3.0
     */
    QImage mCachedImage;

    /**
     * mCachedImage
     *
     * Not used
     * \since QGIS 3.0
     */
    QgsRectangle mCachedImageExtent;

    /**
     * The class allowing to reuse the same sqlite handle for more layers
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see getQSqliteHandle()
     * \since QGIS 3.0
     */
    QgsSqliteHandle *mQSqliteHandle = nullptr;

    /**
     * Sets the activeQgsSqliteHandle
     * - checking will be done to insure that the Database connected to is considered valid
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \see mDbLayer
     * \since QGIS 3.0
     */
    bool setSqliteHandle( QgsSqliteHandle *qSqliteHandle );

    /**
     * SpatialiteDbInfo Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;

    /**
     * Sets the active Layer
     * - checking will be done to insure that the Layer is considered valid
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \see mDbLayer
     * \since QGIS 3.0
     */
    bool setDbLayer( SpatialiteDbLayer *dbLayer );

    /**
     * The active Layer
     * - being read by the Provider
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \see setDbLayer
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *mDbLayer = nullptr;

    /**
     * Close the Database
     * - using QgsSqliteHandle [static]
     * \note
     * - if the connection is being shared and used elsewhere, the Database will not be closed
     * \see QgsSqliteHandle::closeDb
     * \since QGIS 3.0
     */
    void closeDb();

    /**
     * For each Band in a RasterLite2 Raster
     * - information about nodata, min/max pixel value
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 0: nodata_pixel as double
     * - position 1: pixel_min value as double
     * - position 2: pixel_max value as double
     * - position 3: Average/Mean value as double
     * - position 4: estimated Variance value as double
     * - position 5: estimated Standard Deviation value as double
     * - position 6: the total count of valid pixels (excluding NoData pixels) value as integer
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QStringList mLayerBandsInfo;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information from RL2_GetBandStatistics_Min
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 1: pixel_min value as integer
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, double> mLayerBandsNodata;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information from RL2_GetBandStatistics_Min
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 1: pixel_min value as integer
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, double> mLayerBandsPixelMin;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information from RL2_GetBandStatistics_Max
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 1: pixel_min value as integer
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, double> mLayerBandsPixelMax;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information from  RL2_GetBandStatistics_Avg
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 3: Average/Mean value as double
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, double> mLayerBandsPixelAverage;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information from RL2_GetBandStatistics_Var
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 4: estimated Variance value as double
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, double> mLayerBandsPixelVariance;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information from RL2_GetBandStatistics_StdDev
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 5: estimated Standard Deviation value as double
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, double> mLayerBandsPixelStandardDeviation;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information from RL2_GetRasterStatistics_ValidPixelsCount
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 6: the total count of valid pixels (excluding NoData pixels) value as integer
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, int> mLayerBandsPixelValidPixelsCount;

    /**
     * For each Band in a RasterLite2 Raster
     * - a PNG image representing the estimated distribution Histogram from a specific Band
     * \note
     *  - RL2_GetBandStatistics_Histogram
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, QImage> mLayerBandsHistograms;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information about nodata, min/max pixel value
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 0: nodata_pixel as double
     * - position 1: pixel_min value as double
     * - position 2: pixel_max value as double
     * - position 3: Average/Mean value as double
     * - position 4: estimated Variance value as double
     * - position 5: estimated Standard Deviation value as double
     * - position 6: the total count of valid pixels (excluding NoData pixels) value as integer
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \see QgsRasterDataProvider::mSrcNoDataValue
     * \since QGIS 3.0
     */
    void setLayerBandsInfo( QStringList layerBandsInfo, QMap<int, QImage> layerBandsHistograms );

    /**
     * Get metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     * \since QGIS 3.0
     */
    QString mMetadata;

    /**
     * Query and build Metadata string about properties of the Rasterlite2 layer
     * - for testing, but also for support of anly metadata output strings [metadata()]
     * \note
     *  - at the moment it is not clear if we can really return the real internal information
     *   about bands, no data etc, since readBlock returns a complete image
     * \see QgsRasterDataProvider::metadata
     * \since QGIS 3.0
     */
    int createLayerMetadata( int i_debug = 0 );

    /**
     * When getMapImageFromRasterLite2 is called during readBlock
     * - a default Background must be given
     * - The canvas background, from the User setting, will be used as default
     * \note
     * - The default Background only used when the image mime-type does not support transparity.
     * \see readBlock
     * \see setDbLayer
     * \since QGIS 3.0
     */
    QString mDefaultImageBackground;

    /**
     * The selected Style
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mDefaultRasterStyle;

    /**
     * ViewExtent of retrieved Image
     * \note
     * - load image if different
     * - fill data from requeded Band
     * \see readBlock
     * \see imageBands
     * \since QGIS 3.0
     */
    QgsRectangle mImageBandsViewExtent;

    /**
     * List of Image Data by Bands
     * - dependent on Image Type
     * \note
     * - 0=Alpha
     * - 1=Red
     * - 2=Green
     * - 3=Blue
     * \see imageBandsViewExtent
     * \see readBlock
     * \since QGIS 3.0
     */
    QList<QByteArray *> mImageBands;

    /**
     * RasterLite2 Raster-Layer Image Width [QgsRasterInterface::xSize() ]
     * \note
     *  - based on LayerExtent and Resolution from raster_coverage table
     * \see xSize()
     * \see setDbLayer
     * \see QgsRasterInterface::xSize
     * \see SpatialiteDbLayer::getLayerImageWidth
     * \since QGIS 3.0
     */
    int mWidth = 0;

    /**
     * RasterLite2 Raster-Layer Image Height [QgsRasterInterface::ySize() ]
     * \note
     *  - based on LayerExtent and Resolution from raster_coverage table
     * \see :ySize
     * \see setDbLayer
     * \see QgsRasterInterface::ySize
     * \see SpatialiteDbLayer::getLayerImageHeight
     * \since QGIS 3.0
     */
    int mHeight = 0;

    /**
     * RasterLite2 Raster-Layer tile height [QgsRasterInterface::xBlockSize]
     *  - based on // num_bands, tile_width, tile_height entries from raster_coverage table
     * \note
     *  - num_bands = mLayerBandsTileSize.x()
     *  - mLayerBandsTileSize.y() : not used
     *  - tile_width = mLayerBandsTileSize.width()
     *  - tile_height = mLayerBandsTileSize.height()
     * \see xBlockSize
     * \see setDbLayer
     * \see QgsRasterInterface::xBlockSize
     * \see SpatialiteDbLayer::getLayerTileWidth
     * \since QGIS 3.0
     */
    int mXBlockSize = 0;

    /**
     * RasterLite2 Raster-Layer tile height [QgsRasterInterface::yBlockSize]
     *  - based on // num_bands, tile_width, tile_height entries from raster_coverage table
     * \note
     *  - num_bands = mLayerBandsTileSize.x()
     *  - mLayerBandsTileSize.y() : not used
     *  - tile_width = mLayerBandsTileSize.width()
     *  - tile_height = mLayerBandsTileSize.height()
     * \see yBlockSize
     * \see setDbLayer
     * \see QgsRasterInterface::yBlockSize
     * \see SpatialiteDbLayer::getLayerTileHeight
     * \since QGIS 3.0
     */
    int mYBlockSize = 0;

    /**
     * Get number of bands
     *
     * \see bandCount
     * \see setDbLayer
     * \see QgsRasterInterface::bandCount
     * \see SpatialiteDbLayer::getLayerNumBands
     * \since QGIS 3.0
     */
    int mBandCount = 1;

    /**
     * Layer-Name
     * \note
     *  Vector: Layer format: 'table_name(geometry_name)'
     *  Raster: Layer format: 'coverage_name'
     * \see SpatialiteDbLayer::getLayerName
     * \since QGIS 3.0
     */
    QString mLayerName;

    /**
     * Title of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite since 4.5.0: vector_coverages(title)
     *  - RasterLite2: raster_coverages(title)
     *  - GeoPackage: gpkg_contents(identifier)
     *  - MbTiles: metadata(name)
     *  - FdoOgr: none
     * \see SpatialiteDbLayer::getTitle
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QString mTitle;

    /**
     * Abstract of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite since 4.5.0: vector_coverages(abstract)
     *  - RasterLite2: raster_coverages(abstract)
     *  - GeoPackage: gpkg_contents(description)
     *  - MbTiles: metadata(description)
     *  - FdoOgr: none
     * \see SpatialiteDbLayer::getAbstract
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QString mAbstract;

    /**
     * Copyright of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite since 4.5.0: vector_coverages(copyright)
     *  - RasterLite2: raster_coverages(copyright)
     *  - GeoPackage: none
     *  - MbTiles: none
     *  - FdoOgr: none
     * \see SpatialiteDbLayer::getCopyright
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QString mCopyright;

    /**
     * Srid of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite 4.0: vector_layers(srid) [geometry_columns(srid)]
     *  - RasterLite2: raster_coverages(srid)
     *  - GeoPackage: gpkg_contents(srs_id)
     *  - MbTiles: always 4326
     *  - FdoOgr: geometry_columns(srid)
     * \see SpatialiteDbLayer::getSrid
     * \since QGIS 3.0
     */
    int mSrid;

    /**
     * Is the read Database supported by QgsSpatiaLiteProvider or
     * -  by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles, RasterLite1
     * \since QGIS 3.0
     */
    bool mIsDbValid;

    /**
     * Is the Layer valid
     * \note
     *  when false: the Layer should not be rendered
     * \since QGIS 3.0
     */
    bool mIsLayerValid;

};

#endif // QGSRASTERLITE2PROVIDER_H
