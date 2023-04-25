/***************************************************************************
   qgsredshiftdataitemguiprovider.cpp
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
#include "qgsredshiftdataitemguiprovider.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "qgsmanageconnectionsdialog.h"
#include "qgsnewnamedialog.h"
#include "qgsredshiftdataitems.h"
#include "qgsredshiftnewconnection.h"
#include "qgsredshiftprovider.h"
#include "qgsredshiftsourceselect.h"

void QgsRedshiftDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &,
    QgsDataItemGuiContext )
{
  if ( QgsRSRootItem *rootItem = qobject_cast<QgsRSRootItem *>( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionSaveServers = new QAction( tr( "Save Connections…" ), this );
    connect( actionSaveServers, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSaveServers );

    QAction *actionLoadServers = new QAction( tr( "Load Connections…" ), this );
    connect( actionLoadServers, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoadServers );
  }

  if ( QgsRSConnectionItem *connItem = qobject_cast<QgsRSConnectionItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), this );
    connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), this );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete Connection" ), this );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); } );
    menu->addAction( actionDelete );

    menu->addSeparator();

    QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), this );
    connect( actionCreateSchema, &QAction::triggered, this, [connItem] { createSchema( connItem ); } );
    menu->addAction( actionCreateSchema );
  }

  if ( QgsRSSchemaItem *schemaItem = qobject_cast<QgsRSSchemaItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), this );
    connect( actionRefresh, &QAction::triggered, this, [schemaItem] { schemaItem->refresh(); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionRename = new QAction( tr( "Rename Schema…" ), this );
    connect( actionRename, &QAction::triggered, this, [schemaItem] { renameSchema( schemaItem ); } );
    menu->addAction( actionRename );

    QAction *actionDelete = new QAction( tr( "Delete Schema" ), this );
    connect( actionDelete, &QAction::triggered, this, [schemaItem] { deleteSchema( schemaItem ); } );
    menu->addAction( actionDelete );
  }

  if ( QgsRSLayerItem *layerItem = qobject_cast<QgsRSLayerItem *>( item ) )
  {
    const QgsRedshiftLayerProperty &layerInfo = layerItem->layerInfo();
    QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );

    QAction *actionRenameLayer = new QAction( tr( "Rename %1…" ).arg( typeName ), this );
    connect( actionRenameLayer, &QAction::triggered, this, [layerItem] { renameLayer( layerItem ); } );
    menu->addAction( actionRenameLayer );

    if ( !layerInfo.isView )
    {
      QAction *actionTruncateLayer = new QAction( tr( "Truncate %1" ).arg( typeName ), this );
      connect( actionTruncateLayer, &QAction::triggered, this, [layerItem] { truncateTable( layerItem ); } );
      menu->addAction( actionTruncateLayer );
    }

    if ( layerInfo.isMaterializedView )
    {
      QAction *actionRefreshMaterializedView = new QAction( tr( "Refresh Materialized View" ), this );
      connect( actionRefreshMaterializedView, &QAction::triggered, this,
               [layerItem] { refreshMaterializedView( layerItem ); } );
      menu->addAction( actionRefreshMaterializedView );
    }
  }
}

bool QgsRedshiftDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsRSLayerItem *layerItem = qobject_cast<QgsRSLayerItem *>( item ) )
  {
    const QgsRedshiftLayerProperty &layerInfo = layerItem->layerInfo();
    QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );

    if ( QMessageBox::question(
           nullptr, tr( "Delete %1" ).arg( typeName ),
           QObject::tr( "Are you sure you want to delete %1.%2?" ).arg( layerInfo.schemaName, layerInfo.tableName ),
           QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    QString errCause;
    bool res = QgsRedshiftUtils::deleteLayer( layerItem->uri(), errCause );
    if ( !res )
    {
      notify( tr( "Delete %1" ).arg( typeName ), errCause, context, Qgis::MessageLevel::Warning );
      return false;
    }
    else
    {
      notify( tr( "Delete %1" ).arg( typeName ), tr( "%1 deleted successfully." ).arg( typeName ), context,
              Qgis::MessageLevel::Success );
      if ( layerItem->parent() )
        layerItem->parent()->refresh();
      return true;
    }
  }
  return false;
}

bool QgsRedshiftDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast<QgsRSConnectionItem *>( item ) )
    return true;
  if ( qobject_cast<QgsRSSchemaItem *>( item ) )
    return true;

  return false;
}

bool QgsRedshiftDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data,
    Qt::DropAction )
{
  if ( QgsRSConnectionItem *connItem = qobject_cast<QgsRSConnectionItem *>( item ) )
  {
    return connItem->handleDrop( data, QString() );
  }
  else if ( QgsRSSchemaItem *schemaItem = qobject_cast<QgsRSSchemaItem *>( item ) )
  {
    QgsRSConnectionItem *connItem = qobject_cast<QgsRSConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return connItem->handleDrop( data, schemaItem->name() );
  }
  return false;
}

QWidget *QgsRedshiftDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  QgsRSRootItem *rsRootItem = qobject_cast<QgsRSRootItem *>( root );
  if ( rsRootItem != nullptr )
  {
    QgsRedshiftSourceSelect *select = new QgsRedshiftSourceSelect( nullptr, Qt::WindowFlags(), QgsProviderRegistry::WidgetMode::Manager );
    connect( select, &QgsRedshiftSourceSelect::connectionsChanged, rsRootItem, &QgsRSRootItem::onConnectionsChanged );
    return select;
  }
  else
  {
    return nullptr;
  }
}

void QgsRedshiftDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsRedshiftNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsRedshiftDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsRedshiftNewConnection nc( nullptr, item->name() );
  nc.setWindowTitle( tr( "Edit Redshift Connection" ) );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsRedshiftDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsRedshiftConn::deleteConnection( item->name() );
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsRedshiftDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsRedshiftDataItemGuiProvider::createSchema( QgsDataItem *item )
{
  QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  QgsDataSourceUri uri = QgsRedshiftConn::connUri( item->name() );
  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "New Schema" ), tr( "Unable to create schema." ) );
    return;
  }

  // create the schema
  QString sql = QStringLiteral( "CREATE SCHEMA %1" ).arg( QgsRedshiftConn::quotedIdentifier( schemaName ) );

  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "New Schema" ),
                          tr( "Unable to create schema %1\n%2" ).arg( schemaName, result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();

  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsRedshiftDataItemGuiProvider::deleteSchema( QgsRSSchemaItem *schemaItem )
{
  // check if schema contains tables/views
  QgsDataSourceUri uri = QgsRedshiftConn::connUri( schemaItem->connectionName() );
  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Delete Schema" ), tr( "Unable to delete schema." ) );
    return;
  }

  QString sql = QStringLiteral( "SELECT table_name FROM information_schema.tables WHERE "
                                "table_schema='%1'" )
                .arg( schemaItem->name() );
  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QMessageBox::warning( nullptr, tr( "Delete Schema" ), tr( "Unable to delete schema." ) );
    conn->unref();
    return;
  }

  QStringList childObjects;
  int maxListed = 10;
  for ( int idx = 0; idx < result.PQntuples(); idx++ )
  {
    childObjects << result.PQgetvalue( idx, 0 );
    QgsRedshiftSchemaProperty schema;
    if ( idx == maxListed - 1 )
      break;
  }

  int count = result.PQntuples();
  if ( count > 0 )
  {
    QString objects = childObjects.join( QStringLiteral( "\n" ) );
    if ( count > maxListed )
    {
      objects += QStringLiteral( "\n[%1 additional objects not listed]" ).arg( count - maxListed );
    }
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ),
                                QObject::tr( "Schema '%1' contains objects:\n\n%2\n\nAre you sure "
                                    "you want to delete the schema and all these objects?" )
                                .arg( schemaItem->name(), objects ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      conn->unref();
      return;
    }
  }
  else
  {
    if ( QMessageBox::question(
           nullptr, QObject::tr( "Delete Schema" ),
           QObject::tr( "Are you sure you want to delete the schema '%1'?" ).arg( schemaItem->name() ),
           QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  QString errCause;
  bool res = QgsRedshiftUtils::deleteSchema( schemaItem->name(), uri, errCause, count > 0 );
  if ( !res )
  {
    QMessageBox::warning( nullptr, tr( "Delete Schema" ), errCause );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Delete Schema" ), tr( "Schema deleted successfully." ) );
    if ( schemaItem->parent() )
      schemaItem->parent()->refreshConnections();
  }
}

void QgsRedshiftDataItemGuiProvider::renameSchema( QgsRSSchemaItem *schemaItem )
{
  QgsNewNameDialog dlg( tr( "schema '%1'" ).arg( schemaItem->name() ), schemaItem->name() );
  dlg.setWindowTitle( tr( "Rename Schema" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == schemaItem->name() )
    return;

  QString schemaName = QgsRedshiftConn::quotedIdentifier( schemaItem->name() );
  QgsDataSourceUri uri = QgsRedshiftConn::connUri( schemaItem->connectionName() );
  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Rename Schema" ), tr( "Unable to rename schema." ) );
    return;
  }

  // rename the schema
  QString sql =
    QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" ).arg( schemaName, QgsRedshiftConn::quotedIdentifier( dlg.name() ) );

  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Rename Schema" ),
                          tr( "Unable to rename schema %1\n%2" ).arg( schemaName, result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Rename Schema" ), tr( "Schema renamed successfully." ) );
  if ( schemaItem->parent() )
    schemaItem->parent()->refreshConnections();
}

void QgsRedshiftDataItemGuiProvider::renameLayer( QgsRSLayerItem *layerItem )
{
  const QgsRedshiftLayerProperty &layerInfo = layerItem->layerInfo();
  QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );
  QString lowerTypeName = layerInfo.isView ? tr( "view" ) : tr( "table" );

  QgsNewNameDialog dlg( tr( "%1 %2.%3" ).arg( lowerTypeName, layerInfo.schemaName, layerInfo.tableName ),
                        layerInfo.tableName );
  dlg.setWindowTitle( tr( "Rename %1" ).arg( typeName ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == layerInfo.tableName )
    return;

  QString schemaName = layerInfo.schemaName;
  QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsRedshiftConn::quotedIdentifier( schemaName ) + '.';
  }
  QString oldName = schemaTableName + QgsRedshiftConn::quotedIdentifier( tableName );
  QString newName = QgsRedshiftConn::quotedIdentifier( dlg.name() );

  QgsDataSourceUri dsUri( layerItem->uri() );
  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename %1." ).arg( lowerTypeName ) );
    return;
  }

  // rename the layer
  QString sql;
  if ( layerInfo.isView )
  {
    sql = QStringLiteral( "ALTER %1 VIEW %2 RENAME TO %3" ).arg( layerInfo.isMaterializedView ? QStringLiteral( "MATERIALIZED" ) : QString(),
          oldName, newName );
  }
  else
  {
    sql = QStringLiteral( "ALTER TABLE %1 RENAME TO %2" ).arg( oldName, newName );
  }

  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning(
      nullptr, tr( "Rename %1" ).arg( typeName ),
      tr( "Unable to rename %1 %2\n%3" ).arg( lowerTypeName, layerItem->name(), result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  if ( layerItem->parent() )
    layerItem->parent()->refreshConnections();
}

void QgsRedshiftDataItemGuiProvider::truncateTable( QgsRSLayerItem *layerItem )
{
  const QgsRedshiftLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ),
                              QObject::tr( "Are you sure you want to truncate %1.%2?\n\nThis will "
                                  "delete all data within the table." )
                              .arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsDataSourceUri dsUri( layerItem->uri() );
  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Truncate Table" ), tr( "Unable to truncate table." ) );
    return;
  }

  QString schemaName = layerInfo.schemaName;
  QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsRedshiftConn::quotedIdentifier( schemaName ) + '.';
  }
  QString tableRef = schemaTableName + QgsRedshiftConn::quotedIdentifier( tableName );

  QString sql = QStringLiteral( "TRUNCATE TABLE %1" ).arg( tableRef );

  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Truncate Table" ),
                          tr( "Unable to truncate %1\n%2" ).arg( layerItem->name(), result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Truncate Table" ), tr( "Table truncated successfully." ) );
}

void QgsRedshiftDataItemGuiProvider::refreshMaterializedView( QgsRSLayerItem *layerItem )
{
  const QgsRedshiftLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Refresh Materialized View" ),
                              QObject::tr( "Are you sure you want to refresh the materialized view "
                                  "%1.%2?\n\nThis will update all data within the view." )
                              .arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsDataSourceUri dsUri( layerItem->uri() );
  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Refresh Materialized View" ), tr( "Unable to refresh the view." ) );
    return;
  }

  QString schemaName = layerInfo.schemaName;
  QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsRedshiftConn::quotedIdentifier( schemaName ) + '.';
  }
  QString tableRef = schemaTableName + QgsRedshiftConn::quotedIdentifier( tableName );

  QString sql = QStringLiteral( "REFRESH MATERIALIZED VIEW %1" ).arg( tableRef );

  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Refresh Materialized View" ),
                          tr( "Unable to refresh view %1\n%2" ).arg( layerItem->name(), result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Refresh View" ), tr( "Materialized view refreshed successfully." ) );
}

void QgsRedshiftDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::Redshift );
  dlg.exec();
}

void QgsRedshiftDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  QString fileName =
    QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::Redshift,
                                  fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
