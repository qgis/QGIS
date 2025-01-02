/***************************************************************************
  qgstiledscenedataitemguiprovider.cpp
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

#include "qgstiledscenedataitemguiprovider.h"
#include "moc_qgstiledscenedataitemguiprovider.cpp"
#include "qgsquantizedmeshdataprovider.h"
#include "qgstiledscenedataitems.h"
#include "qgstiledsceneconnection.h"
#include "qgstiledsceneconnectiondialog.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsdataitemguiproviderutils.h"

#include <QMessageBox>
#include <QFileDialog>

///@cond PRIVATE

void QgsTiledSceneDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsTiledSceneLayerItem *layerItem = qobject_cast<QgsTiledSceneLayerItem *>( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [layerItem] { editConnection( layerItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), menu );
    connect( actionDuplicate, &QAction::triggered, this, [layerItem] { duplicateConnection( layerItem ); } );
    menu->addAction( actionDuplicate );

    const QList<QgsTiledSceneLayerItem *> sceneConnectionItems = QgsDataItem::filteredItems<QgsTiledSceneLayerItem>( selection );
    QAction *actionDelete = new QAction( sceneConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [sceneConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( sceneConnectionItems, []( const QString &connectionName ) { QgsTiledSceneProviderConnection( QString() ).remove( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );
  }

  if ( QgsTiledSceneRootItem *rootItem = qobject_cast<QgsTiledSceneRootItem *>( item ) )
  {
    QAction *actionNewCesium = new QAction( tr( "New Cesium 3D Tiles Connection…" ), menu );
    connect( actionNewCesium, &QAction::triggered, this, [rootItem] { newConnection( rootItem, "cesiumtiles" ); } );
    menu->addAction( actionNewCesium );

    QAction *actionNewQM = new QAction( tr( "New Quantized Mesh Connection…" ), menu );
    connect( actionNewQM, &QAction::triggered, this, [rootItem] { newConnection( rootItem, "quantizedmesh" ); } );
    menu->addAction( actionNewQM );

    menu->addSeparator();

    QAction *actionSave = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSave, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSave );

    QAction *actionLoadXyzTilesServers = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoadXyzTilesServers, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoadXyzTilesServers );
  }
}

void QgsTiledSceneDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  const QgsTiledSceneProviderConnection::Data connection = QgsTiledSceneProviderConnection::connection( item->name() );
  const QString uri = QgsTiledSceneProviderConnection::encodedUri( connection );

  QgsTiledSceneConnectionDialog dlg;

  dlg.setConnection( item->name(), uri );
  if ( !dlg.exec() )
    return;

  QgsTiledSceneProviderConnection( QString() ).remove( item->name() );

  QgsTiledSceneProviderConnection::Data newConnection = QgsTiledSceneProviderConnection::decodedUri( dlg.connectionUri() );
  newConnection.provider = connection.provider;

  QgsTiledSceneProviderConnection::addConnection( dlg.connectionName(), newConnection );

  item->parent()->refreshConnections();
}

void QgsTiledSceneDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  const QgsTiledSceneProviderConnection::Data connection = QgsTiledSceneProviderConnection::connection( connectionName );
  const QStringList connections = QgsTiledSceneProviderConnection::sTreeConnectionTiledScene->items();

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  QgsTiledSceneProviderConnection::addConnection( newConnectionName, connection );
  item->parent()->refreshConnections();
}

void QgsTiledSceneDataItemGuiProvider::newConnection( QgsDataItem *item, QString provider )
{
  QgsTiledSceneConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  QgsTiledSceneProviderConnection::Data conn = QgsTiledSceneProviderConnection::decodedUri( dlg.connectionUri() );
  conn.provider = provider;

  QgsTiledSceneProviderConnection::addConnection( dlg.connectionName(), conn );

  item->refreshConnections();
}

void QgsTiledSceneDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::TiledScene );
  dlg.exec();
}

void QgsTiledSceneDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::TiledScene, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

///@endcond
