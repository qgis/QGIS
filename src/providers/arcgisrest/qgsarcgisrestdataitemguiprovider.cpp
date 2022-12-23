/***************************************************************************
  qgsarcgisrestdataitemguiprovider.cpp
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

#include "qgsarcgisrestdataitemguiprovider.h"
#include "qgsarcgisrestdataitems.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsnewarcgisrestconnection.h"
#include "qgsowsconnection.h"
#include "qgsafsprovider.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsbrowsertreeview.h"
#include "qgsguiutils.h"
#include "qgsvectorlayer.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>


void QgsArcGisRestDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  if ( QgsArcGisRestRootItem *rootItem = qobject_cast< QgsArcGisRestRootItem * >( item ) )
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
  else if ( QgsArcGisRestConnectionItem *connectionItem = qobject_cast< QgsArcGisRestConnectionItem * >( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [connectionItem] { refreshConnection( connectionItem ); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [connectionItem] { editConnection( connectionItem ); } );
    menu->addAction( actionEdit );

    QAction *actionDelete = new QAction( tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [connectionItem] { deleteConnection( connectionItem ); } );
    menu->addAction( actionDelete );

    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( connectionItem->url() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsArcGisRestFolderItem *folderItem = qobject_cast< QgsArcGisRestFolderItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( folderItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsArcGisFeatureServiceItem *serviceItem = qobject_cast< QgsArcGisFeatureServiceItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( serviceItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsArcGisMapServiceItem *serviceItem = qobject_cast< QgsArcGisMapServiceItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( serviceItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsArcGisRestParentLayerItem *layerItem = qobject_cast< QgsArcGisRestParentLayerItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( layerItem->path() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsArcGisFeatureServiceLayerItem *layerItem = qobject_cast< QgsArcGisFeatureServiceLayerItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( layerItem->path() ) );
    } );
    menu->addAction( viewInfo );
    menu->addSeparator();

    QAction *addWithFilterAction = new QAction( tr( "Add Filtered Layer to Project…" ), menu );
    // bit of a gross hack to get this action to appear after the "add layer to project" action, which won't have been created yet...
    addWithFilterAction->setProperty( "insertAfter", "addLayerToProject" );
    menu->addAction( addWithFilterAction );
    const QgsMimeDataUtils::UriList uris = layerItem->mimeUris();

    connect( addWithFilterAction, &QAction::triggered, this, [uris, context, this]
    {
      addFilteredLayer( uris.value( 0 ), context );
    } );
  }
  else if ( QgsArcGisMapServiceLayerItem *layerItem = qobject_cast< QgsArcGisMapServiceLayerItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( layerItem->path() ) );
    } );
    menu->addAction( viewInfo );
    menu->addSeparator();
  }
}

void QgsArcGisRestDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsNewArcGisRestConnectionDialog nc( nullptr, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ), QString() );
  nc.setWindowTitle( tr( "Create a New ArcGIS REST Server Connection" ) );

  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsArcGisRestDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsNewArcGisRestConnectionDialog nc( nullptr, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ), item->name() );
  nc.setWindowTitle( tr( "Modify ArcGIS REST Server Connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    item->refresh();
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsArcGisRestDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connection" ),
                              QObject::tr( "Are you sure you want to remove the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsOwsConnection::deleteConnection( QStringLiteral( "arcgisfeatureserver" ), item->name() );

  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsArcGisRestDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsArcGisRestDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::ArcgisFeatureServer );
  dlg.exec();
}

void QgsArcGisRestDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::ArcgisFeatureServer, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

void QgsArcGisRestDataItemGuiProvider::addFilteredLayer( const QgsMimeDataUtils::Uri &uri, QgsDataItemGuiContext context )
{
  // Query available fields
  QgsDataSourceUri ds( uri.uri );
  ds.setSql( QStringLiteral( "1=0" ) ); // don't retrieve any records

  QgsDataProvider::ProviderOptions providerOptions;
  QgsTemporaryCursorOverride cursor( Qt::WaitCursor );
  QgsAfsProvider provider( ds.uri( false ), providerOptions );
  if ( !provider.isValid() )
  {
    return;
  }
  cursor.release();

  //show expression builder
  QgsExpressionBuilderDialog d( nullptr, QString(), context.view() ? context.view()->window() : nullptr );

  //add available attributes to expression builder
  QgsExpressionBuilderWidget *w = d.expressionBuilder();
  w->initWithFields( provider.fields() );

  if ( d.exec() == QDialog::Accepted )
  {
    const QString sql = w->expressionText();
    ds.setSql( sql );

    std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( ds.uri( false ), uri.name, QStringLiteral( "arcgisfeatureserver" ) );
    QgsProject::instance()->addMapLayer( layer.release() );
  }
}
