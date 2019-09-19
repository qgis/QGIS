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
#include "qgswmssourceselect.h"

#include <QFileDialog>
#include <QMessageBox>

static QWidget *_paramWidget( QgsDataItem *root )
{
  if ( qobject_cast<QgsWMSRootItem *>( root ) != nullptr )
  {
    return new QgsWMSSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
  }
  else
  {
    return nullptr;
  }
}

void QgsWmsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsWMSConnectionItem *connItem = qobject_cast< QgsWMSConnectionItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit…" ), this );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete" ), this );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); } );
    menu->addAction( actionDelete );
  }

  if ( QgsWMSRootItem *wmsRootItem = qobject_cast< QgsWMSRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    connect( actionNew, &QAction::triggered, this, [wmsRootItem] { newConnection( wmsRootItem ); } );
    menu->addAction( actionNew );
  }
}

QWidget *QgsWmsDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  return _paramWidget( root );
}

void QgsWmsDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWms, QStringLiteral( "qgis/connections-wms/" ), item->name() );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
  }
}

void QgsWmsDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Delete Connection" ), tr( "Are you sure you want to delete the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsWMSConnection::deleteConnection( item->name() );
  // the parent should be updated
  item->parent()->refreshConnections();
}

void QgsWmsDataItemGuiProvider::newConnection( QgsDataItem *item )
{
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
    connect( actionEdit, &QAction::triggered, this, [layerItem] { editConnection( layerItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete" ), this );
    connect( actionDelete, &QAction::triggered, this, [layerItem] { deleteConnection( layerItem ); } );
    menu->addAction( actionDelete );
  }

  if ( QgsXyzTileRootItem *rootItem = qobject_cast< QgsXyzTileRootItem * >( item ) )
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

QWidget *QgsXyzDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  return _paramWidget( root );
}

void QgsXyzDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsXyzConnectionDialog dlg;
  dlg.setConnection( QgsXyzConnectionUtils::connection( item->name() ) );
  if ( !dlg.exec() )
    return;

  QgsXyzConnectionUtils::deleteConnection( item->name() );
  QgsXyzConnectionUtils::addConnection( dlg.connection() );

  item->parent()->refreshConnections();
}

void QgsXyzDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Delete Connection" ), tr( "Are you sure you want to delete the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsXyzConnectionUtils::deleteConnection( item->name() );

  item->parent()->refreshConnections();
}

void QgsXyzDataItemGuiProvider::newConnection( QgsDataItem *item )
{
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

void QgsXyzDataItemGuiProvider::loadXyzTilesServers( QgsDataItem *item )
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
