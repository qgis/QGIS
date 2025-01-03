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
#include "moc_qgssensorthingsdataitemguiprovider.cpp"
#include "qgssensorthingsdataitems.h"
#include "qgssensorthingsconnection.h"
#include "qgssensorthingsconnectiondialog.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsdataitemguiproviderutils.h"

#include <QMessageBox>
#include <QFileDialog>

///@cond PRIVATE

void QgsSensorThingsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsSensorThingsConnectionItem *connectionItem = qobject_cast<QgsSensorThingsConnectionItem *>( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connectionItem] { editConnection( connectionItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), menu );
    connect( actionDuplicate, &QAction::triggered, this, [connectionItem] { duplicateConnection( connectionItem ); } );
    menu->addAction( actionDuplicate );

    const QList<QgsSensorThingsConnectionItem *> stConnectionItems = QgsDataItem::filteredItems<QgsSensorThingsConnectionItem>( selection );
    QAction *actionDelete = new QAction( stConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [stConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( stConnectionItems, []( const QString &connectionName ) { QgsSensorThingsProviderConnection( QString() ).remove( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );
  }

  if ( QgsSensorThingsRootItem *rootItem = qobject_cast<QgsSensorThingsRootItem *>( item ) )
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

void QgsSensorThingsDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  const QgsSensorThingsProviderConnection::Data connection = QgsSensorThingsProviderConnection::connection( connectionName );
  const QStringList connections = QgsSensorThingsProviderConnection::sTreeSensorThingsConnections->items();

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  QgsSensorThingsProviderConnection::addConnection( newConnectionName, connection );
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
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::SensorThings, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

///@endcond
