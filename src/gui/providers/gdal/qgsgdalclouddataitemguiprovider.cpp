/***************************************************************************
  qgsgdalclouddataitemguiprovider.cpp
  --------------------------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalclouddataitemguiprovider.h"
#include "moc_qgsgdalclouddataitemguiprovider.cpp"
#include "qgsgdalclouddataitems.h"
#include "qgsgdalcloudconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgsgdalcloudconnectiondialog.h"
#include "qgsgdalutils.h"

#include <QMessageBox>
#include <QFileDialog>

///@cond PRIVATE

void QgsGdalCloudDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsGdalCloudConnectionItem *providerItem = qobject_cast<QgsGdalCloudConnectionItem *>( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [providerItem] { editConnection( providerItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), menu );
    connect( actionDuplicate, &QAction::triggered, this, [providerItem] { duplicateConnection( providerItem ); } );
    menu->addAction( actionDuplicate );

    const QList<QgsGdalCloudConnectionItem *> stConnectionItems = QgsDataItem::filteredItems<QgsGdalCloudConnectionItem>( selection );
    QAction *actionDelete = new QAction( stConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [stConnectionItems, item, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( stConnectionItems, []( const QString &connectionName ) { QgsGdalCloudProviderConnection( QString() ).remove( connectionName ); }, context );

      if ( QgsGdalCloudProviderItem *cloudItem = qobject_cast<QgsGdalCloudProviderItem *>( item->parent() ) )
        cloudItem->refresh();
    } );
    menu->addAction( actionDelete );
  }
  else if ( QgsGdalCloudProviderItem *providerItem = qobject_cast<QgsGdalCloudProviderItem *>( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [providerItem] { newConnection( providerItem, providerItem->vsiHandler() ); } );
    menu->addAction( actionNew );
  }
  else if ( QgsGdalCloudRootItem *rootItem = qobject_cast<QgsGdalCloudRootItem *>( item ) )
  {
    QList<QgsGdalUtils::VsiNetworkFileSystemDetails> vsiDetails = QgsGdalUtils::vsiNetworkFileSystems();
    std::sort( vsiDetails.begin(), vsiDetails.end(), []( const QgsGdalUtils::VsiNetworkFileSystemDetails &a, const QgsGdalUtils::VsiNetworkFileSystemDetails &b ) {
      return QString::localeAwareCompare( a.name, b.name ) < 0;
    } );

    QMenu *newConnectionMenu = new QMenu( tr( "New Connection" ), menu );
    for ( const QgsGdalUtils::VsiNetworkFileSystemDetails &vsiDetail : std::as_const( vsiDetails ) )
    {
      if ( vsiDetail.identifier == QLatin1String( "vsicurl" ) )
        continue;

      QAction *actionNew = new QAction( tr( "%1…" ).arg( vsiDetail.name ), menu );
      connect( actionNew, &QAction::triggered, this, [rootItem, vsiDetail] { newConnection( rootItem, vsiDetail ); } );
      newConnectionMenu->addAction( actionNew );
    }
    menu->addMenu( newConnectionMenu );

    menu->addSeparator();

    QAction *actionSave = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSave, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSave );

    QAction *actionLoad = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoad, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoad );
  }
}

void QgsGdalCloudDataItemGuiProvider::editConnection( QgsGdalCloudConnectionItem *item )
{
  QgsGdalCloudProviderItem *cloudItem = qobject_cast<QgsGdalCloudProviderItem *>( item->parent() );
  if ( !cloudItem )
    return;

  QgsGdalCloudRootItem *rootItem = qobject_cast<QgsGdalCloudRootItem *>( cloudItem->parent() );
  if ( !rootItem )
    return;

  const QString connectionName = item->name();

  const QgsGdalCloudProviderConnection::Data connection = QgsGdalCloudProviderConnection::connection( connectionName );
  const QString uri = QgsGdalCloudProviderConnection::encodedUri( connection );
  QgsGdalCloudConnectionDialog dlg;

  dlg.setConnection( connectionName, uri );
  if ( !dlg.exec() )
    return;

  QgsGdalCloudProviderConnection( QString() ).remove( connectionName );

  QgsGdalCloudProviderConnection::Data newConnection = QgsGdalCloudProviderConnection::decodedUri( dlg.connectionUri() );
  QgsGdalCloudProviderConnection::addConnection( dlg.connectionName(), newConnection );

  rootItem->refreshConnections();
  if ( dlg.connectionName() != connectionName )
    cloudItem->refresh();
  else
    item->refresh();
}

void QgsGdalCloudDataItemGuiProvider::duplicateConnection( QgsGdalCloudConnectionItem *item )
{
  QgsGdalCloudProviderItem *cloudItem = qobject_cast<QgsGdalCloudProviderItem *>( item->parent() );
  if ( !cloudItem )
    return;

  QgsGdalCloudRootItem *rootItem = qobject_cast<QgsGdalCloudRootItem *>( cloudItem->parent() );
  if ( !rootItem )
    return;

  const QString connectionName = item->name();
  const QgsGdalCloudProviderConnection::Data connection = QgsGdalCloudProviderConnection::connection( connectionName );
  const QStringList connections = QgsGdalCloudProviderConnection::sTreeConnectionCloud->items();

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  QgsGdalCloudProviderConnection::addConnection( newConnectionName, connection );
  cloudItem->refresh();
}

void QgsGdalCloudDataItemGuiProvider::newConnection( QgsDataItem *item, const QgsGdalUtils::VsiNetworkFileSystemDetails &driver )
{
  QgsGdalCloudRootItem *rootItem = qobject_cast<QgsGdalCloudRootItem *>( item ) ? qobject_cast<QgsGdalCloudRootItem *>( item ) : qobject_cast<QgsGdalCloudRootItem *>( item->parent() );

  QgsGdalCloudProviderItem *cloudItem = nullptr;
  const QVector<QgsDataItem *> cloudChildren = rootItem->children();
  for ( QgsDataItem *child : cloudChildren )
  {
    if ( QgsGdalCloudProviderItem *providerRoot = qobject_cast<QgsGdalCloudProviderItem *>( child ) )
    {
      if ( providerRoot->vsiHandler().identifier == driver.identifier )
      {
        cloudItem = providerRoot;
        break;
      }
    }
  }

  QgsGdalCloudConnectionDialog dlg;
  dlg.setVsiHandler( driver.identifier );
  dlg.setWindowTitle( tr( "New %1 Connection" ).arg( driver.name ) );
  if ( !dlg.exec() )
    return;

  QgsGdalCloudProviderConnection::Data conn = QgsGdalCloudProviderConnection::decodedUri( dlg.connectionUri() );
  QgsGdalCloudProviderConnection::addConnection( dlg.connectionName(), conn );

  if ( cloudItem )
    cloudItem->refresh();
  else
    rootItem->refresh();

  rootItem->refreshConnections();
}

void QgsGdalCloudDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::CloudStorage );
  dlg.exec();
}

void QgsGdalCloudDataItemGuiProvider::loadConnections( QgsGdalCloudRootItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::CloudStorage, fileName );
  if ( dlg.exec() == QDialog::Accepted )
  {
    item->refreshConnections();
    item->refresh();
  }
}

///@endcond
