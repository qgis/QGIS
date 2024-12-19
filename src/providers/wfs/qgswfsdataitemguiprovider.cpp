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
#include "moc_qgswfsdataitemguiprovider.cpp"

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
  if ( QgsWfsRootItem *rootItem = qobject_cast<QgsWfsRootItem *>( item ) )
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

  if ( QgsWfsConnectionItem *connItem = qobject_cast<QgsWfsConnectionItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), menu );
    connect( actionDuplicate, &QAction::triggered, this, [connItem] { duplicateConnection( connItem ); } );
    menu->addAction( actionDuplicate );

    const QList<QgsWfsConnectionItem *> wfsConnectionItems = QgsDataItem::filteredItems<QgsWfsConnectionItem>( selection );
    QAction *actionDelete = new QAction( wfsConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [wfsConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( wfsConnectionItems, []( const QString &connectionName ) { QgsWfsConnection::deleteConnection( connectionName ); }, context );
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

void QgsWfsDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  const QStringList connections = QgsOwsConnection::sTreeOwsConnections->items( { QStringLiteral( "wfs" ) } );

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  const QStringList detailsParameters { QStringLiteral( "wfs" ), connectionName };
  const QStringList newDetailsParameters { QStringLiteral( "wfs" ), newConnectionName };

  QgsOwsConnection::settingsUrl->setValue( QgsOwsConnection::settingsUrl->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( QgsOwsConnection::settingsIgnoreAxisOrientation->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsInvertAxisOrientation->setValue( QgsOwsConnection::settingsInvertAxisOrientation->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsPreferCoordinatesForWfsT11->setValue( QgsOwsConnection::settingsPreferCoordinatesForWfsT11->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsVersion->setValue( QgsOwsConnection::settingsVersion->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsMaxNumFeatures->setValue( QgsOwsConnection::settingsMaxNumFeatures->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsPagesize->setValue( QgsOwsConnection::settingsPagesize->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsPagingEnabled->setValue( QgsOwsConnection::settingsPagingEnabled->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsUsername->setValue( QgsOwsConnection::settingsUsername->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsPassword->setValue( QgsOwsConnection::settingsPassword->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsAuthCfg->setValue( QgsOwsConnection::settingsAuthCfg->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsHeaders->setValue( QgsOwsConnection::settingsHeaders->value( detailsParameters ), newDetailsParameters );

  item->parent()->refreshConnections();
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
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WFS, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
