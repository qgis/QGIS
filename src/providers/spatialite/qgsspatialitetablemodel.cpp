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

QgsSpatiaLiteTableModel::QgsSpatiaLiteTableModel(): QStandardItemModel(), mTableCount( 0 )
{
  QStringList headerLabels;
  headerLabels << tr( "Table" );
  headerLabels << tr( "Type" );
  headerLabels << tr( "Geometry column" );
  headerLabels << tr( "Sql" );
  setHorizontalHeaderLabels( headerLabels );
}

QgsSpatiaLiteTableModel::~QgsSpatiaLiteTableModel()
{

}

void QgsSpatiaLiteTableModel::addTableEntry( const QString& type, const QString& tableName, const QString& geometryColName, const QString& sql )
{
  //is there already a root item ?
  QStandardItem *dbItem;
  QList < QStandardItem * >dbItems = findItems( mSqliteDb, Qt::MatchExactly, 0 );

  //there is already an item
  if ( !dbItems.isEmpty() )
  {
    dbItem = dbItems.at( 0 );
  }
  else                        //create a new toplevel item
  {
    dbItem = new QStandardItem( mSqliteDb );
    dbItem->setFlags( Qt::ItemIsEnabled );
    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), dbItem );
  }

  //path to icon for specified type
  QGis::WkbType wkbType = qgisTypeFromDbType( type );
  QIcon iconFile = iconForType( wkbType );

  QList < QStandardItem * >childItemList;
  QStandardItem *typeItem = new QStandardItem( QIcon( iconFile ), type );
  typeItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem *tableItem = new QStandardItem( tableName );
  tableItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem *geomItem = new QStandardItem( geometryColName );
  geomItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem* sqlItem = new QStandardItem( sql );
  sqlItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );


  childItemList.push_back( tableItem );
  childItemList.push_back( typeItem );
  childItemList.push_back( geomItem );
  childItemList.push_back( sqlItem );

  dbItem->appendRow( childItemList );
  ++mTableCount;
}

void QgsSpatiaLiteTableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }

  //find out table name
  QModelIndex tableSibling = index.sibling( index.row(), 0 );
  QModelIndex geomSibling = index.sibling( index.row(), 2 );

  if ( !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  QModelIndex sqlIndex = index.sibling( index.row(), 3 );
  if ( sqlIndex.isValid() )
  {
    itemFromIndex( sqlIndex )->setText( sql );
  }
}

void QgsSpatiaLiteTableModel::setGeometryTypesForTable( const QString & table, const QString & attribute, const QString & type )
{
  bool typeIsEmpty = type.isEmpty();  //true means the table has no valid geometry entry and the item for this table should be removed
  QStringList typeList = type.split( ',' );

  //find schema item and table item
  QStandardItem *dbItem;
  QList < QStandardItem * >dbItems = findItems( mSqliteDb, Qt::MatchExactly, 0 );

  if ( dbItems.size() < 1 )
  {
    return;
  }
  dbItem = dbItems.at( 0 );
  int numChildren = dbItem->rowCount();

  QModelIndex currentChildIndex;
  QModelIndex currentTableIndex;
  QModelIndex currentTypeIndex;
  QModelIndex currentGeomColumnIndex;

  for ( int i = 0; i < numChildren; ++i )
  {
    currentChildIndex = indexFromItem( dbItem->child( i, 0 ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }
    currentTableIndex = currentChildIndex.sibling( i, 1 );
    currentTypeIndex = currentChildIndex.sibling( i, 2 );
    currentGeomColumnIndex = currentChildIndex.sibling( i, 3 );
    QString geomColText = itemFromIndex( currentGeomColumnIndex )->text();

    if ( !currentTypeIndex.isValid() || !currentTableIndex.isValid() || !currentGeomColumnIndex.isValid() )
    {
      continue;
    }

    if ( itemFromIndex( currentTableIndex )->text() == table &&
         ( geomColText == attribute || geomColText.startsWith( attribute + " AS " ) ) )
    {
      if ( typeIsEmpty )
      {
        removeRow( i, indexFromItem( dbItem ) );
        return;
      }

      QGis::WkbType wkbType = qgisTypeFromDbType( typeList.at( 0 ) );
      QIcon myIcon = iconForType( wkbType );
      itemFromIndex( currentTypeIndex )->setText( typeList.at( 0 ) ); //todo: add other rows
      itemFromIndex( currentTypeIndex )->setIcon( myIcon );
      if ( !geomColText.contains( " AS " ) )
      {
        itemFromIndex( currentGeomColumnIndex )->setText( geomColText + " AS " + typeList.at( 0 ) );
      }

      for ( int j = 1; j < typeList.size(); ++j )
      {
        //todo: add correct type
        addTableEntry( typeList.at( j ), table, geomColText + " AS " + typeList.at( j ), "" );
      }
    }
  }
}

QIcon QgsSpatiaLiteTableModel::iconForType( QGis::WkbType type ) const
{
  if ( type == QGis::WKBPoint || type == QGis::WKBPoint25D || type == QGis::WKBMultiPoint || type == QGis::WKBMultiPoint25D )
  {
    return QgsLayerItem::iconPoint();
  }
  else if ( type == QGis::WKBLineString || type == QGis::WKBLineString25D || type == QGis::WKBMultiLineString
            || type == QGis::WKBMultiLineString25D )
  {
    return QgsLayerItem::iconLine();
  }
  else if ( type == QGis::WKBPolygon || type == QGis::WKBPolygon25D || type == QGis::WKBMultiPolygon
            || type == QGis::WKBMultiPolygon25D )
  {
    return QgsLayerItem::iconPolygon();
  }
  else
    return QIcon();
}

QString QgsSpatiaLiteTableModel::displayStringForType( QGis::WkbType type ) const
{
  if ( type == QGis::WKBPoint || type == QGis::WKBPoint25D )
  {
    return tr( "Point" );
  }
  else if ( type == QGis::WKBMultiPoint || type == QGis::WKBMultiPoint25D )
  {
    return tr( "Multipoint" );
  }
  else if ( type == QGis::WKBLineString || type == QGis::WKBLineString25D )
  {
    return tr( "Line" );
  }
  else if ( type == QGis::WKBMultiLineString || type == QGis::WKBMultiLineString25D )
  {
    return tr( "Multiline" );
  }
  else if ( type == QGis::WKBPolygon || type == QGis::WKBPolygon25D )
  {
    return tr( "Polygon" );
  }
  else if ( type == QGis::WKBMultiPolygon || type == QGis::WKBMultiPolygon25D )
  {
    return tr( "Multipolygon" );
  }
  return "Unknown";
}

QGis::WkbType QgsSpatiaLiteTableModel::qgisTypeFromDbType( const QString & dbType ) const
{
  if ( dbType == "POINT" )
  {
    return QGis::WKBPoint;
  }
  else if ( dbType == "MULTIPOINT" )
  {
    return QGis::WKBMultiPoint;
  }
  else if ( dbType == "LINESTRING" )
  {
    return QGis::WKBLineString;
  }
  else if ( dbType == "MULTILINESTRING" )
  {
    return QGis::WKBMultiLineString;
  }
  else if ( dbType == "POLYGON" )
  {
    return QGis::WKBPolygon;
  }
  else if ( dbType == "MULTIPOLYGON" )
  {
    return QGis::WKBMultiPolygon;
  }
  return QGis::WKBUnknown;
}
