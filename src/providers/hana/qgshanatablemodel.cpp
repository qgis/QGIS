/***************************************************************************
  qgshanatablemodel.cpp
  --------------------------------------
  Date      : 31-05-2019
  Copyright : (C) SAP SE
  Author    : Maxim Rylov
***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgshanaprimarykeys.h"
#include "qgshanatablemodel.h"
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgslogger.h"

QgsHanaTableModel::QgsHanaTableModel( QObject *parent )
  : QgsAbstractDbTableModel( parent )

{
  mColumns << tr( "Schema" )
           << tr( "Table" )
           << tr( "Comment" )
           << tr( "Column" )
           << tr( "Type" )
           << tr( "SRID" )
           << tr( "Feature id" )
           << tr( "Select at id" )
           << tr( "SQL" );
  setHorizontalHeaderLabels( mColumns );
}

QStringList QgsHanaTableModel::columns() const
{
  return mColumns;
}

int QgsHanaTableModel::defaultSearchColumn() const
{
  return static_cast<int>( DbtmTable );
}

bool QgsHanaTableModel::searchableColumn( int column ) const
{
  Columns col = static_cast<Columns>( column );
  switch ( col )
  {
    case DbtmSchema:
    case DbtmTable:
    case DbtmComment:
    case DbtmGeomCol:
    case DbtmSrid:
    case DbtmSql:
      return true;

    case DbtmGeomType:
    case DbtmPkCol:
    case DbtmSelectAtId:
      return false;
  }
  BUILTIN_UNREACHABLE
}

void QgsHanaTableModel::addTableEntry( const QString &connName, const QgsHanaLayerProperty &layerProperty )
{
  QgsWkbTypes::Type wkbType = layerProperty.type;
  int srid = layerProperty.srid;

  if ( wkbType == QgsWkbTypes::Unknown && layerProperty.geometryColName.isEmpty() )
    wkbType = QgsWkbTypes::NoGeometry;

  bool withTipButSelectable = false;
  QString tip;
  if ( wkbType == QgsWkbTypes::Unknown )
    tip = tr( "Specify a geometry type in the '%1' column" ).arg( tr( "Data Type" ) );
  else if ( wkbType != QgsWkbTypes::NoGeometry && srid == std::numeric_limits<int>::min() )
    tip = tr( "Enter a SRID into the '%1' column" ).arg( tr( "SRID" ) );
  else if ( !layerProperty.pkCols.empty() )
  {
    tip = tr( "Select columns in the '%1' column that uniquely identify features of this layer" ).arg( tr( "Feature id" ) );
    withTipButSelectable = true;
  }

  QStandardItem *schemaNameItem = new QStandardItem( layerProperty.schemaName );
  QStandardItem *typeItem = new QStandardItem( iconForWkbType( wkbType ),
      wkbType == QgsWkbTypes::Unknown ? tr( "Select…" ) : QgsWkbTypes::displayString( wkbType ) );
  typeItem->setData( wkbType == QgsWkbTypes::Unknown, Qt::UserRole + 1 );
  typeItem->setData( wkbType, Qt::UserRole + 2 );
  if ( wkbType == QgsWkbTypes::Unknown )
    typeItem->setFlags( typeItem->flags() | Qt::ItemIsEditable );

  QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
  QStandardItem *commentItem = new QStandardItem( layerProperty.tableComment );
  QStandardItem *geomItem = new QStandardItem( layerProperty.geometryColName );
  QStandardItem *sridItem = new QStandardItem( wkbType != QgsWkbTypes::NoGeometry ? QString::number( srid ) : QString() );
  sridItem->setEditable( wkbType != QgsWkbTypes::NoGeometry && srid < 0 );
  if ( sridItem->isEditable() )
  {
    sridItem->setText( tr( "Enter…" ) );
    sridItem->setFlags( sridItem->flags() | Qt::ItemIsEditable );
  }

  QStandardItem *pkItem = new QStandardItem( QString() );
  if ( !layerProperty.pkCols.isEmpty() )
  {
    pkItem->setText( tr( "Select…" ) );
    pkItem->setFlags( pkItem->flags() | Qt::ItemIsEditable );
  }
  else
    pkItem->setFlags( pkItem->flags() & ~Qt::ItemIsEditable );

  pkItem->setData( layerProperty.pkCols, Qt::UserRole + 1 );

  QgsHanaSettings settings( connName, true );
  QStringList pkColumns;
  if ( !layerProperty.pkCols.isEmpty() )
  {
    QStringList pkColumnsStored = settings.keyColumns( layerProperty.schemaName, layerProperty.tableName );
    if ( !pkColumnsStored.empty() )
    {
      // We check whether the primary key columns still exist.
      auto intersection = qgis::listToSet( pkColumnsStored ).intersect( qgis::listToSet( layerProperty.pkCols ) );
      if ( intersection.size() == pkColumnsStored.size() )
        pkColumns = pkColumnsStored;
    }
  }

  pkItem->setData( pkColumns, Qt::UserRole + 2 );
  if ( !pkColumns.isEmpty() )
    pkItem->setText( pkColumns.join( ',' ) );

  QStandardItem *selItem = new QStandardItem( QString( ) );
  selItem->setFlags( selItem->flags() | Qt::ItemIsUserCheckable );
  selItem->setCheckState( Qt::Checked );
  selItem->setToolTip( tr( "Disable 'Fast Access to Features at ID' capability to force keeping "
                           "the attribute table in memory (e.g. in case of expensive views)." ) );

  QStandardItem *sqlItem = new QStandardItem( layerProperty.sql );

  QList<QStandardItem *> childItemList;

  childItemList << schemaNameItem;
  childItemList << tableItem;
  childItemList << commentItem;
  childItemList << geomItem;
  childItemList << typeItem;
  childItemList << sridItem;
  childItemList << pkItem;
  childItemList << selItem;
  childItemList << sqlItem;

  for ( QStandardItem *item :  std::as_const( childItemList ) )
  {
    if ( tip.isEmpty() || withTipButSelectable )
      item->setFlags( item->flags() | Qt::ItemIsSelectable );
    else
      item->setFlags( item->flags() & ~Qt::ItemIsSelectable );

    if ( tip.isEmpty() )
    {
      item->setToolTip( QString( ) );
    }
    else
    {
      if ( item == schemaNameItem )
        item->setData( QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) ), Qt::DecorationRole );

      if ( item == schemaNameItem || item == tableItem || item == geomItem )
        item->setToolTip( tip );
    }
  }

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

  schemaItem->appendRow( childItemList );

  ++mTableCount;
}

void QgsHanaTableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
    return;

  //find out schema name and table name
  QModelIndex schemaSibling = index.sibling( index.row(), DbtmSchema );
  QModelIndex tableSibling = index.sibling( index.row(), DbtmTable );
  QModelIndex geomSibling = index.sibling( index.row(), DbtmGeomCol );

  if ( !schemaSibling.isValid() || !tableSibling.isValid() || !geomSibling.isValid() )
    return;

  QString schemaName = itemFromIndex( schemaSibling )->text();
  QString tableName = itemFromIndex( tableSibling )->text();
  QString geomName = itemFromIndex( geomSibling )->text();

  QList<QStandardItem *> schemaItems = findItems( schemaName, Qt::MatchExactly, DbtmSchema );
  if ( schemaItems.size() < 1 )
    return;

  QStandardItem *schemaItem = schemaItems.at( DbtmSchema );

  int n = schemaItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    QModelIndex currentChildIndex = indexFromItem( schemaItem->child( i, DbtmSchema ) );
    if ( !currentChildIndex.isValid() )
      continue;

    QModelIndex currentTableIndex = currentChildIndex.sibling( i, DbtmTable );
    if ( !currentTableIndex.isValid() )
      continue;

    QModelIndex currentGeomIndex = currentChildIndex.sibling( i, DbtmGeomCol );
    if ( !currentGeomIndex.isValid() )
      continue;

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

QIcon QgsHanaTableModel::iconForWkbType( QgsWkbTypes::Type type )
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
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLayer.png" ) );
  }
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLayer.png" ) );
}

bool QgsHanaTableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
  if ( !QStandardItemModel::setData( idx, value, role ) )
    return false;

  if ( idx.column() == DbtmGeomType || idx.column() == DbtmSrid || idx.column() == DbtmPkCol )
  {
    QgsWkbTypes::Type wkbType = static_cast<QgsWkbTypes::Type>( idx.sibling( idx.row(), DbtmGeomType ).data( Qt::UserRole + 2 ).toInt() );

    QString tip;
    if ( wkbType == QgsWkbTypes::Unknown )
    {
      tip = tr( "Specify a geometry type in the '%1' column" ).arg( tr( "Data Type" ) );
    }
    else if ( wkbType != QgsWkbTypes::NoGeometry )
    {
      bool ok;
      int srid = idx.sibling( idx.row(), DbtmSrid ).data().toInt( &ok );

      if ( !ok || srid == std::numeric_limits<int>::min() )
        tip = tr( "Enter a SRID into the '%1' column" ).arg( tr( "SRID" ) );
    }

    QStringList pkCols = idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 1 ).toStringList();
    if ( tip.isEmpty() && !pkCols.isEmpty() )
    {
      QSet<QString> s0( qgis::listToSet( idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 2 ).toStringList() ) );
      QSet<QString> s1( qgis::listToSet( pkCols ) );
      if ( !s0.intersects( s1 ) )
        tip = tr( "Select columns in the '%1' column that uniquely identify features of this layer" ).arg( tr( "Feature id" ) );
    }

    for ( int i = 0; i < columnCount(); i++ )
    {
      QStandardItem *item = itemFromIndex( idx.sibling( idx.row(), i ) );
      if ( tip.isEmpty() )
      {
        if ( i == DbtmSchema )
        {
          item->setData( QVariant(), Qt::DecorationRole );
        }

        item->setFlags( item->flags() | Qt::ItemIsSelectable );
        item->setToolTip( QString( ) );
      }
      else
      {
        item->setFlags( item->flags() & ~Qt::ItemIsSelectable );

        if ( i == DbtmSchema )
          item->setData( QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) ), Qt::DecorationRole );

        if ( i == DbtmSchema || i == DbtmTable || i == DbtmGeomCol )
        {
          item->setFlags( item->flags() );
          item->setToolTip( tip );
        }
      }
    }
  }

  return true;
}

QString QgsHanaTableModel::layerURI( const QModelIndex &index, const QString &connName, const QString &connInfo )
{
  if ( !index.isValid() )
    return QString();

  QgsWkbTypes::Type wkbType = static_cast<QgsWkbTypes::Type>( itemFromIndex( index.sibling( index.row(), DbtmGeomType ) )->data( Qt::UserRole + 2 ).toInt() );
  if ( wkbType == QgsWkbTypes::Unknown )
    // no geometry type selected
    return QString();

  QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), DbtmPkCol ) );
  const QSet<QString> pkColumnsAll( qgis::listToSet( pkItem->data( Qt::UserRole + 1 ).toStringList() ) );
  const QSet<QString> pkColumnsSelected( qgis::listToSet( pkItem->data( Qt::UserRole + 2 ).toStringList() ) );
  if ( !pkColumnsAll.isEmpty() && !pkColumnsAll.intersects( pkColumnsSelected ) )
  {
    QgsDebugMsg( QStringLiteral( "no pk candidate selected" ) );
    return QString();
  }

  QString schemaName = index.sibling( index.row(), DbtmSchema ).data( Qt::DisplayRole ).toString();
  QString tableName = index.sibling( index.row(), DbtmTable ).data( Qt::DisplayRole ).toString();

  QStringList pkColumns = qgis::setToList( pkColumnsSelected );

  QgsHanaSettings settings( connName, true );
  settings.setKeyColumns( schemaName, tableName, pkColumns );
  settings.save();

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
  uri.setDataSource( schemaName, tableName, geomColumnName, sql,  QgsHanaPrimaryKeyUtils::buildUriKey( pkColumns ) );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );

  return uri.uri();
}
