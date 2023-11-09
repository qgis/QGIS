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
#include "qgsarcgisvectortileconnectiondialog.h"
#include "qgsvectortileconnection.h"
#include "qgsmanageconnectionsdialog.h"

#include <QFileDialog>
#include <QMessageBox>

///@cond PRIVATE

void QgsVectorTileDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsVectorTileLayerItem *layerItem = qobject_cast< QgsVectorTileLayerItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [layerItem] { editConnection( layerItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), menu );
    connect( actionDelete, &QAction::triggered, this, [layerItem] { deleteConnection( layerItem ); } );
    menu->addAction( actionDelete );
  }

  if ( QgsVectorTileRootItem *rootItem = qobject_cast< QgsVectorTileRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Generic Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionNewArcGISConnection = new QAction( tr( "New ArcGIS Vector Tile Service Connection…" ), menu );
    connect( actionNewArcGISConnection, &QAction::triggered, this, [rootItem] { newArcGISConnection( rootItem ); } );
    menu->addAction( actionNewArcGISConnection );

    menu->addSeparator();

    QAction *actionSaveXyzTilesServers = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSaveXyzTilesServers, &QAction::triggered, this, [] { saveXyzTilesServers(); } );
    menu->addAction( actionSaveXyzTilesServers );

    QAction *actionLoadXyzTilesServers = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoadXyzTilesServers, &QAction::triggered, this, [rootItem] { loadXyzTilesServers( rootItem ); } );
    menu->addAction( actionLoadXyzTilesServers );
  }
}

void QgsVectorTileDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  const QgsVectorTileProviderConnection::Data connection = QgsVectorTileProviderConnection::connection( item->name() );
  const QString uri = QgsVectorTileProviderConnection::encodedUri( connection );

  switch ( connection.serviceType )
  {
    case QgsVectorTileProviderConnection::Generic:
    {
      QgsVectorTileConnectionDialog dlg;

      dlg.setConnection( item->name(), uri );
      if ( !dlg.exec() )
        return;

      QgsVectorTileProviderConnection::deleteConnection( item->name() );
      const QgsVectorTileProviderConnection::Data conn = QgsVectorTileProviderConnection::decodedUri( dlg.connectionUri() );
      QgsVectorTileProviderConnection::addConnection( dlg.connectionName(), conn );
      break;
    }

    case QgsVectorTileProviderConnection::ArcgisVectorTileService:
    {
      QgsArcgisVectorTileConnectionDialog dlg;

      dlg.setConnection( item->name(), uri );
      if ( !dlg.exec() )
        return;

      QgsVectorTileProviderConnection::deleteConnection( item->name() );
      const QgsVectorTileProviderConnection::Data conn = QgsVectorTileProviderConnection::decodedUri( dlg.connectionUri() );
      QgsVectorTileProviderConnection::addConnection( dlg.connectionName(), conn );
      break;
    }
  }

  item->parent()->refreshConnections();
}

void QgsVectorTileDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Remove Connection" ), tr( "Are you sure you want to remove the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsVectorTileProviderConnection::deleteConnection( item->name() );

  item->parent()->refreshConnections();
}

void QgsVectorTileDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsVectorTileConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  const QgsVectorTileProviderConnection::Data conn = QgsVectorTileProviderConnection::decodedUri( dlg.connectionUri() );
  QgsVectorTileProviderConnection::addConnection( dlg.connectionName(), conn );

  item->refreshConnections();
}

void QgsVectorTileDataItemGuiProvider::newArcGISConnection( QgsDataItem *item )
{
  QgsArcgisVectorTileConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  const QgsVectorTileProviderConnection::Data conn = QgsVectorTileProviderConnection::decodedUri( dlg.connectionUri() );
  QgsVectorTileProviderConnection::addConnection( dlg.connectionName(), conn );

  item->refreshConnections();
}

void QgsVectorTileDataItemGuiProvider::saveXyzTilesServers()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::VectorTile );
  dlg.exec();
}

void QgsVectorTileDataItemGuiProvider::loadXyzTilesServers( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::VectorTile, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

///@endcond
