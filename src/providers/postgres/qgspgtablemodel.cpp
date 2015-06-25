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

#include <climits>

QgsPgTableModel::QgsPgTableModel()
    : QStandardItemModel()
    , mTableCount( 0 )
{
  QStringList headerLabels;
  headerLabels << tr( "Schema" );
  headerLabels << tr( "Table" );
  headerLabels << tr( "Column" );
  headerLabels << tr( "Data Type" );
  headerLabels << tr( "Spatial Type" );
  headerLabels << tr( "SRID" );
  headerLabels << tr( "Primary Key" );
  headerLabels << tr( "Select at id" );
  headerLabels << tr( "Sql" );
  setHorizontalHeaderLabels( headerLabels );
}

QgsPgTableModel::~QgsPgTableModel()
{
}

void QgsPgTableModel::addTableEntry( const QgsPostgresLayerProperty& layerProperty )
{
  QgsDebugMsg( layerProperty.toString() );

  // is there already a root item with the given scheme Name?
  QStandardItem *schemaItem = 0;

  for ( int i = 0; i < layerProperty.size(); i++ )
  {
    QGis::WkbType wkbType = layerProperty.types[ i ];
    int srid = layerProperty.srids[ i ];

    if ( wkbType == QGis::WKBUnknown && layerProperty.geometryColName.isEmpty() )
    {
      wkbType = QGis::WKBNoGeometry;
    }

    QString tip;
    if ( wkbType == QGis::WKBUnknown )
    {
      tip = tr( "Specify a geometry type" );
    }
    else if ( wkbType != QGis::WKBNoGeometry && srid == INT_MIN )
    {
      tip = tr( "Enter a SRID" );
    }
    else if ( layerProperty.pkCols.size() > 0 )
    {
      tip = tr( "Select a primary key" );
    }

    QStandardItem *schemaNameItem = new QStandardItem( layerProperty.schemaName );
    QStandardItem *typeItem = new QStandardItem( iconForWkbType( wkbType ), wkbType == QGis::WKBUnknown ? tr( "Select..." ) : QgsPostgresConn::displayStringForWkbType( wkbType ) );
    typeItem->setData( wkbType == QGis::WKBUnknown, Qt::UserRole + 1 );
    typeItem->setData( wkbType, Qt::UserRole + 2 );
    if ( wkbType == QGis::WKBUnknown )
      typeItem->setFlags( typeItem->flags() | Qt::ItemIsEditable );

    QStandardItem *geomTypeItem = new QStandardItem( QgsPostgresConn::displayStringForGeomType( layerProperty.geometryColType ) );

    QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
    QStandardItem *geomItem  = new QStandardItem( layerProperty.geometryColName );
    QStandardItem *sridItem  = new QStandardItem( wkbType != QGis::WKBNoGeometry ? QString::number( srid ) : "" );
    sridItem->setEditable( wkbType != QGis::WKBNoGeometry && srid == INT_MIN );
    if ( sridItem->isEditable() )
    {
      sridItem->setText( tr( "Enter..." ) );
      sridItem->setFlags( sridItem->flags() | Qt::ItemIsEditable );
    }

    QStandardItem *pkItem = new QStandardItem( "" );
    if ( layerProperty.pkCols.size() > 0 )
    {
      pkItem->setText( tr( "Select..." ) );
      pkItem->setFlags( pkItem->flags() | Qt::ItemIsEditable );
    }
    else
      pkItem->setFlags( pkItem->flags() & ~Qt::ItemIsEditable );

    pkItem->setData( layerProperty.pkCols, Qt::UserRole + 1 );
    pkItem->setData( "", Qt::UserRole + 2 );

    QStandardItem *selItem = new QStandardItem( "" );
    selItem->setFlags( selItem->flags() | Qt::ItemIsUserCheckable );
    selItem->setCheckState( Qt::Checked );
    selItem->setToolTip( tr( "Disable 'Fast Access to Features at ID' capability to force keeping the attribute table in memory (e.g. in case of expensive views)." ) );

    QStandardItem* sqlItem = new QStandardItem( layerProperty.sql );

    QList<QStandardItem*> childItemList;

    childItemList << schemaNameItem;
    childItemList << tableItem;
    childItemList << geomItem;
    childItemList << geomTypeItem;
    childItemList << typeItem;
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

        if ( item == schemaNameItem || item == tableItem || item == geomItem )
        {
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          item->setToolTip( tip );
        }
      }
    }

    if ( !schemaItem )
    {
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
    }

    schemaItem->appendRow( childItemList );

    ++mTableCount;
  }
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

QIcon QgsPgTableModel::iconForWkbType( QGis::WkbType type )
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

bool QgsPgTableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
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

      if ( !ok || srid == INT_MIN )
        tip = tr( "Enter a SRID" );
    }

    QStringList pkCols = idx.sibling( idx.row(), dbtmPkCol ).data( Qt::UserRole + 1 ).toStringList();
    if ( tip.isEmpty() && pkCols.size() > 0 )
    {
      if ( !pkCols.contains( idx.sibling( idx.row(), dbtmPkCol ).data().toString() ) )
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
        if ( i == dbtmSchema || i == dbtmTable || i == dbtmGeomCol )
        {
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          item->setToolTip( tip );
        }
      }
    }
  }

  return true;
}

QString QgsPgTableModel::layerURI( const QModelIndex &index, const QString& connInfo, bool useEstimatedMetadata )
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
  QString pkColumnName = pkItem->data( Qt::UserRole + 2 ).toString();

  if ( pkItem->data( Qt::UserRole + 1 ).toStringList().size() > 0 &&
       !pkItem->data( Qt::UserRole + 1 ).toStringList().contains( pkColumnName ) )
  {
    // no valid primary candidate selected
    QgsDebugMsg( "no pk candidate selected" );
    return QString::null;
  }

  QString schemaName = index.sibling( index.row(), dbtmSchema ).data( Qt::DisplayRole ).toString();
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
  uri.setDataSource( schemaName, tableName, geomColumnName, sql, pkColumnName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );

  QgsDebugMsg( QString( "returning uri %1" ).arg( uri.uri() ) );
  return uri.uri();
}
