/***************************************************************************
  qgsamsdataitemguiprovider.cpp
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

#include "qgsamsdataitemguiprovider.h"
#include "qgsamsdataitems.h"
#include "qgsnewhttpconnection.h"
#include "qgsowsconnection.h"

#include <QDesktopServices>
#include <QMessageBox>


void QgsAmsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsAmsRootItem *rootItem = qobject_cast< QgsAmsRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );
  }
  if ( QgsAmsConnectionItem *connectionItem = qobject_cast< QgsAmsConnectionItem * >( item ) )
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
  else if ( QgsAmsFolderItem *folderItem = qobject_cast< QgsAmsFolderItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( folderItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsAmsServiceItem *serviceItem = qobject_cast< QgsAmsServiceItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( serviceItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsAmsLayerItem *layerItem = qobject_cast< QgsAmsLayerItem * >( item ) )
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

void QgsAmsDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgismapserver/" ), QString(), QgsNewHttpConnection::FlagShowHttpSettings );
  nc.setWindowTitle( tr( "Create a New ArcGIS Map Server Connection" ) );

  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsAmsDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgismapserver/" ), item->name(), QgsNewHttpConnection::FlagShowHttpSettings );
  nc.setWindowTitle( tr( "Modify ArcGIS Map Server Connection" ) );

  if ( nc.exec() )
  {
    item->refresh();
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsAmsDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsOwsConnection::deleteConnection( QStringLiteral( "arcgismapserver" ), item->name() );
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsAmsDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}
