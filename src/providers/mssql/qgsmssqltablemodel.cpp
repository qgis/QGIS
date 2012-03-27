/***************************************************************************
                         qgsmssqltablemodel.cpp  -  description
                         -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqltablemodel.h"
#include "qgsdataitem.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"

QgsMssqlTableModel::QgsMssqlTableModel()
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

QgsMssqlTableModel::~QgsMssqlTableModel()
{
}

void QgsMssqlTableModel::addTableEntry( QgsMssqlLayerProperty layerProperty )
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

  QGis::WkbType wkbType = QgsMssqlTableModel::wkbTypeFromMssql( layerProperty.type );
  if ( wkbType == QGis::WKBUnknown && layerProperty.geometryColName.isEmpty() )
  {
    wkbType = QGis::WKBNoGeometry;
  }

  bool needToDetect = wkbType == QGis::WKBUnknown && layerProperty.type != "GEOMETRYCOLLECTION";

  QList<QStandardItem*> childItemList;

  QStandardItem *schemaNameItem = new QStandardItem( layerProperty.schemaName );
  schemaNameItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  QStandardItem *typeItem = new QStandardItem( iconForWkbType( wkbType ),
      needToDetect
      ? tr( "Detecting..." )
      : QgsMssqlTableModel::displayStringForWkbType( wkbType ) );
  typeItem->setData( needToDetect, Qt::UserRole + 1 );
  typeItem->setData( wkbType, Qt::UserRole + 2 );

  QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
  QStandardItem *geomItem = new QStandardItem( layerProperty.geometryColName );
  QStandardItem *sridItem = new QStandardItem( layerProperty.srid );
  sridItem->setEditable( false );

  QString pkText, pkCol = "";
  switch ( layerProperty.pkCols.size() )
  {
    case 0:   pkText = ""; break;
    case 1:   pkText = layerProperty.pkCols[0]; pkCol = pkText; break;
    default:  pkText = tr( "Select..." ); break;
  }

  QStandardItem *pkItem = new QStandardItem( pkText );
  if ( pkText == tr( "Select..." ) )
    pkItem->setFlags( pkItem->flags() | Qt::ItemIsEditable );

  pkItem->setData( layerProperty.pkCols, Qt::UserRole + 1 );
  pkItem->setData( pkCol, Qt::UserRole + 2 );

  QStandardItem *selItem = new QStandardItem( "" );
  selItem->setFlags( selItem->flags() | Qt::ItemIsUserCheckable );
  selItem->setCheckState( Qt::Checked );
  selItem->setToolTip( tr( "Disable 'Fast Access to Features at ID' capability to force keeping the attribute table in memory (e.g. in case of expensive views)." ) );

  QStandardItem* sqlItem = new QStandardItem( layerProperty.sql );

  childItemList << schemaNameItem;
  childItemList << tableItem;
  childItemList << typeItem;
  childItemList << geomItem;
  childItemList << sridItem;
  childItemList << pkItem;
  childItemList << selItem;
  childItemList << sqlItem;

  bool detailsFromThread = needToDetect ||
                           ( wkbType != QGis::WKBNoGeometry && layerProperty.srid.isEmpty() );

  if ( detailsFromThread || pkText == tr( "Select..." ) )
  {
    Qt::ItemFlags flags = Qt::ItemIsSelectable;
    if ( detailsFromThread )
      flags |= Qt::ItemIsEnabled;

    foreach( QStandardItem *item, childItemList )
    {
      item->setFlags( item->flags() & ~flags );
    }
  }

  schemaItem->appendRow( childItemList );

  ++mTableCount;
}

void QgsMssqlTableModel::setSql( const QModelIndex &index, const QString &sql )
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

void QgsMssqlTableModel::setGeometryTypesForTable( QgsMssqlLayerProperty layerProperty )
{
  QStringList typeList = layerProperty.type.split( ",", QString::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ",", QString::SkipEmptyParts );
  Q_ASSERT( typeList.size() == sridList.size() );

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

    QList<QStandardItem *> row;

    for ( int j = 0; j < dbtmColumns; j++ )
    {
      row << itemFromIndex( currentChildIndex.sibling( i, j ) );
    }

    if ( row[ dbtmTable ]->text() == layerProperty.tableName && row[ dbtmGeomCol ]->text() == layerProperty.geometryColName )
    {
      row[ dbtmSrid ]->setText( layerProperty.srid );

      if ( typeList.isEmpty() )
      {
        row[ dbtmType ]->setText( tr( "Select..." ) );
        row[ dbtmType ]->setFlags( row[ dbtmType ]->flags() | Qt::ItemIsEditable );

        row[ dbtmSrid ]->setText( tr( "Enter..." ) );
        row[ dbtmSrid ]->setFlags( row[ dbtmSrid ]->flags() | Qt::ItemIsEditable );

        foreach( QStandardItem *item, row )
        {
          item->setFlags( item->flags() | Qt::ItemIsEnabled );
        }
      }
      else
      {
        // update existing row
        QGis::WkbType wkbType = QgsMssqlTableModel::wkbTypeFromMssql( typeList.at( 0 ) );

        row[ dbtmType ]->setIcon( iconForWkbType( wkbType ) );
        row[ dbtmType ]->setText( QgsMssqlTableModel::displayStringForWkbType( wkbType ) );
        row[ dbtmType ]->setData( false, Qt::UserRole + 1 );
        row[ dbtmType ]->setData( wkbType, Qt::UserRole + 2 );

        row[ dbtmSrid ]->setText( sridList.at( 0 ) );

        Qt::ItemFlags flags = Qt::ItemIsEnabled;
        if ( layerProperty.pkCols.size() < 2 )
          flags |= Qt::ItemIsSelectable;

        foreach( QStandardItem *item, row )
        {
          item->setFlags( item->flags() | flags );
        }

        for ( int j = 1; j < typeList.size(); j++ )
        {
          layerProperty.type = typeList[j];
          layerProperty.srid = sridList[j];
          addTableEntry( layerProperty );
        }
      }
    }
  }
}

QIcon QgsMssqlTableModel::iconForWkbType( QGis::WkbType type )
{
  switch ( type )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconPointLayer.png" ) );
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconLineLayer.png" ) );
    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconPolygonLayer.png" ) );
    case QGis::WKBNoGeometry:
      return QIcon( QgsDataItem::getThemePixmap( "/mIconTableLayer.png" ) );
    case QGis::WKBUnknown:
      break;
  }
  return QIcon( QgsDataItem::getThemePixmap( "/mIconLayer.png" ) );
}

bool QgsMssqlTableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
  if ( !QStandardItemModel::setData( idx, value, role ) )
    return false;

  if ( idx.column() == dbtmType || idx.column() == dbtmSrid || idx.column() == dbtmPkCol )
  {
    QGis::GeometryType geomType = ( QGis::GeometryType ) idx.sibling( idx.row(), dbtmType ).data( Qt::UserRole + 2 ).toInt();

    bool ok = geomType != QGis::UnknownGeometry;

    if ( ok && geomType != QGis::NoGeometry )
      idx.sibling( idx.row(), dbtmSrid ).data().toInt( &ok );

    QStringList pkCols = idx.sibling( idx.row(), dbtmPkCol ).data( Qt::UserRole + 1 ).toStringList();
    if ( ok && pkCols.size() > 0 )
      ok = pkCols.contains( idx.sibling( idx.row(), dbtmPkCol ).data().toString() );

    for ( int i = 0; i < dbtmColumns; i++ )
    {
      QStandardItem *item = itemFromIndex( idx.sibling( idx.row(), i ) );
      if ( ok )
        item->setFlags( item->flags() | Qt::ItemIsSelectable );
      else
        item->setFlags( item->flags() & ~Qt::ItemIsSelectable );
    }
  }

  return true;
}

QString QgsMssqlTableModel::layerURI( const QModelIndex &index, QString connInfo, bool useEstimatedMetadata )
{
  if ( !index.isValid() )
    return QString::null;

  QGis::WkbType wkbType = ( QGis::WkbType ) itemFromIndex( index.sibling( index.row(), dbtmType ) )->data( Qt::UserRole + 2 ).toInt();
  if ( wkbType == QGis::WKBUnknown )
    // no geometry type selected
    return QString::null;

  QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), dbtmPkCol ) );
  QString pkColumnName = pkItem->data( Qt::UserRole + 2 ).toString();

  if ( pkItem->data( Qt::UserRole + 1 ).toStringList().size() > 0 &&
       !pkItem->data( Qt::UserRole + 1 ).toStringList().contains( pkColumnName ) )
    // no valid primary candidate selected
    return QString::null;

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
      return QString::null;
  }

  bool selectAtId = itemFromIndex( index.sibling( index.row(), dbtmSelectAtId ) )->checkState() == Qt::Checked;
  QString sql = index.sibling( index.row(), dbtmSql ).data( Qt::DisplayRole ).toString();

  QgsDataSourceURI uri( connInfo );
  uri.setDataSource( schemaName, tableName, geomColumnName, sql, pkColumnName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );

  return uri.uri();
}

QGis::WkbType QgsMssqlTableModel::wkbTypeFromMssql( QString type )
{
  type = type.toUpper();

  if ( type == "POINT" )
  {
    return QGis::WKBPoint;
  }
  else if ( type == "POINTM" )
  {
    return QGis::WKBPoint25D;
  }
  else if ( type == "MULTIPOINT" )
  {
    return QGis::WKBMultiPoint;
  }
  else if ( type == "MULTIPOINTM" )
  {
    return QGis::WKBMultiPoint25D;
  }
  else if ( type == "LINESTRING" )
  {
    return QGis::WKBLineString;
  }
  else if ( type == "LINESTRINGM" )
  {
    return QGis::WKBLineString25D;
  }
  else if ( type == "MULTILINESTRING" )
  {
    return QGis::WKBMultiLineString;
  }
  else if ( type == "MULTILINESTRINGM" )
  {
    return QGis::WKBMultiLineString25D;
  }
  else if ( type == "POLYGON" )
  {
    return QGis::WKBPolygon;
  }
  else if ( type == "POLYGONM" )
  {
    return QGis::WKBPolygon25D;
  }
  else if ( type == "MULTIPOLYGON" )
  {
    return QGis::WKBMultiPolygon;
  }
  else if ( type == "MULTIPOLYGONM" )
  {
    return QGis::WKBMultiPolygon25D;
  }
  else if ( type == "NONE" )
  {
    return QGis::WKBNoGeometry;
  }
  else
  {
    return QGis::WKBUnknown;
  }
}

QString QgsMssqlTableModel::displayStringForWkbType( QGis::WkbType type )
{
  switch ( type )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
      return tr( "Point" );

    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      return tr( "Multipoint" );

    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
      return tr( "Line" );

    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      return tr( "Multiline" );

    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
      return tr( "Polygon" );

    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      return tr( "Multipolygon" );

    case QGis::WKBNoGeometry:
      return tr( "No Geometry" );

    case QGis::WKBUnknown:
      return tr( "Unknown Geometry" );
  }

  Q_ASSERT( !"unexpected wkbType" );
  return QString::null;
}
