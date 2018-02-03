/***************************************************************************
                         qgsspatialitedbinfomodel.h  -  description
                         -------------------
    begin                : December 2017
    copyright            : (C) 2017 by Mark Johnson
    email                : mj10777@googlemail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALITEDBINFOMODEL_H
#define QGSSPATIALITEDBINFOMODEL_H

#include <QAbstractItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QVector>
#include "qgsspatialiteconnection.h"
// -- ---------------------------------- --
// Based on QT-Demo editabletreemodel [/qtbase/examples/widgets/itemviews/editabletreemodel/]
// qmake ; make ; ./editabletreemodel &
// -- ---------------------------------- --
class QgsSpatialiteDbInfo;
class QgsSpatialiteDbLayer;
class QgsSpatialiteDbInfoItem;

/**
 * This TreeView Modul will reflect the information stored in the QgsSpatialiteDbInfo class
  * - which contains all information collected from a Spatialite-Database
  * \note
  *  - The QgsSpatialiteDbInfo is contained in the QgsSqliteHandle class
  *  -> so that the Information must only be called once for each connection
  *  --> When shared, the QgsSqliteHandle class will used the same connection of each Layer in the same Database
  * \see QgsSqliteHandle
  * \see QgsSpatialiteDbInfo
  * \see QgsSpatialiteDbLayer
  * \since QGIS 3.0
 */
// bin/qgis': double free or corruption
class QgsSpatialiteDbInfoModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    QgsSpatialiteDbInfoModel( QObject *parent = nullptr );
    ~QgsSpatialiteDbInfoModel();

    enum SpatialiteDbInfoModelType
    {
      ModelTypeConnections = 0,
      ModelTypeLayerOrder = 1,
      ModelTypeUnknown = 99999
    };

    /**
     * General function to retrieve Data from Item
     * \note
     *  - should be used by specific column functions [candidate to be moved to private]
    * \returns QVariant of column value
    * \since QGIS 3.0
    */
    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;

    /**
     * Retrieve Header Column Name
     * \note
     *  - may not be needed [candidate to be removed]
    * \returns QVariant of column value
    * \since QGIS 3.0
    */
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;

    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
    QModelIndex parent( const QModelIndex &index ) const Q_DECL_OVERRIDE;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;

    /**
     * Returns amount of Columns of the Model of the rootItem
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns count of columns
    * \since QGIS 3.0
    */
    int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;

    /**
     * Returns the set flag for the given Item's column of the row
     * \note
     *  - set during construction of Item
    * \param index with row() and column() of Item
    * \returns flags for the given row and column of the Item
    * \see QgsSpatialiteDbInfoItem::flags
    * \since QGIS 3.0
    */
    Qt::ItemFlags flags( const QModelIndex &index ) const Q_DECL_OVERRIDE;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) Q_DECL_OVERRIDE;
    bool setHeaderData( int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole ) Q_DECL_OVERRIDE;

    bool insertColumns( int position, int columns, const QModelIndex &parent = QModelIndex() ) Q_DECL_OVERRIDE;
    bool removeColumns( int position, int columns, const QModelIndex &parent = QModelIndex() ) Q_DECL_OVERRIDE;
    bool insertRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) Q_DECL_OVERRIDE;
    bool removeRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) Q_DECL_OVERRIDE;

    /**
     * Retrieve the Modul-Type
     * Modul-Type being used
     * \note
     *  ModelTypeConnections
     *  ModelTypeLayerOrder
     * \since QGIS 3.0
     */
    SpatialiteDbInfoModelType getModelType() const { return mModelType; }

    /**
     * Returns the String value of the enum of Model-Types of the QgsSpatialiteDbInfoItem
     * - QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelType
     * \note
     * \see getItemTypeString
     * \since QGIS 3.0
     */
    static QString SpatialiteDbInfoModelTypeName( QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelType modelType );

    /**
     * Returns  the enum of Model-Type of the QgsSpatialiteDbInfoItem as String
     * \note
     *  -
    * \returns Model-Type of the QgsSpatialiteDbInfoItem
    * \see mItemType
    * \see getItemType()
    * \since QGIS 3.0
    */
    QString getModelTypeString() const { return QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelTypeName( mModelType ); }

    /**
     * Placeholder for Model RootItem
     * - used as insert point of child Spatial Items when building
     * \note
     * - removes all entries when a new Database is being opened
     * \see setSpatialiteDbInfo
     * \see addDatabaseEntry
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *getModelRootItem() { return mModelRootItem; }


    /**
     * Set the Modul-Type
     * Modul-Type being used
     * \note
     *  ModelTypeConnections
     *  ModelTypeLayerOrder
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *setModelType( SpatialiteDbInfoModelType modelType );

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
     * Returns the Text of Column 'Table' from the Header
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnTable
    * \see QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect
    * \since QGIS 3.0
    */
    QString getTableNameText() const;

    /**
     * Returns the Value of Column 'Table' from the selected Item
     * \note
     *  - Index value set during construction. Avoid manual use of numeric value in code
    * \returns QString-Value of column being used in QgsSpatiaLiteSourceSelect
    * \see getTableNameIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    QString getTableName( const QModelIndex &index ) const;

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
     * Returns the Text of Column 'Geometry' from the Header
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnGeometryName
    * \see QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect
    * \since QGIS 3.0
    */
    QString getGeometryNameText() const;

    /**
     * Returns the Value of Column 'Geometry-Type' from the selected Item
     * \note
     *  - Index value set during construction. Avoid manual use of numeric value in code
     * \param index QModelIndex to use to retrieve the Item
    * \returns QString-Value of column being used in QgsSpatiaLiteSourceSelect
    * \see getTableNameIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    QString getGeometryName( const QModelIndex &index ) const;

    /**
     * Returns the LayerName based on  Values of the Columns 'Table' and 'Geometry' from the selected Item
     * \note
     *  - checking is done to insure the the retured value exists
     * \param index QModelIndex to use to retrieve the Item
    * \returns QString Value of LayerName
    * \see getTableNameIndex
    * \see getLayerNameUris
    * \since QGIS 3.0
    */
    QString getLayerName( const QModelIndex &index ) const;

    /**
     * Returns the LayerName based on  Values of the Columns 'Table' and 'Geometry' from the selected Item
     * \note
     *  - checking is done to insure the the retured value exists
     * \param index QModelIndex to use to retrieve the Item
     * \param iParm 0: return any Item found ; 1=specific Item-Type
     * \returns layerItem* Item which contains a  QgsSpatialiteDbLayer
     * \see getTableNameIndex
     * \see getLayerNameUris
     * \since QGIS 3.0
    */
    QgsSpatialiteDbInfoItem *getLayerItem( const QModelIndex &index, int iParm = 0 );

    /**
     * Remove a Item, inside its Parent/Group
     *  - if, after the removal, the Parent/Group is empty: the Group will be removed
     * \note
     * Type: ModelTypeLayerOrder only
     *  -beginRemoveRows and endRemoveRows are used to insure that the calling View does not become confused
     * \param iMoveCount a +/- number away from present position inside the parent
     * \returns QModelIndex of the next Item to be used for setCurrentIndex of the View, when valid
     * \see QgsSpatialiteDbInfoItem::removeChild
     * \see QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequest
     * \since QGIS 3.0
    */
    QModelIndex removeLayerOrderChild( QgsSpatialiteDbInfoItem *layerChild );

    /**
     * Move a Item, inside its Parent, to a new position
     *  - for Up/Down ; First to Last ; Last to First or a specific position inside the Parent
     * \note
     * Type: ModelTypeLayerOrder only
     * presentPosition + (negative value) < 0 = Last Position
     * presentPosition + (positive value) >= count = First Position
     *  - beginMoveRows and endMoveRows are used to insure that the calling View does not become confused
     * \param iMoveCount is a +/- number from the present position inside the parent
     * \returns QModelIndex of the next Item to be used for setCurrentIndex of the View, when valid
     * \see QgsSpatialiteDbInfoItem::moveChildPosition
     * \see QgsSpatialiteDbInfoItem::moveChild
     * \see QgsSpatiaLiteSourceSelect::moveLayerOrderUpDown
     * \since QGIS 3.0
    */
    QModelIndex moveLayerOrderChild( QgsSpatialiteDbInfoItem *layerChild, int iMoveCount = 0 );

    /**
     * Set Selected Style
     *  The selected style will be set in the Layer entry
     * \note
     * Emits layoutAboutToBeChanged before calling the Item function
     * and layoutChanged after to insure that the change is shown
     *  - This will be used for the 'addDbMapLayers' logic
     * \see QgsSpatialiteDbInfoItem::setSelectedStyle
     * \see QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_doubleClicked
     * \since QGIS 3.0
     */
    bool setSelectedStyle( QgsSpatialiteDbInfoItem *layerChild );

    /**
     * Returns the Urls based on  LayerName  from the selected Item
     * \note
     *  - checking is done to insure the the retured value exists
     * \param index QModelIndex to use to retrieve the Item
     * \returns QString Value of LayerName
     * \see getLayerName
     * \since QGIS 3.0
    */
    QString getLayerNameUris( const QModelIndex &index ) const;

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
     * ExpandToDepth
     * \note
     *  - 3: ItemTypeHelpRoot: all level should be shown
     *  - 2: - ItemTypeDb: down to Table/View name, but not the Columns
     * \since QGIS 3.0
     */
    int getExpandToDepth();

    /**
     * Returns the Text of Column 'Geometry-Type' from the Header
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnGeometryType
    * \see QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect
    * \since QGIS 3.0
    */
    QString getGeometryTypeText() const;

    /**
     * Returns the GeometryType based on  Value of the Column 'GeometryType' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns QString Value of GeometryType
    * \see getTableNameIndex
    * \see getLayerNameUris
    * \since QGIS 3.0
    */
    QString getGeometryType( const QModelIndex &index ) const;

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
     * Returns the Text of Column 'SqlQuery' from the Header
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnSql
    * \see QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect
    * \since QGIS 3.0
    */
    QString getSqlQueryText() const;

    /**
     * Returns the Sql-Query based on  Value of the Column 'Sql' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns QString Value of GeometryType
    * \see getTableNameIndex
    * \see getLayerNameUris
    * \since QGIS 3.0
    */
    QString getSqlQuery( const QModelIndex &index ) const;

    /**
     * Sets an sql statement that belongs to a cell specified by a model index
     * \note
     *  - checking is done that the Table and Geometry-Name are valid
    * \returns QString Value of GeometryType
    * \see QgsSpatialiteDbInfoModel::setLayerSqlQuery
    * \see QgsSpatialiteDbInfoModel::getLayerSqlQuery
    * \see QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_doubleClicked
    * \since QGIS 3.0
    */
    void setLayerSqlQuery( const QModelIndex &index, const QString &sSelectedLayerSql );

    /**
     * Returns the Sql-Query based on  Value of the Column 'Sql' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns QString Value of GeometryType
    * \see getTableNameIndex
    * \see getLayerNameUris
    * \since QGIS 3.0
    */
    QString getLayerSqlQuery( const QModelIndex &index ) const;

    /**
     * Set the active Database
     * \note
     *  - this will build the Table-Model
    * \returns QString Value of Database file
    * \see QgsSpatiaLiteSourceSelect::setQgsSpatialiteDbInfo
    * \see QgsSpatialiteDbInfoModel::createDatabase
    * \since QGIS 3.0
    */
    void setSpatialiteDbInfo( QgsSpatialiteDbInfo *spatialiteDbInfo );

    /**
     * Add or Remove Items to 'LayerOrder'
     * \note
     *  - ModelTypeLayerOrder only
    * \param bRemoveItems are the selected Item to be added or removed
    * \returns iCountItems count of unique-Items added or removed
    * \see QgsSpatialiteDbInfoItem::handelSelectedPrepairedChildren
    * \see QgsSpatiaLiteSourceSelec::setLayerOrderData
    * \since QGIS 3.0
    */
    int setLayerOrderData( bool bRemoveItems = false );

    /**
     * Returns the active Database file-path
     * \note
     *  - checking is done to insure the the retured value exists
    * \param withPath default: full path of file, otherwise base-name of file
    * \returns QString Value of Database file
    * \see QgsSpatiaLiteSourceSelect::updateStatistics
    * \since QGIS 3.0
    */
    QString getDbName( bool withPath = true ) const;

    /**
     * Connection info (DB-path) without table and geometry
     * - this will be called from classes using QgsSpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: QgsSpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
    * \see QgsSpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    QString getDatabaseUri() const;

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
    bool UpdateLayerStatistics( QStringList saLayers );

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
    int addDbMapLayers( QStringList saSelectedLayers, QStringList saSelectedLayersStyles, QStringList saSelectedLayersSql ) const {  return mSpatialiteDbInfo->addDbMapLayers( saSelectedLayers, saSelectedLayersStyles, saSelectedLayersSql );  }

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
     * Returns QgsSpatialiteDbInfoItem of the selected Item
     * \note
     *  - QgsSpatialiteDbInfoItem
     * Can be used in the same way as 'itemFromIndex' in QStandardItemModel
    * \returns QgsSpatialiteDbInfoItem of type QgsSpatialiteDbInfoItem
    * \since QGIS 3.0
    */
    QgsSpatialiteDbInfoItem *getItem( const QModelIndex &index ) const;

    /**
     * Return layer tree node for given index. Returns root node for invalid index.
     * Returns null pointer if index does not refer to a layer tree node (e.g. it is a legend node)
    * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *index2node( const QModelIndex &index ) const;

    /**
     * Return index for a given node. If the node does not belong to the layer tree, the result is undefined
    * \since QGIS 3.0
     */
    QModelIndex node2index( QgsSpatialiteDbInfoItem *node ) const;

    /**
     * Loads Help text after Contruction
     * - Display text for the User, giving some information about what the Model is used for
     * \note
     *  This can only be called after 'connectToRootItem' has run
     *  - Will be removed when the first Database is loaded
     * \see setSpatialiteDbInfo
    * \since QGIS 3.0
     */
    void loadItemHelp();

    /**
     * Returns a list of items that match the given \a text, using the given \a flags, in the given \a column.
     * \note
     * - taken from QStandardItemModel
     * \since QGIS 3.0
     */
    QList<QgsSpatialiteDbInfoItem *> findItems( const QString &text, Qt::MatchFlags flags = Qt::MatchExactly, int column = 0 ) const;

  private:

    /**
     * The Modul-Type
     * Modul-Type being used
     * \note
     *  ModelTypeConnections
     *  ModelTypeLayerOrder
     * \since QGIS 3.0
     */
    SpatialiteDbInfoModelType mModelType = SpatialiteDbInfoModelType::ModelTypeUnknown;

    /**
     * Placeholder for Model RootItem
     * - used as insert point of child Spatial Items when building
     * \note
     * - removes all entries when a new Database is being opened
     * \see setSpatialiteDbInfo
     * \see addDatabaseEntry
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *mModelRootItem = nullptr;

    /**
     * Placeholder for HelpItem
     * - used as insert point of child Spatial Items when building
     * \note
     * - removes all entries when a new Database is being opened
     * \see setSpatialiteDbInfo
     * \see addDatabaseEntry
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *mModelHelpItem = nullptr;

    /**
     * Placeholder for RootItem
     * - used as insert point of child Spatial Items when building
     * \note
     * - removes all entries when a new Database is being opened
     * \see setSpatialiteDbInfo
     * \see addDatabaseEntry
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *mModelDataItem = nullptr;

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
     * Returns the Index of Column 'Hidden' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getColumnSortHidden
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int mColumnSortHidden = -1;

    /**
     * Returns the Index of Column 'Table' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getTableNameIndex
    * \see getTableName
    * \since QGIS 3.0
    */
    int mColumnTable = -1;

    /**
     * Returns the Index of Column 'Geometry' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getGeometryTypeIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int mColumnGeometryType = -1;

    /**
     * Returns the Index of Column 'Geometry' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getGeometryNameIndex
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int mColumnGeometryName = -1;

    /**
     * Returns the Index of Column 'Sql' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see getSqlQueryIndex
    * \see getSqlQuery
    * \since QGIS 3.0
    */
    int mColumnSql = -1;

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
     * Retrieve Layer of Item
     * - contains Layer-Name and a QgsSpatialiteDbLayer-Pointer
     * \note
     * - child Item will receive Layer from Parent
     * -- when  getDbLoadLayers is true, getQgsSpatialiteDbLayer will load all layers
     * \returns mDbLayer of active Layer
     *  \see onInit
     * \since QGIS 3.0
     */
    QgsSpatialiteDbLayer *mDbLayer = nullptr;

    /**
     * Coulumn Names of Model
     * \note
     *  - set during construction.
    * \since QGIS 3.0
    */
    QVector<QVariant> mRootData;

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
    * \see QgsSpatialiteDbInfo::getTableCounter
    * \since QGIS 3.0
    */
    int mTableCounter = 0;

    /**
     *  Total amount of non-SpatialTables/Views in Database
     * \note
     *  - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int mNonSpatialTablesCounter = 0;

    /**
     * Count of used Srid's in  the Database
     * \note
     *  - Used to build the SridGroup
    * \see QgsSpatialiteDbInfo::getDbSridInfo
    * \since QGIS 3.0
    */
    int mSridInfoCounter = 0;

    /**
     *  Total amount of Styles in Database
     * \note
     *  - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int mStylesCounter = 0;

    /**
     * Connect the RootNode to signels for creation/removel and changes to the QgsSpatialiteDbInfoItem
     * \note
     * - taken from QgsLayerTreeModel
     * \see mModelRootItem
     * \since QGIS 3.0
     */
    void connectToRootItem();

    /**
     * Disconnect the RootNode to signels for creation/removel and changes to the QgsSpatialiteDbInfoItem
     * - may not needed, since the mModelRootItem will remain constant
     * \note
     * - taken from QgsLayerTreeModel
     * \see mModelRootItem
     * \since QGIS 3.0
     */
    void disconnectFromRootItem();

    /**
     * ExpandToDepth
     * \note
     *  - for use with mConnectionsTreeView->expandToDepth
     * \since QGIS 3.9
     */
    int mExpandToDepth = 3;

  public slots:
    // void spatialiteDbInfoMenuRequested( QPoint point_position );

  protected slots:
    void nodeWillAddChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo );
    void nodeAddedChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo );
    void nodeWillRemoveChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo );
    void nodeRemovedChildren();
};

/**
 * Class to contain all information stored in the mItemData member of the QgsSpatialiteDbInfoItem
  * \note
  *  One QgsSpatialiteDbInfoItem object will contain all Columns
  *  - helper functions exist, to set or extract specific Information-Types for a specific Column
  * \see column
  * \see value
  * \see role
  * \see QgsSpatialiteDbInfoItem::setData
  * \see QgsSpatialiteDbInfoItem::data
  * \see QgsSpatialiteDbInfoItem::setText
  * \see QgsSpatialiteDbInfoItem::text
  * \see QgsSpatialiteDbInfoItem::setFlags
  * \see QgsSpatialiteDbInfoItem::flags
  * \see QgsSpatialiteDbInfoItem::setIcon
  * \see QgsSpatialiteDbInfoItem::icon
  * \see QgsSpatialiteDbInfoItem::setBackground
  * \see QgsSpatialiteDbInfoItem::setForeground
  * \see QgsSpatialiteDbInfoItem::setAlignment
  * \see QgsSpatialiteDbInfoItem::setToolTip
  * \see QgsSpatialiteDbInfoItem::toolTip
  * \see QgsSpatialiteDbInfoItem::setStatusTip
  * \see QgsSpatialiteDbInfoItem::statusTip
  * \see QgsSpatialiteDbInfoItem::setFlags
  * \see QgsSpatialiteDbInfoItem::flags
  * \see QgsSpatialiteDbInfoItem::setSelectable
  * \see QgsSpatialiteDbInfoItem::isSelectable
  * \see QgsSpatialiteDbInfoItem::setEditable
  * \see QgsSpatialiteDbInfoItem::isEditable
  * \see QgsSpatialiteDbInfoItem::setEnabled
  * \see QgsSpatialiteDbInfoItem::isEnabled
  * \since QGIS 3.0
 */
class QgsSpatialiteDbInfoData
{
  public:
    inline QgsSpatialiteDbInfoData() : role( -1 ) {}
    inline QgsSpatialiteDbInfoData( int r, const QVariant &v, int c ) : role( r ), value( v ), column( c ) {}
    int role;
    QVariant value;
    int column;
    inline bool operator==( const QgsSpatialiteDbInfoData &other ) const { return role == other.role && value == other.value && column == other.column; }
};
Q_DECLARE_TYPEINFO( QgsSpatialiteDbInfoData, Q_MOVABLE_TYPE );
// -- ---------------------------------- --

/**
 * Class to contain all information needed for a QgsSpatialiteDbInfo object
  * \note
  *  An Item-Column, is by default:
  *  -> Enabled, Selectable but not Editable but is set with Qt::EditRole
  *  --> setEditable must be called for each Column for which this is allowed and the TreeView must be properly set
  * TreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed)
  * \see QgsSpatialiteDbInfo
  * \see QgsSpatialiteDbLayer
  * \since QGIS 3.0
 */
class QgsSpatialiteDbInfoItem : public QObject
{
    Q_OBJECT

  public:
    enum SpatialiteDbInfoItemType
    {
      ItemTypeRoot = 0,
      ItemTypeDb = 1,
      ItemTypeLayer = 2,
      ItemTypeColumn = 3,
      ItemTypeCommonMetadata = 10,
      ItemTypeMBTilesMetadata = 11,
      ItemTypeStylesMetadata = 12,
      ItemTypePointMetadata = 20,
      ItemTypeRectangleMetadata = 21,
      ItemTypeNonSpatialTable = 100,
      ItemTypeNonSpatialTablesSubGroups = 101,
      ItemTypeNonSpatialTablesSubGroup = 102,
      ItemTypeSpatialRefSysAuxSubGroup = 200,
      ItemTypeSpatialRefSysAux = 201,
      ItemTypeLayerOrderRasterGroup = 500,
      ItemTypeLayerOrderRasterLayer = 501,
      ItemTypeLayerOrderRasterItem = 502,
      ItemTypeLayerOrderVectorGroup = 520,
      ItemTypeLayerOrderVectorLayer = 521,
      ItemTypeLayerOrderVectorItem = 522,
      ItemTypeHelpText = 998,
      ItemTypeHelpRoot = 999,
      ItemTypeMetadataRoot = 1000,
      ItemTypeMetadataGroup = 1001,
      ItemTypeGroupSpatialTable = QgsSpatialiteDbInfo::SpatialTable + 10000,
      ItemTypeGroupSpatialView = QgsSpatialiteDbInfo::SpatialView + 10000,
      ItemTypeGroupVirtualShape = QgsSpatialiteDbInfo::VirtualShape + 10000,
      ItemTypeGroupRasterLite1 = QgsSpatialiteDbInfo::RasterLite1 + 10000,
      ItemTypeGroupRasterLite2Vector = QgsSpatialiteDbInfo::RasterLite2Vector + 10000,
      ItemTypeGroupRasterLite2Raster = QgsSpatialiteDbInfo::RasterLite2Raster + 10000,
      ItemTypeGroupSpatialiteTopology = QgsSpatialiteDbInfo::SpatialiteTopology + 10000,
      ItemTypeGroupTopologyExport = QgsSpatialiteDbInfo::TopologyExport + 10000,
      ItemTypeGroupStyleVector = QgsSpatialiteDbInfo::StyleVector + 10000,
      ItemTypeGroupStyleRaster = QgsSpatialiteDbInfo::StyleRaster + 10000,
      ItemTypeGroupGdalFdoOgr = QgsSpatialiteDbInfo::GdalFdoOgr + 10000,
      ItemTypeGroupGeoPackageVector = QgsSpatialiteDbInfo::GeoPackageVector + 10000,
      ItemTypeGroupGeoPackageRaster = QgsSpatialiteDbInfo::GeoPackageRaster + 10000,
      ItemTypeGroupMBTilesTable = QgsSpatialiteDbInfo::MBTilesTable + 10000,
      ItemTypeGroupMBTilesView = QgsSpatialiteDbInfo::MBTilesView + 10000,
      ItemTypeGroupMetadata = QgsSpatialiteDbInfo::Metadata + 10000,
      ItemTypeGroupAllSpatialLayers = QgsSpatialiteDbInfo::AllSpatialLayers + 10000,
      ItemTypeGroupNonSpatialTables = QgsSpatialiteDbInfo::NonSpatialTables + 10000,
      ItemTypeSpatialRefSysAuxGroups = QgsSpatialiteDbInfo::SpatialRefSysAux + 10000,
      ItemTypeGroupWarning = 99980,
      ItemTypeWarning = 99981,
      ItemTypeGroupError = 99990,
      ItemTypeError = 99991,
      ItemTypeUnknown = 99999
    };

    /**
     * QgsSpatialiteDbInfoItem Object
     * - creating a Root with ModelType
     * \note
     * \param parent parent Item
     * \param itemType Type of Item-Group
     * \see createGroupLayerItem
     * \see createGroupLayerItem
     * \since QGIS 3.0
     */
    explicit QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, SpatialiteDbInfoItemType itemType, QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelType modelType );

    /**
     * QgsSpatialiteDbInfoItem Object
     * - containing all Information about Database file
     * \note
     * - this constructor will call everything it needed to represent the Database
     * -> in sniffSpatialiteDbInfo
     * \param parent parent Item
     * \param spatialiteDbInfo QgsSpatialiteDbInfo to represent/display [may be nullptr]
     * \see buildDbInfoItem
     * \see sniffSpatialiteDbInfo
     * \since QGIS 3.0
     */
    explicit QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, QgsSpatialiteDbInfo *spatialiteDbInfo );

    /**
     * QgsSpatialiteDbInfoItem Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \param parent parent Item
     * \param dbLayer QgsSpatialiteDbLayer to represent/display [may be nullptr]
     * \see QgsSpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    explicit QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, QgsSpatialiteDbLayer *dbLayer );

    /**
     * QgsSpatialiteDbInfoItem Object
     * - creating a Sub-Group
     * \note
     * \param parent parent Item
     * \param itemType Type of Item-Group
     * \see createGroupLayerItem
     * \see createGroupLayerItem
     * \since QGIS 3.0
     */
    explicit QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, SpatialiteDbInfoItemType itemType );

    /**
     * QgsSpatialiteDbInfoItem Object
     * - creating a Sub-Group inside a tem-Group
     * \note
     * \param parent parent Item
     * \param itemType Type of SubGroup
     * \param sGroupName Name of SubGroup
     * \param saTableNames List of TableName to add
     * \see addTableItemMap
     * \since QGIS 3.0
     */
    explicit QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, SpatialiteDbInfoItemType itemType, QString sItemName, QStringList saItemInfos );

    /**
     * QgsSpatialiteDbInfoItem distructor
     * \note
     * - imDbLayer /QgsSpatialiteDbLayer) will be set to nullptr
     * -> must never be deleted, may be used elsewhere
     * \since QGIS 3.0
     */
    ~QgsSpatialiteDbInfoItem();

    /**
     * Retrieve the Modul-Type
     * Modul-Type being used
     * \note
     *  ModelTypeConnections
     *  ModelTypeLayerOrder
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelType getModelType() const { return mModelType; }

    /**
     * Returns  the enum of Model-Type of the QgsSpatialiteDbInfoItem as String
     * \note
     *  -
    * \returns Model-Type of the QgsSpatialiteDbInfoItem
    * \see mItemType
    * \see getItemType()
    * \since QGIS 3.0
    */
    QString getModelTypeString() const { return QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelTypeName( mModelType ); }

    /**
     * Set the Modul-Type
     * Modul-Type being used
     * \note
     *  ModelTypeConnections
     *  ModelTypeLayerOrder
     * \since QGIS 3.0
     */
    void setModelType( QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelType modelType ) { mModelType = modelType; }

    /**
     * Retrieve MenuText and ToolTip Text for ContextMenue spatialiteDbInfoCopyCellText
     * - setting for QAction
     * \note
     * \param iColumn IN Column selected
     * \param iCommandRole OUT 0=no CommandRole ; 1=EditRole ; 2=RemoveRole
     * \param sMenuText OUT Text for the QAction.setText
     * \param sToolTip OUT Text for the QAction.setToolTip
     * \returns if the Item-Type and Columns should support something
     * \since QGIS 3.0
     */
    bool getCopyCellText( int iColumn, int &iCommandRole, QString &sMenuText, QString &sToolTip );

    /**
     * Returns the String value of the enum of Item-Types of the QgsSpatialiteDbInfoItem
     * - QgsSpatialiteDbInfoItem::SpatialiteDbInfoItemType
     * \note
     * \see getItemTypeString
     * \since QGIS 3.0
     */
    static QString SpatialiteDbInfoItemTypeName( QgsSpatialiteDbInfoItem::SpatialiteDbInfoItemType itemType );

    /**
     * Connect the Node to signels for creation/removel and changes to the QgsSpatialiteDbInfoItem
     * - forward the signal towards the root
     * \note
     * - taken from QgsLayerTreeModel
     * \see mModelRootItem
     * \since QGIS 3.0
     */
    void connectToChildItem( QgsSpatialiteDbInfoItem *node );

    /**
     * Add Columns Items to Geometry-Table entry
     * - List other columns of the Layer, excluding the used geometry
     * \note
     * - mDbLayer must be set
     * -> displaying information about the columns
     * --> this does not include the geometry of the Layer
    * \returns count of colums the geometry belongs to
     * \see buildMetadataGroupItem
     * \see buildMetadataRootItem
     * \see buildDbLayerItem
     * \since QGIS 3.0
     */
    int addColumnItems();

    /**
     * Add Raster Metadata Items to RasterLite1, RasterLite2 and GeoPackage-Raster-Table entry
     * \note
     * - can be any Layer container [Vecot/Raster]
     * -> displaying name, value of the metadata-Table
     * \returns count of Child-Items added
     * \see buildMetadataGroupItem
     * \see buildMetadataRootItem
     * \see buildDbLayerItem
     * \since QGIS 3.0
     */
    int addCommonMetadataItems();

    /**
     * Add Style-Items [Vector/Raster]
     * - List registered Styles of the Layer
     * \note
     * - can be any Layer container [Vector/Raster]
     * -> displaying name, value of the metadata-Table
     * \returns count of Child-Items added
     * \see buildMetadataGroupItem
     * \see buildMetadataRootItem
     * \see buildDbLayerItem
     * \since QGIS 3.0
     */
    int addLayerStylesItems();

    /**
     * Add Columns Items to MBTiles-Table entry
     * - List the entries in the MbTiles specifice metadata TABLE
     * \note
     * - must be a MBTiles-Container
     * -> displaying name, value of the metadata-Table
     * \returns count of Child-Items added
     * \see buildMetadataGroupItem
     * \see buildMetadataRootItem
     * \see buildDbLayerItem
     * \since QGIS 3.0
     */
    int addMbTilesMetadataItems();

    /**
     * Common task during construction
     * \note
     * - mDbLayer /QgsSpatialiteDbLayer) will be set to nullptr
     * -> must never be deleted, may be used elsewhere
     * \since QGIS 3.0
     */
    bool onInit();

    /**
     * Resolve unset settings
     * Goal: to (semi) Automate unresolved settings when needed
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *child( int number );


    /**
     * Remove the child inside a parent
     * - logic test are done
     * \note
     *  - or are less or greater than count
     *  QList.removeAt() can (possibly) not be called from outside the class
     * \param iFrom 0 based position
     * \returns iNextChildRow row of next Child after, otherwise before ; -1 if none ; -2 if out of range
     * \see QgsSpatialiteDbInfoModel::moveLayerchild
     * \since QGIS 3.0
     */
    int removeChild( int iFrom = -1 );

    /**
     * Move the child inside a parent
     * - logic test are done
     * \note
     *  - iFrom and iTo must not be the same
     *  - or are less or greater than count
     *  QList.move() cannot be called from outside the class
     * \param iFrom 0 based position
     * \param iTo 0 based position
     * \returns true of logic test are correct
     * \see QgsSpatialiteDbInfoModel::moveLayerchild
     * \since QGIS 3.0
     */
    bool moveChild( int iFrom = -1, int iTo = -1 );

    /**
     * Returns the stored Item-Children
     * \note
     *  -
    * \returns List of Item-Type of the QgsSpatialiteDbInfoItem
    * \see mItemType
    * \see getItemTypeString()
    * \since QGIS 3.0
    */
    QList<QgsSpatialiteDbInfoItem *> getChildItems() { return mChildItems; }

    /**
     * How many children does this Item contain
     * \since QGIS 3.0
     */
    int childCount() const;

    /**
     * How many columns does this Item contain
     * \note
     *  default: mColumnSortHidden + 1
     * \since QGIS 3.0
     */
    int columnCount() const;

    /**
     * Add a child to this Item
     * \note
     *  willAddChildren and addedChildren will be emitted when bWillAddChildren=true
     *  - calling connectToChildItem to the child
     *  Using willAddChildren crashes the Application and is works without it
     * \param child Child to add
     * \param position At which position to insert child
     * \param bWillAddChildren emit willAddChildren  [default=false]
     * \see connectToChildItem
     * \since QGIS 3.0
     */
    bool addChild( QgsSpatialiteDbInfoItem *child, int position = -1, bool bWillAddChildren = false );

    /**
     * Add x-amount of children to this Item
     * \note
     *  willAddChildren and addedChildren will be emitted
     *  - calling connectToChildItem to each child
     * \param count amount of Children to add [default=1]
     * \param itemType ItemType to set [default=ItemTypeUnknown]
     * \param columns amount of Columns to add [default=-1 (takes value of  columnCount())]
     * \see connectToChildItem
     * \see columnCount()
     * \since QGIS 3.0
     */
    bool insertChildren( int position, int count = 1, SpatialiteDbInfoItemType itemType = ItemTypeUnknown, int columns = -1 );

    /**
     * Add Prepaired children to this Item
     * \note
     *  willAddChildren and addedChildren will be emitted
     *  - calling connectToChildItem to each child
     * \returns count of valid Child-Items added
     * \see connectToChildItem
     * \since QGIS 3.0
     */
    int insertPrepairedChildren();

    /**
     * Add a list of database layers to the map
     * - to fill a Map of QgsVectorLayers and/or QgsRasterLayers
     * -> can be for both QgsSpatiaLiteProvider or QgsOgr/GdalProvider
     * \note
     * - requested by User to add to main QGis
     * -> emit addDatabaseLayers for each supported Provider
     * \param saSelectedLayers formatted as 'table_name(geometry_name)'
     * \param saSelectedLayersStyles Style-Name for selected Layers
     * \param saSelectedLayersSql Extra Sql-Query for selected Layers
     * \returns amount of SelectedLayers entries
     * \see QgsSpatialiteDbInfo::addDbMapLayers
     * \since QGIS 3.0
     */
    int addSelectedDbLayers( QStringList &saSelectedLayers, QStringList &saSelectedLayersStyles, QStringList &saSelectedLayersSql );

    /**
     * Add Prepaired children to this Item
     * Type: ModelTypeLayerOrder
     * \note
     *  willAddChildren and addedChildren will be emitted
     *  - calling connectToChildItem to each child
     * \returns count of valid Child-Items added
     * \see connectToChildItem
     * \since QGIS 3.0
     */
    int handelSelectedPrepairedChildren( bool bRemoveItems = false );

    /**
     * Add Prepaired children to this Item
     * Type: ModelTypeLayerOrder
     * \note
     *  willAddChildren and addedChildren will be emitted
     *  - calling connectToChildItem to each child
     * \returns count of valid Child-Items added
     * \see connectToChildItem
     * \since QGIS 3.0
     */
    int addConnectionLayer( QgsSpatialiteDbInfoItem *childItem );

    /**
     * Add Prepaired children to this Item
     * Type: ModelTypeLayerOrder
     * \note
     *  willAddChildren and addedChildren will be emitted
     *  - calling connectToChildItem to each child
     * \returns count of valid Child-Items added
     * \see connectToChildItem
     * \since QGIS 3.0
     */
    int addPrepairedChildItem( QgsSpatialiteDbInfoItem *childItem );

    /**
     * Insert columns into Item
     * \note
     *  presently not used
     *  canditate to be removed
     * \since QGIS 3.0
     */
    bool insertColumns( int position, int columns );

    /**
     * Retrieve Parent Item
     * \note
     *  - with const
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *parent() const { return mParentItem; }

    /**
     * Retrieve Cousin-LayerItem
     * \note
     *  - without const
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *getCousinLayerItem() const { return mCousinLayerItem; }

    /**
     * Retrieve Selected LayersSql
     * \note
     *  - This will be used for the 'addDbMapLayers' logic
     * \see addSelectedDbLayers
     * \since QGIS 3.0
     */
    QString getLayerSqlQuery() const;

    /**
     * Set the created LayersSqlQuery
     * \note
     *  - This will be used for the 'addDbMapLayers' logic
     * \since QGIS 3.0
     */
    bool setLayerSqlQuery( QString sLayerSqlQuery );

    /**
     * Set Selected Style in the Layer-Item entry
     *  ModelTypeLayerOrder, ItemTypeStylesMetadata only
     * \note
     *  - This will be used for the 'addDbMapLayers' logic
     * This function must be called from the Model to be rendered properly after the change
     * \see QgsSpatialiteDbInfoModel::setSelectedStyle
     * \see QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_doubleClicked
     * \since QGIS 3.0
     */
    bool setSelectedStyle();

    /**
     * Get Selected Style in the Layer-Item entry
     *  ModelTypeLayerOrder, ItemTypeLayerOrderVector/RasterLayer only
     * \note
     *  - This will be used for the 'addDbMapLayers' logic
     * \see addSelectedDbLayers
     * \since QGIS 3.0
     */
    QString getSelectedStyle();

    /**
     * Retrieve Parent Item
     * \note
     *  - without const
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *getParent()  { return mParentItem; }

    /**
     * Retrieve Grand-Parent Item
     * \note
     *  - without const
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *getGrandParent()  { return getParent()->getParent(); }

    /**
     * Remove Children
     * \note
     *  presently not used
     *  canditate to be removed
     * \since QGIS 3.0
     */
    bool removeChildren( int position, int count );

    /**
     * Remove Columns
     * \note
     *  presently not used
     *  canditate to be removed
     * \since QGIS 3.0
     */
    bool removeColumns( int position, int columns );

    /**
     * Position of this Item in the Parent Item's Children
     * \since QGIS 3.0
     */
    int childNumber() const;

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
     * Set the active Database
     * \note
     *  - this will build the Table-Model
    * \returns QString Value of Database file
    * \see QgsSpatiaLiteSourceSelect::setQgsSpatialiteDbInfo
    * \see QgsSpatialiteDbInfoModel::createDatabase
    * \since QGIS 3.0
    */
    bool setSpatialiteDbInfo( QgsSpatialiteDbInfo *spatialiteDbInfo );
    ;

    /**
     * Collection of reasons for the Database-Layer being invalid
     * \note
     *  -> format: 'table_name(geometry_name)', Error or Warning text
     * \param bWarnings default:false return Errors, else Warnings Messages
     * \returns QMape of Strings with function name and Text of Warning/Error
     * \since QGIS 3.0
     */
    QMap<QString, QString> getSpatialiteDbInfoErrors( bool bWarnings = false ) const;

    /**
     * Retrieve Layer of Item
     * - contains Layer-Name and a QgsSpatialiteDbLayer-Pointer
     * \note
     * - child Item will receive Layer from Parent
     * -- when  getDbLoadLayers is true, getQgsSpatialiteDbLayer will load all layers
     * \returns mDbLayer of active Layer
     *  \see onInit
     * \since QGIS 3.0
     */
    QgsSpatialiteDbLayer *getDbLayer() const { return mDbLayer; }

    /**
     * Retrieve Layer of Item
     * - contains Layer-Name and a QgsSpatialiteDbLayer-Pointer
     * \note
     * - child Item will receive Layer from Parent
     * -- when  getDbLoadLayers is true, getQgsSpatialiteDbLayer will load all layers
     * \returns mDbLayer of active Layer
     *  \see onInit
     * \since QGIS 3.0
     */
    bool setDbLayer( QgsSpatialiteDbLayer *dbLayer );

    /**
     * The Spatialite Layer-Type of the Layer
     * - representing mLayerType
     *  \see setGeometryType
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfo::SpatialiteLayerType getLayerType() const { return mLayerType; }

    /**
     * The Spatialite Layer-Type of the Layer
     * - for geometries, this will be the Geometry-Type Icon
     * - a Icon representing mLayerType
     *  \see setGeometryType
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    QIcon getLayerTypeIcon() const;

    /**
     * A standard (non-typed) Group Icon
     * - for general use
     *  \see buildSpatialSubGroup
     *  \see addConnectionLayer
     * \since QGIS 3.0
     */
    QIcon getGroupIcon() const { return QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( QStringLiteral( "Group" ) ); }

    /**
     * Is this Layer-Type a Geometry or Raster that can be displaced by a Provider
     * - used within QgsSpatiaLiteSourceSelect
     *  \see onInit
     *  \see QgsSpatiaLiteSourceSelect::collectSelectedTables
     * \since QGIS 3.0
     */
    bool mIsLayerSelectable = false;

    /**
     * Is the Layer-Type a Geometry or Raster
     * - only with mLayerType
     *  \see setGeometryType
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool mIsVectorType = false;

    /**
     * Is the Layer-Type a Geometry or Raster
     * - only with mLayerType
     *  \see setGeometryType
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool mIsRasterType = false;

    /**
     * Is the Layer-Type a RasterLite2 Layer
     * - only with mLayerType
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool mIsSpatialView = false;

    /**
     * Is the Layer-Type a RasterLite2 Layer
     * - only with mLayerType
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool mIsRasterLite2 = false;

    /**
     * Is the Layer-Type a RasterLite1 Layer
     * - Without title, Abstract and copywrite Information
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool mIsRasterLite1 = false;

    /**
     * Is the Layer-Type a MbTiles Layer
     * - only with mLayerType
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool mIsMbTiles = false;

    /**
     * Is the Layer-Type a GeoPackage
     * - only with mLayerType
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool mIsGeoPackage = false;

    /**
     * Layer-Name
     * \note
     *  Vector: Layer format: 'table_name(geometry_name)'
     *  Raster: Layer format: 'coverage_name'
     * \since QGIS 3.0
     */
    QString getLayerName() const { return mLayerName; }

    /**
     * Set the Spatialite Layer-Type String
     * - representing mLayerType
     *  \see setLayerType
     * \see SpatialiteLayerTypeName
     * \since QGIS 3.0
     */
    QString getLayerTypeString() const { return mLayerTypeString; }

    /**
     * The Spatialite internal Database structure being read
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QgsSpatialiteDbInfo::SpatialMetadata dbSpatialMetadata() const { return mSpatialMetadata; }

    /**
     * The Spatialite internal Database structure being read (as String)
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString dbSpatialMetadataString() const { return mSpatialMetadataString; }

    /**
     * Returns  the enum of Item-Type of the QgsSpatialiteDbInfoItem
     * \note
     *  -
    * \returns Item-Type of the QgsSpatialiteDbInfoItem
    * \see mItemType
    * \see getItemTypeString()
    * \since QGIS 3.0
    */
    SpatialiteDbInfoItemType getItemType() const { return mItemType; }

    /**
     * Returns  the enum of Item-Type of the QgsSpatialiteDbInfoItem as String
     * \note
     *  -
    * \returns Item-Type of the QgsSpatialiteDbInfoItem
    * \see mItemType
    * \see getItemType()
    * \since QGIS 3.0
    */
    QString getItemTypeString() const { return QgsSpatialiteDbInfoItem::SpatialiteDbInfoItemTypeName( mItemType ); }

    /**
     * Returns the Index of Column 'Table' from the Header
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnTable
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int getTableNameIndex() const { return mColumnTable; }

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
     * Returns the Index of Column 'Geometry-Type' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnGeometryType
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int getGeometryTypeIndex() const { return mColumnGeometryType; }

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
     * Returns the Index of Column 'Hidden' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
    * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
    * \see mColumnSortHidden
    * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
    * \since QGIS 3.0
    */
    int getColumnSortHidden() const { return mColumnSortHidden; }
    //-----------------------------------------------------------------
    // Emulating QStandardItem
    //-----------------------------------------------------------------

    /**
     * Sets the item's data for the given \a role to the specified \a value.
     * \note
     *  - The default implementation treats Qt::EditRole and Qt::DisplayRole as referring to the same data.
     * Based on logic used in QStandardItem
     * \see data
     * \see setFlags
     * \see mItemData
     * \param column Column to set Value
     * \param value Value to set in Column
     * \param role Role-Tpe to set in Column
     * \since QGIS 3.0
    */
    bool setData( int column, const QVariant &value, int role = Qt::EditRole );

    /**
     * Returns the item's data for the given \a role, or an invalid QVariant if there is no data for the role.
     * \note
     *  - The default implementation treats Qt::EditRole and Qt::DisplayRole as referring to the same data.
     * Based on logic used in QStandardItem
     * \see data
     * \see setFlags
     * \see mItemData
     * \param column Column to retrieve Value from [default=0]
     * \since QGIS 3.0
    */
    QVariant data( int column, int role = Qt::EditRole ) const;

    /**
     * Returns the set flag for the given Item's column
     * \note
     *  - set during construction of Item
    * \param column of the Item [default=0]
    * \returns flags for the given column of the Item
    * \see QgsSpatialiteDbInfoModel::flags
    * \since QGIS 3.0
    */
    Qt::ItemFlags flags( int column = 0 ) const;

    /**
     * Sets the item flags for the item to \a flags.
     *  - The item flags determine how the user can interact with the item.
     * \note
     *  - when column < 0: all columns will be set with the given value
     * \param column Column to set Value to
     * \param flags Value to set in Column
    * \since QGIS 3.0
    */
    void setFlags( int column, Qt::ItemFlags flags );

    /**
     * Sets the item's accessible description to the string specified by \a  accessibleDescription.
     *  - The accessible description is used by assistive technologies (i.e. for users who cannot use conventional means of interaction).
     * \note
     *  - default:  [Qt::ItemIsEnabled]
     * \param column Column to retrieve Value from [default=0]
    * \since QGIS 3.0
    */
    inline bool isEnabled( int column = 0 ) const { return ( flags( column ) & Qt::ItemIsEnabled ) != 0; }

    /**
     * Sets whether the item is enabled.
     *  - If \a enabled is true, the item is enabled, meaning that the user can interact with the item; if \a enabled is false, the user cannot interact with the item.
     * \note
     *  This flag takes precedence over the other item flags; e.g. if an item is not enabled, it cannot be selected by the user, even if the Qt::ItemIsSelectable flag has been set.
     *  - default:  [Qt::ItemIsEnabled]
     * \param column Column to set Value to [default=0]
     * \param value Value to set to True or False
    * \since QGIS 3.0
    */
    void setEnabled( int column, bool value );

    /**
     * Returns whether the item can be edited by the user.
     *  -When an item is editable (and enabled), the user can edit the item by invoking one of the view's edit triggers;
     * \note
     *  - default:  [false]
     * \param column Column to retrieve Value from [default=0]
    * \since QGIS 3.0
    */
    inline bool isEditable( int column ) const { return ( flags( column ) & Qt::ItemIsEditable ) != 0; }

    /**
     * Sets whether the Item-Column is editable.
     *  - If \a editable is true, the Item-Column can be edited by the user; otherwise, the user cannot edit the item.
     *  - since the Item-Column must be enabled, setEnabled will be called if not already enabled
     * \note
     *  - How the user can edit items in a view is determined by the view's edit triggers;
     *  - default:  [false]
     *  - mConnectionsTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
     * \param column Column to set Value to [default=0]
     * \param value Value to set to True or False
     * \see setEnabled
     * \since QGIS 3.0
    */
    void setEditable( int column, bool value );

    /**
     * Returns whether the item is selectable by the user.
     *  -The default value is true
     * \note
     *  - default:  [Qt::ItemIsSelectable]
     * \param column Column to retrieve Value from [default=0]
    * \since QGIS 3.0
    */
    inline bool isSelectable( int column = 0 ) const { return ( flags( column ) & Qt::ItemIsSelectable ) != 0; }

    /**
     * Is this Layer-Type a Geometry or Raster that can be displaced by a Provider
     * - used within QgsSpatiaLiteSourceSelect
     *  \see onInit
     *  \see QgsSpatiaLiteSourceSelect::collectSelectedTables
     * \since QGIS 3.0
     */
    bool isLayerSelectable() const { return mIsLayerSelectable; }

    /**
     * Is the Layer-Type a Geometry or Raster
     * - only with mLayerType
     *  \see setGeometryType
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool isVector() const { return mIsVectorType; }

    /**
     * Is the Layer-Type a Geometry or Raster
     * - only with mLayerType
     *  \see setGeometryType
     *  \see addCommonMetadataItems
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    bool isRaster() const { return mIsRasterType; }

    /**
     * Sets whether the item is selectable
     *  - If \a selectable is true, the item can be selected by the user; otherwise, the user cannot select the item.
     * \note
     *  -   You can control the selection behavior and mode by manipulating their view properties
     * \param column Column to set Value to [default=0]
     * \param value Value to set to True or False
     * \since QGIS 3.0
    */
    void setSelectable( int column, bool value );

    /**
     * Calculate new position to Move an LayerOrder-Item within the Raster/Vector Group to
     *  - for Up/Down ; First to Last ; Last to First or a specific position inside the Parent
     *  - ModelTypeLayerOrder only, where there is no sorting
     *  - ItemTypeLayerOrderVectorLayer
     *  - ItemTypeLayerOrderRasterLayer
     * \note
     * presentPosition + (negative value) < 0 = Last Position
     * presentPosition + (positive value) >= count = First Position
     * This does NOT move the Item
     *  - For support of Up/down Buttion in Tab 'Layer Order'
     *  - Out of range values will place the Item in the First or Last position
     *  - beginMoveRows and endMoveRows are used to insure that the calling View does not become confused
     *  iMoveToRows: beginMoveRows need to know the new position at a time when the old position still exists, thus +1
     *  iMoveToChild: moveChild[QList.take] needs to know the old position (removeAt), and after removal,  position to insert
     * \param  iMoveCount IN move the Item within the Group [+/- from childNumber() ]
     * \param iMoveToChild OUT  new position needed for beginMoveRows
     * \returns iMoveToRows new position needed for beginMoveRows
     * \see moveChild
     * \see QgsSpatialiteDbInfoModel::moveLayerchild
     * \since QGIS 3.0
    */
    int moveChildPosition( int iMoveCount, int &iMoveToChild );

    /**
     * Returns the item's icon.
     * \note
     *  - default:  [Qt::DecorationRole, QIcon]
     * \param column Column to retrieve Value from [default=0]
     * \since QGIS 3.0
    */
    inline QIcon icon( int column = 0 ) const { return qvariant_cast<QIcon>( data( column, Qt::DecorationRole ) ); }

    /**
     * Sets the item's icon to the \a icon specified.
     * \note
     *  - default:  [Qt::DecorationRole, QIcon]
     * \param column Column to set Value to [default=0]
     * \param icon Value to set in Column
     * \since QGIS 3.0
    */
    inline void setIcon( int column,  const QIcon &icon )  { setData( column, icon, Qt::DecorationRole ); }

    /**
     * Sets the item's text to the \a text specified.
     * \note
     *  - default:  [Qt::EditRole, Edit]
     *  - when column < 0: all columns will be set with the given value
     * \param column Column to set Value to [default=0]
     * \param sText Value to set in Column
     * \since QGIS 3.0
    */
    inline void setText( int column, const QString &sText, int role = Qt::EditRole );

    /**
     * Returns the item's text. This is the text that's presented to the user in a view.
     * \note
     *  - default:  [Qt::EditRole, Edit]
     * \param column Column to retrieve Value from [default=0]
     * \since QGIS 3.0
    */
    inline QString text( int column = 0, int role = Qt::EditRole ) const { return qvariant_cast<QString>( data( column, role ) ); }

    /**
     * Sets the item's text Background to the color specified.
     * \note
     *  - default:  [lightyellow, #FFFFE0, rgb(255, 255, 224)]
     * \param column Column to set Value to [default=0]
     * \param value QColor background value to set in Column
     * \since QGIS 3.0
    */
    inline void setBackground( int column = 0, QColor value = QColor( "lightyellow" ) )  { setData( column, value, Qt::BackgroundRole ); }

    /**
     * Sets the item's text Background to the color specified.
     * \note
     *  - default:  [black, #000000, rgb(0, 0, 0)]
     * \param column Column to set Value to [default=0]
     * \param value QColor foreground value to set in Column
     * \since QGIS 3.0
    */
    inline void setForeground( int column = 0, QColor value = QColor( "black" ) )  { setData( column, value, Qt::ForegroundRole ); }

    /**
     * Sets the item's text Alignment to the Alignment specified.
     * \note
     *  - default:  [Qt::AlignLeft, Browse] can be [Qt::AlignCenter,Qt::AlignRight,Qt::AlignJustify etc.]
     * \param column Column to set Value to [default=0]
     * \param sText Value to set in Column
     * \since QGIS 3.0
    */
    inline void setAlignment( int column = 0, int value = Qt::AlignLeft )  { setData( column, value, Qt::TextAlignmentRole ); }

    /**
     * Sets the item's tooltip to the string specified by \a toolTip.
     * \note
     *  - default:  [Qt::ToolTipRole]
     * \param column Column to set Value to [default=0]
     * \param sText Value to set in Column
     * \since QGIS 3.0
    */
    inline void setToolTip( int column, const QString &sText )  { setData( column, sText, Qt::ToolTipRole ); }

    /**
     * Returns the item's tooltip.
     * \note
     *  - default:  [Qt::ToolTipRole]
     * \param column Column to retrieve Value from [default=0]
     * \since QGIS 3.0
    */
    inline QString toolTip( int column = 0 ) const { return qvariant_cast<QString>( data( column, Qt::ToolTipRole ) ); }

    /**
     * Sets the item's status tip to the string specified by \a statusTip.
     * \note
     *  -  [Qt::StatusTipRole]
     * \param column Column to set Value to [default=0]
     * \param sText Value to set in Column
     * \since QGIS 3.0
    */
    inline void setStatusTip( int column, const QString &sText )  { setData( column, sText, Qt::StatusTipRole ); }

    /**
     * Returns the item's tooltip.
     * \note
     *  -  [Qt::StatusTipRole]
     * \param column Column to retrieve Value from [default=0]
     * \since QGIS 3.0
    */
    inline QString statusTip( int column = 0 ) const { return qvariant_cast<QString>( data( column, Qt::StatusTipRole ) ); }

    /**
     * Returns the row where the item is located in its parent's child table
     *  -   -1 if the item has no parent.
     * \note
     *  -  returns 'first' value from result of position() pair
     * \since QGIS 3.0
    */
    inline int row() const { return position().first; }

    /**
     *  Returns the column where the item is located in its parent's child table,
     *  -   -1 if the item has no parent.
     * \note
     *  -  returns 'second' value from result of position() pair
     * \since QGIS 3.0
    */
    inline int column() const { return position().second; }

    /**
     *  Total amount of Layers in Database
     * \note
     * - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int getTableCounter() { return mTableCounter; }

    /**
     *  Total amount of non-SpatialTables/Views in Database
     * \note
     *  - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int getNonSpatialTablesCounter() { return mNonSpatialTablesCounter; }

    /**
     *  Total amount of Styles in Database
     * \note
     *  - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int getStylesCounter() { return mStylesCounter; }

    /**
     *  Total amount of Srid's in Database
     * \note
     *  - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int getSridInfoCounter() { return mSridInfoCounter; }

    /**
     *  Set Total amount of Layers in Database
     * - Used mainly to set in value in mModelRootItem
     * \note
     * - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    void setTableCounter( int iTableCounter ) { mTableCounter = iTableCounter; }

    /**
     *  Set Total amount of non-SpatialTables/Views  in Database
     * - Used mainly to set in value in mModelRootItem
     * \note
     * - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    void setNonSpatialTablesCounter( int iNonSpatialTablesCounter ) { mNonSpatialTablesCounter = iNonSpatialTablesCounter; }

    /**
     *  Set Total amount of Styles  in Database
     * - Used mainly to set in value in mModelRootItem
     * \note
     * - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    void setStylesCounter( int iStylesCounter ) { mStylesCounter = iStylesCounter; }

    /**
     *  Set Total amount of Srids  in Database
     * - Used mainly to set in value in mModelRootItem
     * \note
     * - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    void setSridInfoCounter( int iSridInfoCounter ) { mSridInfoCounter = iSridInfoCounter; }

    /**
     * Store the Item-Children
     * \note
     *
     * \since QGIS 3.0
    */
    QList<QgsSpatialiteDbInfoItem *> getPrepairedChildItems() { return mPrepairedChildItems; }

    /**
     * Is this Item considered valid
     * \note
     *  - dependent on Item Type
     *  - making checking after creation possible
     * \since QGIS 3.0
     */
    bool isValid() const { return mIsItemValid; }

  signals:
    // - Based on QgsLayerTreeModel / QgsLayerTreeNode
    //! Emitted when one or more nodes will be added to a node within the tree
    void willAddChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes have been added to a node within the tree
    void addedChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes will be removed from a node within the tree
    void willRemoveChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes has been removed from a node within the tree
    void removedChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo );
    //! Emitted when check state of a node within the tree has been changed
    void visibilityChanged( QgsSpatialiteDbInfoItem *node );
    //! Emitted when a custom property of a node within the tree has been changed or removed
    void customPropertyChanged( QgsSpatialiteDbInfoItem *node, const QString &key );
    //! Emitted when the collapsed/expanded state of a node within the tree has been changed
    // void expandedChanged( QgsSpatialiteDbInfoItem *node, bool expanded );

  private:

    /**
     * Set Cousin-LayerItem
     * \note
     * When a LayerOrder-LayerItem is created from a Connections-Layer
     * -> in 'addConnectionLayer'
     * -> both must be isLayerSelectable() [i.e. Root of QgsSpatialiteDbLayer-Item]
     * The creating Connections-Layer must be stored so the latest version
     *  version of the (possibly create) Sql-Query can be retrieved.
     * This will be used for the 'addDbMapLayers' logic
     * \since QGIS 3.0
     */
    bool setCousinLayerItem( QgsSpatialiteDbInfoItem *cousinLayerItem );

    /**
     * The Item-Type
     * - Item-Type being used
     * \since QGIS 3.0
     *
     */
    SpatialiteDbInfoItemType mItemType = SpatialiteDbInfoItemType::ItemTypeUnknown;

    /**
     * The Modul-Type
     * Modul-Type being used
     * \note
     *  ModelTypeConnections
     *  ModelTypeLayerOrder
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelType mModelType = QgsSpatialiteDbInfoModel::ModelTypeUnknown;

    /**
     * The Spatialite Layer-Type of the Layer
     * - representing mLayerType
     * \note
     *  - The Layer-Type of a set mDblayer, otherwise SpatialiteUnknown
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfo::SpatialiteLayerType mLayerType = QgsSpatialiteDbInfo::SpatialiteUnknown;

    /**
     * Set the Spatialite Layer-Type String
     * - representing mLayerType
     *  \see setLayerType
     * \see SpatialiteLayerTypeName
     * \since QGIS 3.0
     */
    QString mLayerTypeString = QString( "SpatialiteUnknown" );

    /**
     * Layer-Name
     * \note
     *  Vector: Layer format: 'table_name(geometry_name)'
     *  Raster: Layer format: 'coverage_name'
     * \since QGIS 3.0
     */
    QString mLayerName = QString();

    /**
     * The Spatialite internal Database structure being read
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QgsSpatialiteDbInfo::SpatialMetadata mSpatialMetadata = QgsSpatialiteDbInfo::SpatialUnknown;

    /**
     * The Spatialite internal Database structure being read (as String)
     * \note
     *  -  based on result of CheckSpatialMetaData
     * \see getSniffDatabaseType
    * \since QGIS 3.0
    */
    QString mSpatialMetadataString = QString( "SpatialUnknown" );

    /**
     * Returns the Index of Column 'Hidden' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
     * - must be the last, since (mColumnSortHidden+1) == columnCount
     * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
     * \see getColumnSortHidden
     * \see QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
     * \since QGIS 3.0
    */
    int mColumnSortHidden = -1;

    /**
     * Returns the Index of Column 'Table' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
     * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
     * \see getTableNameIndex
     * \see getTableName
     * \since QGIS 3.0
    */
    int mColumnTable = -1;

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
    int mColumnGeometryName = -1;

    /**
     * Returns the Index of Column 'Sql' from the selected Item
     * \note
     *  - set during construction. Avoid manual use of numeric value in code
     * \returns index-number of column being used in QgsSpatiaLiteSourceSelect
     * \see getSqlQueryIndex
     * \see getSqlQuery
     * \since QGIS 3.0
    */
    int mColumnSql = -1;

    /**
     * Store the Item-Children
     * \note
     *
     * \since QGIS 3.0
    */
    QList<QgsSpatialiteDbInfoItem *> mChildItems;

    /**
     * Store the Item-Children
     * \note
     *
     * \since QGIS 3.0
    */
    QList<QgsSpatialiteDbInfoItem *> mPrepairedChildItems;

    /**
     * Store the Item-Data
     * \note
     *  - for RootItem, this will contain the Column-Names
     *  - Text [Qt::EditRole]
     *  - Icon  [Qt::DecorationRole]
     *  - Flags  [255: Qt::UserRole - 1 ]
     * The default implementation treats Qt::EditRole and Qt::DisplayRole as referring to the same data.
     * \since QGIS 3.0
    */
    QVector<QgsSpatialiteDbInfoData> mItemData;

    /**
     * Store the Item-Parent
     * \note
     *  Set during construction
     * \since QGIS 3.0
    */
    QgsSpatialiteDbInfoItem *mParentItem = nullptr;

    /**
     * Retrieve Cousin-LayerItem
     * \note
     *  - without const
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *mCousinLayerItem = nullptr;

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
     * The active Layer
     * - being read by the Provider
     * \note
     * - isLayerValid() return true if everything is considered correct
     * \see QgsSpatialiteDbLayer::isLayerValid
     * \see setDbLayer
     * \since QGIS 3.0
     */
    QgsSpatialiteDbLayer *mDbLayer = nullptr;

    /**
     * Helper Function to implement row() and column()
     * \note
     *  - Internal function
     *  - Emulating QStandardItem
     * \since QGIS 3.0
    */
    QPair<int, int> position() const;
    int mColumnNr = -1;

    /**
     *  Total amount of Layers in Database
     * \note
     *  - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int mTableCounter = 0;

    /**
     *  Total amount of non-SpatialTables/Views in Database
     * \note
     *  - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int mNonSpatialTablesCounter = 0;

    /**
     * Count of used Srid's in  the Database
     * \note
     *  - Used to build the SridGroup
    * \see QgsSpatialiteDbInfo::getDbSridInfo
    * \since QGIS 3.0
    */
    int mSridInfoCounter = 0;

    /**
     *  Total amount of Styles in Database
     * \note
     *  - Used to show the total amount of Layers
     * - calculated only ItemTypeDb from onInit, retrieved from parent for others
    * \see sniffSpatialiteDbInfo
    * \since QGIS 3.0
    */
    int mStylesCounter = 0;

    /**
     * Sets the mTableCounter
     * \note
     * - Used to show the total amount of Layers
     * - runs only when ItemTypeDb from onInit
     * \since QGIS 3.0
    */
    bool sniffSpatialiteDbInfo();

    /**
     * Add Items to display the Database
     * \note
     * - called during creation of Model
     * -> removed during first Database loading
     * \returns count of Child-Items added
     * \since QGIS 3.0
    */
    int buildDbInfoItem();

    /**
     * Add Items to display the Layer
     * \note
     * - called during creation of Model
     * -> removed during first Database loading
     * \returns count of Child-Items added
     * \see addMbTilesMetadataItems
     * \see addCommonMetadataItems
     * \since QGIS 3.0
    */
    int buildDbLayerItem();

    /**
     * Add Items to display the Layer
     * \note
     * - called during creation of Model
     * -> removed during first Database loading
     * \returns count of Child-Items added
     * \see addMbTilesMetadataItems
     * \see addCommonMetadataItems
     * \since QGIS 3.0
    */
    int buildLayerOrderItem();

    /**
     * Add Items to display as HelpText
     * \note
     * - called during creation of Model
     * -> removed during first Database loading
     * \returns count of Child-Items added
     * \since QGIS 3.0
    */
    int buildHelpItem();

    /**
     * Add Items to display as MetadataText
     * \note
     * - called during creation of Item with a Layer
     * \returns count of Child-Items added
     * \since QGIS 3.0
    */
    int buildMetadataRootItem();

    /**
     * Add Items to display as MetadataText
     * \note
     * - called during creation of Item with a Layer
     * \returns count of Child-Items added
     * \since QGIS 3.0
    */
    int buildMetadataGroupItem( QString sGroupName, QStringList saParmValues );

    /**
     * Add Items to display as MetadataText
     * \note
     * - called during creation of Item with a Layer
     * \returns count of Child-Items added
     * \since QGIS 3.0
    */
    int buildStylesSubGroups( );

    /**
     * Add Items to display as MetadataText
     * \note
     * - called during creation of Item with a Layer
     * \returns count of Child-Items added
     * \since QGIS 3.0
    */
    int buildStylesGroupItem( QString sGroupName, QStringList saParmValues );

    /**
     * Adding and filling SpatialTab Sub-Groups
     ** within the [SpatialView,SpatialView,GeoPackageVector,GdalFdoOgr]
     * \note
     *  When: 'table_name' contains at least 2 '_' (such as 'berlin_streets_1650')
     *  - assume it belongs to a group of 'berlin_streets' for QgsSpatialiteDbInfoItem
     * \param sKey table name
     * \param sValue Name of Sub-Group
     * \returns count of Child-Items added
     * \see sniffSpatialiteDbInfo()
     * \see QgsSpatialiteDbInfo::dbNonSpatialTablesCount
     * \see createGroupLayerItem
     * \see QgsSpatialiteDbInfo::getDbLayersType
     * \see addTableItemMap
     * \since QGIS 3.0
    */
    int buildSpatialSubGroups();

    /**
     * Adding and filling SpatialTab Sub-Group
     ** within the [SpatialView,SpatialView,GeoPackageVector,GdalFdoOgr]
     * \note
     *  When: 'table_name' contains at least 2 '_' (such as 'berlin_streets_1650')
     *  - assume it belongs to a group of 'berlin_streets' for QgsSpatialiteDbInfoItem
     * \param SubGroupName Name of the subgroup to be added
     * \param saLayerNames List of SpatialLayers to be added Sub-Group
     * \returns count of Items added
     * \see sniffSpatialiteDbInfo()
     * \see QgsSpatialiteDbInfo::dbNonSpatialTablesCount
     * \see createGroupLayerItem
     * \see QgsSpatialiteDbInfo::getDbLayersType
     * \see QgsSpatialiteDbInfo::getListGroupNames
     * \see QgsSpatialiteDbInfo::prepareGroupLayers
     * \see addTableItemMap
     * \since QGIS 3.0
    */
    int buildSpatialSubGroup( QString SubGroupName, QStringList saLayerNames );

    /**
     * Adding and filling SpatialTab Sub-Groups
     ** within the [SpatialView,SpatialView,GeoPackageVector,GdalFdoOgr]
     * \note
     *  When: 'table_name' contains at least 2 '_' (such as 'berlin_streets_1650')
     *  - assume it belongs to a group of 'berlin_streets' for QgsSpatialiteDbInfoItem
     * \param sKey table name
     * \param sValue Name of Sub-Group
     * \returns count of Items added
     * \see sniffSpatialiteDbInfo()
     * \see QgsSpatialiteDbInfo::dbNonSpatialTablesCount
     * \see createGroupLayerItem
     * \see QgsSpatialiteDbInfo::getDbLayersType
     * \see addTableItemMap
     * \since QGIS 3.0
    */
    int buildNonSpatialSubGroups();

    /**
     * Adding and filling NonSpatialTables Sub-Groups
     *  or an NonSpatialTables-Item that does not belong to a Sub-Group
     * \note
     * - called from createGroupLayerItem
     * - calls buildNonSpatialTablesSubGroups for NonSpatialTables-Items that belong to a Sub-Group
     * - calls buildNonSpatialTable for each NonSpatialTables-Item that does not belong to a Sub-Group
     * \returns count of Child-Items added
     * \see createGroupLayerItem
     * \see buildNonSpatialTablesSubGroups
     * \see buildNonSpatialTable
     * \since QGIS 3.0
    */
    int buildNonSpatialTables();

    /**
     * Add Sub-Groups or an NonSpatialTables-Item that does not belong to a Sub-Group
     * \note
     * - called from buildNonSpatialTables
     * - calls buildNonSpatialTablesSubGroup when Sub-Groups exist
     * - calls buildNonSpatialTable for each NonSpatialTables-Item that does not belong to a Sub-Group
     *  TableTypes structure
     *  - field 0=TableType
     *  - field 1=GroupName
     *  - field 2=ParentGroupName
     *  - field 3=GroupSort
     *  - field 4=TableName  (added in buildNonSpatialTablesGroups)
     * \param sGroupInfo table name
     * \param saTableTypeInfo List of saTableTypeInfo to add to the Sub-Group
     * \returns count of Child-Items added
     * \see buildNonSpatialTables
     * \see buildNonSpatialTablesSubGroup
     * \see buildNonSpatialTable
     * \since QGIS 3.0
    */
    int buildNonSpatialTablesSubGroups( QString sGroupInfo, QStringList saTableTypeInfo );

    /**
     * Add Items to display as HelpText
     * \note
     * - called from  buildNonSpatialTablesSubGroups
     * - calls buildNonSpatialTable for each NonSpatialTables-Item
     *  TableTypes structure
     *  - field 0=TableType
     *  - field 1=GroupName
     *  - field 2=ParentGroupName
     *  - field 3=GroupSort
     *  - field 4=TableName  (added in buildNonSpatialTablesGroups
     * \param sGroupInfo table name
     * \param saListValues Name of Sub-Group
     * \returns count of Child-Items added
     * \see buildNonSpatialTablesSubGroups
     * \since QGIS 3.0
    */
    int buildNonSpatialTablesSubGroup( QString sGroupInfo, QStringList saTableTypeInfo );

    /**
     * Add NonSpatialTables-Item within the main NonSpatialTables group
     * \note
     * - called from buildNonSpatialTablesSubGroups when Item is within a Sub-Group
     * - called from buildNonSpatialTables when Item does not belong to any Sub-Group
     *  TableTypes structure
     *  - field 0=TableType
     *  - field 1=GroupName
     *  - field 2=ParentGroupName
     *  - field 3=GroupSort
     *  - field 4=TableName  (added in buildNonSpatialTablesGroups
     * \returns count of Child-Items added
     * \see buildNonSpatialTables
     * \see buildNonSpatialTablesSubGroup
     * \since QGIS 3.0
    */
    int buildNonSpatialTable( QString sTableTypeInfo, QStringList saParmValues );

    /**
     * Adding and filling SpatialRefSysAux Group
     *  or an NonSpatialTables-Item that does not belong to a Sub-Group
     * \note
     * - called from createGroupLayerItem
     * - calls buildNonSpatialTablesSubGroups for NonSpatialTables-Items that belong to a Sub-Group
     * - calls buildNonSpatialTable for each NonSpatialTables-Item that does not belong to a Sub-Group
     * \returns count of Child-Items added
     * \see createGroupLayerItem
     * \see buildNonSpatialTablesSubGroups
     * \see buildNonSpatialTable
     * \since QGIS 3.0
    */
    int buildSpatialRefSysAuxGroup();

    /**
     * Add SpatialRefSysAux-Item within the main NonSpatialTables group
     * \note
     * - called from buildNonSpatialTablesSubGroups when Item is within a Sub-Group
     * - called from buildNonSpatialTables when Item does not belong to any Sub-Group
     *  TableTypes structure
     *  - field 0=TableType
     *  - field 1=GroupName
     *  - field 2=ParentGroupName
     *  - field 3=GroupSort
     *  - field 4=TableName  (added in buildNonSpatialTablesGroups
     * \returns count of Child-Items added
     * \see buildNonSpatialTables
     * \see buildNonSpatialTablesSubGroup
     * \since QGIS 3.0
    */
    int buildSpatialRefSysAuxSubGroup( QString sTableTypeInfo, QStringList saParmValues );

    /**
     * Retrieve the  Layer-Type entries
     * \note
     *  Counter will be set for Items added
     * \param layerType to be retrieved from SpatialiteDbInfo
     * \returns true if a QgsSpatialiteDbInfoItem has been added to mPrepairedChildItems
     * \see mTableCounter
     * \see mNonSpatialTablesCounter
     * \see mPrepairedChildItems
     * \since QGIS 3.0
     */
    bool createGroupLayerItem( QgsSpatialiteDbInfo::SpatialiteLayerType layerType = QgsSpatialiteDbInfo::SpatialiteUnknown );

    /**
     * Create and fill Group(s) for Errors/Warnings
     *  - collected by QgsSpatialiteDbInfo durring creation
     * \note
     *  information only
     * \returns true if any errors or warning exist
     * \see getSpatialiteDbInfoErrors
     * \see mNonSpatialTablesCounter
     * \see mPrepairedChildItems
     * \since QGIS 3.0
     */
    bool createErrorWarningsGroups();

    /**
     * Is this Item considered valid
     * \note
     *  - dependent on Item Type
     *  - making checking after creation possible
     * \since QGIS 3.0
     */
    bool mIsItemValid = false;

};
#endif // QGSSPATIALITEDBINFOMODEL_H
