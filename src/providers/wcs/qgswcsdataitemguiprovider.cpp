/***************************************************************************
  qgswcsdataitemguiprovider.cpp
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

#include "qgswcsdataitemguiprovider.h"

#include "qgswcsdataitems.h"
#include "qgsnewhttpconnection.h"
#include "qgsowsconnection.h"

#include <QMessageBox>


void QgsWcsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsWCSRootItem *rootItem = qobject_cast< QgsWCSRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    setItemForAction( actionNew, rootItem );
    connect( actionNew, &QAction::triggered, this, &QgsWcsDataItemGuiProvider::newConnection );
    menu->addAction( actionNew );
  }

  if ( QgsWCSConnectionItem *connItem = qobject_cast< QgsWCSConnectionItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit…" ), this );
    setItemForAction( actionEdit, connItem );
    connect( actionEdit, &QAction::triggered, this, &QgsWcsDataItemGuiProvider::editConnection );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Delete" ), this );
    setItemForAction( actionDelete, connItem );
    connect( actionDelete, &QAction::triggered, this, &QgsWcsDataItemGuiProvider::deleteConnection );
    menu->addAction( actionDelete );
  }
}

void QgsWcsDataItemGuiProvider::newConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWcs, QStringLiteral( "qgis/connections-wcs/" ) );

  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsWcsDataItemGuiProvider::editConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWcs, QStringLiteral( "qgis/connections-wcs/" ), item->name() );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
  }
}

void QgsWcsDataItemGuiProvider::deleteConnection()
{
  QPointer< QgsDataItem > item = itemFromAction( qobject_cast<QAction *>( sender() ) );
  if ( !item )
    return;

  if ( QMessageBox::question( nullptr, tr( "Delete Connection" ), tr( "Are you sure you want to delete the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsOwsConnection::deleteConnection( QStringLiteral( "WCS" ), item->name() );
  // the parent should be updated
  item->parent()->refreshConnections();
}
