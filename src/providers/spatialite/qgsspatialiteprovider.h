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
  \class QgsSpatiaLiteProvider
  \brief Data provider for SQLite/SpatiaLite layers.

  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a SQLite/SpatiaLite enabled database.
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

    long featureCount() const override;
    virtual QgsRectangle extent() const override;
    virtual void updateExtents() override;
    QgsFields fields() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    virtual QSet<QVariant> uniqueValues( int index, int limit = -1 ) const override;
    virtual QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
        QgsFeedback *feedback = nullptr ) const override;

    bool isValid() const override;
    virtual bool isSaveAndLoadStyleToDatabaseSupported() const override { return true; }
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = 0 ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool truncate() override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    QgsVectorDataProvider::Capabilities capabilities() const override;
    QVariant defaultValue( int fieldId ) const override;
    bool createAttributeIndex( int field ) override;
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
    //! The Database filename being read
    QString getDatabaseFileName() const { return getSpatialiteDbInfo()->getDatabaseFileName(); }
    //! The Spatialite internal Database structure being read
    SpatialiteDbInfo::SpatialMetadata dbSpatialMetadata() const { return getSpatialiteDbInfo()->dbSpatialMetadata(); }
    //! The Spatialite Version as returned by spatialite_version()
    QString dbSpatialiteVersionInfo() const { return getSpatialiteDbInfo()->dbSpatialiteVersionInfo(); }
    //! The major Spatialite Version being used
    int dbSpatialiteVersionMajor() const { return getSpatialiteDbInfo()->dbSpatialiteVersionMajor(); }
    //! The minor Spatialite Version being used
    int dbSpatialiteVersionMinor() const { return getSpatialiteDbInfo()->dbSpatialiteVersionMinor(); }
    //! The revision Spatialite Version being used
    int dbSpatialiteVersionRevision() const { return getSpatialiteDbInfo()->dbSpatialiteVersionRevision(); }
    //! Does the read Database contain SpatialTables [ 0=none, otherwise amount]
    int dbSpatialTablesCount() const { return getSpatialiteDbInfo()->dbSpatialTablesCount(); }
    //! Does the read Database contain SpatialViews views [ 0=none, otherwise amount]
    int dbSpatialViewsCount() const { return getSpatialiteDbInfo()->dbSpatialViewsCount(); }
    //! Does the read Database contain VirtualShapes tables [0=none, otherwise amount]
    int dbVirtualShapesCount() const { return getSpatialiteDbInfo()->dbVirtualShapesCount(); }
    //! Does the read Database contain RasterLite1 coverages [-1=no rasterlit1 logic found, otherwise amount (0 being empty)]
    int dbRasterLite1TablesCount() const { return getSpatialiteDbInfo()->dbRasterLite1TablesCount(); }
    //! Does the read Database contain RasterLite2 coverages [-1=no raster_coverages table, otherwise amount (0 being empty)]
    int dbRasterLite2TablesCount() const { return getSpatialiteDbInfo()->dbRasterLite2TablesCount(); }
    //! Does the read Database contain Topology tables [-1=no topologies table, otherwise amount (0 being empty)]
    int dbTopologyTablesCount() const { return getSpatialiteDbInfo()->dbTopologyTablesCount(); }
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

    /** The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool isDbGdalOgr() const { return getSpatialiteDbInfo()->isDbGdalOgr(); }
    //! The active Layer
    SpatialiteDbLayer *getDbLayer() const { return mDbLayer; }
    //! The sqlite handler
    sqlite3 *getSqliteHandle() const { return getDbLayer()->getSqliteHandle(); }
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
    QgsWkbTypes::Type getGeomType() const { return getDbLayer()->getGeomType(); }
    //! The Spatialite Geometry-Type being read (as String)
    QString getGeomTypeString() const { return getDbLayer()->getGeomTypeString(); }
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
     * - SpatialiteTopopogy: will serve only TopopogyLayer, which ate SpatialTables
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
    QString layerConnectionInfo() const { return getDbLayer()->layerConnectionInfo(); }
    //! Is the Layer valid
    bool isLayerValid() const { if ( getDbLayer() ) return getDbLayer()->isLayerValid(); else return false;}

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

    static QString quotedIdentifier( QString id );
    static QString quotedValue( QString value );

    struct SLFieldNotFound {}; //! Exception to throw

    struct SLException
    {
        explicit SLException( char *msg ) : errMsg( msg )
        {
        }

        SLException( const SLException &e ) : errMsg( e.errMsg )
        {
        }

        ~SLException()
        {
          if ( errMsg )
            sqlite3_free( errMsg );
        }

        SLException &operator=( const SLException &other ) = delete;

        QString errorMessage() const
        {
          return errMsg ? QString::fromUtf8( errMsg ) : QStringLiteral( "unknown cause" );
        }

      private:
        char *errMsg = nullptr;

    };

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
     * sqlite3 handles pointer
     */
    QgsSqliteHandle *mHandle = nullptr;
    bool setSqliteHandle( QgsSqliteHandle *sqliteHandle );

    /** SpatialiteDbInfo Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;
    bool setDbLayer( SpatialiteDbLayer *dbLayer );
    SpatialiteDbLayer *mDbLayer = nullptr;

    //! Convert a QgsField to work with SL
    static bool convertField( QgsField &field );

    QString geomParam() const;

    /**
     * Search all the layers using the given table.
     */
    static QList<QgsVectorLayer *> searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &tableName );

    QgsFields mAttributeFields;

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
    QgsWkbTypes::Type mGeomType;

    //! String used to define a subset of the layer
    QString mSubsetString;

    //! Spatial reference id of the layer
    int mSrid;

    //! this Geometry is supported by an R*Tree spatial index
    bool mSpatialIndexRTree;

    //! this Geometry is supported by an MBR cache spatial index
    bool mSpatialIndexMbrCache;

    QgsField field( int index ) const;

    /**
     * internal utility functions used to handle common SQLite tasks
     */
    //void sqliteOpen();
    void closeDb();
    bool checkQuery();
    bool getQueryGeometryDetails();

    bool prepareStatement( sqlite3_stmt *&stmt,
                           const QgsAttributeList &fetchAttributes,
                           bool fetchGeometry,
                           QString whereClause );
    bool getFeature( sqlite3_stmt *stmt, bool fetchGeometry,
                     QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );

    // Note 20170523: not sure if this is still needed.
    // void updatePrimaryKeyCapabilities();





    /**
     * Handles an error encountered while executing an sql statement.
     */
    void handleError( const QString &sql, char *errorMessage, bool rollback = false );

    friend class QgsSpatiaLiteFeatureSource;

};

#endif
