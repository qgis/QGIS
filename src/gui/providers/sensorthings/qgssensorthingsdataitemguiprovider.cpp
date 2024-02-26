/***************************************************************************
  qgssensorthingsdataitemguiprovider.cpp
  --------------------------------------
    begin                : December 2023
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

#include "qgssensorthingsdataitemguiprovider.h"
#include "qgssensorthingsdataitems.h"
#include "qgssensorthingsconnection.h"
#include "qgssensorthingsconnectiondialog.h"
#include "qgsmanageconnectionsdialog.h"

#include <QMessageBox>
#include <QFileDialog>

///@cond PRIVATE

void QgsSensorThingsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsSensorThingsConnectionItem *connectionItem = qobject_cast< QgsSensorThingsConnectionItem * >( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connectionItem] { editConnection( connectionItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection" ), menu );
    connect( actionDelete, &QAction::triggered, this, [connectionItem] { deleteConnection( connectionItem ); } );
    menu->addAction( actionDelete );
  }

  if ( QgsSensorThingsRootItem *rootItem = qobject_cast< QgsSensorThingsRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New SensorThings Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    menu->addSeparator();

    QAction *actionSave = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSave, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSave );

    QAction *actionLoad = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoad, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoad );
  }
}

void QgsSensorThingsDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();

  const QgsSensorThingsProviderConnection::Data connection = QgsSensorThingsProviderConnection::connection( connectionName );
  const QString uri = QgsSensorThingsProviderConnection::encodedUri( connection );
  QgsSensorThingsConnectionDialog dlg;

  dlg.setConnection( connectionName, uri );
  if ( !dlg.exec() )
    return;

  QgsSensorThingsProviderConnection( QString() ).remove( connectionName );

  QgsSensorThingsProviderConnection::Data newConnection = QgsSensorThingsProviderConnection::decodedUri( dlg.connectionUri() );
  QgsSensorThingsProviderConnection::addConnection( dlg.connectionName(), newConnection );

  item->parent()->refreshConnections();
}

void QgsSensorThingsDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, tr( "Remove Connection" ), tr( "Are you sure you want to remove the connection “%1”?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsSensorThingsProviderConnection( QString() ).remove( item->name() );

  item->parent()->refreshConnections();
}

void QgsSensorThingsDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsSensorThingsConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  QgsSensorThingsProviderConnection::Data conn = QgsSensorThingsProviderConnection::decodedUri( dlg.connectionUri() );
  QgsSensorThingsProviderConnection::addConnection( dlg.connectionName(), conn );

  item->refreshConnections();
}

void QgsSensorThingsDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::SensorThings );
  dlg.exec();
}

void QgsSensorThingsDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::SensorThings, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

///@endcond
