/***************************************************************************
      qgsogritemguiprovider.h
      -------------------
    begin                : June, 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogritemguiprovider.h"
///@cond PRIVATE

#include <QAction>
#include <QMenu>
#include <QString>
#include <QMessageBox>

#include "qgsproject.h"
#include "qgsogrutils.h"
#include "qgsproviderregistry.h"
#include "qgslayeritem.h"
#include "qgsdatacollectionitem.h"
#include "qgsogrproviderutils.h"
#include "qgsgeopackagedataitems.h"

void QgsOgrItemGuiProvider::populateContextMenu(
  QgsDataItem *item,
  QMenu *menu,
  const QList<QgsDataItem *> &,
  QgsDataItemGuiContext context )
{
  if ( QgsLayerItem *layerItem = qobject_cast< QgsLayerItem * >( item ) )
  {
    if ( layerItem->providerKey() == QLatin1String( "ogr" ) && !qobject_cast< QgsGeoPackageAbstractLayerItem * >( item ) )
    {
      if ( !( layerItem->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile ) )
      {
        // item is a layer which sits inside a collection.

        QMenu *manageLayerMenu = new QMenu( tr( "Manage" ), menu );

        // test if GDAL supports deleting this layer
        const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layerItem->providerKey(), layerItem->uri() );
        const QString path = parts.value( QStringLiteral( "path" ) ).toString();
        bool canDeleteLayers = false;
        if ( !path.isEmpty() )
        {
          OGRSFDriverH hDriver = nullptr;
          const gdal::ogr_datasource_unique_ptr hDS( OGROpen( path.toUtf8().constData(), TRUE, &hDriver ) );
          canDeleteLayers = hDS && OGR_DS_TestCapability( hDS.get(), ODsCDeleteLayer );
        }

        QAction *actionDeleteLayer = new QAction( QObject::tr( "Delete Layer “%1”…" ).arg( layerItem->name() ), menu );
        QVariantMap data;
        data.insert( QStringLiteral( "uri" ), layerItem->uri() );
        data.insert( QStringLiteral( "name" ), layerItem->name() );
        data.insert( QStringLiteral( "parent" ), QVariant::fromValue( QPointer< QgsDataItem >( layerItem->parent() ) ) );
        actionDeleteLayer->setData( data );
        connect( actionDeleteLayer, &QAction::triggered, this, [ = ] { onDeleteLayer( context ); } );
        actionDeleteLayer->setEnabled( canDeleteLayers );
        manageLayerMenu->addAction( actionDeleteLayer );

        menu->addMenu( manageLayerMenu );
      }
    }
  }

  if ( QgsDataCollectionItem *collectionItem = qobject_cast< QgsDataCollectionItem * >( item ) )
  {
    if ( collectionItem->providerKey() == QLatin1String( "ogr" ) && !qobject_cast< QgsGeoPackageCollectionItem *>( item ) )
    {
      const bool isFolder = QFileInfo( collectionItem->path() ).isDir();
      // Messages are different for files and tables
      const QString message = QObject::tr( "Delete %1 “%2”…" ).arg( isFolder ? tr( "Folder" ) : tr( "File" ), collectionItem->name() );
      QAction *actionDeleteCollection = new QAction( message, menu );

      QVariantMap data;
      data.insert( QStringLiteral( "path" ), collectionItem->path() );
      data.insert( QStringLiteral( "parent" ), QVariant::fromValue( QPointer< QgsDataItem >( collectionItem->parent() ) ) );
      actionDeleteCollection->setData( data );
      connect( actionDeleteCollection, &QAction::triggered, this, [ = ] { deleteCollection( context ); } );
      menu->addAction( actionDeleteCollection );
    }
  }
}

void QgsOgrItemGuiProvider::onDeleteLayer( QgsDataItemGuiContext context )
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  const QString uri = data[QStringLiteral( "uri" )].toString();
  const QString name = data[QStringLiteral( "name" )].toString();
  const QPointer< QgsDataItem > parent = data[QStringLiteral( "parent" )].value<QPointer< QgsDataItem >>();

  const QString title = QObject::tr( "Delete Layer" );
  // Check if the layer is in the registry
  const QgsMapLayer *projectLayer = nullptr;
  const auto constMapLayers = QgsProject::instance()->mapLayers();
  for ( const QgsMapLayer *layer : constMapLayers )
  {
    if ( layer->publicSource() == uri )
    {
      projectLayer = layer;
    }
  }
  if ( ! projectLayer )
  {
    const QString confirmMessage = QObject::tr( "Are you sure you want to delete layer '%1' from datasource?" ).arg( name );
    if ( QMessageBox::question( nullptr, title,
                                confirmMessage,
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;

    QString errCause;
    const bool res = QgsOgrProviderUtils::deleteLayer( uri, errCause );
    if ( !res )
    {
      notify( title, errCause, context, Qgis::MessageLevel::Critical );
    }
    else
    {
      notify( title, tr( "Layer deleted successfully." ), context, Qgis::MessageLevel::Success );
      if ( parent )
        parent->refresh();
    }
  }
  else
  {
    notify( title, QObject::tr( "The layer '%1' cannot be deleted because it is in the current project as '%2',"
                                " remove it from the project and retry." ).arg( name, projectLayer->name() ), context, Qgis::MessageLevel::Warning );
  }
}

void QgsOgrItemGuiProvider::deleteCollection( QgsDataItemGuiContext context )
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  const QString path = data[QStringLiteral( "path" )].toString();
  const QPointer< QgsDataItem > parent = data[QStringLiteral( "parent" )].value<QPointer< QgsDataItem >>();

  const bool isFolder = QFileInfo( path ).isDir();
  const QString type = isFolder ? tr( "folder" ) : tr( "file" );
  const QString typeCaps = isFolder ? tr( "Folder" ) : tr( "File" );
  const QString title = QObject::tr( "Delete %1" ).arg( type );
  // Check if the layer is in the project
  const QgsMapLayer *projectLayer = nullptr;
  const auto mapLayers = QgsProject::instance()->mapLayers();
  for ( auto it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( it.value()->providerType(), it.value()->source() );
    if ( parts.value( QStringLiteral( "path" ) ).toString() == path )
    {
      projectLayer = it.value();
    }
  }
  if ( ! projectLayer )
  {
    const QString confirmMessage = QObject::tr( "Are you sure you want to delete '%1'?" ).arg( path );

    if ( QMessageBox::question( nullptr, title,
                                confirmMessage,
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;

    bool res = false;
    if ( isFolder )
    {
      // e.g. the abomination which is gdb
      QDir dir( path );
      res = dir.removeRecursively();
    }
    else
    {
      res = QFile::remove( path );
    }
    if ( !res )
    {
      notify( title, tr( "Could not delete %1." ).arg( type ), context, Qgis::MessageLevel::Warning );
    }
    else
    {
      notify( title, tr( "%1 deleted successfully." ).arg( typeCaps ), context, Qgis::MessageLevel::Success );
      if ( parent )
        parent->refresh();
    }
  }
  else
  {
    notify( title, tr( "The %1 '%2' cannot be deleted because it is in the current project as '%3',"
                       " remove it from the project and retry." ).arg( type, path, projectLayer->name() ), context, Qgis::MessageLevel::Warning );
  }
}
///@endcond
