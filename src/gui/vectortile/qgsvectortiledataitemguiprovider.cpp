/***************************************************************************
  qgsvectortiledataitemguiprovider.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortiledataitemguiprovider.h"

#include "qgsvectortiledataitems.h"
#include "qgsvectortileconnectiondialog.h"
#include "qgsvectortileconnection.h"
#include "qgsmanageconnectionsdialog.h"

#include <QFileDialog>
#include <QMessageBox>


void QgsVectorTileDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsVectorTileLayerItem *layerItem = qobject_cast< QgsVectorTileLayerItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit…" ), this );
    connect( actionEdit, &QAction::triggered, this, [layerItem] { editConnection( layerItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete" ), this );
    connect( actionDelete, &QAction::triggered, this, [layerItem] { deleteConnection( layerItem ); } );
    menu->addAction( actionDelete );
  }

  if ( QgsVectorTileRootItem *rootItem = qobject_cast< QgsVectorTileRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionSaveXyzTilesServers = new QAction( tr( "Save Connections…" ), this );
    connect( actionSaveXyzTilesServers, &QAction::triggered, this, [] { saveXyzTilesServers(); } );
    menu->addAction( actionSaveXyzTilesServers );

    QAction *actionLoadXyzTilesServers = new QAction( tr( "Load Connections…" ), this );
    connect( actionLoadXyzTilesServers, &QAction::triggered, this, [rootItem] { loadXyzTilesServers( rootItem ); } );
    menu->addAction( actionLoadXyzTilesServers );
  }
}

void QgsVectorTileDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsVectorTileConnectionDialog dlg;
  dlg.setConnection( QgsVectorTileConnectionUtils::connection( item->name() ) );
  if ( !dlg.exec() )
    return;

  QgsVectorTileConnectionUtils::deleteConnection( item->name() );
  QgsVectorTileConnectionUtils::addConnection( dlg.connection() );

  item->parent()->refreshConnections();
}

void QgsVectorTileDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Delete Connection" ), tr( "Are you sure you want to delete the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsVectorTileConnectionUtils::deleteConnection( item->name() );

  item->parent()->refreshConnections();
}

void QgsVectorTileDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsVectorTileConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  QgsVectorTileConnectionUtils::addConnection( dlg.connection() );
  item->refreshConnections();
}

void QgsVectorTileDataItemGuiProvider::saveXyzTilesServers()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::XyzTiles );
  dlg.exec();
}

void QgsVectorTileDataItemGuiProvider::loadXyzTilesServers( QgsDataItem *item )
{
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
