/***************************************************************************
      qgsgeopackageitemguiprovider.h
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

#include "qgsgeopackageitemguiprovider.h"
///@cond PRIVATE

#include <QAction>
#include <QMenu>
#include <QString>
#include <QMessageBox>

#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsgeopackagedataitems.h"
#include "qgsnewnamedialog.h"
#include "qgsnewgeopackagelayerdialog.h"
#include "qgsmessageoutput.h"
#include "qgsapplication.h"
#include "qgsgeopackagerasterwritertask.h"
#include "qgsvectorlayerexporter.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"
#include "gdal.h"
#include "qgsogrdbconnection.h"
#include "qgsgeopackageproviderconnection.h"
#include "qgsmessagebar.h"
#include "qgsprovidermetadata.h"
#include "qgsogrproviderutils.h"

void QgsGeoPackageItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu,
    const QList<QgsDataItem *> &selectedItems,
    QgsDataItemGuiContext context )
{
  if ( QgsGeoPackageVectorLayerItem *layerItem = qobject_cast< QgsGeoPackageVectorLayerItem * >( item ) )
  {
    // Check capabilities
    // (We only show the rename action when the user has a single layer selected -- it doesn't work on multi-layers at once)
    if ( selectedItems.size() == 1 && layerItem->capabilities2() & Qgis::BrowserItemCapability::Rename )
    {
      QMenu *manageLayerMenu = new QMenu( tr( "Manage" ), menu );

      QAction *actionRenameLayer = new QAction( tr( "Rename Layer “%1”…" ).arg( layerItem->name() ), menu );
      const QString uri = layerItem->uri();
      const QString providerKey = layerItem->providerKey();
      const QStringList tableNames = layerItem->tableNames();
      const QPointer< QgsDataItem > itemPointer( layerItem );
      connect( actionRenameLayer, &QAction::triggered, this, [this, uri, providerKey, tableNames, itemPointer, context ]
      {
        renameVectorLayer( uri, providerKey, tableNames, itemPointer, context );
      } );
      manageLayerMenu->addAction( actionRenameLayer );

      menu->addMenu( manageLayerMenu );
    }
  }

  if ( QgsGeoPackageRootItem *rootItem = qobject_cast< QgsGeoPackageRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, rootItem, &QgsGeoPackageRootItem::newConnection );
    menu->addAction( actionNew );

    QAction *actionCreateDatabase = new QAction( tr( "Create Database…" ), menu );
    const QPointer< QgsGeoPackageRootItem > rootItemPointer( rootItem );
    connect( actionCreateDatabase, &QAction::triggered, this, [this, rootItemPointer ]
    {
      createDatabase( rootItemPointer );
    } );
    menu->addAction( actionCreateDatabase );
  }

  if ( QgsGeoPackageCollectionItem *collectionItem = qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
  {
    if ( !( item->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile ) )
    {
      // add a refresh action, but ONLY if the collection item isn't representing a file
      // (if so, then QgsAppFileItemGuiProvider will add the Refresh item)
      QAction *actionRefresh = new QAction( QObject::tr( "Refresh" ), menu );
      connect( actionRefresh, &QAction::triggered, collectionItem, [collectionItem] { collectionItem->refresh(); } );
      if ( !menu->actions().empty() )
      {
        QAction *firstAction = menu->actions().at( 0 );
        menu->insertAction( firstAction, actionRefresh );
        menu->insertSeparator( firstAction );
      }
      else
      {
        menu->addAction( actionRefresh );
      }
    }

    menu->addSeparator();

    if ( QgsOgrDbConnection::connectionList( QStringLiteral( "GPKG" ) ).contains( collectionItem->name() ) )
    {
      QAction *actionDeleteConnection = new QAction( tr( "Remove Connection" ), menu );
      connect( actionDeleteConnection, &QAction::triggered, collectionItem, &QgsGeoPackageConnectionItem::deleteConnection );
      menu->addAction( actionDeleteConnection );
    }
    else
    {
      // Add to stored connections
      QAction *actionAddConnection = new QAction( tr( "Add Connection" ), menu );
      connect( actionAddConnection, &QAction::triggered, collectionItem, &QgsGeoPackageCollectionItem::addConnection );
      menu->addAction( actionAddConnection );
    }

    menu->addSeparator();
  }
}

bool QgsGeoPackageItemGuiProvider::rename( QgsDataItem *item, const QString &newName, QgsDataItemGuiContext context )
{
  if ( QgsGeoPackageVectorLayerItem *layerItem = qobject_cast< QgsGeoPackageVectorLayerItem * >( item ) )
  {
    // Checks that name does not exist yet
    if ( layerItem->tableNames().contains( newName ) )
    {
      return true;
    }
    // Check if the layer(s) are in the registry
    const QList<QgsMapLayer *> layersList( layerItem->layersInProject() );
    if ( ! layersList.isEmpty( ) )
    {
      if ( QMessageBox::question( nullptr, QObject::tr( "Rename Layer" ), QObject::tr( "The layer <b>%1</b> is loaded in the current project with name <b>%2</b>,"
                                  " do you want to remove it from the project and rename it?" ).arg( layerItem->name(), layersList.at( 0 )->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      {
        return true;
      }
    }
    if ( ! layersList.isEmpty() )
    {
      QgsProject::instance()->removeMapLayers( layersList );
    }

    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layerItem->providerKey(), layerItem->uri() );
    QString errCause;
    if ( parts.empty() || QgsVariantUtils::isNull( parts.value( QStringLiteral( "path" ) ) ) || QgsVariantUtils::isNull( parts.value( QStringLiteral( "layerName" ) ) ) )
    {
      errCause = QObject::tr( "Layer URI %1 is not valid!" ).arg( layerItem->uri() );
    }
    else
    {
      const QList<QgsMapLayer *> layersList( layerItem->layersInProject() );
      if ( ! layersList.isEmpty( ) )
      {
        if ( QMessageBox::question( nullptr, QObject::tr( "Rename Layer" ), QObject::tr( "The layer <b>%1</b> exists in the current project <b>%2</b>,"
                                    " do you want to remove it from the project and rename it?" ).arg( layerItem->name(), layersList.at( 0 )->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
        {
          return true;
        }
      }
      if ( ! layersList.isEmpty() )
      {
        QgsProject::instance()->removeMapLayers( layersList );
      }

      // Actually rename
      QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
      std::unique_ptr<QgsGeoPackageProviderConnection> conn( static_cast<QgsGeoPackageProviderConnection *>( md->createConnection( layerItem->collection()->path(), QVariantMap() ) ) );
      const QString oldName = parts.value( QStringLiteral( "layerName" ) ).toString();
      if ( ! conn->tableExists( QString(), oldName ) )
      {
        errCause = QObject::tr( "There was an error retrieving the connection %1!" ).arg( layerItem->collection()->name() );
      }
      else
      {
        try
        {
          conn->renameVectorTable( QString(), oldName, newName );
        }
        catch ( QgsProviderConnectionException &ex )
        {
          errCause = ex.what();
        }
      }
      if ( errCause.isEmpty() && context.messageBar() )
      {
        context.messageBar()->pushMessage( tr( "Rename Layer" ), tr( "The layer <b>%1</b> was successfully renamed." ).arg( oldName ), Qgis::MessageLevel::Success );
      }
    }

    if ( ! errCause.isEmpty() )
    {
      notify( QObject::tr( "Error renaming layer" ), errCause, context, Qgis::MessageLevel::Critical );
    }
    else if ( layerItem->parent() )
    {
      layerItem->parent()->refresh();
    }

    return errCause.isEmpty();
  }

  return false;
}

void QgsGeoPackageItemGuiProvider::renameVectorLayer( const QString &uri, const QString &key, const QStringList &tableNames,
    const QPointer< QgsDataItem > &item, QgsDataItemGuiContext context )
{
  // Get layer name from layer URI
  QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( key, uri ) );
  const QString layerName = pieces[QStringLiteral( "layerName" )].toString();

  QgsNewNameDialog dlg( uri, layerName, QStringList(), tableNames );

  // Allow any character, except |, which could create confusion, due to it being
  // the URI componenent separator. And ideally we should remove that restriction
  // by using proper escaping of |
  dlg.setRegularExpression( QStringLiteral( R"re([^|]+)re" ) );

  dlg.setOverwriteEnabled( false );

  if ( dlg.exec() != dlg.Accepted || dlg.name().isEmpty() || dlg.name() == layerName )
    return;

  rename( item, dlg.name(), context );
}


bool QgsGeoPackageItemGuiProvider::deleteLayer( QgsLayerItem *layerItem, QgsDataItemGuiContext context )
{
  if ( QgsGeoPackageAbstractLayerItem *item = qobject_cast< QgsGeoPackageAbstractLayerItem * >( layerItem ) )
  {
    // Check if the layer(s) are in the registry
    const QList<QgsMapLayer *> layersList( item->layersInProject( ) );
    if ( ! layersList.isEmpty( ) )
    {
      if ( QMessageBox::question( nullptr, QObject::tr( "Delete Layer" ), QObject::tr( "The layer <b>%1</b> exists in the current project <b>%2</b>,"
                                  " do you want to remove it from the project and delete it?" ).arg( item->name(),
                                      layersList.at( 0 )->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      {
        return false;
      }
    }
    else if ( QMessageBox::question( nullptr, QObject::tr( "Delete Layer" ),
                                     QObject::tr( "Are you sure you want to delete layer <b>%1</b> from GeoPackage?" ).arg( item->name() ),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      return false;
    }

    if ( ! layersList.isEmpty() )
    {
      QgsProject::instance()->removeMapLayers( layersList );
    }

    QString errCause;
    const bool res = item->executeDeleteLayer( errCause );
    if ( !res )
    {
      notify( tr( "Delete Layer" ), errCause, context, Qgis::MessageLevel::Critical );
    }
    else
    {

      QgsGeoPackageConnectionItem *connectionParentItem = qobject_cast<QgsGeoPackageConnectionItem *>( item->parent() );
      if ( connectionParentItem )
      {
        if ( QMessageBox::question( nullptr, QObject::tr( "Delete Layer" ), QObject::tr( "The layer <b>%1</b> was successfully deleted."
                                    " Compact database (VACUUM) <b>%2</b> now?" ).arg( item->name(), connectionParentItem->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
        {
          vacuumGeoPackageDbAction( connectionParentItem->path(), connectionParentItem->name(), context );
        }
      }
      else
      {
        notify( tr( "Delete Layer" ), tr( "The layer <b>%1</b> was successfully deleted." ).arg( item->name() ), context, Qgis::MessageLevel::Success );
      }
      if ( item->parent() )
      {
        item->parent()->refresh();
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}

void QgsGeoPackageItemGuiProvider::vacuumGeoPackageDbAction( const QString &path, const QString &name, QgsDataItemGuiContext context )
{
  Q_UNUSED( path )
  QString errCause;
  const bool result = QgsGeoPackageCollectionItem::vacuumGeoPackageDb( name, path, errCause );
  if ( !result || !errCause.isEmpty() )
  {
    notify( tr( "Database compact (VACUUM)" ), errCause, context, Qgis::MessageLevel::Critical );
  }
  else if ( context.messageBar() )
  {
    context.messageBar()->pushMessage( tr( "Database compacted" ), Qgis::MessageLevel::Success );
  }
}

void QgsGeoPackageItemGuiProvider::createDatabase( const QPointer< QgsGeoPackageRootItem > &item )
{
  if ( item )
  {
    QgsNewGeoPackageLayerDialog dialog( nullptr );
    dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
    if ( dialog.exec() == QDialog::Accepted )
    {
      // Call QFileInfo to normalize paths, see: https://github.com/qgis/QGIS/issues/36832
      if ( QgsOgrProviderUtils::saveConnection( QFileInfo( dialog.databasePath() ).filePath(), QStringLiteral( "GPKG" ) ) )
      {
        item->refreshConnections();
      }
    }
  }
}

bool QgsGeoPackageItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
  {
    return true;
  }
  return false;
}

bool QgsGeoPackageItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction )
{
  if ( QgsGeoPackageCollectionItem *collectionItem = qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
  {
    return handleDropGeopackage( collectionItem, data, context );
  }
  return false;
}

bool QgsGeoPackageItemGuiProvider::handleDropGeopackage( QgsGeoPackageCollectionItem *item, const QMimeData *data, QgsDataItemGuiContext context )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  QString uri;

  QStringList importResults;
  bool hasError = false;

  // Main task
  std::unique_ptr< QgsTaskWithSerialSubTasks > mainTask( new QgsTaskWithSerialSubTasks( tr( "GeoPackage import" ) ) );
  bool hasSubTasks = false;

  const auto lst = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &dropUri : lst )
  {
    // Check that we are not copying over self
    if ( dropUri.uri.startsWith( item->path() ) )
    {
      importResults.append( tr( "You cannot import layer %1 over itself!" ).arg( dropUri.name ) );
      hasError = true;
    }
    // Neither we should support copying the whole GPKG file
    else if ( dropUri.providerKey.isEmpty() && dropUri.uri == dropUri.filePath )
    {
      importResults.append( tr( "You cannot import a GeoPackage file over another GeoPackage file!" ) );
      hasError = true;
    }
    else
    {
      QgsMapLayer *srcLayer = nullptr;
      bool owner;
      bool isVector = false;
      QString error;
      // Common checks for raster and vector
      // aspatial is treated like vector
      if ( dropUri.layerType == QLatin1String( "vector" ) )
      {
        // open the source layer
        srcLayer = dropUri.vectorLayer( owner, error );
        isVector = true;
      }
      else if ( dropUri.layerType == QLatin1String( "mesh" ) )
      {
        // unsupported
        hasError = true;
        continue;
      }
      else
      {
        srcLayer = dropUri.rasterLayer( owner, error );
      }
      if ( !srcLayer )
      {
        importResults.append( tr( "%1: %2" ).arg( dropUri.name, error ) );
        hasError = true;
        continue;
      }

      if ( srcLayer->isValid() )
      {
        uri = item->path().remove( QStringLiteral( "gpkg:/" ) );
        QgsDebugMsgLevel( "URI " + uri, 3 );

        // check if the destination layer already exists
        bool exists = false;
        const auto c( item->children() );
        for ( const QgsDataItem *child : c )
        {
          if ( child->name() == dropUri.name )
          {
            exists = true;
          }
        }

        if ( exists && !isVector )
        {
          notify( tr( "Cannot Overwrite Layer" ),
                  tr( "Destination layer <b>%1</b> already exists. Overwriting with raster layers is not currently supported." ).arg( dropUri.name ), context, Qgis::MessageLevel::Critical );
        }
        else if ( ! exists || QMessageBox::question( nullptr, tr( "Overwrite Layer" ),
                  tr( "Destination layer <b>%1</b> already exists. Do you want to overwrite it?" ).arg( dropUri.name ), QMessageBox::Yes |  QMessageBox::No ) == QMessageBox::Yes )
        {
          if ( isVector ) // Import vectors and aspatial
          {
            QgsVectorLayer *vectorSrcLayer = qobject_cast < QgsVectorLayer * >( srcLayer );
            QVariantMap options;
            options.insert( QStringLiteral( "driverName" ), QStringLiteral( "GPKG" ) );
            options.insert( QStringLiteral( "update" ), true );
            options.insert( QStringLiteral( "overwrite" ), true );
            options.insert( QStringLiteral( "layerName" ), dropUri.name );
            QgsVectorLayerExporterTask *exportTask = new QgsVectorLayerExporterTask( vectorSrcLayer, uri, QStringLiteral( "ogr" ), vectorSrcLayer->crs(), options, owner );
            mainTask->addSubTask( exportTask );
            hasSubTasks = true;
            // when export is successful:
            connect( exportTask, &QgsVectorLayerExporterTask::exportComplete, item, [ = ]()
            {
              notify( tr( "Import to GeoPackage database" ), tr( "Import was successful." ), context, Qgis::MessageLevel::Success );
              item->refresh();
            } );

            // when an error occurs:
            connect( exportTask, &QgsVectorLayerExporterTask::errorOccurred, item, [ = ]( Qgis::VectorExportResult error, const QString & errorMessage )
            {
              if ( error != Qgis::VectorExportResult::UserCanceled )
              {
                QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
                output->setTitle( tr( "Import to GeoPackage database" ) );
                output->setMessage( tr( "Failed to import some vector layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
                output->showMessage();
              }
            } );

          }
          else  // Import raster
          {
            QgsGeoPackageRasterWriterTask  *exportTask = new QgsGeoPackageRasterWriterTask( dropUri, item->path() );
            mainTask->addSubTask( exportTask );
            hasSubTasks = true;
            // when export is successful:
            connect( exportTask, &QgsGeoPackageRasterWriterTask::writeComplete, item, [ = ]()
            {
              notify( tr( "Import to GeoPackage database" ), tr( "Import was successful." ), context, Qgis::MessageLevel::Success );
              item->refresh();
            } );

            // when an error occurs:
            connect( exportTask, &QgsGeoPackageRasterWriterTask::errorOccurred, item, [ = ]( QgsGeoPackageRasterWriter::WriterError error, const QString & errorMessage )
            {
              if ( error != QgsGeoPackageRasterWriter::WriterError::ErrUserCanceled )
              {
                QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
                output->setTitle( tr( "Import to GeoPackage database" ) );
                output->setMessage( tr( "Failed to import some raster layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
                output->showMessage();
              }
              // Always try to delete the imported raster, in case the gpkg has been left
              // in an inconsistent status. Ignore delete errors.
              QString deleteErr;
              item->deleteRasterLayer( dropUri.name, deleteErr );
            } );

          }
        } // do not overwrite
      }
      else
      {
        importResults.append( tr( "%1: Not a valid layer!" ).arg( dropUri.name ) );
        hasError = true;
      }
    } // check for self copy
  } // for each

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to GeoPackage database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }
  if ( hasSubTasks )
  {
    QgsApplication::taskManager()->addTask( mainTask.release() );
  }
  return true;
}

///@endcond
