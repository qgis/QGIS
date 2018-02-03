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
#ifndef QGSSPATIALITETABLEMODEL_H
#define QGSSPATIALITETABLEMODEL_H
#include <QStandardItemModel>
class QIcon;
#include "qgis.h"
#include "qgsspatialiteconnection.h"

/**
 * A model that holds the tables of a database in a hierarchy where the
SQLite DB is the root elements that contain the individual tables as children.
The tables have the following columns: Type, Tablename, Geometry Column*/
class  QgsSpatiaLiteTableModel: public QStandardItemModel
{
  Q_OBJECT public:

    QgsSpatiaLiteTableModel();

    /**
     * Returns the number of tables in the model
     * \note
     *  - set, but not not used [candidate to be removed]
    * \returns count of Tables in Database
    * \since QGIS 3.0
    */
    int tableCount() const
    {
      return mTableCount;
    }

    /**
     * Returns the Index of Column 'Hidden' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnSortHidden
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int getColumnSortHidden() const { return mColumnSortHidden; }

    /**
     * Returns the Index of Column 'Table' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnTable
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int getTableNameIndex() const { return mColumnTable; }

    /**
     * Returns the Value of Column 'Table' from the selected Item
     * \note
     *  - Index value set during construction. Avoid manual use of numeric value in code
    * \returns QString-Value of column being used in QgsSpatiaLiteSourceSelect
    * \see getTableNameIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    QString getTableName( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), getTableNameIndex() ) )->text();
    }

    /**
     * Returns NonSpatialItemIndex  Item-Index
     * \note
     *  - not used [candidate to be removed]
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \since QGIS 3.0
    */
    QModelIndex getNonSpatialItemIndex() const { return mNonSpatialItem->index(); }

    /**
     * Returns the Index of Column 'Geometry' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnGeometryName
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int getGeometryNameIndex() const { return mColumnGeometryName; }

    /**
     * Returns the Value of Column 'Geometry' from the selected Item
     * \note
     *  - Index value set during construction. Avoid manual use of numeric value in code
    * \returns QString-Value of column being used in QgsSpatiaLiteSourceSelect
    * \see getTableNameIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    QString getGeometryName( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), getGeometryNameIndex() ) )->text();
    }

    /**
     * Returns the LayerName based on  Values of the Columns 'Table' and 'Geometry' from the selected Item
     * \note
     *  - checking is done to insure the the retured value exists
    * \returns QString Value of LayerName
    * \see getTableNameIndex
    * \see getLayerNameUris
    * \since QGIS 3.0
    */
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

    /**
     * Returns the Urls based on  LayerName  from the selected Item
     * \note
     *  - checking is done to insure the the retured value exists
    * \returns QString Value of LayerName
    * \see getLayerName
    * \since QGIS 3.0
    */
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

    /**
     * Returns the Index of Column 'Geometry' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnGeometryType
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int getGeometryTypeIndex() const { return mColumnGeometryType; }

    /**
     * Returns the GeometryType based on  Value of the Column 'GeometryType' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns QString Value of GeometryType
    * \see getTableNameIndex
    * \see getLayerNameUris
    * \since QGIS 3.0
    */
    QString getGeometryType( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), getGeometryTypeIndex() ) )->text();
    }

    /**
     * Returns the Index of Column 'Sql' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getSqlQueryIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int getSqlQueryIndex() const { return mColumnSql; }

    /**
     * Returns the Sql-Query based on  Value of the Column 'Sql' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns QString Value of GeometryType
    * \see getTableNameIndex
    * \see getLayerNameUris
    * \since QGIS 3.0
    */
    QString getSqlQuery( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), getSqlQueryIndex() ) )->text();
    }

    /**
     * Sets an sql statement that belongs to a cell specified by a model index
     * \note
     *  - checking is done that the Table and Geometry-Name are valid
    * \returns QString Value of GeometryType
    * \see getTableNameIndex
    * \see getGeometryNameIndex
    * \see QgsSpatiaLiteSourceSelect::on_mConnectionsTreeView_doubleClicked
    * \since QGIS 3.0
    */
    void setSql( const QModelIndex &index, const QString &sql );

    /**
     * Set the active Database
     * \note
     *  - this will build the Table-Model
    * \returns QString Value of Database file
    * \see QgsSpatiaLiteSourceSelect::setQgsSpatialiteDbInfo
    * \see QgsSpatiaLiteTableModel::createDatabase
    * \since QGIS 3.0
    */
    void setSpatialiteDbInfo( QgsSpatialiteDbInfo *spatialiteDbInfo, bool loadGeometrylessTables = true );

    /**
     * Returns the active Database file-path
     * \note
     *  - checking is done to insure the the retured value exists
    * \returns QString Value of Database file
     * \param withPath default: full path of file, otherwise base-name of file
    * \see QgsSpatiaLiteSourceSelect::updateStatistics
    * \since QGIS 3.0
    */
    QString getDbName( bool withPath = true ) const
    {
      QString sDatabaseName = QString();
      if ( ( mSpatialiteDbInfo ) && ( mSpatialiteDbInfo->isDbValid() ) )
      {
        if ( withPath )
        {
          sDatabaseName = mSpatialiteDbInfo->getDatabaseFileName();
        }
        else
        {
          sDatabaseName = mSpatialiteDbInfo->getFileName();
        }
      }
      return sDatabaseName;
    }

    /**
     * Connection info (DB-path) without table and geometry
     * - this will be called from classes using QgsSpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: QgsSpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
    * \see QgsSpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    QString getDatabaseUri() const
    {
      QString sDatabaseUri = QString();
      if ( ( mSpatialiteDbInfo ) && ( mSpatialiteDbInfo->isDbValid() ) )
      {
        sDatabaseUri = mSpatialiteDbInfo->getDatabaseUri();
      }
      return sDatabaseUri;
    }

    /**
     * Retrieve QgsSpatialiteDbInfo
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see QgsSpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfo *getSpatialiteDbInfo() const { return mSpatialiteDbInfo; }

    /**
     * UpdateLayerStatistics for the Database or Layers
     * - this will be called from the QgsSpatialiteDbLayer::UpdateLayerStatistics
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
     * \see QgsSpatialiteDbInfo::::UpdateLayerStatistics
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

    /**
     * Is the Container a Spatialite Database
     * \note
     *  - Spatialite specific functions should not be called when false
     *  -> UpdateLayerStatistics()
     * \see QgsSpatialiteDbInfo::isDbSpatialite()
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
     * \see getQgsSpatialiteDbLayer
     * \see prepareDataSourceUris
     * \since QGIS 3.0
     */
    QMap<QString, QString> getDataSourceUris() const { return mSpatialiteDbInfo->getDataSourceUris(); }

    /**
     * Map of valid Selected Layers requested by the User
     * - only Uris that created a valid QgsVectorLayer/QgsRasterLayer
     * -> corresponding QgsMapLayer contained in  getSelectedDb??????Layers
     * \note
     * - Key: LayerName formatted as 'table_name(geometry)'
     * - Value: Uris dependent on provider
     * - can be for both QgsSpatiaLiteProvider or QgsGdalProvider
     * \returns mSelectedLayersUris  Map of LayerNames and  valid Layer-Uris entries
     * \see QgsSpatialiteDbInfo::setSelectedDbLayers
     * \see QgsSpatialiteDbInfo::getSelectedDbRasterLayers
     * \see QgsSpatialiteDbInfo::getSelectedDbVectorLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString>  getConnectionsSelectedLayersUris() const { return mSpatialiteDbInfo->getSelectedLayersUris(); }

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
    int addDbMapLayers( QStringList saSelectedLayers, QStringList saSelectedLayersSql, QStringList saSelectedLayersStyles ) const {  return mSpatialiteDbInfo->addDbMapLayers( saSelectedLayers,  saSelectedLayersStyles, saSelectedLayersSql );  }

    /**
     * Create a new Database
     *  - for use with QgsSpatialiteDbInfo::createDatabase
     * \note
     * - SpatialMetadata::Spatialite40: InitSpatialMetadata only
     * - SpatialMetadata::Spatialite50: also with Styles/Raster/VectorCoveragesTable's
     * - SpatialMetadata::SpatialiteGpkg: using spatialite 'gpkgCreateBaseTables' function
     * - SpatialMetadata::SpatialiteMBTiles: creating a view-base MbTiles file with grid tables
     * \returns true if the created Database is valid and of the Container-Type requested
     * \see QgsSpatialiteDbInfo::createDatabase
     * \since QGIS 3.0
     */
    bool createDatabase( QString sDatabaseFileName, QgsSpatialiteDbInfo::SpatialMetadata dbCreateOption = QgsSpatialiteDbInfo::Spatialite40 );

  private:
    enum EntryType
    {
      EntryTypeLayer = 0,
      EntryTypeMap = 1
    };

    /**
     * Returns the number of tables in the model
     * \note
     *  - set, but not not used [candidate to be removed]
    * \returns count of Tables in Database
    * \since QGIS 3.0
    */
    int mTableCount;

    /**
     * Builds expected amout of Tables the number of tables in the model
     * \note
     *  - set and used for display
    * \returns count of Tables in Database
    * \see addRootEntry
    * \since QGIS 3.0
    */
    int mTableCounter;

    /**
     * Returns the Index of Column 'Hidden' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getColumnSortHidden
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int mColumnSortHidden;

    /**
     * Returns the Index of Column 'Table' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getTableNameIndex
    * \see getTableName
    * \since QGIS 3.0
    */
    int mColumnTable;

    /**
     * Returns the Index of Column 'Geometry' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getGeometryTypeIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int mColumnGeometryType;

    /**
     * Returns the Index of Column 'Geometry' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getGeometryNameIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int mColumnGeometryName;

    /**
     * Returns the Index of Column 'Sql' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getSqlQueryIndex
    * \see getSqlQuery
    * \since QGIS 3.0
    */
    int mColumnSql;

    /**
     * Coulumn Names of Model
     * \note
     *  - set during construction.
    * \since QGIS 3.0
    */
    QStringList headerLabels;

    /**
     * Retrieve the Table Entries based on a list of Layer-Types
     * \see addTableEntryType
     * \since QGIS 3.0
     */
    void addTableEntryTypes();

    /**
     * Retrieve the  Layer-Type entries
     * \see addTableEntry
     * \since QGIS 3.0
     */
    void addTableEntryType( QMap<QString, QString> mapLayers, QgsSpatiaLiteTableModel::EntryType entryType = QgsSpatiaLiteTableModel::EntryTypeLayer, QgsSpatialiteDbInfo::SpatialiteLayerType layerType = QgsSpatialiteDbInfo::SpatialiteUnknown );

    /**
     * Build entry for the  Layer-Type in Root
     * \see addRootEntry
     * \since QGIS 3.0
     */
    QList < QStandardItem * > createLayerTypeEntry( QgsSpatialiteDbInfo::SpatialiteLayerType layerType = QgsSpatialiteDbInfo::SpatialTable, int amountLayers = 0 );

    /**
     * Fill the model Item based on the retrieved Layer
     * \since QGIS 3.0
     */
    void addRootEntry();

    /**
     * Fill the model Item based on the retrieved Layer
     * \since QGIS 3.0
     */
    void addTableEntryLayer( QgsSpatialiteDbLayer *dbLayer, int iLayersCount = 1 );

    /**
     * Fill the model Item based on the retrieved Layer
     * \since QGIS 3.0
     */
    void addTableEntryMap( QString sKey, QString sValue, QgsSpatialiteDbInfo::SpatialiteLayerType layerType = QgsSpatialiteDbInfo::NonSpatialTables );

    /**
     * QgsSpatialiteDbInfo Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see QgsSpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfo *mSpatialiteDbInfo = nullptr;

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
     * \see getQgsSpatialiteDbLayer
     * \see prepareDataSourceUris
     * \since QGIS 3.0
     */
    QMap<QString, QString> mDbLayersDataSourceUris;

    /**
     * Placeholder for RootItem
     * - used as insert point of child Spatial Items when building
     * \note
     * - removes all entries when a new Database is being opened
     * \see setSpatialiteDbInfo
     * \see addRootEntry
     * \since QGIS 3.0
     */
    QStandardItem *mDbRootItem = nullptr;

    /**
     * Placeholder for NonSpatial Items
     * - used as insert point of child NonSpatial Items when building
     * \note
     * - removes all entries when a new Database is being opened
     * \see addTableEntryMap
     * \since QGIS 3.0
     */
    QStandardItem *mNonSpatialItem = nullptr;

    /**
     * Returns if Non-spatialTable should be shown
     * \note
     *  - set, but not used since all tables are shown [candidate to be removed]
    * \returns true as default
    * \since QGIS 3.0
    */
    bool mLoadGeometrylessTables = true;
};

#endif // QGSSPATIALITETABLEMODEL_H
