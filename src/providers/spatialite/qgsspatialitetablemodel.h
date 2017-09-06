/***************************************************************************
                         qgsspatialitetablemodel.h  -  description
                         -------------------
    begin                : Dec 2008
    copyright            : (C) 2008 by Sandro Furieri
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

#include <QStandardItemModel>
class QIcon;
#include "qgis.h"
#include "qgsspatialiteconnection.h"

/** A model that holds the tables of a database in a hierarchy where the
SQLite DB is the root elements that contain the individual tables as children.
The tables have the following columns: Type, Tablename, Geometry Column*/
class QgsSpatiaLiteTableModel: public QStandardItemModel
{
  Q_OBJECT public:

    QgsSpatiaLiteTableModel();
    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql );

    //! Returns the number of tables in the model
    int tableCount() const
    {
      return mTableCount;
    }
    //! Returns the Column 'Table' from the selected Item
    int getColumnSortHidden() const { return i_field_sort_hidden; }
    //! Returns the Column 'Table' from the selected Item
    int getTableNameIndex() const { return i_field_table; }
    QString getTableName( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), i_field_table ) )->text();
    }
    //! Returns NonSpatialItemIndex  Item-Inxex
    QModelIndex getNonSpatialItemIndex() const { return mNonSpatialItem->index(); }
    //! Returns the Column 'Geometry column' from the selected Item
    int getGeometryNameIndex() const { return i_field_geometry_name; }
    QString getGeometryName( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), i_field_geometry_name ) )->text();
    }
    QString getLayerName( const QModelIndex &index ) const
    {
      QString sLayerName = getTableName( index );
      QString sGeometryName = getGeometryName( index );
      if ( !sGeometryName.isEmpty() )
      {
        sLayerName = QString( "%1(%2)" ).arg( sLayerName ).arg( sGeometryName );
      }
      if ( mDbLayersDataSourceUris.contains( sLayerName ) )
      {
        return sLayerName;
      }
      return QString();
    }
    QString getLayerNameUris( const QModelIndex &index ) const
    {
      QString sLayerUris = QString();
      QString sLayerName = getLayerName( index );
      if ( !sLayerName.isEmpty() )
      {
        sLayerUris = mDbLayersDataSourceUris.value( sLayerName );
      }
      return sLayerUris;
    }
    //! Returns the Column 'Geometry column' from the selected Item
    int getGeometryTypeIndex() const { return i_field_geometry_type; }
    QString getGeometryType( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), i_field_geometry_type ) )->text();
    }
    //! Returns the Column 'Sql' from the selected Item
    int getSqlQueryIndex() const { return i_field_sql; }
    QString getSqlQuery( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), i_field_sql ) )->text();
    }
    void setSqliteDb( SpatialiteDbInfo *spatialiteDbInfo, bool loadGeometrylessTables = false );
    QString getDbName( bool withPath = true ) const
    {
      QString sDBName = QString();
      if ( ( mSpatialiteDbInfo ) && ( mSpatialiteDbInfo->isDbValid() ) )
      {
        if ( withPath )
        {
          sDBName = mSpatialiteDbInfo->getDatabaseFileName();
        }
        else
        {
          sDBName = mSpatialiteDbInfo->getFileName();
        }
      }
      return sDBName;
    }
    QString getDbConnectionInfo( ) const
    {
      QString sDbConnectionInfo = QString();
      if ( ( mSpatialiteDbInfo ) && ( mSpatialiteDbInfo->isDbValid() ) )
      {
        sDbConnectionInfo = mSpatialiteDbInfo->dbConnectionInfo( ) ;
      }
      return sDbConnectionInfo;
    }

    /** Retrieve SpatialiteDbInfo
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *getSpatialiteDbInfo() const { return mSpatialiteDbInfo; }

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
     * \see SpatialiteDbInfo::::UpdateLayerStatistics
     * \since QGIS 3.0
     */
    bool UpdateLayerStatistics( QStringList saLayers )
    {
      if ( ( mSpatialiteDbInfo ) && ( mSpatialiteDbInfo->isDbValid() ) && ( mSpatialiteDbInfo->isDbSpatialite() ) )
      {
        return mSpatialiteDbInfo->UpdateLayerStatistics( saLayers );
      }
      return false;
    }

    /** Is the Container a Spatialite Database
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \see SpatialiteDbInfo::isDbSpatialite()
     * \since QGIS 3.0
     */
    bool isSpatialite() const
    {
      if ( ( mSpatialiteDbInfo ) && ( mSpatialiteDbInfo->isDbValid() ) && ( mSpatialiteDbInfo->isDbSpatialite() ) )
      {
        return true;
      }
      return false;
    }

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
    QMap<QString, QString> getDataSourceUris() const { return mSpatialiteDbInfo->getDataSourceUris(); }

    /** Map of valid Selected Layers requested by the User
     * - only Uris that created a valid QgsVectorLayer/QgsRasterLayer
     * -> corresponding QgsMapLayer contained in  getSelectedDb??????Layers
     * \note
     * - Key: LayerName formatted as 'table_name(geometry)'
     * - Value: Uris dependent on provider
     * - can be for both QgsSpatiaLiteProvider or QgsGdalProvider
     * \returns mSelectedLayersUris  Map of LayerNames and  valid Layer-Uris entries
     * \see SpatialiteDbInfo::setSelectedDbLayers
     * \see SpatialiteDbInfo::getSelectedDbRasterLayers
     * \see SpatialiteDbInfo::getSelectedDbVectorLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString>  getSelectedLayersUris() const { return mSpatialiteDbInfo->getSelectedLayersUris(); }

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
    int addDbMapLayers( QStringList saSelectedLayers, QStringList saSelectedLayersSql ) const {  return mSpatialiteDbInfo->addDbMapLayers( saSelectedLayers, saSelectedLayersSql );  }

    /** Create a new Database
     *  - for use with SpatialiteDbInfo::createDatabase
     * \note
     * - SpatialMetadata::Spatialite40: InitSpatialMetadata only
     * - SpatialMetadata::Spatialite45: also with Styles/Raster/VectorCoveragesTable's
     * - SpatialMetadata::SpatialiteGpkg: using spatialite 'gpkgCreateBaseTables' function
     * - SpatialMetadata::SpatialiteMBTiles: creating a view-base MbTiles file with grid tables
     * \returns true if the created Database is valid and of the Container-Type requested
     * \see SpatialiteDbInfo::createDatabase
     * \since QGIS 3.0
     */
    bool createDatabase( QString sDatabaseFileName, SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::Spatialite40 );


  private:
    enum EntryType
    {
      EntryTypeLayer = 0,
      EntryTypeMap = 1
    };

    //! Number of tables in the model
    int mTableCount;
    int mTableCounter;
    int i_field_sort_hidden;
    int i_field_table;
    int i_field_geometry_type;
    int i_field_geometry_name;
    int i_field_sql;
    QStringList headerLabels;

    /** Retrieve the Table Entries based on a list of Layer-Types
     * \see addTableEntryType
     * \since QGIS 3.0
     */
    void addTableEntryTypes( );

    /** Retrieve the  Layer-Type entries
     * \see addTableEntry
     * \since QGIS 3.0
     */
    void addTableEntryType( QMap<QString, QString> mapLayers, QgsSpatiaLiteTableModel::EntryType entryType = QgsSpatiaLiteTableModel::EntryTypeLayer, SpatialiteDbInfo::SpatialiteLayerType layerType = SpatialiteDbInfo::SpatialiteUnknown );

    /** Build entry for the  Layer-Type in Root
     * \see addRootEntry
     * \since QGIS 3.0
     */
    QList < QStandardItem * > createLayerTypeEntry( SpatialiteDbInfo::SpatialiteLayerType layerType = SpatialiteDbInfo::SpatialTable, int amountLayers = 0 );

    /** Fill the model Item based on the retrieved Layer
     * \since QGIS 3.0
     */
    void addRootEntry( );

    /** Fill the model Item based on the retrieved Layer
     * \since QGIS 3.0
     */
    void addTableEntryLayer( SpatialiteDbLayer *dbLayer, int iLayersCount = 1 );

    /** Fill the model Item based on the retrieved Layer
     * \since QGIS 3.0
     */
    void addTableEntryMap( QString sKey, QString sValue, SpatialiteDbInfo::SpatialiteLayerType layerType = SpatialiteDbInfo::NonSpatialTables );

    /** SpatialiteDbInfo Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;

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

    QStandardItem *mDbRootItem = nullptr;
    QStandardItem *mNonSpatialItem = nullptr;
    bool mLoadGeometrylessTables = true;
};
