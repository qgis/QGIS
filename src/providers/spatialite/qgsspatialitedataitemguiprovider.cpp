/***************************************************************************
  qgsspatialitedataitemguiprovider.cpp
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

#include "qgsspatialitedataitemguiprovider.h"
#include "moc_qgsspatialitedataitemguiprovider.cpp"
#include "qgsspatialitedataitems.h"
#include "qgsspatialitesourceselect.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsspatialiteproviderconnection.h"

#include "qgsapplication.h"
#include "qgsmessageoutput.h"
#include "qgsmimedatautils.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"
#include "qgsfileutils.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgsdbimportvectorlayerdialog.h"
#include "qgsmessagebar.h"

#include <QFileDialog>
#include <QMessageBox>


void QgsSpatiaLiteDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsSLRootItem *rootItem = qobject_cast<QgsSLRootItem *>( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionCreateDatabase = new QAction( tr( "Create Database…" ), menu );
    connect( actionCreateDatabase, &QAction::triggered, this, [rootItem] { createDatabase( rootItem ); } );
    menu->addAction( actionCreateDatabase );
  }

  if ( QgsSLConnectionItem *connItem = qobject_cast<QgsSLConnectionItem *>( item ) )
  {
    QAction *importVectorAction = new QAction( QObject::tr( "Import Vector Layer…" ), menu );
    menu->addAction( importVectorAction );
    QObject::connect( importVectorAction, &QAction::triggered, item, [connItem, context, this] { handleImportVector( connItem, context ); } );

    const QList<QgsSLConnectionItem *> slConnectionItems = QgsDataItem::filteredItems<QgsSLConnectionItem>( selection );
    QAction *actionDeleteConnection = new QAction( slConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDeleteConnection, &QAction::triggered, this, [slConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( slConnectionItems, []( const QString &connectionName ) {
        QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "spatialite" ) );
        providerMetadata->deleteConnection( connectionName ); }, context );
    } );
    menu->addAction( actionDeleteConnection );
  }
}

bool QgsSpatiaLiteDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsSLLayerItem *layerItem = qobject_cast<QgsSLLayerItem *>( item ) )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Object" ), QObject::tr( "Are you sure you want to delete %1?" ).arg( layerItem->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    const QgsDataSourceUri uri( layerItem->uri() );
    QString errCause;
    if ( !SpatiaLiteUtils::deleteLayer( uri.database(), uri.table(), errCause ) )
    {
      notify( tr( "Delete Layer" ), errCause, context, Qgis::MessageLevel::Warning );
    }
    else
    {
      notify( tr( "Delete Layer" ), tr( "Layer deleted successfully." ), context, Qgis::MessageLevel::Success );
      if ( layerItem->parent() )
        layerItem->parent()->refresh();
      return true;
    }
  }
  return false;
}

bool QgsSpatiaLiteDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast<QgsSLConnectionItem *>( item ) )
    return true;

  return false;
}

bool QgsSpatiaLiteDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction action )
{
  if ( QgsSLConnectionItem *connItem = qobject_cast<QgsSLConnectionItem *>( item ) )
  {
    return handleDropConnectionItem( connItem, data, action, context );
  }
  return false;
}


void QgsSpatiaLiteDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  if ( QgsSpatiaLiteSourceSelect::newConnection( nullptr ) )
  {
    item->refreshConnections();
  }
}

void QgsSpatiaLiteDataItemGuiProvider::createDatabase( QgsDataItem *item )
{
  const QgsSettings settings;
  const QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::homePath() ).toString();

  QString filename = QFileDialog::getSaveFileName( nullptr, tr( "New SpatiaLite Database File" ), lastUsedDir, tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db)" );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "sqlite" ) << QStringLiteral( "db" ) << QStringLiteral( "sqlite3" ) << QStringLiteral( "db3" ) << QStringLiteral( "s3db" ) );

  QString errCause;
  if ( SpatiaLiteUtils::createDb( filename, errCause ) )
  {
    QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "spatialite" ) );
    std::unique_ptr<QgsSpatiaLiteProviderConnection> providerConnection( qgis::down_cast<QgsSpatiaLiteProviderConnection *>( providerMetadata->createConnection( QStringLiteral( "dbname='%1'" ).arg( filename ), QVariantMap() ) ) );
    if ( providerConnection )
    {
      const QFileInfo fi( filename );
      providerMetadata->saveConnection( providerConnection.get(), fi.fileName() );
    }

    item->refresh();
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Create SpatiaLite database" ), tr( "Failed to create the database:\n" ) + errCause );
  }
}

bool QgsSpatiaLiteDataItemGuiProvider::handleDropConnectionItem( QgsSLConnectionItem *connItem, const QMimeData *data, Qt::DropAction, QgsDataItemGuiContext context )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  const QgsMimeDataUtils::UriList sourceUris = QgsMimeDataUtils::decodeUriList( data );
  if ( sourceUris.size() == 1 && sourceUris.at( 0 ).layerType == QLatin1String( "vector" ) )
  {
    return handleDropUri( connItem, sourceUris.at( 0 ), context );
  }

  // TODO: when dropping multiple layers, we need a dedicated "bulk import" dialog for settings which apply to ALL layers

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  QStringList importResults;
  bool hasError = false;

  for ( const QgsMimeDataUtils::Uri &u : sourceUris )
  {
    // open the source layer
    bool owner;
    QString error;
    QgsVectorLayer *srcLayer = u.vectorLayer( owner, error );
    if ( !srcLayer )
    {
      importResults.append( tr( "%1: %2" ).arg( u.name, error ) );
      hasError = true;
      continue;
    }

    if ( srcLayer->isValid() )
    {
      QString geomColumn { QStringLiteral( "geom" ) };
      if ( !srcLayer->dataProvider()->geometryColumnName().isEmpty() )
      {
        geomColumn = srcLayer->dataProvider()->geometryColumnName();
      }

      QgsAbstractDatabaseProviderConnection::VectorLayerExporterOptions exporterOptions;
      exporterOptions.layerName = u.name;
      exporterOptions.wkbType = srcLayer->wkbType();
      exporterOptions.geometryColumn = geomColumn;

      QVariantMap providerOptions;
      const QString destUri = databaseConnection->createVectorLayerExporterDestinationUri( exporterOptions, providerOptions );
      QgsDebugMsgLevel( "URI " + destUri, 2 );

      auto exportTask = std::make_unique<QgsVectorLayerExporterTask>( srcLayer, destUri, QStringLiteral( "spatialite" ), srcLayer->crs(), providerOptions, owner );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, connItem, [=]() {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to SpatiaLite database" ), tr( "Import was successful." ) );
        connItem->refresh();
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, connItem, [=]( Qgis::VectorExportResult error, const QString &errorMessage ) {
        if ( error != Qgis::VectorExportResult::UserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to SpatiaLite database" ) );
          output->setMessage( tr( "Failed to import layer!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        connItem->refresh();
      } );

      QgsApplication::taskManager()->addTask( exportTask.release() );
    }
    else
    {
      importResults.append( tr( "%1: Not a valid layer!" ).arg( u.name ) );
      hasError = true;
    }
  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to SpatiaLite database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

bool QgsSpatiaLiteDataItemGuiProvider::handleDropUri( QgsSLConnectionItem *connectionItem, const QgsMimeDataUtils::Uri &sourceUri, QgsDataItemGuiContext context )
{
  QPointer< QgsSLConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  auto onSuccess = [connectionItemPointer]() {
    if ( connectionItemPointer )
      connectionItemPointer->refresh();
  };

  auto onFailure = [connectionItemPointer]( Qgis::VectorExportResult, const QString & ) {
    if ( connectionItemPointer )
      connectionItemPointer->refresh();
  };

  return QgsDataItemGuiProviderUtils::handleDropUriForConnection( std::move( databaseConnection ), sourceUri, QString(), context, tr( "Spatialite Import" ), tr( "Import to SpatiaLite database" ), QVariantMap(), onSuccess, onFailure, this );
}

void QgsSpatiaLiteDataItemGuiProvider::handleImportVector( QgsSLConnectionItem *connectionItem, QgsDataItemGuiContext context )
{
  if ( !connectionItem )
    return;

  QPointer< QgsSLConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return;

  auto onSuccess = [connectionItemPointer]() {
    if ( connectionItemPointer )
      connectionItemPointer->refresh();
  };

  auto onFailure = [connectionItemPointer]( Qgis::VectorExportResult, const QString & ) {
    if ( connectionItemPointer )
      connectionItemPointer->refresh();
  };

  QgsDataItemGuiProviderUtils::handleImportVectorLayerForConnection( std::move( databaseConnection ), QString(), context, tr( "Spatialite Import" ), tr( "Import to SpatiaLite database" ), QVariantMap(), onSuccess, onFailure, this );
}
