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

#include "qgsmanageconnectionsdialog.h"
#include "qgswcsdataitems.h"
#include "qgsnewhttpconnection.h"
#include "qgsowsconnection.h"

#include <QFileDialog>
#include <QMessageBox>


void QgsWcsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsWCSRootItem *rootItem = qobject_cast< QgsWCSRootItem * >( item ) )
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

  if ( QgsWCSConnectionItem *connItem = qobject_cast< QgsWCSConnectionItem * >( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), menu );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); } );
    menu->addAction( actionDelete );
  }
}

void QgsWcsDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWcs, QStringLiteral( "WCS" ) );

  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsWcsDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWcs, QStringLiteral( "WCS" ), item->name() );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
  }
}

void QgsWcsDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Remove Connection" ), tr( "Are you sure you want to remove the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsOwsConnection::deleteConnection( QStringLiteral( "WCS" ), item->name() );
  // the parent should be updated
  item->parent()->refreshConnections();
}

void QgsWcsDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsWcsDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WCS );
  dlg.exec();
}

void QgsWcsDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WCS, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
