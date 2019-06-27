/***************************************************************************
  qgswmsdataitemguiproviders.cpp
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

#include "qgswmsdataitemguiproviders.h"

#include "qgswmsdataitems.h"

#include "qgsnewhttpconnection.h"
#include "qgswmsconnection.h"
#include "qgsxyzconnectiondialog.h"
#include "qgsxyzconnection.h"
#include "qgsmanageconnectionsdialog.h"

#include <QFileDialog>
#include <QMessageBox>


void QgsWmsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsWMSConnectionItem *connItem = qobject_cast< QgsWMSConnectionItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit…" ), this );
    setItemForAction( actionEdit, connItem );
    connect( actionEdit, &QAction::triggered, this, &QgsWmsDataItemGuiProvider::editConnection );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete" ), this );
    setItemForAction( actionDelete, connItem );
    connect( actionDelete, &QAction::triggered, this, &QgsWmsDataItemGuiProvider::deleteConnection );
    menu->addAction( actionDelete );
  }

  if ( QgsWMSRootItem *wmsRootItem = qobject_cast< QgsWMSRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    setItemForAction( actionNew, wmsRootItem );
    connect( actionNew, &QAction::triggered, this, &QgsWmsDataItemGuiProvider::newConnection );
    menu->addAction( actionNew );
  }
}

void QgsWmsDataItemGuiProvider::editConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWms, QStringLiteral( "qgis/connections-wms/" ), item->name() );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
  }
}

void QgsWmsDataItemGuiProvider::deleteConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  if ( QMessageBox::question( nullptr, tr( "Delete Connection" ), tr( "Are you sure you want to delete the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsWMSConnection::deleteConnection( item->name() );
  // the parent should be updated
  item->parent()->refreshConnections();
}

void QgsWmsDataItemGuiProvider::newConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QgsNewHttpConnection nc( nullptr );

  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}


// -----------


void QgsXyzDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsXyzLayerItem *layerItem = qobject_cast< QgsXyzLayerItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit…" ), this );
    setItemForAction( actionEdit, layerItem );
    connect( actionEdit, &QAction::triggered, this, &QgsXyzDataItemGuiProvider::editConnection );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete" ), this );
    setItemForAction( actionDelete, layerItem );
    connect( actionDelete, &QAction::triggered, this, &QgsXyzDataItemGuiProvider::deleteConnection );
    menu->addAction( actionDelete );
  }

  if ( QgsXyzTileRootItem *rootItem = qobject_cast< QgsXyzTileRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    setItemForAction( actionNew, rootItem );
    connect( actionNew, &QAction::triggered, this, &QgsXyzDataItemGuiProvider::newConnection );
    menu->addAction( actionNew );

    QAction *saveXyzTilesServers = new QAction( tr( "Save Connections…" ), this );
    setItemForAction( saveXyzTilesServers, rootItem );
    connect( saveXyzTilesServers, &QAction::triggered, this, &QgsXyzDataItemGuiProvider::saveXyzTilesServers );
    menu->addAction( saveXyzTilesServers );

    QAction *loadXyzTilesServers = new QAction( tr( "Load Connections…" ), this );
    setItemForAction( loadXyzTilesServers, rootItem );
    connect( loadXyzTilesServers, &QAction::triggered, this, &QgsXyzDataItemGuiProvider::loadXyzTilesServers );
    menu->addAction( loadXyzTilesServers );
  }
}

void QgsXyzDataItemGuiProvider::editConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QgsXyzConnectionDialog dlg;
  dlg.setConnection( QgsXyzConnectionUtils::connection( item->name() ) );
  if ( !dlg.exec() )
    return;

  QgsXyzConnectionUtils::deleteConnection( item->name() );
  QgsXyzConnectionUtils::addConnection( dlg.connection() );

  item->parent()->refreshConnections();
}

void QgsXyzDataItemGuiProvider::deleteConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  if ( QMessageBox::question( nullptr, tr( "Delete Connection" ), tr( "Are you sure you want to delete the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsXyzConnectionUtils::deleteConnection( item->name() );

  item->parent()->refreshConnections();
}

void QgsXyzDataItemGuiProvider::newConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QgsXyzConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  QgsXyzConnectionUtils::addConnection( dlg.connection() );
  item->refreshConnections();
}

void QgsXyzDataItemGuiProvider::saveXyzTilesServers()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::XyzTiles );
  dlg.exec();
}

void QgsXyzDataItemGuiProvider::loadXyzTilesServers()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::XyzTiles, fileName );
  dlg.exec();
  item->refreshConnections();
}
