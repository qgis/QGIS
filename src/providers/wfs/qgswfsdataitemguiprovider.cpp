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

#include "qgsmanageconnectionsdialog.h"
#include "qgswfsnewconnection.h"
#include "qgswfsconnection.h"
#include "qgswfsconstants.h"
#include "qgswfsdataitems.h"
#include "qgsdataitemguiproviderutils.h"

#include <QFileDialog>
#include <QMessageBox>


void QgsWfsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsWfsRootItem *rootItem = qobject_cast< QgsWfsRootItem * >( item ) )
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

  if ( QgsWfsConnectionItem *connItem = qobject_cast< QgsWfsConnectionItem * >( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    const QList< QgsWfsConnectionItem * > wfsConnectionItems = QgsDataItem::filteredItems<QgsWfsConnectionItem>( selection );
    QAction *actionDelete = new QAction( wfsConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [wfsConnectionItems, context]
    {
      QgsDataItemGuiProviderUtils::deleteConnections( wfsConnectionItems, []( const QString & connectionName )
      {
        QgsWfsConnection::deleteConnection( connectionName );
      }, context );
    } );
    menu->addAction( actionDelete );
  }
}

void QgsWfsDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsWFSNewConnection nc( nullptr );
  nc.setWindowTitle( tr( "Create a New WFS Connection" ) );

  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsWfsDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsWFSNewConnection nc( nullptr, item->name() );
  nc.setWindowTitle( tr( "Modify WFS Connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
  }
}

void QgsWfsDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsWfsDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WFS );
  dlg.exec();
}

void QgsWfsDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WFS, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
