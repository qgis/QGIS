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
#include "qgsmssqlconnection.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsiconutils.h"

QgsMssqlTableModel::QgsMssqlTableModel( QObject *parent )
  : QgsAbstractDbTableModel( parent )
{
  mColumns  << tr( "Schema" )
            << tr( "Table" )
            << tr( "Type" )
            << tr( "Geometry column" )
            << tr( "SRID" )
            << tr( "Primary key column" )
            << tr( "Select at id" )
            << tr( "SQL" )
            << tr( "View" );
  setHorizontalHeaderLabels( mColumns );
}

QStringList QgsMssqlTableModel::columns() const
{
  return mColumns;
}

int QgsMssqlTableModel::defaultSearchColumn() const
{
  return static_cast<int>( DbtmTable );
}

bool QgsMssqlTableModel::searchableColumn( int column ) const
{
  Columns col = static_cast<Columns>( column );
  switch ( col )
  {
    case DbtmSchema:
    case DbtmTable:
    case DbtmGeomCol:
    case DbtmType:
    case DbtmSrid:
    case DbtmSql:
      return true;

    case DbtmPkCol:
    case DbtmSelectAtId:
    case DbtmView:
      return false;
  }

  BUILTIN_UNREACHABLE
}

void QgsMssqlTableModel::addTableEntry( const QgsMssqlLayerProperty &layerProperty )
{
  QgsDebugMsg( QStringLiteral( "%1.%2.%3 type=%4 srid=%5 pk=%6 sql=%7 view=%8" )
               .arg( layerProperty.schemaName,
                     layerProperty.tableName,
                     layerProperty.geometryColName,
                     layerProperty.type,
                     layerProperty.srid,
                     layerProperty.pkCols.join( ',' ),
                     layerProperty.sql,
                     layerProperty.isView ? "yes" : "no" ) );

  // is there already a root item with the given scheme Name?
  QStandardItem *schemaItem = nullptr;
  QList<QStandardItem *> schemaItems = findItems( layerProperty.schemaName, Qt::MatchExactly, DbtmSchema );

  // there is already an item for this schema
  if ( !schemaItems.isEmpty() )
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

  QgsWkbTypes::Type wkbType = QgsMssqlTableModel::wkbTypeFromMssql( layerProperty.type );
  if ( wkbType == QgsWkbTypes::Unknown && layerProperty.geometryColName.isEmpty() )
  {
    wkbType = QgsWkbTypes::NoGeometry;
  }

  bool needToDetect = wkbType == QgsWkbTypes::Unknown && layerProperty.type != QLatin1String( "GEOMETRYCOLLECTION" );

  QList<QStandardItem *> childItemList;

  QStandardItem *schemaNameItem = new QStandardItem( layerProperty.schemaName );
  schemaNameItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  QStandardItem *typeItem = new QStandardItem( QgsIconUtils::iconForWkbType( wkbType ),
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

  QStandardItem *selItem = new QStandardItem( QString() );
  selItem->setFlags( selItem->flags() | Qt::ItemIsUserCheckable );
  selItem->setCheckState( Qt::Checked );
  selItem->setToolTip( tr( "Disable 'Fast Access to Features at ID' capability to force keeping the attribute table in memory (e.g. in case of expensive views)." ) );

  QStandardItem *sqlItem = new QStandardItem( layerProperty.sql );

  QStandardItem *isViewItem = new QStandardItem( layerProperty.isView ? tr( "yes" ) : tr( "no" ) );
  isViewItem->setData( layerProperty.isView, Qt::UserRole + 1 );

  childItemList << schemaNameItem;
  childItemList << tableItem;
  childItemList << typeItem;
  childItemList << geomItem;
  childItemList << sridItem;
  childItemList << pkItem;
  childItemList << selItem;
  childItemList << sqlItem;
  childItemList << isViewItem;

  bool detailsFromThread = needToDetect ||
                           ( wkbType != QgsWkbTypes::NoGeometry && layerProperty.srid.isEmpty() );

  if ( detailsFromThread || pkText == tr( "Select…" ) )
  {
    Qt::ItemFlags flags = Qt::ItemIsSelectable;
    if ( detailsFromThread )
      flags |= Qt::ItemIsEnabled;

    const auto constChildItemList = childItemList;
    for ( QStandardItem *item : constChildItemList )
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

void QgsMssqlTableModel::setGeometryTypesForTable( QgsMssqlLayerProperty layerProperty )
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList typeList = layerProperty.type.split( ',', QString::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ',', QString::SkipEmptyParts );
#else
  QStringList typeList = layerProperty.type.split( ',', Qt::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ',', Qt::SkipEmptyParts );
#endif
  Q_ASSERT( typeList.size() == sridList.size() );

  //find schema item and table item
  QStandardItem *schemaItem = nullptr;
  QList<QStandardItem *> schemaItems = findItems( layerProperty.schemaName, Qt::MatchExactly, DbtmSchema );

  if ( schemaItems.size() < 1 )
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
    row.reserve( columnCount() );

    for ( int j = 0; j < columnCount(); j++ )
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

        const auto constRow = row;
        for ( QStandardItem *item : constRow )
        {
          item->setFlags( item->flags() | Qt::ItemIsEnabled );
        }
      }
      else
      {
        // update existing row
        QgsWkbTypes::Type wkbType = QgsMssqlTableModel::wkbTypeFromMssql( typeList.at( 0 ) );

        row[ DbtmType ]->setIcon( QgsIconUtils::iconForWkbType( wkbType ) );
        row[ DbtmType ]->setText( QgsWkbTypes::translatedDisplayString( wkbType ) );
        row[ DbtmType ]->setData( false, Qt::UserRole + 1 );
        row[ DbtmType ]->setData( wkbType, Qt::UserRole + 2 );

        row[ DbtmSrid ]->setText( sridList.at( 0 ) );

        Qt::ItemFlags flags = Qt::ItemIsEnabled;
        if ( layerProperty.pkCols.size() < 2 )
          flags |= Qt::ItemIsSelectable;

        for ( QStandardItem *item : std::as_const( row ) )
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

bool QgsMssqlTableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
  if ( !QStandardItemModel::setData( idx, value, role ) )
    return false;

  if ( idx.column() == DbtmType || idx.column() == DbtmSrid || idx.column() == DbtmPkCol )
  {
    QgsWkbTypes::Type wkbType = static_cast< QgsWkbTypes::Type >( idx.sibling( idx.row(), DbtmType ).data( Qt::UserRole + 2 ).toInt() );

    bool ok = wkbType != QgsWkbTypes::Unknown;

    if ( ok && wkbType != QgsWkbTypes::NoGeometry )
      idx.sibling( idx.row(), DbtmSrid ).data().toInt( &ok );

    QStringList pkCols = idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 1 ).toStringList();
    if ( ok && !pkCols.isEmpty() )
      ok = pkCols.contains( idx.sibling( idx.row(), DbtmPkCol ).data().toString() );

    for ( int i = 0; i < columnCount(); i++ )
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

QString QgsMssqlTableModel::layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata, bool disableInvalidGeometryHandling )
{
  if ( !index.isValid() )
    return QString();

  QgsWkbTypes::Type wkbType = static_cast< QgsWkbTypes::Type >( itemFromIndex( index.sibling( index.row(), DbtmType ) )->data( Qt::UserRole + 2 ).toInt() );
  if ( wkbType == QgsWkbTypes::Unknown )
    // no geometry type selected
    return QString();

  QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), DbtmPkCol ) );
  QString pkColumnName = pkItem->data( Qt::UserRole + 2 ).toString();

  if ( !pkItem->data( Qt::UserRole + 1 ).toStringList().isEmpty() &&
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
    ( void )srid.toInt( &ok );
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

  uri.setParam( QStringLiteral( "disableInvalidGeometryHandling" ), disableInvalidGeometryHandling ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  if ( QgsMssqlConnection::geometryColumnsOnly( mConnectionName ) )
  {
    uri.setParam( QStringLiteral( "extentInGeometryColumns" ), QgsMssqlConnection::extentInGeometryColumns( mConnectionName ) ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  }

  QStandardItem *isViewItem = itemFromIndex( index.sibling( index.row(), DbtmView ) );

  if ( isViewItem->data( Qt::UserRole + 1 ).toBool() )
    uri.setParam( QStringLiteral( "primaryKeyInGeometryColumns" ), QgsMssqlConnection::primaryKeyInGeometryColumns( mConnectionName ) ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  return uri.uri();
}

QgsWkbTypes::Type QgsMssqlTableModel::wkbTypeFromMssql( QString type )
{
  type = type.toUpper();
  return QgsWkbTypes::parseType( type );
}

void QgsMssqlTableModel::setConnectionName( const QString &connectionName )
{
  mConnectionName = connectionName;
}
