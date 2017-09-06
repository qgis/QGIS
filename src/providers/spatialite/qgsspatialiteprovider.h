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

extern "C"
{
#include <sys/types.h>
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
}

#include "qgsvectordataprovider.h"
#include "qgsrectangle.h"
#include "qgsvectorlayerexporter.h"
#include "qgsfields.h"
#include <list>
#include <queue>
#include <fstream>
#include <set>

class QgsFeature;
class QgsField;

class QgsSqliteHandle;
class QgsSpatiaLiteFeatureIterator;

#include "qgsdatasourceuri.h"
#include "qgsspatialiteconnection.h"

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
    //! Import a vector layer into the database
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
    QgsWkbTypes::Type wkbType() const override;

    /** Return the number of layers for the current data source
     *
     * \note Should this be subLayerCount() instead?
     */
    size_t layerCount() const;

    /** Amount of features
     *  - implementation of Provider function 'featureCount'
     * \note
     *  - implemented in SpatialiteDbLayer
     *  \returns amount of features of the Layer
     * \see getNumberFeatures
     * \since QGIS 3.0
     */
    long featureCount() const override;

    /** Extent of the Layer
     *  - implementation of Provider function 'extent'
     * \note
     *  - implemented in SpatialiteDbLayer
     *  \returns QgsRectangle extent of the Layer
     *  - getLayerExtent is called retrieving the extent without calling UpdateLayerStatistics
     * \see getLayerExtent
     * \since QGIS 3.0
     */
    virtual QgsRectangle extent() const override;

    /** Extent of the Layer
     *  - implementation of Provider function 'updateExtents'
     * \note
     *  - implemented in SpatialiteDbLayer
     *  - getLayerExtent is called retrieving the extent after a UpdateLayerStatistics
     * \see getLayerExtent
     * \since QGIS 3.0
     */
    virtual void updateExtents() override;

    /** List of fields
     *  - implementation of Provider function 'fields'
     * \note
     *  - implemented in SpatialiteDbLayer
     *  \returns QgsFields of the Layer
     * \see getAttributeFields
     * \since QGIS 3.0
     */
    QgsFields fields() const override;

    /** Returns the minimum value of an attribute
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

    /** Returns the maximum value of an attribute
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

    /** Returns a list of unique values
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

    /** UniqueStringsMatching
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

    /** Adds features
     *  - implementation of Provider function 'addFeatures'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param flist feature to add
     *  \returns True in case of success and False in case of error or container not supported
     * \see SpatialiteDbLayer::addLayerFeatures
     * \since QGIS 3.0
     */
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = 0 ) override;

    /** Deletes features
     *  - implementation of Provider function 'deleteFeatures'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param id QgsFeatureIds to delete
     *  \returns True in case of success and False in case of error or container not supported
     * \see SpatialiteDbLayer::deleteLayerFeatures
     * \since QGIS 3.0
     */
    bool deleteFeatures( const QgsFeatureIds &id ) override;

    /** Deletes all records of Layer-Table
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

    /** Adds attributes
     *  - implementation of Provider function 'addAttributes'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param attributes List of QgsField
     *  \returns True in case of success and False in case of error
     * \see SpatialiteDbLayer::addLayerAttributes
     * \since QGIS 3.0
     */
    bool addAttributes( const QList<QgsField> &attributes ) override;

    /** Updates attributes
     *  - implementation of Provider function 'changeAttributeValues'
     * \note
     *  - Spatialite specific [GeoPackage could be implemented, but not forseen]
     * \param attr_map collection of attributes to change
     *   \returns True in case of success and False in case of error
     * \see SpatialiteDbLayer::changeAttributeValues
     * \since QGIS 3.0
     */
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

    /** Updates features
     *  - implementation of Provider function 'changeGeometryValues'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param geometry_map collection of geometries to change
     *  \returns True in case of success and False in case of error
     * \see SpatialiteDbLayer::changeLayerGeometryValues
     * \since QGIS 3.0
     */
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    /** Based on Layer-Type, set QgsVectorDataProvider::Capabilities
     *  - implementation of Provider function 'capabilities'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \see getCapabilities
     * \since QGIS 3.0
    */
    QgsVectorDataProvider::Capabilities capabilities() const override;

    /** Based on Layer-Type, set QgsVectorDataProvider::Capabilities
     *  - implementation of Provider function 'defaultValue'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \see getDefaultValues()
     * \returns value from Map of field index of the default values
     * \since QGIS 3.0
    */
    QVariant defaultValue( int fieldId ) const override;

    /** Creates attributes Index
     *  - implementation of Provider function 'createAttributeIndex'
     * \note
     *  - implemented in SpatialiteDbLayer
     * \param field number of attribute to created the index for
     * \returns True in case of success and False in case of error
     * \see QgsSpatiaLiteProvider::createLayerAttributeIndex
     * \since QGIS 3.0
     */
    bool createAttributeIndex( int field ) override;

    /** The class allowing to reuse the same sqlite handle for more layers
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    QgsSqliteHandle *getQSqliteHandle() const { return mHandle; }

    /** Retrieve SpatialiteDbInfo
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *getSpatialiteDbInfo() const { return mSpatialiteDbInfo; }

    /** The Database filename (with Path)
    * \returns mDatabaseFileName complete Path (without without symbolic links)
    * \since QGIS 3.0
    */
    QString getDatabaseFileName() const { return getSpatialiteDbInfo()->getDatabaseFileName(); }

    /** The Spatialite internal Database structure being read
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see SpatialiteDbInfo::getSniffDatabaseType
    * \since QGIS 3.0
    */
    SpatialiteDbInfo::SpatialMetadata dbSpatialMetadata() const { return getSpatialiteDbInfo()->dbSpatialMetadata(); }

    /** The Spatialite Version Driver being used
     * \note
     *  - returned from spatialite_version()
     * \see SpatialiteDbInfo::getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString dbSpatialiteVersionInfo() const { return getSpatialiteDbInfo()->dbSpatialiteVersionInfo(); }

    /** The major Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see SpatialiteDbInfo::getSniffDatabaseType
     * \since QGIS 3.0
    */
    int dbSpatialiteVersionMajor() const { return getSpatialiteDbInfo()->dbSpatialiteVersionMajor(); }

    /** The minor Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see SpatialiteDbInfo::getSniffDatabaseType
     * \since QGIS 3.0
    */
    int dbSpatialiteVersionMinor() const { return getSpatialiteDbInfo()->dbSpatialiteVersionMinor(); }

    /** The revision Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see SpatialiteDbInfo::getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionRevision() const { return getSpatialiteDbInfo()->dbSpatialiteVersionRevision(); }

    /** Amount of SpatialTables  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialTables that have been loaded
     * \since QGIS 3.0
     */
    int dbSpatialTablesLayersCount() const { return getSpatialiteDbInfo()->dbSpatialTablesLayersCount(); }

    /** Amount of SpatialViews  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialViews that have been loaded
     * \since QGIS 3.0
     */
    int dbSpatialViewsLayersCount() const { return getSpatialiteDbInfo()->dbSpatialViewsLayersCount(); }

    /** Amount of VirtualShapes found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of VirtualShapes that have been loaded
     * \since QGIS 3.0
     */
    int dbVirtualShapesLayersCount() const { return getSpatialiteDbInfo()->dbVirtualShapesLayersCount(); }

    /** Amount of RasterLite1-Rasters found in the Database
     * - only the count of valid Layers are returned
     * \note
     * - the Gdal-RasterLite1-Driver is needed to Determineeee this
     * - this does not reflect the amount of RasterLite1-Rasters that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterLite1LayersCount() const { return getSpatialiteDbInfo()->dbRasterLite1LayersCount(); }

    /** Amount of RasterLite2 Vector-Coverages found in the Database
     * - from the vector_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 Vector-Coverages that have been loaded
     * \since QGIS 3.0
     */
    int dbVectorCoveragesLayersCount() const { return getSpatialiteDbInfo()->dbVectorCoveragesLayersCount(); }

    /** Amount of RasterLite2 Raster-Coverages found in the Database
     * - from the raster_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 Raster-Coverages that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterCoveragesLayersCount() const { return getSpatialiteDbInfo()->dbRasterCoveragesLayersCount(); }
    //! Does the read Database contain Topology tables [-1=no topologies table, otherwise amount (0 being empty)]
    int dbTopologyExportLayersCount() const { return getSpatialiteDbInfo()->dbTopologyExportLayersCount(); }
    //! Is the used Spatialite compiled with Spatialite-Gcp support
    bool hasDbGcpSupport() const { return getSpatialiteDbInfo()->hasDbGcpSupport(); }
    //! Is the used Spatialite compiled with Topology (and thus RtTopo) support
    bool hasDbTopologySupport() const { return getSpatialiteDbInfo()->hasDbTopologySupport(); }
    //! Is the used Spatialite 4.5.0 or greater
    bool isDbVersion45() const { return getSpatialiteDbInfo()-> isDbVersion45(); }

    /** Loaded Layers-Counter
     * - contained in mDbLayers
     * \note
     * - only when GetSpatialiteDbInfoWrapper is called with LoadLayers=true
     * -- will all the Layers be loaded
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    int dbLayersCount() const { return getSpatialiteDbInfo()->dbLayersCount(); }

    /** Amount of Vector-Layers found
     * - SpatialTables, SpatialViews and virtualShapes [from the vector_layers View]
     * \note
     * - this amount may differ from dbLayersCount()
     * -- which only returns the amount of Loaded-Vector-Layers
     * \see dbLayersCount()
     * \since QGIS 3.0
     */
    int dbVectorLayersCount() const { return getSpatialiteDbInfo()->dbVectorLayersCount(); }
    //! Flag indicating if the layer data source has ReadOnly restrictions
    bool isDbReadOnly() const { return getSpatialiteDbInfo()->isDbReadOnly(); }

    /** Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \since QGIS 3.0
     */
    bool isDbValid() const { return getSpatialiteDbInfo()->isDbValid(); }

    /** Is the read Database a Spatialite Database
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \since QGIS 3.0
     */
    bool isDbSpatialite() const { return getSpatialiteDbInfo()->isDbSpatialite(); }

    /** The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool isDbGdalOgr() const { return getSpatialiteDbInfo()->isDbGdalOgr(); }

    /** The active Layer
     * - being read by the Provider
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \see setDbLayer
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *getDbLayer() const { return mDbLayer; }

    /** The sqlite handler
     * - contained in the QgsSqliteHandle class being used by the layer
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    sqlite3 *dbSqliteHandle() const { return getDbLayer()->dbSqliteHandle(); }
    //! Name of the table with no schema
    QString getTableName() const { return getDbLayer()->getTableName(); }
    //! Name of the geometry column in the table
    QString getGeometryColumn() const { return getDbLayer()->getGeometryColumn(); }
    //! Name of the table which contains the SpatialView-Geometry (underlining table)
    QString getViewTableName() const { return getDbLayer()->getViewTableName(); }
    QString getIndexTable() const { if ( getLayerType() == SpatialiteDbInfo::SpatialView ) return getViewTableName(); else return getTableName(); }
    //! Name of the table-geometry which contains the SpatialView-Geometry (underlining table)
    QString getViewTableGeometryColumn() const { return getDbLayer()->getViewTableGeometryColumn(); }
    QString getIndexGeometry() const { if ( getLayerType() == SpatialiteDbInfo::SpatialView ) return getViewTableGeometryColumn(); else return getGeometryColumn(); }
    //! Name of the Layer format: 'table_name(geometry_name)'
    QString getLayerName() const { return getDbLayer()->getLayerName(); }
    //! Title [RasterLite2]
    QString getTitle() const { return getDbLayer()->getTitle(); }
    //! Title [RasterLite2]
    QString getAbstract() const { return getDbLayer()->getAbstract(); }
    //! Copyright [RasterLite2]
    QString getCopyright() const { return getDbLayer()->getCopyright(); }
    //! The Srid of the Geometry
    int getSrid() const { return getDbLayer()->getSrid(); }
    //! AuthId [auth_name||':'||auth_srid]
    QString getAuthId() const { return getDbLayer()->getAuthId(); }
    //! Proj4text [from mSrid]
    QString getProj4text() const { return getDbLayer()->getProj4text(); }
    //! The SpatialiIndex used for the Geometry
    int getSpatialIndexType() const { return getDbLayer()->getSpatialIndexType(); }
    //! The Spatialite Layer-Type being read
    SpatialiteDbInfo::SpatialiteLayerType getLayerType() const { return getDbLayer()->getLayerType(); }
    //! The Spatialite Geometry-Type being read
    QgsWkbTypes::Type getGeometryType() const { return getDbLayer()->getGeometryType(); }
    //! The Spatialite Geometry-Type being read (as String)
    QString getGeometryTypeString() const { return getDbLayer()->getGeometryTypeString(); }
    //! The Spatialite Coord-Dimensions
    int getCoordDimensions() const { return getDbLayer()->getCoordDimensions(); }

    /** Rectangle that contains the extent (bounding box) of the layer
     * \note
     *  With UpdateLayerStatistics the Number of features will also be updated and retrieved
     * \param bUpdate force reading from Database
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see mLayerExtent
     * \see mNumberFeatures
     * \since QGIS 3.0
     */
    QgsRectangle getLayerExtent( bool bUpdate = false, bool bUpdateStatistics = false ) const { return getDbLayer()->getLayerExtent( bUpdate, bUpdateStatistics ); }

    /** Number of features in the layer
     * \note
     *  With UpdateLayerStatistics the Extent will also be updated and retrieved
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see getLayerExtent
     * \see getNumberFeatures
     * \since QGIS 3.0
     */
    long getNumberFeatures( bool bUpdateStatistics = false ) const { return getDbLayer()->getNumberFeatures( bUpdateStatistics ); }
    //! The Spatialite Layer-Readonly status [true or false]
    int isLayerReadOnly() const { return getDbLayer()->isLayerReadOnly(); }
    //! The Spatialite Layer-Hidden status [true or false]
    int isLayerHidden() const { return getDbLayer()->isLayerHidden(); }
    //! The Spatialite Layer-Id being created
    int getLayerId() const { return getDbLayer()->getLayerId(); }

    /** Based on Layer-Type, set QgsVectorDataProvider::Capabilities
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
    //! A possible Query from QgsDataSourceUri
    QString getLayerQuery() const { return getDbLayer()->getLayerQuery(); }
    //! A possible Query from QgsDataSourceUri
    void setLayerQuery( QString sQuery ) { return getDbLayer()->setLayerQuery( sQuery ); }
    //! Is the read Database supported by QgsSpatiaLiteProvider
    //! Name of the primary key column in the table
    QString getPrimaryKey() const { return getDbLayer()->getPrimaryKey(); }
    //! List of primary key columns in the table
    QgsAttributeList getPrimaryKeyAttrs() const { return getDbLayer()->getPrimaryKeyAttrs(); }
    //! List of layer fields in the table
    QgsFields getAttributeFields() const { return getDbLayer()->getAttributeFields(); }
    //! Map of field index to default value [for Topology, the Topology-Layers]
    QMap<int, QVariant> getDefaultValues() const { return getDbLayer()->getDefaultValues(); }

    /** Connection info (DB-path) without table and geometry
     * - this will be called from the SpatialiteDbLayer::dbConnectionInfo()
     * \note
     *  - to call for Database and Table/Geometry portion use: SpatialiteDbLayer::dbConnectionInfo()
    * \returns uri with Database only
    * \since QGIS 3.0
    */
    QString dbConnectionInfo() const { return getSpatialiteDbInfo()->dbConnectionInfo(); }

    /** Connection info (DB-path) with table and geometry
     * \note
     *  - to call for Database portion only, use: SpatialiteDbInfo::dbConnectionInfo()
     *  - For RasterLite1: GDAL-Syntax will be used
    * \returns uri with Database and Table/Geometry Information
    * \since QGIS 3.0
    */
    QString getLayerDataSourceUri() const { return getDbLayer()->getLayerDataSourceUri(); }
    //! Is the Layer valid
    bool isLayerValid() const { if ( getDbLayer() ) return getDbLayer()->isLayerValid(); else return false;}

    /** Is the Layer
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \since QGIS 3.0
     */
    bool isLayerSpatialite() const { if ( getDbLayer() ) return getDbLayer()->isLayerSpatialite(); else return false;}

    /** The SpatiaLite provider does its own transforms so we return
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

    QString name() const override;
    QString description() const override;
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

    /** The class allowing to reuse the same sqlite handle for more layers
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see getQSqliteHandle()
     * \since QGIS 3.0
     */
    QgsSqliteHandle *mHandle = nullptr;

    /** Sets the activeQgsSqliteHandle
     * - checking will be done to insure that the Database connected to is considered valid
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \see mDbLayer
     * \since QGIS 3.0
     */
    bool setSqliteHandle( QgsSqliteHandle *sqliteHandle );

    /** SpatialiteDbInfo Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;

    /** Sets the active Layer
     * - checking will be done to insure that the Layer is considered valid
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \see mDbLayer
     * \since QGIS 3.0
     */
    bool setDbLayer( SpatialiteDbLayer *dbLayer );

    /** The active Layer
     * - being read by the Provider
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see SpatialiteDbLayer::isLayerValid
     * \see setDbLayer
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *mDbLayer = nullptr;

    //! Convert a QgsField to work with SL
    static bool convertField( QgsField &field );

    QString geomParam() const;

    /**
     * Search all the layers using the given table.
     */
    static QList<QgsVectorLayer *> searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &tableName );

    //! Flag indicating if the layer data source is a valid SpatiaLite layer
    bool mValid;

    //! Flag indicating if the layer data source is based on a query
    bool mIsQuery;

    //! Flag indicating if the layer data source is based on a plain Table
    bool mTableBased;
    //! Flag indicating if the layer data source is based on a View
    bool mViewBased;
    //! Flag indicating if the layer data source is based on a VirtualShape
    bool mVShapeBased;
    //! Flag indicating if the layer data source has ReadOnly restrictions
    bool mReadOnly;
    //! DB full path
    QString mSqlitePath;

    //! Name of the table with no schema
    QString mUriTableName;

    //! Name of the table or subquery
    QString mQuery;

    //! Name of the primary key column in the table from QgsDataSourceUri
    QString mUriPrimaryKey;

    //! Name of the geometry column in the table
    QString mUriGeometryColumn;
    //!  Name of the Layer to search for format: 'table_name(geometry_name)'
    QString mUriLayerName;

    //! Geometry type
    QgsWkbTypes::Type mGeometryType;

    //! String used to define a subset of the layer
    QString mSubsetString;

    //! Spatial reference id of the layer
    int mSrid;

    //! this Geometry is supported by an R*Tree spatial index
    bool mSpatialIndexRTree;

    //! this Geometry is supported by an MBR cache spatial index
    bool mSpatialIndexMbrCache;
    //! Retrieve a specific of layer fields of the table
    QgsField field( int index ) const;

    /** Close the Database
     * - using QgsSqliteHandle [static]
     * \note
     * - if the connection is being shared and used elsewhere, the Database will not be closed
     * \see QgsSqliteHandle::closeDb
     * \since QGIS 3.0
     */
    void closeDb();
    bool checkQuery();
    bool prepareStatement( sqlite3_stmt *&stmt,
                           const QgsAttributeList &fetchAttributes,
                           bool fetchGeometry,
                           QString whereClause );
    bool getFeature( sqlite3_stmt *stmt, bool fetchGeometry,
                     QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );

    friend class QgsSpatiaLiteFeatureSource;

};

#endif
