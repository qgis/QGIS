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
#include "moc_qgswcsdataitemguiprovider.cpp"

#include "qgsmanageconnectionsdialog.h"
#include "qgswcsdataitems.h"
#include "qgsnewhttpconnection.h"
#include "qgsowsconnection.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"

#include <QFileDialog>
#include <QMessageBox>


void QgsWcsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsWCSRootItem *rootItem = qobject_cast<QgsWCSRootItem *>( item ) )
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

  if ( QgsWCSConnectionItem *connItem = qobject_cast<QgsWCSConnectionItem *>( item ) )
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

    const QList<QgsWCSConnectionItem *> wcsConnectionItems = QgsDataItem::filteredItems<QgsWCSConnectionItem>( selection );
    QAction *actionDelete = new QAction( wcsConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [wcsConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( wcsConnectionItems, []( const QString &connectionName ) { QgsOwsConnection::deleteConnection( QStringLiteral( "WCS" ), connectionName ); }, context );
    } );
    menu->addAction( actionDelete );
  }
}

void QgsWcsDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWcs, QStringLiteral( "WCS" ), QString(), QgsNewHttpConnection::FlagShowHttpSettings );

  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsWcsDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWcs, QStringLiteral( "WCS" ), item->name(), QgsNewHttpConnection::FlagShowHttpSettings );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
  }
}

void QgsWcsDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  const QStringList connections = QgsOwsConnection::sTreeOwsConnections->items( { QStringLiteral( "wcs" ) } );

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  const QStringList detailsParameters { QStringLiteral( "wcs" ), connectionName };
  const QStringList newDetailsParameters { QStringLiteral( "wcs" ), newConnectionName };

  QgsOwsConnection::settingsUrl->setValue( QgsOwsConnection::settingsUrl->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( QgsOwsConnection::settingsIgnoreAxisOrientation->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsInvertAxisOrientation->setValue( QgsOwsConnection::settingsInvertAxisOrientation->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsReportedLayerExtents->setValue( QgsOwsConnection::settingsReportedLayerExtents->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsIgnoreGetMapURI->setValue( QgsOwsConnection::settingsIgnoreGetMapURI->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsSmoothPixmapTransform->setValue( QgsOwsConnection::settingsSmoothPixmapTransform->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsDpiMode->setValue( QgsOwsConnection::settingsDpiMode->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsTilePixelRatio->setValue( QgsOwsConnection::settingsTilePixelRatio->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsHeaders->setValue( QgsOwsConnection::settingsHeaders->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsUsername->setValue( QgsOwsConnection::settingsUsername->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsPassword->setValue( QgsOwsConnection::settingsPassword->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsAuthCfg->setValue( QgsOwsConnection::settingsAuthCfg->value( detailsParameters ), newDetailsParameters );

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
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WCS, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
