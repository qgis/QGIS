/***************************************************************************
   qgsredshiftablemodel.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshifttablemodel.h"

#include <climits>

#include "qgsapplication.h"
#include "qgsdataitem.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgssettings.h"

QgsRedshiftTableModel::QgsRedshiftTableModel( QObject *parent )
  : QgsAbstractDbTableModel( parent )
{
  QStringList headerLabels;
  headerLabels << tr( "Database" );
  headerLabels << tr( "Schema" );
  headerLabels << tr( "Table" );
  headerLabels << tr( "Comment" );
  headerLabels << tr( "Column" );
  headerLabels << tr( "Data Type" );
  headerLabels << tr( "Spatial Type" );
  headerLabels << tr( "SRID" );
  headerLabels << tr( "Feature id" );
  headerLabels << tr( "Select at id" );
  headerLabels << tr( "Check PK unicity" );
  headerLabels << tr( "Sql" );
  setHorizontalHeaderLabels( headerLabels );
  setHeaderData( Columns::DbtmSelectAtId, Qt::Orientation::Horizontal,
                 tr( "Disable 'Fast Access to Features at ID' capability to force keeping "
                     "the attribute table in memory (e.g. in case of expensive views)." ),
                 Qt::ToolTipRole );
  setHeaderData( Columns::DbtmCheckPkUnicity, Qt::Orientation::Horizontal,
                 tr( "Enable check for primary key unicity when loading views "
                     "and materialized views. This option can make loading of "
                     "large datasets significantly slower." ),
                 Qt::ToolTipRole );
}

QStringList QgsRedshiftTableModel::columns() const
{
  return mColumns;
}

int QgsRedshiftTableModel::defaultSearchColumn() const
{
  return static_cast<int>( DbtmTable );
}

void QgsRedshiftTableModel::addTableEntry( const QgsRedshiftLayerProperty &layerProperty )
{
  QgsDebugMsgLevel( layerProperty.toString(), 3 );

  // is there already a root item with the given scheme Name?
  QStandardItem *databaseItem = nullptr;

  for ( int i = 0; i < layerProperty.size(); i++ )
  {
    Qgis::WkbType wkbType = layerProperty.types[i];
    int srid = layerProperty.srids[i];

    if ( wkbType == Qgis::WkbType::Unknown && layerProperty.geometryColName.isEmpty() )
    {
      wkbType = Qgis::WkbType::NoGeometry;
    }

    QString tip;
    bool withTipButSelectable = false;

    if ( wkbType == Qgis::WkbType::Unknown )
    {
      tip = tr( "Specify a geometry type in the '%1' column" ).arg( tr( "Data Type" ) );
    }
    else if ( wkbType != Qgis::WkbType::NoGeometry && srid == std::numeric_limits<int>::min() )
    {
      tip = tr( "Enter a SRID into the '%1' column" ).arg( tr( "SRID" ) );
    }
    else if ( !layerProperty.pkCols.isEmpty() )
    {
      tip = tr( "Select columns in the '%1' column that uniquely identify "
                "features of this layer" )
            .arg( tr( "Feature id" ) );
      withTipButSelectable = true;
    }
    QStandardItem *databaseNameItem = new QStandardItem( layerProperty.databaseName );
    QStandardItem *schemaNameItem = new QStandardItem( layerProperty.schemaName );
    QStandardItem *typeItem = nullptr;

    typeItem = new QStandardItem( iconForWkbType( wkbType ), wkbType == Qgis::WkbType::Unknown
                                  ? tr( "Select…" )
                                  : QgsRedshiftConn::displayStringForWkbType( wkbType ) );

    typeItem->setData( wkbType == Qgis::WkbType::Unknown, Qt::UserRole + 1 );
    typeItem->setData( static_cast< quint32>( wkbType ), Qt::UserRole + 2 );
    if ( wkbType == Qgis::WkbType::Unknown )
      typeItem->setFlags( typeItem->flags() | Qt::ItemIsEditable );

    QStandardItem *geomTypeItem =
      new QStandardItem( QgsRedshiftConn::displayStringForGeomType( layerProperty.geometryColType ) );

    QStandardItem *tableItem = new QStandardItem( layerProperty.tableName );
    QStandardItem *commentItem = new QStandardItem( layerProperty.tableComment );
    if ( !layerProperty.tableComment.isEmpty() )
    {
      // word wrap
      QString commentText{layerProperty.tableComment};
      commentText.replace( QRegularExpression( QStringLiteral( "^\n*" ) ), QString() );
      commentItem->setText( commentText );
      commentItem->setToolTip(
        QStringLiteral( "<span>%1</span>" ).arg( commentText.replace( '\n', QStringLiteral( "<br/>" ) ) ) );
      commentItem->setTextAlignment( Qt::AlignTop );
    }
    QStandardItem *geomItem = new QStandardItem( layerProperty.geometryColName );
    QStandardItem *sridItem =
      new QStandardItem( wkbType != Qgis::WkbType::NoGeometry ? QString::number( srid ) : QString() );
    sridItem->setEditable( wkbType != Qgis::WkbType::NoGeometry && srid == std::numeric_limits<int>::min() );
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

    QStringList defPk( QgsSettings()
                       .value( QStringLiteral( "/Redshift/connections/%1/keys/%2/%3" )
                               .arg( mConnName, layerProperty.schemaName, layerProperty.tableName ),
                               QStringList() )
                       .toStringList() );

    if ( !layerProperty.pkCols.isEmpty() && defPk.isEmpty() )
    {
      // If we have a view with multiple possible columns to be used as the
      // primary key, for convenience let's select the first one - this is what
      // the browser dock already does. We risk that a wrong column will be
      // used, but most of the time we should be fine.
      defPk = QStringList( layerProperty.pkCols[0] );
    }

    pkItem->setData( defPk, Qt::UserRole + 2 );
    if ( !defPk.isEmpty() )
      pkItem->setText( defPk.join( ',' ) );

    QStandardItem *selItem = new QStandardItem( QString() );
    selItem->setFlags( selItem->flags() | Qt::ItemIsUserCheckable );
    selItem->setCheckState( Qt::Checked );
    selItem->setToolTip(
      headerData( Columns::DbtmSelectAtId, Qt::Orientation::Horizontal, Qt::ToolTipRole ).toString() );

    QStandardItem *checkPkUnicityItem = new QStandardItem( QString() );
    checkPkUnicityItem->setFlags( checkPkUnicityItem->flags() | Qt::ItemIsUserCheckable );

    // Legacy: default value is determined by project option to trust layer's
    // metadata
    // TODO: remove this default from QGIS 4 and leave default value to false?
    // checkPkUnicity has only effect on views and materialized views, so we can
    // safely disable it
    if ( layerProperty.isView || layerProperty.isMaterializedView )
    {
      checkPkUnicityItem->setCheckState( ( QgsProject::instance( )->flags() & Qgis::ProjectFlag::TrustStoredLayerStatistics ) ? Qt::CheckState::Unchecked
                                         : Qt::CheckState::Checked );
      checkPkUnicityItem->setToolTip(
        headerData( Columns::DbtmCheckPkUnicity, Qt::Orientation::Horizontal, Qt::ToolTipRole ).toString() );
    }
    else
    {
      checkPkUnicityItem->setCheckState( Qt::CheckState::Unchecked );
      checkPkUnicityItem->setFlags( checkPkUnicityItem->flags() & ~Qt::ItemIsEnabled );
      checkPkUnicityItem->setToolTip( tr( "This option is only available for views and materialized views." ) );
    }

    QStandardItem *sqlItem = new QStandardItem( layerProperty.sql );

    QList<QStandardItem *> childItemList;

    childItemList << databaseNameItem;
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
          item->setData( QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) ),
                         Qt::DecorationRole );

        if ( item == schemaNameItem || item == tableItem || item == geomItem )
        {
          item->setToolTip( tip );
        }
      }
    }

    if ( !databaseItem )
    {
      QList<QStandardItem *> databaseItems =
        findItems( layerProperty.databaseName, Qt::MatchExactly,
                   DbtmDatabase );

      // there is already an item for this schema
      if ( !databaseItems.isEmpty() )
      {
        databaseItem = databaseItems.at( DbtmDatabase );
      }
      else
      {
        // create a new toplevel item for this database
        QString currentDatabase( "local" );
        if ( !layerProperty.databaseName.isEmpty() )
        {
          currentDatabase = layerProperty.databaseName;
        }
        databaseItem = new QStandardItem( currentDatabase );
        databaseItem->setFlags( Qt::ItemIsEnabled );
        invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), databaseItem );
        if ( layerProperty.tableComment == "external table" )
        {
          databaseItem->setData( QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) ),
                                 Qt::DecorationRole );
          tip = tr( "This is an external database - it will be open read-only" );
          databaseItem->setToolTip( tip );
        }
      }
    }

    databaseItem->appendRow( childItemList );

    ++mTableCount;
  }
}

void QgsRedshiftTableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }

  // find out schema name and table name
  QModelIndex databaseSibling = index.sibling( index.row(), DbtmDatabase );
  QModelIndex schemaSibling = index.sibling( index.row(), DbtmSchema );
  QModelIndex tableSibling = index.sibling( index.row(), DbtmTable );
  QModelIndex geomSibling = index.sibling( index.row(), DbtmGeomCol );
  QModelIndex geomTypeSibling = index.sibling( index.row(), DbtmType );

  if ( !databaseSibling.isValid() || !schemaSibling.isValid() ||
       !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  QString databaseName = itemFromIndex( databaseSibling )->text();
  QString schemaName = itemFromIndex( schemaSibling )->text();
  QString tableName = itemFromIndex( tableSibling )->text();
  QString geomName = itemFromIndex( geomSibling )->text();
  QString geomType = itemFromIndex( geomTypeSibling )->text();

  QList<QStandardItem *> databaseItems =
    findItems( databaseName, Qt::MatchExactly, DbtmDatabase );
  if ( databaseItems.empty() )
  {
    return;
  }

  QStandardItem *databaseItem = databaseItems.at( DbtmDatabase );

  int n = databaseItem->rowCount();
  for ( int i = 0; i < n; i++ )
  {
    QModelIndex currentChildIndex =
      indexFromItem( databaseItem->child( i, DbtmDatabase ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }

    QModelIndex currentSchemaIndex = currentChildIndex.sibling( i, DbtmSchema );
    if ( !currentSchemaIndex.isValid() )
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

    QModelIndex currentGeomType = currentChildIndex.sibling( i, DbtmType );
    if ( !currentGeomType.isValid() )
    {
      continue;
    }

    if ( itemFromIndex( currentTableIndex )->text() == tableName &&
         itemFromIndex( currentGeomIndex )->text() == geomName && itemFromIndex( currentGeomType )->text() == geomType )
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

QIcon QgsRedshiftTableModel::iconForWkbType( Qgis::WkbType type )
{
  Qgis::GeometryType geomType = QgsWkbTypes::geometryType( Qgis::WkbType( type ) );
  switch ( geomType )
  {
    case Qgis::GeometryType::Point:
      return QgsApplication::getThemeIcon( "/mIconPointLayer.svg" );
    case Qgis::GeometryType::Line:
      return QgsApplication::getThemeIcon( "/mIconLineLayer.svg" );
    case Qgis::GeometryType::Polygon:
      return QgsApplication::getThemeIcon( "/mIconPolygonLayer.svg" );
    default:
      break;
  }
  return QgsApplication::getThemeIcon( "/mIconLayer.png" );
}

bool QgsRedshiftTableModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
  if ( !QStandardItemModel::setData( idx, value, role ) )
    return false;

  if ( idx.column() == DbtmType || idx.column() == DbtmSrid || idx.column() == DbtmPkCol )
  {
    Qgis::WkbType wkbType = ( Qgis::WkbType )idx.sibling( idx.row(), DbtmType ).data( Qt::UserRole + 2 ).toInt();

    QString tip;
    if ( wkbType == Qgis::WkbType::Unknown )
    {
      tip = tr( "Specify a geometry type in the '%1' column" ).arg( tr( "Data Type" ) );
    }
    else if ( wkbType != Qgis::WkbType::NoGeometry )
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
        tip = tr( "Select columns in the '%1' column that uniquely identify "
                  "features of this layer" )
              .arg( tr( "Feature id" ) );
    }

    for ( int i = 0; i < columnCount(); i++ )
    {
      QStandardItem *item = itemFromIndex( idx.sibling( idx.row(), i ) );
      if ( tip.isEmpty() )
      {
        if ( i == DbtmDatabase )
        {
          item->setData( QVariant(), Qt::DecorationRole );
        }

        item->setFlags( item->flags() | Qt::ItemIsSelectable );
        item->setToolTip( QString() );
      }
      else
      {
        item->setFlags( item->flags() & ~Qt::ItemIsSelectable );

        if ( i == DbtmDatabase )
          item->setData( QgsApplication::getThemeIcon(
                           QStringLiteral( "/mIconWarning.svg" ) ),
                         Qt::DecorationRole );

        if ( i == DbtmDatabase || i == DbtmSchema || i == DbtmTable ||
             i == DbtmGeomCol )
        {
          item->setFlags( item->flags() );
          item->setToolTip( tip );
        }
      }
    }
  }

  return true;
}

QString QgsRedshiftTableModel::layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata )
{
  if ( !index.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "invalid index" ) );
    return QString();
  }

  Qgis::WkbType wkbType = static_cast<Qgis::WkbType>(
                            itemFromIndex( index.sibling( index.row(), DbtmType ) )->data( Qt::UserRole + 2 ).toInt() );
  if ( wkbType == Qgis::WkbType::Unknown )
  {
    QgsDebugMsg( QStringLiteral( "unknown geometry type" ) );
    // no geometry type selected
    return QString();
  }

  QStandardItem *pkItem = itemFromIndex( index.sibling( index.row(), DbtmPkCol ) );
  QSet<QString> s0( qgis::listToSet( pkItem->data( Qt::UserRole + 1 ).toStringList() ) );
  QSet<QString> s1( qgis::listToSet( pkItem->data( Qt::UserRole + 2 ).toStringList() ) );
  if ( !s0.isEmpty() && !s0.intersects( s1 ) )
  {
    // no valid primary candidate selected
    QgsDebugMsg( QStringLiteral( "no pk candidate selected" ) );
    return QString();
  }
  QString databaseName = index.sibling( index.row(), DbtmDatabase ).data( Qt::DisplayRole ).toString();
  QString schemaName = index.sibling( index.row(), DbtmSchema ).data( Qt::DisplayRole ).toString();
  QString tableName = index.sibling( index.row(), DbtmTable ).data( Qt::DisplayRole ).toString();

  QString geomColumnName;
  QString srid;
  if ( wkbType != Qgis::WkbType::NoGeometry )
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
  bool checkPrimaryKeyUnicity =
    itemFromIndex( index.sibling( index.row(), DbtmCheckPkUnicity ) )->checkState() == Qt::Checked;

  QgsDataSourceUri uri( connInfo );

  QStringList cols;
  const auto constS1 = s1;
  for ( const QString &col : constS1 )
  {
    cols << QgsRedshiftConn::quotedIdentifier( col );
  }

  QgsSettings().setValue( QStringLiteral( "/Redshift/connections/%1/keys/%2/%3" ).arg( mConnName, schemaName, tableName ),
                          QVariant( qgis::setToList( s1 ) ) );

  uri.setDataSource( schemaName, tableName, geomColumnName, sql,
                     cols.join( ',' ), databaseName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  uri.setWkbType( wkbType );
  uri.setSrid( srid );
  uri.disableSelectAtId( !selectAtId );
  uri.setParam( QStringLiteral( "checkPrimaryKeyUnicity" ),
                checkPrimaryKeyUnicity ? QLatin1String( "1" ) : QLatin1String( "0" ) );

  QgsDebugMsg( QStringLiteral( "returning uri %1" ).arg( uri.uri( false ) ) );
  return uri.uri( false );
}
