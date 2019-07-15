/***************************************************************************
  qgshanatablemodel.cpp
  --------------------------------------
  Date      : 31-05-2019
  Copyright : (C) SAP SE
  Author    : Maksim Rylov
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
#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"
#include "qgshanatablemodel.h"
#include "qgslogger.h"

QgsHanaTableModel::QgsHanaTableModel()
{
  QStringList headerLabels;
  headerLabels << tr( "Schema" );
  headerLabels << tr( "Table" );
  headerLabels << tr( "Comment" );
  headerLabels << tr( "Column" );
  headerLabels << tr( "Spatial Type" );
  headerLabels << tr( "SRID" );
  headerLabels << tr( "Feature ID" );
  headerLabels << tr( "Select at ID" );
  headerLabels << tr( "Sql" );
  setHorizontalHeaderLabels( headerLabels );
}

void QgsHanaTableModel::addTableEntry( const QgsHanaLayerProperty &layerProperty )
{
  // is there already a root item with the given scheme Name?
  QStandardItem *schemaItem = nullptr;

  QgsWkbTypes::Type wkbType = layerProperty.type;
  int srid = layerProperty.srid;

  if ( wkbType == QgsWkbTypes::Unknown && layerProperty.geometryColName.isEmpty() )
    wkbType = QgsWkbTypes::NoGeometry;

  QString tip;

  if ( wkbType == QgsWkbTypes::Unknown )
    tip = tr( "Specify a geometry type in the '%1' column" ).arg( tr( "Data Type" ) );
  else if ( wkbType != QgsWkbTypes::NoGeometry && srid == std::numeric_limits<int>::min() )
    tip = tr( "Enter a SRID into the '%1' column" ).arg( tr( "SRID" ) );
  else if ( !layerProperty.pkCols.isEmpty() )
    tip = tr( "Select columns in the '%1' column that uniquely identify features of this layer" ).arg( tr( "Feature ID" ) );

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
  QStandardItem *sridItem = new QStandardItem( wkbType != QgsWkbTypes::NoGeometry ? QString::number( srid ) : "" );
  sridItem->setEditable( wkbType != QgsWkbTypes::NoGeometry && srid < 0 );
  if ( sridItem->isEditable() )
  {
    sridItem->setText( tr( "Enter…" ) );
    sridItem->setFlags( sridItem->flags() | Qt::ItemIsEditable );
  }

  QStandardItem *pkItem = new QStandardItem( QLatin1String( "" ) );
  if ( !layerProperty.pkCols.isEmpty() )
  {
    pkItem->setText( tr( "Select…" ) );
    pkItem->setFlags( pkItem->flags() | Qt::ItemIsEditable );
  }
  else
    pkItem->setFlags( pkItem->flags() & ~Qt::ItemIsEditable );

  pkItem->setData( layerProperty.pkCols, Qt::UserRole + 1 );
  pkItem->setData( "", Qt::UserRole + 2 );

  QStandardItem *selItem = new QStandardItem( QLatin1String( "" ) );
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

  Q_FOREACH ( QStandardItem *item, childItemList )
  {
    if ( tip.isEmpty() )
    {
      item->setFlags( item->flags() | Qt::ItemIsSelectable );
      item->setToolTip( QLatin1String( "" ) );
    }
    else
    {
      item->setFlags( item->flags() & ~Qt::ItemIsSelectable );

      if ( item == schemaNameItem )
        item->setData( QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) ), Qt::DecorationRole );

      if ( item == schemaNameItem || item == tableItem || item == geomItem )
        item->setToolTip( tip );
    }
  }

  if ( !schemaItem )
  {
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
    QgsWkbTypes::Type wkbType = ( QgsWkbTypes::Type ) idx.sibling( idx.row(), DbtmGeomType ).data( Qt::UserRole + 2 ).toInt();

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
      QSet<QString> s0( idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 2 ).toStringList().toSet() );
      QSet<QString> s1( pkCols.toSet() );
      if ( s0.intersect( s1 ).isEmpty() )
        tip = tr( "Select columns in the '%1' column that uniquely identify features of this layer" ).arg( tr( "Feature ID" ) );
    }

    for ( int i = 0; i < DbtmColumns; i++ )
    {
      QStandardItem *item = itemFromIndex( idx.sibling( idx.row(), i ) );
      if ( tip.isEmpty() )
      {
        if ( i == DbtmSchema )
        {
          item->setData( QVariant(), Qt::DecorationRole );
        }

        item->setFlags( item->flags() | Qt::ItemIsSelectable );
        item->setToolTip( QLatin1String( "" ) );
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

QString QgsHanaTableModel::layerURI( const QModelIndex &index, const QString &connInfo )
{
  if ( !index.isValid() )
    return QString();

  QgsWkbTypes::Type wkbType = ( QgsWkbTypes::Type ) itemFromIndex( index.sibling( index.row(), DbtmGeomType ) )->data( Qt::UserRole + 2 ).toInt();
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
    srid.toInt( &ok );
    if ( !ok )
      return QString();
  }

  bool selectAtId = itemFromIndex( index.sibling( index.row(), DbtmSelectAtId ) )->checkState() == Qt::Checked;
  QString sql = index.sibling( index.row(), DbtmSql ).data( Qt::DisplayRole ).toString();

  QgsDataSourceUri uri( connInfo );
  uri.setDataSource( schemaName, tableName, geomColumnName, sql, pkColumnName );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );

  return uri.uri();
}

QgsWkbTypes::Type QgsHanaTableModel::wkbTypeFromHana( const QString &type )
{
  return QgsWkbTypes::parseType( type.toUpper() );
}
