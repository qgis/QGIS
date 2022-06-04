/***************************************************************************
                         qgsoracletablemodel.cpp  -  description
                         -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoracletablemodel.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsiconutils.h"

QgsOracleTableModel::QgsOracleTableModel( QObject *parent )
  : QgsAbstractDbTableModel( parent )
{
  mColumns << tr( "Owner" )
           << tr( "Table" )
           << tr( "Type" )
           << tr( "Geometry column" )
           << tr( "SRID" )
           << tr( "Primary key column" )
           << tr( "Select at id" )
           << tr( "SQL" );
  setHorizontalHeaderLabels( mColumns );
}

QStringList QgsOracleTableModel::columns() const
{
  return mColumns;
}

int QgsOracleTableModel::defaultSearchColumn() const
{
  return static_cast<int>( DbtmTable );
}

bool QgsOracleTableModel::searchableColumn( int column ) const
{
  Columns col = static_cast<Columns>( column );
  switch ( col )
  {
    case DbtmOwner:
    case DbtmTable:
    case DbtmGeomCol:
    case DbtmType:
    case DbtmSrid:
    case DbtmSql:
      return true;

    case DbtmPkCol:
    case DbtmSelectAtId:
      return false;
  }
  return false;
}

void QgsOracleTableModel::addTableEntry( const QgsOracleLayerProperty &layerProperty )
{
  QgsDebugMsg( layerProperty.toString() );

  if ( layerProperty.isView && layerProperty.pkCols.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "View without pk skipped." ) );
    return;
  }

  // is there already a root item with the given scheme Name?
  QStandardItem *ownerItem = nullptr;

  for ( int i = 0; i < layerProperty.size(); i++ )
  {
    QgsWkbTypes::Type wkbType = layerProperty.types[ i ];
    int srid = layerProperty.srids[ i ];


    QString tip;
    if ( wkbType == QgsWkbTypes::Unknown )
    {
      tip = tr( "Specify a geometry type" );
    }
    else if ( wkbType != QgsWkbTypes::NoGeometry && srid == 0 )
    {
      tip = tr( "Enter a SRID" );
    }

    if ( tip.isEmpty() && layerProperty.isView )
    {
      tip = tr( "Select a primary key" );
    }

    QStandardItem *ownerNameItem = new QStandardItem( layerProperty.ownerName );
    QStandardItem *typeItem = new QStandardItem(
      QgsIconUtils::iconForWkbType( wkbType ),
      wkbType == QgsWkbTypes::Unknown ? tr( "Select…" ) : QgsWkbTypes::translatedDisplayString( wkbType ) );
    typeItem->setData( wkbType == QgsWkbTypes::Unknown, Qt::UserRole + 1 );
    typeItem->setData( wkbType, Qt::UserRole + 2 );
    if ( wkbType == QgsWkbTypes::Unknown )
      typeItem->setFlags( typeItem->flags() | Qt::ItemIsEditable );

    QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
    QStandardItem *geomItem  = new QStandardItem( layerProperty.geometryColName );
    QStandardItem *sridItem  = new QStandardItem( wkbType != QgsWkbTypes::NoGeometry ? QString::number( srid ) : "" );
    sridItem->setEditable( wkbType != QgsWkbTypes::NoGeometry && srid == 0 );
    if ( sridItem->isEditable() )
    {
      sridItem->setText( tr( "Enter…" ) );
      sridItem->setFlags( sridItem->flags() | Qt::ItemIsEditable );
    }

    QStandardItem *pkItem = new QStandardItem( "" );
    if ( layerProperty.isView )
    {
      pkItem->setText( tr( "Select…" ) );
      pkItem->setFlags( pkItem->flags() | Qt::ItemIsEditable );
    }
    else
      pkItem->setFlags( pkItem->flags() & ~Qt::ItemIsEditable );

    pkItem->setData( layerProperty.isView, Qt::UserRole + 1 );
    pkItem->setData( false, Qt::UserRole + 2 ); // not selected

    QStandardItem *selItem = new QStandardItem( "" );
    selItem->setFlags( selItem->flags() | Qt::ItemIsUserCheckable );
    selItem->setCheckState( Qt::Checked );
    selItem->setToolTip( tr( "Disable 'Fast Access to Features at ID' capability to force keeping the attribute table in memory (e.g. in case of expensive views)." ) );

    QStandardItem *sqlItem = new QStandardItem( layerProperty.sql );

    QList<QStandardItem *> childItemList;
    childItemList << ownerNameItem;
    childItemList << tableItem;
    childItemList << typeItem;
    childItemList << geomItem;
    childItemList << sridItem;
    childItemList << pkItem;
    childItemList << selItem;
    childItemList << sqlItem;

    const auto constChildItemList = childItemList;
    for ( QStandardItem *item : constChildItemList )
    {
      if ( tip.isEmpty() )
      {
        item->setFlags( item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        item->setToolTip( "" );
      }
      else
      {
        item->setFlags( item->flags() & ~Qt::ItemIsSelectable );

        if ( item == ownerNameItem || item == tableItem || item == geomItem )
        {
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          item->setToolTip( tip );
        }
      }
    }

    if ( !ownerItem )
    {
      QList<QStandardItem *> ownerItems = findItems( layerProperty.ownerName, Qt::MatchExactly, DbtmOwner );

      // there is already an item for this schema
      if ( ownerItems.size() > 0 )
      {
        ownerItem = ownerItems.at( DbtmOwner );
      }
      else
      {
        // create a new toplevel item for this schema
        ownerItem = new QStandardItem( layerProperty.ownerName );
        ownerItem->setFlags( Qt::ItemIsEnabled );
        invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), ownerItem );
      }
    }

    ownerItem->appendRow( childItemList );

    ++mTableCount;
  }
}

void QgsOracleTableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }

  //find out schema name and table name
  QModelIndex ownerSibling = index.sibling( index.row(), DbtmOwner );
  QModelIndex tableSibling = index.sibling( index.row(), DbtmTable );
  QModelIndex geomSibling = index.sibling( index.row(), DbtmGeomCol );

  if ( !ownerSibling.isValid() || !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  QString ownerName = itemFromIndex( ownerSibling )->text();
  QString tableName = itemFromIndex( tableSibling )->text();
  QString geomName = itemFromIndex( geomSibling )->text();

  QList<QStandardItem *> ownerItems = findItems( ownerName, Qt::MatchExactly, DbtmOwner );
  if ( ownerItems.size() < 1 )
  {
    return;
  }

  QStandardItem *ownerItem = ownerItems.at( DbtmOwner );

  int n = ownerItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    QModelIndex currentChildIndex = indexFromItem( ownerItem->child( i, DbtmOwner ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }

    QModelIndex currentTableIndex = currentChildIndex.sibling( i, DbtmTable );
    if ( !currentTableIndex.isValid() )
    {
      continue;
    }

    QModelIndex currentGeomIndex = currentChildIndex.sibling( i, DbtmGeomCol );
    if ( !currentGeomIndex.isValid() )
    {
      continue;
    }

    if ( itemFromIndex( currentTableIndex )->text() == tableName && itemFromIndex( currentGeomIndex )->text() == geomName )
    {
      QModelIndex sqlIndex = currentChildIndex.sibling( i, DbtmSql );
      if ( sqlIndex.isValid() )
      {
        itemFromIndex( sqlIndex )->setText( sql );
        break;
      }
    }
  }
}

bool QgsOracleTableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
  if ( !QStandardItemModel::setData( idx, value, role ) )
    return false;

  if ( idx.column() == DbtmType || idx.column() == DbtmSrid || idx.column() == DbtmPkCol )
  {
    QgsWkbTypes::Type wkbType = ( QgsWkbTypes::Type ) idx.sibling( idx.row(), DbtmType ).data( Qt::UserRole + 2 ).toInt();

    QString tip;
    if ( wkbType == QgsWkbTypes::Unknown )
    {
      tip = tr( "Specify a geometry type" );
    }
    else if ( wkbType != QgsWkbTypes::NoGeometry )
    {
      bool ok;
      int srid = idx.sibling( idx.row(), DbtmSrid ).data().toInt( &ok );

      if ( !ok || srid == 0 )
        tip = tr( "Enter a SRID" );
    }

    if ( tip.isEmpty() && idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 1 ).toBool() )
    {
      if ( !idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 2 ).toBool() )
        tip = tr( "Select a primary key" );
    }

    for ( int i = 0; i < columnCount(); i++ )
    {
      QStandardItem *item = itemFromIndex( idx.sibling( idx.row(), i ) );
      if ( tip.isEmpty() )
      {
        item->setFlags( item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        item->setToolTip( "" );
      }
      else
      {
        item->setFlags( item->flags() & ~Qt::ItemIsSelectable );
        if ( i == DbtmOwner || i == DbtmTable || i == DbtmGeomCol )
        {
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          item->setToolTip( tip );
        }
      }
    }
  }

  return true;
}

QString QgsOracleTableModel::layerURI( const QModelIndex &index, const QgsDataSourceUri &connInfo )
{
  if ( !index.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "invalid index" ) );
    return QString();
  }

  QgsWkbTypes::Type wkbType = ( QgsWkbTypes::Type ) itemFromIndex( index.sibling( index.row(), DbtmType ) )->data( Qt::UserRole + 2 ).toInt();
  if ( wkbType == QgsWkbTypes::Unknown )
  {
    QgsDebugMsg( QStringLiteral( "unknown geometry type" ) );
    // no geometry type selected
    return QString();
  }

  QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), DbtmPkCol ) );
  QString pkColumnName = pkItem->data( Qt::DisplayRole ).toString();
  bool isView = pkItem->data( Qt::UserRole + 1 ).toBool();
  bool isSet  = pkItem->data( Qt::UserRole + 2 ).toBool();

  if ( isView && !isSet )
  {
    // no valid primary candidate selected
    QgsDebugMsg( QStringLiteral( "no pk candidate selected" ) );
    return QString();
  }

  QString ownerName = index.sibling( index.row(), DbtmOwner ).data( Qt::DisplayRole ).toString();
  QString tableName = index.sibling( index.row(), DbtmTable ).data( Qt::DisplayRole ).toString();

  QString geomColumnName;
  QString srid;
  if ( wkbType != QgsWkbTypes::NoGeometry )
  {
    geomColumnName = index.sibling( index.row(), DbtmGeomCol ).data( Qt::DisplayRole ).toString();

    srid = index.sibling( index.row(), DbtmSrid ).data( Qt::DisplayRole ).toString();
    bool ok;
    ( void )srid.toInt( &ok );
    if ( !ok )
    {
      QgsDebugMsg( QStringLiteral( "srid not numeric" ) );
      return QString();
    }
  }

  bool selectAtId = itemFromIndex( index.sibling( index.row(), DbtmSelectAtId ) )->checkState() == Qt::Checked;
  QString sql = index.sibling( index.row(), DbtmSql ).data( Qt::DisplayRole ).toString();

  QgsDataSourceUri uri( connInfo );
  uri.setDataSource( ownerName, tableName, geomColumnName, sql, pkColumnName );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );

  QgsDebugMsg( QStringLiteral( "returning uri %1" ).arg( uri.uri( false ) ) );
  return uri.uri( false );
}
