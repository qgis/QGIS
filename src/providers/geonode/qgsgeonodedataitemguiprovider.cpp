/***************************************************************************
  qgsgeonodedataitemguiprovider.cpp
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

#include "qgsgeonodedataitemguiprovider.h"
#include "qgsgeonodedataitems.h"
#include "qgsgeonodenewconnection.h"
#include "qgsmanageconnectionsdialog.h"

#include <QFileDialog>
#include <QMessageBox>

void QgsGeoNodeDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsGeoNodeRootItem *rootItem = qobject_cast< QgsGeoNodeRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionSaveServers = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSaveServers, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSaveServers );

    QAction *actionLoadServers = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoadServers, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoadServers );
  }
  else if ( QgsGeoNodeConnectionItem *connItem = qobject_cast< QgsGeoNodeConnectionItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), menu );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); } );
    menu->addAction( actionDelete );
  }
}

void QgsGeoNodeDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsGeoNodeNewConnection nc( nullptr );

  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsGeoNodeDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsGeoNodeNewConnection nc( nullptr, item->name() );
  nc.setWindowTitle( tr( "Modify GeoNode connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refresh();
  }
}

void QgsGeoNodeDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Remove Connection" ), tr( "Are you sure you want to remove the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsGeoNodeConnectionUtils::deleteConnection( item->name() );
  item->parent()->refresh();
}

void QgsGeoNodeDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::GeoNode );
  dlg.exec();
}

void QgsGeoNodeDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::GeoNode, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
