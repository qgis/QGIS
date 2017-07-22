/***************************************************************************
                         qgsspatialitetablemodel.cpp  -  description
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

#include "qgsspatialitetablemodel.h"
#include "qgsapplication.h"
#include "qgsdataitem.h" // for icons

QgsSpatiaLiteTableModel::QgsSpatiaLiteTableModel(): QStandardItemModel(), mTableCount( 0 ), mDbRootItem( nullptr ), mLoadGeometrylessTables( false )
{
  QStringList headerLabels;
  headerLabels << tr( "Table" );
  i_field_table = 0;
  headerLabels << tr( "Geometry column" );
  i_field_geometry_name = i_field_table + 1;
  headerLabels << tr( "Geometry-Type" );
  i_field_geometry_type = i_field_geometry_name + 1;
  headerLabels << tr( "Sql" );
  i_field_sql = i_field_geometry_type + 1;
  setHorizontalHeaderLabels( headerLabels );
}
void QgsSpatiaLiteTableModel::setSqliteDb( SpatialiteDbInfo *spatialiteDbInfo, bool loadGeometrylessTables )
{
  if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbValid() ) )
  {
    if ( mSpatialiteDbInfo )
    {
      if ( mSpatialiteDbInfo->getQSqliteHandle() )
      {
        // TODO QgsSqliteHandle::closeDb( mSpatialiteDbInfo->getQSqliteHandle() );
        mSpatialiteDbInfo = nullptr;
      }
      else
      {
        delete spatialiteDbInfo;
        mSpatialiteDbInfo = nullptr;
      }
      mTableCount = 0;
      if ( mDbRootItem )
      {
        delete mDbRootItem;
        mDbRootItem = nullptr;
      }
    }
    mSpatialiteDbInfo = spatialiteDbInfo;
    mTableCounter = 0;
    mLoadGeometrylessTables = loadGeometrylessTables;
    // mLoadGeometrylessTables = true;
    addTableEntryTypes( );
  }
}
QList < QStandardItem * > QgsSpatiaLiteTableModel::createLayerTypeEntry( SpatialiteDbInfo::SpatialiteLayerType layerType, int amountLayers )
{
  QString sTypeName = SpatialiteDbInfo::SpatialiteLayerTypeName( layerType );
  QIcon iconType = SpatialiteDbInfo::SpatialiteLayerTypeIcon( layerType );
  int i_typeCount = amountLayers;
  QString sLayerText = QString( "%1 Layers" ).arg( i_typeCount );
  if ( i_typeCount == 1 )
  {
    sLayerText = QString( "%1 Layer" ).arg( i_typeCount );
  }
  QStandardItem *tbTypeItem = new QStandardItem( iconType, sTypeName );
  tbTypeItem->setFlags( Qt::ItemIsEnabled );
  QStandardItem *tbLayersItem = new QStandardItem( sLayerText );
  tbLayersItem->setFlags( Qt::ItemIsEnabled );
  //  Ever Columns must be filled (even if empty)
  QStandardItem *emptyItem_01 = new QStandardItem( );
  emptyItem_01->setFlags( Qt::ItemIsEnabled );
  QStandardItem *emptyItem_02 = new QStandardItem( );
  emptyItem_02->setFlags( Qt::ItemIsEnabled );
  // this must be done in the same order as the Header
  QList < QStandardItem *> tbItemList;
  tbItemList.push_back( tbTypeItem );
  tbItemList.push_back( emptyItem_01 );
  tbItemList.push_back( emptyItem_02 );
  tbItemList.push_back( tbLayersItem );
  return tbItemList;
}
void QgsSpatiaLiteTableModel::addRootEntry( )
{
  if ( ( mSpatialiteDbInfo ) && ( mSpatialiteDbInfo->isDbValid() ) )
  {
    mTableCounter = 0;
    mTableCounter += mSpatialiteDbInfo->dbSpatialTablesLayersCount();
    mTableCounter += mSpatialiteDbInfo->dbSpatialViewsLayersCount();
    mTableCounter += mSpatialiteDbInfo->dbVirtualShapesLayersCount();
    mTableCounter += mSpatialiteDbInfo->dbRasterLite2LayersCount();
    mTableCounter += mSpatialiteDbInfo->dbRasterLite1LayersCount();
    mTableCounter += mSpatialiteDbInfo->dbMBTilesLayersCount();
    mTableCounter += mSpatialiteDbInfo->dbGeoPackageLayersCount();
    mTableCounter += mSpatialiteDbInfo->dbFdoOgrLayersCount();
    mTableCounter += mSpatialiteDbInfo->dbTopologyExportTablesCount();
    //is there already a root item ?
    QStandardItem *dbItem = nullptr;
    // Only searches top level items
    QList < QStandardItem * >dbItems = findItems( mSpatialiteDbInfo->getFileName(), Qt::MatchExactly, 0 );
    //there is already an item
    if ( !dbItems.isEmpty() )
    {
      dbItem = dbItems.at( 0 );
    }
    else  //create a new toplevel item
    {
      QStandardItem *dbTypeItem = new QStandardItem( mSpatialiteDbInfo->getSpatialMetadataIcon(), mSpatialiteDbInfo->dbSpatialMetadataString() );
      dbTypeItem->setFlags( Qt::ItemIsEnabled );
      QStandardItem *layersItem = new QStandardItem( QString( "%1 Layers" ).arg( mTableCounter ) );
      layersItem->setFlags( Qt::ItemIsEnabled );
      QString sProvider = QStringLiteral( "QgsSpatiaLiteProvider" );
      QString sInfoText = "";
      switch ( mSpatialiteDbInfo->dbSpatialMetadata() )
      {
        case SpatialiteDbInfo::SpatialiteFdoOgr:
          sProvider = QStringLiteral( "QgsOgrProvider" );
          sInfoText = "The 'SQLite' Driver is NOT active and cannot be displayed";
          if ( mSpatialiteDbInfo->hasDbFdoOgrDriver() )
          {
            sInfoText = "The 'SQLite' Driver is active and can be displayed";
          }
          break;
        case SpatialiteDbInfo::SpatialiteGpkg:
          sProvider = QStringLiteral( "QgsOgrProvider" );
          sInfoText = "The 'GPKG' Driver is NOT active and cannot be displayed";
          if ( mSpatialiteDbInfo->hasDbGdalGeoPackageDriver() )
          {
            sInfoText = "The 'GPKG' Driver is active and can be displayed";
          }
          break;
        case SpatialiteDbInfo::SpatialiteMBTiles:
          sProvider = QStringLiteral( "QgsGdalProvider" );
          sInfoText = "The 'MBTiles' Driver is NOT active and cannot be displayed";
          if ( mSpatialiteDbInfo->hasDbGdalMBTilesDriver() )
          {
            sInfoText = "The 'MBTiles' Driver is active and can be displayed";
          }
          break;
        case SpatialiteDbInfo::SpatialiteLegacy:
          if ( mSpatialiteDbInfo->dbRasterLite1LayersCount() > 0 )
          {
            sProvider = QStringLiteral( "QgsGdalProvider" );
            sInfoText = "The 'RASTERLITE' Driver is NOT active and cannot be displayed";
            if ( mSpatialiteDbInfo->hasDbGdalRasterLite1Driver() )
            {
              sInfoText = "The 'RASTERLITE' Driver is active and can be displayed";
            }
          }
          break;
        default:
          break;
      }
      QStandardItem *providerItem = new QStandardItem( sProvider );
      providerItem->setFlags( Qt::ItemIsEnabled );
      QStandardItem *textInfoItem = new QStandardItem( sInfoText );
      textInfoItem->setFlags( Qt::ItemIsEnabled );
      QList < QStandardItem * >rootItemList;
      rootItemList.push_back( dbTypeItem );
      rootItemList.push_back( layersItem );
      rootItemList.push_back( providerItem );
      rootItemList.push_back( textInfoItem );
      // dbRootItem->appendRow( rootItemList );
      // this must be done in the same order as the Header
      dbItem = new QStandardItem( mSpatialiteDbInfo->getSpatialMetadataIcon(), mSpatialiteDbInfo->getFileName() );
      dbItem->setFlags( Qt::ItemIsEnabled );
      invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), dbItem );
      dbItems = findItems( mSpatialiteDbInfo->getFileName(), Qt::MatchExactly, 0 );
      //there is already an item
      if ( !dbItems.isEmpty() )
      {
        mDbRootItem = dbItems.at( 0 );
        QStandardItem *dbMetaDataItem = nullptr;
        QIcon iconType = mSpatialiteDbInfo->getSpatialMetadataIcon();
        QStandardItem *tbTypeItem = new QStandardItem( iconType, "Metadata" );
        tbTypeItem->setFlags( Qt::ItemIsEnabled );
        QStandardItem *emptyItem_01 = new QStandardItem( );
        emptyItem_01->setFlags( Qt::ItemIsEnabled );
        //  Ever Columns must be filled (even if empty)
        QStandardItem *emptyItem_02 = new QStandardItem( );
        emptyItem_02->setFlags( Qt::ItemIsEnabled );
        QStandardItem *emptyItem_03 = new QStandardItem( );
        emptyItem_03->setFlags( Qt::ItemIsEnabled );
        // this must be done in the same order as the Header
        QList < QStandardItem *> tbItemList;
        tbItemList.push_back( tbTypeItem );
        tbItemList.push_back( emptyItem_01 );
        tbItemList.push_back( emptyItem_02 );
        tbItemList.push_back( emptyItem_03 );
        mDbRootItem->appendRow( tbItemList );
        dbItems = findItems( "Metadata", Qt::MatchExactly | Qt::MatchRecursive, 0 );
        if ( !dbItems.isEmpty() )
        {
          dbMetaDataItem = dbItems.at( 0 );
        }
        dbMetaDataItem->appendRow( rootItemList );
        qDebug() << QString( "QgsSpatiaLiteTableModel::addRootEntry -01- mDbRootItem[%1] " ).arg( mSpatialiteDbInfo->getFileName() );
        if ( mSpatialiteDbInfo->dbSpatialTablesLayersCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::SpatialTable, mSpatialiteDbInfo->dbSpatialTablesLayersCount() );
          // mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
          mDbRootItem->appendRow( tbItemList );
        }
        if ( mSpatialiteDbInfo->dbSpatialViewsLayersCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::SpatialView, mSpatialiteDbInfo->dbSpatialViewsLayersCount() );
          // mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
          mDbRootItem->appendRow( tbItemList );
        }
        if ( mSpatialiteDbInfo->dbVirtualShapesLayersCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::VirtualShape, mSpatialiteDbInfo->dbSpatialViewsLayersCount() );
          mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
        }
        if ( mSpatialiteDbInfo->dbRasterLite2LayersCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::RasterLite1, mSpatialiteDbInfo->dbRasterLite2LayersCount() );
          mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
        }
        if ( mSpatialiteDbInfo->dbRasterLite1LayersCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::RasterLite1, mSpatialiteDbInfo->dbRasterLite1LayersCount() );
          mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
        }
        if ( mSpatialiteDbInfo->dbMBTilesLayersCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::MBTilesTable, mSpatialiteDbInfo->dbMBTilesLayersCount() );
          mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
        }
        if ( mSpatialiteDbInfo->dbGeoPackageLayersCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::GeoPackageVector, mSpatialiteDbInfo->dbGeoPackageLayersCount() );
          mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
        }
        if ( mSpatialiteDbInfo->dbTopologyExportTablesCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::TopopogyExport, mSpatialiteDbInfo->dbTopologyExportTablesCount() );
          mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
        }
        if ( mSpatialiteDbInfo->dbFdoOgrLayersCount() > 0 )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::TopopogyExport, mSpatialiteDbInfo->dbFdoOgrLayersCount() );
          mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
        }
        if ( ( mLoadGeometrylessTables ) && ( mSpatialiteDbInfo->dbNonSpatialTablesCount() > 0 ) )
        {
          QList < QStandardItem *> tbItemList = createLayerTypeEntry( SpatialiteDbInfo::NonSpatialTables, mSpatialiteDbInfo->dbNonSpatialTablesCount() );
          // mDbRootItem->insertRow( dbItem->rowCount(), tbItemList );
          mDbRootItem->appendRow( tbItemList );
        }
      }
    }
  }
}
void QgsSpatiaLiteTableModel::addTableEntryTypes( )
{
  addRootEntry( );
  QMap<QString, QString> mapLayers;
  if ( mSpatialiteDbInfo->dbVectorLayersCount() > 0 )
  {
    // - SpatialTables, SpatialViews and VirtualShapes [from the vector_layers View]
    // -- contains: Layer-Name as 'table_name(geometry_name)', Geometry-Type (with dimension) and Srid
    if ( mSpatialiteDbInfo->dbSpatialTablesLayersCount() > 0 )
    {
      addTableEntryType( mSpatialiteDbInfo->getDbLayersType( SpatialiteDbInfo::SpatialTable ), QgsSpatiaLiteTableModel::EntryTypeLayer );
    }
    if ( mSpatialiteDbInfo->dbSpatialViewsLayersCount() > 0 )
    {
      addTableEntryType( mSpatialiteDbInfo->getDbLayersType( SpatialiteDbInfo::SpatialView ), QgsSpatiaLiteTableModel::EntryTypeLayer );
    }
    if ( mSpatialiteDbInfo->dbVirtualShapesLayersCount() > 0 )
    {
      addTableEntryType( mSpatialiteDbInfo->getDbLayersType( SpatialiteDbInfo::VirtualShape ), QgsSpatiaLiteTableModel::EntryTypeLayer );
    }
  }
  if ( mSpatialiteDbInfo->dbRasterLite2LayersCount() > 0 )
  {
    addTableEntryType( mSpatialiteDbInfo->getDbRasterLite2Layers(), QgsSpatiaLiteTableModel::EntryTypeLayer );
  }
  // getDbTopologyLayers()
  // -> getDbTopologyNames()
  if ( mSpatialiteDbInfo->dbRasterLite1LayersCount() > 0 )
  {
    addTableEntryType( mSpatialiteDbInfo->getDbRasterLite1Layers(), QgsSpatiaLiteTableModel::EntryTypeLayer );
  }
  if ( mSpatialiteDbInfo->dbMBTilesLayersCount() > 0 )
  {
    addTableEntryType( mSpatialiteDbInfo->getDbMBTilesLayers(), QgsSpatiaLiteTableModel::EntryTypeLayer );
  }
  if ( mSpatialiteDbInfo->dbGeoPackageLayersCount() > 0 )
  {
    addTableEntryType( mSpatialiteDbInfo->getDbGeoPackageLayers(), QgsSpatiaLiteTableModel::EntryTypeLayer );
  }
  if ( mSpatialiteDbInfo->dbFdoOgrLayersCount() > 0 )
  {
    addTableEntryType( mSpatialiteDbInfo->getDbFdoOgrLayers(), QgsSpatiaLiteTableModel::EntryTypeLayer );
  }
  if ( ( mLoadGeometrylessTables ) && ( mSpatialiteDbInfo->dbNonSpatialTablesCount() > 0 ) )
  {
    addTableEntryType( mSpatialiteDbInfo->getDbNonSpatialTables(), QgsSpatiaLiteTableModel::EntryTypeMap, SpatialiteDbInfo::NonSpatialTables );
  }
}
void QgsSpatiaLiteTableModel::addTableEntryType( QMap<QString, QString> mapLayers, QgsSpatiaLiteTableModel::EntryType entryType, SpatialiteDbInfo::SpatialiteLayerType layerType )
{
  for ( QMap<QString, QString>::iterator itLayers = mapLayers.begin(); itLayers != mapLayers.end(); ++itLayers )
  {
    if ( !itLayers.key().isEmpty() )
    {
      switch ( entryType )
      {
        case QgsSpatiaLiteTableModel::EntryTypeMap:
        {
          addTableEntryMap( itLayers.key(), itLayers.value(), layerType );
        }
        break;
        case QgsSpatiaLiteTableModel::EntryTypeLayer:
        default:
        {
          SpatialiteDbLayer *dbLayer = mSpatialiteDbInfo->getSpatialiteDbLayer( itLayers.key(), true );
          if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) )
          {
            addTableEntryLayer( dbLayer );
          }
        }
        break;
      }
    }
  }
}
void QgsSpatiaLiteTableModel::addTableEntryMap( QString sKey, QString sValue, SpatialiteDbInfo::SpatialiteLayerType layerType )
{
  switch ( layerType )
  {
    case SpatialiteDbInfo::NonSpatialTables:
    default:
    {
      QStandardItem *nonSpatialItem = nullptr;
      QList < QStandardItem * >dbItems = findItems( "NonSpatial", Qt::MatchExactly | Qt::MatchRecursive, 0 );
      if ( !dbItems.isEmpty() )
      {
        nonSpatialItem = dbItems.at( 0 );
      }
      else
      {
        return;
      }
      QStandardItem *nonSpatialGroupItem = nullptr;
      QString sGroup = sValue;
      QString sSubGroup = sValue;
      QStringList sa_list_type = sValue.split( "-" );
      if ( sa_list_type.size() == 2 )
      {
        sGroup = sa_list_type[0];
        sSubGroup = sa_list_type[1];
      }
      dbItems = findItems( sGroup, Qt::MatchExactly | Qt::MatchRecursive, 0 );
      if ( !dbItems.isEmpty() )
      {
        nonSpatialGroupItem = dbItems.at( 0 );
      }
      else
      {
        QIcon iconType = SpatialiteDbInfo::SpatialiteLayerTypeIcon( SpatialiteDbInfo::NonSpatialTables );
        QStandardItem *tbTypeItem = new QStandardItem( iconType, sGroup );
        tbTypeItem->setFlags( Qt::ItemIsEnabled );
        QStandardItem *emptyItem_01 = new QStandardItem( );
        emptyItem_01->setFlags( Qt::ItemIsEnabled );
        //  Ever Columns must be filled (even if empty)
        QStandardItem *emptyItem_02 = new QStandardItem( );
        emptyItem_02->setFlags( Qt::ItemIsEnabled );
        QStandardItem *emptyItem_03 = new QStandardItem( );
        emptyItem_03->setFlags( Qt::ItemIsEnabled );
        // this must be done in the same order as the Header
        QList < QStandardItem *> tbItemList;
        tbItemList.push_back( tbTypeItem );
        tbItemList.push_back( emptyItem_01 );
        tbItemList.push_back( emptyItem_02 );
        tbItemList.push_back( emptyItem_03 );
        nonSpatialItem->appendRow( tbItemList );
        dbItems = findItems( sGroup, Qt::MatchExactly | Qt::MatchRecursive, 0 );
        if ( !dbItems.isEmpty() )
        {
          nonSpatialGroupItem = dbItems.at( 0 );
        }
        else
        {
          return;
        }
      }
      QIcon iconType = SpatialiteDbInfo::SpatialiteLayerTypeIcon( SpatialiteDbInfo::NonSpatialTables );
      QStandardItem *tbTypeItem = new QStandardItem( iconType, sKey );
      tbTypeItem->setFlags( Qt::ItemIsEnabled );
      QStandardItem *tbSubTypeItem = new QStandardItem( iconType, sValue );
      tbSubTypeItem->setFlags( Qt::ItemIsEnabled );
      //  Ever Columns must be filled (even if empty)
      QStandardItem *emptyItem_02 = new QStandardItem( );
      emptyItem_02->setFlags( Qt::ItemIsEnabled );
      QStandardItem *emptyItem_03 = new QStandardItem( );
      emptyItem_03->setFlags( Qt::ItemIsEnabled );
      // this must be done in the same order as the Header
      QList < QStandardItem *> tbItemList;
      tbItemList.push_back( tbTypeItem );
      tbItemList.push_back( tbSubTypeItem );
      tbItemList.push_back( emptyItem_02 );
      tbItemList.push_back( emptyItem_03 );
      nonSpatialGroupItem->appendRow( tbItemList );
      qDebug() << QString( "QgsSpatiaLiteTableModel::addTableEntryMap -I->-  Key[%1] Value[%2] Group[%3] SubGroup[%3]" ).arg( sKey ).arg( sValue ).arg( sGroup ).arg( sSubGroup );
    }
    break;
  }
}
void QgsSpatiaLiteTableModel::addTableEntryLayer( SpatialiteDbLayer *dbLayer )
{
  if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) )
  {
    //is there already a root item ?
    QStandardItem *dbItem = nullptr;
    // also searches lower level items
    QList < QStandardItem * >dbItems = findItems( dbLayer->getLayerTypeString(), Qt::MatchExactly | Qt::MatchRecursive, 0 );
    //there is already an item
    if ( !dbItems.isEmpty() )
    {
      dbItem = dbItems.at( 0 );
    }
    else  //create a new toplevel item
    {
      qDebug() << QString( "QgsSpatiaLiteTableModel::addTableEntry --E->-  TypeItem[%1] " ).arg( dbLayer->getLayerTypeString() );
      return;
    }
    // qDebug() << QString( "QgsSpatiaLiteTableModel::addTableEntry -02- TableName[%1] GeometryColName[%2] Type[%3] sql[%4]" ).arg( dbLayer->getTableName() ).arg( dbLayer->getGeometryColumn() ).arg( dbLayer->getGeometryTypeString() ).arg( dbLayer->getLayerQuery() );
    QList < QStandardItem * >childItemList;
    QStandardItem *tableNameItem = new QStandardItem( dbLayer->getLayerTypeIcon(), dbLayer->getTableName() );
    tableNameItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QStandardItem *geomItem = new QStandardItem( dbLayer->getGeometryColumn() );
    geomItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QStandardItem *geomTypeItem = new QStandardItem( dbLayer->getGeometryTypeIcon(), dbLayer->getGeometryTypeString() );
    geomTypeItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QStandardItem *sqlItem = new QStandardItem( dbLayer->getLayerQuery() );
    sqlItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
    // this must be done in the same order as the Header
    childItemList.push_back( tableNameItem );
    childItemList.push_back( geomItem );
    childItemList.push_back( geomTypeItem );
    childItemList.push_back( sqlItem );
    dbItem->insertRow( dbItem->rowCount(), childItemList );
    ++mTableCount;
  }
}
void QgsSpatiaLiteTableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }

  //find out table name
  QModelIndex tableSibling = index.sibling( index.row(), getTableNameIndex() );
  QModelIndex geomSibling = index.sibling( index.row(), getGeometryNameIndex() );

  if ( !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  QModelIndex sqlIndex = index.sibling( index.row(), getSqlQueryIndex() );
  if ( sqlIndex.isValid() )
  {
    itemFromIndex( sqlIndex )->setText( sql );
  }
}
