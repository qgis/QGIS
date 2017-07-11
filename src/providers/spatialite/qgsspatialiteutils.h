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
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include "qgsvectordataprovider.h"

#include <sqlite3.h>
#include <spatialite.h>

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
      RasterLite2 = 5,
      SpatialiteTopopogy = 6,
      TopopogyLayer = 7,
      VectorStyle = 8,
      RasterStyle = 9,
      GeoPackageVector = 10, // Ogr
      GeoPackageRaster = 11, // Gdal
      MBTilesTable = 12, // Gdal
      MBTilesView = 13 // Gdal
    };
    SpatialiteDbInfo( QString sDatabaseFilename, sqlite3 *sqlite_handle )
      : mDatabaseFileName( sDatabaseFilename )
      , mSqliteHandle( sqlite_handle )
      , mQSqliteHandle( nullptr )
      , mSpatialMetadata( SpatialUnknown )
      , mSpatialiteVersionInfo( QString::null )
      , mSpatialiteVersionMajor( -1 )
      , mSpatialiteVersionMinor( -1 )
      , mSpatialiteVersionRevision( -1 )
      , mHasVectorLayers( -1 )
      , mHasSpatialTables( 0 )
      , mHasSpatialViews( 0 )
      , mHasVirtualShapes( 0 )
      , mHasRasterLite1Tables( -1 )
      , mHasGdalRasterLite1Driver( false )
      , mHasRasterLite2Tables( -1 )
      , mHasTopologyTables( -1 )
      , mHasMBTilesTables( -1 )
      , mHasGeoPackageTables( -1 )
      , mHasGcp( false )
      , mHasTopology( false )
      , mIsVersion45( false )
      , mLayersCount( 0 )
      , mReadOnly( false )
      , mLoadLayers( true )
      , mIsValid( false )
    {}
    ~SpatialiteDbInfo();
    //! The Database filename being read
    QString getDatabaseFileName() const { return mDatabaseFileName; }
    //! The sqlite handler
    sqlite3 *dbSqliteHandle() const { return mSqliteHandle; }
    //! The class allowing to reuse the same sqlite handle for more layers
    QgsSqliteHandle *getQSqliteHandle() const { return mQSqliteHandle; }

    /** Connection info (DB-path) without table and geometry
     * - this will be called from the SpatialiteDbLayer::layerConnectionInfo()
     * \note
     *  - to call for Database and Table/Geometry portion use: SpatialiteDbLayer::dbConnectionInfo()
    * \returns uri with Database only
     * \see SpatialiteDbLayer::layerConnectionInfo()
    * \since QGIS 3.0
    */
    QString dbConnectionInfo() const
    {
      return QString( "dbname='%1'" ).arg( QString( mDatabaseFileName ).replace( '\'', QLatin1String( "\\'" ) ) );
    }
    QString getSummary() const
    {
      QString sSummary = QString( "%1 SpatialTables" ).arg( dbSpatialTablesCount() );
      if ( dbSpatialViewsCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 SpatialViews" ).arg( sSummary ).arg( dbSpatialViewsCount() );
      }
      if ( dbVirtualShapesCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 VirtualShapes" ).arg( sSummary ).arg( dbVirtualShapesCount() );
      }
      if ( dbRasterLite2TablesCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 RL2 Coverages" ).arg( sSummary ).arg( dbRasterLite2TablesCount() );
      }
      if ( dbRasterLite1TablesCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 RL1 Coverages" ).arg( sSummary ).arg( dbRasterLite1TablesCount() );
      }
      if ( dbTopologyTablesCount() > 0 )
      {
        sSummary = QString( "%1 ; %2 Topologies" ).arg( sSummary ).arg( dbTopologyTablesCount() );
      }

      return sSummary;
    }
    //! The Spatialite internal Database structure being read
    SpatialiteDbInfo::SpatialMetadata dbSpatialMetadata() const { return mSpatialMetadata; }
    QString dbSpatialMetadataString() const { return mSpatialMetadataString; }
    //! The Spatialite Version as returned by spatialite_version()
    QString dbSpatialiteVersionInfo() const { return mSpatialiteVersionInfo; }
    //! The major Spatialite Version being used
    int dbSpatialiteVersionMajor() const { return mSpatialiteVersionMajor; }
    //! The minor Spatialite Version being used
    int dbSpatialiteVersionMinor() const { return mSpatialiteVersionMinor; }
    //! The revision Spatialite Version being used
    int dbSpatialiteVersionRevision() const { return mSpatialiteVersionRevision; }

    /** Amount of SpatialTables  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialTables that have been loaded
     * \since QGIS 3.0
     */
    int dbSpatialTablesCount() const { return mHasSpatialTables; }

    /** Amount of SpatialViews  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialViews that have been loaded
     * \since QGIS 3.0
     */
    int dbSpatialViewsCount() const { return mHasSpatialViews; }

    /** Amount of VirtualShapes found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of VirtualShapes that have been loaded
     * \since QGIS 3.0
     */
    int dbVirtualShapesCount() const { return mHasVirtualShapes; }

    /** Amount of RasterLite1-Rasters found in the Database
     * - only the count of valid Layers are returned
     * \note
     * - the Gdal-RasterLite1-Driver is needed to determine this
     * - this does not reflect the amount of RasterLite1-Rasters that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterLite1TablesCount() const { return mHasRasterLite1Tables; }

    /** Is the Gdal-RasterLite1-Driver available ?
     * \note
     * - RasterLite1-Rasters can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalRasterLite1Driver() const { return mHasGdalRasterLite1Driver; }

    /** Amount of RasterLite2-Coverages found in the Database
     * - from the raster_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2-Coverages that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterLite2TablesCount() const { return mHasRasterLite2Tables; }

    /** Amount of Topologies found in the Database
     * - from the topologies table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of Topologies that have been loaded
     * \since QGIS 3.0
     */
    int dbTopologyTablesCount() const { return mHasTopologyTables; }
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
     * - SpatialTables, SpatialViews and virtualShapes [from the vector_layers View]
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
     * - only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles, RasterLite1
     * \since QGIS 3.0
     */
    bool isDbValid() const { return mIsValid; }
    //! Map of tables and views that are contained in the VectorLayers
    QMap<QString, QString> getDbVectorLayers() const { return mVectorLayers; }
    //! Map of tables and views that are contained in the RasterLite2Layers
    QMap<QString, QString> getDbRasterLite2Layers() const { return mRasterLite2Layers; }
    //! Map of tables and views that are contained in the RasterLite2Layers
    QMap<QString, QString> getDbTopologyLayers() const { return mTopologyLayers; }
    //! Map of tables and views that are contained in the RasterLite2Layers
    QMap<QString, QString> getDbRasterLite1Layers() const { return mRasterLite1Layers; }
    //! List of Topologies contained in the 'topologies' TABLE
    QStringList getDbTopologyNames() const { return mTopologyNames; }

    /** UpdateLayerStatistics for the Database or Layer
     * - this will be called from the SpatialiteDbLayer::UpdateLayerStatistics
     * \note
     *  - when only a specific table or table with geometry is to be done
     *  -> format: 'table_name(geometry_name)' or only 'table_name', with all of its geometries
     *  - otherwise all tables with geometries
     * \param sLayerName when used
     * \param sGeometryName when used
     * \returns result of the internal Spatalite function update_layer_statistics
     * \see SpatialiteDbLayer::UpdateLayerStatistics
     * \since QGIS 3.0
     */
    bool UpdateLayerStatistics( const QString sLayerName = QString::null, const QString sGeometryName = QString::null );

    /** Map of valid Layers supported by this Database [mLayerName as Key]
     * - contains Layer-Name and a SpatialiteDbLayer-Pointer
     * \note
     * - contains all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * -- when  getDbLoadLayers is true, getSpatialiteDbLayer will load all layers
     * \see getDbLoadLayers
     * \since QGIS 3.0
     */
    QMap<QString, SpatialiteDbLayer *> getDbLayers() const { return mDbLayers; }

    /** List of DataSourceUri of valid Layers
     * -  contains Layer-Name and DataSourceUri
     * \note
     * - Lists all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * \see mDbLayersDataSourceUris
     * \see mDbLayers
     * \see getSpatialiteDbLayer
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

    /** Retrieve SpatialiteDbLayer-Pointer for a given
     * -  LayerId
     * \note
     * - LayerId is only valid for the active session and MUST not be used for project storing
     * \param sLayerName Layer-Name or Uri of Layer
     * \param loadLayer when true [default], Load Layer-Information if not already loaded
     * \see getSpatialiteDbLayer
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *getSpatialiteDbLayer( int iLayerId );

    /** Returns the String value of the enum of Layer-Types
     * \see SpatialiteDbLayer::setLayerType
     * \since QGIS 3.0
     */
    static QString SpatialiteLayerTypeName( SpatialiteDbInfo::SpatialiteLayerType layerType );

    /** Returns the String value of the enum of SpatialIndex-Types
     * \see SpatialiteDbLayer::setSpatialIndexType
     * \since QGIS 3.0
     */
    static QString SpatiaIndexTypeName( SpatialiteDbInfo::SpatialIndexType spatialIndexType );

    /** Returns the String value of the enum of SpatialMetadata-Types
     * \see SpatialiteDbInfo::setSpatialMetadata
     * \since QGIS 3.0
     */
    static QString SpatialMetadataTypeName( SpatialiteDbInfo::SpatialMetadata spatialMetadataType );

    /** Returns the enum  value from the given String of a Layer-Types
     * \note
     * - at the moment not used but intended for external use
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialiteLayerType SpatialiteLayerTypeFromName( const QString &typeName );

    /** Returns the enum  value from the given String of a SpatialIndex-Types
     * \note
     * - at the moment not used but intended for external use
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialIndexType SpatiaIndexTypeFromName( const QString &typeName );

    /** Returns the enum  value from the given String of a SpatialMetadata-Types
     * \note
     * - at the moment not used but intended for external use
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialMetadata SpatialMetadataTypeFromName( const QString &typeName );

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
    bool GetSpatialiteDbInfo( QString sLayerName = QString::null, bool bLoadLayers = false );

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
    QStringList getVectorLayersMissing() const { return mVectorLayersMissing; }
    //! Collection of warnings for the Layer being invalid
    QStringList getWarnings() const { return mWarnings; }
    //! Collection of reasons for the Database being invalid
    QStringList getErrors() const { return mErrors; }
  protected:
  private:
    //! The Database filename being read
    QString mDatabaseFileName;
    //! The sqlite handler
    sqlite3 *mSqliteHandle = nullptr;
    //! The class allowing to reuse the same sqlite handle for more layers
    QgsSqliteHandle *mQSqliteHandle = nullptr;
    //! The Spatialite internal Database structure being read
    SpatialiteDbInfo::SpatialMetadata mSpatialMetadata;
    //! The Spatialite internal Database structure being read (as String)
    QString mSpatialMetadataString;
    //! Set the Spatialite internal Database structure being read (and the String)
    void setSpatialMetadata( int iSpatialMetadata );
    //! The Spatialite Version as returned by spatialite_version()
    QString mSpatialiteVersionInfo;
    //! The major Spatialite Version being used
    int mSpatialiteVersionMajor;
    //! The minor Spatialite Version being used
    int mSpatialiteVersionMinor;
    //! The revision Spatialite Version being used
    int mSpatialiteVersionRevision;
    //! Does the read Database contain SpatialTables [ 0=none, otherwise amount]
    int mHasVectorLayers;
    //! Does the read Database contain SpatialTables [ 0=none, otherwise amount]
    int mHasSpatialTables;
    //! Does the read Database contain SpatialViews views [ 0=none, otherwise amount]
    int mHasSpatialViews;
    //! Does the read Database contain VirtualShapes tables [0=none, otherwise amount]
    int mHasVirtualShapes;
    //! Does the read Database contain RasterLite1 coverages [-1=no rasterlit1 logic found, otherwise amount (0 being empty)]
    int mHasRasterLite1Tables;
    //! Is the Gdal-RasterLite1-Driver available ?
    bool mHasGdalRasterLite1Driver;
    //! Does the read Database contain RasterLite2 coverages [-1=no raster_coverages table, otherwise amount (0 being empty)]
    int mHasRasterLite2Tables;
    //! Does the read Database contain Topology tables [-1=no topologies table, otherwise amount (0 being empty)]
    int mHasTopologyTables;
    //! Does the read Database contain MbTiles-Tables [ 0=none, otherwise 3=View-based MbTiles, else 2]
    int mHasMBTilesTables;
    //! Does the read Database contain GPKG-Tables [ 0=none,  if 'gpkg_contents'  exists GeoPackage Revision 10]
    int mHasGeoPackageTables;
    //! Is the used Spatialite compiled with Spatialite-Gcp support
    bool mHasGcp;
    //! Is the used Spatialite compiled with Topology (and thus RtTopo) support
    bool mHasTopology;
    //! Is the used Spatialite 4.5.0 or greater
    bool mIsVersion45;
    //! Layers-Counter [not all of which may be valid and contained in mDbLayers]
    int mLayersCount;
    //! Flag indicating if the layer data source has ReadOnly restrictions
    bool mReadOnly;
    //! Load all Layer-Information [default] or only 'sniff' the Database
    bool mLoadLayers;
    //! Is the read Database supported by QgsSpatiaLiteProvider
    bool mIsValid;
    //! Set the Database as invalid, with possible Message, returns amount of Errors collected
    int setDatabaseInvalid( QString errCause = QString::null );

    /** Map of valid Layers supported by this Database [mLayerName as Key]
     * - contains Layer-Name and a SpatialiteDbLayer-Pointer
     * \note
     * - contains all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * \see getSpatialiteDbLayer
     * \since QGIS 3.0
     */
    QMap<QString, SpatialiteDbLayer *> mDbLayers;
    //!

    /** Map of tables and views that are not part of the Layers
     * - i.e. not Spatial-Tables or Spatial-Administration Tables
     * - containsTable-Name and Table-Type listed in mSpatialiteTypes
     * \see mSpatialiteTypes
     * \since QGIS 3.0
     */
    QMap<QString, QString> mTables;
    //! Map of tables and views that are contained in the VectorLayers [LayerName,LayerType]
    QMap<QString, QString> mVectorLayers;
    //! Map of tables and views that are contained in the RasterLite2Layers [LayerName,LayerType]
    QMap<QString, QString> mRasterLite2Layers;
    //! Map of tables and views that are contained in the RasterLite2Layers [LayerName,LayerType]
    QMap<QString, QString> mTopologyLayers;
    //! Map of tables and views that are contained in the RasterLite2Layers [LayerName,LayerType]
    QMap<QString, QString> mRasterLite1Layers;
    //! List of Topologies contained in the 'topologies' TABLE
    QStringList mTopologyNames;
    //! Map of tables and views that are contained in the MBTiles [LayerName,LayerType]
    QMap<QString, QString> mMBTilesLayers;
    //! Map of tables and views that are contained in the GeoPackage [LayerName,LayerType]
    QMap<QString, QString> mGeoPackageLayers;

    /** Map of Connection-String of mDbLayers
     * - contains Layer-Name and DataSourceUri
     * \note
     * - contains all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * \since QGIS 3.0
     */
    QMap<QString, QString> mDbLayersDataSourceUris;

    /** Retrieve Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * - gathers all information from gaiaGetVectorLayersList
     * -> complementary information will be retrieved from SpatialiteGetLayerSettingsWrapper
     * -> RasterLite2 information will be retrieved from SpatialiteGetRL2LayersInfoWrapper
     * -> Topology information will be retrieved from SpatialiteGetRL2LayersInfoWrapper
     * -> RasterLite1 information will be retrieved from SpatialiteGetRL2LayersInfoWrapper
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param  sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbConnectionInfo
     * \see SpatialiteGetLayerSettingsWrapper
     * \see SpatialiteGetRL2LayersInfoWrapper
     * \see SpatialiteGetTopologyLayersInfoWrapper
     * \see SpatialiteGetRL1LayersInfoWrapper
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

    /** Retrieve RasterLite2 Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * - results are stored in mVectorLayers
     * - there is no direct RasterLite2 support needed during this function
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param  sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool readRL2Layers();

    /** Determine if valid RasterLite1 Layers exist
     * - called only when mHasRasterLite1Tables > 0 during GetSpatialiteDbInfo
     * \note
     * - results are stored in mRasterLite1Layers
     * - checking is done if the Gdal-Driver for RasterLite1 can be used
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool readRL1Layers();

    /** Determine if valid Topology Layers exist
     * - called only when mHasTopologyTables > 0 during GetSpatialiteDbInfo
     * \note
     * - results are stored in mTopologyLayers
     * \returns true if the the count of valid-layers > 0
     * \see GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool readTopologyLayers();

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

    /** Retrieve RasterLite2 Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * - there is no direct RasterLite2 support needed during this function
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param  sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetRL2LayersInfo( const QString sLayerName = QString::null );

    /** Retrieve Topology Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param  sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetTopologyLayersInfo( QString sLayerName = QString::null );

    /** Retrieve RasterLite1 Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param  sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetRL1LayersInfo( QString sLayerName = QString::null );

    /** Retrieve MBTiles Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param  sLayerName Name of the Layer to search for format: 'name'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetMBTilesLayersInfo( QString sLayerName = QString::null );

    /** Retrieve GeoPackage Layers-Information of spatialite connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param  sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetGeoPackageLayersInfo( QString sLayerName = QString::null );

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
     * \param  sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
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
    //! List of tables or views that could not be resolved
    QStringList mVectorLayersMissing;
    //! Collection of warnings for the Database being invalid
    QStringList mWarnings;
    //! Collection of reasons for the Database being invalid
    QStringList mErrors;
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
      , mGeomType( QgsWkbTypes::Unknown )
      , mGeomTypeString( QString::null )
      , mCoordDimensions( GAIA_XY )
      , mLayerExtent( QgsRectangle( 0.0, 0.0, 0.0, 0.0 ) )
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
    {}
    ~SpatialiteDbLayer();
    //! The Database Info-Structure being read
    SpatialiteDbInfo *getDbConnectionInfo() const { return mDbConnectionInfo; }
    //! The sqlite handler
    sqlite3 *getSqliteHandle() const { return getDbConnectionInfo()->dbSqliteHandle(); }
    //! The Database filename being read
    QString getDatabaseFileName() const { return mDatabaseFileName; }

    /** Connection info (DB-path) with table and geometry
     *  -> this should be the only function that deals with connection-String formatting
     * \note
     *  - to call for Database portion only, use: SpatialiteDbInfo::dbConnectionInfo()
     *  - For RasterLite1: GDAL-Syntax will be used
    * \returns uri with Database and Table/Geometry Information
     * \see SpatialiteDbInfo::dbConnectionInfo()
    * \since QGIS 3.0
    */
    QString layerConnectionInfo() const
    {
      if ( mLayerType == SpatialiteDbInfo::RasterLite1 )
      {
        return QString( "RASTERLITE:%1,table=%2" ).arg( mDatabaseFileName ).arg( "RASTERLITE" ).arg( mTableName );
      }
      if ( !mGeometryColumn.isEmpty() )
      {
        return QString( "%1 table=\"%2\" (%3)" ).arg( getDbConnectionInfo()->dbConnectionInfo() ).arg( mTableName ).arg( mGeometryColumn );
      }
      return QString( "%1 table=\"%2\"" ).arg( getDbConnectionInfo()->dbConnectionInfo() ).arg( mTableName );
    }

    /** Ogr/Gdal Connection info (DB-path) with table and geometry
     * \note
     *  - For RasterLite1: GDAL-Syntax will be used
    * \returns uri with Database and Table/Geometry Information
    * \since QGIS 3.0
    */
    QString dbConnectionInfoOgr() const
    {
      if ( mLayerType == SpatialiteDbInfo::RasterLite1 )
      {
        return QString( "RASTERLITE:%1,table=%2" ).arg( mDatabaseFileName ).arg( "RASTERLITE" ).arg( mTableName );
      }
      if ( !mGeometryColumn.isEmpty() )
      {
        return QString( "%1|%2:%3:%4:%5" ).arg( mDatabaseFileName ).arg( mLayerName ).arg( getGeomTypeString() ).arg( mNumberFeatures ).arg( getLayerTypeString() );
      }
      return QString( "%1|%2" ).arg( mDatabaseFileName ).arg( mLayerName );
    }
    //! Name of the table with no schema
    QString getTableName() const { return mTableName; }
    //! Name of the geometry column in the table
    QString getGeometryColumn() const { return mGeometryColumn; }
    //! Name of the Layer format: 'table_name(geometry_name)'
    QString getLayerName() const { return mLayerName; }
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
    //! The Spatialite Geometry-Type being read
    QgsWkbTypes::Type getGeomType() const { return mGeomType; }
    //! The Spatialite Geometry-Type being read (as String)
    QString getGeomTypeString() const { return mGeomTypeString; }
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

    /** Rectangle that contains the extent (bounding box) of the layer, with Srid
     * \note
     * \since QGIS 3.0
     */
    QString getLayerExtentEWKT() const { return mLayerExtentEWKT; }

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
     * - SpatialiteTopopogy: will serve only TopopogyLayer, which ate SpatialTables
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
    //! Collection of warnings for the Layer being invalid
    QStringList getWarnings() const { return mWarnings; }
    //! Collection of reasons for the Layer being invalid
    QStringList getErrors() const { return mErrors; }
    //! Is the Layer valid
    bool isLayerValid() const { return mIsValid; }

    /** Resolve unset settings
     * Goal: to (semi) Automate unresolved settings when needed
     * \see prepareCapabilities
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
    //! Name of the geometry column in the table
    QString mGeometryColumn;
    //! Name of the Layer format: 'table_name(geometry_name)'
    QString mLayerName;
    //! Name of the table which contains the SpatialView-Geometry (underlining table)
    QString mViewTableName;
    //! Name of the table-geometry which contains the SpatialView-Geometry (underlining table)
    QString mViewTableGeometryColumn;
    //! Name of the Layer format: 'table_name(geometry_name)'
    QString mLayerViewTableName;
    //! Title [RasterLite2]
    QString mTitle;
    //! Abstract [RasterLite2]
    QString mAbstract;
    //! Copyright [RasterLite2]
    QString mCopyright;
    //! The Srid of the Geometry
    int mSrid;
    //! AuthId [auth_name||':'||auth_srid]
    QString mAuthId;
    //! Proj4text [from mSrid]
    QString mProj4text;
    //! The SpatialIndex-Type used for the Geometry
    SpatialiteDbInfo::SpatialIndexType mSpatialIndexType;
    //! The SpatialiIndex-Type used for the Geometry (as String)
    QString mSpatialIndexTypeString;
    //! Set the SpatialIndex-Type used for the Geometry (and the String)
    void setSpatialIndexType( int iSpatialIndexType );
    //! The Spatialite Layer-Type being read
    SpatialiteDbInfo::SpatialiteLayerType mLayerType;
    //! The Spatialite Layer-Type being read (as String)
    QString mLayerTypeString;
    //! Set the Spatialite Layer-Type being read  (and the String)
    void setLayerType( SpatialiteDbInfo::SpatialiteLayerType layerType );
    //! The Spatialite Geometry-Type being read
    QgsWkbTypes::Type mGeomType;
    //! The Spatialite Geometry-Type being read (as String)
    QString  mGeomTypeString;
    //! Set the Spatialite Geometry-Type being read  (and the String)
    void setGeomType( int iGeomType );
    //! The Spatialite Coord-Dimensions
    int mCoordDimensions;
    //! Rectangle that contains the extent (bounding box) of the layer
    QgsRectangle mLayerExtent;
    //! The extent (bounding box) of the layer AS Extended Well Known Text
    QString mLayerExtentEWKT;
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
    int setLayerInvalid( QString errCause = QString::null );
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
     * \see GetRL1LayersInfo
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
    //! Collection of warnings for the Layer being invalid
    QStringList mWarnings;
    //! Collection of reasons for the Layer being invalid
    QStringList mErrors;

    friend class SpatialiteDbInfo;
};

/**
  * Class to CREATE and read specific Spatialite Gcp-Databases
  *  - mainly intended for use with the Georeferencer
  *  -> but designed to be used elsewere
  * @note 'PROJECTNAME.gcp.db'
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
