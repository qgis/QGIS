/***************************************************************************
     qgsspatialiteutils.h
     --------------------------------------
    Date                 : 2017-05-14
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

#ifndef QGSSPATIALITEUTILS_H
#define QGSSPATIALITEUTILS_H

#include <limits>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QString>
#include <QTextStream>
#include <QDebug>

#include <sqlite3.h>
#include <spatialite.h>

#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"

class SpatialiteDbLayer;
class QgsSqliteHandle;

/** Class to contain all information needed for a Spatialite/Rasterlite2 connection
  * - it will 'sniff' all Tables, retaining minimal information for each Table/Layer found
  * - upond demand, a list of SpatialiteDbLayer classes will be collected
  *  -> thus only called once for each connection
  * \note
  *  - The result of this class will be contained in the QgsSqliteHandle class
  *  -> so that the Information must only be called once for each connection
  *  --> When shared, the QgsSqliteHandle class will used the same connection of each Layer in the same Database
  * \see QgsSqliteHandle
  * \see SpatialiteDbLayer
  * \since QGIS 3.0
 */
class SpatialiteDbInfo : public QObject
{
    Q_OBJECT
  public:
    typedef QPair<QVariant::Type, QVariant::Type> TypeSubType;

    /** 'json'
     * - New versions of OGR convert list types to JSON when it stores a Spatialite table.
     */
    static const QString SPATIALITE_ARRAY_PREFIX;

    /** 'list'
     * - New versions of OGR convert list types to JSON when it stores a Spatialite table.
     */
    static const QString SPATIALITE_ARRAY_SUFFIX;

    /** List of Table-Types in a Spatialite Database
     * - For all versions of Spatialite
     * \since QGIS 3.0
     */
    static QStringList mSpatialiteTypes;

    /** Convert QVariant::String to a more realistic type
     * -DateTime ?
     * \note
     * New versions of OGR convert list types (StringList, IntegerList, Integer64List and RealList)
     * to JSON when it stores a Spatialite table. It sets the column type as JSONSTRINGLIST,
     * JSONINTEGERLIST, JSONINTEGER64LIST or JSONREALLIST
     * \since QGIS 3.0
     */
    static TypeSubType GetVariantType( const QString &type );

    /**
      * SniffTypes
      *  - based on the Task, restrict unneeded activities
      *  -> For Browser: only Sqlite3-Container-Type is needed
      *  -> For Provider-Lists: only Tables/Layers without details
      *  -> For Provider: only Tables/Layers with all details
      * \note
      *  - SniffDatabaseType: Determineee Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer]
      *  - SniffExtendend: (planned) possible Editing of of Layer-Propeties such as Column-Names
      * \since QGIS 3.0
      */
    enum SpatialSniff
    {
      SniffUnknown = 0,
      SniffDatabaseType = 1,
      SniffMinimal = 2,
      SniffLoadLayers = 3,
      SniffExtendend = 100
    };

    /**
      * Spatialite-Database Structure
      *  - based on result of CheckSpatialMetaData
      *  -> on the Database being read
      * \note 'Spatialite45'
      *  - is not returned by CheckSpatialMetaData
      * \since QGIS 3.0
      */
    enum SpatialMetadata
    {
      SpatialUnknown = 0,
      SpatialiteLegacy = 1,
      SpatialiteFdoOgr = 2,
      Spatialite40 = 3,
      SpatialiteGpkg = 4,
      Spatialite45 = 5,
      SpatialiteMBTiles = 100
    };
    enum SpatialIndexType
    {
      //! Vector Layer: no Spatial Index - GAIA_SPATIAL_INDEX_NONE    0
      SpatialIndexNone = GAIA_SPATIAL_INDEX_NONE,
      //! Vector Layer: Spatial Index RTree - GAIA_SPATIAL_INDEX_RTREE  1
      SpatialIndexRTree = GAIA_SPATIAL_INDEX_RTREE,
      //! Vector Layer: Spatial Index MbrCache - GAIA_SPATIAL_INDEX_MBRCACHE  2
      SpatialIndexMbrCache = GAIA_SPATIAL_INDEX_MBRCACHE
    };
    enum SpatialiteLayerType
    {
      SpatialiteUnknown = -1,
      SpatialTable = 1,
      SpatialView = 2,
      VirtualShape = 3,
      RasterLite1 = 4, // Gdal
      RasterLite2Vector = 5,
      RasterLite2Raster = 6,
      SpatialiteTopology = 7,
      TopologyExport = 8,
      StyleVector = 9,
      StyleRaster = 20,
      GdalFdoOgr = 21, // Ogr
      GeoPackageVector = 22, // Ogr
      GeoPackageRaster = 23, // Gdal
      MBTilesTable = 24, // Gdal
      MBTilesView = 25, // Gdal
      Metadata = 1000,
      AllSpatialLayers = 1500,
      NonSpatialTables = 1501,
      AllLayers = 1502
    };
    SpatialiteDbInfo( QString sDatabaseFilename, sqlite3 *sqlite_handle  = nullptr )
      : mDatabaseFileName( sDatabaseFilename )
      , mFileName( QString::null )
      , mSqliteHandle( sqlite_handle )
      , mQSqliteHandle( nullptr )
      , mSpatialMetadata( SpatialiteDbInfo::SpatialUnknown )
      , mSpatialiteVersionInfo( QString::null )
      , mSpatialiteVersionMajor( -1 )
      , mSpatialiteVersionMinor( -1 )
      , mSpatialiteVersionRevision( -1 )
      , mHasVectorLayers( -1 )
      , mHasLegacyGeometryLayers( -1 )
      , mHasSpatialTables( 0 )
      , mHasSpatialViews( 0 )
      , mHasVirtualShapes( 0 )
      , mHasRasterLite1Tables( -1 )
      , mHasGdalRasterLite1Driver( false )
      , mHasVectorCoveragesTables( -1 )
      , mHasRasterCoveragesTables( -1 )
      , mHasGdalRasterLite2Driver( false )
      , mHasTopologyExportTables( -1 )
      , mHasMBTilesTables( -1 )
      , mHasGdalMBTilesDriver( false )
      , mHasGeoPackageTables( -1 )
      , mHasGeoPackageVectors( -1 )
      , mHasGeoPackageRasters( -1 )
      , mHasGdalGeoPackageDriver( false )
      , mHasFdoOgrTables( -1 )
      , mHasFdoOgrDriver( false )
      , mHasVectorStylesView( -1 )
      , mHasRasterStylesView( -1 )
      , mHasGcp( false )
      , mHasTopology( false )
      , mIsVersion45( false )
      , mLayersCount( 0 )
      , mReadOnly( false )
      , mLoadLayers( false )
      , mIsValid( false )
      , mIsShared( false )
      , mIsSpatialite( false )
      , mIsSpatialite40( false )
      , mIsSpatialite45( false )
      , mIsGdalOgr( false )
      , mIsSqlite3( false )
      , mSniffType( SpatialiteDbInfo::SniffUnknown )
    {
      if ( SpatialiteDbInfo::readSqlite3MagicHeaderString( mDatabaseFileName ) )
      {
        // The File exists and is a Sqlite3 container
        mIsSqlite3 = true;
        QFileInfo file_info( mDatabaseFileName );
        mDatabaseFileName = file_info.canonicalFilePath();
        mFileName = file_info.fileName();
      }
      // else Either the File does not exists or is not a Sqlite3 container
    }
    ~SpatialiteDbInfo();

    /** Create a SpatialiteDbInfo based Connection
     *  -> containing all needed Information about a Spatial Sqlite3 Container
     * \note
     *  - check result with spatialiteDbInfo->isDbSqlite3()
     *  -> if File exists and is a Sqlite3 Container.
     *  - check result with spatialiteDbInfo->isDbGdalOgr()
     *  -> if File only supported by QgsOgrProvider or QgsGdalProvider
     *  -> otherwise supported by QgsSpatiaLiteProvider
    * \param sDatabaseFileName file to open
    * \param bShared if this connection should be shared
    * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
    * \param bLoadLayers load Layer details
    * \param sniffType based on the Task, restrict unneeded activities
    * \returns true if file is a sqlite3 Database
    * \since QGIS 3.0
    */
    static SpatialiteDbInfo *CreateSpatialiteConnection( const QString sDatabaseFileName, bool bShared = false,
        QString sLayerName = QString::null, bool bLoadLayers = false, SpatialSniff sniffType = SpatialiteDbInfo::SniffUnknown );

    /** The Database filename (with Path)
    * \returns mDatabaseFileName complete Path (without without symbolic links)
    * \since QGIS 3.0
    */
    QString getDatabaseFileName() const { return mDatabaseFileName; }

    /** The Database filename without path
    * \returns mFileName  name of the file, excluding the path
    * \since QGIS 3.0
    */
    QString getFileName() const { return mFileName; }
    //! The sqlite handler
    sqlite3 *dbSqliteHandle() const { return mSqliteHandle; }
    //! The class allowing to reuse the same sqlite handle for more layers
    QgsSqliteHandle *getQSqliteHandle() const { return mQSqliteHandle; }

    /** Connection info (DB-path) without table and geometry
     * - this will be called from classes using SpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: SpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
     * \see SpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    QString dbConnectionInfo( ) const
    {
      return QString( "dbname='%1'" ).arg( QString( mDatabaseFileName ).replace( '\'', QLatin1String( "\\'" ) ) );
    }

    /** The Summary of Layer-Types contained in the Database
     * \note
     *  - at the moment not used
    * \since QGIS 3.0
    */
    QString getSummary() const
    {
      QString sSummary = QString( "%1 SpatialTables" ).arg( dbSpatialTablesLayersCount() );
      if ( dbSpatialViewsLayersCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 SpatialViews" ).arg( sSummary ).arg( dbSpatialViewsLayersCount() );
      }
      if ( dbVirtualShapesLayersCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 VirtualShapes" ).arg( sSummary ).arg( dbVirtualShapesLayersCount() );
      }
      if ( dbRasterCoveragesLayersCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 Rasterlite2 Coverages" ).arg( sSummary ).arg( dbRasterCoveragesLayersCount() );
      }
      if ( dbRasterLite1LayersCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 Rasterlite1 Coverages" ).arg( sSummary ).arg( dbRasterLite1LayersCount() );
      }
      if ( dbTopologyExportLayersCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 Topologies" ).arg( sSummary ).arg( dbTopologyExportLayersCount() );
      }
      return sSummary;
    }
    //! The Spatialite internal Database structure being read
    SpatialiteDbInfo::SpatialMetadata dbSpatialMetadata() const { return mSpatialMetadata; }
    //! The Spatialite internal Database structure being read (as String)
    QString dbSpatialMetadataString() const { return mSpatialMetadataString; }

    /** The Spatialite Version Driver being used
     * \note
     *  - returned from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString dbSpatialiteVersionInfo() const { return mSpatialiteVersionInfo; }

    /** The major Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionMajor() const { return mSpatialiteVersionMajor; }

    /** The minor Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionMinor() const { return mSpatialiteVersionMinor; }

    /** The revision Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionRevision() const { return mSpatialiteVersionRevision; }

    /** Amount of SpatialTables  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialTables that have been loaded
     * \since QGIS 3.0
     */
    int dbSpatialTablesLayersCount() const { return mHasSpatialTables; }

    /** Amount of SpatialViews  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialViews that have been loaded
     * \since QGIS 3.0
     */
    int dbSpatialViewsLayersCount() const { return mHasSpatialViews; }

    /** Amount of VirtualShapes found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of VirtualShapes that have been loaded
     * \since QGIS 3.0
     */
    int dbVirtualShapesLayersCount() const { return mHasVirtualShapes; }

    /** Amount of RasterLite1-Rasters found in the Database
     * - only the count of valid Layers are returned
     * \note
     * - the Gdal-RasterLite1-Driver is needed to Determineeee this
     * - this does not reflect the amount of RasterLite1-Rasters that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterLite1LayersCount() const { return mHasRasterLite1Tables; }

    /** Is the Gdal-RasterLite1-Driver available ?
     * \note
     * - RasterLite1-Rasters can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalRasterLite1Driver() const { return mHasGdalRasterLite1Driver; }

    /** Amount of RasterLite2 Vector-Coverages found in the Database
     * - from the vector_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 Vector-Coverages that have been loaded
     * \since QGIS 3.0
     */
    int dbVectorCoveragesLayersCount() const { return mHasVectorCoveragesTables; }

    /** Amount of RasterLite2 Raster-Coverages found in the Database
     * - from the raster_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 -Raster-Coverages that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterCoveragesLayersCount() const { return mHasRasterCoveragesTables; }

    /** Is the Gdal-RasterLite2-Driver available ?
     * \note
     * - RasterLite2-Rasters can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalRasterLite2Driver() const { return mHasGdalRasterLite2Driver; }

    /** Does the read Database contain RasterLite2 SE_vector_styles_view
     * \note
     *   -1=no SE_vector_styles table, otherwise amount (0 being empty)
     * \since QGIS 3.0
     */
    int dbVectorStylesViewsCount() const { return mHasVectorStylesView; }

    /** Does the read Database contain RasterLite2 SE_raster_styles_view
     * \note
     *   -1=no SE_raster_styles table, otherwise amount (0 being empty)
     * \since QGIS 3.0
     */
    int dbRasterStylesViewCount() const { return mHasRasterStylesView; }

    /** Does the read Database contain any Styles (Bector or Raster)
     * \note
     *   - information about the Styles are stored in mStyleLayersInfo
     * \see readVectorRasterStyles
     * \see mStyleLayersInfo
     * \see getDbCoveragesStylesInfo
     * \since QGIS 3.0
     */
    int dbStylesCount() const { return mStyleLayersInfo.count(); }

    /** Amount of Topologies found in the Database
     * - from the topologies table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of Topology-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbTopologyExportLayersCount() const { return mHasTopologyExportTables; }

    /** Amount of MBTiles found in the Database
     * - from the metadata table Table [-1 if Table not found, otherwise 1]
     * \note
     * - this does not reflect the amount of MBTiles-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbMBTilesLayersCount() const { return mHasMBTilesTables; }

    /** Is the Gdal-MBTiles-Driver available ?
     * \note
     * - MBTiles can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalMBTilesDriver() const { return mHasGdalMBTilesDriver; }

    /** Amount of GeoPackage Layers found in the Database
     * - from the geopackage_contents Table [-1 if Table not found, otherwise amount]
     * \note
     * - this does not reflect the amount of GeoPackage-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbGeoPackageLayersCount() const { return mHasGeoPackageTables; }

    /** Amount of GeoPackage Vector-Layers found in the Database
     * - from the geopackage_contents Table [-1 if Table not found, otherwise amount]
     * \note
     * - this does not reflect the amount of GeoPackage-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbGeoPackageVectorsCount() const { return mHasGeoPackageVectors; }

    /** Amount of GeoPackage Vector-Layers found in the Database
     * - from the geopackage_contents Table [-1 if Table not found, otherwise amount]
     * \note
     * - this does not reflect the amount of GeoPackage-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbGeoPackageRastersCount() const { return mHasGeoPackageRasters; }

    /** Is the Gdal-GeoPackage-Driver available ?
     * \note
     * - GeoPackage can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalGeoPackageDriver() const { return mHasGdalGeoPackageDriver; }

    /** Amount of FdoOgr Layers found in the Database
     * - from the TODO Table [-1 if Table not found, otherwise amount]
     * \note
     * - this does not reflect the amount of FdoOgr-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbFdoOgrLayersCount() const { return mHasFdoOgrTables; }

    /** Is the Gdal-FdoOgr-Driver available (SQLite) ?
     * \note
     * - GeoPackage can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbFdoOgrDriver() const { return mHasFdoOgrDriver; }

    //! Is the used Spatialite compiled with Spatialite-Gcp support
    bool hasDbGcpSupport() const { return mHasGcp; }
    //! Is the used Spatialite compiled with Topology (and thus RtTopo) support
    bool hasDbTopologySupport() const { return mHasTopology; }
    //! Is the used Spatialite 4.5.0 or greater
    bool isDbVersion45() const { return mIsVersion45; }

    /** Loaded Layers-Counter
     * - contained in mDbLayers
     * -- all deatails of the Layer are known
     * \note
     * - only when GetSpatialiteDbInfoWrapper is called with LoadLayers=true
     * -- will all the Layers be loaded
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    int dbLayersCount() const { return mLayersCount; }

    /** Amount of Vector-Layers found
     * - SpatialTables, SpatialViews and VirtualShapes [from the vector_layers View]
     * -- contains: Layer-Name as 'table_name(geometry_name)', Geometry-Type (with dimension) and Srid
     * \note
     * - this amount may differ from dbLayersCount()
     * -- which only returns the amount of Loaded-Vector-Layers
     * \see dbLayersCount()
     * \since QGIS 3.0
     */
    int dbVectorLayersCount() const { return mVectorLayers.size(); }
    //! Flag indicating if the layer data source has ReadOnly restrictions
    bool isDbReadOnly() const { return mReadOnly; }
    //! Load all Layer-Information [default] or only 'sniff' the Database
    bool getDbLoadLayers() const { return mLoadLayers; }

    /** Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \since QGIS 3.0
     */
    bool isDbValid() const { return mIsValid; }

    /** Is the QgsSqliteHandle Connection to be shared
     * \see QgsSqliteHandle::openDb
     * \since QGIS 3.0
     */
    bool isConnectionShared() const { return mIsShared; }

    /** Set the QgsSqliteHandle Connection to be shared
     * \see QgsSqliteHandle::openDb
     * \since QGIS 3.0
     */
    void setConnectionShared( bool bIsShared ) { mIsShared = bIsShared; }

    /** Is the read Database a Spatialite Database
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \since QGIS 3.0
     */
    bool isDbSpatialite() const { return mIsSpatialite; }

    /** Does the Spatialite Database support Spatialite commands >= 4.0 ?
     * - avoids execution of such cammands
     * \note
     *  - InvalidateLayerStatistics
     * \since QGIS 3.0
     */
    bool isDbSpatialite40() const { return mIsSpatialite40; }

    /** Does the Spatialite Database support Spatialite commands >= 4.5 ?
     * - avoids execution of such cammands
     * \note
     *  - Topology
     * \since QGIS 3.0
     */
    bool isDbSpatialite45() const { return mIsSpatialite45; }

    /** The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool isDbGdalOgr() const { return mIsGdalOgr; }

    /** Does the file contain the Sqlite3 'Magic Header String'
     * - UTF-8 string "SQLite format 3" including the nul terminator character at the end.
     * \since QGIS 3.0
     */
    bool isDbSqlite3() const { return mIsSqlite3; }

    /**
      * Get SniffType
      *  - based on the Task, restrict unneeded activities
      *  -> For Browser: only Sqlite3-Container-Type is needed
      *  -> For Provider-Lists: only Tables/Layers without details
      *  -> For Provider: only Tables/Layers with all details
      * \note
      *  - SniffDatabaseType: Determineee Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer]
      *  - SniffExtendend: (planned) possible Editing of of Layer-Propeties such as Column-Names
      * \since QGIS 3.0
      */
    SpatialSniff getSniffType() const { return mSniffType; }

    /**
      * Set SniffType
      *  - based on the Task, restrict unneeded activities
      *  -> For Browser: only Sqlite3-Container-Type is needed
      *  -> For Provider-Lists: only Tables/Layers without details
      *  -> For Provider: only Tables/Layers with all details
      * \note
      *  - SniffDatabaseType: Determineee Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer]
      *  - SniffExtendend: (planned) possible Editing of of Layer-Propeties such as Column-Names
      * \since QGIS 3.0
      */
    void setSniffType( SpatialSniff sniffType ) {  mSniffType = sniffType;  }

    /** Set if checking has been done to insure that this is a Sqlite3 file
     * \see SpatialiteDbInfo::CreateSpatialiteConnection
     * \see SpatialiteDbInfo::readSqlite3MagicHeaderString
     * \since QGIS 3.0
     */
    void setDbSqlite3( bool bIsSqlite3 ) { mIsSqlite3 = bIsSqlite3; }

    /** Retrieve Map of corresponding to the given Layer-Type
     * - short list of the Layers retrieved from a Layer-Type
     * \note
     *  This is a convenience function
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see getDbVectorLayers()
     * \see getDbRasterCoveragesLayers()
     * \see getDbRasterLite1Layers()
     * \see getDbTopologyExportLayers
     * \see getDbMBTilesLayers()
     * \see getDbGeoPackageLayers()
     * \see getDbFdoOgrLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbLayersType( SpatialiteLayerType typeLayer = SpatialiteDbInfo::SpatialTable );

    /** Map of tables and views that are contained in the VectorLayers
     * - short list of the Layers retrieved from vector_layers
     *  - ordered by LayerType (SpatialTable, SpatialView and VirtualShape)
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readVectorLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbVectorLayers() const { return mVectorLayers; }

    /** Map of coverage_name that are contained in the RasterLite2 Vector-Layers
     * - short list of the Layers retrieved from vector_coverages
     *  - ordered by coverage_name
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readVectorRasterCoverages()
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbVectorCoveragesLayers() const { return mVectorCoveragesLayers; }

    /** Map of coverage_name that are contained in the RasterLite2 Raster-Layers
     * - short list of the Layers retrieved from raster_coverages
     *  - ordered by coverage_name
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readVectorRasterCoverages()
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbRasterCoveragesLayers() const { return mRasterCoveragesLayers; }

    /** Map of style_name and XML-Document
     * \note
     * - Key: StyleName as retrieved from SE_raster_styles
     * - Value: Style XML-Document
     * -> style_type: StyleRaster/StyleVector
     * - SELECT style_name, XB_GetDocument(style,1) FROM SE_vector/raster_styles;
     * \see
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbCoveragesStylesData() const { return mStyleLayersData; }

    /** Map of style_name and Metadata
     * \note
     * - Key: StyleName as retrieved from SE_vector_styles or SE_raster_styles
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleVector,StyleRaster
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbCoveragesStylesInfo() const { return mStyleLayersInfo; }

    /** Map of table_name that are contained in the RasterLite1Layers
     * - short list of the Layers retrieved from layer_statistics
     *  - Either:
     *  -> table_name with  raster_layer=1
     *  -> table_name_metadata with  raster_layer=0
     *  - Checks:
     *  -> TABLEs 'table_name'_metadata and 'table_name'_rasters must exist
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readRasterLite1Layers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbRasterLite1Layers() const { return mRasterLite1Layers; }

    /** List of topolayer_name that are contained in the  'topologies' TABLE
     * \note
     * - Used to retrieve Export Layers
     * \see readTopologyLayers()
     * \since QGIS 3.0
     */
    QStringList getDbTopologyNames() const { return mTopologyNames; }

    /** Map of topolayer_name that are contained in the Export-TopologyLayers
     * - Export Layers retrieved from topology_name of topologies
     *  -> and topology_name from 'topology_name'_topolayers TABLE
     * \note
     * - Key: LayerName formatted as 'topologies(topolayer_name)'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readTopologyLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbTopologyExportLayers() const { return mTopologyExportLayers; }

    /** Map of layers from metadata
     * - there can be only 1 Raster-Layer
     *  - no support for Grids due to lask of samples
     *  -> this must be rendered be the QgsGdalProvider
     * \note
     * - Key: LayerName taken from 'value' of the metadata table where field name = 'name'
     * - Value: GeometryType and 4326 as Srid formatted as 'layer_type:srid'
     * -> layer_type: MBTilesTable of MBTilesView
     * - Checking is done if the 'MBTiles' Gdal Driver is active
     * \see readMBTilesLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbMBTilesLayers() const { return mMBTilesLayers; }

    /** Map of table_name that are contained in geopackage_contents
     *  -> this must be rendered be the QgsGdalProvider or QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name'
     * - Value: LayerType and Srid formatted as 'layer_type:srid'
     * -> layer_type: GeoPackageVector or GeoPackageRaster
     * - Checking is done if the 'GPKG' Gdal/Ogr Driver is active
     * \see readGeoPackageLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbGeoPackageLayers() const { return mGeoPackageLayers; }

    /** Map of f_table_name that are contained in the TABLE geometry_columns for Gdal-FdoOgr
     *  - FdoOgr  geometry_columns contains different fields as the Spatialite Version
     *  -> this must be rendered with QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * - Checking is done if the SQLite' Gdal/Ogr Driver is active
     * \see readFdoOgrLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbFdoOgrLayers() const { return mFdoOgrLayers; }

    /** Map of table_name that are contained in geopackage_contents
     *  -> this must be rendered be the QgsGdalProvider or QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name'
     * - Value: LayerType and Srid formatted as 'layer_type:srid'
     * -> layer_type: GeoPackageVector or GeoPackageRaster
     * - Checking is done if the 'GPKG' Gdal/Ogr Driver is active
     * \see readGeoPackageLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbNonSpatialTables() const { return mNonSpatialTables; }

    /** Amount of SpatialTables  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialTables that have been loaded
     * \since QGIS 3.0
     */
    int dbNonSpatialTablesCount() const { return mNonSpatialTables.count(); }

    /** UpdateLayerStatistics for the Database or Layers
     * - this will be called from the SpatialiteDbLayer::UpdateLayerStatistics
     * - this is also called with a selection of tables/geometries
     *  -> calls InvalidateLayerStatistics before UpdateLayerStatistics
     * \note
     *  - if the sent List is empty or containe 1 empty QString
     *  -> the whole Database will be done
     *  - when only a specific table or table with geometry is to be done
     *  -> format: 'table_name(geometry_name)' or only 'table_name', with all of its geometries
     *  - otherwise all tables with geometries
     *  - All commands within a TRANSACTION
     * \param saLayers List of LayerNames formatted as 'table_name(geometry_name)'
     * \returns result of the last returned rc
     * \see SpatialiteDbLayer::UpdateLayerStatistics
     * \since QGIS 3.0
     */
    bool UpdateLayerStatistics( QStringList saLayers );

    /** Retrieve Map of valid Layers supported by this Database [mLayerName as Key]
     * - contains Layer-Name and a SpatialiteDbLayer-Pointer
     * \note
     * - contains all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * -- when  getDbLoadLayers is true, getSpatialiteDbLayer will load all layers
     * \returns mDbLayers Map of LayerNames and valid SpatialiteDbLayer entries
     * \see getDbLoadLayers
     * \since QGIS 3.0
     */
    QMap<QString, SpatialiteDbLayer *> getDbLayers() const { return mDbLayers; }

    /** Map of valid Selected Layers requested by the User
     * - only Uris that created a valid QgsVectorLayer/QgsRasterLayer
     * -> corresponding QgsMapLayer contained in  getSelectedDb??????Layers
     * \note
     * - Key: LayerName formatted as 'table_name(geometry)'
     * - Value: Uris dependent on provider
     * - can be for both QgsSpatiaLiteProvider or QgsGdalProvider
     * \returns mSelectedLayersUris  Map of LayerNames and  valid Layer-Uris entries
     * \see getDataSourceUris()
     * \see setSelectedDbLayers
     * \see getSelectedDbRasterLayers
     * \see getSelectedDbVectorLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString>  getSelectedLayersUris() const { return mSelectedLayersUris; }

    /** Add a list of database layers to the map
     * - to fill a Map of QgsVectorLayers and/or QgsRasterLayers
     * -> can be for both QgsSpatiaLiteProvider or QgsOgr/GdalProvider
     * \note
     * - requested by User to add to main QGis
     * -> emit addDatabaseLayers for each supported Provider
     * \param saSelectedLayers formatted as 'table_name(geometry_name)'
     * \param saSelectedLayersSql Extra Sql-Query for selected Layers
     * \returns amount of Uris entries
     * \see getSelectedLayersUris
     * \since QGIS 3.0
     */
    int addDbMapLayers( QStringList saSelectedLayers, QStringList saSelectedLayersSql );

    /** List of DataSourceUri of valid Layers
     * -  contains Layer-Name and DataSourceUri
     * \note
     * - Lists all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * - and MbTiles, FdoOgr and Geopoackage
     * - Key: LayerName formatted as 'table_name(geometry)'
     * - Value: dependent on provider
     * -> Spatialite: 'PathToFile table="table_name" (geometry_name)'
     * -> RasterLite2: [TODO] 'PathToFile table="coverage_name"'
     * -> RasterLite1 [Gdal]: 'RASTERLITE:/long/path/to/database/ItalyRail.atlas,table=srtm'
     * -> MBTiles [Gdal]: 'PathToFile'
     * -> FdoOgr [Ogr]:  'PathToFile|layername=table_name(geometry)'
     * -> GeoPackage [Ogr]: 'PathToFile|layername=table_name'
     * \see mDbLayersDataSourceUris
     * \see mDbLayers
     * \see getSpatialiteDbLayer
     * \see prepareDataSourceUris
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDataSourceUris() const { return mDbLayersDataSourceUris; }
    //! Resolve unset settings
    bool prepare();

    /** Retrieve SpatialiteDbLayer-Pointer for a given
     * -  Layer-Name or Uri of Layer
     * \note
     * - can retrieve all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * \param sLayerName Layer-Name or Uri of Layer
     * \param loadLayer when true [default], Load Layer-Information if not already loaded
     * \see getSpatialiteDbLayer
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *getSpatialiteDbLayer( QString sLayerName, bool loadLayer = true );

    /** Retrieve formatted Uris
     * - retrieved frommDbLayersDataSourceUris
     * \note
     * - created in prepareDataSourceUris
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    QString getDbLayerUris( QString sLayerName );

    /** Retrieve SpatialiteDbLayer-Pointer for a given
     * -  LayerId
     * \note
     * - LayerId is only valid for the active session and MUST not be used for project storing
     * \param sLayerName Layer-Name or Uri of Layer
     * \param loadLayer when true [default], Load Layer-Information if not already loaded
     * \see getSpatialiteDbLayer
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *getSpatialiteDbLayer( int layerId );

    /** Returns the enum  value from the given String of the Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialiteLayerType SpatialiteLayerTypeFromName( const QString &typeName );

    /** Returns the String value of the enum of Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteDbLayer::setLayerType
     * \since QGIS 3.0
     */
    static QString SpatialiteLayerTypeName( SpatialiteDbInfo::SpatialiteLayerType layerType );

    /** Returns the QIcon representation  of the enum of Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteDbLayer::setLayerType
     * \since QGIS 3.0
     */
    static QIcon SpatialiteLayerTypeIcon( SpatialiteDbInfo::SpatialiteLayerType layerType );

    /** Returns the QIcon representation  of non-Spatialial or Spatial-Admin TABLEs in a Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteDbLayer::setLayerType
     * \since QGIS 3.0
     */
    static QIcon NonSpatialTablesTypeIcon( QString typeName );

    /** Returns QIcon representation of the enum of a Geometry-Type
     * - QgsWkbTypes::Type
     * \note
     * - QgsWkbTypes::Point, Multi and 25D
     * - QgsWkbTypes::LineString, Multi and 25D
     * - QgsWkbTypes::Polygon, Multi and 25D
     * \see getSpatialiteTypes
     * \since QGIS 3.0
     */
    static QIcon SpatialGeometryTypeIcon( QgsWkbTypes::Type geometryType );

    /** Returns the String value of the enum of SpatialIndex-Types
     * \see SpatialiteDbLayer::setSpatialIndexType
     * \since QGIS 3.0
     */
    static QString SpatialIndexTypeName( SpatialiteDbInfo::SpatialIndexType spatialIndexType );

    /** Returns the enum  value from the given String of a Sqlite3 Container-Types
     * - SpatialiteDbInfo::SpatialMetadata
     * \note
     * - SpatialiteLegacy, SpatialiteFdoOgr, Spatialite40, SpatialiteGpkg, Spatialite45, SpatialiteMBTiles and SpatialUnknown
     * \param typeName enum as String
     * \returns SpatialMetadata enum
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialMetadata SpatialMetadataTypeFromName( const QString &typeName );

    /** Returns the String value of the enum of SpatialMetadata-Types (Sqlite3 Container-Types)
     * - SpatialiteDbInfo::SpatialMetadata
     * \note
     * - SpatialiteLegacy, SpatialiteFdoOgr, Spatialite40, SpatialiteGpkg, Spatialite45, SpatialiteMBTiles and SpatialUnknown
     * \param spatialMetadataType SpatialMetadata enum
     * \returns SpatialMetadata enum as String
     * \see SpatialiteDbInfo::setSpatialMetadata
     * \since QGIS 3.0
     */
    static QString SpatialMetadataTypeName( SpatialiteDbInfo::SpatialMetadata spatialMetadataType );

    /** Returns QIcon representation of the enum of SpatialMetadata-Types (Sqlite3 Container-Types)
     * - SpatialiteDbInfo::SpatialMetadata
     * \note
     * - SpatialiteLegacy, SpatialiteFdoOgr, Spatialite40, SpatialiteGpkg, Spatialite45, SpatialiteMBTiles and SpatialUnknown
     * \param spatialMetadataType SpatialMetadata enum
     * \returns SpatialMetadata enum as String
     * \see SpatialiteDbInfo::getSpatialMetadataIcon
     * \since QGIS 3.0
     */
    static QIcon SpatialMetadataTypeIcon( SpatialiteDbInfo::SpatialMetadata spatialMetadataType );

    /** Returns QIcon representation of the enum of SpatialMetadata-Types (Sqlite3 Container-Types)
     * - SpatialiteDbInfo::SpatialMetadata
     * \note
     * - SpatialiteLegacy, SpatialiteFdoOgr, Spatialite40, SpatialiteGpkg, Spatialite45, SpatialiteMBTiles and SpatialUnknown
     * \param spatialMetadataType SpatialMetadata enum
     * \returns SpatialMetadata enum as Icon
     * \see SpatialiteDbInfo::setSpatialMetadata
     * \since QGIS 3.0
     */
    QIcon getSpatialMetadataIcon() const { return SpatialiteDbInfo::SpatialMetadataTypeIcon( mSpatialMetadata ); }

    /** Returns the enum  value from the given String of a SpatialIndex-Types
     * \note
     * - at the moment not used but intended for external use
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialIndexType SpatiaIndexTypeFromName( const QString &typeName );

    /** List of Table-Types
     * -  with short description on how the Table is used
     * \note
     * - intended for gui use when listing tables, or as title of a sub-group where those type are stored in
     * \since QGIS 3.0
     */
    static QStringList getSpatialiteTypes();

    /** Retrieve Capabilities of spatialite connection
     * - at this point we are 'sniffing' the Capabilities of the opened Database
     * \note
     *  - when only a specific table or table with geometry are being looked for
     *  -> format: 'table_name(geometry_name)' or only 'table_name', with all of its geometries
     *  - otherwise all tables with geometries
     * \param sLayerName when used
     * \param bLoadLayers Load all Layer-Information or only 'sniff' [default] the Database
     * \returns SpatialiteDbInfo with collected results
     * \see QgsSLConnect
     * \since QGIS 3.0
     */
    bool GetSpatialiteDbInfo( QString sLayerName = QString::null, bool bLoadLayers = false, SpatialSniff sniffType = SpatialiteDbInfo::SniffUnknown );

    /** Retrieve Capabilities of spatialite connection
     * - at this point we are 'sniffing' the Capabilities of the opened Database
     * \note
     *  - when only a specific table or table with geometry are being looked for
     *  -> format: 'table_name(geometry_name)' or only 'table_name', with all of its geometries
     *  - otherwise all tables with geometries
     * \param sLayerName when used
     * \returns SpatialiteDbInfo with collected results
     * \see QgsSLConnect
     * \since QGIS 3.0
     */
    bool attachQSqliteHandle( QgsSqliteHandle *qSqliteHandle );
    //! Collection of warnings for the Layer being invalid
    QMap<QString, QString> getVectorLayersMissing() const { return mVectorLayersMissing; }

    /** Collection of warnings for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> getWarnings() const { return mWarnings; }

    /** Collection of reasons for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> getErrors() const { return mErrors; }

    /** The internal Parse Separator
     * - for general usage
     * \note
     *  - where it may be assumed that ':' is not used
     *  \see ParseSeparatorCoverage
     * \since QGIS 3.0
     */
    static const QString ParseSeparatorGeneral;

    /** The internal Parse Separator
     * - for general usage in Uril
     * \note
     *  - where it may be assumed that ':' is not used
     *  \see ParseSeparatorCoverage
     * \since QGIS 3.0
     */
    static const QString ParseSeparatorUris;

    /** The internal Parse Separator
     * - used for the Coverage/Style-Type/Title/Abstract/Copyright
     * \note
     *  - hoping that '€' will not be found in the Title/Abstract/Copyright Text
     *  \see readVectorRasterCoverages
     *  \see readVectorRasterStyles
     *  \see SpatialiteDbLayer::checkLayerStyles
     *  \see SpatialiteDbLayer::setLayerStyleSelected
     * \since QGIS 3.0
     */
    static const QString ParseSeparatorCoverage;

    /** Retrieve the selected Style
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only one Style can be rendered
     * \param sStyleName to retrieve
     * \returns xml QString of Xml-Document
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getDbStyleXml( QString sStyleName = QString::null )
    {
      if ( !mStyleLayersData.isEmpty() )
      {
        if ( mStyleLayersData.contains( sStyleName ) )
        {
          return mStyleLayersData.value( sStyleName );
        }
      }
      return QString();
    }

    /** Retrieve the selected Style
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - return QDomElement to be used with QgsVectorLayer::readSld
     *   - called while loading Layers
     * \param sStyleName to retrieve
     * \param errorMessage for messages when testing
     * \param iDebug to call test when set to 1, returning return code
     * \param sSaveXmlFileName filename to save xlm, when not null - for testing
     * \returns xlm QDomElement containing 'NamedLayer'
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QDomElement getDbStyleNamedLayerElement( QString sStyleName, QString &errorMessage, int *iDebug = nullptr, QString sSaveXmlFileName = QString::null );

    /** Create LayerInfo String and Uri
     * - intended a a central point where these setting should be made
     * - any changes in the Provirder Names or Uri-Syntax must bedone here
     * \note
     *  LayerInfo
     *  - field 0=Geometry-Type [for Raster-Types: LayerType]
     *  - field 1=Srid of the Layer
     *  - field 2=The Proverder Name
     *  DataSourceUri
     *  - Provider dependent
     * \param sLayerInfo String to return the result of the LayerInfo
     * \param DataSourceUri String to return the result of the DataSourceUri
     * \param sLayerName Name of the Layer to to use format: 'table_name(geometry_name)'
     * \param layerType the Layer-Type being formatted
     * \param sGeometryType the Layer Geometry-Type to use
     * \param iSrid the Layer Geometry-Type to use
     * \returns sLayerInfo
     * \see prepareDataSourceUris
     * \see SpatialiteDbLayer::prepare
     * \see readVectorLayers
     * \see readVectorRasterCoverages
     * \see GetTopologyLayersInfo
     * \see readRasterLite1Layers
     * \see readFdoOgrLayers
     * \see readGeoPackageLayers
     * \see readMBTilesLayers
     * \since QGIS 3.0
     */
    QString createDbLayerInfoUri( QString &sLayerInfo, QString &sDataSourceUri, QString sLayerName, SpatialiteDbInfo::SpatialiteLayerType layerType, QString sGeometryType, int iSrid );

    /** Parse the LayerInfo String
     * - intended a a central point where this task should be done
     * \note
     *  - the LayerInfo String is created in createDbLayerInfoUri
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * \param sLayerInfo the input to parse
     * \param sGeometryType the returned result from field 0 of sLayerInfo
     * \param iSrid the returned result from field 1 of sLayerInfo
     * \param sProvider the returned result from field 2 of sLayerInfo
     * \returns xlm QDomElement containing 'NamedLayer'
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    bool parseLayerInfo( QString sLayerInfo, QString &sGeometryType, int &iSrid, QString &sProvider );
  protected:
  signals:
    //! Add a list of database layers to the map
    void addDatabaseLayers( QStringList const &paths, QString const &providerKey );
  private:

    /** The Database filename (with Path)
    * \returns mDatabaseFileName complete Path (without without symbolic links)
    * \since QGIS 3.0
    */
    QString mDatabaseFileName;

    /** The Database filename without path
    * \returns mFileName  name of the file, excluding the path
    * \since QGIS 3.0
    */
    QString mFileName;

    /** The sqlite handler
     *  - created in QgsSqliteHandle
     * \note
     *  - closing done through QgsSqliteHandle
     * \see QgsSqliteHandle::openDb
     * \see attachQSqliteHandle
    * \since QGIS 3.0
    */
    sqlite3 *mSqliteHandle = nullptr;
    //! The class allowing to reuse the same sqlite handle for more layers
    QgsSqliteHandle *mQSqliteHandle = nullptr;
    //! The Spatialite internal Database structure being read
    SpatialiteDbInfo::SpatialMetadata mSpatialMetadata;
    //! The Spatialite internal Database structure being read (as String)
    QString mSpatialMetadataString;

    /** Set the Sqlite3-Container type being read (and the String)
     * \note
     *  - if not known, will check for MBTiles
     * \see getSniffDatabaseType
     * \see checkMBTiles
    * \since QGIS 3.0
    */
    void setSpatialMetadata( int iSpatialMetadata );

    /** The Spatialite Version Driver being used
     * \note
     *  - returned from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString mSpatialiteVersionInfo;

    /** The major Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mSpatialiteVersionMajor;

    /** The minor Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mSpatialiteVersionMinor;

    /** The revision Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mSpatialiteVersionRevision;

    /** Does the read Database contain SpatialTables, SpatialViews or VirtualShapes
     * - determine through Sql-Queries, without usage of any Drivers
     * -> for Databases created with a version before 4.0.0, vector_layers will be simulated
     * \note
     *   -1=no vector_layers entries, otherwise amount (0 being empty)
     * \see getSniffSniffMinimal
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasVectorLayers;

    /** Does the read Database contain SpatialTables, without vector_layers [ 0=none, otherwise amount]
     * - when set the VectorLayers logic will read geometry_columns instead of vector_layers
     * \note
     *  - final result will be set to VectorLayers
     *  -> so that externaly it is the same
     * \see getSniffSniffMinimal
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasLegacyGeometryLayers;

    /** Does the read Database contain SpatialTables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no Spatialite specific geometry_columns entries, otherwise amount (0 being empty)
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasSpatialTables;

    /** Does the read Database contain SpatialViews views
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no views_geometry_columns entries, otherwise amount (0 being empty)
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasSpatialViews;

    /** Does the read Database contain VirtualShapes tables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no virts_geometry_columns entries, otherwise amount (0 being empty)
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasVirtualShapes;

    /** Does the read Database contain RasterLite1 coverages
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no rasterlit1 logic found, otherwise amount (0 being empty)
     * \see getSniffSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readRasterLite1Layers
     * \see GetRasterLite1LayersInfo
     * \since QGIS 3.0
     */
    int mHasRasterLite1Tables;

    /** Is the Gdal-RasterLite1-Driver available ?
     * - GDALGetDriverByName( 'RasterLite' )
     * \note
     *  - QgsGdalProvider:  RasterLite1
     * \since QGIS 3.0
     */
    bool mHasGdalRasterLite1Driver;

    /** Does the read Database contain RasterLite2 Vector coverages
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no raster_coverages table, otherwise amount (0 being empty)
     * \see getSniffSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readVectorRasterCoverages()
     * \see GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    int mHasVectorCoveragesTables;

    /** Does the read Database contain RasterLite2 Raster coverages
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no raster_coverages table, otherwise amount (0 being empty)
     * \see getSniffSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readVectorRasterCoverages()
     * \see GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    int mHasRasterCoveragesTables;

    /** Is the Gdal-RasterLite2-Driver available ?
     * - GDALGetDriverByName( 'SQLite' )
     * \note
     *  - QgsGdalProvider:  RasterLite2
     *  - assuming Gdal has been compiled with Rasterite2 support
     * \see readVectorRasterCoverages()
     * \since QGIS 3.0
     */
    bool mHasGdalRasterLite2Driver;

    /** Does the read Database contain Topology tables ]
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no topologies table, otherwise amount (0 being empty)
     * \see getSniffSniffMinimal
     * \see getSniffReadLayers
     * \see readTopologyLayers
     * \see GetTopologyLayersInfo
     * \since QGIS 3.0
     */
    int mHasTopologyExportTables;

    /** Does the read Database contain MbTiles-Tables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none, otherwise 3=View-based MbTiles, else 2
     * \see checkMBTiles
     * \see readMBTilesLayers
     * \see GetMBTilesLayersInfo
     * \since QGIS 3.0
     */
    int mHasMBTilesTables;

    /** Is the Gdal-MBTiles-Driver available ?
     * - GDALGetDriverByName( 'MBTiles' )
     * \note
     *  - QgsGdalProvider:  MBTiles
     * \see readMBTilesLayers()
     * \since QGIS 3.0
     */
    bool mHasGdalMBTilesDriver;

    /** Does the read Database contain GPKG-Tables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none,  if 'gpkg_contents'  exists,  otherwise amount (0 being empty) [GeoPackage Revision 10]
     * \see getSniffSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readGeoPackageLayers
     * \see GetGeoPackageLayersInfo
     * \since QGIS 3.0
     */
    int mHasGeoPackageTables;

    /** Does the read Database contain GPKG-Vector-Tables [features]
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none,  if 'gpkg_contents'  exists,  otherwise amount (0 being empty) [GeoPackage Revision 10]
     * \see readGeoPackageLayers
     * \see GetGeoPackageLayersInfo
     * \since QGIS 3.0
     */
    int mHasGeoPackageVectors;

    /** Does the read Database contain GPKG-Vector-Tables [tiles]
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none,  if 'gpkg_contents'  exists,  otherwise amount (0 being empty) [GeoPackage Revision 10]
     * \see readGeoPackageLayers
     * \see GetGeoPackageLayersInfo
     * \since QGIS 3.0
     */
    int mHasGeoPackageRasters;

    /** Is the Gdal-GeoPackage-Driver available ?
     * - GDALGetDriverByName( 'GPKG' )
     * \note
     *  - QgsGdalProvider / QgsOgrProvider:  GPKG
     * \see readGeoPackageLayers
     * \since QGIS 3.0
     */
    bool mHasGdalGeoPackageDriver;

    /** Does the read Database contain FdoOgr-Tables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none,  if OGR specific 'geometry_columns'  exists,  otherwise amount (0 being empty)
     * \see getSniffSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readFdoOgrLayers
     * \see GetFdoOgrLayersInfo
     * \since QGIS 3.0
     */
    int mHasFdoOgrTables;

    /** Is the Gdal-FdoOgr-Driver available ?
     * - GDALGetDriverByName( 'SQLite' )
     * \note
     *  - QgsOgrProvider:  SQLite
     * \see readFdoOgrLayers
     * \since QGIS 3.0
     */
    bool mHasFdoOgrDriver;

    /** Does the read Database contain RasterLite2 SE_vector_styles_view
     * \note
     *   -1=no SE_vector_styles table, otherwise amount (0 being empty)
     * \see getSniffSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readVectorRasterCoverages()
     * \see dbVectorStylesViewCount
     * \since QGIS 3.0
     */
    int mHasVectorStylesView;

    /** Does the read Database contain RasterLite2 SE_raster_styles_view
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no SE_raster_styles, otherwise amount (0 being empty)
     * \see getSniffSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readVectorRasterCoverages
     * \see dbRasterStylesViewCount
     * \since QGIS 3.0
     */
    int mHasRasterStylesView;

    /** Has the used Spatialite compiled with Spatialite-Gcp support
     * - ./configure --enable-gcp=yes
     * \note
     *  - Based on GRASS GIS was initially released under the GPLv2+ license terms.
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=Ground+Control+Points
     * \see getSniffSniffMinimal
     * \since QGIS 3.0
     */
    bool mHasGcp;

    /** Has the used Spatialite compiled with Topology (and thus RtTopo) support
     * - ./configure --enable-rttopo
     * \note
     *  - Based on RT Topology Library
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=ISO+Topology
     * \see https://git.osgeo.org/gogs/rttopo/librttopo
     * \see getSniffSniffMinimal
     * \since QGIS 3.0
     */
    bool mHasTopology;

    /** Is the used connection Spatialite 4.5.0 or greater
     * \note
     *  - bsed on values offrom spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    bool mIsVersion45;

    /** Layers-Counter
     * - not all of which may be valid and contained in mDbLayers
     * \note
     *  - mDbLayers will only contain Layers that are valid
     * \see GetDbLayersInfo
    * \since QGIS 3.0
    */
    int mLayersCount;

    /** Flag indicating if the layer data source (Sqkite3) has ReadOnly restrictions
     * \note
     *  - sqlite3_db_readonly( mSqliteHandle, "main" )
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    bool mReadOnly;

    /** Load all Layer-Information [default]
     * \note
     *  -  or only 'sniff' the Database
     * \see GetSpatialiteDbInfo
    * \since QGIS 3.0
    */
    bool mLoadLayers;

    /** Is the read Database supported by QgsSpatiaLiteProvider or
     * - only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles, RasterLite1
     * \since QGIS 3.0
     */
    bool mIsValid;

    /** Is the QgsSqliteHandle Connection to be shared
     * \see QgsSqliteHandle::openDb
     * \since QGIS 3.0
     */
    bool mIsShared;

    /** Is the read Database a Spatialite Database
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \since QGIS 3.0
     */
    bool mIsSpatialite;

    /** Does the Spatialite Database support Spatialite commands >= 4?
     * - avoids execution of such cammands
     * \note
     *  - InvalidateLayerStatistics
     * \since QGIS 3.0
     */
    bool mIsSpatialite40;

    /** Does the Spatialite Database support Spatialite commands >= 4.5?
     * - avoids execution of such cammands
     * \note
     *  - Topology
     * \since QGIS 3.0
     */
    bool mIsSpatialite45;

    /** Is the read Database not supported by QgsSpatiaLiteProvider but
     * - supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool mIsGdalOgr;

    /** Does the file contain the Sqlite3 'Magic Header String'
     * - UTF-8 string "SQLite format 3" including the nul terminator character at the end.
     * \since QGIS 3.0
     */
    bool mIsSqlite3;

    /** Set the Database as invalid
     * - With possible Message, returns amount of Errors collected
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    int setDatabaseInvalid( QString sLayerName = QString::null, QString errCause = QString::null );

    /**
      * SniffType
      *  - based on the Task, restrict unneeded activities
      *  -> For Browser: only Sqlite3-Container-Type is needed
      *  -> For Provider-Lists: only Tables/Layers without details
      *  -> For Provider: only Tables/Layers with all details
      * \note
      *  - SniffDatabaseType: Determineee Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer]
      *  - SniffExtendend: (planned) possible Editing of of Layer-Propeties such as Column-Names
      * \since QGIS 3.0
      */
    SpatialSniff mSniffType;

    /** Retrieves basic information about contents of the Database
     * - Implementation of SpatialSniff::SniffDatabaseType
     * \note
     *  - Is the Database in a Read-Only status
     *  -> retrieved by sqlite3
     *  - Retrieve MetaData returned by Spatialite (including Spatialite Version-String)
     *  -> SpatialTables/Views/RasterLite2/1, Topology, GeoPackage and MBtiles
     * \returns mIsValid if no errors where found
     * \see GetSpatialiteDbInfo
     * \see setSpatialMetadata
     * \since QGIS 3.0
     */
    bool getSniffDatabaseType();

    /** Retrieves basic information about contents of the Database
     * - Implementation of SpatialSniff::SniffMinimal [Part 1]
     * \note
     *  - Parses Spatialite Version Information
     *  -> if Spatialite < 4.5, manually Determineeee if reading a Spatialite 4.5 Database
     *  -> setting of Spatialite Major, Minor and Refsion version numbers
     *  - Determine if Tables exist to Determineeee
     *  -> SpatialTables/Views/RasterLite2/1, Topology and GeoPackage
     *  - Determine Spatialite Capabilities
     *  -> HasTopology(), HasGCP()
     * \returns mIsValid if no errors where found
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool getSniffSniffMinimal( );

    /** Retrieves basic information about contents of the Database
     * - Implementation of SpatialSniff::SniffMinimal [Part 2]
     * \note
     *  getSniffSniffMinimal( ) will Determineeee catalog tables exist from which the Metadata will be read
     *  - Determine the amount of RaststerLite1 Tables that exist
     *  -> minimal check if needed subtables exist with valid values
     *  - Determine the amount of Tables that exist for
     *  -> SpatialTables/Views/VirtualShapes
     *  - Determine the amount of Tables that exist for
     *  -> SpatialTables/Views/VirtualShapes
     *  -> RasterLite2 Coverages
     *  -> Topologies
     *  -> Geopackage
     *  - Build a list of non-SpatialTables
     * \returns mIsValid if no errors where found
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool getSniffLayerMetadata( );

    /** Reading Layer-MetaTable data found, with sanity-checks
     * - Implementation of SpatialSniff::SniffMinimal [Part 2]
     * \note
     *  getSniffSniffMinimal( ) will Determineeee catalog tables exist from which the Metadata will be read
     *  - Determine the amount of Tables that exist for RaststerLite1 Tables
     *  -> minimal check if needed subtables exist with valid values
     *  - Determine the amount of Tables that exist for
     *  -> SpatialTables/Views/VirtualShapes
     *  - Determine the amount of Tables that exist for
     *  -> SpatialTables/Views/VirtualShapes
     *  -> RasterLite2 Coverages
     *  -> Topologies
     *  - Build a list of non-SpatialTables
     *  - Clears mDbLayers [Collection of Loaded-layers]
     *  -> and minimal information (Layer-Name and Geometry-Type) stored in mVectorLayers
     *  -  Activating Foreign Key constraints for the Database
     *  -> sqlite3 specific
     * \returns mIsValid if no errors where found
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool getSniffReadLayers();

    /** Retrieves extensive information about Layers in the Database
     * - Implementation of SpatialSniff::SniffLoadLayers
     * \note
     *  This is a convenience function, so that GetDbLayersInfo must not be called in an extra step
     *  - Calls GetDbLayersInfo
     *  -> if sLayerName is Empty
     *  -> all Layers will be loaded
     *  - if only a TABLE-Name is given
     *  -> all Layers with Geometries will be loaded
     *  - if 'table_name(geometry_name)' are given
     *  -> only that layer will be loaded
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns mIsValid if no errors where found
     * \see GetSpatialiteDbInfo
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool getSniffLoadLayers( QString sLayerName );

    /** Map of valid Layers supported by this Database [mLayerName as Key]
     * - contains Layer-Name and a SpatialiteDbLayer-Pointer
     * \note
     * - contains all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * \see getSpatialiteDbLayer
     * \since QGIS 3.0
     */
    QMap<QString, SpatialiteDbLayer *> mDbLayers;

    /** Function to remove known Admin-Tables from the Layers-List
     * - these TABLEs/VIEWs should NOT show up as User-Layer
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value:    Group-Name to be displayed in QgsSpatiaLiteTableModel as NonSpatialTables
     * - the found entries will be 'moved' to the mNonSpatialTables
     * - this is a helper function to simplify the task in the same way, everywhere where it is needed
     * \param layerTypes Layer and Group-Names to search for
     * \returns i_removed_count returns amount of entries deleted
     * \see mDbLayers
     * \see mNonSpatialTables
     * \see mVectorLayers
     * \see mVectorLayersTypes
     * \see GetRasterLite2RasterLayersInfo
     * \see GetRasterLite1LayersInfo
     * \see GetTopologyLayersInfo
     * \since QGIS 3.0
     */
    int removeAdminTables( QMap<QString, QString> layerTypes );

    /** Map of valid Selected Layers requested by the User
     * - only Uris that created a valid QgsVectorLayer/QgsRasterLayer
     * \note
     * - Key: LayerName formatted as 'table_name(geometry)'
     * - Value: Uris dependent on provider
     * - can be for both QgsSpatiaLiteProvider or QgsGdalProvider
     * \see getDataSourceUris()
     * \see setSelectedDbLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString>  mSelectedLayersUris;

    /** Map of tables and views that are not part of the Layers
     * - i.e. not Spatial-Tables or Spatial-Administration Tables
     * - containsTable-Name and Table-Type listed in mSpatialiteTypes
     * \see mSpatialiteTypes
     * \since QGIS 3.0
     */
    QMap<QString, QString> mNonSpatialTables;

    /** Map of tables and views that are contained in the VectorLayers
     * - short list of the Layers retrieved from vector_layers
     *  - ordered by LayerType (SpatialTable, SpatialView and VirtualShape)
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readVectorLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> mVectorLayers;

    /** Map of tables and views that are contained in the VectorLayers
     * - with the Layer-Type as Value
     *  - used to retrieve a specific Layer-Type contained in mVectorLayers
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: LayerType SpatialTable, SpatialView and VirtualShape
     * \see getDbLayersType
     * \since QGIS 3.0
     */
    QMap<QString, QString> mVectorLayersTypes;

    /** Map of coverage_name that are contained in the RasterLite2 Vector-Layers
     * - short list of the Layers retrieved from raster_coverages
     *  - ordered by coverage_name
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readVectorRasterCoverages()
     * \since QGIS 3.0
     */
    QMap<QString, QString> mVectorCoveragesLayers;

    /** Map of coverage_name that are contained in the RasterLite2 Raster-Layers
     * - short list of the Layers retrieved from raster_coverages
     *  - ordered by coverage_name
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readVectorRasterCoverages()
     * \since QGIS 3.0
     */
    QMap<QString, QString> mRasterCoveragesLayers;

    /** Map of table_name that are contained in the RasterLite1Layers
     * - short list of the Layers retrieved from layer_statistics
     *  - Either:
     *  -> table_name with  raster_layer=1
     *  -> table_name_metadata with  raster_layer=0
     *  - Checks:
     *  -> TABLEs 'table_name'_metadata and 'table_name'_rasters must exist
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readRasterLite1Layers
     * \since QGIS 3.0
     */
    QMap<QString, QString> mRasterLite1Layers;

    /** List of topolayer_name that are contained in the  'topologies' TABLE
     * \note
     * - Used to retrieve Export Layers
     * \see readTopologyLayers()
     * \since QGIS 3.0
     */
    QStringList mTopologyNames;

    /** Map of topolayer_name that are contained in the Export-TopologyLayers
     * - Export Layers retrieved from topology_name of topologies
     *  -> and topology_name from 'topology_name'_topolayers TABLE
     * \note
     * - Key: LayerName formatted as 'topologies(topolayer_name)'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider'
     * \see readTopologyLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> mTopologyExportLayers;

    /** Map of layers from metadata
     * - there can be only 1 Raster-Layer
     *  - no support for Grids due to lask of samples
     *  -> this must be rendered be the QgsGdalProvider
     * \note
     * - Key: LayerName taken from 'value' of the metadata table where field name = 'name'
     * - Value: GeometryType and 4326 as Srid formatted as 'layer_type:srid'
     * -> layer_type: MBTilesTable of MBTilesView
     * - Checking is done if the 'MBTiles' Gdal Driver is active
     * \see readMBTilesLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> mMBTilesLayers;

    /** Map of table_name that are contained in geopackage_contents
     *  -> this must be rendered be the QgsGdalProvider or QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name'
     * - Value: LayerType and Srid formatted as 'layer_type:srid'
     * -> layer_type: GeoPackageVector or GeoPackageRaster
     * - Checking is done if the 'GPKG' Gdal/Ogr Driver is active
     * \see readGeoPackageLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> mGeoPackageLayers;

    /** Map of Layers that are contained in Gdal-FdoOgr
     *  - // Not yet implemented
     *  -> this must be rendered be the  QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name'
     * - Value: LayerType and Srid formatted as 'layer_type:srid'
     * -> layer_type:
     * - Checking is done if the SQLite' Ogr Driver is active
     * \see readFdoOgrLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> mFdoOgrLayers;

    /** List of DataSourceUri of valid Layers
     * -  contains Layer-Name and DataSourceUri
     * \note
     * - Lists all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * - and MbTiles, FdoOgr and Geopoackage
     * - Key: LayerName formatted as 'table_name(geometry)'
     * - Value: dependent on provider
     * -> Spatialite: 'PathToFile table="table_name" (geometry_name)'
     * -> RasterLite2: [TODO] 'PathToFile table="coverage_name"'
     * -> RasterLite1 [Gdal]: 'RASTERLITE:/long/path/to/database/ItalyRail.atlas,table=srtm'
     * -> MBTiles [Gdal]: 'PathToFile'
     * -> FdoOgr [Ogr]:  'PathToFile|layername=table_name(geometry)'
     * -> GeoPackage [Ogr]: 'PathToFile|layername=table_name'
     * \see mDbLayersDataSourceUris
     * \see mDbLayers
     * \see getSpatialiteDbLayer
     * \see prepareDataSourceUris
     * \since QGIS 3.0
     */
    QMap<QString, QString> mDbLayersDataSourceUris;

    /** Map of style_name and XML-Document
     * \note
     * - Key: StyleName as retrieved from SE_raster_styles
     * - Value: Style XML-Document
     * -> style_type: StyleRaster/StyleVector
     * - SELECT style_name, XB_GetDocument(style,1) FROM SE_vector/raster_styles;
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<QString, QString> mStyleLayersData;

    /** Map of style_name and Metadata
     * \note
     * - Key: StyleName as retrieved from SE_vector_styles or SE_raster_styles
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleVector,StyleRaster
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<QString, QString> mStyleLayersInfo;

    /** Test NamedLayer-Element
     * - will emulate the testing done to validate the Xml for rendering in Qgis
     * - called from getDbStyleNamedLayerElement
     * \note
     *  Based on code found in
     * - QgsMapLayer::loadSldStyle
     * - QgsVectorLayer::readSld
     * - QgsFeatureRenderer::writeSld
     *  - [singleSymbol] QgsSingleSymbolRenderer::createFromSld
     *  - [singleSymbol] QgsSymbolLayerUtils::createSymbolLayerListFromSld
     *  - [RuleRenderer] QgsRuleBasedRenderer::createFromSld
     *  - [RuleRenderer] QgsRuleBasedRenderer::Rule::createFromSld
     * \param sStyleName to retrieve
     * \returns xlm QDomElement containing 'NamedLayer'
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    int testNamedLayerElement( QDomElement namedLayerElement,  QString &errorMessage );

    /** Retrieve Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * Spatial* : QString( "%1,table=%2 (%3)" ).arg( getDatabaseFileName() ).arg( sTableName ).arg(sGeometryColumn);
     * - gathers all information from gaiaGetVectorLayersList
     * -> complementary information will be retrieved from SpatialiteGetLayerSettingsWrapper
     * -> RasterLite2 information will be retrieved from SpatialiteGetRasterLite2RasterLayersInfoWrapper
     * -> Topology information will be retrieved from SpatialiteGetRasterLite2RasterLayersInfoWrapper
     * -> RasterLite1 information will be retrieved from SpatialiteGetRasterLite2RasterLayersInfoWrapper
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbConnectionInfo
     * \see SpatialiteGetLayerSettingsWrapper
     * \see SpatialiteGetRasterLite2RasterLayersInfoWrapper
     * \see SpatialiteGetTopologyLayersInfoWrapper
     * \see SpatialiteGetRasterLite1LayersInfoWrapper
     * \since QGIS 3.0
     */
    bool GetDbLayersInfo( QString sLayerName = QString::null );

    /** Determine if valid SpatialTable/Views and VirtualShapes Layers exist
     * - called only when mHasVectorLayers > 0 during GetSpatialiteDbInfo
     * \note
     * - results are stored in mVectorLayers
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool readVectorLayers();

    /** Retrieve RasterLite2 Vector/Raster Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * - results are stored in mVectorLayers
     * - there is no direct RasterLite2 support needed during this function
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool readVectorRasterCoverages();

    /** Retrieve RasterLite2 Vector/Raster Styles-Information
     * - used to fill list in SpatialiteDbLayer
     * \note
     * - As default: only load Styles that are used by the Layers [true]
     * - false=Retrieve all the Styles contained in the Database [may, one day, be needed]
     * \param bSelectUsedStylesOnly true=Retrieve only those Styles being used  (ignore the others)
     * \param bTestStylesForQgis true=call testNamedLayerElement from getDbStyleNamedLayerElement to test if valid for Qgis
     * \returns mStyleLayersData.count() amount of VectorRaster-Styles loaded
     * \see GetDbLayersInfo
     * \see mStyleLayersData
     * \see mStyleLayersInfo
     * \since QGIS 3.0
     */
    int readVectorRasterStyles( bool bSelectUsedStylesOnly = true, bool bTestStylesForQgis = false );

    /** Determine if valid RasterLite1 Layers exist
     * - called only when mHasRasterLite1Tables > 0 during GetSpatialiteDbInfo
     * \note
     * - results are stored in mRasterLite1Layers
     * - checking is done if the Gdal-Driver for RasterLite1 can be used
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool readRasterLite1Layers();

    /** Determine if valid Topology Layers exist
     * - called only when mHasTopologyExportTables > 0 during GetSpatialiteDbInfo
     * \note
     * - results are stored in mTopologyExportLayers
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool readTopologyLayers();

    /** Check the file contain the Sqlite3 'Magic Header String'
     * - UTF-8 string "SQLite format 3" including the nul terminator character at the end.
     * \note
     * - only called when
     * \since QGIS 3.0
     */
    static bool readSqlite3MagicHeaderString( QString sDatabaseFileName );

    /** Check if Database contains possible MbTiles-Tables
     * - sets mHasMBTilesTables
     * \note
     * \returns true mHasMBTilesTables=0
     * \since QGIS 3.0
     */
    bool checkMBTiles();

    /** Determine if valid MbTiles Layers exist
     * - called only when mHasMBTilesTables > 0 during GetSpatialiteDbInfo
     * \note
     * - results are stored in mMBTilesLayers
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool readMBTilesLayers();

    /** Determine if valid GeoPackage Layers exist
     * - called only when mHasGeoPackageTables > 0 during GetSpatialiteDbInfo
     * \note
     * - results are stored in mMBTilesLayers
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool readGeoPackageLayers();

    /** Determine if valid Gdal-FdoOgr Layers exist
     * - called only when mHasFdoOgrTables > 0 during GetSpatialiteDbInfo
     *  - FdoOgr  geometry_columns contains different fields as the Spatialite Version
     *  -> this must be rendered with QgsOgrProvider
     * \note
     * - results are stored in mFdoOgrLayers
     * - Checking is done if the SQLite' Gdal/Ogr Driver is active
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool readFdoOgrLayers();

    /** Retrieve RasterLite2 Vector-Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     *  RasterLite2 [gdal]: QString( "%3:%1:%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "RASTERLITE2" )
     * - there is no direct RasterLite2 support needed during this function
     * - at present (2017-07-31) QGdalProvider cannot read RasterLite2 created with the development version
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetRasterLite2VectorLayersInfo( const QString sLayerName = QString::null );

    /** Retrieve RasterLite2 Raster-Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     *  RasterLite2 [gdal]: QString( "%3:%1:%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "RASTERLITE2" )
     * - there is no direct RasterLite2 support needed during this function
     * - at present (2017-07-31) QGdalProvider cannot read RasterLite2 created with the development version
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetRasterLite2RasterLayersInfo( const QString sLayerName = QString::null );

    /** Retrieve Topology Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetTopologyLayersInfo( QString sLayerName = QString::null );

    /** Retrieve RasterLite1 Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     *  RasterLite1: QString( "%3:%1,table=%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "RASTERLITE" );
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetRasterLite1LayersInfo( QString sLayerName = QString::null );

    /** Retrieve MBTiles Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     *  MBTiles: QString( "%1" ).arg( getDatabaseFileName() )
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'name'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetMBTilesLayersInfo( QString sLayerName = QString::null );

    /** Retrieve GeoPackage Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     *  GeoPackageVector: QString( "%1|%3=%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "layername" );
     *  GeoPackageRaster: QString( "%3:%1:%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "GPKG" );
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetGeoPackageLayersInfo( QString sLayerName = QString::null );

    /** Retrieve FdoOgr Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     *  GdalFdoOgr: QString( "%1|%3=%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "layername" );
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetFdoOgrLayersInfo( QString sLayerName = QString::null );

    /** Retrieve and store Non-Spatial tables, views and all triggers
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool readNonSpatialTables( );

    /** Sanity checks on tables
     * -
     * \note
     *  In cases such as a 'SpatialView' that contains invalid sql-syntax:
     * - 'SQL error: 'table' is not a function' will be returned
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    int checkLayerSanity( QString sLayerName = QString::null );

    /** Builds list of missing layers
     * Layers found in vector_layers, but not resolved
     * \see mVectorLayersMissing
     * \since QGIS 3.0
     */
    bool prepareVectorLayersMissing();

    /** Builds Uri of layer
     * for use with QgsDataSourceUri
     * \see prepare
     * \since QGIS 3.0
     */
    bool prepareDataSourceUris();

    /** List of tables or views that could not be resolved
     * \note
     *  -> format: 'table_name(geometry_name)', value
     * \since QGIS 3.0
     */
    QMap<QString, QString> mVectorLayersMissing;

    /** Collection of warnings for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Warning text
     * \since QGIS 3.0
     */
    QMap<QString, QString> mWarnings;

    /** Collection of reasons for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> mErrors;
};

/** Structure to contain everything needed for a Spatialite/Rasterlite2 source
 * \returns point with all needed data
 * \since QGIS 3.0
 */
class SpatialiteDbLayer : public QObject
{
    Q_OBJECT
  public:
    SpatialiteDbLayer( SpatialiteDbInfo *dbConnectionInfo )
      : mDbConnectionInfo( dbConnectionInfo )
      , mDatabaseFileName( dbConnectionInfo->getDatabaseFileName() )
      , mTableName( QString::null )
      , mGeometryColumn( QString::null )
      , mLayerName( QString::null )
      , mCoverageName( QString::null )
      , mViewTableName( QString::null )
      , mViewTableGeometryColumn( QString::null )
      , mLayerViewTableName( QString::null )
      , mTitle( QString::null )
      , mAbstract( QString::null )
      , mCopyright( QString::null )
      , mSrid( -2 )
      , mAuthId( QString::null )
      , mProj4text( QString::null )
      , mSpatialIndexType( SpatialiteDbInfo::SpatialIndexNone )
      , mLayerType( SpatialiteDbInfo::SpatialiteUnknown )
      , mLayerTypeString( QString::null )
      , mLayerInfo( QString::null )
      , mDataSourceUri( QString::null )
      , mIsSpatialite( false )
      , mGeometryType( QgsWkbTypes::Unknown )
      , mGeometryTypeString( QString::null )
      , mCoordDimensions( GAIA_XY )
      , mLayerExtent( QgsRectangle( 0.0, 0.0, 0.0, 0.0 ) )
      , mLayerExtentEWKT( QString::null )
      , mLayerExtentWsg84( QgsRectangle( 0.0, 0.0, 0.0, 0.0 ) )
      , mLayerExtentWsg84EWKT( QString::null )
      , mLayerStyleSelected( QString::null )
      , mLayerStyleSelectedType( QString::null )
      , mLayerStyleSelectedTitle( QString::null )
      , mLayerStyleSelectedAbstract( QString::null )
      , mNumberFeatures( 0 )
      , mLayerReadOnly( 1 )
      , mLayerIsHidden( 0 )
      , mTriggerInsert( false )
      , mTriggerUpdate( false )
      , mTriggerDelete( false )
      , mLayerId( -1 )
      , mEnabledCapabilities( QgsVectorDataProvider::NoCapabilities )
      , mQuery( QString::null )
      , mPrimaryKey( QString::null )
      , mIsValid( false )
      , mHasStyle( false )
    {}
    ~SpatialiteDbLayer();
    //! The Database Info-Structure being read
    SpatialiteDbInfo *getDbConnectionInfo() const { return mDbConnectionInfo; }
    //! The sqlite handler
    sqlite3 *dbSqliteHandle() const { return getDbConnectionInfo()->dbSqliteHandle(); }
    //! The Database filename being read
    QString getDatabaseFileName() const { return mDatabaseFileName; }

    //! Name of the table with no schema
    QString getTableName() const { return mTableName; }

    /** Name of the geometry column in the table
     * \note
     *  Vector: should always be filled
     *  Raster: may be empty
     * \since QGIS 3.0
     */
    QString getGeometryColumn() const { return mGeometryColumn; }

    /** Layer-Name
     * \note
     *  Vector: Layer format: 'table_name(geometry_name)'
     *  Raster: Layer format: 'coverage_name'
     * \since QGIS 3.0
     */
    QString getLayerName() const { return mLayerName; }

    /** Coverage-Name [internal]
     * \note
     *  retrieved from vector_coverages, which may be different from the Layer-Name
     * \since QGIS 3.0
     */
    QString getCoverageName() const { return mCoverageName; }
    //! Name of the table which contains the SpatialView-Geometry (underlining table)
    QString getViewTableName() const { return mViewTableName; }
    //! Name of the table-geometry which contains the SpatialView-Geometry (underlining table)
    QString getViewTableGeometryColumn() const { return mViewTableGeometryColumn; }
    //! Name of the Layer format: 'table_name(geometry_name)'
    QString getLayerViewTableName() const { return mLayerViewTableName; }
    //! Title [RasterLite2]
    QString getTitle() const { return mTitle; }
    //! Title [RasterLite2]
    QString getAbstract() const { return mAbstract; }
    //! Copyright [RasterLite2]
    QString getCopyright() const { return mCopyright; }
    //! The Srid of the Geometry
    int getSrid() const { return mSrid; }
    //! The Srid of the Geometry
    QString getSridEpsg() const { return QString( "EPSG:%1" ).arg( mSrid ); }

    /** UpdateLayerStatistics for the Layer
     * - calls SpatialiteDbInfo::UpdateLayerStatistics
     * \note
     *  - to call for all geometries of a Table, call SpatialiteDbInfo::UpdateLayerStatistics
     *  -- with the Table-Name only
     * \returns result of the internal Spatalite function update_layer_statistics
     * \see SpatialiteDbInfo::UpdateLayerStatistics
     * \since QGIS 3.0
     */
    bool UpdateLayerStatistics();

    /** Set Srid, retrieving information for
     * Goal: to (semi) Automate unresolved setting when needed
     * \see mSrid
     * \see mAuthId
     * \see mProj4text
     * \since QGIS 3.0
     */
    bool setSrid( int iSrid );
    //! AuthId [auth_name||':'||auth_srid]
    QString getAuthId() const { return mAuthId; }
    //! Proj4text [from mSrid]
    QString getProj4text() const { return mProj4text; }
    //! The SpatialiIndex used for the Geometry
    SpatialiteDbInfo::SpatialIndexType getSpatialIndexType() const { return mSpatialIndexType; }
    //! The SpatialiIndex used for the Geometry
    QString getSpatialIndexString() const { return mSpatialIndexTypeString; }
    //! The Spatialite Layer-Type being read
    SpatialiteDbInfo::SpatialiteLayerType getLayerType() const { return mLayerType; }
    //! The Spatialite Layer-Type being read (as String)
    QString getLayerTypeString() const { return mLayerTypeString; }

    /** Set the Layer-InfoString
     * \note
     *  - field 0=Geometry-Type [for Raster-Types: LayerType]
     *  - field 1=Srid of the Layer
     *  - field 2=The Proverder Name
     * \see prepare
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString getLayerInfo() const { return mLayerInfo; }

    /** Set the  Layer-DataSourceUri String
     * - Proverder dependent
     * \see prepare
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString getLayerDataSourceUri() const { return mDataSourceUri; }

    /** Is the Layer supported by QgsSpatiaLiteProvider
     * - QgsSpatiaLiteProvider should never accept a Layer when this is not true
     * \note
     *  - check for valid Layer in the provider
     * \see setLayerType
     * \see QgsSpatiaLiteProvider::setDbLayer
     * \since QGIS 3.0
     */
    bool isLayerSpatialite() const { return mIsSpatialite; }

    /** Returns QIcon representation of the enum of Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteLayerTypeNameIcon
     * \since QGIS 3.0
     */
    QIcon getLayerTypeIcon() const { return SpatialiteDbInfo::SpatialiteLayerTypeIcon( mLayerType ); }
    //! The Spatialite Geometry-Type being read
    QgsWkbTypes::Type getGeometryType() const { return mGeometryType; }
    //! The Spatialite Geometry-Type being read (as String)
    QString getGeometryTypeString() const { return mGeometryTypeString; }

    /** Return the QgsLayerItem Icon for the Spatialite Geometry-Type
     * \since QGIS 3.0
     */
    QIcon getGeometryTypeIcon() const { return SpatialiteDbInfo::SpatialGeometryTypeIcon( mGeometryType ); }
    //! The Spatialite Coord-Dimensions
    int getCoordDimensions() const { return mCoordDimensions; }

    /** Rectangle that contains the extent (bounding box) of the layer
     * \note
     *  With UpdateLayerStatistics the Number of features will also be updated and retrieved
     * \param bUpdate force reading from Database
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see mLayerExtent
     * \see mNumberFeatures
     * \since QGIS 3.0
     */
    QgsRectangle getLayerExtent( bool bUpdate = false, bool bUpdateStatistics = false );

    /** Set the Rectangle that contains the extent (bounding box) of the layer
     *  - will also set the EWKT
     * \note
     *  The Srid must be set beforhand for the correct result
     * \see mLayerExtent
     * \see mLayerExtentEWKT
     * \since QGIS 3.0
     */
    QString setLayerExtent( QgsRectangle layerExtent )
    {
      mLayerExtent = layerExtent;
      mLayerExtentEWKT = QString( "'SRID=%1;%2'" ).arg( getSrid() ).arg( mLayerExtent.asWktPolygon() );
      return mLayerExtentEWKT;
    }

    /** Set the Wsg84 Rectangle that contains the extent (bounding box) of the layer
     *  - will also set the EWKT
     * \note
     *  This is a part of the vector_coverages logic
     * \see mLayerExtentWsg84
     * \see mLayerExtentWsg84EWKT
     * \since QGIS 3.0
     */
    QString setLayerExtentWsg84( QgsRectangle layerExtent )
    {
      mLayerExtentWsg84 = layerExtent;
      mLayerExtentWsg84EWKT = QString( "'SRID=%1;%2'" ).arg( 4326 ).arg( mLayerExtent.asWktPolygon() );
      return mLayerExtentWsg84EWKT;
    }

    /** Rectangle that contains the extent (bounding box) of the layer, with Srid
     * \note
     * \since QGIS 3.0
     */
    QString getLayerExtentEWKT() const { return mLayerExtentEWKT; }

    /** Rectangle that contains the extent (bounding box) of the layer, with Srid
     * \note
     * \since QGIS 3.0
     */
    QString getLayerExtentWsg84EWKT() const { return mLayerExtentWsg84EWKT; }

    /** The selected Style
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getLayerStyleSelected() const { return mLayerStyleSelected; }

    /** The selected Style
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - will set the StyleType, Title and Abstract
     *  \see setLayerStyleSelected
     *  \see  getLayerStyleSelectedType
     *  \see  getLayerStyleSelectedTitle
     *  \see  getLayerStyleSelectedAbstract
     * \since QGIS 3.0
     */
    QString setLayerStyleSelected( QString sStyle = QString::null );

    /** Retrieve the selected Style as Xml-String
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only convenience function for outside use
     *  - getLayerStyleNamedLayerElement  will be used in most cases
     * \param sStyleName to retrieve
     * \returns xml QString of Xml-Document
     * \see setLayerStyleSelected
     * \see SpatialiteDbInfo::getDbStyleXml
     * \see getLayerStyleNamedLayerElement
     * \since QGIS 3.0
     */
    QString getLayerStyleXml( QString sStyleName = QString::null )
    {
      if ( !mStyleLayersInfo.isEmpty() )
      {
        if ( sStyleName == QString::null )
        {
          sStyleName = mLayerStyleSelected;
        }
        return getDbConnectionInfo()->getDbStyleXml( sStyleName );
      }
      return QString();
    }

    /** Retrieve the selected Style
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - return QDomElement to be used with QgsVectorLayer::readSld
     *   - called while loading Layers
     * \param sStyleName to retrieve
     * \returns xlm QDomElement containing 'NamedLayer' stored in SpatialiteDbInfo
     * \see setLayerStyleSelected
     * \see SpatialiteDbInfo::getDbStyleNamedLayerElement
     * \see SpatialiteDbInfo::
     * \since QGIS 3.0
     */
    QDomElement getLayerStyleNamedLayerElement( QString sStyleName = QString::null )
    {
      if ( !mStyleLayersInfo.isEmpty() )
      {
        if ( sStyleName == QString::null )
        {
          sStyleName = mLayerStyleSelected;
        }
        QString errorMessage;
        return getDbConnectionInfo()->getDbStyleNamedLayerElement( sStyleName, errorMessage, nullptr, QString::null );
      }
      return QDomElement();
    }

    /** Does this Layer contain a Style
     * \note
     *  -  which can be retrieved with getLayerStyle
     * \see mLayerStyleSelected
     * \see getLayerStyle
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    bool hasLayerStyle() const { return mHasStyle; }

    /** The selected Style: Type
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - StyleVector or StyleRaster
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getLayerStyleSelectedType() const { return mLayerStyleSelectedType; }

    /** The selected Style: Title
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getLayerStyleSelectedTitle() const { return mLayerStyleSelectedTitle; }

    /** The selected Style: Abstract
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getLayerStyleSelectedAbstract() const { return mLayerStyleSelectedAbstract; }

    /** Number of features in the layer
     * \note
     *  With UpdateLayerStatistics the Extent will also be updated and retrieved
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see mLayerExtent
     * \see mNumberFeatures
     * \since QGIS 3.0
     */
    long getNumberFeatures( bool bUpdateStatistics = false );
    //! The Spatialite Layer-Readonly status [true or false]
    int isLayerReadOnly() const { return mLayerReadOnly; }
    //! The Spatialite Layer-Hidden status [true or false]
    int isLayerHidden() const { return mLayerIsHidden; }
    //! The Spatialite Layer-Id being created
    int getLayerId() const { return mLayerId; }

    /** Based on Layer-Type, set QgsVectorDataProvider::Capabilities
     * - Writable Spatialview: based on found TRIGGERs
     * \note
     * - this should be called after the LayerType and PrimaryKeys have been set
     * \note
     * The following receive: QgsVectorDataProvider::NoCapabilities
     * - SpatialiteTopology: will serve only TopologyExport, which ate SpatialTables
     * - VectorStyle: nothing as yet
     * - RasterStyle: nothing as yet
     * \note
     * - this should be called with Update, after alterations of the TABLE have been made
     * -> will call GetDbLayersInfo to re-read field data
     * \param bUpdate force reading from Database
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool checkLayerStyles( );

    /** Map of style_name and Metadata
     * \note
     * - Key: StyleName as retrieved from SE_vector_styles or SE_raster_styles
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleVector,StyleRaster
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<QString, QString> getLayerCoverageStylesInfo() const { return mStyleLayersInfo; }

    /** Based on Layer-Type, set QgsVectorDataProvider::Capabilities
     * - Writable Spatialview: based on found TRIGGERs
     * \note
     * - this should be called after the LayerType and PrimaryKeys have been set
     * \note
     * The following receive: QgsVectorDataProvider::NoCapabilities
     * - SpatialiteTopology: will serve only TopologyExport, which ate SpatialTables
     * - VectorStyle: nothing as yet
     * - RasterStyle: nothing as yet
     * \note
     * - this should be called with Update, after alterations of the TABLE have been made
     * -> will call GetDbLayersInfo to re-read field data
     * \param bUpdate force reading from Database
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    QgsVectorDataProvider::Capabilities getCapabilities( bool bUpdate = false );
    //! A possible Query from QgsDataSourceUri
    QString getLayerQuery() const { return mQuery; }
    //! A possible Query from QgsDataSourceUri
    void setLayerQuery( QString sQuery ) { mQuery = sQuery; }
    //! Name of the primary key column in the table
    QString getPrimaryKey() const { return mPrimaryKey; }
    //! List of primary key columns in the table
    QgsAttributeList getPrimaryKeyAttrs() const { return mPrimaryKeyAttrs; }
    //! List of layer fields in the table
    QgsFields getAttributeFields() const { return mAttributeFields; }
    //! Map of field index to default value [for Topology, the Topology-Layers]
    QMap<int, QVariant> getDefaultValues() const { return mDefaultValues; }

    /** Collection of warnings for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> getWarnings() const { return mWarnings; }

    /** Collection of reasons for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> getErrors() const { return mErrors; }
    //! Is the Layer valid
    bool isLayerValid() const { return mIsValid; }

    /** Resolve unset settings
     * Goal: to (semi) Automate unresolved settings when needed
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \see prepareCapabilities
     * \see checkLayerStyles
     * \returns mIsValid if the Layer is considered valid
     * \since QGIS 3.0
     */
    bool prepare();
  private:
    //! The Database Info-Structure being read
    SpatialiteDbInfo *mDbConnectionInfo = nullptr;;
    //! The Database filename being read
    QString mDatabaseFileName;
    //! Name of the table with no schema
    QString mTableName;

    /** Name of the geometry column in the table
     * \note
     *  Vector: should always be filled
     *  Raster: may be empty
     * \since QGIS 3.0
     */
    QString mGeometryColumn;

    /** Layer-Name
     * \note
     *  Vector: Layer format: 'table_name(geometry_name)'
     *  Raster: Layer format: 'coverage_name'
     * \since QGIS 3.0
     */
    QString mLayerName;

    /** Coverage-Name [internal]
     * \note
     *   retrieved from vector_coverages, which may be different from the Layer-Name
     + - this can be empty
     * \since QGIS 3.0
     */
    QString mCoverageName;
    //! Name of the table which contains the SpatialView-Geometry (underlining table)
    QString mViewTableName;
    //! Name of the table-geometry which contains the SpatialView-Geometry (underlining table)
    QString mViewTableGeometryColumn;
    //! Name of the Layer format: 'table_name(geometry_name)'
    QString mLayerViewTableName;

    /** Title of the Layer
     * - based on Provider specific values
     * \note
     *  - RasterLite2: Column title of 'raster_coverages'
     *  - MbTiles: Column  value of 'metadata' WHERE (name='description')
     *  - GeoPackage: Column identifier of 'gpkg_contents'
     * \since QGIS 3.0
     */
    QString mTitle;

    /** Abstract of the Layer
     * - based on Provider specific values
     * \note
     *  - RasterLite2: Column abstract of 'raster_coverages'
     *  - MbTiles: Column  value of 'metadata' WHERE (name='name')
     *  - GeoPackage: Column description of 'gpkg_contents'
     * \since QGIS 3.0
     */
    QString mAbstract;

    /** Copyright of the Layer
     * - based on Provider specific values
     * \note
     *  - RasterLite2: Column copyright of 'raster_coverages'
     *  - MbTiles: nothing
     *  - GeoPackage: nothing
     * \since QGIS 3.0
     */
    QString mCopyright;

    /** The Srid of the Geometry
     *  \see setSrid
     * \since QGIS 3.0
     */
    int mSrid;

    /** The AuthId [auth_name||':'||auth_srid]
     *  \see setSrid
     * \since QGIS 3.0
     */
    QString mAuthId;

    /** The Proj4text [from mSrid]
     *  \see setSrid
     * \since QGIS 3.0
     */
    QString mProj4text;

    /** The SpatialIndex-Type used for the Geometry
     * \see setSpatialIndexType
     * \see mSpatialIndexTypeString
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialIndexType mSpatialIndexType;

    /** The SpatialIndex-Type used for the Geometry as String
     * \see setSpatialIndexType
     * \see mSpatialIndexType
     * \since QGIS 3.0
     */
    QString mSpatialIndexTypeString;

    /** Set the SpatialIndex-Type used for the Geometry
     * - will also set the String version of theSpatialIndex-Type
     * \see SpatialIndexTypeName
     * \see mSpatialIndexType
     * \since QGIS 3.0
     */
    void setSpatialIndexType( int iSpatialIndexType );

    /** The Spatialite Layer-Type of the Layer
     *  \see setGeometryType
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialiteLayerType mLayerType;

    /** Set the Spatialite Layer-Type String
     * - representing mLayerType
     *  \see setLayerType
     * \see SpatialiteLayerTypeName
     * \since QGIS 3.0
     */
    QString mLayerTypeString;

    /** Set the Layer-InfoString
     * \note
     *  - field 0=Geometry-Type [for Raster-Types: LayerType]
     *  - field 1=Srid of the Layer
     *  - field 2=The Proverder Name
     * \see prepare
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString mLayerInfo;

    /** Set the  Layer-DataSourceUri String
     * - Proverder dependent
     * \see prepare
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString mDataSourceUri;

    /** Set the Spatialite Layer-Type
     * - will also set the String version of the Layer-Type
     * \note
     *  - will set whether this Layer is supported by the QgsSpatiaLiteProvider
     * \see SpatialiteLayerTypeName
     * \see mLayerTypeString
     * \see mIsSpatialite
     * \since QGIS 3.0
     */
    void setLayerType( SpatialiteDbInfo::SpatialiteLayerType layerType );

    /** Is the Layer
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \since QGIS 3.0
     */
    bool mIsSpatialite;

    /** The Spatialite Geometry-Type of the Layer
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see setGeometryType
     * \since QGIS 3.0
     */
    QgsWkbTypes::Type mGeometryType;

    /** Set the Spatialite Geometry-Type String
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see setGeometryType
     * \since QGIS 3.0
     */
    QString  mGeometryTypeString;

    /** Set the Spatialite Geometry-Type
     * - will also set the String version of the Geometry-Type
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    void setGeometryType( int iGeomType );

    /** The Spatialite Coord-Dimensions of the Layer
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    int mCoordDimensions;

    /** Rectangle that contains the extent (bounding box) of the layer
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QgsRectangle mLayerExtent;

    /** The extent (bounding box) of the layer AS Extended Well Known Text
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QString mLayerExtentEWKT;

    /** Rectangle that contains the extent (bounding box) of the layer
     * - based on the Wsg84 value of the Layer found in vector_coverages
     * \note
     *  - part of the vector_coverages logic
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QgsRectangle mLayerExtentWsg84;

    /** The extent (bounding box) of the layer AS Extended Well Known Text
     * - based on the Wsg84 value of the Layer found in vector_coverages
     * \note
     *  - part of the vector_coverages logic
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QString mLayerExtentWsg84EWKT;

    /** The selected Style
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     *  \see getLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mLayerStyleSelected;

    /** The selected Style: Type
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - StyleVector or StyleRaster
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mLayerStyleSelectedType;

    /** The selected Style: Title
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     *  \see getLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mLayerStyleSelectedTitle;

    /** The selected Style: Abstract
     * - by default: the first entry of mStyleLayersData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     *  \see getLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mLayerStyleSelectedAbstract;

    //! Number of features in the layer
    long mNumberFeatures;
    //! The Spatialite Layer-Readonly status [true or false]
    int mLayerReadOnly;
    //! The Spatialite Layer-Hidden status [true or false]
    int mLayerIsHidden;
    //! Flag indicating if the Capabilities SpatialView supports Inserting since QGIS 3.0
    bool mTriggerInsert;
    //! Flag indicating if the Capabilities SpatialView supports Updating since QGIS 3.0
    bool mTriggerUpdate;
    //! Flag indicating if the Capabilities SpatialView supports Deleting since QGIS 3.0
    bool mTriggerDelete;
    //! The Spatialite Layer-Id being created
    int mLayerId;
    //! The Spatialite Layer Capabilities
    QgsVectorDataProvider::Capabilities mEnabledCapabilities;
    //! Name of the table or subquery
    QString mQuery;
    //! Name of the primary key column in the table
    QString mPrimaryKey;
    //! Is the Layer valid
    bool mIsValid;
    //! Set the Layer as invalid, with possible Message, returns amount of Errors collected
    int setLayerInvalid( QString sLayerName = QString::null, QString errCause = QString::null );
    //! List of primary key columns in the table
    QgsAttributeList mPrimaryKeyAttrs;
    //! List of layer fields in the table
    QgsFields mAttributeFields;
    //! Map of field index to default value [for Topology, the Topology-Layers]
    QMap<int, QVariant> mDefaultValues;

    /** Map of valid Topology-ExportLayers
     * - contains Layer-Name and a SpatialiteDbLayer-Pointer
     * \note
     * - these are exported SpatialTables [Topology-Features (Metadata) and Geometry]
     * see chapter: TopoGeo_ExportTopoLayer: exporting a full TopoLayer into a GeoTable
     * \returns mTopologyExportLayers as QgsWkbTypes::Type
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=topo-advanced#ExportTopoLayer
     * \since QGIS 3.0
     */
    QMap<QString, SpatialiteDbLayer *> mTopologyExportLayers;

    /** Map of mDataSourceUri of mTopologyExportLayers
     * -  contains Layer-Name and DataSourceUri
     * \note
     * - Topology-Features (Metadata) and Geometry
     * \since QGIS 3.0
     */
    QMap<QString, QString> mTopologyExportLayersDataSourceUris;

    /** Convert Spatialite GeometryType to QgsWkbTypes::Type
     * \note
     * - this also be used to interpret the type field returned by pragma table_info
     * \param spatialiteGeometryType Spatialite-GeometryType
     * \param spatialiteGeometryDimension Spatialite-Dimensions
     * \returns geomType as QgsWkbTypes::Type
     * \see GetRasterLite1LayersInfo
     * \see GetSpatialiteQgsField
     * \since QGIS 3.0
     */
    static QgsWkbTypes::Type GetGeometryTypeLegacy( const QString spatialiteGeometryType, const QString spatialiteGeometryDimension = QString::null );

    /** Convert Spatialite GeometryType to QgsWkbTypes::Type
     * \note
     * - this also be used to interpret the type field returned by pragma table_info
     * \param sName from field name returned by pragma table_info
     * \param sType from field type returned by pragma table_info
     * \returns geomType as QgsWkbTypes::Type
     * \see setSpatialiteAttributeFields
     * \see GetSpatialiteQgsField
     * \since QGIS 3.0
     */
    static QgsField GetSpatialiteQgsField( const QString sName, const QString sType, const QString sDefaultValue, QVariant &defaultVariant );

    /** Read and fill AttributeFields and Default values
     * \note
     * - SpatialView
     * \param sName from field name returned by pragma table_info
     * \param sType from field type returned by pragma table_info
     * \returns geomType as QgsWkbTypes::Type
     * \see GetSpatialiteQgsField
     * \see mPrimaryKey
     * \see mPrimaryKeyAttrs
     * \see mAttributeFields
     * \see mDefaultValues
     * \since QGIS 3.0
     */
    int setSpatialiteAttributeFields( );

    /** Convert Spatialite GeometryType to QgsWkbTypes::Type
     * \param spatialiteGeometryType Spatialite-GeometryType
     * \param spatialiteGeometryDimension Spatialite-Dimensions
     * \returns geomType as QgsWkbTypes::Type
     * \since QGIS 3.0
     */
    static QgsWkbTypes::Type GetGeometryType( const int spatialiteGeometryType, const int spatialiteGeometryDimension );

    /** Retrieve and set View Capabilities
     * - used to set entries in SpatialiteDbLayer
     * \note
     * Starting with Spatialite 4.5.0, this information is contained within the results of gaiaGetVectorLayersList
     * \returns isReadOnly true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetViewTriggers();

    /** Retrieve extended Layer-Information
     * - after all information has been retrieved from a layer returned by gaiaGetVectorLayersList
     * \note
     * - Table:  has ROWID, unique, notNull properties, default-Values
     * - View: primary key, Underlining TABLE from views_geometry_columns
     * - View (writable): retrieve (possible) INSERT and UPDATE TRIGGERs
     * --> retrieve defaults and notNull properties from underlining TABLE
     * --> analyse TRIGGERs for alternative default-values
     * - common: setting EnabledCapabilities
     * \returns true or false
     * \see GetDbLayersInfo
     * \see setSpatialiteAttributeFields
     * \see GetSpatialiteQgsField
     * \see getCapabilities
     * \since QGIS 3.0
     */
    bool GetLayerSettings();

    /** Collection of warnings for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> mWarnings;

    /** Collection of reasons for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> mErrors;

    /** Map of style_name and Metadata
     * \note
     * - Key: StyleName as retrieved from SE_vector_styles or SE_raster_styles
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleVector,StyleRaster
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<QString, QString> mStyleLayersInfo;

    /** Does this Layer contain a Style
     * \note
     *  -  which can be retrieved with getLayerStyle
     * \see mLayerStyleSelected
     * \see getLayerStyle
     * \see setLayerStyleSelected
     * \see hasLayerStyle
     * \since QGIS 3.0
     */
    bool mHasStyle;

    friend class SpatialiteDbInfo;
};

/**
  * Class to CREATE and read specific Spatialite Gcp-Databases
  *  - mainly intended for use with the Georeferencer
  *  -> but designed to be used elsewere
  * \note 'PROJECTNAME.gcp.db'
  *  Will be a Database that will contain TABLEs for each raster in a directory
  * \since QGIS 3.0
  */
class QgsSpatiaLiteUtils
{
  public:
    enum GEOS_3D
    {
      GEOS_3D_POINT              = -2147483647,
      GEOS_3D_LINESTRING         = -2147483646,
      GEOS_3D_POLYGON            = -2147483645,
      GEOS_3D_MULTIPOINT         = -2147483644,
      GEOS_3D_MULTILINESTRING    = -2147483643,
      GEOS_3D_MULTIPOLYGON       = -2147483642,
      GEOS_3D_GEOMETRYCOLLECTION = -2147483641,
    };
    // static functions
    static void convertToGeosWKB( const unsigned char *blob, int blob_size,
                                  unsigned char **wkb, int *geom_size );
    static int computeMultiWKB3Dsize( const unsigned char *p_in, int little_endian,
                                      int endian_arch );
    static int computeSizeFromMultiWKB2D( const unsigned char *p_in, int nDims,
                                          int little_endian,
                                          int endian_arch );
    static int computeSizeFromMultiWKB3D( const unsigned char *p_in, int nDims,
                                          int little_endian,
                                          int endian_arch );
    static void convertFromGeosWKB2D( const unsigned char *blob, int blob_size,
                                      unsigned char *wkb, int geom_size,
                                      int nDims, int little_endian, int endian_arch );
    static void convertFromGeosWKB3D( const unsigned char *blob, int blob_size,
                                      unsigned char *wkb, int geom_size,
                                      int nDims, int little_endian, int endian_arch );
    static void convertFromGeosWKB( const unsigned char *blob, int blob_size,
                                    unsigned char **wkb, int *geom_size,
                                    int dims );
    static int computeSizeFromGeosWKB3D( const unsigned char *blob, int size,
                                         int type, int nDims, int little_endian,
                                         int endian_arch );
    static int computeSizeFromGeosWKB2D( const unsigned char *blob, int size,
                                         int type, int nDims, int little_endian,
                                         int endian_arch );

    /** Retrieve Capabilities of spatialite connection
     * - at this point we are 'sniffing' the Capabilities of the opened Database
     * \note
     *  - when only a specific table or table with geometry are being looked for
     *  -> format: 'table_name(geometry_name)' or only 'table_name', with all of its geometries
     *  - otherwise all tables with geometries
     * \param sDatabaseFileName Database Filename
     * \param sLayerName when used
     * \param bLoadLayers Load all Layer-Information or only 'sniff' [default] the Database
     * \param sqlite_handle opened using QgsSLConnect class
     * \returns SpatialiteDbInfo with collected results
     * \see QgsSLConnect
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo *GetSpatialiteDbInfoWrapper( QString sDatabaseFileName, QString sLayerName = QString::null, bool bLoadLayers = false, sqlite3 *sqlite_handle  = nullptr );
  private:

};

#endif
