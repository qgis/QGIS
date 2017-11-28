/***************************************************************************
            qgsspatialiteprovider.h Data provider for SpatiaLite DBMS
begin                : Dec 2008
copyright            : (C) 2008 Sandro Furieri
email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALITEPROVIDER_H
#define QGSSPATIALITEPROVIDER_H

#include "qgsdatasourceuri.h"
#include "qgsspatialiteconnpool.h"

#include "qgsvectordataprovider.h"
#include "qgsvectorlayerexporter.h"
#include "qgsfields.h"
#include "qgswkbtypes.h"

#include <list>
#include <queue>
#include <fstream>
#include <set>

struct sqlite3;

class QgsFeature;
class QgsField;

class QgsSqliteHandle;
class QgsSpatiaLiteFeatureIterator;

/**
  * \class QgsSpatiaLiteProvider
  * \brief Data provider for SQLite/SpatiaLite Vector layers.
  *  This provider implements the interface defined in the QgsDataProvider class
  *    to provide access to spatial data residing in a Spatialite enabled database.
  * \note
  *  Each Provider must be defined as an extra library in the CMakeLists.txt
  *  -> 'PROVIDER_KEY' and 'PROVIDER__DESCRIPTION' must be defined
  *  --> QGISEXTERN bool isProvider(), providerKey(),description() and Class Factory method must be defined
  *
 */
class QgsSpatiaLiteProvider: public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    /**
     * Import a vector layer into the database
     * \param uri  uniform resource locator (URI) for a dataset
     */
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
    explicit QgsSpatiaLiteProvider( QString const &uri = "" );

    virtual ~ QgsSpatiaLiteProvider();

    virtual QgsAbstractFeatureSource *featureSource() const override;
    virtual QString storageType() const override;
    virtual QgsCoordinateReferenceSystem crs() const override;
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    virtual QString subsetString() const override;
    virtual bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    virtual bool supportsSubsetString() const override { return true; }

    /**
     * Return the feature type
         *  \returns getGeometryType()
         * \see getGeometryType
         * \since QGIS 3.0
     */
    QgsWkbTypes::Type wkbType() const override;

    /**
     * Return the number of layers for the current data source
     *
     * \note Should this be subLayerCount() instead?
     *  \returns getNumberFeatures( true )
     */
    size_t layerCount() const;

    /**
     * Amount of features
     *  - implementation of Provider function 'featureCount'
     * \note
     *  - implemented in SpatialiteDbLayer
     *  \returns amount of features of the Layer
     * \see getNumberFeatures
     * \since QGIS 3.0
     */
    long featureCount() const override;

    /**
     * Extent of the Layer
     *  - implementation of Provider function 'extent'
     * \note
     *  - implemented in SpatialiteDbLayer
     *  \returns QgsRectangle extent of the Layer
     *  - getLayerExtent is called retrieving the extent without calling UpdateLayerStatistics
     * \see getLayerExtent
     * \since QGIS 3.0
     */
    virtual QgsRectangle extent() const override;

    /**
     * Extent of the Layer
     *  - implementation of Provider function 'updateExtents'
     * \note
     *  - implemented in SpatialiteDbLayer
     *  - getLayerExtent is called retrieving the extent after a UpdateLayerStatistics
     * \see getLayerExtent
     * \since QGIS 3.0
     */
    virtual void updateExtents() override;

    /**
     * List of fields
     *  - implementation of Provider function 'fields'
     * \note
     *  - implemented in SpatialiteDbLayer
     *  \returns QgsFields of the Layer
     * \see getAttributeFields
     * \since QGIS 3.0
     */
    QgsFields fields() const override;

    /**
     * Returns the minimum value of an attribute
     *  - implementation of Provider function 'minimumValue'
     * \note
     *  - must remain in QgsSpatiaLiteProvider, since the results (mQuery, mSubsetString) could effect Layers that are being used elsewhere
     * \param field number of attribute to rfetrieve the value from
     * \returns result as a QVariant
     * \see QgsVectorDataProvider::convertValue
     * \see mQuery
     * \see mSubsetString
     * \since QGIS 3.0
     */
    QVariant minimumValue( int index ) const override;

    /**
     * Returns the maximum value of an attribute
     *  - implementation of Provider function 'minimumValue'
     * \note
     *  - must remain in QgsSpatiaLiteProvider, since the results (mQuery, mSubsetString) could effect Layers that are being used elsewhere
     * \param field number of attribute to rfetrieve the value from
     * \returns result as a QVariant
     * \see QgsVectorDataProvider::convertValue
     * \see mQuery
     * \see mSubsetString
     * \since QGIS 3.0
     */
    QVariant maximumValue( int index ) const override;

    /**
     * Returns a list of unique values
     *  - implementation of Provider function 'uniqueValues'
     * \note
     *  - must remain in QgsSpatiaLiteProvider, since the results (mQuery, mSubsetString) could effect Layers that are being used elsewhere
     * \param field number of attribute to rfetrieve the value from
     * \returns result as a QSet of QVariant
     * \see mQuery
     * \see mSubsetString
     * \since QGIS 3.0
     */
    virtual QSet<QVariant> uniqueValues( int index, int limit = -1 ) const override;

    /**
     * UniqueStringsMatching
     * \note
     *  - must remain in QgsSpatiaLiteProvider, since the results (mQuery, mSubsetString) could effect Layers that are being used elsewhere
     * \see mQuery
     * \see mSubsetString
    * \since QGIS 3.0
    */
    virtual QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
        QgsFeedback *feedback = nullptr ) const override;

    bool isValid() const override;
    virtual bool isSaveAndLoadStyleToDatabaseSupported() const override { return true; }
<<<<<<< HEAD
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = nullptr ) override;
=======

    /**
     * Adds features
     *  - implementation of Provider function 'addFeatures'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param flist feature to add
     *  \returns True in case of success and False in case of error or container not supported
     * \see SpatialiteDbLayer::addLayerFeatures
     * \since QGIS 3.0
     */
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = 0 ) override;

    /**
     * Deletes features
     *  - implementation of Provider function 'deleteFeatures'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param id QgsFeatureIds to delete
     *  \returns True in case of success and False in case of error or container not supported
     * \see SpatialiteDbLayer::deleteLayerFeatures
     * \since QGIS 3.0
     */
>>>>>>> Refactured version based master from 2017-11-17. Minimal Spatialite connection with load_extension. RasterLite2 without source dependency. Full QgsLayerItem and QgsLayerMetadata support.
    bool deleteFeatures( const QgsFeatureIds &id ) override;

    /**
     * Deletes all records of Layer-Table
     *  - implementation of Provider function 'truncate'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param id QgsFeatureIds to delete
     * \returns True in case of success and False in case of error or container not supported
     * \see getNumberFeatures
     * \see SpatialiteDbLayer::truncateLayerTableRows
     * \since QGIS 3.0
     */
    bool truncate() override;

    /**
     * Adds attributes
     *  - implementation of Provider function 'addAttributes'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param attributes List of QgsField
     *  \returns True in case of success and False in case of error
     * \see SpatialiteDbLayer::addLayerAttributes
     * \since QGIS 3.0
     */
    bool addAttributes( const QList<QgsField> &attributes ) override;

    /**
     * Updates attributes
     *  - implementation of Provider function 'changeAttributeValues'
     * \note
     *  - Spatialite specific [GeoPackage could be implemented, but not forseen]
     * \param attr_map collection of attributes to change
     *   \returns True in case of success and False in case of error
     * \see SpatialiteDbLayer::changeAttributeValues
     * \since QGIS 3.0
     */
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

    /**
     * Updates features
     *  - implementation of Provider function 'changeGeometryValues'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param geometry_map collection of geometries to change
     *  \returns True in case of success and False in case of error
     * \see SpatialiteDbLayer::changeLayerGeometryValues
     * \since QGIS 3.0
     */
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    /**
     * Based on Layer-Type, set QgsVectorDataProvider::Capabilities
     *  - implementation of Provider function 'capabilities'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \see getCapabilities
     * \since QGIS 3.0
    */
    QgsVectorDataProvider::Capabilities capabilities() const override;

    /**
     * Based on Layer-Type, set QgsVectorDataProvider::Capabilities
     *  - implementation of Provider function 'defaultValue'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \see getDefaultValues()
     * \returns value from Map of field index of the default values
     * \since QGIS 3.0
    */
    QVariant defaultValue( int fieldId ) const override;
<<<<<<< HEAD
    virtual bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const override;
=======

    /**
     * Creates attributes Index
     *  - implementation of Provider function 'createAttributeIndex'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param field number of attribute to created the index for
     * \returns True in case of success and False in case of error
     * \see QgsSpatiaLiteProvider::createLayerAttributeIndex
     * \since QGIS 3.0
     */
>>>>>>> Refactured version based master from 2017-11-17. Minimal Spatialite connection with load_extension. RasterLite2 without source dependency. Full QgsLayerItem and QgsLayerMetadata support.
    bool createAttributeIndex( int field ) override;

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
     * Loaded Layers-Counter
     * - contained in mDbLayers
     * -- all details of the Layer are known
     * \note
     * - only when GetSpatialiteLayerInfoWrapper is called with LoadLayers=true
     * -- will all the Layers be loaded
     * \see getSpatialiteLayerInfo
     * \since QGIS 3.0
     */
    int dbLoadedLayersCount() const { return getSpatialiteDbInfo()->dbLoadedLayersCount(); }

    /**
     * Amount of SpatialTables  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialTables that have been loaded
     * \see SpatialiteDbInfo::dbSpatialTablesLayersCount
     * \since QGIS 3.0
     */
    int dbSpatialTablesLayersCount() const { return getSpatialiteDbInfo()->dbSpatialTablesLayersCount(); }

    /**
     * Amount of SpatialViews  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialViews that have been loaded
     * \see SpatialiteDbInfo::dbSpatialViewsLayersCount
     * \since QGIS 3.0
     */
    int dbSpatialViewsLayersCount() const { return getSpatialiteDbInfo()->dbSpatialViewsLayersCount(); }

    /**
     * Amount of VirtualShapes found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of VirtualShapes that have been loaded
     * \see SpatialiteDbInfo::dbVirtualShapesLayersCount
     * \since QGIS 3.0
     */
    int dbVirtualShapesLayersCount() const { return getSpatialiteDbInfo()->dbVirtualShapesLayersCount(); }

    /**
     * Amount of RasterLite1-Rasters found in the Database
     * - only the count of valid Layers are returned
     * \note
     * - the Gdal-RasterLite1-Driver is needed to Determineeee this
     * - this does not reflect the amount of RasterLite1-Rasters that have been loaded
     * \see SpatialiteDbInfo::dbRasterLite1LayersCount
     * \since QGIS 3.0
     */
    int dbRasterLite1LayersCount() const { return getSpatialiteDbInfo()->dbRasterLite1LayersCount(); }

    /**
     * Amount of RasterLite2 Vector-Coverages found in the Database
     * - from the vector_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 Vector-Coverages that have been loaded
     * \see SpatialiteDbInfo::dbVectorCoveragesLayersCount
     * \since QGIS 3.0
     */
    int dbVectorCoveragesLayersCount() const { return getSpatialiteDbInfo()->dbVectorCoveragesLayersCount(); }

    /**
     * Amount of RasterLite2 Raster-Coverages found in the Database
     * - from the raster_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 Raster-Coverages that have been loaded
     * \see SpatialiteDbInfo::dbRasterCoveragesLayersCount
     * \since QGIS 3.0
     */
    int dbRasterCoveragesLayersCount() const { return getSpatialiteDbInfo()->dbRasterCoveragesLayersCount(); }

    /**
     * Amount of Topologies found in the Database
     * - from the topologies table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of Topology-Layers that have been loaded
     * \see SpatialiteDbInfo::dbTopologyExportLayersCount
     * \since QGIS 3.0
     */
    int dbTopologyExportLayersCount() const { return getSpatialiteDbInfo()->dbTopologyExportLayersCount(); }

    /**
     * Has the used Spatialite compiled with Spatialite-Gcp support
     * - ./configure --enable-gcp=yes
     * \note
     *  - Based on GRASS GIS was initially released under the GPLv2+ license terms.
     * \returns true if GCP_* Sql-Functions are supported
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=Ground+Control+Points
     * \see SpatialiteDbInfo::hasDbGcpSupport
     * \since QGIS 3.0
     */
    bool hasDbGcpSupport() const { return getSpatialiteDbInfo()->hasDbGcpSupport(); }

    /**
     * Has the used Spatialite compiled with Topology (and thus RtTopo) support
     * - ./configure --enable-rttopo
     * \note
     *  - Based on RT Topology Library
     * \returns true if Topology Sql-Functions are supported
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=ISO+Topology
     * \see https://git.osgeo.org/gogs/rttopo/librttopo
     * \see SpatialiteDbInfo::hasDbTopologySupport
     * \since QGIS 3.0
     */
    bool hasDbTopologySupport() const { return getSpatialiteDbInfo()->hasDbTopologySupport(); }

    /**
     * Is the used connection Spatialite 4.5.0 or greater
     * \note
     *  - bsed on values offrom spatialite_version()
     * \see SpatialiteDbInfo::isDbVersion45
    * \since QGIS 3.0
    */
    bool isDbVersion45() const { return getSpatialiteDbInfo()-> isDbVersion45(); }

    /**
     * Loaded Layers-Counter
     * - contained in mDbLayers
     * \note
     * - only when GetSpatialiteDbInfoWrapper is called with LoadLayers=true
     * -- will all the Layers be loaded
     * \see SpatialiteDbInfo::dbLayersCount
     * \since QGIS 3.0
     */
    int dbLayersCount() const { return getSpatialiteDbInfo()->dbLayersCount(); }

    /**
     * Amount of Vector-Layers found
     * - SpatialTables, SpatialViews and virtualShapes [from the vector_layers View]
     * \note
     * - this amount may differ from dbLayersCount()
     * -- which only returns the amount of Loaded-Vector-Layers
     * \see SpatialiteDbInfo::dbVectorLayersCount
     * \since QGIS 3.0
     */
    int dbVectorLayersCount() const { return getSpatialiteDbInfo()->dbVectorLayersCount(); }

    /**
     * Flag indicating if the layer data source (Sqkite3) has ReadOnly restrictions
     * \note
     *  Uses sqlite3_db_readonly( mSqliteHandle, "main" )
     * \returns result of sqlite3_db_readonly( mSqliteHandle, "main" )
     * \see SpatialiteDbInfo::isDbReadOnly
    * \since QGIS 3.0
    */
    bool isDbReadOnly() const { return getSpatialiteDbInfo()->isDbReadOnly(); }

    /**
     * Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \see SpatialiteDbInfo::isDbValid
     * \since QGIS 3.0
     */
    bool isDbValid() const { if ( getSpatialiteDbInfo() ) return getSpatialiteDbInfo()->isDbValid(); else return false;}

    /**
     * Is the read Database a Spatialite Database
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \see SpatialiteDbInfo::isDbSpatialite
     * \since QGIS 3.0
     */
    bool isDbSpatialite() const { return getSpatialiteDbInfo()->isDbSpatialite(); }

    /**
     * Has 'mod_spatialite' or 'spatialite_init' been called for the QgsSpatialiteProvider and QgsRasterLite2Provider
     * \note
     *  - For a Provider, this must always be active
     *  -> will be started through the calling of dbHasSpatialite, if not already active
     * the Provider will fail if not atice
     * \see setSqliteHandle
     * \see dbHasSpatialite
     * \see QgsSqliteHandle::isDbSpatialiteActive
     * \see SpatialiteDbInfo::isDbSpatialiteActive
     * \since QGIS 3.0
     */
    bool isDbSpatialiteActive() const { return getSpatialiteDbInfo()->isDbSpatialiteActive(); }

    /**
     * If Spatialite can be used
     *  -  must be called before any Spatialite functions are called
     * \note
     *   load Spatialite-Driver if not active
     * \returns true if Driver is active
     * \see SpatialiteDbInfo::dbHasSpatialite
     * \since QGIS 3.0
     */
    bool dbHasSpatialite() const { return getSpatialiteDbInfo()->dbHasSpatialite( true ); }

    /**
     * The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool isDbGdalOgr() const { return getSpatialiteDbInfo()->isDbGdalOgr(); }

    /**
     * The active Layer
     * - being read by the Provider
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \see setDbLayer
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *getDbLayer() const  { return mDbLayer; }

    /**
     * The sqlite handler
     * - contained in the QgsSqliteHandle class being used by the layer
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid
     * \since QGIS 3.0
     */
    sqlite3 *dbSqliteHandle() const { return getSpatialiteDbInfo()->dbSqliteHandle(); }

    /**
     * Name of the table/view with no schema
     * \note
     *  For SpatialViews
     *  - the regiestered TABLE can be retrieved with getViewTableName()
     * \see SpatialiteDbLayer::getTableName
     * \see SpatialiteDbLayer::getViewTableName
     * \since QGIS 3.0
     */
    QString getTableName() const { return mTableName; }

    /**
     * Name of the table which contains the SpatialView-Geometry (underlining table)
     * \note
     *  For SpatialTable/VirtualShapes: empty
     *  For SpatialViews:
     *  - the registered VIEW can be retrieved with getTableName()
     * \see SpatialiteDbLayer::getViewTableName
     * \since QGIS 3.0
     */
    QString getViewTableName() const { return mViewTableName; }

    /**
     * Return the Table-Name to use to build the IndexTable-Name
     * \note
     *  For SpatialTable/VirtualShapes: result of getTableName will be used
     *  For SpatialViews: result of getViewTableName will be used
     * \see SpatialiteDbLayer::getTableName
     * \see SpatialiteDbLayer::getViewTableName
     * \see QgsSpatiaLiteFeatureSource::mIndexTable
     * \since QGIS 3.0
     */
    QString getIndexTable() const { return mIndexTable; }

    /**
     * Name of the geometry column in the table
     * \note
     *  Vector: should always be filled
     *  Raster: may be empty
     * \see SpatialiteDbLayer::getGeometryColumn
     * \since QGIS 3.0
     */
    QString getGeometryColumn() const { return mGeometryColumn; }

    /**
     * Name of the table-geometry which contains the SpatialView-Geometry (underlining table)
     * \note
     *  For SpatialTable/VirtualShapes: empty
     *  For SpatialViews:
     *  - the registered VIEW can be retrieved with getTableName()
     * \see SpatialiteDbLayer::getViewTableGeometryColumn
     * \since QGIS 3.0
     */
    QString getViewTableGeometryColumn() const { return mViewTableGeometryColumn; }

    /**
     * Return the Geometry-Name to use to build the IndexTable-Name
     * \note
     *  For SpatialTable/VirtualShapes: result of getGeometryColumn will be used
     *  For SpatialViews: result of getViewTableGeometryColumn will be used
     * \see SpatialiteDbLayer::getGeometryColumn
     * \see SpatialiteDbLayer::getViewTableGeometryColumn
     * \see QgsSpatiaLiteFeatureSource::mIndexGeometry
     * \since QGIS 3.0
     */
    QString getIndexGeometry() const { return mIndexGeometry; }

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
     * The AuthId [auth_name||':'||auth_srid]
     * \see SpatialiteDbLayer::getAuthId
     * \since QGIS 3.0
     */
    QString getAuthId() const { return mAuthId; }

    /**
     * The Proj4text [from mSrid]
     * \see SpatialiteDbLayer::getProj4text
     * \since QGIS 3.0
     */
    QString getProj4text() const { return mProj4text; }

    /**
     * The SpatialIndex-Type used for the Geometry
     * \note
     *  Uses the same numbering as Spatialite [0,1,2]
     * \see SpatialiteDbLayer::getSpatialIndexType
     * \since QGIS 3.0
     */
    int getSpatialIndexType() const { return mSpatialIndexType; }

    /**
     * The Spatialite Layer-Type of the Layer
     * - representing mLayerType
     * \see SpatialiteDbLayer::getLayerType
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialiteLayerType getLayerType() const { return mLayerType; }

    /**
     * The Spatialite Geometry-Type of the Layer
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     * \see SpatialiteDbLayer::getGeometryType
     * \since QGIS 3.0
     */
    QgsWkbTypes::Type getGeometryType() const { return mGeometryType; }

    /**
     * The Spatialite Geometry-Type of the Layer (as String)
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     * \see SpatialiteDbLayer::getGeometryType
     * \since QGIS 3.0
     */
    QString getGeometryTypeString() const { return mGeometryTypeString; }

    /**
     * The Spatialite Coord-Dimensions of the Layer
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     * \see SpatialiteDbLayer::getCoordDimensions
     * \since QGIS 3.0
     */
    int getCoordDimensions() const { return getDbLayer()->getCoordDimensions(); }

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     * \note
     *  With UpdateLayerStatistics the Number of features will also be updated and retrieved
     * \param bUpdate force reading from Database
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see SpatialiteDbLayer::getLayerExtent
     * \see SpatialiteDbLayer::getNumberFeatures
     * \since QGIS 3.0
     */
    QgsRectangle getLayerExtent( bool bUpdate = false, bool bUpdateStatistics = false ) const { return getDbLayer()->getLayerExtent( bUpdate, bUpdateStatistics ); }

    /**
     * Number of features in the layer
     * \note
     *  With UpdateLayerStatistics the Extent will also be updated and retrieved
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see getLayerExtent
     * \see getNumberFeatures
     * \since QGIS 3.0
     */
    long getNumberFeatures( bool bUpdateStatistics = false ) const { return getDbLayer()->getNumberFeatures( bUpdateStatistics ); }

    /**
     * The Spatialite Layer-Readonly status [true or false]
     * \note
     *  SpatialTable: always false [writable]
     *  SpatialView: default: true [readonly]
     *  -  if Insert/Update or Delete TRIGGERs exists:  false [writable]
     *  VirtualShape: always true [readonly]
     * \see SpatialiteDbLayer::isLayerReadOnly
     * \since QGIS 3.0
     */
    int isLayerReadOnly() const { return getDbLayer()->isLayerReadOnly(); }

    /**
     * The Spatialite Layer-Hidden status [true or false]
     * \note
     *  No idea what this is used for [at the moment not being retrieved]
     *  - from 'vector_layers_auth'
     * \see SpatialiteDbLayer::isLayerHidden
     * \since QGIS 3.0
     */
    int isLayerHidden() const { return getDbLayer()->isLayerHidden(); }

    /**
     * The Spatialite Layer-Id being created
     * \note
     *  Only a simple counter when inserting into mDbLayers
     * - value is never used
     * \see SpatialiteDbLayer::getLayerId
     * \since QGIS 3.0
     */
    int getLayerId() const { return getDbLayer()->getLayerId(); }

    /**
     * Based on Layer-Type, set QgsVectorDataProvider::Capabilities
     * - Writable Spatialview: based on found TRIGGERs
     * \note
     * - this should be called after the LayerType and PrimaryKeys have been set
     * \note
     * The following receive: QgsVectorDataProvider::NoCapabilities
     * - SpatialiteTopology: will serve only TopopogyLayer, which ate SpatialTables
     * - VectorStyle: nothing as yet
     * - RasterStyle: nothing as yet
     * \note
     * - this should be called with Update, after alterations of the TABLE have been made
     * -> will call GetDbLayersInfo to re-read field data
     * \param bUpdate force reading from Database
     * \see SpatialiteDbLayer::GetDbLayersInfo
     * \since QGIS 3.0
     */
    QgsVectorDataProvider::Capabilities getCapabilities( bool bUpdate = false ) const { return getDbLayer()->getCapabilities( bUpdate ); }

    /**
     * Name of the primary key column in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see SpatialiteDbLayer::getPrimaryKey
     * \since QGIS 3.0
     */
    QString getPrimaryKey() const { return mPrimaryKey; }

    /**
     * Column-Number of the primary key
     * \note
     *  - SpatialView: name from entry of view_rowid of views_geometry_columns
     *  -> position from inside the PRAGMA table_info('name')
     * \see SpatialiteDbLayer::getPrimaryKeyCId
     * \since QGIS 3.0
     */
    int getPrimaryKeyCId() const { return mPrimaryKeyCId; }

    /**
     * List of primary key columns in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see SpatialiteDbLayer::getPrimaryKeyAttrs
     * \since QGIS 3.0
     */
    QgsAttributeList getPrimaryKeyAttrs() const { return mPrimaryKeyAttrs; }

    /**
     * List of layer fields in the table
     * \note
     *  - from PRAGMA table_info()
     * \see SpatialiteDbLayer::getAttributeFields
     * \since QGIS 3.0
     */
    QgsFields getAttributeFields() const { return mAttributeFields; }

    /**
     * Map of field index to default value [for Topology, the Topology-Layers]
     * List of default values of the table and possible the view
     * \note
     *  - from PRAGMA table_info()
     * \see SpatialiteDbLayer::getDefaultValues
     * \since QGIS 3.0
     */
    QMap<int, QVariant> getDefaultValues() const { return getDbLayer()->getDefaultValues(); }

    /**
     * Connection info (DB-path) without table and geometry
     * - this will be called from the SpatialiteDbLayer::getDatabaseUri()
     * \note
     *  - to call for Database and Table/Geometry portion use: SpatialiteDbLayer::getDatabaseUri()
    * \returns uri with Database only
     * \see SpatialiteDbInfo::getDatabaseUri
    * \since QGIS 3.0
    */
    QString getDatabaseUri() const { return getSpatialiteDbInfo()->getDatabaseUri(); }

    /**
     * Connection info (DB-path) with table and geometry
     * \note
     *  - to call for Database portion only, use: SpatialiteDbInfo::getDatabaseUri()
     *  - For RasterLite1: GDAL-Syntax will be used
    * \returns uri with Database and Table/Geometry Information
     * \see SpatialiteDbLayer::getLayerDataSourceUri
    * \since QGIS 3.0
    */
    QString getLayerDataSourceUri() const { return getDbLayer()->getLayerDataSourceUri(); }

    /**
     * Is the Layer valid
     * \note
     *  when false: the Layer should not be rendered
     * \see SpatialiteDbLayer::isLayerValid
     * \since QGIS 3.0
     */
    bool isLayerValid() const { if ( getDbLayer() ) return getDbLayer()->isLayerValid(); else return false;}

    /**
     * Is the Layer
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  when false: the Layer should not be rendered
     * \see SpatialiteDbLayer::isLayerSpatialite
     * \since QGIS 3.0
     */
    bool isLayerSpatialite() const { if ( getDbLayer() ) return getDbLayer()->isLayerSpatialite(); else return false;}

    /**
     * The SpatiaLite provider does its own transforms so we return
     * true for the following three functions to indicate that transforms
     * should not be handled by the QgsCoordinateTransform object. See the
     * documentation on QgsVectorDataProvider for details on these functions.
     */
    // XXX For now we have disabled native transforms in the SpatiaLite
    //   (following the PostgreSQL provider example)
    bool supportsNativeTransform()
    {
      return false;
    }

    /**
     * Return name contained in SPATIALITE_KEY
     *
     * Return a terse string describing what the provider is.
     * \since QGIS 3.0
     */
    QString name() const override;

    /**
     * Return description contained in SPATIALITE_DESCRIPTION
     *
     * Return a terse string describing what the provider is.
     * \since QGIS 3.0
     */
    QString description() const override;
    static QgsSpatiaLiteProvider *createProvider( const QString &uri );

    /**
     * List of primary key columns in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see getPrimaryKeyAttrs
     * \since QGIS 3.0
     */
    QgsAttributeList pkAttributeIndexes() const override;
    void invalidateConnections( const QString &connection ) override;
    QList<QgsRelation> discoverRelations( const QgsVectorLayer *self, const QList<QgsVectorLayer *> &layers ) const override;

  signals:

    /**
     *   This is emitted whenever the worker thread has fully calculated the
     *   extents for this layer, and its event has been received by this
     *   provider.
     */
    void fullExtentCalculated();

    /**
     *   This is emitted when this provider is satisfied that all objects
     *   have had a chance to adjust themselves after they'd been notified that
     *   the full extent is available.
     *
     *   \note  It currently isn't being emitted because we don't have an easy way
     *          for the overview canvas to only be repainted.  In the meantime
     *          we are satisfied for the overview to reflect the new extent
     *          when the user adjusts the extent of the main map canvas.
     */
    void repaintRequested();

  private:

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
     * \see SpatialiteDbInfo::isDbValid
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;

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
     * Is the Layer valid
     * \note
     *  when false: the Layer should not be rendered
     * \since QGIS 3.0
     */
    bool mIsLayerValid;

    /**
     * Convert a QgsField to work with Spatialite
     */
    static bool convertField( QgsField &field );

    /**
     * nothing [unknown what this should be]
     */
    QString geomParam() const;

    /**
     * Search all the layers using the given table.
     */
    static QList<QgsVectorLayer *> searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &tableName );

    /**
     * Flag indicating if the layer data source is a valid SpatiaLite layer
     * \note
     *  result of setSqliteHandle and setDbLayer
     * otherwise not used
     * \see setSqliteHandle
     * \since QGIS 3.0
     */
    bool mValid;

    /**
     * Flag indicating if the layer data source is based on a query
     * \note
     *
     * \see checkQuery
     * \since QGIS 3.0
     */
    bool mIsQuery;

    /**
     * Flag indicating if the layer data source is based on a Spatial Table
     * \note
     *  result of getLayerType() == SpatialiteDbInfo::SpatialTable or TopologyExport
     * otherwise not used
     * \see setDbLayer
     * \since QGIS 3.0
     */
    bool mTableBased;

    /**
     *Flag indicating if the layer data source is based on a View
     * \note
     *  result of getLayerType() == SpatialiteDbInfo::SpatialView
     * otherwise not used
     * \see setDbLayer
     * \since QGIS 3.0
     */
    bool mViewBased;

    /**
     * Flag indicating if the layer data source is based on a VirtualShape
     * \note
     *  result of getLayerType() == SpatialiteDbInfo::VirtualShape
     * otherwise not used
     * \see setDbLayer
     * \since QGIS 3.0
     */
    bool mVShapeBased;

    /**
     * Flag indicating if the layer data source has ReadOnly restrictions
     * \note
     *
     * \see checkQuery
     * \since QGIS 3.0
     */
    bool mReadOnly;

    /**
     * DB full path
     * extracted from dataSourceUri()
     * \note
     * The Uri is based on the ogr version, which is NOT supported by QgsDataSourceUri
     * \since QGIS 3.0
     */
    QString mSqlitePath;

    /**
     * Name of the table with no schema
     * extracted from dataSourceUri()
     * \note
     * The Uri is based on the ogr version, which is NOT supported by QgsDataSourceUri
     * \since QGIS 3.0
     */
    QString mUriTableName;

    /**
     * Name of the table or subquery
     * \note
     *
     * \see checkQuery
     * \since QGIS 3.0
     */
    QString mQuery;

    /**
     * Name of the primary key column in the table from QgsDataSourceUri
     * \note
     *  from QgsDataSourceUri::anUri.keyColumn
     *  Used to build 'mUriLayerName'
     * \see QgsSpatiaLiteProvider
     * \since QGIS 3.0
     */
    QString mUriPrimaryKey;

<<<<<<< HEAD
    //! Flag indicating whether the primary key is auto-generated
    bool mPrimaryKeyAutoIncrement = false;

    //! List of primary key columns in the table
    QgsAttributeList mPrimaryKeyAttrs;
=======
    /**
     * Name of the geometry column in the table
     * \note
     *  from QgsDataSourceUri::geometryColumn().toLower()
     *  Used to build 'mUriLayerName'
     * \see QgsSpatiaLiteProvider
     * \since QGIS 3.0
     */
    QString mUriGeometryColumn;
>>>>>>> Refactured version based master from 2017-11-17. Minimal Spatialite connection with load_extension. RasterLite2 without source dependency. Full QgsLayerItem and QgsLayerMetadata support.

    /**
     * Name of the geometry column in the table
     * \note
     *  from QgsDataSourceUri::geometryColumn().toLower()
     *  Used to build Database _connection with loading of Layer
     * \see QgsSpatiaLiteProvider
     * \see QgsSqliteHandle::openDb
     * \since QGIS 3.0
     */
    QString mUriLayerName;

    /**
     * Name of the table/view with no schema
     * \note
     *  For SpatialViews
     *  - the registered TABLE can be retrieved with getViewTableName()
     * \see mViewTableName
     * \since QGIS 3.0
     */
    QString mTableName;

    /**
     * Name of the geometry column in the table
     * \note
     *  Vector: should always be filled
     *  Raster: may be empty
     * \since QGIS 3.0
     */
    QString mGeometryColumn;

    /**
     * Name of the table which contains the SpatialView-Geometry (underlining table)
     * \note
     *  For SpatialTable/VirtualShapes: empty
     *  For SpatialViews:
     *  - the registered VIEW can be retrieved with getTableName()
     * \see mTableName
     * \since QGIS 3.0
     */
    QString mViewTableName;

    /**
     * Name of the table-geometry which contains the SpatialView-Geometry (underlining table)
     * \note
     *  For SpatialTable/VirtualShapes: empty
     *  For SpatialViews:
     *  - the registered VIEW can be retrieved with getTableName()
     * \see mTableName
     * \since QGIS 3.0
     */
    QString mViewTableGeometryColumn;

    /**
     * Return the Table-Name to use to build the IndexTable-Name
     * \note
     *  For SpatialTable/VirtualShapes: result of getTableName will be used
     *  For SpatialViews: result of getViewTableName will be used
     * \see SpatialiteDbLayer::getTableName
     * \see SpatialiteDbLayer::getViewTableName
     * \see QgsSpatiaLiteFeatureSource::mIndexTable
     * \since QGIS 3.0
     */
    QString mIndexTable;

    /**
     * The Spatialite Layer-Type of the Layer
     * - representing mLayerType
     * \see SpatialiteDbLayer::getLayerType
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialiteLayerType mLayerType;

    /**
     * The SpatialIndex-Type used for the Geometry
     * \note
     *  Uses the same numbering as Spatialite [0,1,2]
     * \see SpatialiteDbLayer::getSpatialIndexType
     * \since QGIS 3.0
     */
    int mSpatialIndexType;

    /**
     * Geometry type
     * \note
     *  result of getDbLayer()->getGeometryType()
     * otherwise not used
     * \see setDbLayer
     * \since QGIS 3.0
     */
    QgsWkbTypes::Type mGeometryType;

    /**
     * The Spatialite Geometry-Type of the Layer (as String)
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     * \see SpatialiteDbLayer::getGeometryType
     * \since QGIS 3.0
     */
    QString mGeometryTypeString;

    /**
     * List of layer fields in the table
     * \note
     *  - from PRAGMA table_info()
     * \see SpatialiteDbLayer::getAttributeFields
     * \since QGIS 3.0
     */
    QgsFields mAttributeFields;

    /**
     * Name of the primary key column in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see SpatialiteDbLayer::getPrimaryKey
     * \since QGIS 3.0
     */
    QString mPrimaryKey;

    /**
     * Column-Number of the primary key
     * \note
     *  - SpatialView: name from entry of view_rowid of views_geometry_columns
     *  -> position from inside the PRAGMA table_info('name')
     * \see SpatialiteDbLayer::getPrimaryKeyCId
     * \since QGIS 3.0
     */
    int mPrimaryKeyCId;

    /**
     * List of primary key columns in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see SpatialiteDbLayer::getPrimaryKeyAttrs
     * \since QGIS 3.0
     */
    QgsAttributeList mPrimaryKeyAttrs;

    /**
     * Return the Geometry-Name to use to build the IndexTable-Name
     * \note
     *  For SpatialTable/VirtualShapes: result of getGeometryColumn will be used
     *  For SpatialViews: result of getViewTableGeometryColumn will be used
     * \see SpatialiteDbLayer::getGeometryColumn
     * \see SpatialiteDbLayer::getViewTableGeometryColumn
     * \see QgsSpatiaLiteFeatureSource::mIndexGeometry
     * \since QGIS 3.0
     */
    QString mIndexGeometry;

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

<<<<<<< HEAD
    QgsVectorDataProvider::Capabilities mEnabledCapabilities = nullptr;
=======
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
>>>>>>> Refactured version based master from 2017-11-17. Minimal Spatialite connection with load_extension. RasterLite2 without source dependency. Full QgsLayerItem and QgsLayerMetadata support.

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
     * The AuthId [auth_name||':'||auth_srid]
     * \see SpatialiteDbLayer::getAuthId
     * \since QGIS 3.0
     */
    QString mAuthId;

    /**
     * The Proj4text [from mSrid]
     * \see SpatialiteDbLayer::getProj4text
     * \since QGIS 3.0
     */
    QString mProj4text;

    /**
     * String used to define a subset of the layer
     * \note
     *  QgsDataSourceUri is reset base on this value
     * \see setSubsetString
     * \since QGIS 3.0
     */
    QString mSubsetString;

    /**
     * this Geometry is supported by an R*Tree spatial index
     * \note
     *  result of getDbLayer()->getSpatialIndexType() == SpatialiteDbInfo::SpatialIndexRTree
     *  Used in QgsSpatiaLiteFeatureIterator
     * \see setDbLayer
     * \see QgsSpatiaLiteFeatureSource::mSpatialIndexRTree
     * \see QgsSpatiaLiteFeatureIterator::whereClauseRect
     * \since QGIS 3.0
     */
    bool mSpatialIndexRTree;

    /**
     * this Geometry is supported by an MBR cache spatial index
     * \note
     *  result of getDbLayer()->getSpatialIndexType() == SpatialiteDbInfo::SpatialIndexMbrCache
     *  Used in QgsSpatiaLiteFeatureIterator
     * \see setDbLayer
     * \see QgsSpatiaLiteFeatureSource::mSpatialIndexMbrCache
     * \see QgsSpatiaLiteFeatureIterator::whereClauseRect
     * \since QGIS 3.0
     */
    bool mSpatialIndexMbrCache;

    /**
     * Retrieve a specific of layer fields of the table
     * - using SpatialiteDbLayer
     * \note
     * \returns result from getDbLayer()->getAttributeField( index );
     * \see SpatialiteDbLayer::getAttributeField
     * \since QGIS 3.0
     */
    QgsField field( int index ) const;

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
     * checkQuery
     *
     * \note
     */
    bool checkQuery();

    /**
     * prepareStatement
     *
     * \note
     */
    bool prepareStatement( sqlite3_stmt *&stmt,
                           const QgsAttributeList &fetchAttributes,
                           bool fetchGeometry,
                           QString whereClause );
<<<<<<< HEAD
    bool getFeature( sqlite3_stmt *stmt, bool fetchGeometry,
                     QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );

    void updatePrimaryKeyCapabilities();

    int computeSizeFromMultiWKB2D( const unsigned char *p_in, int nDims,
                                   int little_endian,
                                   int endian_arch );
    int computeSizeFromMultiWKB3D( const unsigned char *p_in, int nDims,
                                   int little_endian,
                                   int endian_arch );
    void convertFromGeosWKB2D( const unsigned char *blob, int blob_size,
                               unsigned char *wkb, int geom_size,
                               int nDims, int little_endian, int endian_arch );
    void convertFromGeosWKB3D( const unsigned char *blob, int blob_size,
                               unsigned char *wkb, int geom_size,
                               int nDims, int little_endian, int endian_arch );
    void convertFromGeosWKB( const unsigned char *blob, int blob_size,
                             unsigned char **wkb, int *geom_size,
                             int dims );
    int computeSizeFromGeosWKB3D( const unsigned char *blob, int size,
                                  QgsWkbTypes::Type type, int nDims, int little_endian,
                                  int endian_arch );
    int computeSizeFromGeosWKB2D( const unsigned char *blob, int size,
                                  QgsWkbTypes::Type type, int nDims, int little_endian,
                                  int endian_arch );

    void fetchConstraints();

    void insertDefaultValue( int fieldIndex, QString defaultVal );
=======
>>>>>>> Refactured version based master from 2017-11-17. Minimal Spatialite connection with load_extension. RasterLite2 without source dependency. Full QgsLayerItem and QgsLayerMetadata support.

    /**
     * getFeature
     *
     * \note
     */
    bool getFeature( sqlite3_stmt *stmt, bool fetchGeometry,
                     QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );

    friend class QgsSpatiaLiteFeatureSource;

};

#endif
