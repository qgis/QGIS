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

#include "qgspostgresdataitems.h"
#include "qgspostgresprovider.h"
#include "qgspgnewconnection.h"
#include "qgsnewnamedialog.h"
#include "qgspgsourceselect.h"

#include <QInputDialog>
#include <QMessageBox>


void QgsPostgresDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsPGRootItem *rootItem = qobject_cast< QgsPGRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );
  }

  if ( QgsPGConnectionItem *connItem = qobject_cast< QgsPGConnectionItem * >( item ) )
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

    QAction *actionCreateSchema = new QAction( tr( "Create Schema…" ), this );
    connect( actionCreateSchema, &QAction::triggered, this, [connItem] { createSchema( connItem ); } );
    menu->addAction( actionCreateSchema );
  }

  if ( QgsPGSchemaItem *schemaItem = qobject_cast< QgsPGSchemaItem * >( item ) )
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

  if ( QgsPGLayerItem *layerItem = qobject_cast< QgsPGLayerItem * >( item ) )
  {
    const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
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
      connect( actionRefreshMaterializedView, &QAction::triggered, this, [layerItem] { refreshMaterializedView( layerItem ); } );
      menu->addAction( actionRefreshMaterializedView );
    }
  }
}


bool QgsPostgresDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext )
{
  if ( QgsPGLayerItem *layerItem = qobject_cast< QgsPGLayerItem * >( item ) )
  {
    const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
    QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );

    if ( QMessageBox::question( nullptr, tr( "Delete %1" ).arg( typeName ),
                                QObject::tr( "Are you sure you want to delete %1.%2?" ).arg( layerInfo.schemaName, layerInfo.tableName ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    QString errCause;
    bool res = QgsPostgresUtils::deleteLayer( layerItem->uri(), errCause );
    if ( !res )
    {
      QMessageBox::warning( nullptr, tr( "Delete %1" ).arg( typeName ), errCause );
      return false;
    }
    else
    {
      QMessageBox::information( nullptr, tr( "Delete %1" ).arg( typeName ), tr( "%1 deleted successfully." ).arg( typeName ) );
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
    QgsPgSourceSelect *select = new QgsPgSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
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
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsPostgresDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsPostgresConn::deleteConnection( item->name() );
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

void QgsPostgresDataItemGuiProvider::createSchema( QgsDataItem *item )
{
  QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  QgsDataSourceUri uri = QgsPostgresConn::connUri( item->name() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Create Schema" ), tr( "Unable to create schema." ) );
    return;
  }

  //create the schema
  QString sql = QStringLiteral( "CREATE SCHEMA %1" ).arg( QgsPostgresConn::quotedIdentifier( schemaName ) );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Create Schema" ), tr( "Unable to create schema %1\n%2" ).arg( schemaName,
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();

  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}


void QgsPostgresDataItemGuiProvider::deleteSchema( QgsPGSchemaItem *schemaItem )
{
  // check if schema contains tables/views
  QgsDataSourceUri uri = QgsPostgresConn::connUri( schemaItem->connectionName() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Delete Schema" ), tr( "Unable to delete schema." ) );
    return;
  }

  QString sql = QStringLiteral( "SELECT table_name FROM information_schema.tables WHERE table_schema='%1'" ).arg( schemaItem->name() );
  QgsPostgresResult result( conn->PQexec( sql ) );
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
    QgsPostgresSchemaProperty schema;
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
                                QObject::tr( "Are you sure you want to delete the schema '%1'?" ).arg( schemaItem->name() ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  QString errCause;
  bool res = QgsPostgresUtils::deleteSchema( schemaItem->name(), uri, errCause, count > 0 );
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

void QgsPostgresDataItemGuiProvider::renameSchema( QgsPGSchemaItem *schemaItem )
{
  QgsNewNameDialog dlg( tr( "schema '%1'" ).arg( schemaItem->name() ), schemaItem->name() );
  dlg.setWindowTitle( tr( "Rename Schema" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == schemaItem->name() )
    return;

  QString schemaName = QgsPostgresConn::quotedIdentifier( schemaItem->name() );
  QgsDataSourceUri uri = QgsPostgresConn::connUri( schemaItem->connectionName() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Rename Schema" ), tr( "Unable to rename schema." ) );
    return;
  }

  //rename the schema
  QString sql = QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" )
                .arg( schemaName, QgsPostgresConn::quotedIdentifier( dlg.name() ) );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Rename Schema" ), tr( "Unable to rename schema %1\n%2" ).arg( schemaName,
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Rename Schema" ), tr( "Schema renamed successfully." ) );
  if ( schemaItem->parent() )
    schemaItem->parent()->refreshConnections();
}

void QgsPostgresDataItemGuiProvider::renameLayer( QgsPGLayerItem *layerItem )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );
  QString lowerTypeName = layerInfo.isView ? tr( "view" ) : tr( "table" );

  QgsNewNameDialog dlg( tr( "%1 %2.%3" ).arg( lowerTypeName, layerInfo.schemaName, layerInfo.tableName ), layerInfo.tableName );
  dlg.setWindowTitle( tr( "Rename %1" ).arg( typeName ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == layerInfo.tableName )
    return;

  QString schemaName = layerInfo.schemaName;
  QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  QString oldName = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );
  QString newName = QgsPostgresConn::quotedIdentifier( dlg.name() );

  QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename %1." ).arg( lowerTypeName ) );
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

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename %1 %2\n%3" ).arg( lowerTypeName, layerItem->name(),
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  if ( layerItem->parent() )
    layerItem->parent()->refreshConnections();
}

void QgsPostgresDataItemGuiProvider::truncateTable( QgsPGLayerItem *layerItem )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ),
                              QObject::tr( "Are you sure you want to truncate %1.%2?\n\nThis will delete all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
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
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  QString sql = QStringLiteral( "TRUNCATE TABLE %1" ).arg( tableRef );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Truncate Table" ), tr( "Unable to truncate %1\n%2" ).arg( layerItem->name(),
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Truncate Table" ), tr( "Table truncated successfully." ) );
}

void QgsPostgresDataItemGuiProvider::refreshMaterializedView( QgsPGLayerItem *layerItem )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Refresh Materialized View" ),
                              QObject::tr( "Are you sure you want to refresh the materialized view %1.%2?\n\nThis will update all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Refresh View" ), tr( "Unable to refresh the view." ) );
    return;
  }

  QString schemaName = layerInfo.schemaName;
  QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  QString sql = QStringLiteral( "REFRESH MATERIALIZED VIEW CONCURRENTLY %1" ).arg( tableRef );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Refresh View" ), tr( "Unable to refresh view %1\n%2" ).arg( layerItem->name(),
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Refresh View" ), tr( "Materialized view refreshed successfully." ) );
}
