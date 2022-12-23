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
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsiconutils.h"
#include <QRegularExpression>
#include <climits>

QgsPgTableModel::QgsPgTableModel( QObject *parent )
  : QgsAbstractDbTableModel( parent )
{
  mColumns << tr( "Schema" )
           << tr( "Table" )
           << tr( "Comment" )
           << tr( "Column" )
           << tr( "Data Type" )
           << tr( "Spatial Type" )
           << tr( "SRID" )
           << tr( "Feature id" )
           << tr( "Select at id" )
           << tr( "Check PK unicity" )
           << tr( "SQL" );
  setHorizontalHeaderLabels( mColumns );
  setHeaderData( Columns::DbtmSelectAtId, Qt::Orientation::Horizontal, tr( "Disable 'Fast Access to Features at ID' capability to force keeping the attribute table in memory (e.g. in case of expensive views)." ), Qt::ToolTipRole );
  setHeaderData( Columns::DbtmCheckPkUnicity, Qt::Orientation::Horizontal, tr( "Enable check for primary key unicity when loading views and materialized views. This option can make loading of large datasets significantly slower." ), Qt::ToolTipRole );
}

QStringList QgsPgTableModel::columns() const
{
  return mColumns;
}

int QgsPgTableModel::defaultSearchColumn() const
{
  return static_cast<int>( DbtmTable );
}

bool QgsPgTableModel::searchableColumn( int column ) const
{
  Columns col = static_cast<Columns>( column );
  switch ( col )
  {
    case DbtmSchema:
    case DbtmTable:
    case DbtmComment:
    case DbtmGeomCol:
    case DbtmType:
    case DbtmSrid:
    case DbtmSql:
      return true;

    case DbtmGeomType:
    case DbtmPkCol:
    case DbtmSelectAtId:
    case DbtmCheckPkUnicity:
      return false;
  }

  BUILTIN_UNREACHABLE
}

void QgsPgTableModel::addTableEntry( const QgsPostgresLayerProperty &layerProperty )
{
  QgsDebugMsgLevel( layerProperty.toString(), 3 );

  // is there already a root item with the given scheme Name?
  QStandardItem *schemaItem = nullptr;

  for ( int i = 0; i < layerProperty.size(); i++ )
  {
    QgsWkbTypes::Type wkbType = layerProperty.types[ i ];
    const int srid = layerProperty.srids[ i ];

    if ( wkbType == QgsWkbTypes::Unknown && layerProperty.geometryColName.isEmpty() )
    {
      wkbType = QgsWkbTypes::NoGeometry;
    }

    QString tip;
    bool withTipButSelectable = false;
    if ( ! layerProperty.isRaster )
    {
      if ( wkbType == QgsWkbTypes::Unknown )
      {
        tip = tr( "Specify a geometry type in the '%1' column" ).arg( tr( "Data Type" ) );
      }
      else if ( wkbType != QgsWkbTypes::NoGeometry && srid == std::numeric_limits<int>::min() )
      {
        tip = tr( "Enter a SRID into the '%1' column" ).arg( tr( "SRID" ) );
      }
      else if ( !layerProperty.pkCols.isEmpty() )
      {
        tip = tr( "Select columns in the '%1' column that uniquely identify features of this layer" ).arg( tr( "Feature id" ) );
        withTipButSelectable = true;
      }
    }

    QStandardItem *schemaNameItem = new QStandardItem( layerProperty.schemaName );
    QStandardItem *typeItem = nullptr;
    if ( layerProperty.isRaster )
    {
      typeItem = new QStandardItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterLayer.svg" ) ), tr( "Raster" ) );
    }
    else
    {
      typeItem = new QStandardItem( QgsIconUtils::iconForWkbType( wkbType ), wkbType == QgsWkbTypes::Unknown ? tr( "Select…" ) : QgsPostgresConn::displayStringForWkbType( wkbType ) );
    }
    typeItem->setData( wkbType == QgsWkbTypes::Unknown, Qt::UserRole + 1 );
    typeItem->setData( wkbType, Qt::UserRole + 2 );
    typeItem->setData( layerProperty.isRaster, Qt::UserRole + 3 );
    if ( wkbType == QgsWkbTypes::Unknown )
      typeItem->setFlags( typeItem->flags() | Qt::ItemIsEditable );

    QStandardItem *geomTypeItem = new QStandardItem( QgsPostgresConn::displayStringForGeomType( layerProperty.geometryColType ) );

    QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
    QStandardItem *commentItem = new QStandardItem( layerProperty.tableComment );
    if ( ! layerProperty.tableComment.isEmpty() )
    {
      // word wrap
      QString commentText { layerProperty.tableComment };
      commentText.replace( QRegularExpression( QStringLiteral( "^\n*" ) ), QString() );
      commentItem->setText( commentText );
      commentItem->setToolTip( QStringLiteral( "<span>%1</span>" ).arg( commentText.replace( '\n', QLatin1String( "<br/>" ) ) ) );
      commentItem->setTextAlignment( Qt::AlignTop );
    }
    QStandardItem *geomItem  = new QStandardItem( layerProperty.geometryColName );
    QStandardItem *sridItem  = new QStandardItem( wkbType != QgsWkbTypes::NoGeometry ? QString::number( srid ) : QString() );
    sridItem->setEditable( wkbType != QgsWkbTypes::NoGeometry && srid == std::numeric_limits<int>::min() );
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

    QStringList defPk( QgsSettings().value( QStringLiteral( "/PostgreSQL/connections/%1/keys/%2/%3" ).arg( mConnName, layerProperty.schemaName, layerProperty.tableName ), QStringList() ).toStringList() );

    if ( !layerProperty.pkCols.isEmpty() && defPk.isEmpty() )
    {
      // If we have a view with multiple possible columns to be used as the primary key, for convenience
      // let's select the first one - this is what the browser dock already does. We risk that a wrong column
      // will be used, but most of the time we should be fine.
      defPk = QStringList( layerProperty.pkCols[0] );
    }

    pkItem->setData( defPk, Qt::UserRole + 2 );
    if ( !defPk.isEmpty() )
      pkItem->setText( defPk.join( ',' ) );

    QStandardItem *selItem = new QStandardItem( QString() );
    selItem->setFlags( selItem->flags() | Qt::ItemIsUserCheckable );
    selItem->setCheckState( Qt::Checked );
    selItem->setToolTip( headerData( Columns::DbtmSelectAtId, Qt::Orientation::Horizontal, Qt::ToolTipRole ).toString() );

    QStandardItem *checkPkUnicityItem  = new QStandardItem( QString() );
    checkPkUnicityItem->setFlags( checkPkUnicityItem->flags() | Qt::ItemIsUserCheckable );

    // Legacy: default value is determined by project option to trust layer's metadata
    // TODO: remove this default from QGIS 4 and leave default value to false?
    // checkPkUnicity has only effect on views and materialized views, so we can safely disable it
    if ( layerProperty.isView || layerProperty.isMaterializedView )
    {
      checkPkUnicityItem->setCheckState( ( QgsProject::instance( )->flags() & Qgis::ProjectFlag::TrustStoredLayerStatistics ) ? Qt::CheckState::Unchecked : Qt::CheckState::Checked );
      checkPkUnicityItem->setToolTip( headerData( Columns::DbtmCheckPkUnicity, Qt::Orientation::Horizontal, Qt::ToolTipRole ).toString() );
    }
    else
    {
      checkPkUnicityItem->setCheckState( Qt::CheckState::Unchecked );
      checkPkUnicityItem->setFlags( checkPkUnicityItem->flags() & ~ Qt::ItemIsEnabled );
      checkPkUnicityItem->setToolTip( tr( "This option is only available for views and materialized views." ) );
    }

    QStandardItem *sqlItem = new QStandardItem( layerProperty.sql );

    // For rasters, disable
    if ( layerProperty.isRaster )
    {
      selItem->setFlags( selItem->flags() & ~ Qt::ItemIsUserCheckable );
      selItem->setCheckState( Qt::Unchecked );
      checkPkUnicityItem->setFlags( checkPkUnicityItem->flags() & ~ Qt::ItemIsUserCheckable );
      checkPkUnicityItem->setCheckState( Qt::Unchecked );
    }

    QList<QStandardItem *> childItemList;

    childItemList << schemaNameItem;
    childItemList << tableItem;
    childItemList << commentItem;
    childItemList << geomItem;
    childItemList << geomTypeItem;
    childItemList << typeItem;
    childItemList << sridItem;
    childItemList << pkItem;
    childItemList << selItem;
    childItemList << checkPkUnicityItem;
    childItemList << sqlItem;

    const auto constChildItemList = childItemList;
    for ( QStandardItem *item : constChildItemList )
    {
      if ( tip.isEmpty() || withTipButSelectable )
        item->setFlags( item->flags() | Qt::ItemIsSelectable );
      else
        item->setFlags( item->flags() & ~Qt::ItemIsSelectable );

      if ( item->toolTip().isEmpty() && tip.isEmpty() && item != checkPkUnicityItem && item != selItem )
      {
        item->setToolTip( QString() );
      }
      else
      {
        if ( item == schemaNameItem )
          item->setData( QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) ), Qt::DecorationRole );

        if ( item == schemaNameItem || item == tableItem || item == geomItem )
        {
          item->setToolTip( tip );
        }
      }
    }

    if ( !schemaItem )
    {
      const QList<QStandardItem *> schemaItems = findItems( layerProperty.schemaName, Qt::MatchExactly, DbtmSchema );

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
}

void QgsPgTableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }

  //find out schema name and table name
  const QModelIndex schemaSibling = index.sibling( index.row(), DbtmSchema );
  const QModelIndex tableSibling = index.sibling( index.row(), DbtmTable );
  const QModelIndex geomSibling = index.sibling( index.row(), DbtmGeomCol );
  const QModelIndex geomTypeSibling = index.sibling( index.row(), DbtmType );

  if ( !schemaSibling.isValid() || !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  const QString schemaName = itemFromIndex( schemaSibling )->text();
  const QString tableName = itemFromIndex( tableSibling )->text();
  const QString geomName = itemFromIndex( geomSibling )->text();
  const QString geomType = itemFromIndex( geomTypeSibling )->text();

  const QList<QStandardItem *> schemaItems = findItems( schemaName, Qt::MatchExactly, DbtmSchema );
  if ( schemaItems.empty() )
  {
    return;
  }

  QStandardItem *schemaItem = schemaItems.at( DbtmSchema );

  const int n = schemaItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    const QModelIndex currentChildIndex = indexFromItem( schemaItem->child( i, DbtmSchema ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }

    const QModelIndex currentTableIndex = currentChildIndex.sibling( i, DbtmTable );
    if ( !currentTableIndex.isValid() )
    {
      continue;
    }

    const QModelIndex currentGeomIndex = currentChildIndex.sibling( i, DbtmGeomCol );
    if ( !currentGeomIndex.isValid() )
    {
      continue;
    }

    const QModelIndex currentGeomType = currentChildIndex.sibling( i, DbtmType );
    if ( !currentGeomType.isValid() )
    {
      continue;
    }

    if ( itemFromIndex( currentTableIndex )->text() == tableName
         && itemFromIndex( currentGeomIndex )->text() == geomName
         && itemFromIndex( currentGeomType )->text() == geomType )
    {
      const QModelIndex sqlIndex = currentChildIndex.sibling( i, DbtmSql );
      if ( sqlIndex.isValid() )
      {
        itemFromIndex( sqlIndex )->setText( sql );
        break;
      }
    }
  }
}

bool QgsPgTableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
  if ( !QStandardItemModel::setData( idx, value, role ) )
    return false;

  if ( idx.column() == DbtmType || idx.column() == DbtmSrid || idx.column() == DbtmPkCol )
  {
    const QgsWkbTypes::Type wkbType = static_cast< QgsWkbTypes::Type >( idx.sibling( idx.row(), DbtmType ).data( Qt::UserRole + 2 ).toInt() );

    QString tip;
    if ( wkbType == QgsWkbTypes::Unknown )
    {
      tip = tr( "Specify a geometry type in the '%1' column" ).arg( tr( "Data Type" ) );
    }
    else if ( wkbType != QgsWkbTypes::NoGeometry )
    {
      bool ok;
      const int srid = idx.sibling( idx.row(), DbtmSrid ).data().toInt( &ok );

      if ( !ok || srid == std::numeric_limits<int>::min() )
        tip = tr( "Enter a SRID into the '%1' column" ).arg( tr( "SRID" ) );
    }

    const QStringList pkCols = idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 1 ).toStringList();
    if ( tip.isEmpty() && !pkCols.isEmpty() )
    {
      const QSet<QString> s0( qgis::listToSet( idx.sibling( idx.row(), DbtmPkCol ).data( Qt::UserRole + 2 ).toStringList() ) );
      const QSet<QString> s1( qgis::listToSet( pkCols ) );
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
        item->setToolTip( QString() );
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

QString QgsPgTableModel::layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata )
{
  if ( !index.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "invalid index" ) );
    return QString();
  }

  const bool isRaster = itemFromIndex( index.sibling( index.row(), DbtmType ) )->data( Qt::UserRole + 3 ).toBool();
  const QgsWkbTypes::Type wkbType = static_cast<QgsWkbTypes::Type>( itemFromIndex( index.sibling( index.row(), DbtmType ) )->data( Qt::UserRole + 2 ).toInt() );
  if ( wkbType == QgsWkbTypes::Unknown )
  {
    if ( isRaster )
    {
      // GDAL/PG connection string
      QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), DbtmPkCol ) );
      const QSet<QString> s1( qgis::listToSet( pkItem->data( Qt::UserRole + 2 ).toStringList() ) );
      QStringList cols;
      cols.reserve( s1.size() );
      for ( const QString &col : std::as_const( s1 ) )
      {
        cols << QgsPostgresConn::quotedIdentifier( col );
      }
      const QString schemaName = index.sibling( index.row(), DbtmSchema ).data( Qt::DisplayRole ).toString();
      const QString tableName = index.sibling( index.row(), DbtmTable ).data( Qt::DisplayRole ).toString();
      const QString geomColumnName = index.sibling( index.row(), DbtmGeomCol ).data( Qt::DisplayRole ).toString();
      QString connString { QStringLiteral( "PG: %1 mode=2 %2schema='%3' column='%4' table='%5'" )
                           .arg( connInfo,
                                 cols.isEmpty() ? QString() : QStringLiteral( "key='%1' " ).arg( cols.join( ',' ) ),
                                 schemaName,
                                 geomColumnName,
                                 tableName ) };
      const QString sql { index.sibling( index.row(), DbtmSql ).data( Qt::DisplayRole ).toString() };
      if ( ! sql.isEmpty() )
      {
        connString.append( QStringLiteral( " sql=%1" ).arg( sql ) );
      }
      return connString;
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "unknown geometry type" ) );
      // no geometry type selected
      return QString();
    }
  }

  QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), DbtmPkCol ) );
  const QSet<QString> s0( qgis::listToSet( pkItem->data( Qt::UserRole + 1 ).toStringList() ) );
  const QSet<QString> s1( qgis::listToSet( pkItem->data( Qt::UserRole + 2 ).toStringList() ) );
  if ( !s0.isEmpty() && !s0.intersects( s1 ) )
  {
    // no valid primary candidate selected
    QgsDebugMsg( QStringLiteral( "no pk candidate selected" ) );
    return QString();
  }

  const QString schemaName = index.sibling( index.row(), DbtmSchema ).data( Qt::DisplayRole ).toString();
  const QString tableName = index.sibling( index.row(), DbtmTable ).data( Qt::DisplayRole ).toString();

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

  const bool selectAtId = itemFromIndex( index.sibling( index.row(), DbtmSelectAtId ) )->checkState() == Qt::Checked;
  const QString sql = index.sibling( index.row(), DbtmSql ).data( Qt::DisplayRole ).toString();
  const bool checkPrimaryKeyUnicity = itemFromIndex( index.sibling( index.row(), DbtmCheckPkUnicity ) )->checkState() == Qt::Checked;

  QgsDataSourceUri uri( connInfo );

  QStringList cols;
  cols.reserve( s1.size() );
  for ( const QString &col : s1 )
  {
    cols << QgsPostgresConn::quotedIdentifier( col );
  }

  QgsSettings().setValue( QStringLiteral( "/PostgreSQL/connections/%1/keys/%2/%3" ).arg( mConnName, schemaName, tableName ), QVariant( qgis::setToList( s1 ) ) );

  uri.setDataSource( schemaName, tableName, geomColumnName, sql, cols.join( ',' ) );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );
  uri.setParam( QStringLiteral( "checkPrimaryKeyUnicity" ), checkPrimaryKeyUnicity ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  QgsDebugMsg( QStringLiteral( "returning uri %1" ).arg( uri.uri( false ) ) );
  return uri.uri( false );
}

