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
#include "qgsogrdataitems.h"
#include "qgsogrdbconnection.h"
#include "qgsgeopackageproviderconnection.h"

void QgsGeoPackageItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu,
    const QList<QgsDataItem *> &,
    QgsDataItemGuiContext context )
{
  if ( QgsGeoPackageVectorLayerItem *layerItem = qobject_cast< QgsGeoPackageVectorLayerItem * >( item ) )
  {
    // Check capabilities
    if ( layerItem->capabilities2() & QgsDataItem::Capability::Rename )
    {
      QAction *actionRenameLayer = new QAction( tr( "Rename Layer '%1'…" ).arg( layerItem->name() ), this );
      QVariantMap data;
      data.insert( QStringLiteral( "uri" ), layerItem->uri() );
      data.insert( QStringLiteral( "key" ), layerItem->providerKey() );
      data.insert( QStringLiteral( "tableNames" ), layerItem->tableNames() );
      data.insert( QStringLiteral( "item" ), QVariant::fromValue( QPointer< QgsDataItem >( layerItem ) ) );
      data.insert( QStringLiteral( "context" ), QVariant::fromValue( context ) );
      actionRenameLayer->setData( data );
      connect( actionRenameLayer, &QAction::triggered, this, &QgsGeoPackageItemGuiProvider::renameVectorLayer );
      menu->addAction( actionRenameLayer );
    }
  }

  if ( QgsGeoPackageRootItem *rootItem = qobject_cast< QgsGeoPackageRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), rootItem->parent() );
    connect( actionNew, &QAction::triggered, rootItem, &QgsGeoPackageRootItem::newConnection );
    menu->addAction( actionNew );

    QAction *actionCreateDatabase = new QAction( tr( "Create Database…" ), rootItem->parent() );
    QVariantMap data;
    data.insert( QStringLiteral( "item" ), QVariant::fromValue( QPointer< QgsGeoPackageRootItem >( rootItem ) ) );
    actionCreateDatabase->setData( data );
    connect( actionCreateDatabase, &QAction::triggered, this, &QgsGeoPackageItemGuiProvider::createDatabase );
    menu->addAction( actionCreateDatabase );
  }

  if ( QgsGeoPackageCollectionItem *collectionItem = qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
  {
    if ( QgsOgrDbConnection::connectionList( QStringLiteral( "GPKG" ) ).contains( collectionItem->name() ) )
    {
      QAction *actionDeleteConnection = new QAction( tr( "Remove Connection" ), collectionItem->parent() );
      connect( actionDeleteConnection, &QAction::triggered, collectionItem, &QgsGeoPackageConnectionItem::deleteConnection );
      menu->addAction( actionDeleteConnection );
    }
    else
    {
      // Add to stored connections
      QAction *actionAddConnection = new QAction( tr( "Add Connection" ), collectionItem->parent() );
      connect( actionAddConnection, &QAction::triggered, collectionItem, &QgsGeoPackageCollectionItem::addConnection );
      menu->addAction( actionAddConnection );
    }

    // Add table to existing DB
    QAction *actionAddTable = new QAction( tr( "Create a New Layer or Table…" ), collectionItem->parent() );
    QPointer<QgsGeoPackageCollectionItem>collectionItemPtr { collectionItem };
    const QString itemPath = collectionItem->path();
    connect( actionAddTable, &QAction::triggered, actionAddTable, [ collectionItemPtr, itemPath ]
    {
      QgsNewGeoPackageLayerDialog dialog( nullptr );
      dialog.setDatabasePath( itemPath );
      dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
      dialog.setOverwriteBehavior( QgsNewGeoPackageLayerDialog::AddNewLayer );
      dialog.lockDatabasePath();
      if ( dialog.exec() == QDialog::Accepted )
      {
        if ( collectionItemPtr )
          collectionItemPtr->refreshConnections();
      }
    } );

    menu->addAction( actionAddTable );

    QAction *sep = new QAction( collectionItem->parent() );
    sep->setSeparator( true );
    menu->addAction( sep );

    QString message = QObject::tr( "Delete %1…" ).arg( collectionItem->name() );
    QAction *actionDelete = new QAction( message, collectionItem->parent() );
    QVariantMap dataDelete;
    dataDelete.insert( QStringLiteral( "path" ), collectionItem->path() );
    dataDelete.insert( QStringLiteral( "parent" ), QVariant::fromValue( QPointer< QgsDataItem >( collectionItem->parent() ) ) );
    actionAddTable->setData( dataDelete );
    connect( actionDelete, &QAction::triggered, this, &QgsGeoPackageItemGuiProvider::deleteGpkg );
    menu->addAction( actionDelete );

    // Run VACUUM
    QAction *actionVacuumDb = new QAction( tr( "Compact Database (VACUUM)" ), collectionItem->parent() );
    QVariantMap dataVacuum;
    dataVacuum.insert( QStringLiteral( "name" ), collectionItem->name() );
    dataVacuum.insert( QStringLiteral( "path" ), collectionItem->path() );
    actionVacuumDb->setData( dataVacuum );
    connect( actionVacuumDb, &QAction::triggered, this, &QgsGeoPackageItemGuiProvider::vacuum );
    menu->addAction( actionVacuumDb );
  }
}

void QgsGeoPackageItemGuiProvider::deleteGpkg()
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  const QString path = data[QStringLiteral( "path" )].toString();
  QPointer< QgsDataItem > parent = data[QStringLiteral( "parent" )].value<QPointer< QgsDataItem >>();
  if ( parent )
  {
    const QString title = QObject::tr( "Delete GeoPackage" );
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

      if ( !QFile::remove( path ) )
      {
        QMessageBox::warning( nullptr, title, tr( "Could not delete GeoPackage." ) );
      }
      else
      {
        QMessageBox::information( nullptr, title, tr( "GeoPackage deleted successfully." ) );
        if ( parent )
          parent->refresh();
      }
    }
    else
    {
      QMessageBox::warning( nullptr, title, QObject::tr( "The GeoPackage '%1' cannot be deleted because it is in the current project as '%2',"
                            " remove it from the project and retry." ).arg( path, projectLayer->name() ) );
    }
  }
}

bool QgsGeoPackageItemGuiProvider::rename( QgsDataItem *item, const QString &newName, QgsDataItemGuiContext )
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
    if ( parts.empty() || parts.value( QStringLiteral( "path" ) ).isNull() || parts.value( QStringLiteral( "layerName" ) ).isNull() )
    {
      errCause = QObject::tr( "Layer URI %1 is not valid!" ).arg( layerItem->uri() );
    }
    else
    {
      QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
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
      QString oldName = parts.value( QStringLiteral( "layerName" ) ).toString();
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
    }

    if ( ! errCause.isEmpty() )
      QMessageBox::critical( nullptr, QObject::tr( "Error renaming layer" ), errCause );
    else if ( layerItem->parent() )
      layerItem->parent()->refreshConnections();

    return errCause.isEmpty();
  }

  return false;
}

void QgsGeoPackageItemGuiProvider::renameVectorLayer()
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  const QString uri = data[QStringLiteral( "uri" )].toString();
  const QString key = data[QStringLiteral( "key" )].toString();
  const QStringList tableNames = data[QStringLiteral( "tableNames" )].toStringList();
  QPointer< QgsDataItem > item = data[QStringLiteral( "item" )].value<QPointer< QgsDataItem >>();
  QgsDataItemGuiContext context = data[QStringLiteral( "context" )].value< QgsDataItemGuiContext >();

  // Get layer name from layer URI
  QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( key, uri ) );
  QString layerName = pieces[QStringLiteral( "layerName" )].toString();

  // Collect existing table names
  const QRegExp checkRe( QStringLiteral( R"re([A-Za-z_][A-Za-z0-9_\s]+)re" ) );
  QgsNewNameDialog dlg( uri, layerName, QStringList(), tableNames, checkRe );
  dlg.setOverwriteEnabled( false );

  if ( dlg.exec() != dlg.Accepted || dlg.name().isEmpty() || dlg.name() == layerName )
    return;

  rename( item, dlg.name(), context );
}


bool QgsGeoPackageItemGuiProvider::deleteLayer( QgsLayerItem *layerItem, QgsDataItemGuiContext )
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
    bool res = item->executeDeleteLayer( errCause );
    if ( !res )
    {
      QMessageBox::warning( nullptr, tr( "Delete Layer" ), errCause );
    }
    else
    {

      QgsGeoPackageConnectionItem *connectionParentItem = qobject_cast<QgsGeoPackageConnectionItem *>( item->parent() );
      if ( connectionParentItem )
      {
        if ( QMessageBox::question( nullptr, QObject::tr( "Delete Layer" ), QObject::tr( "The layer <b>%1</b> was successfully deleted."
                                    " Compact database (VACUUM) <b>%2</b> now?" ).arg( item->name(), connectionParentItem->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
        {
          vacuumGeoPackageDbAction( connectionParentItem->path(), connectionParentItem->name() );
        }
      }
      else
      {
        QMessageBox::information( nullptr, tr( "Delete Layer" ), tr( "The layer <b>%1</b> was successfully deleted." ).arg( item->name() ) );
      }
      if ( item->parent() )
        item->parent()->refreshConnections();

    }
    return true;
  }
  else
  {
    return false;
  }
}

void QgsGeoPackageItemGuiProvider::vacuumGeoPackageDbAction( const QString &path, const QString &name )
{
  Q_UNUSED( path )
  QString errCause;
  bool result = QgsGeoPackageCollectionItem::vacuumGeoPackageDb( name, path, errCause );
  if ( !result || !errCause.isEmpty() )
  {
    QMessageBox::warning( nullptr, tr( "Database compact (VACUUM)" ), errCause );
  }
}

void QgsGeoPackageItemGuiProvider::vacuum()
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  const QString path = data[QStringLiteral( "path" )].toString();
  const QString name = data[QStringLiteral( "name" )].toString();
  vacuumGeoPackageDbAction( path, name );
}

void QgsGeoPackageItemGuiProvider::createDatabase()
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  QPointer< QgsGeoPackageRootItem > item = data[QStringLiteral( "item" )].value<QPointer< QgsGeoPackageRootItem >>();
  if ( item )
  {
    QgsNewGeoPackageLayerDialog dialog( nullptr );
    dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
    if ( dialog.exec() == QDialog::Accepted )
    {
      if ( QgsOgrDataCollectionItem::saveConnection( dialog.databasePath(), QStringLiteral( "GPKG" ) ) )
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

bool QgsGeoPackageItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data, Qt::DropAction )
{
  if ( QgsGeoPackageCollectionItem *collectionItem = qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
  {
    return handleDropGeopackage( collectionItem, data );
  }
  return false;
}

bool QgsGeoPackageItemGuiProvider::handleDropGeopackage( QgsGeoPackageCollectionItem *item, const QMimeData *data )
{

  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  QString uri;

  QStringList importResults;
  bool hasError = false;

  // Main task
  std::unique_ptr< QgsConcurrentFileWriterImportTask > mainTask( new QgsConcurrentFileWriterImportTask( tr( "GeoPackage import" ) ) );
  QgsTaskList importTasks;

  const auto lst = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &dropUri : lst )
  {
    // Check that we are not copying over self
    if ( dropUri.uri.startsWith( item->path() ) )
    {
      importResults.append( tr( "You cannot import layer %1 over itself!" ).arg( dropUri.name ) );
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
      if ( dropUri.layerType == QStringLiteral( "vector" ) )
      {
        // open the source layer
        srcLayer = dropUri.vectorLayer( owner, error );
        isVector = true;
      }
      else if ( dropUri.layerType == QStringLiteral( "mesh" ) )
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
        uri = item->path();
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
          QMessageBox::warning( nullptr, tr( "Cannot Overwrite Layer" ),
                                tr( "Destination layer <b>%1</b> already exists. Overwriting with raster layers is not currently supported." ).arg( dropUri.name ) );
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
            mainTask->addSubTask( exportTask, importTasks );
            importTasks << exportTask;
            // when export is successful:
            connect( exportTask, &QgsVectorLayerExporterTask::exportComplete, item, [ = ]()
            {
              // this is gross - TODO - find a way to get access to messageBar from data items
              QMessageBox::information( nullptr, tr( "Import to GeoPackage database" ), tr( "Import was successful." ) );
              item->refreshConnections();
            } );

            // when an error occurs:
            connect( exportTask, &QgsVectorLayerExporterTask::errorOccurred, item, [ = ]( int error, const QString & errorMessage )
            {
              if ( error != QgsVectorLayerExporter::ErrUserCanceled )
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
            mainTask->addSubTask( exportTask, importTasks );
            importTasks << exportTask;
            // when export is successful:
            connect( exportTask, &QgsGeoPackageRasterWriterTask::writeComplete, item, [ = ]()
            {
              // this is gross - TODO - find a way to get access to messageBar from data items
              QMessageBox::information( nullptr, tr( "Import to GeoPackage database" ), tr( "Import was successful." ) );
              item->refreshConnections();
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
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }
  if ( ! importTasks.isEmpty() )
  {
    QgsApplication::taskManager()->addTask( mainTask.release() );
  }
  return true;
}

///@endcond
