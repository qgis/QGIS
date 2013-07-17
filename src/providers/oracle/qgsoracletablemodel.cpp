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
#include "qgsdataitem.h"
#include "qgslogger.h"

QgsOracleTableModel::QgsOracleTableModel()
    : QStandardItemModel()
    , mTableCount( 0 )
{
  QStringList headerLabels;
  headerLabels << tr( "Owner" );
  headerLabels << tr( "Table" );
  headerLabels << tr( "Type" );
  headerLabels << tr( "Geometry column" );
  headerLabels << tr( "SRID" );
  headerLabels << tr( "Primary key column" );
  headerLabels << tr( "Select at id" );
  headerLabels << tr( "Sql" );
  setHorizontalHeaderLabels( headerLabels );
}

QgsOracleTableModel::~QgsOracleTableModel()
{
}

void QgsOracleTableModel::addTableEntry( const QgsOracleLayerProperty &layerProperty )
{
  QgsDebugMsg( layerProperty.toString() );

  if ( layerProperty.isView && layerProperty.pkCols.isEmpty() )
  {
    QgsDebugMsg( "View without pk skipped." );
    return;
  }

  // is there already a root item with the given scheme Name?
  QStandardItem *ownerItem = 0;

  for ( int i = 0; i < layerProperty.size(); i++ )
  {
    QGis::WkbType wkbType = layerProperty.types[ i ];
    int srid = layerProperty.srids[ i ];


    QString tip;
    if ( wkbType == QGis::WKBUnknown )
    {
      tip = tr( "Specify a geometry type" );
    }
    else if ( wkbType != QGis::WKBNoGeometry && srid == 0 )
    {
      tip = tr( "Enter a SRID" );
    }

    if ( tip.isEmpty() && layerProperty.isView )
    {
      tip = tr( "Select a primary key" );
    }

    QStandardItem *ownerNameItem = new QStandardItem( layerProperty.ownerName );
    QStandardItem *typeItem = new QStandardItem( iconForWkbType( wkbType ), wkbType == QGis::WKBUnknown ? tr( "Select..." ) : QgsOracleConn::displayStringForWkbType( wkbType ) );
    typeItem->setData( wkbType == QGis::WKBUnknown, Qt::UserRole + 1 );
    typeItem->setData( wkbType, Qt::UserRole + 2 );
    if ( wkbType == QGis::WKBUnknown )
      typeItem->setFlags( typeItem->flags() | Qt::ItemIsEditable );

    QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
    QStandardItem *geomItem  = new QStandardItem( layerProperty.geometryColName );
    QStandardItem *sridItem  = new QStandardItem( wkbType != QGis::WKBNoGeometry ? QString::number( srid ) : "" );
    sridItem->setEditable( wkbType != QGis::WKBNoGeometry && srid == 0 );
    if ( sridItem->isEditable() )
    {
      sridItem->setText( tr( "Enter..." ) );
      sridItem->setFlags( sridItem->flags() | Qt::ItemIsEditable );
    }

    QStandardItem *pkItem = new QStandardItem( "" );
    if ( layerProperty.isView )
    {
      pkItem->setText( tr( "Select..." ) );
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

    QStandardItem* sqlItem = new QStandardItem( layerProperty.sql );

    QList<QStandardItem*> childItemList;
    childItemList << ownerNameItem;
    childItemList << tableItem;
    childItemList << typeItem;
    childItemList << geomItem;
    childItemList << sridItem;
    childItemList << pkItem;
    childItemList << selItem;
    childItemList << sqlItem;

    foreach ( QStandardItem *item, childItemList )
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
      QList<QStandardItem*> ownerItems = findItems( layerProperty.ownerName, Qt::MatchExactly, dbtmOwner );

      // there is already an item for this schema
      if ( ownerItems.size() > 0 )
      {
        ownerItem = ownerItems.at( dbtmOwner );
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
  QModelIndex ownerSibling = index.sibling( index.row(), dbtmOwner );
  QModelIndex tableSibling = index.sibling( index.row(), dbtmTable );
  QModelIndex geomSibling = index.sibling( index.row(), dbtmGeomCol );

  if ( !ownerSibling.isValid() || !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  QString ownerName = itemFromIndex( ownerSibling )->text();
  QString tableName = itemFromIndex( tableSibling )->text();
  QString geomName = itemFromIndex( geomSibling )->text();

  QList<QStandardItem*> ownerItems = findItems( ownerName, Qt::MatchExactly, dbtmOwner );
  if ( ownerItems.size() < 1 )
  {
    return;
  }

  QStandardItem* ownerItem = ownerItems.at( dbtmOwner );

  int n = ownerItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    QModelIndex currentChildIndex = indexFromItem( ownerItem->child( i, dbtmOwner ) );
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

    if ( itemFromIndex( currentTableIndex )->text() == tableName && itemFromIndex( currentGeomIndex )->text() == geomName )
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

QIcon QgsOracleTableModel::iconForWkbType( QGis::WkbType type )
{
  switch ( type )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      return QgsApplication::getThemeIcon( "/mIconPointLayer.svg" );
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      return QgsApplication::getThemeIcon( "/mIconLineLayer.svg" );
    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      return QgsApplication::getThemeIcon( "/mIconPolygonLayer.svg" );
    case QGis::WKBNoGeometry:
      return QgsApplication::getThemeIcon( "/mIconTableLayer.png" );
    case QGis::WKBUnknown:
      break;
  }
  return QgsApplication::getThemeIcon( "/mIconLayer.png" );
}

bool QgsOracleTableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
  if ( !QStandardItemModel::setData( idx, value, role ) )
    return false;

  if ( idx.column() == dbtmType || idx.column() == dbtmSrid || idx.column() == dbtmPkCol )
  {
    QGis::WkbType wkbType = ( QGis::WkbType ) idx.sibling( idx.row(), dbtmType ).data( Qt::UserRole + 2 ).toInt();

    QString tip;
    if ( wkbType == QGis::WKBUnknown )
    {
      tip = tr( "Specify a geometry type" );
    }
    else if ( wkbType != QGis::WKBNoGeometry )
    {
      bool ok;
      int srid = idx.sibling( idx.row(), dbtmSrid ).data().toInt( &ok );

      if ( !ok || srid == 0 )
        tip = tr( "Enter a SRID" );
    }

    if ( tip.isEmpty() && idx.sibling( idx.row(), dbtmPkCol ).data( Qt::UserRole + 1 ).toBool() )
    {
      if ( !idx.sibling( idx.row(), dbtmPkCol ).data( Qt::UserRole + 2 ).toBool() )
        tip = tr( "Select a primary key" );
    }

    for ( int i = 0; i < dbtmColumns; i++ )
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
        if ( i == dbtmOwner || i == dbtmTable || i == dbtmGeomCol )
        {
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          item->setToolTip( tip );
        }
      }
    }
  }

  return true;
}

QString QgsOracleTableModel::layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata )
{
  if ( !index.isValid() )
  {
    QgsDebugMsg( "invalid index" );
    return QString::null;
  }

  QGis::WkbType wkbType = ( QGis::WkbType ) itemFromIndex( index.sibling( index.row(), dbtmType ) )->data( Qt::UserRole + 2 ).toInt();
  if ( wkbType == QGis::WKBUnknown )
  {
    QgsDebugMsg( "unknown geometry type" );
    // no geometry type selected
    return QString::null;
  }

  QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), dbtmPkCol ) );
  QString pkColumnName = pkItem->data( Qt::DisplayRole ).toString();
  bool isView = pkItem->data( Qt::UserRole + 1 ).toBool();
  bool isSet  = pkItem->data( Qt::UserRole + 2 ).toBool();

  if ( isView && !isSet )
  {
    // no valid primary candidate selected
    QgsDebugMsg( "no pk candidate selected" );
    return QString::null;
  }

  QString ownerName = index.sibling( index.row(), dbtmOwner ).data( Qt::DisplayRole ).toString();
  QString tableName = index.sibling( index.row(), dbtmTable ).data( Qt::DisplayRole ).toString();

  QString geomColumnName;
  QString srid;
  if ( wkbType != QGis::WKBNoGeometry )
  {
    geomColumnName = index.sibling( index.row(), dbtmGeomCol ).data( Qt::DisplayRole ).toString();

    srid = index.sibling( index.row(), dbtmSrid ).data( Qt::DisplayRole ).toString();
    bool ok;
    srid.toInt( &ok );
    if ( !ok )
    {
      QgsDebugMsg( "srid not numeric" );
      return QString::null;
    }
  }

  bool selectAtId = itemFromIndex( index.sibling( index.row(), dbtmSelectAtId ) )->checkState() == Qt::Checked;
  QString sql = index.sibling( index.row(), dbtmSql ).data( Qt::DisplayRole ).toString();

  QgsDataSourceURI uri( connInfo );
  uri.setDataSource( ownerName, tableName, geomColumnName, sql, pkColumnName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );

  QgsDebugMsg( QString( "returning uri %1" ).arg( uri.uri() ) );
  return uri.uri();
}
