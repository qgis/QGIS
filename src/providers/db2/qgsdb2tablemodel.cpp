/***************************************************************************
  qgsdb2tablemodel.h - description
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
  Adapted from MSSQL provider by Tamas Szekeres
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsdb2tablemodel.h"
#include "qgsdataitem.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsapplication.h"

QgsDb2TableModel::QgsDb2TableModel()
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

void QgsDb2TableModel::addTableEntry( const QgsDb2LayerProperty &layerProperty )
{
  QgsDebugMsg( QStringLiteral( " DB2 **** %1.%2.%3 type=%4 srid=%5 pk=%6 sql=%7" )
               .arg( layerProperty.schemaName )
               .arg( layerProperty.tableName )
               .arg( layerProperty.geometryColName )
               .arg( layerProperty.type )
               .arg( layerProperty.srid )
               .arg( layerProperty.pkCols.join( ',' ) )
               .arg( layerProperty.sql ) );

  // is there already a root item with the given scheme Name?
  QStandardItem *schemaItem = nullptr;
  QList<QStandardItem *> schemaItems = findItems( layerProperty.schemaName, Qt::MatchExactly, DbtmSchema );

  // there is already an item for this schema
  if ( schemaItems.size() > 0 )
  {
    schemaItem = schemaItems.at( DbtmSchema );
  }
  else
  {
    // create a new toplevel item for this schema
    schemaItem = new QStandardItem( layerProperty.schemaName );
    schemaItem->setFlags( Qt::ItemIsEnabled );
    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), schemaItem );
  }

  QgsWkbTypes::Type wkbType = QgsDb2TableModel::wkbTypeFromDb2( layerProperty.type );
  if ( wkbType == QgsWkbTypes::Unknown && layerProperty.geometryColName.isEmpty() )
  {
    wkbType = QgsWkbTypes::NoGeometry;
  }

  bool needToDetect = wkbType == QgsWkbTypes::Unknown && layerProperty.type != QLatin1String( "GEOMETRYCOLLECTION" );

  QList<QStandardItem *> childItemList;

  QStandardItem *schemaNameItem = new QStandardItem( layerProperty.schemaName );
  schemaNameItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  QStandardItem *typeItem = new QStandardItem( iconForWkbType( wkbType ),
      needToDetect
      ? tr( "Detecting…" )
      : QgsWkbTypes::displayString( wkbType ) );
  typeItem->setData( needToDetect, Qt::UserRole + 1 );
  typeItem->setData( wkbType, Qt::UserRole + 2 );

  QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
  QStandardItem *geomItem = new QStandardItem( layerProperty.geometryColName );
  QStandardItem *sridItem = new QStandardItem( layerProperty.srid );
  sridItem->setEditable( false );

  QString pkText;
  QString pkCol;
  switch ( layerProperty.pkCols.size() )
  {
    case 0:
      break;
    case 1:
      pkText = layerProperty.pkCols[0];
      pkCol = pkText;
      break;
    default:
      pkText = tr( "Select…" );
      break;
  }

  QStandardItem *pkItem = new QStandardItem( pkText );
  if ( pkText == tr( "Select…" ) )
    pkItem->setFlags( pkItem->flags() | Qt::ItemIsEditable );

  pkItem->setData( layerProperty.pkCols, Qt::UserRole + 1 );
  pkItem->setData( pkCol, Qt::UserRole + 2 );

  QStandardItem *selItem = new QStandardItem( QLatin1String( "" ) );
  selItem->setFlags( selItem->flags() | Qt::ItemIsUserCheckable );
  selItem->setCheckState( Qt::Checked );
  selItem->setToolTip( tr( "Disable 'Fast Access to Features at ID' capability to force keeping the attribute table in memory (e.g. in case of expensive views)." ) );

  QStandardItem *sqlItem = new QStandardItem( layerProperty.sql );

  childItemList << schemaNameItem;
  childItemList << tableItem;
  childItemList << typeItem;
  childItemList << geomItem;
  childItemList << sridItem;
  childItemList << pkItem;
  childItemList << selItem;
  childItemList << sqlItem;

  bool detailsFromThread = needToDetect ||
                           ( wkbType != QgsWkbTypes::NoGeometry && layerProperty.srid.isEmpty() );

  if ( detailsFromThread || pkText == tr( "Select…" ) )
  {
    Qt::ItemFlags flags = Qt::ItemIsSelectable;
    if ( detailsFromThread )
      flags |= Qt::ItemIsEnabled;

    for ( QStandardItem *item : qgis::as_const( childItemList ) )
    {
      item->setFlags( item->flags() & ~flags );
    }
  }

  schemaItem->appendRow( childItemList );

  ++mTableCount;
}

void QgsDb2TableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }

  //find out schema name and table name
  QModelIndex schemaSibling = index.sibling( index.row(), DbtmSchema );
  QModelIndex tableSibling = index.sibling( index.row(), DbtmTable );
  QModelIndex geomSibling = index.sibling( index.row(), DbtmGeomCol );

  if ( !schemaSibling.isValid() || !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  QString schemaName = itemFromIndex( schemaSibling )->text();
  QString tableName = itemFromIndex( tableSibling )->text();
  QString geomName = itemFromIndex( geomSibling )->text();

  QList<QStandardItem *> schemaItems = findItems( schemaName, Qt::MatchExactly, DbtmSchema );
  if ( schemaItems.size() < 1 )
  {
    return;
  }

  QStandardItem *schemaItem = schemaItems.at( DbtmSchema );

  int n = schemaItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    QModelIndex currentChildIndex = indexFromItem( schemaItem->child( i, DbtmSchema ) );
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

void QgsDb2TableModel::setGeometryTypesForTable( QgsDb2LayerProperty layerProperty )
{
  QStringList typeList = layerProperty.type.split( ',', QString::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ',', QString::SkipEmptyParts );
  Q_ASSERT( typeList.size() == sridList.size() );

  //find schema item and table item
  QStandardItem *schemaItem = nullptr;
  QList<QStandardItem *> schemaItems = findItems( layerProperty.schemaName, Qt::MatchExactly, DbtmSchema );

  if ( schemaItems.empty() )
  {
    return;
  }

  schemaItem = schemaItems.at( 0 );

  int n = schemaItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    QModelIndex currentChildIndex = indexFromItem( schemaItem->child( i, DbtmSchema ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }

    QList<QStandardItem *> row;

    row.reserve( DbtmColumns );
    for ( int j = 0; j < DbtmColumns; j++ )
    {
      row << itemFromIndex( currentChildIndex.sibling( i, j ) );
    }

    if ( row[ DbtmTable ]->text() == layerProperty.tableName && row[ DbtmGeomCol ]->text() == layerProperty.geometryColName )
    {
      row[ DbtmSrid ]->setText( layerProperty.srid );

      if ( typeList.isEmpty() )
      {
        row[ DbtmType ]->setText( tr( "Select…" ) );
        row[ DbtmType ]->setFlags( row[ DbtmType ]->flags() | Qt::ItemIsEditable );

        row[ DbtmSrid ]->setText( tr( "Enter…" ) );
        row[ DbtmSrid ]->setFlags( row[ DbtmSrid ]->flags() | Qt::ItemIsEditable );

        for ( QStandardItem *item : qgis::as_const( row ) )
        {
          item->setFlags( item->flags() | Qt::ItemIsEnabled );
        }
      }
      else
      {
        // update existing row
        QgsWkbTypes::Type wkbType = QgsDb2TableModel::wkbTypeFromDb2( typeList.at( 0 ) );

        row[ DbtmType ]->setIcon( iconForWkbType( wkbType ) );
        row[ DbtmType ]->setText( QgsWkbTypes::displayString( wkbType ) );
        row[ DbtmType ]->setData( false, Qt::UserRole + 1 );
        row[ DbtmType ]->setData( wkbType, Qt::UserRole + 2 );

        row[ DbtmSrid ]->setText( sridList.at( 0 ) );

        Qt::ItemFlags flags = Qt::ItemIsEnabled;
        if ( layerProperty.pkCols.size() < 2 )
          flags |= Qt::ItemIsSelectable;

        for ( QStandardItem *item : qgis::as_const( row ) )
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

QIcon QgsDb2TableModel::iconForWkbType( QgsWkbTypes::Type type )
{
  switch ( QgsWkbTypes::geometryType( type ) )

  {
    case QgsWkbTypes::PointGeometry:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) );
    case QgsWkbTypes::LineGeometry:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) );
    case QgsWkbTypes::PolygonGeometry:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) );
    case QgsWkbTypes::NullGeometry:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconTableLayer.svg" ) );
    case QgsWkbTypes::UnknownGeometry:
      break;
  }
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLayer.png" ) );
}

bool QgsDb2TableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
  if ( !QStandardItemModel::setData( idx, value, role ) )
    return false;

  if ( idx.column() == DbtmType || idx.column() == DbtmSrid || idx.column() == DbtmPkCol )
  {
    const QgsWkbTypes::Type wkbType = static_cast< QgsWkbTypes::Type >( idx.sibling( idx.row(), DbtmType ).data( Qt::UserRole + 2 ).toInt() );

    bool ok = wkbType != QgsWkbTypes::Unknown;

    if ( ok && wkbType != QgsWkbTypes::NoGeometry )
      idx.sibling( idx.row(), DbtmSrid ).data().toInt( &ok );

    QStringList pkCols = idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 1 ).toStringList();
    if ( ok && pkCols.size() > 0 )
      ok = pkCols.contains( idx.sibling( idx.row(), DbtmPkCol ).data().toString() );

    for ( int i = 0; i < DbtmColumns; i++ )
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

QString QgsDb2TableModel::layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata )
{
  if ( !index.isValid() )
    return QString();

  const QgsWkbTypes::Type wkbType = static_cast< QgsWkbTypes::Type >( itemFromIndex( index.sibling( index.row(), DbtmType ) )->data( Qt::UserRole + 2 ).toInt() );
  if ( wkbType == QgsWkbTypes::Unknown )
    // no geometry type selected
    return QString();

  QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), DbtmPkCol ) );
  QString pkColumnName = pkItem->data( Qt::UserRole + 2 ).toString();

  if ( pkItem->data( Qt::UserRole + 1 ).toStringList().size() > 0 &&
       !pkItem->data( Qt::UserRole + 1 ).toStringList().contains( pkColumnName ) )
    // no valid primary candidate selected
    return QString();

  QString schemaName = index.sibling( index.row(), DbtmSchema ).data( Qt::DisplayRole ).toString();
  QString tableName = index.sibling( index.row(), DbtmTable ).data( Qt::DisplayRole ).toString();

  QString geomColumnName;
  QString srid;
  if ( wkbType != QgsWkbTypes::NoGeometry )
  {
    geomColumnName = index.sibling( index.row(), DbtmGeomCol ).data( Qt::DisplayRole ).toString();

    srid = index.sibling( index.row(), DbtmSrid ).data( Qt::DisplayRole ).toString();
    bool ok;
    srid.toInt( &ok );
    if ( !ok )
      return QString();
  }

  bool selectAtId = itemFromIndex( index.sibling( index.row(), DbtmSelectAtId ) )->checkState() == Qt::Checked;
  QString sql = index.sibling( index.row(), DbtmSql ).data( Qt::DisplayRole ).toString();

  QgsDataSourceUri uri( connInfo );
  uri.setDataSource( schemaName, tableName, geomColumnName, sql, pkColumnName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );
  QgsDebugMsg( "Layer URI: " + uri.uri( false ) );
  return uri.uri( false );
}

QgsWkbTypes::Type QgsDb2TableModel::wkbTypeFromDb2( QString type, int dim )
{
  type = type.toUpper();

  if ( dim == 3 )
  {
    if ( type == QLatin1String( "ST_POINT" ) )
      return QgsWkbTypes::Point25D;
    if ( type == QLatin1String( "ST_LINESTRING" ) )
      return QgsWkbTypes::LineString25D;
    if ( type == QLatin1String( "ST_POLYGON" ) )
      return QgsWkbTypes::Polygon25D;
    if ( type == QLatin1String( "ST_MULTIPOINT" ) )
      return QgsWkbTypes::MultiPoint25D;
    if ( type == QLatin1String( "ST_MULTILINESTRING" ) )
      return QgsWkbTypes::MultiLineString25D;
    if ( type == QLatin1String( "ST_MULTIPOLYGON" ) )
      return QgsWkbTypes::MultiPolygon25D;
    if ( type == QLatin1String( "NONE" ) )
      return QgsWkbTypes::NoGeometry;
    else
      return QgsWkbTypes::Unknown;
  }
  else
  {
    if ( type == QLatin1String( "ST_POINT" ) )
      return QgsWkbTypes::Point;
    if ( type == QLatin1String( "ST_LINESTRING" ) )
      return QgsWkbTypes::LineString;
    if ( type == QLatin1String( "ST_POLYGON" ) )
      return QgsWkbTypes::Polygon;
    if ( type == QLatin1String( "ST_MULTIPOINT" ) )
      return QgsWkbTypes::MultiPoint;
    if ( type == QLatin1String( "ST_MULTILINESTRING" ) )
      return QgsWkbTypes::MultiLineString;
    if ( type == QLatin1String( "ST_MULTIPOLYGON" ) )
      return QgsWkbTypes::MultiPolygon;
    if ( type == QLatin1String( "NONE" ) )
      return QgsWkbTypes::NoGeometry;
    else
      return QgsWkbTypes::Unknown;
  }
}
