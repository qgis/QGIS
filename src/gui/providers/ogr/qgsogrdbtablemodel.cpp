/***************************************************************************
  qgsogrdbtablemodel.cpp - QgsOgrDbTableModel

 ---------------------
 begin                : 5.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsogrdbtablemodel.h"
///@cond PRIVATE

#include "qgsapplication.h"

#include <QIcon>

QgsOgrDbTableModel::QgsOgrDbTableModel( QObject *parent )
  : QgsAbstractDbTableModel( parent )
{
  mColumns << tr( "Table" )
           << tr( "Type" )
           << tr( "Geometry column" )
           << tr( "SQL" );
  setHorizontalHeaderLabels( mColumns );
}

QStringList QgsOgrDbTableModel::columns() const
{
  return mColumns;
}

int QgsOgrDbTableModel::defaultSearchColumn() const
{
  return 0;
}

bool QgsOgrDbTableModel::searchableColumn( int column ) const
{
  Columns col = static_cast<Columns>( column );
  switch ( col )
  {
    case QgsOgrDbTableModel::DbtmTable:
    case QgsOgrDbTableModel::DbtmType:
    case QgsOgrDbTableModel::DbtmGeomCol:
    case QgsOgrDbTableModel::DbtmSql:
      return true;
  }
  BUILTIN_UNREACHABLE
}

void QgsOgrDbTableModel::addTableEntry( Qgis::BrowserLayerType layerType, const QString &tableName, const QString &uri, const QString &geometryColName, const QString &geometryType, const QString &sql )
{
  //is there already a root item ?
  QStandardItem *dbItem = nullptr;
  const QList < QStandardItem * >dbItems = findItems( mPath, Qt::MatchExactly, 0 );

  //there is already an item
  if ( !dbItems.isEmpty() )
  {
    dbItem = dbItems.at( 0 );
  }
  else //create a new toplevel item
  {
    dbItem = new QStandardItem( mPath );
    dbItem->setFlags( Qt::ItemIsEnabled );
    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), dbItem );
  }

  QList < QStandardItem * >childItemList;
  QStandardItem *typeItem = new QStandardItem( QgsApplication::getThemeIcon( QgsLayerItem::iconName( layerType ) ), geometryType );
  typeItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem *tableItem = new QStandardItem( tableName );
  tableItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem *geomItem = new QStandardItem( geometryColName );
  geomItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem *sqlItem = new QStandardItem( sql );
  sqlItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );

  // Add uri and geometryType information
  tableItem->setData( uri, Qt::UserRole + 1 );
  tableItem->setData( geometryType, Qt::UserRole + 2 );

  childItemList.push_back( tableItem );
  childItemList.push_back( typeItem );
  childItemList.push_back( geomItem );
  childItemList.push_back( sqlItem );

  dbItem->appendRow( childItemList );
  ++mTableCount;
}

void QgsOgrDbTableModel::setSql( const QModelIndex &index, const QString &sql )
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

///@endcond
