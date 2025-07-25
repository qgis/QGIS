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
#include "moc_qgspostgresdataitemguiprovider.cpp"

#include "qgsapplication.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgspostgresdataitems.h"
#include "qgspgnewconnection.h"
#include "qgsnewnamedialog.h"
#include "qgspgsourceselect.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgssettings.h"
#include "qgspostgresconn.h"
#include "qgspostgresutils.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsvectorlayerexporter.h"
#include "qgstaskmanager.h"
#include "qgsmessageoutput.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsdbimportvectorlayerdialog.h"
#include "qgsproject.h"
#include "qgsdatabaseschemaselectiondialog.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>


void QgsPostgresDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsPGRootItem *rootItem = qobject_cast<QgsPGRootItem *>( item ) )
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

  if ( QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( item ) )
  {
    const QList<QgsPGConnectionItem *> pgConnectionItems = QgsDataItem::filteredItems<QgsPGConnectionItem>( selection );

    if ( pgConnectionItems.size() == 1 )
    {
      QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
      connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
      menu->addAction( actionRefresh );

      menu->addSeparator();

      QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
      connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
      menu->addAction( actionEdit );

      QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), menu );
      connect( actionDuplicate, &QAction::triggered, this, [connItem] { duplicateConnection( connItem ); } );
      menu->addAction( actionDuplicate );
    }

    QAction *actionDelete = new QAction( pgConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [pgConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( pgConnectionItems, []( const QString &connectionName ) {
        QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) );
        md->deleteConnection( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );

    if ( pgConnectionItems.size() == 1 )
    {
      menu->addSeparator();

      QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), menu );
      connect( actionCreateSchema, &QAction::triggered, this, [connItem, context] { createSchema( connItem, context ); } );
      menu->addAction( actionCreateSchema );
    }
  }

  if ( QgsPGSchemaItem *schemaItem = qobject_cast<QgsPGSchemaItem *>( item ) )
  {
    QAction *importVectorAction = new QAction( QObject::tr( "Import Vector Layer…" ), menu );
    menu->addAction( importVectorAction );
    const QString destinationSchema = schemaItem->name();
    QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( schemaItem->parent() );
    QObject::connect( importVectorAction, &QAction::triggered, item, [connItem, context, destinationSchema, this] { handleImportVector( connItem, destinationSchema, context ); } );

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

  if ( QgsPGLayerItem *layerItem = qobject_cast<QgsPGLayerItem *>( item ) )
  {
    const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = typeNameFromLayer( layerInfo );

    QMenu *maintainMenu = new QMenu( tr( "%1 Operations" ).arg( typeName ), menu );

    QAction *actionRenameLayer = new QAction( tr( "Rename %1…" ).arg( typeName ), menu );
    connect( actionRenameLayer, &QAction::triggered, this, [layerItem, context] { renameLayer( layerItem, context ); } );
    maintainMenu->addAction( actionRenameLayer );

    if ( layerInfo.relKind != Qgis::PostgresRelKind::View && layerInfo.relKind != Qgis::PostgresRelKind::MaterializedView )
    {
      QAction *actionTruncateLayer = new QAction( tr( "Truncate %1…" ).arg( typeName ), menu );
      connect( actionTruncateLayer, &QAction::triggered, this, [layerItem, context] { truncateTable( layerItem, context ); } );
      maintainMenu->addAction( actionTruncateLayer );
    }

    if ( layerInfo.relKind == Qgis::PostgresRelKind::MaterializedView )
    {
      QAction *actionRefreshMaterializedView = new QAction( tr( "Refresh Materialized View…" ), menu );
      connect( actionRefreshMaterializedView, &QAction::triggered, this, [layerItem, context] { refreshMaterializedView( layerItem, context ); } );
      maintainMenu->addAction( actionRefreshMaterializedView );
    }
    menu->addMenu( maintainMenu );
  }

  if ( QgsPGProjectItem *projectItem = qobject_cast<QgsPGProjectItem *>( item ) )
  {
    QAction *exportProjectToFileAction = new QAction( tr( "Export Project to File…" ), menu );
    connect( exportProjectToFileAction, &QAction::triggered, this, [projectItem, context] { exportProjectToFile( projectItem, context ); } );
    menu->addAction( exportProjectToFileAction );

    QAction *renameProjectAction = new QAction( tr( "Rename Project…" ), menu );
    connect( renameProjectAction, &QAction::triggered, this, [projectItem, context] { renameProject( projectItem, context ); } );
    menu->addAction( renameProjectAction );

    QAction *deleteProjectAction = new QAction( tr( "Delete Project…" ), menu );
    connect( deleteProjectAction, &QAction::triggered, this, [projectItem, context] { deleteProject( projectItem, context ); } );
    menu->addAction( deleteProjectAction );

    QAction *duplicateProjectAction = new QAction( tr( "Duplicate Project…" ), menu );
    connect( duplicateProjectAction, &QAction::triggered, this, [projectItem, context] { duplicateProject( projectItem, context ); } );
    menu->addAction( duplicateProjectAction );

    QAction *moveProjectToSchemaAction = new QAction( tr( "Move Project to Schema…" ), menu );
    connect( moveProjectToSchemaAction, &QAction::triggered, this, [projectItem, context] { moveProjectToSchema( projectItem, context ); } );
    menu->addAction( moveProjectToSchemaAction );
  }
}


bool QgsPostgresDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsPGLayerItem *layerItem = qobject_cast<QgsPGLayerItem *>( item ) )
  {
    const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = typeNameFromLayer( layerInfo );

    if ( QMessageBox::question( nullptr, tr( "Delete %1" ).arg( typeName ), QObject::tr( "Are you sure you want to delete %1 '%2.%3'?" ).arg( typeName.toLower(), layerInfo.schemaName, layerInfo.tableName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
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
  if ( qobject_cast<QgsPGConnectionItem *>( item ) )
    return true;
  if ( qobject_cast<QgsPGSchemaItem *>( item ) )
    return true;

  return false;
}

bool QgsPostgresDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction )
{
  if ( QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( item ) )
  {
    return handleDrop( connItem, data, QString(), context );
  }
  else if ( QgsPGSchemaItem *schemaItem = qobject_cast<QgsPGSchemaItem *>( item ) )
  {
    QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return handleDrop( connItem, data, schemaItem->name(), context );
  }
  return false;
}

QWidget *QgsPostgresDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  QgsPGRootItem *pgRootItem = qobject_cast<QgsPGRootItem *>( root );
  if ( pgRootItem )
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

QString QgsPostgresDataItemGuiProvider::typeNameFromLayer( const QgsPostgresLayerProperty &layer )
{
  switch ( layer.relKind )
  {
    case Qgis::PostgresRelKind::View:
      return tr( "View" );

    case Qgis::PostgresRelKind::MaterializedView:
      return tr( "Materialized View" );

    case Qgis::PostgresRelKind::NotSet:
    case Qgis::PostgresRelKind::Unknown:
    case Qgis::PostgresRelKind::OrdinaryTable:
    case Qgis::PostgresRelKind::Index:
    case Qgis::PostgresRelKind::Sequence:
    case Qgis::PostgresRelKind::CompositeType:
    case Qgis::PostgresRelKind::ToastTable:
    case Qgis::PostgresRelKind::ForeignTable:
    case Qgis::PostgresRelKind::PartitionedTable:
      return tr( "Table" );
  }

  BUILTIN_UNREACHABLE
}

void QgsPostgresDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsPgNewConnection nc( QgsApplication::instance()->activeWindow() );
  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsPostgresDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsPgNewConnection nc( nullptr, item->name() );
  nc.setWindowTitle( tr( "Edit PostgreSQL Connection" ) );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsPostgresDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/PostgreSQL/connections" ) );
  const QStringList connections = settings.childGroups();
  settings.endGroup();

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  QgsPostgresConn::duplicateConnection( connectionName, newConnectionName );

  if ( item->parent() )
  {
    item->parent()->refreshConnections();
  }
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
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
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
    notify( tr( "New Schema" ), tr( "Unable to create schema '%1'\n%2" ).arg( schemaName, result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
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
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
  if ( !conn )
  {
    notify( tr( "Delete Schema" ), tr( "Unable to delete schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString sql = QStringLiteral( "SELECT table_name FROM information_schema.tables WHERE table_schema=%1" ).arg( QgsPostgresConn::quotedValue( schemaItem->name() ) );
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
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ), QObject::tr( "Schema '%1' contains objects:\n\n%2\n\nAre you sure you want to delete the schema and all these objects?" ).arg( schemaItem->name(), objects ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      conn->unref();
      return;
    }
  }
  else
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ), QObject::tr( "Are you sure you want to delete schema '%1'?" ).arg( schemaItem->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
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
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
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
    notify( tr( "Rename Schema" ), tr( "Unable to rename schema '%1'\n%2" ).arg( schemaItem->name(), result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  notify( tr( "Rename Schema" ), tr( "Schema '%1' renamed correctly to '%2'." ).arg( schemaItem->name(), dlg.name() ), context, Qgis::MessageLevel::Success );

  conn->unref();
  if ( schemaItem->parent() )
    schemaItem->parent()->refresh();
}

void QgsPostgresDataItemGuiProvider::renameLayer( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  const QString typeName = typeNameFromLayer( layerInfo );
  const QString lowerTypeName = ( layerInfo.relKind == Qgis::PostgresRelKind::View || layerInfo.relKind == Qgis::PostgresRelKind::MaterializedView )
                                  ? tr( "view" )
                                  : tr( "table" );

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
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( !conn )
  {
    notify( tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename '%1'." ).arg( lowerTypeName ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //rename the layer
  QString sql;
  if ( layerInfo.relKind == Qgis::PostgresRelKind::View || layerInfo.relKind == Qgis::PostgresRelKind::MaterializedView )
  {
    sql = QStringLiteral( "ALTER %1 VIEW %2 RENAME TO %3" ).arg( layerInfo.relKind == Qgis::PostgresRelKind::MaterializedView ? QStringLiteral( "MATERIALIZED" ) : QString(), oldName, newName );
  }
  else
  {
    sql = QStringLiteral( "ALTER TABLE %1 RENAME TO %2" ).arg( oldName, newName );
  }

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename '%1' %2\n%3" ).arg( lowerTypeName, layerItem->name(), result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  notify( tr( "Rename %1" ).arg( typeName ), tr( "%1 '%2' renamed correctly to '%3'." ).arg( typeName, oldName, newName ), context, Qgis::MessageLevel::Success );

  conn->unref();

  if ( layerItem->parent() )
    layerItem->parent()->refresh();
}

void QgsPostgresDataItemGuiProvider::truncateTable( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ), QObject::tr( "Are you sure you want to truncate \"%1.%2\"?\n\nThis will delete all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
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
    notify( tr( "Truncate Table" ), tr( "Unable to truncate '%1'\n%2" ).arg( tableName, result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();
  notify( tr( "Truncate Table" ), tr( "Table '%1' truncated successfully." ).arg( tableName ), context, Qgis::MessageLevel::Success );
}

void QgsPostgresDataItemGuiProvider::refreshMaterializedView( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Refresh Materialized View" ), QObject::tr( "Are you sure you want to refresh the materialized view \"%1.%2\"?\n\nThis will update all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
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
    notify( tr( "Refresh View" ), tr( "Unable to refresh the view '%1'\n%2" ).arg( tableRef, result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
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
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::PostGIS, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

bool QgsPostgresDataItemGuiProvider::handleDrop( QgsPGConnectionItem *connectionItem, const QMimeData *data, const QString &toSchema, QgsDataItemGuiContext context )
{
  if ( !QgsMimeDataUtils::isUriList( data ) || !connectionItem )
    return false;

  const QgsMimeDataUtils::UriList sourceUris = QgsMimeDataUtils::decodeUriList( data );
  if ( sourceUris.size() == 1 && sourceUris.at( 0 ).layerType == QLatin1String( "vector" ) )
  {
    return handleDropUri( connectionItem, sourceUris.at( 0 ), toSchema, context );
  }

  QPointer< QgsPGConnectionItem > connectionItemPointer( connectionItem );

  QgsDataSourceUri uri = connectionItem->connectionUri();

  // TODO: when dropping multiple layers, we need a dedicated "bulk import" dialog for settings which apply to ALL layers

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  QStringList importResults;
  bool hasError = false;

  for ( const QgsMimeDataUtils::Uri &u : sourceUris )
  {
    // open the source layer
    bool owner;
    QString error;
    QgsVectorLayer *srcLayer = u.vectorLayer( owner, error );
    if ( !srcLayer )
    {
      importResults.append( tr( "%1: %2" ).arg( u.name, error ) );
      hasError = true;
      continue;
    }

    if ( srcLayer->isValid() )
    {
      // Try to get source col from uri

      QString geomColumn { QStringLiteral( "geom" ) };
      if ( !srcLayer->dataProvider()->geometryColumnName().isEmpty() )
      {
        geomColumn = srcLayer->dataProvider()->geometryColumnName();
      }

      QgsAbstractDatabaseProviderConnection::VectorLayerExporterOptions exporterOptions;
      exporterOptions.layerName = u.name;
      exporterOptions.schema = toSchema;
      exporterOptions.wkbType = srcLayer->wkbType();
      exporterOptions.geometryColumn = geomColumn;

      QVariantMap providerOptions;
      const QString destUri = databaseConnection->createVectorLayerExporterDestinationUri( exporterOptions, providerOptions );

      QgsDebugMsgLevel( "URI " + destUri, 2 );

      auto exportTask = std::make_unique<QgsVectorLayerExporterTask>( srcLayer, destUri, QStringLiteral( "postgres" ), srcLayer->crs(), providerOptions, owner );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [connectionItemPointer, toSchema]() {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to PostgreSQL database" ), tr( "Import was successful." ) );
        if ( connectionItemPointer )
          connectionItemPointer->refreshSchema( toSchema );
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [connectionItemPointer, toSchema]( Qgis::VectorExportResult error, const QString &errorMessage ) {
        if ( error != Qgis::VectorExportResult::UserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to PostgreSQL database" ) );
          output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        if ( connectionItemPointer )
          connectionItemPointer->refreshSchema( toSchema );
      } );

      QgsApplication::taskManager()->addTask( exportTask.release() );
    }
    else
    {
      importResults.append( tr( "%1: Not a valid layer!" ).arg( u.name ) );
      hasError = true;
    }
  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to PostgreSQL database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

bool QgsPostgresDataItemGuiProvider::handleDropUri( QgsPGConnectionItem *connectionItem, const QgsMimeDataUtils::Uri &sourceUri, const QString &toSchema, QgsDataItemGuiContext context )
{
  QPointer< QgsPGConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  auto onSuccess = [connectionItemPointer, toSchema]() {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer )
        connectionItemPointer->refreshSchema( toSchema );
    }
  };

  auto onFailure = [connectionItemPointer = std::move( connectionItemPointer ), toSchema]( Qgis::VectorExportResult, const QString & ) {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer )
        connectionItemPointer->refreshSchema( toSchema );
    }
  };

  return QgsDataItemGuiProviderUtils::handleDropUriForConnection( std::move( databaseConnection ), sourceUri, toSchema, context, tr( "PostgreSQL Import" ), tr( "Import to PostgreSQL database" ), QVariantMap(), onSuccess, onFailure, this );
}

void QgsPostgresDataItemGuiProvider::handleImportVector( QgsPGConnectionItem *connectionItem, const QString &toSchema, QgsDataItemGuiContext context )
{
  if ( !connectionItem )
    return;

  QPointer< QgsPGConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return;

  auto onSuccess = [connectionItemPointer, toSchema]() {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer )
        connectionItemPointer->refreshSchema( toSchema );
    }
  };

  auto onFailure = [connectionItemPointer = std::move( connectionItemPointer ), toSchema]( Qgis::VectorExportResult, const QString & ) {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer )
        connectionItemPointer->refreshSchema( toSchema );
    }
  };

  QgsDataItemGuiProviderUtils::handleImportVectorLayerForConnection( std::move( databaseConnection ), toSchema, context, tr( "PostgreSQL Import" ), tr( "Import to PostgreSQL database" ), QVariantMap(), onSuccess, onFailure, this );
}

void QgsPostgresDataItemGuiProvider::exportProjectToFile( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  QgsSettings settings;
  const QString defaultPath = settings.value( QStringLiteral( "UI/lastProjectDir" ), QDir::homePath() ).toString();

  const Qgis::ProjectFileFormat defaultProjectFileFormat = settings.enumValue( QStringLiteral( "/qgis/defaultProjectFileFormat" ), Qgis::ProjectFileFormat::Qgz );
  const QString qgisProjectExt = tr( "QGIS Project Formats" ) + ( defaultProjectFileFormat == Qgis::ProjectFileFormat::Qgz ? " (*.qgz *.QGZ *.qgs *.QGS)" : " (*.qgs *.QGS *.qgz *.QGZ)" );
  const QString qgzProjectExt = tr( "QGIS Bundled Project Format" ) + " (*.qgz *.QGZ)";
  const QString qgsProjectExt = tr( "QGIS XML Project Format" ) + " (*.qgs *.QGS)";

  QString filter;
  const QString path = QFileDialog::getSaveFileName(
    nullptr,
    tr( "Save Project As" ),
    defaultPath,
    qgisProjectExt + QStringLiteral( ";;" ) + qgzProjectExt + QStringLiteral( ";;" ) + qgsProjectExt, &filter
  );

  if ( path.isEmpty() )
    return;

  QFileInfo fullPath( path );
  QgsSettings().setValue( QStringLiteral( "UI/lastProjectDir" ), fullPath.path() );

  const QString ext = fullPath.suffix().toLower();
  if ( filter == qgisProjectExt && ext != QLatin1String( "qgz" ) && ext != QLatin1String( "qgs" ) )
  {
    switch ( defaultProjectFileFormat )
    {
      case Qgis::ProjectFileFormat::Qgs:
      {
        fullPath.setFile( fullPath.filePath() + ".qgs" );
        break;
      }
      case Qgis::ProjectFileFormat::Qgz:
      {
        fullPath.setFile( fullPath.filePath() + ".qgz" );
        break;
      }
    }
  }
  else if ( filter == qgzProjectExt && ext != QLatin1String( "qgz" ) )
  {
    fullPath.setFile( fullPath.filePath() + ".qgz" );
  }
  else if ( filter == qgsProjectExt && ext != QLatin1String( "qgs" ) )
  {
    fullPath.setFile( fullPath.filePath() + ".qgs" );
  }

  QgsProject project;
  project.read( projectItem->path() );
  project.setFileName( fullPath.filePath() );
  const bool result = project.write();

  if ( !result && context.messageBar() )
  {
    context.messageBar()->pushWarning( tr( "Export Project to File" ), tr( "Could not save project file" ) );
  }
}

void QgsPostgresDataItemGuiProvider::renameProject( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  QgsNewNameDialog dlg( tr( "project “%1”" ).arg( projectItem->name() ), projectItem->name() );
  dlg.setWindowTitle( tr( "Rename Project" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == projectItem->name() )
    return;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( projectItem->postgresProjectUri().connInfo, false );
  if ( !conn )
  {
    notify( tr( "Rename Project" ), tr( "Unable to rename project." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString newUri = projectItem->uriWithNewName( dlg.name() );

  // read the project, set title and new filename
  QgsProject project;
  project.read( projectItem->path() );
  project.setTitle( dlg.name() );
  project.setFileName( newUri );

  // write project to the database
  const bool success = project.write();
  if ( !success )
  {
    notify( tr( "Rename Project" ), tr( "Unable to rename project “%1” to “%2”" ).arg( projectItem->name(), dlg.name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  if ( !QgsPostgresUtils::deleteProjectFromSchema( conn, projectItem->name(), projectItem->schemaName() ) )
  {
    notify( tr( "Rename Project" ), tr( "Unable to rename project “%1” to “%2”" ).arg( projectItem->name(), dlg.name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  // refresh
  projectItem->parent()->refresh();

  conn->unref();
}

void QgsPostgresDataItemGuiProvider::deleteProject( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  if ( QMessageBox::question( nullptr, tr( "Delete Project" ), tr( "Are you sure you want to delete project “%1”?" ).arg( projectItem->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( projectItem->postgresProjectUri().connInfo, false );
  if ( !conn )
  {
    notify( tr( "Delete Project" ), tr( "Unable to delete project." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  if ( !QgsPostgresUtils::deleteProjectFromSchema( conn, projectItem->name(), projectItem->schemaName() ) )
  {
    notify( tr( "Delete Project" ), tr( "Unable to delete project “%1” " ).arg( projectItem->name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  // refresh
  projectItem->parent()->refresh();

  conn->unref();
}

void QgsPostgresDataItemGuiProvider::duplicateProject( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  QgsNewNameDialog dlg( tr( "Project “%1”" ).arg( projectItem->name() ), projectItem->name() );
  dlg.setWindowTitle( tr( "Duplicate Project" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == projectItem->name() )
    return;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( projectItem->postgresProjectUri().connInfo, false );
  if ( !conn )
  {
    notify( tr( "Duplicate Project" ), tr( "Unable to duplicate project." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString newUri = projectItem->uriWithNewName( dlg.name() );

  // read the project, set title and new filename
  QgsProject project;
  project.read( projectItem->path() );
  project.setTitle( dlg.name() );
  project.setFileName( newUri );

  // write project to the database
  const bool success = project.write();
  if ( !success )
  {
    notify( tr( "Duplicate Project" ), tr( "Unable to duplicate project “%1” to “%2”" ).arg( projectItem->name(), dlg.name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  // refresh
  projectItem->parent()->refresh();
}

void QgsPostgresDataItemGuiProvider::moveProjectToSchema( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  QgsPGSchemaItem *schemaItem = qobject_cast<QgsPGSchemaItem *>( projectItem->parent() );
  if ( !schemaItem )
  {
    return;
  }

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> dbConnection( schemaItem->databaseConnection() );
  if ( !dbConnection )
  {
    return;
  }

  QgsDatabaseSchemaSelectionDialog *dlg = new QgsDatabaseSchemaSelectionDialog( dbConnection.release() );

  if ( dlg->exec() == QDialog::Accepted )
  {
    QgsPostgresConn *conn = QgsPostgresConn::connectDb( projectItem->postgresProjectUri().connInfo, false );
    if ( !conn )
    {
      notify( tr( "Move Project to Another Schema" ), tr( "Unable to move project to another schema." ), context, Qgis::MessageLevel::Warning );
      return;
    }

    const QString newSchemaName = dlg->selectedSchema();
    if ( newSchemaName == projectItem->schemaName() )
    {
      notify( tr( "Move Project to Another Schema" ), tr( "Cannot copy to the schema where the project already is." ), context, Qgis::MessageLevel::Warning );
      conn->unref();
      return;
    }

    if ( !QgsPostgresUtils::projectsTableExists( conn, newSchemaName ) )
    {
      if ( !QgsPostgresUtils::createProjectsTable( conn, newSchemaName ) )
      {
        const QString errCause = QObject::tr( "Unable to save project. It's not possible to create the destination table on the database. Maybe this is due to database permissions (user=%1). Please contact your database admin." ).arg( projectItem->postgresProjectUri().connInfo.username() );

        notify( tr( "Move Project to Another Schema" ), errCause, context, Qgis::MessageLevel::Warning );
        conn->unref();
        return;
      }
    }

    const QString sql = QStringLiteral( "INSERT INTO %1.qgis_projects SELECT * FROM %2.qgis_projects WHERE name=%3;" )
                          .arg( QgsPostgresConn::quotedIdentifier( newSchemaName ) )
                          .arg( QgsPostgresConn::quotedIdentifier( projectItem->schemaName() ) )
                          .arg( QgsPostgresConn::quotedValue( projectItem->name() ) );

    QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      notify( tr( "Move Project to Another Schema" ), tr( "Unable to move project “%1” to scheme “%2” " ).arg( projectItem->name(), newSchemaName ), context, Qgis::MessageLevel::Warning );
      conn->unref();
      return;
    }

    if ( !QgsPostgresUtils::deleteProjectFromSchema( conn, projectItem->name(), projectItem->schemaName() ) )
    {
      notify( tr( "Move Project to Another Schema" ), tr( "Unable to move project “%1” to scheme “%2” " ).arg( projectItem->name(), newSchemaName ), context, Qgis::MessageLevel::Warning );
      conn->unref();
      return;
    }

    // refresh
    projectItem->parent()->refresh();

    for ( QgsDataItem *item : projectItem->parent()->parent()->children() )
    {
      if ( QgsPGSchemaItem *schemaItem = qobject_cast<QgsPGSchemaItem *>( item ) )
      {
        if ( schemaItem->name() == newSchemaName )
        {
          schemaItem->refresh();
        }
      }
    }

    conn->unref();
  }
}
