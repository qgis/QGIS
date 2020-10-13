/***************************************************************************
  qgsafsdataitemguiprovider.cpp
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

#include "qgsafsdataitemguiprovider.h"
#include "qgsafsdataitems.h"
#include "qgsafssourceselect.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsnewhttpconnection.h"
#include "qgsowsconnection.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>


void QgsAfsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsAfsRootItem *rootItem = qobject_cast< QgsAfsRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionSaveServers = new QAction( tr( "Save Connections…" ), this );
    connect( actionSaveServers, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSaveServers );

    QAction *actionLoadServers = new QAction( tr( "Load Connections…" ), this );
    connect( actionLoadServers, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoadServers );
  }
  else if ( QgsAfsConnectionItem *connectionItem = qobject_cast< QgsAfsConnectionItem * >( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [connectionItem] { refreshConnection( connectionItem ); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connectionItem] { editConnection( connectionItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [connectionItem] { deleteConnection( connectionItem ); } );
    menu->addAction( actionDelete );

    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( connectionItem->url() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsAfsFolderItem *folderItem = qobject_cast< QgsAfsFolderItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( folderItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsAfsServiceItem *serviceItem = qobject_cast< QgsAfsServiceItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( serviceItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsAfsParentLayerItem *layerItem = qobject_cast< QgsAfsParentLayerItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( layerItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsAfsLayerItem *layerItem = qobject_cast< QgsAfsLayerItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( layerItem->path() ) );
    } );
    menu->addAction( viewInfo );
    menu->addSeparator();
  }
}

void QgsAfsDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ), QString(), QgsNewHttpConnection::FlagShowHttpSettings );
  nc.setWindowTitle( tr( "Create a New ArcGIS Feature Service Connection" ) );

  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsAfsDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ), item->name(), QgsNewHttpConnection::FlagShowHttpSettings );
  nc.setWindowTitle( tr( "Modify ArcGIS Feature Service Connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->refresh();
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsAfsDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsOwsConnection::deleteConnection( QStringLiteral( "arcgisfeatureserver" ), item->name() );

  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsAfsDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsAfsDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::ArcgisFeatureServer );
  dlg.exec();
}

void QgsAfsDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::ArcgisFeatureServer, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
