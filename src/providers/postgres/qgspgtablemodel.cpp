/***************************************************************************
                         qgspgtablemodel.cpp  -  description
                         -------------------
    begin                : Dec 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspgtablemodel.h"
#include "qgsdataitem.h"
#include "qgslogger.h"

QgsPgTableModel::QgsPgTableModel()
    : QStandardItemModel()
    , mTableCount( 0 )
{
  QStringList headerLabels;
  headerLabels << tr( "Schema" );
  headerLabels << tr( "Table" );
  headerLabels << tr( "Type" );
  headerLabels << tr( "Geometry column" );
  headerLabels << tr( "SRID" );
  headerLabels << tr( "Primary key column" );
  headerLabels << tr( "Select at id" );
  headerLabels << tr( "Sql" );
  setHorizontalHeaderLabels( headerLabels );
}

QgsPgTableModel::~QgsPgTableModel()
{
}

void QgsPgTableModel::addTableEntry( QgsPostgresLayerProperty layerProperty )
{
  QgsDebugMsg( QString( "%1.%2.%3 type=%4 srid=%5 pk=%6 sql=%7" )
               .arg( layerProperty.schemaName )
               .arg( layerProperty.tableName )
               .arg( layerProperty.geometryColName )
               .arg( layerProperty.type )
               .arg( layerProperty.srid )
               .arg( layerProperty.pkCols.join( "," ) )
               .arg( layerProperty.sql ) );

  // is there already a root item with the given scheme Name?
  QStandardItem *schemaItem;
  QList<QStandardItem*> schemaItems = findItems( layerProperty.schemaName, Qt::MatchExactly, dbtmSchema );

  // there is already an item for this schema
  if ( schemaItems.size() > 0 )
  {
    schemaItem = schemaItems.at( dbtmSchema );
  }
  else
  {
    // create a new toplevel item for this schema
    schemaItem = new QStandardItem( layerProperty.schemaName );
    schemaItem->setFlags( Qt::ItemIsEnabled );
    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), schemaItem );
  }

  QGis::GeometryType geomType = QgsPostgresConn::geomTypeFromPostgis( layerProperty.type );
  if ( geomType == QGis::UnknownGeometry && layerProperty.geometryColName.isEmpty() )
  {
    geomType = QGis::NoGeometry;
  }

  QList<QStandardItem*> childItemList;

  QStandardItem *schemaNameItem = new QStandardItem( layerProperty.schemaName );
  schemaNameItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  QStandardItem *typeItem = new QStandardItem( iconForGeomType( geomType ), geomType == QGis::UnknownGeometry ? tr( "Waiting..." ) : QgsPostgresConn::displayStringForGeomType( geomType ) );
  typeItem->setData( geomType == QGis::UnknownGeometry, Qt::UserRole + 1 );
  typeItem->setData( geomType, Qt::UserRole + 2 );
  typeItem->setFlags(( geomType != QGis::UnknownGeometry ? Qt::ItemIsEnabled : Qt::NoItemFlags ) | Qt::ItemIsSelectable | Qt::ItemIsEditable );

  QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
  tableItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  QStandardItem *geomItem = new QStandardItem( layerProperty.geometryColName );
  geomItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  QStandardItem *sridItem = new QStandardItem( QString::number( layerProperty.srid ) );
  sridItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  QString pkText, pkCol = "";
  switch ( layerProperty.pkCols.size() )
  {
    case 0:   pkText = ""; break;
    case 1:   pkText = layerProperty.pkCols[0]; pkCol = pkText; break;
    default:  pkText = tr( "Select..." ); break;
  }

  QStandardItem *pkItem = new QStandardItem( pkText );
  pkItem->setData( layerProperty.pkCols, Qt::UserRole + 1 );
  pkItem->setData( pkCol, Qt::UserRole + 2 );
  pkItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );

  QStandardItem *selItem = new QStandardItem( "" );
  selItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable );
  selItem->setCheckState( Qt::Checked );
  selItem->setToolTip( tr( "Disable 'Fast Access to Features at ID' capability to force keeping the attribute table in memory (e.g. in case of expensive views)." ) );

  QStandardItem* sqlItem = new QStandardItem( layerProperty.sql );
  sqlItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );

  childItemList << schemaNameItem;
  childItemList << tableItem;
  childItemList << typeItem;
  childItemList << geomItem;
  childItemList << sridItem;
  childItemList << pkItem;
  childItemList << selItem;
  childItemList << sqlItem;

  schemaItem->appendRow( childItemList );

  ++mTableCount;
}

void QgsPgTableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }

  //find out schema name and table name
  QModelIndex schemaSibling = index.sibling( index.row(), dbtmSchema );
  QModelIndex tableSibling = index.sibling( index.row(), dbtmTable );
  QModelIndex geomSibling = index.sibling( index.row(), dbtmGeomCol );

  if ( !schemaSibling.isValid() || !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  QString schemaName = itemFromIndex( schemaSibling )->text();
  QString tableName = itemFromIndex( tableSibling )->text();
  QString geomName = itemFromIndex( geomSibling )->text();

  QList<QStandardItem*> schemaItems = findItems( schemaName, Qt::MatchExactly, dbtmSchema );
  if ( schemaItems.size() < 1 )
  {
    return;
  }

  QStandardItem* schemaItem = schemaItems.at( dbtmSchema );

  int n = schemaItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    QModelIndex currentChildIndex = indexFromItem( schemaItem->child( i, dbtmSchema ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }
    QModelIndex currentTableIndex = currentChildIndex.sibling( i, dbtmTable );
    if ( !currentTableIndex.isValid() )
    {
      continue;
    }

    QModelIndex currentGeomIndex = currentChildIndex.sibling( i, dbtmGeomCol );
    if ( !currentGeomIndex.isValid() )
    {
      continue;
    }

    if ( itemFromIndex( currentTableIndex )->text() == tableName &&
         itemFromIndex( currentGeomIndex )->text() == geomName )
    {
      QModelIndex sqlIndex = currentChildIndex.sibling( i, dbtmSql );
      if ( sqlIndex.isValid() )
      {
        itemFromIndex( sqlIndex )->setText( sql );
        break;
      }
    }
  }
}

void QgsPgTableModel::setGeometryTypesForTable( QgsPostgresLayerProperty layerProperty )
{
  QStringList typeList = layerProperty.type.split( ",", QString::SkipEmptyParts );

  //find schema item and table item
  QStandardItem* schemaItem;
  QList<QStandardItem*> schemaItems = findItems( layerProperty.schemaName, Qt::MatchExactly, dbtmSchema );

  if ( schemaItems.size() < 1 )
  {
    return;
  }

  schemaItem = schemaItems.at( 0 );

  int n = schemaItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    QModelIndex currentChildIndex = indexFromItem( schemaItem->child( i, dbtmSchema ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }

    QModelIndex currentTypeIndex = currentChildIndex.sibling( i, dbtmType );
    QModelIndex currentTableIndex = currentChildIndex.sibling( i, dbtmTable );
    QModelIndex currentGeomColumnIndex = currentChildIndex.sibling( i, dbtmGeomCol );
    QModelIndex currentPkColumnIndex = currentChildIndex.sibling( i, dbtmPkCol );
    QModelIndex currentSridIndex = currentChildIndex.sibling( i, dbtmSrid );

    if ( !currentTypeIndex.isValid()
         || !currentTableIndex.isValid()
         || !currentGeomColumnIndex.isValid()
         || !currentPkColumnIndex.isValid()
         || !currentSridIndex.isValid()
       )
    {
      continue;
    }

    QString tableText = itemFromIndex( currentTableIndex )->text();
    QString geomColText = itemFromIndex( currentGeomColumnIndex )->text();
    QStandardItem *typeItem = itemFromIndex( currentTypeIndex );
    QStandardItem *sridItem = itemFromIndex( currentSridIndex );

    if ( tableText == layerProperty.tableName && geomColText == layerProperty.geometryColName )
    {
      sridItem->setText( QString::number( layerProperty.srid ) );

      if ( typeList.isEmpty() )
      {
        typeItem->setText( tr( "Select..." ) );
      }
      else
      {
        // update existing row
        QGis::GeometryType geomType = QgsPostgresConn::geomTypeFromPostgis( typeList.at( 0 ) );

        typeItem->setIcon( iconForGeomType( geomType ) );
        typeItem->setText( QgsPostgresConn::displayStringForGeomType( geomType ) );
        typeItem->setData( false, Qt::UserRole + 1 );
        typeItem->setData( geomType, Qt::UserRole + 2 );

        for ( int j = 1; j < typeList.size(); j++ )
        {
          layerProperty.type = typeList[j];
          addTableEntry( layerProperty );
        }
      }

      typeItem->setFlags( typeItem->flags() | Qt::ItemIsEnabled );
    }
  }
}

QIcon QgsPgTableModel::iconForGeomType( QGis::GeometryType type )
{
  switch ( type )
  {
    case QGis::Point:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconPointLayer.png" ) );
    case QGis::Line:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconLineLayer.png" ) );
    case QGis::Polygon:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconPolygonLayer.png" ) );
    case QGis::NoGeometry:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconTableLayer.png" ) );
    default:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconLayer.png" ) );
  }
}
