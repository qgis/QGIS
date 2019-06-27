/***************************************************************************
  qgswfsdataitemguiprovider.cpp
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

#include "qgswfsdataitemguiprovider.h"

#include "qgsnewhttpconnection.h"
#include "qgswfsconnection.h"
#include "qgswfsconstants.h"
#include "qgswfsdataitems.h"

#include <QMessageBox>


void QgsWfsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsWfsRootItem *rootItem = qobject_cast< QgsWfsRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    setItemForAction( actionNew, rootItem );
    connect( actionNew, &QAction::triggered, this, &QgsWfsDataItemGuiProvider::newConnection );
    menu->addAction( actionNew );
  }

  if ( QgsWfsConnectionItem *connItem = qobject_cast< QgsWfsConnectionItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit…" ), this );
    setItemForAction( actionEdit, connItem );
    connect( actionEdit, &QAction::triggered, this, &QgsWfsDataItemGuiProvider::editConnection );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete" ), this );
    setItemForAction( actionDelete, connItem );
    connect( actionDelete, &QAction::triggered, this, &QgsWfsDataItemGuiProvider::deleteConnection );
    menu->addAction( actionDelete );
  }
}

void QgsWfsDataItemGuiProvider::newConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWfs, QgsWFSConstants::CONNECTIONS_WFS );
  nc.setWindowTitle( tr( "Create a New WFS Connection" ) );

  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsWfsDataItemGuiProvider::editConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWfs, QgsWFSConstants::CONNECTIONS_WFS, item->name() );
  nc.setWindowTitle( tr( "Modify WFS Connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
  }
}

void QgsWfsDataItemGuiProvider::deleteConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  if ( QMessageBox::question( nullptr, tr( "Delete Connection" ), tr( "Are you sure you want to delete the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsWfsConnection::deleteConnection( item->name() );
  // the parent should be updated
  item->parent()->refreshConnections();
}
