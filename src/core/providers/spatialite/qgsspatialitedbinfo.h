/***************************************************************************
     qgsspatialitedbinfo.h
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

#ifndef QGSSPATIALITEDBINFO_H
#define QGSSPATIALITEDBINFO_H

#include <limits>
#include <QDir>
#include <QDomNode>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QTextStream>
#include <QDebug>

#include "qgswkbtypes.h"
#include "qgsfields.h"
#include "qgslayermetadata.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"
#include "qgsbox3d.h"
#include "qgsvectordataprovider.h"
#include "qgsfeaturesink.h"
extern "C"
{
#include <sqlite3.h>
#include <spatialite.h>
}

class SpatialiteDbLayer;
class QgsSqliteHandle;

/**
 * Class to contain all information needed for a Spatialite/Rasterlite2 connection
  * - it will 'sniff' all Tables, retaining minimal information for each Table/Layer found
  * - upond demand, a list of SpatialiteDbLayer classes will be collected
  *  -> thus only called once for each connection
  * \note
  *  - The result of this class will be contained in the QgsSqliteHandle class
  *  -> so that the Information must only be called once for each connection
  *  --> When shared, the QgsSqliteHandle class will used the same connection of each Layer in the same Database
  * QgsDebugMsgLevel = 3: for connection messages that deal with QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
  * QgsDebugMsgLevel = 4: for connection messages that deal with connections not supporting QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
  * QgsDebugMsgLevel = 5: experimental code under development
  * \see QgsSqliteHandle
  * \see SpatialiteDbLayer
  * \since QGIS 3.0
 */
class CORE_EXPORT SpatialiteDbInfo : public QObject
{
    Q_OBJECT
  public:
    typedef QPair<QVariant::Type, QVariant::Type> TypeSubType;

    /**
     * 'json'
     * - New versions of OGR convert list types to JSON when it stores a Spatialite table.
     */
    static const QString SPATIALITE_ARRAY_PREFIX;

    /**
     * 'list'
     * - New versions of OGR convert list types to JSON when it stores a Spatialite table.
     */
    static const QString SPATIALITE_ARRAY_SUFFIX;

    /**
     * List of Table-Types in a Spatialite Database
     * - For all versions of Spatialite
     * \since QGIS 3.0
     */
    static QStringList mSpatialiteTypes;

    /**
      * SniffTypes
      *  - based on the Task, restrict unneeded activities
      *  -> For Browser: only Sqlite3-Container-Type is needed
      *  -> For Provider-Lists: only Tables/Layers without details
      *  -> For Provider: only Tables/Layers with all details
      * \note
      * Spatialite Connection not needed for these Types:
      *  - SniffDatabaseType: Determinens Sqlite3-Container-Type [SpatialMetadata]
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      * Spatialite Connection needed and will be started for these Types:
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer] and used Spatialite version
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
      SpatialIndexNone = 0,
      //! Vector Layer: Spatial Index RTree - GAIA_SPATIAL_INDEX_RTREE  1
      SpatialIndexRTree = 1,
      //! Vector Layer: Spatial Index MbrCache - GAIA_SPATIAL_INDEX_MBRCACHE  2
      SpatialIndexMbrCache = 2
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
    enum MimeType
    {
      MimeUnknown = -1,
      MimeNotExists = 0,
      MimeFile = 10,
      MimeFileNotReadable = 11,
      MimeFileExecutable = 12,
      MimeDirectory = 20,
      MimeDirectorySymLink = 21,
      MimeDirectoryNotReadable = 22,
      MimeDirectoryExecutable = 23,
      MimeSqlite3 = 1000,
      MimePdf = 1001,
      MimeGif87a = 2000,
      MimeGif89a = 2001,
      MimeTiff = 2010,
      MimeJpeg = 2020,
      MimeJp2 = 2030,
      MimePng = 2040,
      MimeSvg = 2050,
      MimeIco = 2900,
      MimeAvi = 3040,
      MimeWav = 3041,
      MimeWebp = 3042,
      MimeKmz = 5000,
      MimeKml = 5001,
      MimeScripts = 6000,
      MimeBash = 6001,
      MimePython = 6002,
      MimeExeUnix = 6003,
      MimeHtmlDTD = 7000,
      MimeQGisProject = 7001,
      MimeTxt = 8000,
      MimeRtf = 8001,
      MimePid = 8002,
      MimeZip = 9000,
      MimeBz2 = 9001,
      MimeTar = 9002,
      MimeRar = 9003,
      MimeXar = 9004,
      Mime7z = 9005,
      MimeXml = 9100,
      MimeGdalPam = 9101,
      MimeGdalGml = 9102,
      MimeSqlite2 = 9999
    };
    SpatialiteDbInfo( QString sDatabaseFilename, sqlite3 *sqlite_handle  = nullptr, SpatialMetadata dbCreateOption = SpatialiteDbInfo::SpatialUnknown );
    ~SpatialiteDbInfo();

    /**
     * Create a SpatialiteDbInfo based Connection
     *  -> containing all needed Information about a Spatial Sqlite3 Container
     * \note
     *  - check result with spatialiteDbInfo->isDbSqlite3()
     *  -> if File exists and is a Sqlite3 Container.
     *  - check result with spatialiteDbInfo->isDbGdalOgr()
     *  -> if File only supported by QgsOgrProvider or QgsGdalProvider
     *  -> otherwise supported by QgsSpatiaLiteProvider
     * When not a Sqlite3-Container: check result of getFileMimeType()
    * \param sDatabaseFileName file to open
    * \param bShared if this connection should be shared
    * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
    * \param bLoadLayers load Layer details
    * \param dbCreateOption  the Container-Type to create [default=SpatialiteUnknown - i.e. File exists]
    * \param sniffType based on the Task, restrict unneeded activities
    * \returns not nullptr if file exists
    * \since QGIS 3.0
    */
    static SpatialiteDbInfo *FetchSpatialiteDbInfo( const QString sDatabaseFileName, bool bShared = false,
        QString sLayerName = QString::null, bool bLoadLayers = false, SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::SpatialUnknown, SpatialSniff sniffType = SpatialiteDbInfo::SniffUnknown );

    /**
     * Parse the LayerName String into TableName and GeomeryColumn portions
     * - Input as 'table_name' or 'table_name(geometry_name)'
     * \note
     *  - A Vector Layer is expected to have a GeomeryColumn name
     *  - A Raster Layer is not expected to have a GeomeryColumn name
     * \param sLayerName IN: 'table_name' or 'table_name(geometry_name)'
     * \param sTableName OUT: the returned result from field 0 ofsLayerName
     * \param sGeometryColumn OUT: the returned result from field 1 of sLayerName orf empty
     * \returns true if sGeometryColumn is not empty
     * \see readVectorLayers
     * \see createDbLayerInfoUri
     * \see QgsSpatialiteLayerItem
     * \since QGIS 3.0
     */
    static bool parseLayerName( QString sLayerName, QString &sTableName, QString &sGeometryColumn )
    {
      bool bGeometryColumn = false;
      sTableName = sLayerName;
      sGeometryColumn = QString();
      if ( ( sTableName.contains( QStringLiteral( "(" ) ) ) && ( sTableName.endsWith( QStringLiteral( ")" ) ) ) )
      {
        QStringList sa_layername = sTableName.split( QStringLiteral( "(" ) );
        if ( sa_layername.size() == 2 )
        {
          // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
          sGeometryColumn = sa_layername[1].replace( QStringLiteral( ")" ), QStringLiteral( "" ) );
          sTableName = sa_layername.at( 0 );
          bGeometryColumn = true;
        }
      }
      return bGeometryColumn;
    };

    /**
      * MimeType as String
      * \since QGIS 3.0
      */
    static QString FileMimeTypeString( SpatialiteDbInfo::MimeType mimeType );

    /**
      * SniffType as String
      * \since QGIS 3.0
      */
    static QString SniffTypeString( SpatialiteDbInfo::SpatialSniff sniffType );

    /**
     * The Database filename (with Path)
    * \returns mDatabaseFileName complete Path (without without symbolic links)
    * \since QGIS 3.0
    */
    QString getDatabaseFileName() const { return mDatabaseFileName; }

    /**
     * The Database filename without path
    * \returns mFileName  name of the file, excluding the path
    * \since QGIS 3.0
    */
    QString getFileName() const { return mFileName; }

    /**
     * The Database Directory without file
    * \returns mDirectoryName  name of the complete  path,  (without without symbolic links), excluding the file-name
    * \since QGIS 3.0
    */
    QString getDirectoryName() const { return mDirectoryName; }

    /**
      * MimeType of file
      * \note
     *  Set in constructor
      * \see readMagicHeaderFromFile
      * \since QGIS 3.0
      */
    MimeType getFileMimeType() const { return mMimeType; }

    /**
      * SniffType of file as String
      * \note
     *  Set in constructor
      *  \see readMagicHeaderFromFile
      * \since QGIS 3.0
      */
    QString getFileMimeTypeString() const { return SpatialiteDbInfo::FileMimeTypeString( mMimeType ); }

    /**
     * The sqlite handler
     * - contained in the QgsSqliteHandle class being used
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    sqlite3 *dbSqliteHandle() const { return mSqliteHandle; }

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
     * Count on how often this Connection is being used when shared
     * \note
     *  -1 not being shared
    * \returns Count on how often this Connection is being used
     * \see SpatialiteDbInfo::getQSqliteHandle()
     * \since QGIS 3.0
     */
    int getConnectionRef() const;

    /**
     * Removes reference to this connection
     * \note
     *  -1 not being shared
    * \returns Count on how often this Connection is being used
     * \see SpatialiteDbInfo::getQSqliteHandle()
     * \since QGIS 3.0
     */
    int removeConnectionRef() const;

    /**
     * Check if this connection is needed
     * \note
     *  if false, removeConnectionRef will have been called
     *  - SpatialiteDbInfo can be deleted
    * \returns false if not needed and 'delete' can be called
     * \see SpatialiteDbInfo::getQSqliteHandle()
     * \since QGIS 3.0
     */
    bool checkConnectionNeeded();

    /**
     * Unique number for this Connection
     * - mainly used to check if a specfic connection was found
     * \note
     *  - created with QUuid::createUuid().toString()
     * \see attachQSqliteHandle
     * \see QgsSqliteHandle::setStatus
     * \since QGIS 3.0
     */
    QString getConnectionUuid() const;

    /**
     * Is the QgsSqliteHandle Connection being shared
     * \see QgsSqliteHandle::openDb
     * \since QGIS 3.0
     */
    bool isConnectionShared() const;

    /**
     * Set the QgsSqliteHandle Connection to be shared
     * \see QgsSqliteHandle::openDb
     * \since QGIS 3.0
     */
    bool setConnectionShared( bool bIsShared );

    /**
     * Count on how many Spatialite Connections are active
     * - A limit of 64 Spatialite-Connections exist
     * \note
     *  -1 when not compiled with SPATIALITE_HAS_INIT_EX
    * \see QgsSqliteHandle::getSharedSpatialiteConnectionsCount
     * \since QGIS 3.0
     */
    int getSharedSpatialiteConnectionsCount();

    /**
     * Connection info (DB-path) without table and geometry
     * - this will be called from classes using SpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: SpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
    * \see SpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    QString getDatabaseUri() const
    {
      return QString( "dbname='%1'" ).arg( QString( mDatabaseFileName ).replace( '\'', QLatin1String( "\\'" ) ) );
    }

    /**
     * Has 'mod_spatialite' or 'spatialite_init' been called for the QgsSpatialiteProvider and QgsRasterLite2Provider
     * \note
     *  - QgsSpatialiteProvider and QgsRasterLite2Provider
    * \see QgsSqliteHandle::isDbSpatialiteActive
     * \since QGIS 3.0
     */
    bool isDbSpatialiteActive() const;

    /**
     * Has 'mod_rasterlite2' or 'rl2_init' been called for the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
    * \see QgsSqliteHandle::isDbSpatialiteActive
     * \since QGIS 3.0
     */
    bool isDbRasterLite2Active() const;

    /**
     * Spatialite Provider Key
     * - for use with QgsVectorLayer and for building Uri
     * \note
     *  - Spatialite-Geometries
    * \see createDbLayerInfoUri
    * \see addDbMapLayers
    * \since QGIS 3.0
    */
    const QString mSpatialiteProviderKey = QStringLiteral( "spatialite" );

    /**
     * Ogr Provider Key
     * - for use with QgsVectorLayer and for building Uri
     * \note
     *  - non-Spatialite Geometries, GeoPackage
    * \see createDbLayerInfoUri
    * \see addDbMapLayers
    * \since QGIS 3.0
    */
    const QString mOgrProviderKey = QStringLiteral( "ogr" );

    /**
     * Rasterlite2 Provider Key
     * - for use with QgsRasterLayer and for building Uri
     * \note
     *  - Spatialite-RasterLite2
    * \see createDbLayerInfoUri
    * \see addDbMapLayers
    * \since QGIS 3.0
    */
    const QString mRasterLite2ProviderKey = QStringLiteral( "rasterlite2" );

    /**
     * Gdal Provider Key
     * - for use with QgsRasterLayer and for building Uri
     * \note
     *  - Rasters, GeoPackage-Rasters, RasterLite1 and MBTiles
    * \see createDbLayerInfoUri
    * \see addDbMapLayers
    * \since QGIS 3.0
    */
    const QString mGdalProviderKey = QStringLiteral( "gdal" );

    /**
     * The Summary of Layer-Types contained in the Database
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

    /**
     * The Spatialite internal Database structure being read
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    SpatialiteDbInfo::SpatialMetadata dbSpatialMetadata() const { return mSpatialMetadata; }

    /**
     * Contains collected Metadata for the Database
     * \brief A structured metadata store for a map layer.
     * \note
     *  - SpatialiteDbLayer will use a copy this as starting point
     * \see QgsMapLayer::htmlMetadata()
     * \see QgsMapLayer::metadata
     * \see QgsMapLayer::setMetadata
    * \since QGIS 3.0
    */
    QgsLayerMetadata getDbMetadata() const { return mDbMetadata; }

    /**
     * If Spatialite can be used
     *  -  must be called before any Spatialite/RasterLite2 functions are called
     * \note
     *  If QGis has not been compiled with RasterLite2 support, false will be returned for RasterLite2
     * \param loadSpatialite when true: load Spatialite-Driver if not active
     * \param loadRasterLite2 when true: load Spatialite and RasterLite2-Driver if not active
     * \returns true if Driver(s) are active
     * \see getSpatialiteVersion
     * \see dbHasRasterlite2
     * \see QgsSqliteHandle::initRasterlite2
     * \see QgsSpatiaLiteProvider::setSqliteHandle
     * \see QgsRasterLite2Provider::setSqliteHandle
     * \since QGIS 3.0
     */
    bool dbHasSpatialite( bool loadSpatialite = false, bool loadRasterLite2 = false );

    /**
     * The Spatialite internal Database structure being read (as String)
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString dbSpatialMetadataString() const { return mSpatialMetadataString; }

    /**
     * The Spatialite Version Driver being used
     * \note
     *  - returned from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString dbSpatialiteVersionInfo() const { return mSpatialiteVersionInfo; }

    /**
     * The major Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionMajor() const { return mSpatialiteVersionMajor; }

    /**
     * The minor Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionMinor() const { return mSpatialiteVersionMinor; }

    /**
     * The revision Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionRevision() const { return mSpatialiteVersionRevision; }

    /**
     * The RasterLite2 Version Driver being used
     * \note
     *  - returned from RL2_Version()
     * \see dbHasRasterlite2
    * \since QGIS 3.0
    */
    QString dbRasterLite2VersionInfo() const { return mRasterLite2VersionInfo; }

    /**
     * The major RasterLite2 Version being used
     * \note
     *  - extracted from RL2_Version()
     * \see dbHasRasterlite2
    * \since QGIS 3.0
    */
    int dbRasterLite2VersionMajor() const { return mRasterLite2VersionMajor; }

    /**
     * The minor RasterLite2 Version being used
     * \note
     *  - extracted from RL2_Version()
     * \see dbHasRasterlite2
    * \since QGIS 3.0
    */
    int dbRasterLite2VersionMinor() const { return mRasterLite2VersionMinor; }

    /**
     * The revision RasterLite2 Version being used
     * \note
     *  - extracted from RL2_Version()
     * \see dbHasRasterlite2
    * \since QGIS 3.0
    */
    int dbRasterLite2VersionRevision() const { return mRasterLite2VersionRevision; }

    /**
     * If  RasterLite2 can be used
     *  -  must be called before any RasterLite2 functions are called
     * \note
     *  If QGis has not been compiled with RasterLite2 support, false will be returned
     * \returns true if mRasterLite2VersionMajor is GT 0 (i.e. RL2_Version() has returned a value)
     * \see QgsSqliteHandle::initRasterlite2
     * \since QGIS 3.0
     */
    bool dbHasRasterlite2();

    /**
     * Amount of SpatialTables  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialTables that have been loaded
     * \since QGIS 3.0
     */
    int dbSpatialTablesLayersCount() const { return mHasSpatialTables; }

    /**
     * Amount of SpatialViews  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialViews that have been loaded
     * \since QGIS 3.0
     */
    int dbSpatialViewsLayersCount() const { return mHasSpatialViews; }

    /**
     * Amount of VirtualShapes found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of VirtualShapes that have been loaded
     * \since QGIS 3.0
     */
    int dbVirtualShapesLayersCount() const { return mHasVirtualShapes; }

    /**
     * Amount of RasterLite1-Rasters found in the Database
     * - only the count of valid Layers are returned
     * \note
     * - the Gdal-RasterLite1-Driver is needed to Determinse this
     * - this does not reflect the amount of RasterLite1-Rasters that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterLite1LayersCount() const { return mHasRasterLite1Tables; }

    /**
     * Is the Gdal-RasterLite1-Driver available ?
     * \note
     * - RasterLite1-Rasters can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalRasterLite1Driver() const { return mHasGdalRasterLite1Driver; }

    /**
     * Amount of RasterLite2 Vector-Coverages found in the Database
     * - from the vector_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 Vector-Coverages that have been loaded
     * \since QGIS 3.0
     */
    int dbVectorCoveragesLayersCount() const { return mHasVectorCoveragesTables; }

    /**
     * Amount of RasterLite2 Raster-Coverages found in the Database
     * - from the raster_coverages table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of RasterLite2 -Raster-Coverages that have been loaded
     * \since QGIS 3.0
     */
    int dbRasterCoveragesLayersCount() const { return mHasRasterCoveragesTables; }

    /**
     * Is the Gdal-RasterLite2-Driver available ?
     * \note
     * - RasterLite2-Rasters can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalRasterLite2Driver() const { return mHasGdalRasterLite2Driver; }

    /**
     * Does the read Database contain RasterLite2 SE_vector_styles_view
     * \note
     *   -1=no SE_vector_styles table, otherwise amount (0 being empty)
     * \since QGIS 3.0
     */
    int dbVectorStylesViewsCount() const { return mHasVectorStylesView; }

    /**
     * Does the read Database contain RasterLite2 SE_raster_styles_view
     * \note
     *   -1=no SE_raster_styles table, otherwise amount (0 being empty)
     * \since QGIS 3.0
     */
    int dbRasterStylesViewCount() const { return mHasRasterStylesView; }

    /**
     * Does the read Database contain any Styles (Vector
     * \note
     *   - information about the Styles are stored in mVectorStyleInfo
     * \see readVectorRasterStyles
     * \see mVectorStyleInfo
     * \see getDbVectorStylesInfo
     * \since QGIS 3.0
     */
    int dbVectorStylesCount() const { return mVectorStyleInfo.count(); }

    /**
     * Does the read Database contain any Styles ( Raster)
     * \note
     *   - information about the Styles are stored in mRasterInfo
     * \see readVectorRasterStyles
     * \see mRasterInfo     * \see getDbRastersInfo
     * \since QGIS 3.0
     */
    int dbRasterStylesCount() const { return mRasterStyleInfo.count(); }

    /**
     * Amount of Topologies found in the Database
     * - from the topologies table Table [-1 if Table not found]
     * \note
     * - this does not reflect the amount of Topology-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbTopologyExportLayersCount() const { return mHasTopologyExportTables; }

    /**
     * Amount of MBTiles found in the Database
     * - from the metadata table Table [-1 if Table not found, otherwise 1]
     * \note
     * - this does not reflect the amount of MBTiles-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbMBTilesLayersCount() const { return mHasMBTilesTables; }

    /**
     * Is the Gdal-MBTiles-Driver available ?
     * \note
     * - MBTiles can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalMBTilesDriver() const { return mHasGdalMBTilesDriver; }

    /**
     * Amount of GeoPackage Layers found in the Database
     * - from the geopackage_contents Table [-1 if Table not found, otherwise amount]
     * \note
     * - this does not reflect the amount of GeoPackage-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbGeoPackageLayersCount() const { return mHasGeoPackageTables; }

    /**
     * Amount of GeoPackage Vector-Layers found in the Database
     * - from the geopackage_contents Table [-1 if Table not found, otherwise amount]
     * \note
     * - this does not reflect the amount of GeoPackage-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbGeoPackageVectorsCount() const { return mHasGeoPackageVectors; }

    /**
     * Amount of GeoPackage Vector-Layers found in the Database
     * - from the geopackage_contents Table [-1 if Table not found, otherwise amount]
     * \note
     * - this does not reflect the amount of GeoPackage-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbGeoPackageRastersCount() const { return mHasGeoPackageRasters; }

    /**
     * Is the Gdal-GeoPackage-Driver available ?
     * \note
     * - GeoPackage can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbGdalGeoPackageDriver() const { return mHasGdalGeoPackageDriver; }

    /**
     * Amount of FdoOgr Layers found in the Database
     * - from the TODO Table [-1 if Table not found, otherwise amount]
     * \note
     * - this does not reflect the amount of FdoOgr-Layers that have been loaded
     * \since QGIS 3.0
     */
    int dbFdoOgrLayersCount() const { return mHasFdoOgrTables; }

    /**
     * Is the Gdal-FdoOgr-Driver available (SQLite) ?
     * \note
     * - GeoPackage can only be rendered when the Driver is available
     * \since QGIS 3.0
     */
    bool hasDbFdoOgrDriver() const { return mHasFdoOgrDriver; }

    /**
     * Has the used Spatialite compiled with Spatialite-Gcp support
     * - ./configure --enable-gcp=yes
     * \note
     *  - Based on GRASS GIS was initially released under the GPLv2+ license terms.
     * \returns true if GCP_* Sql-Functions are supported
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=Ground+Control+Points
     * \see getSpatialiteVersion
     * \since QGIS 3.0
     */
    bool hasDbGcpSupport() const { return mHasGcp; }

    /**
     * Has the used Spatialite compiled with Topology (and thus RtTopo) support
     * - ./configure --enable-rttopo
     * \note
     *  - Based on RT Topology Library
     * \returns true if Topology Sql-Functions are supported
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=ISO+Topology
     * \see https://git.osgeo.org/gogs/rttopo/librttopo
     * \see getSpatialiteVersion
     * \since QGIS 3.0
     */
    bool hasDbTopologySupport() const { return mHasTopology; }

    /**
     * Is the used connection Spatialite 4.5.0 or greater
     * \note
     *  - bsed on values offrom spatialite_version()
     * \see getSpatialiteVersion
    * \since QGIS 3.0
    */
    bool isDbVersion45() const { return mIsVersion45; }

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
    int dbLoadedLayersCount() const { return mLayersCount; }

    /**
     * Sum of all LayerTypes found
     * \note
     *   independent of Layer-Types
     * \see mHasVectorLayers
     * \see mHasRasterLite1Tables
     * \see mHasTopologyExportTables
     * \see mHasVectorCoveragesTables
     * \see mHasRasterCoveragesTables
     * \see mHasGeoPackageTables
     * \see mHasFdoOgrTables
     * \see mHasMBTilesTables
     * \see getSniffLayerMetadata
     * \since QGIS 3.0
     */
    int dbLayersCount() const { return mTotalCountLayers; }

    /**
     * Amount of Vector-Layers found
     * - SpatialTables, SpatialViews and VirtualShapes [from the vector_layers View]
     * -- contains: Layer-Name as 'table_name(geometry_name)', Geometry-Type (with dimension) and Srid
     * \note
     * - this amount may differ from dbLayersCount()
     * -- which only returns the amount of Loaded-Vector-Layers
     * \see dbLayersCount()
     * \since QGIS 3.0
     */
    int dbVectorLayersCount() const { return mVectorLayers.size(); }

    /**
     * Flag indicating if the layer data source (Sqkite3) has ReadOnly restrictions
     * \note
     *  Uses sqlite3_db_readonly( mSqliteHandle, "main" )
     * \returns result of sqlite3_db_readonly( mSqliteHandle, "main" )
     * \see getSniffDatabaseType
     * \see attachQSqliteHandle
    * \since QGIS 3.0
    */
    bool isDbReadOnly() const { return mReadOnly; }

    /**
     * Load all Layer-Information or only 'sniff' the Database
     * \note
     *  Default: extra Layer information is only retrieved when needed
     *  - this will be done automaticly, allways when used by a Provider
     * \returns true if all information  will be retrieved when running
     * \see getSpatialiteLayerInfo
    * \since QGIS 3.0
    */
    bool getDbLoadLayers() const { return mLoadLayers; }

    /**
     * Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \since QGIS 3.0
     */
    bool isDbValid() const { return mIsDbValid; }

    /**
     * Is the read Database locked
     * \note
     *  After a crash, a Database journal file may exist
     *  - sqlite3_prepare_v2 may fail with rc=5
     * \since QGIS 3.0
     */
    bool isDbLocked() const { return mIsDbLocked; }

    /**
     * Is the read Database valid, but does not contain any usable SpatialTables
     * \note
     *  Database may be used for something else or was newly created
     * \since QGIS 3.0
     */
    bool isDbEmpty() const { return mIsEmpty; }

    /**
     * Is the read Database a Spatialite Database
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \since QGIS 3.0
     */
    bool isDbSpatialite() const { return mIsSpatialite; }

    /**
     * Is the read Database a Spatialite Database that contains a RasterLite2 Layer
     * - supported by QgsRasterLite2Provider
     * \note
     *  - RasterLite2 specific functions should not be called when false
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    bool isDbRasterLite2() const { return mIsRasterLite2; }

    /**
     * Does the Spatialite Database support Spatialite commands >= 4.0 ?
     * - avoids execution of such cammands
     * \note
     *  - InvalidateLayerStatistics
     * \since QGIS 3.0
     */
    bool isDbSpatialite40() const { return mIsSpatialite40; }

    /**
     * Does the Spatialite Database support Spatialite commands >= 4.5 ?
     * - avoids execution of such cammands
     * \note
     *  - Topology
     * \since QGIS 3.0
     */
    bool isDbSpatialite45() const { return mIsSpatialite45; }

    /**
     * The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool isDbGdalOgr() const { return mIsGdalOgr; }

    /**
     * Does the file contain the Sqlite3 'Magic Header String'
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
      *  - SniffDatabaseType: Determins Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer]
      *  - SniffExtendend: (planned) possible Editing of of Layer-Propeties such as Column-Names
      * \since QGIS 3.0
      */
    SpatialSniff getSniffType() const { return mSniffType; }

    /**
      * Get SniffType as String
      *  - based on the Task, restrict unneeded activities
      *  -> For Browser: only Sqlite3-Container-Type is needed
      *  -> For Provider-Lists: only Tables/Layers without details
      *  -> For Provider: only Tables/Layers with all details
      * \note
      *  - SniffDatabaseType: Determins Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer]
      *  - SniffExtendend: (planned) possible Editing of of Layer-Propeties such as Column-Names
      * \since QGIS 3.0
      */
    QString getSniffTypeString() const { return SpatialiteDbInfo::SniffTypeString( mSniffType ); }

    /**
      * Set SniffType
      *  - based on the Task, restrict unneeded activities
      *  -> For Browser: only Sqlite3-Container-Type is needed
      *  -> For Provider-Lists: only Tables/Layers without details
      *  -> For Provider: only Tables/Layers with all details
      * \note
      *  - SniffDatabaseType: Determine Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer]
      *  - SniffExtendend: (planned) possible Editing of of Layer-Propeties such as Column-Names
      * \since QGIS 3.0
      */
    void setSniffType( SpatialSniff sniffType );

    /**
     * Set if checking has been done to insure that this is a Sqlite3 file
     * \see SpatialiteDbInfo::FetchSpatialiteDbInfo
     * \see SpatialiteDbInfo::readSqlite3MagicHeader
     * \since QGIS 3.0
     */
    void setDbSqlite3( bool bIsSqlite3 ) { mIsSqlite3 = bIsSqlite3; }

    /**
     * Retrieve Map of corresponding to the given Layer-Type
     * - short list of the Layers retrieved from a Layer-Type
     * \note
     *  This is a convenience function
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
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

    /**
     * Map of tables and views that are contained in the VectorLayers
     * - short list of the Layers retrieved from vector_layers
     *  - ordered by LayerType (SpatialTable, SpatialView and VirtualShape)
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbVectorLayers() const { return mVectorLayers; }

    /**
     * Map of coverage_name that are contained in the RasterLite2 Vector-Layers
     * - short list of the Layers retrieved from vector_coverages_ref_sys
     *  - ordered by coverage_name
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbVectorCoveragesLayers() const { return mVectorCoveragesLayers; }

    /**
     * Map of coverage_name Extent information that are contained in the RasterLite2 Vector-Layers
     * - short list of the Layers retrieved from vector_coverages_ref_sys
     *  - ordered by coverage_name, native_srid DESC
     * \note
     * - There will allways be a 'native' srid/extent with the corresponding Wsg84 (4326) values
     * - There can be any amount of 'alternative' srid/extent with the corresponding Wsg84 (4326) values
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: native_srid€srid€;extent_minx€extent_miny€extent_maxx€extent_maxy€geo_minx€geo_miny€geo_maxx€geo_maxy
     * \see mVectorCoveragesLayersExtent
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMultiMap<QString, QString> getDbVectorCoveragesLayersExtent() const { return mVectorCoveragesLayersExtent; }

    /**
     * Map of coverage_name Extent information that are contained in the RasterLite2 Raster-Layers
     * - short list of the Layers retrieved from raster_coverages_ref_sys
     *  - ordered by coverage_name, native_srid DESC
     * \note
     * - There will allways be a 'native' srid/extent with the corresponding Wsg84 (4326) values
     * - There can be any amount of 'alternative' srid/extent with the corresponding Wsg84 (4326) values
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: native_srid€srid€;extent_minx€extent_miny€extent_maxx€extent_maxy€geo_minx€geo_miny€geo_maxx€geo_maxy
     * \see mVectorCoveragesLayersExtent
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMultiMap<QString, QString> getDbRasterCoveragesLayersExtent() const { return mRasterCoveragesLayersExtent; }

    /**
     * Map of coverage_name that are contained in the RasterLite2 Raster-Layers
     * - short list of the Layers retrieved from raster_coverages_ref_sys
     *  - ordered by coverage_name
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbRasterCoveragesLayers() const { return mRasterCoveragesLayers; }

    /**
     * Map of style_id of Vector-Style and XML-Document
     * \note
     * - Key: StyleId as retrieved from SE_vector_styled_layers_view
     * - Value: Style XML-Document
     * -> style_type: StyleVector
     * - SELECT style_id, XB_GetDocument(style,1) FROM SE_vector_styled_layers_view;
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> getDbVectorStylesData() const { return mVectorStyleData; }

    /**
     * Map of style_id of Raster-Style and XML-Document
     * \note
     * - Key: StyleId as retrieved from SE_raster_styled_layers_view
     * - Value: Style XML-Document
     * -> style_type: StyleRaster
     * - SELECT style_id, XB_GetDocument(style,1) FROM SE_raster_styled_layers_view
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> getDbRasterStylesData() const { return mRasterStyleData; }

    /**
     * Map of style_name and Metadata of VectorStyle
     * \note
     * - Key: StyleName as retrieved from SE_vector_styled_layers_view
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleVector
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> getDbVectorStylesInfo() const { return mVectorStyleInfo; }

    /**
     * Map of style_name and Metadata of RasterStyle
     * \note
     * - Key: StyleName as retrieved from SE_raster_styled_layers_view
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleRaster
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> getDbRasterStylesInfo() const { return mRasterStyleInfo; }

    /**
     * Map of table_name that are contained in the RasterLite1Layers
     * - short list of the Layers retrieved from layer_statistics
     *  - Either:
     *  -> table_name with  raster_layer=1
     *  -> table_name_metadata with  raster_layer=0
     *  - Checks:
     *  -> TABLEs 'table_name'_metadata and 'table_name'_rasters must exist
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readRasterLite1Layers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbRasterLite1Layers() const { return mRasterLite1Layers; }

    /**
     * List of topolayer_name that are contained in the  'topologies' TABLE
     * \note
     * - Used to retrieve Export Layers
     * \see readTopologyLayers()
     * \since QGIS 3.0
     */
    QStringList getDbTopologyNames() const { return mTopologyNames; }

    /**
     * Map of topolayer_name that are contained in the Export-TopologyLayers
     * - Export Layers retrieved from topology_name of topologies
     *  -> and topology_name from 'topology_name'_topolayers TABLE
     * \note
     * - Key: LayerName formatted as 'topologies(topolayer_name)'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readTopologyLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbTopologyExportLayers() const { return mTopologyExportLayers; }

    /**
     * Map of layers from metadata
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

    /**
     * Map of table_name that are contained in geopackage_contents
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

    /**
     * Map of f_table_name that are contained in the TABLE geometry_columns for Gdal-FdoOgr
     *  - FdoOgr  geometry_columns contains different fields as the Spatialite Version
     *  -> this must be rendered with QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * - Checking is done if the SQLite' Gdal/Ogr Driver is active
     * \see readFdoOgrLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbFdoOgrLayers() const { return mFdoOgrLayers; }

    /**
     * Map of table_name that are contained in geopackage_contents
     *  -> this must be rendered be the QgsGdalProvider or QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name'
     * - Value: LayerType and Srid formatted as 'layer_type:srid'
     * -> layer_type: GeoPackageVector or GeoPackageRaster
     * - Checking is done if the 'GPKG' Gdal/Ogr Driver is active
     * \see readGeoPackageLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDbNonSpatialTables() const { return mNonSpatialTables; }

    /**
     * Amount of SpatialTables  found in the Database
     * - from the vector_layers View
     * \note
     * - this does not reflect the amount of SpatialTables that have been loaded
     * \since QGIS 3.0
     */
    int dbNonSpatialTablesCount() const { return mNonSpatialTables.count(); }

    /**
     * UpdateLayerStatistics for the Database or Layers
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

    /**
     * Retrieve Map of valid Layers supported by this Database [mLayerName as Key]
     * - contains Layer-Name and a SpatialiteDbLayer-Pointer
     * \note
     * - contains all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * -- when  getDbLoadLayers is true, getSpatialiteDbLayer will load all layers
     * \returns mDbLayers Map of LayerNames and valid SpatialiteDbLayer entries
     * \see getDbLoadLayers
     * \since QGIS 3.0
     */
    QMap<QString, SpatialiteDbLayer *> getDbLayers() const { return mDbLayers; }

    /**
     * Collection list of unique (lower case) Group Layers
     * - with the amount of Layer for each Group
     * \note
     *  When: 'table_name' contains at least 2 '_' (such as 'berlin_streets_1650')
     *  - assume it belongs to a group of 'berlin_streets' for QgsSpatiaLiteTableModel
     *  - for SpatialViews only
     * \since QGIS 3.0
     */
    QMap<QString, int> getListGroupNames() const { return mMapGroupNames; }

    /**
     * Map of valid Selected Layers requested by the User
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

    /**
     * Add a list of database layers to the map
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

    /**
     * List of DataSourceUri of valid Layers
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

    /**
     * Resolve unset settings about the Database
     * Goal: to (semi) Automate unresolved settings when needed
     * - will be called after major information has been retrieved from the Database
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \see prepareCapabilities
     * \see checkLayerStyles
     * \returns mIsLayerValid if the Layer is considered valid
     * \since QGIS 3.0
     */
    bool prepare();

    /**
     * Retrieve SpatialiteDbLayer-Pointer for a given
     * -  Layer-Name or Uri of Layer
     * - starts Spatialite Connection, if not done allready
     * \note
     * - can retrieve all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * \param sLayerName Layer-Name or Uri of Layer
     * \param loadLayer when true [default], Load Layer-Information if not already loaded
     * \see GetDbLayersInfo
     * \see QgsRasterLite2Provider::setSqliteHandle
     * \see QgsSpatiaLiteProvider::setSqliteHandle
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *getSpatialiteDbLayer( QString sLayerName, bool loadLayer = true );

    /**
     * Retrieve formatted Uris
     * - retrieved frommDbLayersDataSourceUris
     * \note
     * - created in prepareDataSourceUris
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true if the count of valid-layers > 0
     * \see createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString getDbLayerUris( QString sLayerName );

    /**
     * Retrieve formatted Geometry-Type, Srid and Provider-Key
     * - retrieved mDbLayersDataSourceInfo
     * \note
     * - created in prepareDataSourceUris
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true if the count of valid-layers > 0
     * \see parseLayerInfo
     * \since QGIS 3.0
     */
    QString getDbLayerInfo( QString sLayerName );

    /**
     * Retrieve SpatialiteDbLayer-Pointer for a given
     * -  LayerId
     * \note
     * - LayerId is only valid for the active session and MUST not be used for project storing
     * \param sLayerName Layer-Name or Uri of Layer
     * \param loadLayer when true [default], Load Layer-Information if not already loaded
     * \see getSpatialiteDbLayer
     * \since QGIS 3.0
     */
    SpatialiteDbLayer *getSpatialiteDbLayer( int layerId );

    /**
     * Returns the enum  value from the given String of the Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialiteLayerType SpatialiteLayerTypeFromName( const QString &typeName );

    /**
     * Returns the String value of the enum of Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteDbLayer::setLayerType
     * \since QGIS 3.0
     */
    static QString SpatialiteLayerTypeName( SpatialiteDbInfo::SpatialiteLayerType layerType );

    /**
     * Returns the QIcon representation  of the enum of Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteDbLayer::setLayerType
     * \since QGIS 3.0
     */
    static QIcon SpatialiteLayerTypeIcon( SpatialiteDbInfo::SpatialiteLayerType layerType );

    /**
     * Returns the QIcon representation  of non-Spatialial or Spatial-Admin TABLEs in a Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteDbLayer::setLayerType
     * \since QGIS 3.0
     */
    static QIcon NonSpatialTablesTypeIcon( QString typeName );

    /**
     * Returns QIcon representation of the enum of a Geometry-Type
     * - QgsWkbTypes::Type
     * \note
     * - QgsWkbTypes::Point, Multi and 25D
     * - QgsWkbTypes::LineString, Multi and 25D
     * - QgsWkbTypes::Polygon, Multi and 25D
     * \see getSpatialiteTypes
     * \since QGIS 3.0
     */
    static QIcon SpatialGeometryTypeIcon( QgsWkbTypes::Type geometryType );

    /**
     * Returns the String value of the enum of SpatialIndex-Types
     * \see SpatialiteDbLayer::setSpatialIndexType
     * \since QGIS 3.0
     */
    static QString SpatialIndexTypeName( SpatialiteDbInfo::SpatialIndexType spatialIndexType );

    /**
     * Returns the enum  value from the given String of a Sqlite3 Container-Types
     * - SpatialiteDbInfo::SpatialMetadata
     * \note
     * - SpatialiteLegacy, SpatialiteFdoOgr, Spatialite40, SpatialiteGpkg, Spatialite45, SpatialiteMBTiles and SpatialUnknown
     * \param typeName enum as String
     * \returns SpatialMetadata enum
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialMetadata SpatialMetadataTypeFromName( const QString &typeName );

    /**
     * Returns the String value of the enum of SpatialMetadata-Types (Sqlite3 Container-Types)
     * - SpatialiteDbInfo::SpatialMetadata
     * \note
     * - SpatialiteLegacy, SpatialiteFdoOgr, Spatialite40, SpatialiteGpkg, Spatialite45, SpatialiteMBTiles and SpatialUnknown
     * \param spatialMetadataType SpatialMetadata enum
     * \returns SpatialMetadata enum as String
     * \see SpatialiteDbInfo::setSpatialMetadata
     * \since QGIS 3.0
     */
    static QString SpatialMetadataTypeName( SpatialiteDbInfo::SpatialMetadata spatialMetadataType );

    /**
     * Returns QIcon representation of the enum of SpatialMetadata-Types (Sqlite3 Container-Types)
     * - SpatialiteDbInfo::SpatialMetadata
     * \note
     * - SpatialiteLegacy, SpatialiteFdoOgr, Spatialite40, SpatialiteGpkg, Spatialite45, SpatialiteMBTiles and SpatialUnknown
     * \param spatialMetadataType SpatialMetadata enum
     * \returns SpatialMetadata enum as String
     * \see SpatialiteDbInfo::getSpatialMetadataIcon
     * \since QGIS 3.0
     */
    static QIcon SpatialMetadataTypeIcon( SpatialiteDbInfo::SpatialMetadata spatialMetadataType );

    /**
     * Returns QIcon representation of the enum of SpatialMetadata-Types (Sqlite3 Container-Types)
     * - SpatialiteDbInfo::SpatialMetadata
     * \note
     * - SpatialiteLegacy, SpatialiteFdoOgr, Spatialite40, SpatialiteGpkg, Spatialite45, SpatialiteMBTiles and SpatialUnknown
     * \param spatialMetadataType SpatialMetadata enum
     * \returns SpatialMetadata enum as Icon
     * \see SpatialiteDbInfo::setSpatialMetadata
     * \since QGIS 3.0
     */
    QIcon getSpatialMetadataIcon() const { return SpatialiteDbInfo::SpatialMetadataTypeIcon( mSpatialMetadata ); }

    /**
     * Returns the enum  value from the given String of a SpatialIndex-Types
     * \note
     * - at the moment not used but intended for external use
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo::SpatialIndexType SpatiaIndexTypeFromName( const QString &typeName );

    /**
     * Convert QVariant::String to a more realistic type
     * -DateTime ?
     * \note
     * New versions of OGR convert list types (StringList, IntegerList, Integer64List and RealList)
     * to JSON when it stores a Spatialite table. It sets the column type as JSONSTRINGLIST,
     * JSONINTEGERLIST, JSONINTEGER64LIST or JSONREALLIST
     * \since QGIS 3.0
     */
    static TypeSubType GetVariantType( const QString &type );

    /**
     * List of Table-Types
     * -  with short description on how the Table is used
     * \note
     * - intended for gui use when listing tables, or as title of a sub-group where those type are stored in
     * \since QGIS 3.0
     */
    static QStringList getSpatialiteTypes();

    /**
     * Retrieve Capabilities of QgsSqliteHandle connection
     * - at this point we are 'sniffing' the Capabilities of the opened Database
     * \note
     *  - when only a specific table or table with geometry are being looked for
     *  -> format: 'table_name(geometry_name)' or only 'table_name', with all of its geometries
     *  - otherwise all tables with geometries
     * \param sLayerName when used
     * \param bLoadLayers Load all Layer-Information or only 'sniff' [default] the Database
     * \returns SpatialiteDbInfo with collected results
     * \see QgsSqliteHandle
     * \since QGIS 3.0
     */
    bool getSpatialiteLayerInfo( QString sLayerName = QString::null, bool bLoadLayers = false, SpatialSniff sniffType = SpatialiteDbInfo::SniffUnknown );

    /**
     * Attach and existing SpatialiteDbInfo object to a QSqliteHandle
     * - at this point we are 'sniffing' the Capabilities of the opened Database
     * \note
     *  - checking is done that the Database is not readonly
     * \param qSqliteHandle
     * \returns true is succesfull
     * \see QgsSqliteHandle
     * \see SpatialiteDbInfo::FetchSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool attachQSqliteHandle( QgsSqliteHandle *qSqliteHandle );

    /**
     * Collection of warnings for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> getWarnings() const { return mWarnings; }

    /**
     * Collection of reasons for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> getErrors() const { return mErrors; }

    /**
     * The internal Parse Separator
     * - for general usage
     * \note
     *  - where it may be assumed that ':' is not used
     *  \see ParseSeparatorCoverage
     * \since QGIS 3.0
     */
    static const QString ParseSeparatorGeneral;

    /**
     * The internal Parse Separator
     * - for general usage in Uril
     * \note
     *  - where it may be assumed that ':' is not used
     *  \see ParseSeparatorCoverage
     * \since QGIS 3.0
     */
    static const QString ParseSeparatorUris;

    /**
     * The internal Parse Separator
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

    /**
     * Retrieve the selected vector Style
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     * \param iStyleId to retrieve
     * \returns xml QString of Xml-Document
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getDbVectorStyleXml( int iStyleId )
    {
      if ( !mVectorStyleData.isEmpty() )
      {
        if ( mVectorStyleData.contains( iStyleId ) )
        {
          return mVectorStyleData.value( iStyleId );
        }
      }
      return QString();
    }

    /**
     * Retrieve the selected vector Style
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     * \param iStyleId to retrieve
     * \returns xml QString of Xml-Document
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getDbRasterStyleXml( int iStyleId )
    {
      if ( !mRasterStyleData.isEmpty() )
      {
        if ( mRasterStyleData.contains( iStyleId ) )
        {
          return mRasterStyleData.value( iStyleId );
        }
      }
      return QString();
    }

    /**
     * Map of LayerNames and style_id
     * \note
     * This information can be retrieved without a Spatialite connecton.
     * - Key: coverage_name from SE_vector_styled_layers or SE_raster_styled_layers
     * - Value: style_id
     * -> SpatialiteDbLayer will retrieve the style_id that belongs to it [there may be more than 1]
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMultiMap<QString, int> getLayerNamesStyles() const { return mLayerNamesStyles; }

    /**
     * Retrieve the selected Style
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - return QDomElement to be used with QgsVectorLayer::readSld
     *   - called while loading Layers
     * \param styleType to retrieve [StyleVector, StyleRaster]
     * \param iStyleId to retrieve
     * \param errorMessage for messages returning any caus of error
     * \param iDebug to call test when set to 1, returning return code
     * \param sSaveXmlFileName filename to save xlm, when not null - for testing
     * \returns xlm QDomElement containing 'NamedLayer'
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QDomElement getDbStyleNamedLayerElement( SpatialiteDbInfo::SpatialiteLayerType styleType, int iStyleId, QString &errorMessage, int *iDebug = nullptr, QString sSaveXmlFileName = QString::null );

    /**
     * Create LayerInfo String and Uri
     * - intended a a central point where these setting should be made
     * - any changes in the Provirder Names or Uri-Syntax must bedone here
     * \note
     *  LayerInfo
     *  - field 0=Geometry-Type [for Raster-Types: LayerType]
     *  - field 1=Srid of the Layer
     *  - field 2=The Provider Name
     *  DataSourceUri
     *  - Provider dependent
     * \param sLayerInfo String to return the result of the LayerInfo
     * \param DataSourceUri String to return the result of the DataSourceUri
     * \param sLayerName Name of the Layer to to use format: 'table_name(geometry_name)'
     * \param layerType the Layer-Type being formatted
     * \param sGeometryType the Layer Geometry-Type to use
     * \param iSrid ofthe Layer Geometry-Type to use
     * \param iSpatialIndex the Layer Geometry SpatialIndex to use [-1=GAIA_VECTOR_UNKNOWN]
     * \param iIsHidden is the Layer hidden
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
    QString createDbLayerInfoUri( QString &sLayerInfo, QString &sDataSourceUri, QString sLayerName, SpatialiteDbInfo::SpatialiteLayerType layerType, QString sGeometryType, int iSrid, int iSpatialIndex = -1, int iIsHidden = 0 );

    /**
     * Parse the LayerInfo String
     * - intended to store and retrieve general Infomation about the Layer
     * - needed when 'sniffing' the Layer, when no SpatialiteDbLayer exists
     * \note
     *  - the LayerInfo String is created in createDbLayerInfoUri
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * \param sLayerInfo the input to parse
     * \param sGeometryType the returned result from field 0 of sLayerInfo
     * \param iSrid the returned result from field 1 of sLayerInfo
     * \param sProvider the returned result from field 2 of sLayerInfo
     * \param sLayerType the returned result from field 3 of sLayerInfo
     * \param iSpatialIndex the returned result from field 4 of sLayerInfo
     * \param iIsHidden the returned result from field 5 of sLayerInfo
     * \returns true if the correct amount of ields were found
     * \see prepareDataSourceUris
     * \see addDbMapLayers
     * \see QgsSpatialiteLayerItem
     * \since QGIS 3.0
     */
    bool parseLayerInfo( QString sLayerInfo, QString &sGeometryType, int &iSrid, QString &sProvide, QString &sLayerType, int &iSpatialIndex, int &iIsHidden );

    /**
     * Parse the LayerCoverage String
     * - intended to store and retrieve general Infomation about the Raster Layer
     * - needed when 'sniffing' the Layer, when no SpatialiteDbLayer exists
     * \note
     *  - the LayerInfo String is created in createDbLayerInfoUri
     *  - SpatialiteDbInfo::ParseSeparatorCoverage ('€') is used a separator
     * \param sLayerInfo the input to parse
     * \param sLayerType the returned result from field 0 of sLayerInfo
     * \param sLayerName the returned result from field 1 of sLayerInfo
     * \param sTitle the returned result from field 2 of sLayerInfo
     * \param sAbstract the returned result from field 3 of sLayerInfo
     * \param sCopyright the returned result from field 4 of sLayerInfo
     * \param sLicense the returned result from field 5 of sLayerInfo
     * \param iSrid the returned result from field 5 of sLayerInfo
     * \returns true if the correct amount of ields were found
     * \see prepareDataSourceUris
     * \see addDbMapLayers
     * \see QgsSpatialiteLayerItem
     * \since QGIS 3.0
     */
    bool parseLayerCoverage( QString sLayerInfo, QString &sLayerType,  QString &sLayerName, QString &sTitle, QString &sAbstract, QString &sCopyright, QString &sLicense, int &iSrid );

    /**
     * Drop GeoTable
     * - as Sql-Command since 4.3.0
     * - as Api-Function 'gaiaDropTable' since 4.0.0
     * \note
     *  Based on the running version of spatialite, the Sql or Api Version will be used
     *  - Both the Sql and Api-Versions remove the Spatial-Index and
     * ->  all corresponding entries
     *  - 'VACUUM' will be called, when no errors occured
     *  All Layers contained in this TABLE will be removed, since they no longer exist
     * \param sTableName to delete
     * \param errCause OUT possible error Message
     * \returns true when no errors
     * \see QgsSpatiaLiteProvider::deleteLayer
     * \see removeGeoTable
     * \since QGIS 3.0
     */
    bool dropGeoTable( QString sTableName, QString &errCause );
  protected:
  signals:

    /**
     * Add a list of database layers to the map
     * \note
     * Not used
    * \since QGIS 3.0
    */
    void addDatabaseLayers( QStringList const &paths, QString const &providerKey );
  private:

    /**
     * The Database filename (with Path)
    * \returns mDatabaseFileName complete Path (without without symbolic links)
    * \since QGIS 3.0
    */
    QString mDatabaseFileName;

    /**
     * The Database filename without path
    * \returns mFileName  name of the file, excluding the path
    * \since QGIS 3.0
    */
    QString mFileName;

    /**
     * The Database Directory without file
    * \returns mDirectoryName  name of the complete  path,  (without without symbolic links), excluding the file-name
    * \since QGIS 3.0
    */
    QString mDirectoryName;

    /**
     * The sqlite handler
     *  - created in QgsSqliteHandle
     * \note
     *  - closing done through QgsSqliteHandle
     * \see QgsSqliteHandle::openDb
     * \see attachQSqliteHandle
    * \since QGIS 3.0
    */
    sqlite3 *mSqliteHandle = nullptr;

    /**
     * The class allowing to reuse the same sqlite handle for more layers
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    QgsSqliteHandle *mQSqliteHandle = nullptr;

    /**
     * The Spatialite internal Database structure being read
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    SpatialiteDbInfo::SpatialMetadata mSpatialMetadata;

    /**
     * Database create pption
      * - when empty (or does not exist) create valid SpatialDatabase based on this option
     * \note
     * - SpatialUnknown,: [default]: assume Database exist and is valid (do not create)
     * - SpatialiteLegacy, 40, 45: a valid (empty) SpatialLite Database, if empty
     * - SpatialiteGpkg: a valid (empty) GeoPackage Database, if empty
     * - SpatialiteMBTiles: a valid (empty) Mbtiles Database, if empty
     * - SpatialiteFdoOgr: not supported (treat as if SpatialUnknown)
    * \since QGIS 3.0
    */
    SpatialiteDbInfo::SpatialMetadata mDbCreateOption;

    /**
     * The Spatialite internal Database structure being read (as String)
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString mSpatialMetadataString;

    /**
     * Set the Sqlite3-Container type being read (and the String)
     * \note
     *  - if not known, will check for MBTiles
     * - this function does not need a Spatailite connection
     * \see getSniffDatabaseType
     * \see checkMBTiles
    * \since QGIS 3.0
    */
    void setSpatialMetadata( int iSpatialMetadata );

    /**
     * Contains collected Metadata for the Database
     * \brief A structured metadata store for a map layer.
     * \note
     *  - SpatialiteDbLayer will use a copy this as starting point
     * \see QgsMapLayer::htmlMetadata()
     * \see QgsMapLayer::metadata
     * \see QgsMapLayer::setMetadata
    * \since QGIS 3.0
    */
    QgsLayerMetadata mDbMetadata;

    /**
     * Set the collected Metadata for the Layer
     * \brief A structured metadata store for a map layer.
     * \note
     *  - SpatialiteDbLayer will use a copy of SpatialiteDbInfo  mLayerMetadata as starting point
     * \see prepare
     * \see QgsMapLayer::htmlMetadata()
     * \see QgsMapLayer::metadata
     * \see QgsMapLayer::setMetadata
    * \since QGIS 3.0
    */
    bool setLayerMetadata();

    /**
     * The Spatialite Version Driver being used
     * \note
     *  - returned from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString mSpatialiteVersionInfo;

    /**
     * The major Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mSpatialiteVersionMajor;

    /**
     * The minor Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mSpatialiteVersionMinor;

    /**
     * The revision Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mSpatialiteVersionRevision;

    /**
     * The Spatialite Version Driver being used
     * \note
     *  - returned from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString mRasterLite2VersionInfo;

    /**
     * The major Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mRasterLite2VersionMajor;

    /**
     * The minor Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mRasterLite2VersionMinor;

    /**
     * The revision Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int mRasterLite2VersionRevision;

    /**
     * Sum of all LayerTypes found
     * \note
     *   independent of Layer-Types
     * \see mHasVectorLayers
     * \see mHasRasterLite1Tables
     * \see mHasTopologyExportTables
     * \see mHasVectorCoveragesTables
     * \see mHasRasterCoveragesTables
     * \see mHasGeoPackageTables
     * \see mHasFdoOgrTables
     * \see mHasMBTilesTables
     * \see getSniffLayerMetadata
     * \since QGIS 3.0
     */
    int mTotalCountLayers;

    /**
     * Does the read Database contain SpatialTables, SpatialViews or VirtualShapes
     * - determine through Sql-Queries, without usage of any Drivers
     * -> for Databases created with a version before 4.0.0, vector_layers will be simulated
     * \note
     *   -1=no vector_layers entries, otherwise amount (0 being empty)
     * \see getSniffMinimal
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasVectorLayers;

    /**
     * Does the read Database contain a vector_layers_auth for read_only and hidden
     * - determine through Sql-Queries, without usage of any Drivers
     * -> for Databases created with a version before 4.0.0, vector_layers_auth will be simulated
     * \note
     *   -1=no vector_layers_auth entries, 0= table exists [for read_only and hidden]
     * \see getSniffMinimal
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasVectorLayersAuth;

    /**
     * Does the read Database contain SpatialTables, without vector_layers [ 0=none, otherwise amount]
     * - when set the VectorLayers logic will read geometry_columns instead of vector_layers
     * \note
     *  - final result will be set to VectorLayers
     *  -> so that externaly it is the same
     * \see getSniffMinimal
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasLegacyGeometryLayers;

    /**
     * Does the read Database contain SpatialTables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no Spatialite specific geometry_columns entries, otherwise amount (0 being empty)
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasSpatialTables;

    /**
     * Does the read Database contain SpatialViews views
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no views_geometry_columns entries, otherwise amount (0 being empty)
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasSpatialViews;

    /**
     * Does the read Database contain VirtualShapes tables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no virts_geometry_columns entries, otherwise amount (0 being empty)
     * \see getSniffReadLayers
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mHasVirtualShapes;

    /**
     * Does the read Database contain RasterLite1 coverages
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no rasterlit1 logic found, otherwise amount (0 being empty)
     * \see getSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readRasterLite1Layers
     * \see GetRasterLite1LayersInfo
     * \since QGIS 3.0
     */
    int mHasRasterLite1Tables;

    /**
     * Is the Gdal-RasterLite1-Driver available ?
     * - GDALGetDriverByName( 'RasterLite' )
     * \note
     *  - QgsGdalProvider:  RasterLite1
     * \since QGIS 3.0
     */
    bool mHasGdalRasterLite1Driver;

    /**
     * Does the read Database contain RasterLite2 Vector coverages
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no raster_coverages table, otherwise amount (0 being empty)
     * \see getSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readVectorRasterCoverages
     * \see GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    int mHasVectorCoveragesTables;

    /**
     * Does the read Database contain RasterLite2 Raster coverages
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no raster_coverages table, otherwise amount (0 being empty)
     * \see getSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readVectorRasterCoverages
     * \see GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    int mHasRasterCoveragesTables;

    /**
     * Is the Gdal-RasterLite2-Driver available ?
     * - GDALGetDriverByName( 'SQLite' )
     * \note
     *  - QgsGdalProvider:  RasterLite2
     *  - assuming Gdal has been compiled with Rasterite2 support
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    bool mHasGdalRasterLite2Driver;

    /**
     * Does the read Database contain Topology tables ]
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no topologies table, otherwise amount (0 being empty)
     * \see getSniffMinimal
     * \see getSniffReadLayers
     * \see readTopologyLayers
     * \see GetTopologyLayersInfo
     * \since QGIS 3.0
     */
    int mHasTopologyExportTables;

    /**
     * Does the read Database contain MbTiles-Tables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none, otherwise 3=View-based MbTiles, else 2
     * \see checkMBTiles
     * \see readMBTilesLayers
     * \see GetMBTilesLayersInfo
     * \since QGIS 3.0
     */
    int mHasMBTilesTables;

    /**
     * Is the Gdal-MBTiles-Driver available ?
     * - GDALGetDriverByName( 'MBTiles' )
     * \note
     *  - QgsGdalProvider:  MBTiles
     * \see readMBTilesLayers
     * \since QGIS 3.0
     */
    bool mHasGdalMBTilesDriver;

    /**
     * Does the read Database contain GPKG-Tables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none,  if 'gpkg_contents'  exists,  otherwise amount (0 being empty) [GeoPackage Revision 10]
     * \see getSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readGeoPackageLayers
     * \see GetGeoPackageLayersInfo
     * \since QGIS 3.0
     */
    int mHasGeoPackageTables;

    /**
     * Does the read Database contain GPKG-Vector-Tables [features]
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none,  if 'gpkg_contents'  exists,  otherwise amount (0 being empty) [GeoPackage Revision 10]
     * \see readGeoPackageLayers
     * \see GetGeoPackageLayersInfo
     * \since QGIS 3.0
     */
    int mHasGeoPackageVectors;

    /**
     * Does the read Database contain GPKG-Vector-Tables [tiles]
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none,  if 'gpkg_contents'  exists,  otherwise amount (0 being empty) [GeoPackage Revision 10]
     * \see readGeoPackageLayers
     * \see GetGeoPackageLayersInfo
     * \since QGIS 3.0
     */
    int mHasGeoPackageRasters;

    /**
     * Is the Gdal-GeoPackage-Driver available ?
     * - GDALGetDriverByName( 'GPKG' )
     * \note
     *  - QgsGdalProvider / QgsOgrProvider:  GPKG
     * \see readGeoPackageLayers
     * \since QGIS 3.0
     */
    bool mHasGdalGeoPackageDriver;

    /**
     * Does the read Database contain FdoOgr-Tables
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   0=none,  if OGR specific 'geometry_columns'  exists,  otherwise amount (0 being empty)
     * \see getSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readFdoOgrLayers
     * \see GetFdoOgrLayersInfo
     * \since QGIS 3.0
     */
    int mHasFdoOgrTables;

    /**
     * Is the Gdal-FdoOgr-Driver available ?
     * - GDALGetDriverByName( 'SQLite' )
     * \note
     *  - QgsOgrProvider:  SQLite
     * \see readFdoOgrLayers
     * \since QGIS 3.0
     */
    bool mHasFdoOgrDriver;

    /**
     * Does the read Database contain RasterLite2 SE_vector_styles_view
     * \note
     *   -1=no SE_vector_styles table, otherwise amount (0 being empty)
     * \see getSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readVectorRasterCoverages
     * \see dbVectorStylesViewCount
     * \since QGIS 3.0
     */
    int mHasVectorStylesView;

    /**
     * Does the read Database contain RasterLite2 SE_raster_styles_view
     * - determine through Sql-Queries, without usage of any Drivers
     * \note
     *   -1=no SE_raster_styles, otherwise amount (0 being empty)
     * \see getSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \see readVectorRasterCoverages
     * \see dbRasterStylesViewCount
     * \since QGIS 3.0
     */
    int mHasRasterStylesView;

    /**
     * Has the used Spatialite compiled with Spatialite-Gcp support
     * - ./configure --enable-gcp=yes
     * \note
     *  - Based on GRASS GIS was initially released under the GPLv2+ license terms.
     * \returns true if GCP_* Sql-Functions are supported
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=Ground+Control+Points
     * \see getSpatialiteVersion
     * \since QGIS 3.0
     */
    bool mHasGcp;

    /**
     * Has the used Spatialite compiled with Topology (and thus RtTopo) support
     * - ./configure --enable-rttopo
     * \note
     *  - Based on RT Topology Library
     * \returns true if Topology Sql-Functions are supported
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=ISO+Topology
     * \see https://git.osgeo.org/gogs/rttopo/librttopo
     * \see getSpatialiteVersion
     * \since QGIS 3.0
     */
    bool mHasTopology;

    /**
     * Is the used connection Spatialite 4.5.0 or greater
     * \note
     *  - bsed on values offrom spatialite_version()
     * \see getSpatialiteVersion
    * \since QGIS 3.0
    */
    bool mIsVersion45;

    /**
     * Layers-Counter
     * - not all of which may be valid and contained in mDbLayers
     * \note
     *  - mDbLayers will only contain Layers that are valid
     * \see GetDbLayersInfo
    * \since QGIS 3.0
    */
    int mLayersCount;

    /**
     * Flag indicating if the layer data source (Sqkite3) has ReadOnly restrictions
     * \note
     *  Uses sqlite3_db_readonly( mSqliteHandle, "main" )
     * \returns result of sqlite3_db_readonly( mSqliteHandle, "main" )
     * \see getSniffDatabaseType
     * \see attachQSqliteHandle
    * \since QGIS 3.0
    */
    bool mReadOnly;

    /**
     * Load all Layer-Information or only 'sniff' the Database
     * \note
     *  Default: extra Layer information is only retrieved when needed
     *  - this will be done automaticly, allways when used by a Provider
     * \returns true if all information  will be retrieved when running
     * \see getSpatialiteLayerInfo
    * \since QGIS 3.0
    */
    bool mLoadLayers;

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
     * Is the read Database locked
     * \note
     *  After a crash, a Database journal file may exist
     *  - sqlite3_prepare_v2 may fail with rc=5
     * \since QGIS 3.0
     */
    bool mIsDbLocked;

    /**
     * Is the read Database valid, but does not contain any usable SpatialTables
     * \note
     *  Database may be used for something else or was newly created
     * \since QGIS 3.0
     */
    bool mIsEmpty;

    /**
     * Is the read Database a Spatialite Database
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \since QGIS 3.0
     */
    bool mIsSpatialite;

    /**
     * Is the read Database a Spatialite Database that contains a RasterLite2 Layer
     * - supported by QgsRasterLite2Provider
     * \note
     *  - RasterLite2 specific functions should not be called when false
     * \see SpatialiteDbLayer::rl2GetMapImageFromRaster
     * \since QGIS 3.0
     */
    bool mIsRasterLite2;

    /**
     * Does the Spatialite Database support Spatialite commands >= 4?
     * - avoids execution of such cammands
     * \note
     *  - InvalidateLayerStatistics
     * \since QGIS 3.0
     */
    bool mIsSpatialite40;

    /**
     * Does the Spatialite Database support Spatialite commands >= 4.5?
     * - avoids execution of such cammands
     * \note
     *  - Topology
     * \since QGIS 3.0
     */
    bool mIsSpatialite45;

    /**
     * Is the read Database not supported by QgsSpatiaLiteProvider but
     * - supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool mIsGdalOgr;

    /**
     * Does the file contain the Sqlite3 'Magic Header String'
     * - UTF-8 string "SQLite format 3" including the nul terminator character at the end.
     * \since QGIS 3.0
     */
    bool mIsSqlite3;

    /**
     * Set the Database as invalid
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
      *  - SniffDatabaseType: Determins Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
      *  - SniffMinimal: Load and store Information about Tables/Layers with the amount for each [SpatialiteLayerType]
      *  - SniffLoadLayers: Load and store Information about Layers with details [SpatialiteDbLayer]
      *  - SniffExtendend: (planned) possible Editing of of Layer-Propeties such as Column-Names
      * \since QGIS 3.0
      */
    SpatialSniff mSniffType;

    /**
      * MimeType of file
      * \note
      *  - SniffDatabaseType and SniffMinimal: will search for MimeType when not MimeSqlite3 and the file exists
      *  - SniffLoadLayers: must be MimeSqlite3
      * \since QGIS 3.0
      */
    MimeType mMimeType;

    /**
     * Retrieves basic information about contents of the Database
     * - Implementation of SpatialSniff::SniffDatabaseType
     * \note
     *  - Is the Database in a Read-Only status
     *  -> retrieved by sqlite3
     * \returns mIsDbValid if no errors where found
     * \see getSpatialiteLayerInfo
     * \see setSpatialMetadata
     * \since QGIS 3.0
     */
    bool getSniffDatabaseType();

    /**
     * Retrieves Spatialite Version
     * - starts Spatialite Connection, if not done allready
     * \note
     *  - Retrieve MetaData returned by Spatialite (including Spatialite Version-String)
     *  -> SpatialTables/Views/RasterLite2/1, Topology, GeoPackage and MBtiles
     *  - Determine Spatialite Capabilities
     *  -> HasTopology(), HasGCP()
     * \returns mIsDbValid if no errors where found
     * \see getSpatialiteLayerInfo
     * \see setSpatialMetadata
     * \since QGIS 3.0
     */
    bool getSpatialiteVersion();

    /**
     * Groups the minimal retrieve functions so that can be called at need
     * - avoiding mutiple calls when using getSpatialiteLayerInfo
     * \note
     *  - dbLayersCount() should be 0
     * \returns mIsDbValid if no errors where found
     * \see getSniffDatabaseType
     * \see getSniffMinimal
     * \see getSniffLayerMetadata
     * \see getSniffReadLayers
     * \since QGIS 3.0
     */
    bool retrieveSniffMinimal();

    /**
     * Retrieves basic information about contents of the Database
     * - Implementation of SpatialSniff::SniffMinimal [Part 1]
     * \note
     *  - Parses Spatialite Version Information
     *  -> if Spatialite < 4.5, manually Determinse if reading a Spatialite 4.5 Database
     *  -> setting of Spatialite Major, Minor and Revision version numbers
     *  - Determine if Tables exists and are not empty
     *  -> SpatialTables/Views/RasterLite2/1, Topology and GeoPackage
     * - this function does not need a Spatailite connection
     * \returns mIsDbValid if no errors where found
     * \see getSpatialiteLayerInfo
     * \since QGIS 3.0
     */
    bool getSniffMinimal();

    /**
     * Retrieves basic information about contents of the Database
     * - Implementation of SpatialSniff::SniffMinimal [Part 2]
     * \note
     *  getSniffMinimal() will Determin if catalog tables exist from which the Metadata will be read
     *  - Determine the amount of RaststerLite1 Tables that exist
     *  -> minimal check if needed subtables exist with valid values
     *  - Determine for the Tables that exists how many entries they contain [empty tables will be ignored later]
     *  -> SpatialTables/Views/VirtualShapes
     *  - Determine the amount of Tables that exist for
     *  -> SpatialTables/Views/VirtualShapes
     *  -> RasterLite2 Coverages
     *  -> Topologies
     *  -> Geopackage
     *  - Build a list of non-SpatialTables
     * \returns mIsDbValid if no errors where found
     * \see getSpatialiteLayerInfo
     * \since QGIS 3.0
     */
    bool getSniffLayerMetadata();

    /**
     * Reading Layer-MetaTable data found, with sanity-checks
     * - Implementation of SpatialSniff::SniffMinimal [Part 3]
     * \note
     *  getSniffLayerMetadata( ) will Determin if catalog tables that exist are not empty
     *  - For the Tables that are not empty
     *  -> a specific function will be call to read and store the result
     *  - Clears mDbLayers [Collection of Loaded-layers]
     *  -> and minimal information (Layer-Name and Geometry-Type) stored in mVectorLayers
     *  -  Activating Foreign Key constraints for the Database
     * - there is no direct Spatialite or RasterLite2 driver support needed during this function
     * \returns mIsDbValid if no errors where found
     * \see readVectorLayers
     * \see readVectorRasterCoverages
     * \see readVectorRasterStyles
     * \see readTopologyLayers
     * \see readRasterLite1Layers
     * \see readMBTilesLayers
     * \see readGeoPackageLayers
     * \see readFdoOgrLayers
     * \since QGIS 3.0
     */
    bool getSniffReadLayers();

    /**
     * Retrieves extensive information about Layers in the Database
     * - Implementation of SpatialSniff::SniffLoadLayers
     * - starts Spatialite Connection, if not done allready
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
     * \returns mIsDbValid if no errors where found
     * \see getSpatialiteLayerInfo
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool getSniffLoadLayers( QString sLayerName );

    /**
     * Map of valid Layers supported by this Database [mLayerName as Key]
     * - contains Layer-Name and a SpatialiteDbLayer-Pointer
     * \note
     * - contains all Layer-Types (SpatialTable/View, RasterLite1/2, Topology and VirtualShape)
     * \see getSpatialiteDbLayer
     * \since QGIS 3.0
     */
    QMap<QString, SpatialiteDbLayer *> mDbLayers;

    /**
     * Function to remove known Admin-Tables from the Layers-List
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

    /**
     * Function to remove known Geo-Table from the Layers-List
     * - with any Geometry-Columns
     * \note
     *  Called after succesfull dropGeoTable
     *  - since the Layers no longer exist, they should not be rendered
     *  - What will happen if they are being render is unknown
     * \param sTableName to delete
     * \returns count of entries removed
     * \see dropGeoTable
     * \see mVectorLayers
     * \see mVectorLayersTypes
     * \see mDbLayers
     * \since QGIS 3.0
     */
    int removeGeoTable( QString sTableName );

    /**
     * Map of valid Selected Layers requested by the User
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

    /**
     * Map of tables and views that are not part of the Layers
     * - i.e. not Spatial-Tables or Spatial-Administration Tables
     * - containsTable-Name and Table-Type listed in mSpatialiteTypes
     * \see mSpatialiteTypes
     * \since QGIS 3.0
     */
    QMap<QString, QString> mNonSpatialTables;

    /**
     * Map of tables and views that are contained in the VectorLayers
     * - short list of the Layers retrieved from vector_layers
     *  - ordered by LayerType (SpatialTable, SpatialView and VirtualShape)
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> mVectorLayers;

    /**
     * Map of tables and views that are contained in the VectorLayers
     * - with the Layer-Type as Value
     *  - used to retrieve a specific Layer-Type contained in mVectorLayers
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: LayerType SpatialTable, SpatialView and VirtualShape
     * \see getDbLayersType
     * \since QGIS 3.0
     */
    QMap<QString, QString> mVectorLayersTypes;

    /**
     * Map of coverage_name that are contained in the RasterLite2 Vector-Layers
     * - short list of the Layers retrieved from raster_coverages
     *  - ordered by coverage_name
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMap<QString, QString> mVectorCoveragesLayers;

    /**
     * Map of coverage_name Extent information that are contained in the RasterLite2 Vector-Layers
     * - short list of the Layers retrieved from raster_coverages
     *  - ordered by coverage_name, native_srid DESC
     * \note
     * - There will allways be a 'native' srid/extent with the corresponding Wsg84 (4326) values
     * - There can be any amount of 'alternative' srid/extent with the corresponding Wsg84 (4326) values
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: native_srid€srid€;extent_minx€extent_miny€extent_maxx€extent_maxy€geo_minx€geo_miny€geo_maxx€geo_maxy
     * \see getDbVectorCoveragesLayersExtent
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMultiMap<QString, QString> mVectorCoveragesLayersExtent;

    /**
     * Map of coverage_name Extent information that are contained in the RasterLite2 Raster-Layers
     * - short list of the Layers retrieved from raster_coverages
     *  - ordered by coverage_name, native_srid DESC
     * \note
     * - There will allways be a 'native' srid/extent with the corresponding Wsg84 (4326) values
     * - There can be any amount of 'alternative' srid/extent with the corresponding Wsg84 (4326) values
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: native_srid€srid€;extent_minx€extent_miny€extent_maxx€extent_maxy€geo_minx€geo_miny€geo_maxx€geo_maxy
     * \see getDbVectorCoveragesLayersExtent
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMultiMap<QString, QString> mRasterCoveragesLayersExtent;

    /**
     * Map of coverage_name that are contained in the RasterLite2 Raster-Layers
     * - short list of the Layers retrieved from raster_coverages
     *  - ordered by coverage_name
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMap<QString, QString> mRasterCoveragesLayers;

    /**
     * Map of table_name that are contained in the RasterLite1Layers
     * - short list of the Layers retrieved from layer_statistics
     *  - Either:
     *  -> table_name with  raster_layer=1
     *  -> table_name_metadata with  raster_layer=0
     *  - Checks:
     *  -> TABLEs 'table_name'_metadata and 'table_name'_rasters must exist
     * \note
     * - Key: LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readRasterLite1Layers
     * \since QGIS 3.0
     */
    QMap<QString, QString> mRasterLite1Layers;

    /**
     * List of topolayer_name that are contained in the  'topologies' TABLE
     * \note
     * - Used to retrieve Export Layers
     * \see readTopologyLayers()
     * \since QGIS 3.0
     */
    QStringList mTopologyNames;

    /**
     * Map of topolayer_name that are contained in the Export-TopologyLayers
     * - Export Layers retrieved from topology_name of topologies
     *  -> and topology_name from 'topology_name'_topolayers TABLE
     * \note
     * - Key: LayerName formatted as 'topologies(topolayer_name)'
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see readTopologyLayers()
     * \since QGIS 3.0
     */
    QMap<QString, QString> mTopologyExportLayers;

    /**
     * Map of layers from metadata
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

    /**
     * Map of table_name that are contained in geopackage_contents
     *  -> this must be rendered be the QgsGdalProvider or QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name'
     * - Value: LayerType and Srid formatted as 'layer_type:srid'
     * -> layer_type: GeoPackageVector or GeoPackageRaster
     * - Checking is done if the 'GPKG' Gdal/Ogr Driver is active
     * \see readGeoPackageLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> mGeoPackageLayers;

    /**
     * Map of Layers that are contained in Gdal-FdoOgr
     *  -> this must be rendered be the  QgsOgrProvider
     * \note
     * - Key: LayerName formatted as 'table_name'
     * - Value: LayerType and Srid formatted as 'layer_type:srid'
     * -> layer_type:
     * - Checking is done if the SQLite' Ogr Driver is active
     * \see readFdoOgrLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> mFdoOgrLayers;

    /**
     * List of DataSourceUri of valid Layers
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

    /**
     * List of DataSourceInfo of valid Layers
     * -  contains GeometryType, Srid,  and ProviderKey
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
     * \see createDbLayerInfoUri
     * \see prepareDataSourceUris
     * \since QGIS 3.0
     */
    QMap<QString, QString> mDbLayersDataSourceInfo;

    /**
     * Map of style_name and XML-Document of VectorStyle
     * \note
     * - Key: style_id as retrieved from SE_vector_styled_layers_view
     * - Value: Style XML-Document
     * -> style_type: StyleVector
     * - SELECT style_name, XB_GetDocument(style,1) FROM SE_vector_styled_layers_view
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> mVectorStyleData;

    /**
     * Map of style_name and XML-Document of RasterStyle
     * \note
     * - Key: style_id as retrieved from SE_raster_styled_layers_view
     * - Value: Style XML-Document
     * -> style_type: StyleRaster
     * - SELECT style_name, XB_GetDocument(style,1) FROM SE_vector_styled_layers_view
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> mRasterStyleData;

    /**
     * Map of style_name and Metadata of VectorStyles
     * \note
     * Without a Spatialite connecton the Title, Abstract and Xml cannot be retrieved
     * - Key: style_id as retrieved from SE_vector_styled_layers_view
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleVector
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> mVectorStyleInfo;

    /**
     * Map of style_name and Metadata of RasterStyle
     * \note
     * Without a Spatialite connecton the Title, Abstract and Xml cannot be retrieved
     * - Key: style_id as retrieved from SE_raster_styled_layers
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleRaster
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> mRasterStyleInfo;

    /**
     * Map of LayerNames and style_id
     * \note
     * This information can be retrieved without a Spatialite connecton.
     * - Key: coverage_name from SE_vector_styled_layers or SE_raster_styled_layers
     * - Value: style_id
     * -> SpatialiteDbLayer will retrieve the style_id that belongs to it [there may be more than 1]
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMultiMap<QString, int> mLayerNamesStyles;

    /**
     * Test NamedLayer-Element
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

    /**
     * Retrieve Layers-Information of QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * Spatial* : QString( "%1,table=%2 (%3)" ).arg( getDatabaseFileName() ).arg( sTableName ).arg(sGeometryColumn);
     * - gathers all information from gaiaGetVectorLayersList
     * -> complementary information will be retrieved from SpatialiteGetLayerSettingsWrapper
     * -> RasterLite2 information will be retrieved from SpatialiteGetRasterLite2RasterLayersInfoWrapper
     * -> Topology information will be retrieved from SpatialiteGetRasterLite2RasterLayersInfoWrapper
     * -> RasterLite1 information will be retrieved from SpatialiteGetRasterLite2RasterLayersInfoWrapper
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)' [when empty: search for all]
     * \param bFoundLayerName store result if given LayerName was found
     * \returns true or false [if false and bFoundLayerName: then Layer is invalid]
     * \see getDbSpatialiteInfo
     * \see SpatialiteGetLayerSettingsWrapper
     * \see SpatialiteGetRasterLite2RasterLayersInfoWrapper
     * \see SpatialiteGetTopologyLayersInfoWrapper
     * \see SpatialiteGetRasterLite1LayersInfoWrapper
     * \since QGIS 3.0
     */
    bool GetDbLayersInfo( QString sLayerName, bool &bFoundLayerName );

    /**
     * Determine if valid SpatialTable/Views and VirtualShapes Layers exist
     * - called only when mHasVectorLayers > 0 during getSniffLayerMetadata
     * -
     * \note
     * - results are stored in mVectorLayers
     * - for RasterLite2 entries, the availablity of the Gdal-RasterLite2 driver will be checked
     * - there is no direct Spatialite driver support needed during this function
     * \returns true if the count of valid-layers > 0
     * \see getSniffLayerMetadata
     * \see mVectorLayers
     * \since QGIS 3.0
     */
    bool readVectorLayers();

    /**
     * Retrieve RasterLite2 Vector/Raster Layers-Information QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * - results are stored in mVectorLayers
     * - there is no direct Spatialite/RasterLite2 driver support needed during this function
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \see mVectorCoveragesLayers
     * \see mRasterCoveragesLayers
     * \since QGIS 3.0
     */
    bool readVectorRasterCoverages();

    /**
     * Retrieve RasterLite2 Vector/Raster Styles-Information
     * - used to fill list in SpatialiteDbLayer
     * \note
     * - As default: only load Styles that are used by the Layers [true]
     * - false=Retrieve all the Styles contained in the Database [may, one day, be needed]
     * - Spatialite/RasterLite2 driver support is needed to retreive the xml-documents
     * \param bTestStylesForQgis true=call testNamedLayerElement from getDbStyleNamedLayerElement to test if valid for Qgis
     * \returns mVectorStyleData.count() amount of VectorRaster-Styles loaded
     * \see GetDbLayersInfo
     * \see mVectorStyleData
     * \see mVectorStyleInfo
     * \since QGIS 3.0
     */
    int readVectorRasterStyles( bool bTestStylesForQgis = false );

    /**
     * Determine if valid RasterLite1 Layers exist
     * - called only when mHasRasterLite1Tables > 0 during getSniffLayerMetadata
     * \note
     * - results are stored in mRasterLite1Layers
     * - checking is done if the Gdal-Driver for RasterLite1 can be used
     * - there is no direct Spatialite driver support needed during this function
     * \returns true if the count of valid-layers > 0
     * \see getSniffLayerMetadata
     * \see mRasterLite1Layers
     * \since QGIS 3.0
     */
    bool readRasterLite1Layers();

    /**
     * Determine if valid Topology Layers exist
     * - called only when mHasTopologyExportTables > 0 during getSniffLayerMetadata
     * \note
     * - results are stored in mTopologyExportLayers
     * - there is no direct Spatialite driver support needed during this function
     * \returns true if the count of valid-layers > 0
     * \see getSniffLayerMetadata
     * \see mTopologyExportLayers
     * \since QGIS 3.0
     */
    bool readTopologyLayers();

    /**
     * Check the file contain the 'Magic Header String'
     * - When UTF-8 string "SQLite format 3" including the nul terminator character at the end.
     *  returns  MimeSqlite3 if a Sqlite3 Database
     * \note
     * - checks for File-Types defined MimeType
     * \param sDatabaseFileName filename to read
     * \returns MimeType of the given file
     * \see SpatialiteDbInfo
     * \since QGIS 3.0
     */
    static MimeType readMagicHeaderFromFile( QString sDatabaseFileName );

    /**
     * Create a new Database
     * - called from constructor with a supported option
     * \note
     * - supported containers: Spatialite, GeoPackage and MBTiles
     * \returns true if the created Database is valid and of the Container-Type requested
     * \see mDbCreateOption
     * \see createDatabaseSpatialite
     * \see createDatabaseGeoPackage
     * \see createDatabaseMBtiles
     * \since QGIS 3.0
     */
    bool createDatabase();

    /**
     * Create a new Spatialite Database
     * - called from constructor with a supported option
     * \note
     * - SpatialMetadata::Spatialite40: InitSpatialMetadata only
     * - SpatialMetadata::Spatialite45: also with Styles/Raster/VectorCoveragesTable's
     * \returns true if the Database was created and considered valid
     * \see mDbCreateOption
     * \see createDatabase
     * \since QGIS 3.0
     */
    bool createDatabaseSpatialite();

    /**
     * Create a new GeoPackage Database
     * - called from constructor with a supported option
     * \note
     * - SpatialMetadata::SpatialiteGpkg: using spatialite 'gpkgCreateBaseTables' function
     * \returns true if the Database was created
     * \see mDbCreateOption
     * \see createDatabase
     * \since QGIS 3.0
     */
    bool createDatabaseGeoPackage();

    /**
     * Create a new Spatialite Database
     * - called from constructor with a supported option
     * \note
     * - SpatialMetadata::SpatialiteMBTiles: creating a view-base MbTiles file with grid tables
     * \returns true if the Database was created
     * \see mDbCreateOption
     * \see createDatabase
     * \since QGIS 3.0
     */
    bool createDatabaseMBTiles();

    /**
     * Check if Database contains possible MbTiles-Tables
     * - sets mHasMBTilesTables
     * \note
     * \returns true mHasMBTilesTables=0
     * \since QGIS 3.0
     */
    bool checkMBTiles();

    /**
     * Determine if valid MbTiles Layers exist
     * - called only when mHasMBTilesTables > 0 during getSniffLayerMetadata
     * \note
     * - results are stored in mMBTilesLayers
     * - there is no direct Spatialite/RasterLite2 driver support needed during this function
     * \returns true if the count of valid-layers > 0
     * \see getSniffLayerMetadata
     * \see mMBTilesLayers
     * \since QGIS 3.0
     */
    bool readMBTilesLayers();

    /**
     * Determine if valid GeoPackage Layers exist
     * - called only when mHasGeoPackageTables > 0 during getSniffLayerMetadata
     * \note
     * - results are stored in mMBTilesLayers
     * - there is no direct Spatialite/RasterLite2 driver support needed during this function
     * \returns true if the count of valid-layers > 0
     * \see getSniffLayerMetadata
     * \see mGeoPackageLayers
     * \since QGIS 3.0
     */
    bool readGeoPackageLayers();

    /**
     * Determine if valid Gdal-FdoOgr Layers exist
     * - called only when mHasFdoOgrTables > 0 during getSniffLayerMetadata
     *  - FdoOgr  geometry_columns contains different fields as the Spatialite Version
     *  -> this must be rendered with QgsOgrProvider
     * \note
     * - results are stored in mFdoOgrLayers
     * - Checking is done if the SQLite' Gdal/Ogr Driver is active
     * - there is no direct Spatialite/RasterLite2 driver support needed during this function
     * \returns true if the count of valid-layers > 0
     * \see getSniffLayerMetadata
     * \see mFdoOgrLayers
     * \since QGIS 3.0
     */
    bool readFdoOgrLayers();

    /**
     * Retrieve SpatialTables/SpatialViews or VirtualTables-Layers-Information of QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * All Layers will be loaded if sLayerName is empty
     * \note
     * There is no direct Spatialite driver support needed during this function
     * - if specific Layer is requested, a Spatialite driver will be started
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetVectorLayersInfo( const QString sLayerName = QString::null );

    /**
     * Retrieve RasterLite2 Raster-Layers-Information of QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * All Layers will be loaded if sLayerName is empty
     * \note
     *  RasterLite2 [gdal]: QString( "%3:%1:%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "RASTERLITE2" )
     * - there is no direct RasterLite2 driver support needed during this function
     * - at present (2017-07-31) QGdalProvider cannot read RasterLite2 created with the development version
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetRasterLite2RasterLayersInfo( const QString sLayerName = QString::null );

    /**
     * Retrieve Topology Layers-Information of QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * All Layers will be loaded if sLayerName is empty
     * \note
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetTopologyLayersInfo( QString sLayerName = QString::null );

    /**
     * Retrieve RasterLite1 Layers-Information of QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * All Layers will be loaded if sLayerName is empty
     * \note
     *  RasterLite1: QString( "%3:%1,table=%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "RASTERLITE" );
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetRasterLite1LayersInfo( QString sLayerName = QString::null );

    /**
     * Retrieve MBTiles Layers-Information of QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * All Layers will be loaded if sLayerName is empty
     * \note
     *  MBTiles: QString( "%1" ).arg( getDatabaseFileName() )
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'name'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetMBTilesLayersInfo( QString sLayerName = QString::null );

    /**
     * Retrieve GeoPackage Layers-Information of QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * All Layers will be loaded if sLayerName is empty
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

    /**
     * Retrieve FdoOgr Layers-Information of QgsSqliteHandle connection
     * - used to fill list in  SpatialiteDbInfo
     * All Layers will be loaded if sLayerName is empty
     * \note
     *  GdalFdoOgr: QString( "%1|%3=%2" ).arg( getDatabaseFileName() ).arg( sTableName ).arg( "layername" );
     * \param dbConnectionInfo SpatialiteDbInfo
     * \param sLayerName Name of the Layer to search for format: 'table_name(geometry_name)'
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetFdoOgrLayersInfo( QString sLayerName = QString::null );

    /**
     * Retrieve and store Non-Spatial tables, views and all triggers
     * - used to fill list in  SpatialiteDbInfo
     * \note
     * \returns true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool readNonSpatialTables();

    /**
     * Sanity checks on tables
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

    /**
     * Builds Uri of layer
     * for use with QgsDataSourceUri
     * \see prepare
     * \since QGIS 3.0
     */
    bool prepareDataSourceUris();

    /**
     * Collection of warnings for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Warning text
     * \since QGIS 3.0
     */
    QMap<QString, QString> mWarnings;

    /**
     * Collection of reasons for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> mErrors;

    /**
     * Collection list of unique (lower case) Group Layers [for external use]
     * - with the amount of Layer for each Group
     * \note
     *  When: 'table_name' contains at least 2 '_' (such as 'berlin_streets_1650')
     *  - assume it belongs to a group of 'berlin_streets' for QgsSpatiaLiteTableModel
     *  - for SpatialViews only
     * \since QGIS 3.0
     */
    QMap<QString, int> mMapGroupNames;

    /**
     * Collection list of unique (lower case) Group Layers [for internal use]
     * \note
     *  When: 'table_name' contains at least 2 '_' (such as 'berlin_streets_1650')
     *  - assume it belongs to a group of 'berlin_streets' for QgsSpatiaLiteTableModel
     *  - for SpatialViews only
     * \since QGIS 3.0
     */
    QStringList mListGroupNames;

    /**
     * Builds list of unique Group Layers [lower case entries]
     * removes single entries
     * \see createDbLayerInfoUri
     * \since QGIS 3.0
     */
    int prepareGroupLayers();
};

/**
 * Structure to contain everything needed for a Spatialite/Rasterlite2 source
 * \returns point with all needed data
 * \since QGIS 3.0
 */
class CORE_EXPORT SpatialiteDbLayer : public QObject
{
    Q_OBJECT
  public:
    SpatialiteDbLayer( SpatialiteDbInfo *dbConnectionInfo );
    ~SpatialiteDbLayer();

    /**
     * The Database Info-Structure being read
     * \note
     *  - From the Database filename (with Path)
     * \see mDbSpatialiteInfo
    * \since QGIS 3.0
    */
    SpatialiteDbInfo *getDbSpatialiteInfo() const { return mDbSpatialiteInfo; }

    /**
     * The major Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    int dbSpatialiteVersionMajor() const { return getDbSpatialiteInfo()->dbSpatialiteVersionMajor(); }

    /**
     * The minor Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
     * \since QGIS 3.0
    */
    int dbSpatialiteVersionMinor() const { return getDbSpatialiteInfo()->dbSpatialiteVersionMinor(); }

    /**
     * The revision Spatialite Version being used
     * \note
     *  - extracted from spatialite_version()
     * \see getSniffDatabaseType
     * \since QGIS 3.0
    */
    int dbSpatialiteVersionRevision() const { return getDbSpatialiteInfo()->dbSpatialiteVersionRevision(); }

    /**
     * The sqlite handler
     * - contained in the QgsSqliteHandle class being used
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    sqlite3 *dbSqliteHandle() const { return getDbSpatialiteInfo()->dbSqliteHandle(); }

    /**
     * The Database filename (with Path)
    * \returns mDatabaseFileName complete Path (without without symbolic links)
    * \since QGIS 3.0
    */
    QString getDatabaseFileName() const { return mDatabaseFileName; }

    /**
     * The Database Uri of the Info-Structure being read
     * \note
     *  - used in setParentIdentifier of QgsLayerMetadata
     * \see mDbSpatialiteInfo
     * \see mLayerMetadata
    * \since QGIS 3.0
    */
    QString getDatabaseUri() const { return getDbSpatialiteInfo()->getDatabaseUri(); }

    /**
     * The Database Directory without file
    * \returns mDirectoryName  name of the complete  path,  (without without symbolic links), excluding the file-name
    * \since QGIS 3.0
    */
    QString getDatabaseDirectoryName() const { return getDbSpatialiteInfo()->getDirectoryName(); }

    /**
     * Name of the table/view with no schema
     * \note
     *  For SpatialViews
     *  - the regiestered TABLE can be retrieved with getViewTableName()
     * \see getViewTableName
     * \since QGIS 3.0
     */
    QString getTableName() const { return mTableName; }

    /**
     * Name of the geometry column in the table
     * \note
     *  Vector: should always be filled
     *  Raster: may be empty
     * \since QGIS 3.0
     */
    QString getGeometryColumn() const { return mGeometryColumn; }

    /**
     * Layer-Name
     * \note
     *  Vector: Layer format: 'table_name(geometry_name)'
     *  Raster: Layer format: 'coverage_name'
     * \since QGIS 3.0
     */
    QString getLayerName() const { return mLayerName; }

    /**
     * List-Group-Name (lower case)
     * \note
     *  When: 'table_name' contains at least 2 '_' (such as 'berlin_streets_1650')
     *  - assume it belongs to a group of 'berlin_streets' for QgsSpatiaLiteTableModel
     *  - for SpatialViews only
     * \since QGIS 3.0
     */
    QString getLayerGroupName() const
    {
      if ( getDbSpatialiteInfo()->getListGroupNames().contains( mListGroupName ) )
      {
        return mListGroupName;
      }
      return QString();
    }

    /**
     * Coverage-Name [internal]
     * \note
     *  retrieved from vector_coverages, which may be different from the Layer-Name
     * \since QGIS 3.0
     */
    QString getCoverageName() const { return mCoverageName; }

    /**
     * Name of the table which contains the SpatialView-Geometry (underlining table)
     * \note
     *  For SpatialTable/VirtualShapes: empty
     *  For SpatialViews:
     *  - the registered VIEW can be retrieved with getTableName()
     * \see mViewTableName
     * \since QGIS 3.0
     */
    QString getViewTableName() const { return mViewTableName; }

    /**
     * Name of the table-geometry which contains the SpatialView-Geometry (underlining table)
     * \note
     *  For SpatialTable/VirtualShapes: empty
     *  For SpatialViews:
     *  - the registered VIEW can be retrieved with getTableName()
     * \see mViewTableGeometryColumn
     * \since QGIS 3.0
     */
    QString getViewTableGeometryColumn() const { return mViewTableGeometryColumn; }

    /**
     * Name of the Layer format: 'table_name(geometry_name)'
     * \note
     *  For SpatialTable/VirtualShapes: empty
     *  For SpatialViews:
     *  - the registered VIEW can be retrieved with getTableName()
     * \see SpatialiteDbLayer::GetLayerSettings()
     * \since QGIS 3.0
     */
    QString getLayerViewTableName() const { return mLayerViewTableName; }

    /**
     * Title of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite since 4.5.0: vector_coverages(title)
     *  - RasterLite2: raster_coverages(title)
     *  - GeoPackage: gpkg_contents(identifier)
     *  - MbTiles: metadata(name)
     *  - FdoOgr: none
     * \see mTitle
     * \see readVectorRasterCoverages
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
     * \see mAbstract
     * \see readVectorRasterCoverages
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
     * \see mCopyright
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QString getCopyright() const { return mCopyright; }

    /**
     * License of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite 4.5.0: Column License id of 'vector_coverages', with name from 'data_licenses'
     *  - RasterLite2: Column License id of 'raster_coverages', with name from 'data_licenses'
     * -> From field 'name' of TABLE data_licenses ['PD - Public Domain', 'CC BY 3.0' etc.]
     *  - GeoPackage: none
     *  - MbTiles: none
     *  - FdoOgr: none
     * \see mLicense
     * \see readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QString getLicense() const { return mLicense; }

    /**
     * Srid of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite 4.0: vector_layers(srid) [geometry_columns(srid)]
     *  - RasterLite2: raster_coverages(srid)
     *  - GeoPackage: gpkg_contents(srs_id)
     *  - MbTiles: always 4326
     *  - FdoOgr: geometry_columns(srid)
     * \see mSrid
     * \since QGIS 3.0
     */
    int getSrid() const { return mSrid; }

    /**
     * The Srid of the Geometry or RasterLite2 Layer formatted as 'EPSG:nnnn'
     * \note
     *  - can be used to set the needed QgsCoordinateReferenceSystem
    * \since QGIS 3.0
    */
    QString getSridEpsg() const { return QString( "EPSG:%1" ).arg( mSrid ); }

    /**
     * UpdateLayerStatistics for the Layer
     * - calls SpatialiteDbInfo::UpdateLayerStatistics
     * \note
     *  - to call for all geometries of a Table, call SpatialiteDbInfo::UpdateLayerStatistics
     *  -- with the Table-Name only
     * \returns result of the internal Spatalite function update_layer_statistics
     * \see SpatialiteDbInfo::UpdateLayerStatistics
     * \since QGIS 3.0
     */
    bool UpdateLayerStatistics();

    /**
     * Set Srid, retrieving information for
     * Goal: to (semi) Automate unresolved setting when needed
     * \see mSrid
     * \see mAuthId
     * \see mProj4text
     * \since QGIS 3.0
     */
    bool setSrid( int iSrid );

    /**
     * The AuthId [auth_name||':'||auth_srid]
     *  \see setSrid
     * \since QGIS 3.0
     */
    QString getAuthId() const { return mAuthId; }

    /**
     * The Proj4text [from mSrid]
     *  \see setSrid
     * \since QGIS 3.0
     */
    QString getProj4text() const { return mProj4text; }

    /**
     * The SpatialIndex-Type used for the Geometry
     * \note
     *  Uses the same numbering as Spatialite [0,1,2]
     * \see setSpatialIndexType
     * \see mSpatialIndexTypeString
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialIndexType getSpatialIndexType() const { return mSpatialIndexType; }

    /**
     * The SpatialIndex-Type used for the Geometry as String
     *  Uses the same numbering as Spatialite [0,1,2]
     * \see setSpatialIndexType
     * \see mSpatialIndexType
     * \since QGIS 3.0
     */
    QString getSpatialIndexString() const { return mSpatialIndexTypeString; }

    /**
     * The Spatialite Layer-Type of the Layer
     * - representing mLayerType
     *  \see setGeometryType
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialiteLayerType getLayerType() const { return mLayerType; }

    /**
     * Set the Spatialite Layer-Type String
     * - representing mLayerType
     *  \see setLayerType
     * \see SpatialiteLayerTypeName
     * \since QGIS 3.0
     */
    QString getLayerTypeString() const { return mLayerTypeString; }

    /**
     * Set the Layer-InfoString
     * \note
     *  - field 0=Geometry-Type [for Raster-Types: LayerType]
     *  - field 1=Srid of the Layer
     *  - field 2=The Provider Name
     * \see prepare
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString getLayerInfo() const { return mLayerInfo; }

    /**
     * Contains collected Metadata for the Layer
     * \brief A structured metadata store for a map layer.
     * \note
     *  - SpatialiteDbLayer will use a copy of SpatialiteDbInfo  mLayerMetadata as starting point
     * \see QgsMapLayer::htmlMetadata()
     * \see QgsMapLayer::metadata
     * \see QgsMapLayer::setMetadata
     * \see QgsSpatiaLiteProvider::setDbLayer
     * \see QgsRasterLite2Provider::setDbLayer
     * \since QGIS 3.0
    */
    QgsLayerMetadata getLayerMetadata() const { return mLayerMetadata; }

    /**
     * Set the  Layer-DataSourceUri String
     * - Provider dependent
     * \see prepare
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString getLayerDataSourceUri() const { return mDataSourceUri; }

    /**
     * Is the Layer supported by QgsSpatiaLiteProvider
     * - QgsSpatiaLiteProvider should never accept a Layer when this is not true
     * \note
     *  - check for valid Layer in the provider
     * \see setLayerType
     * \see QgsSpatiaLiteProvider::setDbLayer
     * \since QGIS 3.0
     */
    bool isLayerSpatialite() const { return mIsSpatialite; }

    /**
     * Is the Layer supported by QgsRasterLite2Provider
     * - QgsSpatiaLiteProvider should never accept a Layer when this is not true
     * \note
     *  - check for valid Layer in the provider
     * \see setLayerType
     * \see QgsSpatiaLiteProvider::setDbLayer
     * \since QGIS 3.0
     */
    bool isLayerRasterLite2() const { return mIsRasterLite2; }

    /**
     * Returns QIcon representation of the enum of Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteLayerTypeNameIcon
     * \since QGIS 3.0
     */
    QIcon getLayerTypeIcon() const { return SpatialiteDbInfo::SpatialiteLayerTypeIcon( mLayerType ); }

    /**
     * The Spatialite Geometry-Type of the Layer
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see setGeometryType
     * \since QGIS 3.0
     */
    QgsWkbTypes::Type getGeometryType() const { return mGeometryType; }

    /**
     * Set the Spatialite Geometry-Type String
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  - QgsWkbTypes::parseType(mGeometryTypeString)
     *  \see setGeometryType
     * \since QGIS 3.0
     */
    QString getGeometryTypeString() const { return mGeometryTypeString; }

    /**
     * Return the QgsLayerItem Icon for the Spatialite Geometry-Type
     * \since QGIS 3.0
     */
    QIcon getGeometryTypeIcon() const { return SpatialiteDbInfo::SpatialGeometryTypeIcon( mGeometryType ); }

    /**
     * The Spatialite Coord-Dimensions of the Layer
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    int getCoordDimensions() const { return mCoordDimensions; }

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     * \note
     *  With UpdateLayerStatistics the Number of features will also be updated and retrieved
     * \param bUpdateExtent force reading from Database [Extent, NumFeatures]
     * \param bUpdateStatistics UpdateLayerStatistics before reading Extent and NumFeatures
     * \see mLayerExtent
     * \see mNumberFeatures
     * \since QGIS 3.0
     */
    QgsRectangle getLayerExtent( bool bUpdateExtent = false, bool bUpdateStatistics = false );

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     * - based on the Wsg84 value of the Layer found in vector_coverages
     * \note
     *  - part of the vector_coverages logic
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QgsRectangle getLayerExtentWsg84() const { return mLayerExtentWsg84; }

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     * \note
     *  With UpdateLayerStatistics the Number of features will also be updated and retrieved
     * \param bUpdateExtent force reading from Database [Extent, NumFeatures]
     * \param bUpdateStatistics UpdateLayerStatistics before reading Extent and NumFeatures
     * \see mLayerExtent
     * \see mNumberFeatures
     * \since QGIS 3.0
     */
    QgsBox3d getLayerExtent3d() const { return mLayerExtent3d; }

    /**
     * RasterLite2 Raster-Layer Image Extent [QgsRasterLayer::width(), height() ]
     * \note
     *  - based on LayerExtent and Resolution from raster_coverage table
     * \see QgsRasterLayer::width
     * \see QgsRasterLayer::height
     * \since QGIS 3.0
     */
    QPoint getLayerImageExtent() const { return mLayerImageExtent; }

    /**
     * RasterLite2 Raster-Layer Image Width [QgsRasterInterface::xSize() ]
     * \note
     *  - based on LayerExtent and Resolution from raster_coverage table
     * \see QgsRasterInterface::xSize
     * \since QGIS 3.0
     */
    int getLayerImageWidth() const { return mLayerImageExtent.x(); }

    /**
     * RasterLite2 Raster-Layer Image Height [QgsRasterInterface::ySize() ]
     * \note
     *  - based on LayerExtent and Resolution from raster_coverage table
     * \see QgsRasterInterface::ySize
     * \since QGIS 3.0
     */
    int getLayerImageHeight() const { return mLayerImageExtent.y(); }

    /**
     * RasterLite2 Raster-Layer Extent X (Width, Horizontal)
     *  - result in Map-Units
     * \note
     *  - retrieved from based on extent_maxx minus extent_minx from raster_coverage table
     * \since QGIS 3.0
     */
    double getLayerExtentWidth() const { return mLayerExtent.width(); }

    /**
     * RasterLite2 Raster-Layer Extent Y (Height, Vertical)
     *  - result in Map-Units
     * \note
     *  - retrieved from based on extent_maxy minus extent_miny from raster_coverage table
     * \since QGIS 3.0
     */
    double getLayerExtentHeight() const { return mLayerExtent.height(); }

    /**
     * RasterLite2 Raster-Layer Image X (Width, Vertical) Resolution
     * \note
     *  - retrieved from horz_resolution from raster_coverage table
     * \since QGIS 3.0
     */
    double getLayerImageResolutionX() const { return mLayerResolution.x(); }

    /**
     * RasterLite2 Raster-Layer Image Y (Height, Vertical) Resolution
     * \note
     *  - retrieved from vert_resolution from raster_coverage table
     * \since QGIS 3.0
     */
    double getLayerImageResolutionY() const { return mLayerResolution.y(); }

    /**
     * RasterLite2 Raster-Layer Bands [QgsRasterLayer::bandCount]
     *  - based on // num_bands, tile_width, tile_height entries from raster_coverage table
     * \note
     *  - num_bands = mLayerBandsTileSize.x()
     *  - mLayerBandsTileSize.y(): not used (yet)
     *  - tile_width = mLayerBandsTileSize.width()
     *  - tile_height = mLayerBandsTileSize.height()
     * \see QgsRasterLayer::bandCount
     * \since QGIS 3.0
     */
    int getLayerNumBands() const { return mLayerBandsTileSize.x(); }

    /**
     * QRectangle contains the Pixel size of a Raster-Layer Image
     * - based on the values found in the raster_coverages table
     * \note
     *  - count NoData =mLayerPixelSizes.x()
     *  - count ValidPixels = mLayerPixelSizes.y()
     *  - count NoData+ValidPixels = mLayerPixelSizes.width()
     *  - diff = Calculation based on extent and resolution may differ =  = mLayerPixelSizes.heght()
     *  \see setLayerBandsInfo
     * \since QGIS 3.0
     */
    int getLayerCountImageNodataPixels() const { return mLayerPixelSizes.x(); }

    /**
     * QRectangle contains the Pixel size of a Raster-Layer Image
     * - based on the values found in the raster_coverages table
     * \note
     *  - count NoData =mLayerPixelSizes.x()
     *  - count ValidPixels = mLayerPixelSizes.y()
     *  - count NoData+ValidPixels = mLayerPixelSizes.width()
     *  - diff = Calculation based on extent and resolution may differ =  = mLayerPixelSizes.heght()
     *  \see setLayerBandsInfo
     * \since QGIS 3.0
     */
    int getLayerCountImageValidPixels() const { return mLayerPixelSizes.y(); }

    /**
     * QRectangle contains the Pixel size of a Raster-Layer Image
     * - based on the values found in the raster_coverages table
     * \note
     *  - count NoData =mLayerPixelSizes.x()
     *  - count ValidPixels = mLayerPixelSizes.y()
     *  - count NoData+ValidPixels = mLayerPixelSizes.width()
     *  - diff = Calculation based on extent and resolution may differ =  = mLayerPixelSizes.heght()
     *  \see setLayerBandsInfo
     * \since QGIS 3.0
     */
    int getLayerCountImagePixels() const { return mLayerPixelSizes.width(); }

    /**
     * RasterLite2 Raster-Layer tile width [QgsRasterInterface::xBlockSize]
     *  - based on // num_bands, tile_width, tile_height entries from raster_coverage table
     * \note
     *  - num_bands = mLayerBandsTileSize.x()
     *  - mLayerBandsTileSize.y(): not used (yet)
     *  - tile_width = mLayerBandsTileSize.width()
     *  - tile_height = mLayerBandsTileSize.height()
     * \see QgsRasterInterface::xBlockSize
     * \since QGIS 3.0
     */
    int getLayerTileWidth() const { return mLayerBandsTileSize.width(); }

    /**
     * RasterLite2 Raster-Layer tile height [QgsRasterInterface::::yBlockSize]
     *  - based on num_bands, tile_width, tile_height entries from raster_coverage table
     * \note
     *  - num_bands = mLayerBandsTileSize.x()
     *  - mLayerBandsTileSize.y(): not used (yet)
     *  - tile_width = mLayerBandsTileSize.width()
     *  - tile_height = mLayerBandsTileSize.height()
     * \see QgsRasterInterface::yBlockSize
     * \since QGIS 3.0
     */
    int getLayerTileHeight() const { return mLayerBandsTileSize.height(); }

    /**
     * RasterLite2 Sample-Type
     *  - based on sample_type entry from raster_coverage table
     * \note
     *  Unsigned Integer
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  - Byte,Byte,Byte,Byte,UInt16,UInt32
     *  Signed Integer
     *  - INT8, INT16, INT32
     *  - Byte, Int16, Int32
     *  Floating Point
     *  - FLOAT, DOUBLE
     *  - Float32, Float64
     * \see QgsRasterLayer::htmlMetadata
     * \since QGIS 3.0
     */
    QString getLayerRasterSampleType() const { return mLayerSampleType; }

    /**
     * RasterLite2 Sample-Type as Qgis::DataType
     *  - based on sample_type entry from raster_coverage table
     * \note
     *  Unsigned Integer
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  - Byte,Byte,Byte,Byte,UInt16,UInt32
     *  Signed Integer
     *  - INT8, INT16, INT32
     *  - Byte, Int16, Int32
     *  Floating Point
     *  - FLOAT, DOUBLE
     *  - Float32, Float64
     * \see mLayerRasterDataType
     * \see QgsRasterInterface::dataType
     * \see QgsRasterInterface::sourceDataType
     * \since QGIS 3.0
     */
    Qgis::DataType getLayerRasterDataType() const { return mLayerRasterDataType; }

    /**
     * RasterLite2 Sample-Type as Qgis String
     *  - based on sample_type entry from raster_coverage table
     * \note
     *  Unsigned Integer
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  - Byte,Byte,Byte,Byte,UInt16,UInt32
     *  Signed Integer
     *  - INT8, INT16, INT32
     *  - Byte, Int16, Int32
     *  Floating Point
     *  - FLOAT, DOUBLE
     *  - Float32, Float64
     * \see QgsRasterLayer::htmlMetadata
     * \since QGIS 3.0
     */
    QString getLayerRasterDataTypeString() const { return mLayerRasterDataTypeString; }

    /**
     * RasterLite2 Pixel-Type as Integer
     *  - based on pixel_typeentry from raster_coverage table
     * \note
     *  MONOCHROME
      *  - RasterLite2 constant: Pixel Type Monochrome - Bilevel [0x11 = 17 dec]
     *  - Bands=1
     *  - 1-BIT
     *  PALETTE
     *  - RasterLite2 constant: Pixel Type Palette based [0x12 = 18 dec]
     *  - Bands=1
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8
     *  GRAYSCALE
     *  - RasterLite2 constant: Pixel Type Grayscale [0x13 = 19 dec]
     *  - Bands=1
     *  - 2-BIT, 4-BIT, UINT8
     *  RGB
     *  - RasterLite2 constant: Pixel Type Red-Green-Blue [0x14 = 20 dec]
     *  - Bands=3
     *  - UINT8, UINT16
     *  MULTIBAND
     *  - RasterLite2 constant: Pixel Type Multiband (arbitrary) [0x15 = 21 dec]
     *  - Bands= > 1 < 256
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  DATAGRID
     *  - RasterLite2 constant: Pixel Type Data-Grid [0x16 = 22 dec]
     *  - Bands=1
     *  - UINT8, UINT16, UINT32, INT8, INT16, INT32, FLOAT, DOUBLE
     * \see QgsRasterInterface::generateBandName
     * \since QGIS 3.0
    */
    int getLayerRasterPixelType() const { return mLayerPixelType; }

    /**
     * RasterLite2 Pixel-Type as String
     *  - based on pixel_typeentry from raster_coverage table
     * \note
     *  MONOCHROME
      *  - RasterLite2 constant: Pixel Type Monochrome - Bilevel [0x11 = 17 dec]
     *  - Bands=1
     *  - 1-BIT
     *  PALETTE
     *  - RasterLite2 constant: Pixel Type Palette based [0x12 = 18 dec]
     *  - Bands=1
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8
     *  GRAYSCALE
     *  - RasterLite2 constant: Pixel Type Grayscale [0x13 = 19 dec]
     *  - Bands=1
     *  - 2-BIT, 4-BIT, UINT8
     *  RGB
     *  - RasterLite2 constant: Pixel Type Red-Green-Blue [0x14 = 20 dec]
     *  - Bands=3
     *  - UINT8, UINT16
     *  MULTIBAND
     *  - RasterLite2 constant: Pixel Type Multiband (arbitrary) [0x15 = 21 dec]
     *  - Bands= > 1 < 256
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  DATAGRID
     *  - RasterLite2 constant: Pixel Type Data-Grid [0x16 = 23 dec]
     *  - Bands=1
     *  - UINT8, UINT16, UINT32, INT8, INT16, INT32, FLOAT, DOUBLE
     * \see QgsRasterInterface::generateBandName
     * \since QGIS 3.0
    */
    QString getLayerRasterPixelTypeString() const { return mLayerPixelTypeString; }

    /**
     * RasterLite2 Compression-Type
     *  - based on compression entry from raster_coverage table
     * \note
     *  NONE
     *  - lossless
     *  DEFLATE
     *  - lossless
     *  DEFLATE_NO
     *  - lossless
     *  LZMA
     *  - lossless
     *  LZMA_NO
     *  - lossless
     *  PNG
     *  - lossless
     *  JPEG
     *  - lossy
     *  WEBP
     *  - lossy
     *  LL_WEBP
     *  - lossless
     *  CHARLS
     *  - lossy
     *  JP2
     *  - lossy
     *  LL_JP2
     *  - lossless
     * \since QGIS 3.0
     */
    QString getLayerRasterCompressionType() const { return mLayerCompressionType; }

    /**
     * Set the Rectangle that contains the extent (bounding box) of the layer
     *  - will also set the EWKT
     * \note
     *  The Srid must be set beforhand for the correct result
     * \param layerExtent Layer Extent [Vectors and Rasters]
     * \param layerResolution Layer Resolution [Rasters only]
     * \see mLayerExtent
     * \see mLayerExtentEWKT
     * \since QGIS 3.0
     */
    QString setLayerExtent( QgsRectangle layerExtent, QgsPointXY layerResolution = QgsPointXY( 0.0, 0.0 ) );

    /**
     * Map of coverage_name Extent information that are contained in the RasterLite2 Vector-Layers
     * - short list of the Layers retrieved from raster_coverages
     *  - ordered by coverage_name, native_srid DESC
     * \note
     * - There will allways be a 'native' srid/extent with the corresponding Wsg84 (4326) values
     * - There can be any amount of 'alternative' srid/extent with the corresponding Wsg84 (4326) values
     * - Contains the value contained in mVectorCoveragesLayersExtent
     * - Value: native_srid€srid€;extent_minx€extent_miny€extent_maxx€extent_maxy€geo_minx€geo_miny€geo_maxx€geo_maxy
     * \see SpatialiteDbInfo::getDbVectorCoveragesLayersExtent
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    QMap<int, QgsBox3d> mLayerExtents;

    /**
     * Set the Layer Extents for (possible) Vector/Raster-Coverages
     *  - will also set the EWKT
     * \note
     * - There will allways be a 'native' srid/extent with the corresponding Wsg84 (4326) values
     * - There can be any amount of 'alternative' srid/extent with the corresponding Wsg84 (4326) values
     * - Contains the value contained in mVectorCoveragesLayersExtent
     * - Value: native_srid€srid€;extent_minx€extent_miny€extent_maxx€extent_maxy€geo_minx€geo_miny€geo_maxx€geo_maxy
     * \see prepare()
     * \see SpatialiteDbInfo::getDbVectorCoveragesLayersExtent
     * \see SpatialiteDbInfo::readVectorRasterCoverages
     * \since QGIS 3.0
     */
    int setLayerExtents( QList<QString> layerExtents );

    /**
     * Set the Rectangle that contains the extent (bounding box) of the layer
     *  - num_bands, tile_width, tile_height,sample_type, pixel_type, compression from  raster_coverage table
     * \note
     *  The RasterLite2 SampleType will be 'translated' to the Qgis Data-Type (with it's String representation)
     * \param layerBandsTileSize num_bands and tile_width and tile_height
     * \param sLayerSampleType data sample type of Raster
     * \param sLayerPixelType data pixel type of Raster
     * \param sLayerCompressionType compressiontype of Raster
     * \see mLayerBandsTileSize
     * \see mLayerSampleType
     * \see mLayerRasterDataType
     * \see mLayerRasterDataTypeString
     * \since QGIS 3.0
     */
    bool setLayerRasterTypesInfo( QRect layerBandsTileSize, QString sLayerSampleType, QString  sLayerPixelType, QString  sLayerCompressionType );

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
     * \since QGIS 3.0
     */
    void setLayerBandsInfo( QStringList layerBandsInfo, QMap<int, QImage> layerBandsHistograms );

    /**
     * Retrieve list for each Band in a RasterLite2 Raster
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
     * \see SpatialiteDbLayer::getLayerGetBandStatistics
     * \since QGIS 3.0
     */
    QStringList getLayerBandsInfo() const { return mLayerBandsInfo; }

    /**
     * Create list for each Band in a RasterLite2 Raster
     * - this should be called by a Provider when needed
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
    QStringList getLayerGetBandStatistics();

    /**
     * For each Band in a RasterLite2 Raster
     * - a PNG image representing the estimated distribution Histogram from a specific Band
     * \note
     *  - RL2_GetBandStatistics_Histogram
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, QImage> getLayerBandsHistograms() const { return mLayerBandsHistograms; }

    /**
     * When getMapImageFromRasterLite2 is called during readBlock
     * - a default Background must be given
     * - The default is '#ffffff'
     * \note
     *  If Nodata exist, those values will be used to maximal: '#rrggbb'
     * - maximal 3 bands, when less: will be filled with the last value
     *  QgsRasterLite2Provider will override this value with the canvas background
     * \see setLayerBandsInfo
     * \see QgsRasterLite2Provider::readBlock
     * \since QGIS 3.0
     */
    QString getLayerDefaultImageBackground() const { return mDefaultImageBackground; }

    /**
     * Set the Wsg84 Rectangle that contains the extent (bounding box) of the layer
     *  - will also set the EWKT
     * \note
     *  This is a part of the vector_coverages logic
     * \see mLayerExtentWsg84
     * \see mLayerExtentWsg84EWKT
     * \since QGIS 3.0
     */
    QString setLayerExtentWsg84( QgsRectangle layerExtent );

    /**
     * Rectangle that contains the extent (bounding box) of the layer, with Srid
     * \note
     * \since QGIS 3.0
     */
    QString getLayerExtentEWKT() const { return mLayerExtentEWKT; }

    /**
     * Rectangle that contains the extent (bounding box) of the layer, with Srid
     * \note
     * \since QGIS 3.0
     */
    QString getLayerExtentWsg84EWKT() const { return mLayerExtentWsg84EWKT; }

    /**
     * The selected Style as Id
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    int getLayerStyleSelectedId() const { return mLayerStyleSelected; }

    /**
     * The selected Style as Name
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getLayerStyleSelected() const { return mLayerStyleSelectedString; }

    /**
     * The selected Style
     * - by default: the first entry of mStyleId value
     * \note
     *  - will set the StyleType, Title and Abstract
     *  \see setLayerStyleSelected
     *  \see  getLayerStyleSelectedType
     *  \see  getLayerStyleSelectedTitle
     *  \see  getLayerStyleSelectedAbstract
     * \since QGIS 3.0
     */
    int setLayerStyleSelected( int iStyleId = -1 );

    /**
     * Retrieve the selected Style as Xml-String
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only convenience function for outside use
     *  - getLayerStyleNamedLayerElement  will be used in most cases
     * \param iStyleId to retrieve
     * \returns xml QString of Xml-Document
     * \see setLayerStyleSelected
     * \see SpatialiteDbInfo::getDbStyleXml
     * \see getLayerStyleNamedLayerElement
     * \since QGIS 3.0
     */
    QString getLayerStyleXml( int iStyleId = -1 );

    /**
     * List of style_id of Styles belong to this Layer
     * \note
     * This information can be retrieved without a Spatialite connecton.
     * - Key: coverage_name from SE_vector_styled_layers or SE_raster_styled_layers
     * - Value: style_id
     * -> SpatialiteDbLayer will retrieve the style_id that belongs to it [there may be more than 1]
     * \see SpatialiteDbInfo::readVectorRasterStyles
     * \see checkLayerStyles
     * \since QGIS 3.0
     */
    QList< int> mStylesId;

    /**
     * Retrieve the selected Style
     * - by default: the value of mLayerStyleSelected will be used
     * - otherwise: the first entry of mStylesId will be used if set
     * \note
     *  - return QDomElement to be used with QgsVectorLayer::readSld
     *   - called while loading Layers
     *   - checking is done if the given style_id exists
     *   -> contained in list of registered stled_id in mStylesId
     * \param iStyleId to retrieve
     * \param errorMessage for messages returning any caus of error
     * \returns xlm QDomElement containing 'NamedLayer' stored in SpatialiteDbInfo
     * \see setLayerStyleSelected
     * \see SpatialiteDbInfo::getDbStyleNamedLayerElement
     * \see SpatialiteDbInfo::
     * \since QGIS 3.0
     */
    QDomElement getLayerStyleNamedLayerElement( int iStyleId = -1, QString errorMessage = QString::null );

    /**
     * Does this Layer contain a Style
     * \note
     *  -  which can be retrieved with getLayerStyle
     *  -  mStylesId.cont() > 0
     * \see mLayerStyleSelected
     * \see getLayerStyle
     * \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    bool hasLayerStyle() const { return mHasStyle; }

    /**
     * The selected Style: Name
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - StyleVector or StyleRaster
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getLayerStyleSelectedName() const { return mLayerStyleSelectedName; }

    /**
     * The selected Style: Title
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getLayerStyleSelectedTitle() const { return mLayerStyleSelectedTitle; }

    /**
     * The selected Style: Abstract
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString getLayerStyleSelectedAbstract() const { return mLayerStyleSelectedAbstract; }

    /**
     * Number of features in the layer
     * \note
     *  With UpdateLayerStatistics the Extent will also be updated and retrieved
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see mLayerExtent
     * \see mNumberFeatures
     * \since QGIS 3.0
     */
    long getNumberFeatures( bool bUpdateStatistics = false );

    /**
     * The Spatialite Layer-Readonly status [true or false]
     * \note
     *  SpatialTable: always false [writable]
     *  SpatialView: default: true [readonly]
     *  -  if Insert/Update or Delete TRIGGERs exists:  false [writable]
     *  VirtualShape: always true [readonly]
     * \see GetViewTriggers()
     * \since QGIS 3.0
     */
    int isLayerReadOnly() const { return mLayerReadOnly; }

    /**
     * The Spatialite Layer-Hidden status [true or false]
     * \note
     *  No idea what this is used for [at the moment not being retrieved]
     *  - from 'vector_layers_auth'
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int isLayerHidden() const { return mLayerIsHidden; }

    /**
     * The Spatialite Layer-Id being created
     * \note
     *  Only a simple counter when inserting into mDbLayers
     * - value is never used
     * \see SpatialiteDbInfo::mDbLayers
     * \since QGIS 3.0
     */
    int getLayerId() const { return mLayerId; }

    /**
     * Based on Layer-Type, store Style-Names
     * - SE_vector_styled_layers_view / SE_raster_styled_layers_view
     * \note
     * - VectorStyle: collects names of installed Vector-Styles
     * - RasterStyle: collects names of installed Raster-Styles
     * - there is no direct Spatialite/RasterLite2 driver support needed during this function
     * \param bUpdate force reading from Database
     * \ mVectorStyleInfo
     * \see prepare
     * \since QGIS 3.0
     */
    bool checkLayerStyles();

    /**
     * Map of style_name and Metadata
     * \note
     * - Key: StyleIdas retrieved from SE_vector_styled_layers_view / SE_raster_styled_layers_view
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleVector,StyleRaster
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> getLayerCoverageStylesInfo() const { return mLayerStyleInfo; }

    /**
     * Based on Layer-Type, set QgsVectorDataProvider::Capabilities
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

    /**
     * Name of the table or subquery
     * \note
     *  - Not used, since a Layer could be used in multiple instances of a  QgsSpatiaLiteProvider
     *  -> QgsSpatiaLiteProvider contains ia seperate version of mQuery , since the results (mQuery, mSubsetString) could effect Layers that are being used elsewhere
     * \see QgsSpatiaLiteProvider:.mQuery
     * \since QGIS 3.0
     */
    QString getLayerQuery() const { return mQuery; }

    /**
     * Set of subquery [should not be used]
     * \note
     *  - Not used, since a Layer could be used in multiple instances of a  QgsSpatiaLiteProvider
     *  -> QgsSpatiaLiteProvider contains ia seperate version of mQuery , since the results (mQuery, mSubsetString) could effect Layers that are being used elsewhere
     * \see QgsSpatiaLiteProvider:.mQuery
     * \since QGIS 3.0
     */
    void setLayerQuery( QString sQuery ) { mQuery = sQuery; }

    /**
     * Name of the primary key column in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QString getPrimaryKey() const { return mPrimaryKey; }

    /**
     * Column-Number of the primary key
     * \note
     *  - SpatialView: name from entry of view_rowid of views_geometry_columns
     *  -> position from inside the PRAGMA table_info('name')
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    int getPrimaryKeyCId() const { return mPrimaryKeyCId; }

    /**
     * List of primary key columns in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QgsAttributeList getPrimaryKeyAttrs() const { return mPrimaryKeyAttrs; }

    /**
     * List of layer fields in the table
     * \note
     *  - from PRAGMA table_info()
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QgsFields getAttributeFields() const { return mAttributeFields; }
    //!

    /**
     * Retrieve a specific of layer fields of the table
     * \note
     *  - from PRAGMA table_info()
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QgsField getAttributeField( int index ) const;

    /**
     * Map of field index to default value [for Topology, the Topology-Layers]
     * List of default values of the table and possible the view
     * \note
     *  - from PRAGMA table_info()
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QMap<int, QVariant> getDefaultValues() const { return mDefaultValues; }

    /**
     * Collection of warnings for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> getWarnings() const { return mWarnings; }

    /**
     * Collection of reasons for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> getErrors() const { return mErrors; }

    /**
     * Is the Layer valid
     * \note
     *  when false: the Layer should not be rendered
     * \since QGIS 3.0
     */
    bool isLayerValid() const { return mIsLayerValid; }

    /**
     * Resolve unset settings in Layer
     * Goal: to (semi) Automate unresolved settings when needed
     * - will be called after major information has been retrieved from the Database
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \see prepareCapabilities
     * \see checkLayerStyles
     * \returns mIsLayerValid if the Layer is considered valid
     * \since QGIS 3.0
     */
    bool prepare();

    /**
     * Adds features
     *  - implementation of Provider function 'addFeatures'
     * \note
     *  - updating internal layer-data, such as NumberFeatures
     *  - Spatialite specific [GeoPackage could be implemented, but not forseen]
     * \param flist feature to add
     * \returns True in case of success and False in case of error or container not supported
     * \see getNumberFeatures
     * \see QgsSpatiaLiteProvider::addFeatures
     * \since QGIS 3.0
     */
    bool addLayerFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags, QString &errorMessage );

    /**
     * Deletes features
     *  - implementation of Provider function 'deleteFeatures'
     * \note
     *  - updating internal layer-data, such as NumberFeatures
     * \param id QgsFeatureIds to delete
     * \returns True in case of success and False in case of error or container not supported
     * \see getNumberFeatures
     * \see QgsSpatiaLiteProvider::deleteFeatures
     * \since QGIS 3.0
     */
    bool deleteLayerFeatures( const QgsFeatureIds &id, QString &errorMessage );

    /**
     * Deletes all records of Layer-Table
     *  - implementation of Provider function 'truncate'
     * \note
     *  - updating internal layer-data, such as NumberFeatures and Extent
     * \param id QgsFeatureIds to delete
     * \returns True in case of success and False in case of error or container not supported
     * \see getNumberFeatures
     * \see QgsSpatiaLiteProvider::truncate
     * \since QGIS 3.0
     */
    bool truncateLayerTableRows();

    /**
     * Updates features
     *  - implementation of Provider function 'changeGeometryValues'
     * \note
     *  - Spatialite specific [GeoPackage could be implemented, but not forseen]
     * \param geometry_map collection of geometries to change
     * \returns True in case of success and False in case of error
     * \see QgsSpatiaLiteProvider::changeGeometryValues
     * \since QGIS 3.0
     */
    bool changeLayerGeometryValues( const QgsGeometryMap &geometry_map );

    /**
     * Adds attributes
     *  - implementation of Provider function 'addAttributes'
     * \note
     *  - Spatialite specific [GeoPackage could be implemented, but not forseen]
     * \param attributes List of QgsField
     *  \returns True in case of success and False in case of error
     * \see QgsSpatiaLiteProvider::addAttributes
     * \since QGIS 3.0
     */
    bool addLayerAttributes( const QList<QgsField> &attributes );

    /**
     * Updates attributes
     *  - implementation of Provider function 'changeAttributeValues'
     * \note
     *  - Spatialite specific [GeoPackage could be implemented, but not forseen]
     * \param attr_map collection of attributes to change
     * \returns True in case of success and False in case of error
     * \see QgsSpatiaLiteProvider::changeAttributeValues
     * \since QGIS 3.0
     */
    bool changeLayerAttributeValues( const QgsChangedAttributesMap &attr_map );

    /**
     * Creates attributes Index
     *  - implementation of Provider function 'createAttributeIndex'
     * \note
     *  - Possibly Spatialite specific [GeoPackage could be implemented, but not forseen]
     * \param field number of attribute to created the index for
     * \returns True in case of success and False in case of error
     * \see QgsSpatiaLiteProvider::changeAttributeValues
     * \since QGIS 3.0
     */
    bool createLayerAttributeIndex( int field );

    /**
     * Retrieve rasterlite2 image of a given bound and size.
     * - https://www.gaia-gis.it/fossil/librasterlite2/wiki?name=sql_reference_list
     *  look for: RL2_GetMapImageFromRaster
     * Will return a image that is stored using a 32-bit ARGB format (0xAARRGGBB)
     * This is a convenience function an not used by the QgsRasterLite2Provider
     * \note
     * The RasterLite2 connection will be made, if needed.
     * The viewExtents will use the srid of the layer
     * 'data' expects raw Image-Data, which be extracted from the QImage returned by rl2GetMapImageFromRaster
     * - only mine-types supported by QImage may be used
     * \param width        of image in pixel.
     * \param height       of image in pixel.
     * \param viewExtents OUT::  [west,south,east,north] [minx, miny, maxx, maxy] bounds.
     * \param styleName   default: default', when has not been registered with a RasterStyle
     * \param mimeType   default: 'image/png', 'image/jpeg', 'image/tiff' and 'image/pdf'
     * \param bgColor   default: '#ffffff'
     * \return QImage as returned from RasterLite2 with a 32-bit ARGB format (0xAARRGGBB)
     * \see layerHasRasterlite2
     * \see rl2GetMapImageFromRaster
     * \since QGIS 3.0
     */
    QImage getMapImageFromRasterLite2( int width, int height, const QgsRectangle &viewExtent, QString styleName, QString mimeType, QString bgColor, QString &errCause );

    /**
     * Retrieve rasterlite2 image of a given bound and size.
     * - to be mainly used by the QgsRasterLite2Provider
     *  Goal is to reflect the interstructure as stored by RasterLite2
     * \note
     * The RasterLite2 connection will be made, if needed.
     * The viewExtents will use the srid of the layer
     * - only mine-types supported by QImage may be used
     * MONOCHROME, PALETTE, GRAYSCALE and DATAGRID
     * - will return 1 Band
     * RGB
     * - will return 3 Bands
     * MULTIBAND
     * - will returns 2-255 Bands
     * \param width        of image in pixel.
     * \param height       of image in pixel.
     * \param viewExtents OUT:  [west,south,east,north] [minx, miny, maxx, maxy] bounds.
     * \param styleName   default: default', when has not been registered with a RasterStyle
     * \param mimeType   default: 'image/png'
     * \param bgColor   default: '#ffffff'
     * \return QList<QByteArray *> for each Band based on the internal PixelType
     * \see layerHasRasterlite2
     * \see rl2GetMapImageFromRaster
     * \see QgsRasterLite2Provider::readBlock
     * \since QGIS 3.0
     */
    QList<QByteArray *> getMapBandsFromRasterLite2( int width, int height, const QgsRectangle &viewExtent, QString styleName, QString bgColor, QString &errCause );
  private:

    /**
     * The Database Info-Structure being read
     * \note
     *  - From the Database filename (with Path)
     * \see mDbSpatialiteInfo
    * \since QGIS 3.0
    */
    SpatialiteDbInfo *mDbSpatialiteInfo = nullptr;;

    /**
     * The Database filename (with Path)
    * \returns mDatabaseFileName complete Path (without without symbolic links)
    * \since QGIS 3.0
    */
    QString mDatabaseFileName;

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
     * Layer-Name
     * \note
     *  Vector: Layer format: 'table_name(geometry_name)'
     *  Raster: Layer format: 'coverage_name'
     * \since QGIS 3.0
     */
    QString mLayerName;

    /**
     * List-Group-Name (lower case)
     * \note
     *  When: 'table_name' contains at least 2 '_' (such as 'berlin_streets_1650')
     *  - assume it belongs to a group of 'berlin_streets' for QgsSpatiaLiteTableModel
     *  - for SpatialViews
     * \since QGIS 3.0
     */
    QString mListGroupName;

    /**
     * Coverage-Name [internal]
     * \note
     *   retrieved from vector_coverages, which may be different from the Layer-Name
     + - this can be empty if not a RasterLite2 layer
     * \since QGIS 3.0
     */
    QString mCoverageName;

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
     * Name of the Layer format: 'table_name(geometry_name)'
     * \note
     *  For SpatialTable/VirtualShapes: empty
     *  For SpatialViews:
     *  - the registered VIEW can be retrieved with getTableName()
     * \see SpatialiteDbLayer::GetLayerSettings()
     * \since QGIS 3.0
     */
    QString mLayerViewTableName;

    /**
     * Title of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite since 4.5.0: vector_coverages(title)
     *  - RasterLite2: raster_coverages(title)
     *  - GeoPackage: gpkg_contents(identifier)
     *  - MbTiles: metadata(name)
     *  - FdoOgr: none
     * \see getTitle
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
     * \see getAbstract
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
     * \see getCopyright
     * \since QGIS 3.0
     */
    QString mCopyright;

    /**
     * License of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite 4.5.0: Column License id of 'vector_coverages', with name from 'data_licenses'
     *  - RasterLite2: Column License id of 'raster_coverages', with name from 'data_licenses'
     * -> From field 'name' of TABLE data_licenses ['PD - Public Domain', 'CC BY 3.0' etc.]
     *  - GeoPackage: none
     *  - MbTiles: none
     *  - FdoOgr: none
     * \see getLicense()
     * \since QGIS 3.0
     */
    QString mLicense;

    /**
     * Srid of the Layer
     * - based on Provider specific values
     * \note
     *  - Spatialite 4.0: vector_layers(srid) [geometry_columns(srid)]
     *  - RasterLite2: raster_coverages(srid)
     *  - GeoPackage: gpkg_contents(srs_id)
     *  - MbTiles: always 4326
     *  - FdoOgr: geometry_columns(srid)
     * \see getSrid()
     * \since QGIS 3.0
     */
    int mSrid;

    /**
     * The AuthId [auth_name||':'||auth_srid]
     *  \see setSrid
     * \since QGIS 3.0
     */
    QString mAuthId;

    /**
     * The Proj4text [from mSrid]
     *  \see setSrid
     * \since QGIS 3.0
     */
    QString mProj4text;

    /**
     * The SpatialIndex-Type used for the Geometry
     * \note
     *  Uses the same numbering as Spatialite [0,1,2]
     * \see setSpatialIndexType
     * \see mSpatialIndexTypeString
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialIndexType mSpatialIndexType;

    /**
     * The SpatialIndex-Type used for the Geometry as String
     * \note
     *  Uses the same numbering as Spatialite [0,1,2]
     * \see setSpatialIndexType
     * \see mSpatialIndexType
     * \since QGIS 3.0
     */
    QString mSpatialIndexTypeString;

    /**
     * Set the SpatialIndex-Type used for the Geometry
     * - will also set the String version of theSpatialIndex-Type
     * \note
     *  Uses the same numbering as Spatialite [0,1,2]
     * \see SpatialIndexTypeName
     * \see mSpatialIndexType
     * \since QGIS 3.0
     */
    void setSpatialIndexType( int iSpatialIndexType );

    /**
     * The Spatialite Layer-Type of the Layer
     * - representing mLayerType
     *  \see setGeometryType
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialiteLayerType mLayerType;

    /**
     * Set the Spatialite Layer-Type String
     * - representing mLayerType
     *  \see setLayerType
     * \see SpatialiteLayerTypeName
     * \since QGIS 3.0
     */
    QString mLayerTypeString;

    /**
     * Set the Layer-InfoString
     * \note
     *  - field 0=Geometry-Type [for Raster-Types: LayerType]
     *  - field 1=Srid of the Layer
     *  - field 2=The Provider Name
     * \see prepare
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString mLayerInfo;

    /**
     * Set the  Layer-DataSourceUri String
     * - Provider dependent
     * \see prepare
     * \see SpatialiteDbInfo::createDbLayerInfoUri
     * \since QGIS 3.0
     */
    QString mDataSourceUri;

    /**
     * Set the Spatialite Layer-Type
     * - will also set the String version of the Layer-Type
     * \note
     *  - will set whether this Layer is supported by the QgsSpatiaLiteProvider
     * \see SpatialiteLayerTypeName
     * \see mLayerTypeString
     * \see mIsSpatialite
     * \since QGIS 3.0
     */
    void setLayerType( SpatialiteDbInfo::SpatialiteLayerType layerType );

    /**
     * Is the Layer
     * - supported by QgsSpatiaLiteProvider
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \since QGIS 3.0
     */
    bool mIsSpatialite;

    /**
     * Is the Layer
     * - supported by QgsRasterLite2Provider
     * \note
     *  - RasterLite2 specific functions should not be called when false
     *  -> rl2GetMapImageFromRaster()
     * \since QGIS 3.0
     */
    bool mIsRasterLite2;

    /**
     * The Spatialite Geometry-Type of the Layer
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see setGeometryType
     * \since QGIS 3.0
     */
    QgsWkbTypes::Type mGeometryType;

    /**
     * Set the Spatialite Geometry-Type String
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  - QgsWkbTypes::parseType(mGeometryTypeString)
     *  \see setGeometryType
     * \since QGIS 3.0
     */
    QString  mGeometryTypeString;

    /**
     * Set the Spatialite Geometry-Type
     * - will also set the String version of the Geometry-Type
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    void setGeometryType( int iGeomType );

    /**
     * The Spatialite Coord-Dimensions of the Layer
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    int mCoordDimensions;

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QgsRectangle mLayerExtent;

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QgsBox3d mLayerExtent3d;

    /**
     * Rectangle that contains the extent as WSG84 (bounding box) of the layer
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QgsBox3d mLayerExtent3dWsg84;

    /**
     * QgsPointXY that contains the Horizontal/Vertial Resolution of a Raster-Layer
     * - based on the Extent of the Layer, image width and Height are calculated
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see setLayerExtent
     * \since QGIS 3.0
     */
    QgsPointXY mLayerResolution;

    /**
     * QPoint contains the Width and Height of a Raster-Layer
     * - based on the Extent and Resolusion of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see setLayerExtent
     * \since QGIS 3.0
     */
    QPoint mLayerImageExtent;

    /**
     * QRectangle contains the Pixel size of a Raster-Layer Image
     * - based on the values found in the raster_coverages table
     * \note
     *  - count NoData =mLayerPixelSizes.x()
     *  - count ValidPixels = mLayerPixelSizes.y()
     *  - count NoData+ValidPixels = mLayerPixelSizes.width()
     *  - diff = Calculation based on extent and resolution may differ =  = mLayerPixelSizes.heght()
     *  \see setLayerBandsInfo
     * \since QGIS 3.0
     */
    QRect mLayerPixelSizes;

    /**
     * QRectangle contains the Tile Width and Height and Bands count of a Raster-Layer
     * - based on the values found in the raster_coverages table
     * \note
     *  - num_bands = mLayerBandsTileSize.x()
     *  - mLayerBandsTileSize.y(): not used (yet)
     *  - tile_width = mLayerBandsTileSize.width()
     *  - tile_height = mLayerBandsTileSize.height()
     *  \see setLayerExtent
     * \since QGIS 3.0
     */
    QRect mLayerBandsTileSize;

    /**
     * RasterLite2 Sample-Type
     *  - based on sample_type entry from raster_coverage table
     * \note
     *  Unsigned Integer
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  Signed Integer
     *  - INT8, INT16, INT32
     *  Floating Point
     *  - FLOAT, DOUBLE
     * \since QGIS 3.0
     */
    QString mLayerSampleType;

    /**
     * RasterLite2 Sample-Type
     *  - based on sample_type entry from raster_coverage table
     * \note
     *  Unsigned Integer
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  - Byte,Byte,Byte,Byte,UInt16,UInt32
     *  Signed Integer
     *  - INT8, INT16, INT32
     *  - Byte, Int16, Int32
     *  Floating Point
     *  - FLOAT, DOUBLE
     *  - Float32, Float64
     * \since QGIS 3.0
     */
    Qgis::DataType mLayerRasterDataType;

    /**
     * RasterLite2 Sample-Type as Qgis String
     *  - based on sample_type entry from raster_coverage table
     * \note
     *  Unsigned Integer
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  - Byte,Byte,Byte,Byte,UInt16,UInt32
     *  Signed Integer
     *  - INT8, INT16, INT32
     *  - Byte, Int16, Int32
     *  Floating Point
     *  - FLOAT, DOUBLE
     *  - Float32, Float64
     * \see QgsRasterLayer::htmlMetadata
     * \since QGIS 3.0
     */
    QString mLayerRasterDataTypeString;

    /**
     * RasterLite2 Pixel-Type as Integer
     *  - based on pixel_typeentry from raster_coverage table
     * \note
     *  MONOCHROME
      *  - RasterLite2 constant: Pixel Type Monochrome - Bilevel [0x11 = 17 dec]
     *  - Bands=1
     *  - 1-BIT
     *  PALETTE
     *  - RasterLite2 constant: Pixel Type Palette based [0x12 = 18 dec]
     *  - Bands=1
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8
     *  GRAYSCALE
     *  - RasterLite2 constant: Pixel Type Grayscale [0x13 = 19 dec]
     *  - Bands=1
     *  - 2-BIT, 4-BIT, UINT8
     *  RGB
     *  - RasterLite2 constant: Pixel Type Red-Green-Blue [0x14 = 20 dec]
     *  - Bands=3
     *  - UINT8, UINT16
     *  MULTIBAND
     *  - RasterLite2 constant: Pixel Type Multiband (arbitrary) [0x15 = 21 dec]
     *  - Bands= > 1 < 256
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  DATAGRID
     *  - RasterLite2 constant: Pixel Type Data-Grid [0x16 = 23 dec]
     *  - Bands=1
     *  - UINT8, UINT16, UINT32, INT8, INT16, INT32, FLOAT, DOUBLE
     * \since QGIS 3.0
     */
    int mLayerPixelType;

    /**
     * RasterLite2 Pixel-Type as String
     *  - based on pixel_typeentry from raster_coverage table
     * \note
     *  MONOCHROME
      *  - RasterLite2 constant: Pixel Type Monochrome - Bilevel [0x11 = 17 dec]
     *  - Bands=1
     *  - 1-BIT
     *  PALETTE
     *  - RasterLite2 constant: Pixel Type Palette based [0x12 = 18 dec]
     *  - Bands=1
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8
     *  GRAYSCALE
     *  - RasterLite2 constant: Pixel Type Grayscale [0x13 = 19 dec]
     *  - Bands=1
     *  - 2-BIT, 4-BIT, UINT8
     *  RGB
     *  - RasterLite2 constant: Pixel Type Red-Green-Blue [0x14 = 20 dec]
     *  - Bands=3
     *  - UINT8, UINT16
     *  MULTIBAND
     *  - RasterLite2 constant: Pixel Type Multiband (arbitrary) [0x15 = 21 dec]
     *  - Bands= > 1 < 256
     *  - 1-BIT, 2-BIT, 4-BIT, UINT8, UINT16, UINT32
     *  DATAGRID
     *  - RasterLite2 constant: Pixel Type Data-Grid [0x16 = 23 dec]
     *  - Bands=1
     *  - UINT8, UINT16, UINT32, INT8, INT16, INT32, FLOAT, DOUBLE
     * \since QGIS 3.0
     */
    QString mLayerPixelTypeString;

    /**
     * RasterLite2 Compression-Type
     *  - based on compression entry from raster_coverage table
     * \note
     *  NONE
     *  - lossless
     *  DEFLATE
     *  - lossless
     *  DEFLATE_NO
     *  - lossless
     *  LZMA
     *  - lossless
     *  LZMA_NO
     *  - lossless
     *  PNG
     *  - lossless
     *  JPEG
     *  - lossy
     *  WEBP
     *  - lossy
     *  LL_WEBP
     *  - lossless
     *  CHARLS
     *  - lossy
     *  JP2
     *  - lossy
     *  LL_JP2
     *  - lossless
     * \since QGIS 3.0
     */
    QString mLayerCompressionType;

    /**
     * The extent (bounding box) of the layer AS Extended Well Known Text
     * - based on the Srid of the Layer
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QString mLayerExtentEWKT;

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     * - based on the Wsg84 value of the Layer found in vector_coverages
     * \note
     *  - part of the vector_coverages logic
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QgsRectangle mLayerExtentWsg84;

    /**
     * The extent (bounding box) of the layer AS Extended Well Known Text
     * - based on the Wsg84 value of the Layer found in vector_coverages
     * \note
     *  - part of the vector_coverages logic
     *  \see mGeometryTypeString
     * \since QGIS 3.0
     */
    QString mLayerExtentWsg84EWKT;

    /**
     * The selected Style
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     *  \see getLayerStyleSelected
     * \since QGIS 3.0
     */
    int mLayerStyleSelected;

    /**
     * The selected Style as String (name of Style)
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     *  \see getLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mLayerStyleSelectedString;

    /**
     * The selected Style: Name
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - StyleVector or StyleRaster
     *  \see setLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mLayerStyleSelectedName;

    /**
     * The selected Style: Title
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     *  \see getLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mLayerStyleSelectedTitle;

    /**
     * The selected Style: Abstract
     * - by default: the first entry of mVectorStyleData.value
     * \note
     *  - only one Style can be rendered
     *  \see setLayerStyleSelected
     *  \see getLayerStyleSelected
     * \since QGIS 3.0
     */
    QString mLayerStyleSelectedAbstract;

    /**
     * Number of features in the layer
     * \note
     *  With UpdateLayerStatistics the Extent will also be updated and retrieved
     * \param bUpdateStatistics UpdateLayerStatistics before reading
     * \see mLayerExtent
     * \see mNumberFeatures
     * \since QGIS 3.0
     */
    long mNumberFeatures;

    /**
     * The Spatialite Layer-Readonly status [true or false]
     * \note
     *  SpatialTable: always false [writable]
     *  SpatialView: default: true [readonly]
     *  -  if Insert/Update or Delete TRIGGERs exists:  false [writable]
     *  VirtualShape: always true [readonly]
     * \see GetViewTriggers()
     * \since QGIS 3.0
     */
    int mLayerReadOnly;

    /**
     * The Spatialite Layer-Hidden status [true or false]
     * \note
     *  No idea what this is used for [at the moment not being retrieved]
     *  - from 'vector_layers_auth'
     * \see readVectorLayers
     * \since QGIS 3.0
     */
    int mLayerIsHidden;

    /**
     * Flag indicating if the Capabilities SpatialView supports Inserting
     * \note
     *  The field 'sql' of 'sqlite_master' is searched for 'INSTEAD OF INSERT'
     *  - not testing can be done if the TRIGGER works correctly
     * \see GetViewTriggers()
     * \since QGIS 3.0
     */
    bool mTriggerInsert;

    /**
     * Flag indicating if the Capabilities SpatialView supports Updating
     * \note
     *  The field 'sql' of 'sqlite_master' is searched for 'INSTEAD OF UPDATE'
     *  - not testing can be done if the TRIGGER works correctly
     * \see GetViewTriggers()
     * \since QGIS 3.0
     */
    bool mTriggerUpdate;

    /**
     * Flag indicating if the Capabilities SpatialView supports Deleting
     * \note
     *  The field 'sql' of 'sqlite_master' is searched for 'INSTEAD OF DELETE'
     *  - not testing can be done if the TRIGGER works correctly
     * \see GetViewTriggers()
     * \since QGIS 3.0
     */
    bool mTriggerDelete;

    /**
     * The Spatialite Layer-Id being created
     * \note
     *  Only a simple counter when inserting into mDbLayers
     * - value is never used
     * \see SpatialiteDbInfo::mDbLayers
     * \since QGIS 3.0
     */
    int mLayerId;

    /**
     * The Spatialite Layer Capabilities
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see getCapabilities
     * \since QGIS 3.0
     */
    QgsVectorDataProvider::Capabilities mEnabledCapabilities;

    /**
     * Name of the table or subquery
     * \note
     *  - Not used, since a Layer could be used in multiple instances of a  QgsSpatiaLiteProvider
     *  -> QgsSpatiaLiteProvider contains ia seperate version of mQuery , since the results (mQuery, mSubsetString) could effect Layers that are being used elsewhere
     * \see QgsSpatiaLiteProvider:.mQuery
     * \since QGIS 3.0
     */
    QString mQuery;

    /**
     * Name of the primary key column in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QString mPrimaryKey;

    /**
     * Column-Number of the primary key
     * \note
     *  - SpatialView: name from entry of view_rowid of views_geometry_columns
     *  -> position from inside the PRAGMA table_info('name')
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    int mPrimaryKeyCId;

    /**
     * Is the Layer valid
     * \note
     *  when false: the Layer should not be rendered
     * \since QGIS 3.0
     */
    bool mIsLayerValid;

    /**
     * Set the Layer as invalid, with possible Message, returns amount of Errors collected
     * \note
     *  Cause could be provider specific
     * \see SpatialiteDbInfo::GetVectorLayersInfo
     * \see SpatialiteDbLayer::getCapabilities
     * \since QGIS 3.0
     */
    int setLayerInvalid( QString sLayerName = QString::null, QString errCause = QString::null );

    /**
     * List of primary key columns in the table
     * \note
     *  -> SpatialView: entry of view_rowid of views_geometry_columns
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QgsAttributeList mPrimaryKeyAttrs;

    /**
     * List of layer fields in the table
     * \note
     *  - from PRAGMA table_info()
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QgsFields mAttributeFields;

    /**
     * Map of field index to default value [for Topology, the Topology-Layers]
     * List of default values of the table and possible the view
     * \note
     *  - from PRAGMA table_info()
     * \see setSpatialiteAttributeFields
     * \since QGIS 3.0
     */
    QMap<int, QVariant> mDefaultValues;

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
     * For each Band in a RasterLite2 Raster
     * - a PNG image representing the estimated distribution Histogram from a specific Band
     * \note
     *  - RL2_GetBandStatistics_Histogram
     * \see SpatialiteDbInfo::GetRasterLite2RasterLayersInfo
     * \since QGIS 3.0
     */
    QMap<int, QImage> mLayerBandsHistograms;

    /**
     * Map of valid Topology-ExportLayers
     * - contains Layer-Name and a SpatialiteDbLayer-Pointer
     * \note
     * - these are exported SpatialTables [Topology-Features (Metadata) and Geometry]
     * see chapter: TopoGeo_ExportTopoLayer: exporting a full TopoLayer into a GeoTable
     * \returns mTopologyExportLayers as QgsWkbTypes::Type
     * \see https://www.gaia-gis.it/fossil/libspatialite/wiki?name=topo-advanced#ExportTopoLayer
     * \since QGIS 3.0
     */
    QMap<QString, SpatialiteDbLayer *> mTopologyExportLayers;

    /**
     * Map of mDataSourceUri of mTopologyExportLayers
     * -  contains Layer-Name and DataSourceUri
     * \note
     * - Topology-Features (Metadata) and Geometry
     * \since QGIS 3.0
     */
    QMap<QString, QString> mTopologyExportLayersDataSourceUris;

    /**
     * Convert Spatialite GeometryType to QgsWkbTypes::Type
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

    /**
     * Fill QgsField from values retrieved  from pragma table_info
     * \note
     * - this also be used to interpret the type field returned by pragma table_info
     * - if found and not NULL, sql-syntax for default-valie will be written to the comment
     * \param sName from field name returned by pragma table_info
     * \param sType from field type returned by pragma table_info
     * \param sDefaultValue from field dflt_value returned by pragma table_info
     * \param defaultVariant returns QVariant from field dflt_value
     * \returns QgsField to be inserted into mAttributeFields
     * \see setSpatialiteAttributeFields
     * \see GetSpatialiteQgsField
     * \since QGIS 3.0
     */
    static QgsField GetSpatialiteQgsField( const QString sName, const QString sType, QString sDefaultValue, QVariant &defaultVariant );

    /**
     * Read and fill AttributeFields and Default values
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
    int setSpatialiteAttributeFields();

    /**
     * Convert Spatialite GeometryType to QgsWkbTypes::Type
     * \param spatialiteGeometryType Spatialite-GeometryType
     * \param spatialiteGeometryDimension Spatialite-Dimensions
     * \returns geomType as QgsWkbTypes::Type
     * \since QGIS 3.0
     */
    static QgsWkbTypes::Type GetGeometryType( const int spatialiteGeometryType, const int spatialiteGeometryDimension );

    /**
     * Retrieve and set View Capabilities
     * - used to set entries in SpatialiteDbLayer
     * \note
     * Starting with Spatialite 4.5.0, this information is contained within the results of gaiaGetVectorLayersList
     * \returns isReadOnly true or false
     * \see GetDbLayersInfo
     * \since QGIS 3.0
     */
    bool GetViewTriggers();

    /**
     * Retrieve extended Layer-Information
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

    /**
     * Collection of warnings for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> mWarnings;

    /**
     * Collection of reasons for the Database being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error text
     * \since QGIS 3.0
     */
    QMap<QString, QString> mErrors;

    /**
     * Map of style_id and Metadata
     * \note
     * Without a Spatialite connecton the Title, Abstract and Xml cannot be retrieved
     * - Key: style_id as retrieved from SE_vector_styled_layers or SE_raster_styled_layers
     * - Value: StyleType, StyleTitle, StyleAbstract
     * -> style_type: StyleVector,StyleRaster
     * \see readVectorRasterStyles
     * \since QGIS 3.0
     */
    QMap<int, QString> mLayerStyleInfo;

    /**
     * Contains collected Metadata for the Layer
     * \brief A structured metadata store for a map layer.
     * \note
     *  - SpatialiteDbLayer will use a copy of SpatialiteDbInfo  mLayerMetadata as starting point
     * \see QgsMapLayer::htmlMetadata()
     * \see QgsMapLayer::metadata
     * \see QgsMapLayer::setMetadata
    * \since QGIS 3.0
    */
    QgsLayerMetadata mLayerMetadata;

    /**
     * Set the collected Metadata for the Layer
     * \brief A structured metadata store for a map layer.
     * \note
     *  - SpatialiteDbLayer will use a copy of SpatialiteDbInfo  mLayerMetadata as starting point
     * \see prepare
     * \see QgsMapLayer::htmlMetadata()
     * \see QgsMapLayer::metadata
     * \see QgsMapLayer::setMetadata
    * \since QGIS 3.0
    */
    bool setLayerMetadata();

    /**
     * Does this Layer contain a Style
     * \note
     *  -  which can be retrieved with getLayerStyle
     * \see mLayerStyleSelected
     * \see getLayerStyle
     * \see setLayerStyleSelected
     * \see hasLayerStyle
     * \since QGIS 3.0
     */
    bool mHasStyle;

    /**
     * Style-Type of Layer (if any)
     * \note
     * SpatialiteDbInfo::StyleVector
     *  - SpatialiteDbInfo::SpatialTable
     *  - SpatialiteDbInfo::SpatialView:
     *  - SpatialiteDbInfo::VirtualShape
     *  - SpatialiteDbInfo::TopologyExport
     * SpatialiteDbInfo::StyleRaster
     *  - SpatialiteDbInfo::RasterLite2Raster
     * \see mLayerStyleSelected
     * \see getLayerStyle
     * \see setLayerStyleSelected
     * \see hasLayerStyle
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialiteLayerType mStyleType;

    /**
     * Prepares sql statement for geometry field.
     *  - using ST_Mult when needed and supported (spatialite version > 2.4)
     * \note
     *  - original QgsSpatiaLiteProvider function
     * \see addLayerFeatures;
     * \since QGIS 3.0
     */
    QString geomParam() const;

    /**
     * Handles an error encountered while executing an sql statement.
     *  - implementation of Provider function 'addFeatures'
     * \note
     *  - original QgsSpatiaLiteProvider function
     * \param sql used sql
     * \param errorMessage returned by the driver when using sqlite3_exec
     * \param rollback execute ROLLBACK [default=false]
     * \see getNumberFeatures();
     * \since QGIS 3.0
     */
    void handleError( const QString &sql, char *errorMessage, bool rollback = false );

    /**
     * If  RasterLite2 can be used
     *  -  must be called before any RasterLite2 functions are called
     * \note
     *  If QGis has not been compiled with RasterLite2 support, false will be returned
     * \returns true if mRasterLite2VersionMajor is GT 0 (i.e. RL2_Version() has returned a value)
     * \see dbHasRasterlite2
     * \since QGIS 3.0
     */
    bool layerHasRasterlite2();

    /**
     * When getMapImageFromRasterLite2 is called during readBlock
     * - a default Background must be given
     * - The default is '#ffffff'
     * \note
     *  If Nodata exist, those values will be used to maximal: '#rrggbb'
     * - maximal 3 bands, when less: will be filled with the last value
     *  QgsRasterLite2Provider will override this value with the canvas background
     * \see setLayerBandsInfo
     * \see QgsRasterLite2Provider::readBlock
     * \since QGIS 3.0
     */
    QString mDefaultImageBackground;

    /**
     * Set list for each Band in a RasterLite2 Raster
     * - information from RL2_GetBandStatistics_Min
     * \note
     *  - SpatialiteDbInfo::ParseSeparatorGeneral (';') is used a separator
     * - position 1: pixel_min value as integer
     * \see setLayerBandsInfo
     * \see QgsRasterLite2Provider::readBlock
     * \since QGIS 3.0
     */
    QMap<int, int> mLayerBandsNodata;

    /**
     * Retrieve rasterlite2 image of a given bound and size.
     * - https://www.gaia-gis.it/fossil/librasterlite2/wiki?name=sql_reference_list
     *  look for: RL2_GetMapImageFromRaster
     * \note
     *  The RasterLite2 connection will be made, if needed.
     * - all sanity checks for correct/default parameters will be done
     * - only mine-types supported by QImage may be used
     * \param destSrid     the destination srid (of the rasterlite2 image).
     * \param width        of image in pixel.
     * \param height       of image in pixel.
     * \param viewExtents OUT:  [west,south,east,north] [minx, miny, maxx, maxy] bounds.
     * \param mimeType     'image/tiff' etc. default: 'image/png'
     * \param styleName    registered style used in coverage. Otherwise: 'default' if empty
     * \param bgColor      html-syntax etc. default: '#ffffff'
     * \param bTransparent  true or false
     * \param quality      0-100 (for 'image/jpeg')
     * \param bReaspect true = adapt image width,height if needed based on given bounds
     * \return QImage with filled image data
     * \see layerHasRasterlite2
     * \see getMapImageFromRasterLite2
     * \since QGIS 3.0
     */
    QImage rl2GetMapImageFromRaster( int destSrid, int width, int height, const QgsRectangle &viewExtent, QString &errCause,
                                     QString styleName = QString(), QString mimeType = QString( "image/png" ), QString bgColor = QString( "#ffffff" ),
                                     bool bTransparent = true, int quality = 80, bool bReaspect = true );

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
class CORE_EXPORT QgsSpatiaLiteUtils
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
    // static functions

    /**
     * Create a SpatialiteDbInfo based Connection
     *  -> containing all needed Information about a Spatial Sqlite3 Container
     * \note
     *  - check result with spatialiteDbInfo->isDbSqlite3()
     *  -> if File exists and is a Sqlite3 Container.
     *  - check result with spatialiteDbInfo->isDbGdalOgr()
     *  -> if File only supported by QgsOgrProvider or QgsGdalProvider
     *  -> otherwise supported by QgsSpatiaLiteProvider
     * When not a Sqlite3-Container: check result of getFileMimeType()
    * \returns not nullptr if file exists
    * \since QGIS 3.0
    */
    static SpatialiteDbInfo *CreateSpatialiteDbInfo( QString dbPath, bool bLoadLayers = true, bool bShared = true, SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::SpatialUnknown );

    /**
     * Create a new Database
     * - for use with SpatialiteDbInfo
     * \note
     * - supported containers: Spatialite, GeoPackage and MBTiles
     * \returns true if the created Database is valid and of the Container-Type requested
     * \param sDatabaseFileName name of the Database to be created
     * \param errCause OUT: error Text
     * \param dbCreateOption the Container-Type to create [default=Spatialite45]
     * \see mDbCreateOption
     * \see createDatabaseSpatialite
     * \see createDatabaseGeoPackage
     * \see createDatabaseMBtiles
     * \since QGIS 3.0
     */
    static bool createSpatialDatabase( QString sDatabaseFileName, QString &errCause, SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::Spatialite45 );
    static QString createIndexName( QString tableName, QString field );
    static QString quotedIdentifier( QString id );
    static QString quotedValue( QString value );
    static void deleteWkbBlob( void *wkbBlob );
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

    /**
     * Retrieve Capabilities of QgsSqliteHandle connection
     * - at this point we are 'sniffing' the Capabilities of the opened Database
     * \note
     *  - when only a specific table or table with geometry are being looked for
     *  -> format: 'table_name(geometry_name)' or only 'table_name', with all of its geometries
     *  - otherwise all tables with geometries
     * \param sDatabaseFileName Database Filename
     * \param sLayerName when used
     * \param bLoadLayers Load all Layer-Information or only 'sniff' [default] the Database
     * \param sqlite_handle opened using QgsSqliteHandle class
     * \returns SpatialiteDbInfo with collected results
     * \see QgsSqliteHandle
     * \since QGIS 3.0
     */
    static SpatialiteDbInfo *GetSpatialiteLayerInfoWrapper( QString sDatabaseFileName, QString sLayerName = QString::null, bool bLoadLayers = false, sqlite3 *sqlite_handle  = nullptr );
  private:

};

#endif // QGSSPATIALITEDBINFO_H
