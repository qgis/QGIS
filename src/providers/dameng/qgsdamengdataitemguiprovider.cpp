/***************************************************************************
    qgsdamengdataitemguiprovider.cpp
    --------------------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengdataitemguiprovider.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsdamengdataitems.h"
#include "qgsdamengprovider.h"
#include "qgsdamengnewconnection.h"
#include "qgsnewnamedialog.h"
#include "qgsdamengsourceselect.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>


void QgsDamengDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *>&, QgsDataItemGuiContext context )
{
  if ( QgsDamengRootItem *rootItem = qobject_cast<QgsDamengRootItem *>( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); });
    menu->addAction( actionNew );

    QAction *actionSaveServers = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSaveServers, &QAction::triggered, this, [] { saveConnections(); });
    menu->addAction( actionSaveServers );

    QAction *actionLoadServers = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoadServers, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); });
    menu->addAction( actionLoadServers );
  }

  if ( QgsDamengConnectionItem *connItem = qobject_cast<QgsDamengConnectionItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); });
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); });
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), menu );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); });
    menu->addAction( actionDelete );

    menu->addSeparator();

    QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), menu );
    connect( actionCreateSchema, &QAction::triggered, this, [connItem, context] { createSchema( connItem, context ); });
    menu->addAction( actionCreateSchema );

  }

  if ( QgsDamengSchemaItem *schemaItem = qobject_cast<QgsDamengSchemaItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [schemaItem] { schemaItem->refresh(); });
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionDelete = new QAction( tr( "Delete Schema…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [schemaItem, context] { deleteSchema( schemaItem, context ); });
    menu->addAction( actionDelete );

  }

  if ( QgsDamengLayerItem *layerItem = qobject_cast<QgsDamengLayerItem *>( item ) )
  {
    const QgsDamengLayerProperty &layerInfo = layerItem->layerInfo();
    if ( layerInfo.isView )
    {
      return;
    }

    const QString typeName = layerInfo.isMaterializedView ? tr( "View" ) : tr( "Table" );

    QMenu *maintainMenu = new QMenu( tr( "%1 Operations" ).arg( typeName ), menu );

    if ( !layerInfo.isMaterializedView )
    {
      QAction *actionTruncateLayer = new QAction( tr( "Truncate %1…" ).arg( typeName ), menu );
      connect( actionTruncateLayer, &QAction::triggered, this, [layerItem, context] { truncateTable( layerItem, context ); });
      maintainMenu->addAction( actionTruncateLayer );
      
      QAction *actionRenameLayer = new QAction( tr( "Rename %1…" ).arg( typeName ), menu);
      connect( actionRenameLayer, &QAction::triggered, this, [layerItem, context] { renameLayer( layerItem, context ); });
      maintainMenu->addAction( actionRenameLayer );
    }
    else
    {
      QAction *actionRefreshMaterializedView = new QAction( tr( "Refresh Materialized View…" ), menu );
      connect( actionRefreshMaterializedView, &QAction::triggered, this, [layerItem, context] { refreshMaterializedView( layerItem, context ); });
      maintainMenu->addAction( actionRefreshMaterializedView );
    }
    menu->addMenu( maintainMenu );
  }
}


bool QgsDamengDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsDamengLayerItem *layerItem = qobject_cast< QgsDamengLayerItem * >( item ) )
  {
    const QgsDamengLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = layerInfo.isView || layerInfo.isMaterializedView ? tr( "View" ) : tr( "Table" );

    if ( QMessageBox::question( nullptr, tr( "Delete %1" ).arg( typeName ),
                                QObject::tr( "Are you sure you want to delete %1 '%2.%3'?" ).arg( typeName.toLower(), layerInfo.schemaName, layerInfo.tableName ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    QString errCause;
    const bool res = QgsDamengUtils::deleteLayer( layerItem->uri(), errCause );
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

bool QgsDamengDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast< QgsDamengConnectionItem * >( item ) )
    return true;
  if ( qobject_cast< QgsDamengSchemaItem * >( item ) )
    return true;

  return false;
}

bool QgsDamengDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data, Qt::DropAction )
{
  if ( QgsDamengConnectionItem *connItem = qobject_cast< QgsDamengConnectionItem * >( item ) )
  {
    return connItem->handleDrop( data, QString() );
  }
  else if ( QgsDamengSchemaItem *schemaItem = qobject_cast< QgsDamengSchemaItem * >( item ) )
  {
    QgsDamengConnectionItem *connItem = qobject_cast<QgsDamengConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return connItem->handleDrop( data, schemaItem->name() );
  }
  return false;
}

QString QgsDamengDataItemGuiProvider::name()
{
  return QStringLiteral( "Dameng" );
}

QWidget *QgsDamengDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  QgsDamengRootItem *dmRootItem = qobject_cast<QgsDamengRootItem *>( root );
  if ( dmRootItem != nullptr )
  {
    QgsDamengSourceSelect *select = new QgsDamengSourceSelect( nullptr, QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode::Manager );
    connect( select, &QgsDamengSourceSelect::connectionsChanged, dmRootItem, &QgsDamengRootItem::onConnectionsChanged );
    return select;
  }
  else
  {
    return nullptr;
  }
}


void QgsDamengDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsDamengNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsDamengDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsDamengNewConnection nc( nullptr, item->name() );
  nc.setWindowTitle( tr( "Edit Dameng Connection" ) );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsDamengDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connection" ),
                              QObject::tr( "Are you sure you want to remove the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "dameng" ) );
  md->deleteConnection( item->name() );

  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsDamengDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}


void QgsDamengDataItemGuiProvider::createSchema( QgsDataItem *item, QgsDataItemGuiContext context )
{
  const QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  const QgsDataSourceUri uri = QgsDamengConn::connUri( item->name() );
  QgsDamengConn *conn = QgsDamengConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "New Schema" ), tr( "Unable to create schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //create the schema
  const QString sql = QStringLiteral( "CREATE SCHEMA %1" ).arg( QgsDamengConn::quotedIdentifier( schemaName ) );

  QgsDamengResult result( conn->DMexec( sql ) );
  if ( result.DMresultStatus() != DmResCommandOk )
  {
    notify( tr( "New Schema" ), tr( "Unable to create schema '%1'\n%2" ).arg( schemaName,
            result.DMresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
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

void QgsDamengDataItemGuiProvider::deleteSchema( QgsDamengSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  // check if schema contains tables/views
  const QgsDataSourceUri uri = QgsDamengConn::connUri( schemaItem->connectionName() );
  QgsDamengConn *conn = QgsDamengConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "Delete Schema" ), tr( "Unable to delete schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ),
                                QObject::tr( "Are you sure you want to delete schema '%1'?" ).arg( schemaItem->name() ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;

  QString errCause;
  const bool res = QgsDamengUtils::deleteSchema( schemaItem->name(), uri, errCause, true );
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

void QgsDamengDataItemGuiProvider::renameLayer( QgsDamengLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsDamengLayerProperty &layerInfo = layerItem->layerInfo();

  QgsNewNameDialog dlg( tr( "table %1.%2" ).arg( layerInfo.schemaName, layerInfo.tableName ), layerInfo.tableName );
  dlg.setWindowTitle( tr( "Rename Table" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == layerInfo.tableName )
    return;

  const QString schemaName = layerInfo.schemaName;
  const QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsDamengConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString oldName = schemaTableName + QgsDamengConn::quotedIdentifier( tableName );
  const QString newName = QgsDamengConn::quotedIdentifier( dlg.name() );

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    notify( tr( "Rename Table" ), tr( "Unable to rename 'table'." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //rename the layer
  QString sql = QStringLiteral( "ALTER TABLE %1 RENAME TO %2" ).arg( oldName, newName );

  QgsDamengResult result( conn->DMexec( sql ) );
  if ( result.DMresultStatus() != DmResCommandOk )
  {
    notify( tr( "Rename Table" ), tr( "Unable to rename 'table' %1\n%2" ).arg( layerItem->name(),
            result.DMresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  notify( tr( "Rename Table" ), tr( "Table '%1' renamed correctly to '%2'." ).arg( oldName, newName ),
          context, Qgis::MessageLevel::Success );

  conn->unref();

  if ( layerItem->parent() )
    layerItem->parent()->refresh();
}

void QgsDamengDataItemGuiProvider::truncateTable( QgsDamengLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsDamengLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ),
                              QObject::tr( "Are you sure you want to truncate \"%1.%2\"?\n\nThis will delete all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
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
    schemaTableName = QgsDamengConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString tableRef = schemaTableName + QgsDamengConn::quotedIdentifier( tableName );

  const QString sql = QStringLiteral( "TRUNCATE TABLE %1" ).arg( tableRef );

  QgsDamengResult result( conn->DMexec( sql ) );
  if ( result.DMresultStatus() != DmResCommandOk )
  {
    notify( tr( "Truncate Table" ), tr( "Unable to truncate '%1'\n%2" ).arg( tableName,
            result.DMresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();
  notify( tr( "Truncate Table" ), tr( "Table '%1' truncated successfully." ).arg( tableName ), context, Qgis::MessageLevel::Success );
}

void QgsDamengDataItemGuiProvider::refreshMaterializedView( QgsDamengLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsDamengLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Refresh Materialized View" ),
                              QObject::tr( "Are you sure you want to refresh the materialized view \"%1.%2\"?\n\nThis will update all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
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
    schemaTableName = QgsDamengConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString tableRef = schemaTableName + QgsDamengConn::quotedIdentifier( tableName );

  const QString sql = QStringLiteral( "REFRESH MATERIALIZED VIEW %1" ).arg( tableRef );

  QgsDamengResult result( conn->DMexec( sql ) );
  if ( result.DMresultStatus() != DmResCommandOk )
  {
    notify( tr( "Refresh View" ), tr( "Unable to refresh the view '%1'\n%2" ).arg( tableRef,
            result.DMresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();
  notify( tr( "Refresh View" ), tr( "Materialized view '%1' refreshed successfully." ).arg( tableName ), context, Qgis::MessageLevel::Success );
}

void QgsDamengDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::Dameng );
  dlg.exec();
}

void QgsDamengDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML )" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::Dameng, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
