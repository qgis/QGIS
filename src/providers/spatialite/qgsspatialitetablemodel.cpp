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
#include "qgsiconutils.h"

QgsSpatiaLiteTableModel::QgsSpatiaLiteTableModel( QObject *parent )
  : QgsAbstractDbTableModel( parent )
{
  mColumns << tr( "Table" )
           << tr( "Type" )
           << tr( "Geometry column" )
           << tr( "SQL" );
  setHorizontalHeaderLabels( mColumns );
}

QStringList QgsSpatiaLiteTableModel::columns() const
{
  return mColumns;
}

int QgsSpatiaLiteTableModel::defaultSearchColumn() const
{
  return 0;
}

bool QgsSpatiaLiteTableModel::searchableColumn( int column ) const
{
  Q_UNUSED( column )
  return true;
}

void QgsSpatiaLiteTableModel::addTableEntry( const QString &type, const QString &tableName, const QString &geometryColName, const QString &sql )
{
  //is there already a root item ?
  QStandardItem *dbItem = nullptr;
  const QList < QStandardItem * >dbItems = findItems( mSqliteDb, Qt::MatchExactly, 0 );

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
  const QgsWkbTypes::Type wkbType = qgisTypeFromDbType( type );
  const QIcon iconFile = iconForType( wkbType );

  QList < QStandardItem * >childItemList;
  QStandardItem *typeItem = new QStandardItem( QIcon( iconFile ), type );
  typeItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem *tableItem = new QStandardItem( tableName );
  tableItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem *geomItem = new QStandardItem( geometryColName );
  geomItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem *sqlItem = new QStandardItem( sql );
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
  const QModelIndex tableSibling = index.sibling( index.row(), 0 );
  const QModelIndex geomSibling = index.sibling( index.row(), 2 );

  if ( !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  const QModelIndex sqlIndex = index.sibling( index.row(), 3 );
  if ( sqlIndex.isValid() )
  {
    itemFromIndex( sqlIndex )->setText( sql );
  }
}

void QgsSpatiaLiteTableModel::setGeometryTypesForTable( const QString &table, const QString &attribute, const QString &type )
{
  const bool typeIsEmpty = type.isEmpty();  //true means the table has no valid geometry entry and the item for this table should be removed
  const QStringList typeList = type.split( ',' );

  //find schema item and table item
  QStandardItem *dbItem = nullptr;
  const QList < QStandardItem * >dbItems = findItems( mSqliteDb, Qt::MatchExactly, 0 );

  if ( dbItems.empty() )
  {
    return;
  }
  dbItem = dbItems.at( 0 );
  const int numChildren = dbItem->rowCount();

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
    const QString geomColText = itemFromIndex( currentGeomColumnIndex )->text();

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

      const QgsWkbTypes::Type wkbType = qgisTypeFromDbType( typeList.at( 0 ) );
      const QIcon myIcon = iconForType( wkbType );
      itemFromIndex( currentTypeIndex )->setText( typeList.at( 0 ) ); //todo: add other rows
      itemFromIndex( currentTypeIndex )->setIcon( myIcon );
      if ( !geomColText.contains( QLatin1String( " AS " ) ) )
      {
        itemFromIndex( currentGeomColumnIndex )->setText( geomColText + " AS " + typeList.at( 0 ) );
      }

      for ( int j = 1; j < typeList.size(); ++j )
      {
        //todo: add correct type
        addTableEntry( typeList.at( j ), table, geomColText + " AS " + typeList.at( j ), QString() );
      }
    }
  }
}

QIcon QgsSpatiaLiteTableModel::iconForType( QgsWkbTypes::Type type ) const
{
  if ( type == QgsWkbTypes::Point || type == QgsWkbTypes::Point25D || type == QgsWkbTypes::MultiPoint || type == QgsWkbTypes::MultiPoint25D )
  {
    return QgsIconUtils::iconPoint();
  }
  else if ( type == QgsWkbTypes::LineString || type == QgsWkbTypes::LineString25D || type == QgsWkbTypes::MultiLineString
            || type == QgsWkbTypes::MultiLineString25D )
  {
    return QgsIconUtils::iconLine();
  }
  else if ( type == QgsWkbTypes::Polygon || type == QgsWkbTypes::Polygon25D || type == QgsWkbTypes::MultiPolygon
            || type == QgsWkbTypes::MultiPolygon25D )
  {
    return QgsIconUtils::iconPolygon();
  }
  else
    return QIcon();
}

QString QgsSpatiaLiteTableModel::displayStringForType( QgsWkbTypes::Type type ) const
{
  if ( type == QgsWkbTypes::Point || type == QgsWkbTypes::Point25D )
  {
    return tr( "Point" );
  }
  else if ( type == QgsWkbTypes::MultiPoint || type == QgsWkbTypes::MultiPoint25D )
  {
    return tr( "Multipoint" );
  }
  else if ( type == QgsWkbTypes::LineString || type == QgsWkbTypes::LineString25D )
  {
    return tr( "Line" );
  }
  else if ( type == QgsWkbTypes::MultiLineString || type == QgsWkbTypes::MultiLineString25D )
  {
    return tr( "Multiline" );
  }
  else if ( type == QgsWkbTypes::Polygon || type == QgsWkbTypes::Polygon25D )
  {
    return tr( "Polygon" );
  }
  else if ( type == QgsWkbTypes::MultiPolygon || type == QgsWkbTypes::MultiPolygon25D )
  {
    return tr( "Multipolygon" );
  }
  return QStringLiteral( "Unknown" );
}

QgsWkbTypes::Type QgsSpatiaLiteTableModel::qgisTypeFromDbType( const QString &dbType ) const
{
  if ( dbType == QLatin1String( "POINT" ) )
  {
    return QgsWkbTypes::Point;
  }
  else if ( dbType == QLatin1String( "MULTIPOINT" ) )
  {
    return QgsWkbTypes::MultiPoint;
  }
  else if ( dbType == QLatin1String( "LINESTRING" ) )
  {
    return QgsWkbTypes::LineString;
  }
  else if ( dbType == QLatin1String( "MULTILINESTRING" ) )
  {
    return QgsWkbTypes::MultiLineString;
  }
  else if ( dbType == QLatin1String( "POLYGON" ) )
  {
    return QgsWkbTypes::Polygon;
  }
  else if ( dbType == QLatin1String( "MULTIPOLYGON" ) )
  {
    return QgsWkbTypes::MultiPolygon;
  }
  return QgsWkbTypes::Unknown;
}
