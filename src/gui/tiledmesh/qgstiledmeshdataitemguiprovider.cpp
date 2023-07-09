/***************************************************************************
  qgstiledmeshdataitemguiprovider.cpp
  --------------------------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledmeshdataitemguiprovider.h"
#include "qgstiledmeshdataitems.h"
#include "qgstiledmeshconnection.h"
#include "qgstiledmeshconnectiondialog.h"
#include "qgsmanageconnectionsdialog.h"

#include <QMessageBox>
#include <QFileDialog>

///@cond PRIVATE

void QgsTiledMeshDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsTiledMeshLayerItem *layerItem = qobject_cast< QgsTiledMeshLayerItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [layerItem] { editConnection( layerItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), menu );
    connect( actionDelete, &QAction::triggered, this, [layerItem] { deleteConnection( layerItem ); } );
    menu->addAction( actionDelete );
  }

  if ( QgsTiledMeshRootItem *rootItem = qobject_cast< QgsTiledMeshRootItem * >( item ) )
  {
    QAction *actionNewCesium = new QAction( tr( "New Cesium 3D Tiles Connection…" ), menu );
    connect( actionNewCesium, &QAction::triggered, this, [rootItem] { newCesium3dTilesConnection( rootItem ); } );
    menu->addAction( actionNewCesium );

    menu->addSeparator();

    QAction *actionSave = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSave, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSave );

    QAction *actionLoadXyzTilesServers = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoadXyzTilesServers, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoadXyzTilesServers );
  }
}

void QgsTiledMeshDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  const QgsTiledMeshProviderConnection::Data connection = QgsTiledMeshProviderConnection::connection( item->name() );
  const QString uri = QgsTiledMeshProviderConnection::encodedUri( connection );

  QgsTiledMeshConnectionDialog dlg;

  dlg.setConnection( item->name(), uri );
  if ( !dlg.exec() )
    return;

  QgsTiledMeshProviderConnection( QString() ).remove( item->name() );

  QgsTiledMeshProviderConnection::Data newConnection = QgsTiledMeshProviderConnection::decodedUri( dlg.connectionUri() );
  newConnection.provider = connection.provider;

  QgsTiledMeshProviderConnection::addConnection( dlg.connectionName(), newConnection );

  item->parent()->refreshConnections();
}

void QgsTiledMeshDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Remove Connection" ), tr( "Are you sure you want to remove the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsTiledMeshProviderConnection( QString() ).remove( item->name() );

  item->parent()->refreshConnections();
}

void QgsTiledMeshDataItemGuiProvider::newCesium3dTilesConnection( QgsDataItem *item )
{
  QgsTiledMeshConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  QgsTiledMeshProviderConnection::Data conn = QgsTiledMeshProviderConnection::decodedUri( dlg.connectionUri() );
  conn.provider = QStringLiteral( "cesiumtiles" );

  QgsTiledMeshProviderConnection::addConnection( dlg.connectionName(), conn );

  item->refreshConnections();
}

void QgsTiledMeshDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::TiledMesh );
  dlg.exec();
}

void QgsTiledMeshDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::TiledMesh, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

///@endcond
