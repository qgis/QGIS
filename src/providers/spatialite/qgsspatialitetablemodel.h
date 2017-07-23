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
#if 0
    //! Adds entry for one database table to the model
    void addTableEntryLayer( const QString &type, const QString &tableName, const QString &geometryColName, const QString &sql );
    //! Sets the SQLite DB full path
    void setSqliteDb( const QString &dbName )
    {
      mSqliteDb = dbName;
    }

    /** Sets one or more geometry types to a row. In case of several types, additional rows are inserted.
       This is for tables where the type is detected later by thread*/
    void setGeometryTypesForTable( const QString &table, const QString &attribute, const QString &type );
#endif
    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql );

    //! Returns the number of tables in the model
    int tableCount() const
    {
      return mTableCount;
    }
    //! Returns the Column 'Table' form the selected Item
    int getColumnSortHidden() const { return i_field_sort_hidden; }
    //! Returns the Column 'Table' form the selected Item
    int getTableNameIndex() const { return i_field_table; }
    QString getTableName( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), i_field_table ) )->text();
    }
    //! Returns the Column 'Geometry column' form the selected Item
    int getGeometryNameIndex() const { return i_field_geometry_name; }
    QString getGeometryName( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), i_field_geometry_name ) )->text();
    }
    //! Returns the Column 'Geometry column' form the selected Item
    int getGeometryTypeIndex() const { return i_field_geometry_type; }
    QString getGeometryType( const QModelIndex &index ) const
    {
      return itemFromIndex( index.sibling( index.row(), i_field_geometry_type ) )->text();
    }
    //! Returns the Column 'Sql' form the selected Item
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
    void addTableEntryLayer( SpatialiteDbLayer *dbLayer );

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
    QStandardItem *mDbRootItem = nullptr;
    bool mLoadGeometrylessTables = false;
};
