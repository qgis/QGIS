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
#include "qgsogrdataitems.h"
#include "qgsogrutils.h"
#include "qgsproviderregistry.h"

void QgsOgrItemGuiProvider::populateContextMenu(
  QgsDataItem *item,
  QMenu *menu,
  const QList<QgsDataItem *> &,
  QgsDataItemGuiContext context )
{
  if ( QgsOgrLayerItem *layerItem = qobject_cast< QgsOgrLayerItem * >( item ) )
  {
    // Messages are different for files and tables
    QString message = layerItem->isSubLayer() ? QObject::tr( "Delete Layer “%1”…" ).arg( layerItem->name() ) : QObject::tr( "Delete File “%1”…" ).arg( layerItem->name() );
    QAction *actionDeleteLayer = new QAction( message, layerItem->parent() );
    QVariantMap data;
    data.insert( QStringLiteral( "isSubLayer" ), layerItem->isSubLayer() );
    data.insert( QStringLiteral( "uri" ), layerItem->uri() );
    data.insert( QStringLiteral( "name" ), layerItem->name() );
    data.insert( QStringLiteral( "parent" ), QVariant::fromValue( QPointer< QgsDataItem >( layerItem->parent() ) ) );
    actionDeleteLayer->setData( data );
    connect( actionDeleteLayer, &QAction::triggered, this, [ = ] { onDeleteLayer( context ); } );
    menu->addAction( actionDeleteLayer );
  }

  if ( QgsOgrDataCollectionItem *collectionItem = qobject_cast< QgsOgrDataCollectionItem * >( item ) )
  {
    const bool isFolder = QFileInfo( collectionItem->path() ).isDir();
    // Messages are different for files and tables
    QString message = QObject::tr( "Delete %1 “%2”…" ).arg( isFolder ? tr( "Folder" ) : tr( "File" ), collectionItem->name() );
    QAction *actionDeleteCollection = new QAction( message, collectionItem->parent() );

    QVariantMap data;
    data.insert( QStringLiteral( "path" ), collectionItem->path() );
    data.insert( QStringLiteral( "parent" ), QVariant::fromValue( QPointer< QgsDataItem >( collectionItem->parent() ) ) );
    actionDeleteCollection->setData( data );
    connect( actionDeleteCollection, &QAction::triggered, this, [ = ] { deleteCollection( context ); } );
    menu->addAction( actionDeleteCollection );
  }
}

void QgsOgrItemGuiProvider::onDeleteLayer( QgsDataItemGuiContext context )
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  bool isSubLayer = data[QStringLiteral( "isSublayer" )].toBool();
  const QString uri = data[QStringLiteral( "uri" )].toString();
  const QString name = data[QStringLiteral( "name" )].toString();
  QPointer< QgsDataItem > parent = data[QStringLiteral( "parent" )].value<QPointer< QgsDataItem >>();

  // Messages are different for files and tables
  QString title = isSubLayer ? QObject::tr( "Delete Layer" ) : QObject::tr( "Delete File" );
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
    QString confirmMessage;
    if ( isSubLayer )
    {
      confirmMessage = QObject::tr( "Are you sure you want to delete layer '%1' from datasource?" ).arg( name );
    }
    else
    {
      confirmMessage = QObject::tr( "Are you sure you want to delete file '%1'?" ).arg( uri );
    }
    if ( QMessageBox::question( nullptr, title,
                                confirmMessage,
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;

    QString errCause;
    bool res = QgsOgrProviderUtils::deleteLayer( uri, errCause );
    if ( !res )
    {
      notify( title, errCause, context, Qgis::MessageLevel::Critical );
    }
    else
    {
      notify( title, isSubLayer ? tr( "Layer deleted successfully." ) :  tr( "File deleted successfully." ), context, Qgis::MessageLevel::Success );
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
  QPointer< QgsDataItem > parent = data[QStringLiteral( "parent" )].value<QPointer< QgsDataItem >>();

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
