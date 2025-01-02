/***************************************************************************
  qgswmsdataitemguiproviders.cpp
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

#include "qgswmsdataitemguiproviders.h"
#include "moc_qgswmsdataitemguiproviders.cpp"

#include "qgswmsdataitems.h"

#include "qgsnewhttpconnection.h"
#include "qgswmsconnection.h"
#include "qgsxyzconnectiondialog.h"
#include "qgsxyzconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgswmssourceselect.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgssettingsentryenumflag.h"

#include <QFileDialog>
#include <QMessageBox>

static QWidget *_paramWidget( QgsDataItem *root )
{
  if ( qobject_cast<QgsWMSRootItem *>( root ) )
  {
    return new QgsWMSSourceSelect( nullptr, Qt::WindowFlags(), QgsProviderRegistry::WidgetMode::Manager );
  }
  else
  {
    return nullptr;
  }
}

void QgsWmsDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsWMSConnectionItem *connItem = qobject_cast<QgsWMSConnectionItem *>( item ) )
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

    const QList<QgsWMSConnectionItem *> wmsConnectionItems = QgsDataItem::filteredItems<QgsWMSConnectionItem>( selection );
    QAction *actionDelete = new QAction( wmsConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [wmsConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( wmsConnectionItems, []( const QString &connectionName ) { QgsWMSConnection::deleteConnection( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );
  }

  if ( QgsWMSRootItem *rootItem = qobject_cast<QgsWMSRootItem *>( item ) )
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
}

QWidget *QgsWmsDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  return _paramWidget( root );
}

void QgsWmsDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWms, QStringLiteral( "WMS" ), item->name(), QgsNewHttpConnection::FlagShowHttpSettings );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
  }
}

void QgsWmsDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  const QStringList connections = QgsOwsConnection::sTreeOwsConnections->items( { QStringLiteral( "wms" ) } );

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  const QStringList detailsParameters { QStringLiteral( "wms" ), connectionName };
  const QStringList newDetailsParameters { QStringLiteral( "wms" ), newConnectionName };

  QgsOwsConnection::settingsUrl->setValue( QgsOwsConnection::settingsUrl->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( QgsOwsConnection::settingsIgnoreAxisOrientation->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsInvertAxisOrientation->setValue( QgsOwsConnection::settingsInvertAxisOrientation->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsReportedLayerExtents->setValue( QgsOwsConnection::settingsReportedLayerExtents->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsIgnoreGetMapURI->setValue( QgsOwsConnection::settingsIgnoreGetMapURI->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsSmoothPixmapTransform->setValue( QgsOwsConnection::settingsSmoothPixmapTransform->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsDpiMode->setValue( QgsOwsConnection::settingsDpiMode->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsTilePixelRatio->setValue( QgsOwsConnection::settingsTilePixelRatio->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->setValue( QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsFeatureCount->setValue( QgsOwsConnection::settingsFeatureCount->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsHeaders->setValue( QgsOwsConnection::settingsHeaders->value( detailsParameters ), newDetailsParameters );

  QgsOwsConnection::settingsUsername->setValue( QgsOwsConnection::settingsUsername->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsPassword->setValue( QgsOwsConnection::settingsPassword->value( detailsParameters ), newDetailsParameters );
  QgsOwsConnection::settingsAuthCfg->setValue( QgsOwsConnection::settingsAuthCfg->value( detailsParameters ), newDetailsParameters );

  item->parent()->refreshConnections();
}

void QgsWmsDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWms, QStringLiteral( "WMS" ), QString(), QgsNewHttpConnection::FlagShowHttpSettings );

  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsWmsDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  // Updating the item and its children only
  item->refresh();
}

void QgsWmsDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WMS );
  dlg.exec();
}

void QgsWmsDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WMS, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

// -----------


void QgsXyzDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsXyzLayerItem *layerItem = qobject_cast<QgsXyzLayerItem *>( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), this );
    connect( actionEdit, &QAction::triggered, this, [layerItem] { editConnection( layerItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), this );
    connect( actionDuplicate, &QAction::triggered, this, [layerItem] { duplicateConnection( layerItem ); } );
    menu->addAction( actionDuplicate );

    const QList<QgsXyzLayerItem *> xyzConnectionItems = QgsDataItem::filteredItems<QgsXyzLayerItem>( selection );
    QAction *actionDelete = new QAction( xyzConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [xyzConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( xyzConnectionItems, []( const QString &connectionName ) { QgsXyzConnectionUtils::deleteConnection( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );
  }

  if ( QgsXyzTileRootItem *rootItem = qobject_cast<QgsXyzTileRootItem *>( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionSaveXyzTilesServers = new QAction( tr( "Save Connections…" ), this );
    connect( actionSaveXyzTilesServers, &QAction::triggered, this, [] { saveXyzTilesServers(); } );
    menu->addAction( actionSaveXyzTilesServers );

    QAction *actionLoadXyzTilesServers = new QAction( tr( "Load Connections…" ), this );
    connect( actionLoadXyzTilesServers, &QAction::triggered, this, [rootItem] { loadXyzTilesServers( rootItem ); } );
    menu->addAction( actionLoadXyzTilesServers );
  }
}

QWidget *QgsXyzDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  return _paramWidget( root );
}

void QgsXyzDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsXyzConnectionDialog dlg;
  dlg.setConnection( QgsXyzConnectionUtils::connection( item->name() ) );
  if ( !dlg.exec() )
    return;

  QgsXyzConnectionUtils::deleteConnection( item->name() );
  QgsXyzConnectionUtils::addConnection( dlg.connection() );

  item->parent()->refreshConnections();
}

void QgsXyzDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  const QStringList connections = QgsXyzConnectionSettings::sTreeXyzConnections->items();

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  QgsXyzConnection connection = QgsXyzConnectionUtils::connection( connectionName );
  connection.name = newConnectionName;
  QgsXyzConnectionUtils::addConnection( connection );

  item->parent()->refreshConnections();
}

void QgsXyzDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsXyzConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  QgsXyzConnectionUtils::addConnection( dlg.connection() );
  item->refreshConnections();
}

void QgsXyzDataItemGuiProvider::saveXyzTilesServers()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::XyzTiles );
  dlg.exec();
}

void QgsXyzDataItemGuiProvider::loadXyzTilesServers( QgsDataItem *item )
{
  QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::XyzTiles, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}
