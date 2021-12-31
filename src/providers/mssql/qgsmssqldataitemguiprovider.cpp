/***************************************************************************
  qgsmssqldataitemguiprovider.cpp
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

#include "qgsmanageconnectionsdialog.h"
#include "qgsmssqlconnection.h"
#include "qgsmssqldataitemguiprovider.h"
#include "qgsmssqldataitems.h"
#include "qgsmssqlnewconnection.h"
#include "qgsmssqlsourceselect.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>


void QgsMssqlDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsMssqlRootItem *rootItem = qobject_cast< QgsMssqlRootItem * >( item ) )
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
  else if ( QgsMssqlConnectionItem *connItem = qobject_cast< QgsMssqlConnectionItem * >( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [connItem]
    {
      connItem->refresh();
      if ( connItem->parent() )
        connItem->parent()->refreshConnections();
    } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), menu );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); } );
    menu->addAction( actionDelete );

    menu->addSeparator();

    QAction *actionShowNoGeom = new QAction( tr( "Show Non-spatial Tables" ), menu );
    actionShowNoGeom->setCheckable( true );
    actionShowNoGeom->setChecked( connItem->allowGeometrylessTables() );
    connect( actionShowNoGeom, &QAction::toggled, connItem, &QgsMssqlConnectionItem::setAllowGeometrylessTables );
    menu->addAction( actionShowNoGeom );

    QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), menu );
    connect( actionCreateSchema, &QAction::triggered, this, [connItem] { createSchema( connItem ); } );
    menu->addAction( actionCreateSchema );
  }
  else if ( QgsMssqlSchemaItem *schemaItem = qobject_cast< QgsMssqlSchemaItem * >( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [schemaItem]
    {
      if ( schemaItem->parent() )
        schemaItem->parent()->refresh();
    } );
    menu->addAction( actionRefresh );
  }
  else if ( QgsMssqlLayerItem *layerItem = qobject_cast< QgsMssqlLayerItem * >( item ) )
  {
    QMenu *maintainMenu = new QMenu( tr( "Table Operations" ), menu );

    // truncate
    QAction *actionTruncateLayer = new QAction( tr( "Truncate Table" ), menu );
    connect( actionTruncateLayer, &QAction::triggered, this, [layerItem] { truncateTable( layerItem ); } );
    maintainMenu->addAction( actionTruncateLayer );

    menu->addMenu( maintainMenu );
  }
}

bool QgsMssqlDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsMssqlLayerItem *layerItem = qobject_cast< QgsMssqlLayerItem * >( item ) )
  {
    QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( layerItem->parent() ? layerItem->parent()->parent() : nullptr );
    const QgsMssqlLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );

    if ( QMessageBox::question( nullptr, QObject::tr( "Delete %1" ).arg( typeName ),
                                QObject::tr( "Are you sure you want to delete [%1].[%2]?" ).arg( layerInfo.schemaName, layerInfo.tableName ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return true;

    QString errCause;
    bool res;
    if ( layerInfo.isView )
      res = QgsMssqlConnection::dropView( layerItem->uri(), &errCause );
    else
      res = QgsMssqlConnection::dropTable( layerItem->uri(), &errCause );

    if ( !res )
    {
      notify( tr( "Delete %1" ).arg( typeName ), errCause, context, Qgis::MessageLevel::Warning );
    }
    else
    {
      notify( tr( "Delete %1" ).arg( typeName ), tr( "%1 deleted successfully." ).arg( typeName ), context, Qgis::MessageLevel::Success );
      if ( connItem )
        connItem->refresh();
    }
    return true;
  }
  return false;
}

bool QgsMssqlDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast< QgsMssqlConnectionItem * >( item ) )
    return true;
  if ( qobject_cast< QgsMssqlSchemaItem * >( item ) )
    return true;

  return false;
}

bool QgsMssqlDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data, Qt::DropAction )
{
  if ( QgsMssqlConnectionItem *connItem = qobject_cast< QgsMssqlConnectionItem * >( item ) )
  {
    return connItem->handleDrop( data, QString() );
  }
  else if ( QgsMssqlSchemaItem *schemaItem = qobject_cast< QgsMssqlSchemaItem * >( item ) )
  {
    QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return connItem->handleDrop( data, schemaItem->name() );
  }
  return false;
}

void QgsMssqlDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsMssqlNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsMssqlDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsMssqlNewConnection nc( nullptr, item->name() );
  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
    item->refresh();
  }
}

void QgsMssqlDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connection" ),
                              QObject::tr( "Are you sure you want to remove the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsMssqlSourceSelect::deleteConnection( item->name() );
  // the parent should be updated
  item->parent()->refreshConnections();
}

void QgsMssqlDataItemGuiProvider::createSchema( QgsMssqlConnectionItem *connItem )
{
  const QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  const QString uri = connItem->connInfo();
  QString error;
  if ( !QgsMssqlConnection::createSchema( uri, schemaName, &error ) )
  {
    QMessageBox::warning( nullptr, tr( "Create Schema" ), tr( "Unable to create schema %1\n%2" ).arg( schemaName,
                          error ) );
    return;
  }

  connItem->refresh();
  // the parent should be updated
  if ( connItem->parent() )
    connItem->parent()->refreshConnections();
}


void QgsMssqlDataItemGuiProvider::truncateTable( QgsMssqlLayerItem *layerItem )
{
  const QgsMssqlLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ),
                              QObject::tr( "Are you sure you want to truncate [%1].[%2]?\n\nThis will delete all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QString errCause;
  const bool res = QgsMssqlConnection::truncateTable( layerItem->uri(), &errCause );
  if ( !res )
  {
    QMessageBox::warning( nullptr, tr( "Truncate Table" ), errCause );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Truncate Table" ), tr( "Table truncated successfully." ) );
  }
}

void QgsMssqlDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::MSSQL );
  dlg.exec();
}

void QgsMssqlDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::MSSQL, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
