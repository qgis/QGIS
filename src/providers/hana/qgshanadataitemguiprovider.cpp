/***************************************************************************
   qgshanadataitemguiprovider.cpp
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
#include "qgshanaconnection.h"
#include "qgshanadataitems.h"
#include "qgshanadataitemguiprovider.h"
#include "qgshananewconnection.h"
#include "qgshanaprovider.h"
#include "qgshanaproviderconnection.h"
#include "qgshanasourceselect.h"
#include "qgshanautils.h"
#include "qgsnewnamedialog.h"

#include <QInputDialog>
#include <QMessageBox>

void QgsHanaDataItemGuiProvider::populateContextMenu(
  QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  if ( QgsHanaRootItem *rootItem = qobject_cast<QgsHanaRootItem *>( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );
  }

  if ( QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), this );
    connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), this );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), this );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); } );
    menu->addAction( actionDelete );

    menu->addSeparator();

    QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), this );
    connect( actionCreateSchema, &QAction::triggered, this, [connItem, context] { createSchema( connItem, context ); } );
    menu->addAction( actionCreateSchema );
  }

  if ( QgsHanaSchemaItem *schemaItem = qobject_cast<QgsHanaSchemaItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), this );
    connect( actionRefresh, &QAction::triggered, this, [schemaItem] { schemaItem->refresh(); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QMenu *maintainMenu = new QMenu( tr( "Schema Operations" ), menu );

    QAction *actionRename = new QAction( tr( "Rename Schema…" ), this );
    connect( actionRename, &QAction::triggered, this, [schemaItem, context] { renameSchema( schemaItem, context ); } );
    maintainMenu->addAction( actionRename );

    QAction *actionDelete = new QAction( tr( "Delete Schema…" ), this );
    connect( actionDelete, &QAction::triggered, this, [schemaItem, context] { deleteSchema( schemaItem, context ); } );
    maintainMenu->addAction( actionDelete );

    menu->addMenu( maintainMenu );
  }

  if ( QgsHanaLayerItem *layerItem = qobject_cast< QgsHanaLayerItem * >( item ) )
  {
    const QgsHanaLayerProperty &layerInfo = layerItem->layerInfo();
    if ( !layerInfo.isView )
    {
      QMenu *maintainMenu = new QMenu( tr( "Table Operations" ), menu );

      QAction *actionRenameLayer = new QAction( tr( "Rename Table…" ), this );
      connect( actionRenameLayer, &QAction::triggered, this, [layerItem, context] { renameLayer( layerItem, context ); } );
      maintainMenu->addAction( actionRenameLayer );

      menu->addMenu( maintainMenu );
    }
  }
}

bool QgsHanaDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsHanaLayerItem *layerItem = qobject_cast<QgsHanaLayerItem *>( item ) )
  {
    const QgsHanaLayerProperty &layerInfo = layerItem->layerInfo();
    const QString layerName = QStringLiteral( "%1.%2" ).arg( layerInfo.schemaName, layerInfo.tableName );
    const QString caption = tr( layerInfo.isView ? "Delete View" : "Delete Table" );
    if ( QMessageBox::question( nullptr, caption,
                                tr( "Are you sure you want to delete '%1'?" ).arg( layerName ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    QString errorMsg;
    try
    {
      const QgsHanaProviderConnection providerConn( layerItem->uri(), {} );
      providerConn.dropVectorTable( layerInfo.schemaName, layerInfo.tableName );
    }
    catch ( const QgsProviderConnectionException &ex )
    {
      errorMsg = ex.what();
    }

    if ( errorMsg.isEmpty() )
    {
      notify( caption, tr( "'%1' deleted successfully." ).arg( layerName ), context, Qgis::MessageLevel::Success );

      if ( layerItem->parent() )
        layerItem->parent()->refresh();
      return true;
    }
    else
    {
      notify( caption, errorMsg, context, Qgis::MessageLevel::Warning );
    }
  }

  return false;
}

bool QgsHanaDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast<QgsHanaConnectionItem *>( item ) )
    return true;
  if ( qobject_cast<QgsHanaSchemaItem *>( item ) )
    return true;

  return false;
}

bool QgsHanaDataItemGuiProvider::handleDrop(
  QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data, Qt::DropAction )
{
  if ( QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>( item ) )
  {
    return connItem->handleDrop( data, QString() );
  }
  else if ( QgsHanaSchemaItem *schemaItem = qobject_cast<QgsHanaSchemaItem *>( item ) )
  {
    QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return connItem->handleDrop( data, schemaItem->name() );
  }
  return false;
}

QWidget *QgsHanaDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  QgsHanaRootItem *rootItem = qobject_cast<QgsHanaRootItem *>( root );
  if ( rootItem == nullptr )
    return nullptr;
  QgsHanaSourceSelect *select = new QgsHanaSourceSelect( nullptr, QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsHanaSourceSelect::connectionsChanged, rootItem, &QgsHanaRootItem::onConnectionsChanged );
  return select;
}

void QgsHanaDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsHanaNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsHanaDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsHanaNewConnection nc( nullptr, item->name() );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsHanaDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Remove Connection" ),
                              tr( "Are you sure you want to remove the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsHanaSettings::removeConnection( item->name() );
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsHanaDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsHanaDataItemGuiProvider::createSchema( QgsDataItem *item, QgsDataItemGuiContext context )
{
  const QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  QString errorMsg;
  try
  {
    const QgsHanaProviderConnection providerConn( item->name() );
    providerConn.createSchema( schemaName );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    errorMsg = ex.what();
  }

  if ( errorMsg.isEmpty() )
  {
    notify( tr( "New Schema" ), tr( "Schema '%1' created successfully." ).arg( schemaName ),
            context, Qgis::MessageLevel::Success );

    item->refresh();
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
  else
  {
    notify( tr( "New Schema" ), tr( "Unable to create schema '%1'\n%2" ).arg( schemaName, errorMsg ),
            context, Qgis::MessageLevel::Warning );
  }
}

void QgsHanaDataItemGuiProvider::deleteSchema( QgsHanaSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  const QString schemaName = schemaItem->name();
  const QString caption = tr( "Delete Schema" );
  QString errorMsg;
  try
  {
    const QgsHanaProviderConnection providerConn( schemaItem->connectionName() );
    const auto tables = providerConn.tables( schemaName );
    if ( tables.empty() )
    {
      if ( QMessageBox::question( nullptr, caption,
                                  tr( "Are you sure you want to delete '%1'?" ).arg( schemaName ),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
        return;
    }
    else
    {
      const int MAXIMUM_LISTED_ITEMS = 10;
      QString tableNames;
      for ( int i = 0; i < tables.size(); ++i )
      {
        const auto &tableProperty = tables.at( i );
        if ( i < MAXIMUM_LISTED_ITEMS )
          tableNames += tableProperty.tableName() + QLatin1Char( '\n' );
        else
        {
          tableNames += QStringLiteral( "\n[%1 additional objects not listed]" ).arg( tables.size() - MAXIMUM_LISTED_ITEMS );
          break;
        }
      }

      if ( QMessageBox::question( nullptr, caption,
                                  tr( "Schema '%1' contains objects:\n\n%2\n\nAre you sure you want to delete the schema and all these objects?" ).arg( schemaName, tableNames ),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
        return;
    }

    providerConn.dropSchema( schemaName, !tables.empty() );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    errorMsg = ex.what();
  }

  if ( errorMsg.isEmpty() )
  {
    notify( caption, tr( "Schema '%1' deleted successfully." ).arg( schemaName ),
            context, Qgis::MessageLevel::Success );
    if ( schemaItem->parent() )
      schemaItem->parent()->refresh();
  }
  else
  {
    notify( caption, tr( "Unable to delete schema '%1'\n%2" ).arg( schemaName, errorMsg ),
            context, Qgis::MessageLevel::Warning );
  }
}

void QgsHanaDataItemGuiProvider::renameSchema( QgsHanaSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  const QString schemaName = schemaItem->name();
  const QString caption = tr( "Rename Schema" );
  QgsNewNameDialog dlg( tr( "schema '%1'" ).arg( schemaName ), schemaName );
  dlg.setWindowTitle( caption );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == schemaName )
    return;

  const QString newSchemaName = dlg.name();
  QString errorMsg;
  try
  {
    const QgsHanaProviderConnection providerConn( schemaItem->connectionName() );
    providerConn.renameSchema( schemaName, newSchemaName );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    errorMsg = ex.what();
  }

  if ( errorMsg.isEmpty() )
  {
    notify( caption, tr( "Schema '%1' renamed successfully to '%2'." ).arg( schemaName, newSchemaName ),
            context, Qgis::MessageLevel::Success );
    if ( schemaItem->parent() )
      schemaItem->parent()->refresh();
  }
  else
  {
    notify( caption, tr( "Unable to rename schema '%1'\n%2" ).arg( schemaName, errorMsg ),
            context, Qgis::MessageLevel::Warning );
  }
}

void QgsHanaDataItemGuiProvider::renameLayer( QgsHanaLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsHanaLayerProperty &layerInfo = layerItem->layerInfo();
  const QString caption = tr( "Rename Table" );
  QgsNewNameDialog dlg( tr( "table '%1.%2'" ).arg( layerInfo.schemaName, layerInfo.tableName ), layerInfo.tableName );
  dlg.setWindowTitle( caption );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == layerInfo.tableName )
    return;

  const QString newLayerName = dlg.name();
  QString errorMsg;
  try
  {
    const QgsHanaProviderConnection providerConn( layerItem->uri(), {} );
    providerConn.renameVectorTable( layerInfo.schemaName, layerInfo.tableName, newLayerName );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    errorMsg = ex.what();
  }

  if ( errorMsg.isEmpty() )
  {
    notify( caption, tr( "'%1' renamed successfully to '%2'." ).arg( layerInfo.tableName, newLayerName ),
            context, Qgis::MessageLevel::Success );
    if ( layerItem->parent() )
      layerItem->parent()->refresh();
  }
  else
  {
    notify( caption, tr( "Unable to rename '%1'\n%2" ).arg( layerInfo.tableName, errorMsg ),
            context, Qgis::MessageLevel::Warning );
  }
}
