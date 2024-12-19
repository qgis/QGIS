/***************************************************************************
  qgsstacdataitemguiprovider.cpp
  --------------------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacdataitemguiprovider.h"
#include "moc_qgsstacdataitemguiprovider.cpp"
#include "qgsstaccontroller.h"
#include "qgsstacdataitems.h"
#include "qgsstacconnection.h"
#include "qgsstacconnectiondialog.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgsstacitem.h"
#include "qgsstacdownloadassetsdialog.h"
#include "qgsstacobjectdetailsdialog.h"


///@cond PRIVATE

void QgsStacDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsStacRootItem *rootItem = qobject_cast<QgsStacRootItem *>( item ) )
  {
    QAction *actionNewConnection = new QAction( tr( "New STAC Connection…" ), menu );
    connect( actionNewConnection, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNewConnection );

    menu->addSeparator();

    QAction *actionSave = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSave, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSave );

    QAction *actionLoad = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoad, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoad );
  }

  if ( QgsStacConnectionItem *connItem = qobject_cast<QgsStacConnectionItem *>( item ) )
  {
    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
    menu->addAction( actionEdit );

    const QList<QgsStacConnectionItem *> stacConnectionItems = QgsDataItem::filteredItems<QgsStacConnectionItem>( selection );
    QAction *actionDelete = new QAction( stacConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [stacConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( stacConnectionItems, []( const QString &connectionName ) { QgsStacConnection( QString() ).remove( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );
  }

  if ( QgsStacCatalogItem *catalogItem = qobject_cast<QgsStacCatalogItem *>( item ) )
  {
    menu->addSeparator();

    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [catalogItem] { catalogItem->refresh(); } );
    menu->addAction( actionRefresh );

    if ( catalogItem->stacCatalog() )
    {
      QAction *actionDetails = new QAction( tr( "Details…" ), menu );
      connect( actionDetails, &QAction::triggered, this, [catalogItem] { showDetails( catalogItem ); } );
      menu->addAction( actionDetails );
    }
  }

  if ( QgsStacItemItem *itemItem = qobject_cast<QgsStacItemItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [itemItem] { itemItem->refresh(); } );
    menu->addAction( actionRefresh );

    if ( itemItem->stacItem() )
    {
      menu->addSeparator();

      QAction *actionDownload = new QAction( tr( "Download Assets…" ), menu );
      connect( actionDownload, &QAction::triggered, this, [itemItem, context] { downloadAssets( itemItem, context ); } );
      menu->addAction( actionDownload );

      QAction *actionDetails = new QAction( tr( "Details…" ), menu );
      connect( actionDetails, &QAction::triggered, this, [itemItem] { showDetails( itemItem ); } );
      menu->addAction( actionDetails );
    }
  }
}

void QgsStacDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  const QgsStacConnection::Data connection = QgsStacConnection::connection( item->name() );
  const QString uri = QgsStacConnection::encodedUri( connection );

  QgsStacConnectionDialog dlg;

  dlg.setConnection( item->name(), uri );
  if ( !dlg.exec() )
    return;

  QgsStacConnection( QString() ).remove( item->name() );

  QgsStacConnection::Data newConnection = QgsStacConnection::decodedUri( dlg.connectionUri() );

  QgsStacConnection::addConnection( dlg.connectionName(), newConnection );

  item->parent()->refreshConnections();
}

void QgsStacDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsStacDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsStacConnectionDialog dlg;
  if ( !dlg.exec() )
    return;

  QgsStacConnection::Data conn = QgsStacConnection::decodedUri( dlg.connectionUri() );

  QgsStacConnection::addConnection( dlg.connectionName(), conn );

  item->refreshConnections();
}

void QgsStacDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::STAC );
  dlg.exec();
}

void QgsStacDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::STAC, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

void QgsStacDataItemGuiProvider::showDetails( QgsDataItem *item )
{
  QgsStacObject *obj = nullptr;

  if ( QgsStacItemItem *itemItem = qobject_cast<QgsStacItemItem *>( item ) )
  {
    obj = itemItem->stacItem();
  }
  else if ( QgsStacCatalogItem *catalogItem = qobject_cast<QgsStacCatalogItem *>( item ) )
  {
    obj = catalogItem->stacCatalog();
  }

  if ( obj )
  {
    QgsStacObjectDetailsDialog d;
    d.setStacObject( obj );
    d.exec();
  }
}

void QgsStacDataItemGuiProvider::downloadAssets( QgsDataItem *item, QgsDataItemGuiContext context )
{
  QgsStacItemItem *itemItem = qobject_cast<QgsStacItemItem *>( item );

  if ( !itemItem )
    return;

  QgsStacDownloadAssetsDialog dialog;
  dialog.setStacItem( itemItem->stacItem() );
  dialog.setMessageBar( context.messageBar() );
  dialog.setAuthCfg( itemItem->stacController()->authCfg() );
  dialog.exec();
}

///@endcond
