/***************************************************************************
  qgspostgresdataitemguiprovider.cpp
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresdataitemguiprovider.h"

#include "qgsmanageconnectionsdialog.h"
#include "qgspostgresdataitems.h"
#include "qgspostgresprovider.h"
#include "qgspgnewconnection.h"
#include "qgsnewnamedialog.h"
#include "qgspgsourceselect.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>


void QgsPostgresDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  if ( QgsPGRootItem *rootItem = qobject_cast< QgsPGRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionSaveServers = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSaveServers, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSaveServers );

    QAction *actionLoadServers = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoadServers, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoadServers );
  }

  if ( QgsPGConnectionItem *connItem = qobject_cast< QgsPGConnectionItem * >( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), menu );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); } );
    menu->addAction( actionDelete );

    menu->addSeparator();

    QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), menu );
    connect( actionCreateSchema, &QAction::triggered, this, [connItem, context] { createSchema( connItem, context ); } );
    menu->addAction( actionCreateSchema );

  }

  if ( QgsPGSchemaItem *schemaItem = qobject_cast< QgsPGSchemaItem * >( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [schemaItem] { schemaItem->refresh(); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QMenu *maintainMenu = new QMenu( tr( "Schema Operations" ), menu );

    QAction *actionRename = new QAction( tr( "Rename Schema…" ), menu );
    connect( actionRename, &QAction::triggered, this, [schemaItem, context] { renameSchema( schemaItem, context ); } );
    maintainMenu->addAction( actionRename );

    QAction *actionDelete = new QAction( tr( "Delete Schema…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [schemaItem, context] { deleteSchema( schemaItem, context ); } );
    maintainMenu->addAction( actionDelete );

    menu->addMenu( maintainMenu );
  }

  if ( QgsPGLayerItem *layerItem = qobject_cast< QgsPGLayerItem * >( item ) )
  {
    const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );

    QMenu *maintainMenu = new QMenu( tr( "%1 Operations" ).arg( typeName ), menu );

    QAction *actionRenameLayer = new QAction( tr( "Rename %1…" ).arg( typeName ), menu );
    connect( actionRenameLayer, &QAction::triggered, this, [layerItem, context] { renameLayer( layerItem, context ); } );
    maintainMenu->addAction( actionRenameLayer );

    if ( !layerInfo.isView )
    {
      QAction *actionTruncateLayer = new QAction( tr( "Truncate %1…" ).arg( typeName ), menu );
      connect( actionTruncateLayer, &QAction::triggered, this, [layerItem, context] { truncateTable( layerItem, context ); } );
      maintainMenu->addAction( actionTruncateLayer );
    }

    if ( layerInfo.isMaterializedView )
    {
      QAction *actionRefreshMaterializedView = new QAction( tr( "Refresh Materialized View…" ), menu );
      connect( actionRefreshMaterializedView, &QAction::triggered, this, [layerItem, context] { refreshMaterializedView( layerItem, context ); } );
      maintainMenu->addAction( actionRefreshMaterializedView );
    }
    menu->addMenu( maintainMenu );
  }
}


bool QgsPostgresDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsPGLayerItem *layerItem = qobject_cast< QgsPGLayerItem * >( item ) )
  {
    const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );

    if ( QMessageBox::question( nullptr, tr( "Delete %1" ).arg( typeName ),
                                QObject::tr( "Are you sure you want to delete %1 '%2.%3'?" ).arg( typeName.toLower(), layerInfo.schemaName, layerInfo.tableName ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    QString errCause;
    const bool res = QgsPostgresUtils::deleteLayer( layerItem->uri(), errCause );
    if ( !res )
    {
      notify( tr( "Delete %1" ).arg( typeName ), errCause, context, Qgis::MessageLevel::Warning );
      return false;
    }
    else
    {
      notify( tr( "Delete %1" ).arg( typeName ), tr( "%1 '%2' deleted successfully." ).arg( typeName, layerInfo.tableName ), context, Qgis::MessageLevel::Success );
      if ( layerItem->parent() )
        layerItem->parent()->refresh();
      return true;
    }
  }
  return false;
}

bool QgsPostgresDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast< QgsPGConnectionItem * >( item ) )
    return true;
  if ( qobject_cast< QgsPGSchemaItem * >( item ) )
    return true;

  return false;
}

bool QgsPostgresDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data, Qt::DropAction )
{
  if ( QgsPGConnectionItem *connItem = qobject_cast< QgsPGConnectionItem * >( item ) )
  {
    return connItem->handleDrop( data, QString() );
  }
  else if ( QgsPGSchemaItem *schemaItem = qobject_cast< QgsPGSchemaItem * >( item ) )
  {
    QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return connItem->handleDrop( data, schemaItem->name() );
  }
  return false;
}

QWidget *QgsPostgresDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  QgsPGRootItem *pgRootItem = qobject_cast<QgsPGRootItem *>( root );
  if ( pgRootItem != nullptr )
  {
    QgsPgSourceSelect *select = new QgsPgSourceSelect( nullptr, QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode::Manager );
    connect( select, &QgsPgSourceSelect::connectionsChanged, pgRootItem, &QgsPGRootItem::onConnectionsChanged );
    return select;
  }
  else
  {
    return nullptr;
  }
}


void QgsPostgresDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsPgNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsPostgresDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsPgNewConnection nc( nullptr, item->name() );
  nc.setWindowTitle( tr( "Edit PostGIS Connection" ) );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsPostgresDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connection" ),
                              QObject::tr( "Are you sure you want to remove the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) );
  md->deleteConnection( item->name() );

  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsPostgresDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsPostgresDataItemGuiProvider::createSchema( QgsDataItem *item, QgsDataItemGuiContext context )
{
  const QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  const QgsDataSourceUri uri = QgsPostgresConn::connUri( item->name() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "New Schema" ), tr( "Unable to create schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //create the schema
  const QString sql = QStringLiteral( "CREATE SCHEMA %1" ).arg( QgsPostgresConn::quotedIdentifier( schemaName ) );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "New Schema" ), tr( "Unable to create schema '%1'\n%2" ).arg( schemaName,
            result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();

  notify( tr( "New Schema" ), tr( "Schema '%1' created successfully." ).arg( schemaName ), context, Qgis::MessageLevel::Success );

  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}


void QgsPostgresDataItemGuiProvider::deleteSchema( QgsPGSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  // check if schema contains tables/views
  const QgsDataSourceUri uri = QgsPostgresConn::connUri( schemaItem->connectionName() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "Delete Schema" ), tr( "Unable to delete schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString sql = QStringLiteral( "SELECT table_name FROM information_schema.tables WHERE table_schema='%1'" ).arg( schemaItem->name() );
  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    notify( tr( "Delete Schema" ), tr( "Unable to delete schema." ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  QStringList childObjects;
  const int maxListed = 10;
  for ( int idx = 0; idx < result.PQntuples(); idx++ )
  {
    childObjects << result.PQgetvalue( idx, 0 );
    const QgsPostgresSchemaProperty schema;
    if ( idx == maxListed - 1 )
      break;
  }

  const int count = result.PQntuples();
  if ( count > 0 )
  {
    QString objects = childObjects.join( QLatin1Char( '\n' ) );
    if ( count > maxListed )
    {
      objects += QStringLiteral( "\n[%1 additional objects not listed]" ).arg( count - maxListed );
    }
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ),
                                QObject::tr( "Schema '%1' contains objects:\n\n%2\n\nAre you sure you want to delete the schema and all these objects?" ).arg( schemaItem->name(), objects ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      conn->unref();
      return;
    }
  }
  else
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ),
                                QObject::tr( "Are you sure you want to delete schema '%1'?" ).arg( schemaItem->name() ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  QString errCause;
  const bool res = QgsPostgresUtils::deleteSchema( schemaItem->name(), uri, errCause, count > 0 );
  if ( !res )
  {
    notify( tr( "Delete Schema" ), tr( "Unable to delete schema: '%1'." ).arg( errCause ), context, Qgis::MessageLevel::Warning );
  }
  else
  {
    notify( tr( "Delete Schema" ), tr( "Schema '%1' deleted successfully." ).arg( schemaItem->name() ), context, Qgis::MessageLevel::Success );
    if ( schemaItem->parent() )
      schemaItem->parent()->refresh();
  }
}

void QgsPostgresDataItemGuiProvider::renameSchema( QgsPGSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  QgsNewNameDialog dlg( tr( "schema '%1'" ).arg( schemaItem->name() ), schemaItem->name() );
  dlg.setWindowTitle( tr( "Rename Schema" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == schemaItem->name() )
    return;

  const QString schemaName = QgsPostgresConn::quotedIdentifier( schemaItem->name() );
  const QgsDataSourceUri uri = QgsPostgresConn::connUri( schemaItem->connectionName() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "Rename Schema" ), tr( "Unable to rename schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //rename the schema
  const QString sql = QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" )
                      .arg( schemaName, QgsPostgresConn::quotedIdentifier( dlg.name() ) );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Rename Schema" ), tr( "Unable to rename schema '%1'\n%2" ).arg( schemaItem->name(),
            result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  notify( tr( "Rename Schema" ), tr( "Schema '%1' renamed correctly to '%2'." ).arg( schemaItem->name(),
          dlg.name() ), context, Qgis::MessageLevel::Success );

  conn->unref();
  if ( schemaItem->parent() )
    schemaItem->parent()->refresh();
}

void QgsPostgresDataItemGuiProvider::renameLayer( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  const QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );
  const QString lowerTypeName = layerInfo.isView ? tr( "view" ) : tr( "table" );

  QgsNewNameDialog dlg( tr( "%1 %2.%3" ).arg( lowerTypeName, layerInfo.schemaName, layerInfo.tableName ), layerInfo.tableName );
  dlg.setWindowTitle( tr( "Rename %1" ).arg( typeName ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == layerInfo.tableName )
    return;

  const QString schemaName = layerInfo.schemaName;
  const QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString oldName = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );
  const QString newName = QgsPostgresConn::quotedIdentifier( dlg.name() );

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename '%1'." ).arg( lowerTypeName ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //rename the layer
  QString sql;
  if ( layerInfo.isView )
  {
    sql = QStringLiteral( "ALTER %1 VIEW %2 RENAME TO %3" ).arg( layerInfo.relKind == QLatin1String( "m" ) ? QStringLiteral( "MATERIALIZED" ) : QString(),
          oldName, newName );
  }
  else
  {
    sql = QStringLiteral( "ALTER TABLE %1 RENAME TO %2" ).arg( oldName, newName );
  }

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename '%1' %2\n%3" ).arg( lowerTypeName, layerItem->name(),
            result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  notify( tr( "Rename %1" ).arg( typeName ), tr( "%1 '%2' renamed correctly to '%3'." ).arg( typeName, oldName, newName ),
          context, Qgis::MessageLevel::Success );

  conn->unref();

  if ( layerItem->parent() )
    layerItem->parent()->refresh();
}

void QgsPostgresDataItemGuiProvider::truncateTable( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ),
                              QObject::tr( "Are you sure you want to truncate \"%1.%2\"?\n\nThis will delete all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "Truncate Table" ), tr( "Unable to truncate table." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString schemaName = layerInfo.schemaName;
  const QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  const QString sql = QStringLiteral( "TRUNCATE TABLE %1" ).arg( tableRef );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Truncate Table" ), tr( "Unable to truncate '%1'\n%2" ).arg( tableName,
            result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();
  notify( tr( "Truncate Table" ), tr( "Table '%1' truncated successfully." ).arg( tableName ), context, Qgis::MessageLevel::Success );
}

void QgsPostgresDataItemGuiProvider::refreshMaterializedView( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Refresh Materialized View" ),
                              QObject::tr( "Are you sure you want to refresh the materialized view \"%1.%2\"?\n\nThis will update all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "Refresh View" ), tr( "Unable to refresh the view." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString schemaName = layerInfo.schemaName;
  const QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  const QString sql = QStringLiteral( "REFRESH MATERIALIZED VIEW CONCURRENTLY %1" ).arg( tableRef );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Refresh View" ), tr( "Unable to refresh the view '%1'\n%2" ).arg( tableRef,
            result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();
  notify( tr( "Refresh View" ), tr( "Materialized view '%1' refreshed successfully." ).arg( tableName ), context, Qgis::MessageLevel::Success );
}

void QgsPostgresDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::PostGIS );
  dlg.exec();
}

void QgsPostgresDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::PostGIS, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
