/***************************************************************************
    qgsgeopackagedataitems.h
    ---------------------
    begin                : August 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include <sqlite3.h>

#include "qgssqliteutils.h"
#include "qgsgeopackagedataitems.h"
#include "qgsogrdbconnection.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsogrprovider.h"
#include "qgsogrdataitems.h"
#ifdef HAVE_GUI
#include "qgsnewnamedialog.h"
#include "qgsnewgeopackagelayerdialog.h"
#endif
#include "qgsmessageoutput.h"
#include "qgsvectorlayerexporter.h"
#include "qgsgeopackagerasterwritertask.h"
#include "qgstaskmanager.h"
#include "qgsproviderregistry.h"
#include "qgsproxyprogresstask.h"


QGISEXTERN bool deleteLayer( const QString &uri, const QString &errCause );

QgsDataItem *QgsGeoPackageDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsg( "path = " + path );
  if ( path.isEmpty() )
  {
    return new QgsGeoPackageRootItem( parentItem, QStringLiteral( "GeoPackage" ), QStringLiteral( "gpkg:" ) );
  }
  return nullptr;
}

QgsGeoPackageRootItem::QgsGeoPackageRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mGeoPackage.svg" );
  populate();
}

QVector<QgsDataItem *> QgsGeoPackageRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList connList( QgsOgrDbConnection::connectionList( QStringLiteral( "GPKG" ) ) );
  for ( const QString &connName : connList )
  {
    QgsOgrDbConnection connection( connName, QStringLiteral( "GPKG" ) );
    QgsDataItem *conn = new QgsGeoPackageConnectionItem( this, connection.name(), connection.path() );

    connections.append( conn );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsGeoPackageAbstractLayerItem::actions( QWidget * )
{
  QList<QAction *> lst;
  // Check capabilities: for now rename is only available for vectors
  if ( capabilities2() & QgsDataItem::Capability::Rename )
  {
    QAction *actionRenameLayer = new QAction( tr( "Rename Layer '%1'…" ).arg( mName ), this );
    connect( actionRenameLayer, &QAction::triggered, this, &QgsGeoPackageAbstractLayerItem::renameLayer );
    lst.append( actionRenameLayer );
  }
  return lst;
}

void QgsGeoPackageRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsGeoPackageRootItem::newConnection()
{
  if ( QgsOgrDataCollectionItem::createConnection( QStringLiteral( "GeoPackage" ),  QStringLiteral( "GeoPackage Database (*.gpkg)" ),  QStringLiteral( "GPKG" ) ) )
  {
    refreshConnections();
  }
}

void QgsGeoPackageRootItem::createDatabase()
{
  QgsNewGeoPackageLayerDialog dialog( nullptr );
  dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
  if ( dialog.exec() == QDialog::Accepted )
  {
    if ( QgsOgrDataCollectionItem::storeConnection( dialog.databasePath(), QStringLiteral( "GPKG" ) ) )
    {
      refreshConnections();
    }
  }
}
#endif

QgsGeoPackageCollectionItem::QgsGeoPackageCollectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
  , mPath( path )
{
  mToolTip = path;
  mCapabilities |= Collapse;
}


QVector<QgsDataItem *> QgsGeoPackageCollectionItem::createChildren()
{
  QVector<QgsDataItem *> children;
  const auto layers = QgsOgrLayerItem::subLayers( mPath, QStringLiteral( "GPKG" ) );
  for ( const QgsOgrDbLayerInfo *info : layers )
  {
    if ( info->layerType() == QgsLayerItem::LayerType::Raster )
    {
      children.append( new QgsGeoPackageRasterLayerItem( this, info->name(), info->path(), info->uri() ) );
    }
    else
    {
      children.append( new QgsGeoPackageVectorLayerItem( this, info->name(), info->path(), info->uri(), info->layerType( ) ) );
    }
  }
  qDeleteAll( layers );
  return children;
}

bool QgsGeoPackageCollectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsGeoPackageCollectionItem *o = qobject_cast<const QgsGeoPackageCollectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;

}

#ifdef HAVE_GUI
QList<QAction *> QgsGeoPackageRootItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection…" ), parent );
  connect( actionNew, &QAction::triggered, this, &QgsGeoPackageRootItem::newConnection );
  lst.append( actionNew );

  QAction *actionCreateDatabase = new QAction( tr( "Create Database…" ), parent );
  connect( actionCreateDatabase, &QAction::triggered, this, &QgsGeoPackageRootItem::createDatabase );
  lst.append( actionCreateDatabase );

  return lst;
}

QWidget *QgsGeoPackageRootItem::paramWidget()
{
  return nullptr;
}

QList<QAction *> QgsGeoPackageCollectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst = QgsDataCollectionItem::actions( parent );

  if ( QgsOgrDbConnection::connectionList( QStringLiteral( "GPKG" ) ).contains( mName ) )
  {
    QAction *actionDeleteConnection = new QAction( tr( "Remove Connection" ), parent );
    connect( actionDeleteConnection, &QAction::triggered, this, &QgsGeoPackageConnectionItem::deleteConnection );
    lst.append( actionDeleteConnection );
  }
  else
  {
    // Add to stored connections
    QAction *actionAddConnection = new QAction( tr( "Add Connection" ), parent );
    connect( actionAddConnection, &QAction::triggered, this, &QgsGeoPackageCollectionItem::addConnection );
    lst.append( actionAddConnection );
  }

  // Add table to existing DB
  QAction *actionAddTable = new QAction( tr( "Create a New Layer or Table…" ), parent );
  connect( actionAddTable, &QAction::triggered, this, &QgsGeoPackageCollectionItem::addTable );
  lst.append( actionAddTable );

  QAction *sep = new QAction( parent );
  sep->setSeparator( true );
  lst.append( sep );

  QString message = QObject::tr( "Delete %1…" ).arg( mName );
  QAction *actionDelete = new QAction( message, parent );

  // IMPORTANT - we need to capture the stuff we need, and then hand the slot
  // off to a static method. This is because it's possible for this item
  // to be deleted in the background on us (e.g. by a parent directory refresh)
  const QString path = mPath;
  QPointer< QgsDataItem > parentItem( mParent );
  connect( actionDelete, &QAction::triggered, this, [ path, parentItem ]
  {
    deleteGpkg( path, parentItem );
  } );
  lst.append( actionDelete );

  // Run VACUUM
  QAction *actionVacuumDb = new QAction( tr( "Compact Database (VACUUM)" ), parent );
  connect( actionVacuumDb, &QAction::triggered, this, &QgsGeoPackageConnectionItem::vacuumGeoPackageDbAction );
  lst.append( actionVacuumDb );


  return lst;
}

void QgsGeoPackageCollectionItem::deleteGpkg( const QString &path, QPointer<QgsDataItem> parent )
{
  const QString title = QObject::tr( "Delete GeoPackage" );
  // Check if the layer is in the project
  const QgsMapLayer *projectLayer = nullptr;
  const auto mapLayers = QgsProject::instance()->mapLayers();
  for ( auto it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( it.value()->dataProvider()->name(), it.value()->source() );
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

bool QgsGeoPackageCollectionItem::handleDrop( const QMimeData *data, Qt::DropAction )
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
    if ( dropUri.uri.startsWith( mPath ) )
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
        uri = mPath;
        QgsDebugMsgLevel( "URI " + uri, 3 );

        // check if the destination layer already exists
        bool exists = false;
        const auto c( children() );
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
            options.insert( QStringLiteral( "forceSinglePartGeometryType" ), true );
            QgsVectorLayerExporterTask *exportTask = new QgsVectorLayerExporterTask( vectorSrcLayer, uri, QStringLiteral( "ogr" ), vectorSrcLayer->crs(), options, owner );
            mainTask->addSubTask( exportTask, importTasks );
            importTasks << exportTask;
            // when export is successful:
            connect( exportTask, &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
            {
              // this is gross - TODO - find a way to get access to messageBar from data items
              QMessageBox::information( nullptr, tr( "Import to GeoPackage database" ), tr( "Import was successful." ) );
              refreshConnections();
            } );

            // when an error occurs:
            connect( exportTask, &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( int error, const QString & errorMessage )
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
            QgsGeoPackageRasterWriterTask  *exportTask = new QgsGeoPackageRasterWriterTask( dropUri, mPath );
            mainTask->addSubTask( exportTask, importTasks );
            importTasks << exportTask;
            // when export is successful:
            connect( exportTask, &QgsGeoPackageRasterWriterTask::writeComplete, this, [ = ]()
            {
              // this is gross - TODO - find a way to get access to messageBar from data items
              QMessageBox::information( nullptr, tr( "Import to GeoPackage database" ), tr( "Import was successful." ) );
              refreshConnections();
            } );

            // when an error occurs:
            connect( exportTask, &QgsGeoPackageRasterWriterTask::errorOccurred, this, [ = ]( QgsGeoPackageRasterWriter::WriterError error, const QString & errorMessage )
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
              deleteGeoPackageRasterLayer( QStringLiteral( "GPKG:%1:%2" ).arg( mPath, dropUri.name ), deleteErr );
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

QList<QAction *> QgsGeoPackageConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionDeleteConnection = new QAction( tr( "Remove Connection" ), parent );
  connect( actionDeleteConnection, &QAction::triggered, this, &QgsGeoPackageConnectionItem::deleteConnection );
  lst.append( actionDeleteConnection );

  // Add table to existing DB
  QAction *actionAddTable = new QAction( tr( "Create a New Layer or Table…" ), parent );
  connect( actionAddTable, &QAction::triggered, this, &QgsGeoPackageConnectionItem::addTable );
  lst.append( actionAddTable );

  // Run VACUUM
  QAction *actionVacuumDb = new QAction( tr( "Compact Database (VACUUM)" ), parent );
  connect( actionVacuumDb, &QAction::triggered, this, &QgsGeoPackageConnectionItem::vacuumGeoPackageDbAction );
  lst.append( actionVacuumDb );

  return lst;
}

void QgsGeoPackageCollectionItem::deleteConnection()
{
  QgsOgrDbConnection::deleteConnection( name(), QStringLiteral( "GPKG" ) );
  mParent->refreshConnections();
}

void QgsGeoPackageCollectionItem::addTable()
{
  QgsNewGeoPackageLayerDialog dialog( nullptr );
  dialog.setDatabasePath( mPath );
  dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
  dialog.setOverwriteBehavior( QgsNewGeoPackageLayerDialog::AddNewLayer );
  dialog.lockDatabasePath();
  if ( dialog.exec() == QDialog::Accepted )
  {
    refreshConnections();
  }
}

void QgsGeoPackageCollectionItem::addConnection()
{
  QgsOgrDbConnection connection( mName, QStringLiteral( "GPKG" ) );
  connection.setPath( mPath );
  connection.save();
  mParent->refreshConnections();
}

void QgsGeoPackageCollectionItem::vacuumGeoPackageDbAction()
{
  QString errCause;
  bool result = QgsGeoPackageCollectionItem::vacuumGeoPackageDb( mPath, mName, errCause );
  if ( !result || !errCause.isEmpty() )
  {
    QMessageBox::warning( nullptr, tr( "Database compact (VACUUM)" ), errCause );
  }
}

bool QgsGeoPackageAbstractLayerItem::deleteLayer()
{
  // Check if the layer(s) are in the registry
  const QList<QgsMapLayer *> layersList( layersInProject( ) );
  if ( ! layersList.isEmpty( ) )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Layer" ), QObject::tr( "The layer <b>%1</b> exists in the current project <b>%2</b>,"
                                " do you want to remove it from the project and delete it?" ).arg( mName, layersList.at( 0 )->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      return true;
    }
  }
  else if ( QMessageBox::question( nullptr, QObject::tr( "Delete Layer" ),
                                   QObject::tr( "Are you sure you want to delete layer <b>%1</b> from GeoPackage?" ).arg( mName ),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
  {
    return true;
  }

  if ( ! layersList.isEmpty() )
  {
    QgsProject::instance()->removeMapLayers( layersList );
  }

  QString errCause;
  bool res = executeDeleteLayer( errCause );
  if ( !res )
  {
    QMessageBox::warning( nullptr, tr( "Delete Layer" ), errCause );
  }
  else
  {

    QgsGeoPackageConnectionItem *connectionParentItem = qobject_cast<QgsGeoPackageConnectionItem *>( mParent );
    if ( connectionParentItem )
    {
      if ( QMessageBox::question( nullptr, QObject::tr( "Delete Layer" ), QObject::tr( "The layer <b>%1</b> was successfully deleted."
                                  " Compact database (VACUUM) <b>%2</b> now?" ).arg( mName, connectionParentItem->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
      {
        connectionParentItem->vacuumGeoPackageDbAction();
      }
    }
    else
    {
      QMessageBox::information( nullptr, tr( "Delete Layer" ), tr( "The layer <b>%1</b> was successfully deleted." ).arg( mName ) );
    }
    if ( mParent )
      mParent->refreshConnections();

  }
  return true;
}

void QgsGeoPackageAbstractLayerItem::renameLayer( )
{
  QMessageBox::warning( nullptr, QObject::tr( "Rename layer" ),
                        QObject::tr( "The layer <b>%1</b> cannot be renamed because this feature is not yet implemented for this kind of layers." )
                        .arg( mName ) );
}

void QgsGeoPackageVectorLayerItem::renameLayer()
{

  // Get layer name from layer URI
  QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( providerKey(), mUri ) );
  QString layerName = pieces[QStringLiteral( "layerName" )].toString();

  // Collect existing table names
  const QRegExp checkRe( QStringLiteral( R"re([A-Za-z_][A-Za-z0-9_\s]+)re" ) );
  QgsNewNameDialog dlg( mUri, layerName, QStringList(), tableNames(), checkRe );
  dlg.setOverwriteEnabled( false );

  if ( dlg.exec() != dlg.Accepted || dlg.name().isEmpty() || dlg.name() == layerName )
    return;

  rename( dlg.name() );
}
#endif

bool QgsGeoPackageCollectionItem::vacuumGeoPackageDb( const QString &path, const QString &name, QString &errCause )
{
  QgsScopedProxyProgressTask task( tr( "Vacuuming %1" ).arg( name ) );

  bool result = false;
  // Better safe than sorry
  if ( ! path.isEmpty( ) )
  {
    char *errmsg = nullptr;
    sqlite3_database_unique_ptr database;
    int status = database.open_v2( path, SQLITE_OPEN_READWRITE, nullptr );
    if ( status != SQLITE_OK )
    {
      errCause = sqlite3_errmsg( database.get() );
    }
    else
    {
      ( void )sqlite3_exec(
        database.get(),                      /* An open database */
        "VACUUM",                            /* SQL to be evaluated */
        nullptr,                             /* Callback function */
        nullptr,                             /* 1st argument to callback */
        &errmsg                              /* Error msg written here */
      );
    }
    if ( status != SQLITE_OK || errmsg )
    {
      errCause = tr( "There was an error compacting (VACUUM) the database <b>%1</b>: %2" )
                 .arg( name,
                       QString::fromUtf8( errmsg ) );
    }
    else
    {
      result = true;
    }
    sqlite3_free( errmsg );
  }
  else
  {
    // This should never happen!
    errCause = tr( "Layer path is empty: layer cannot be deleted!" );
  }
  return result;
}

bool QgsGeoPackageCollectionItem::deleteGeoPackageRasterLayer( const QString &uri, QString &errCause )
{
  bool result = false;
  // Better safe than sorry
  if ( ! uri.isEmpty( ) )
  {
    QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri ) );
    QString baseUri = pieces[QStringLiteral( "path" )].toString();
    QString layerName = pieces[QStringLiteral( "layerName" )].toString();

    if ( baseUri.isEmpty() || layerName.isEmpty() )
    {
      errCause = QStringLiteral( "Layer URI is malformed: layer <b>%1</b> cannot be deleted!" ).arg( uri );
    }
    else
    {
      sqlite3_database_unique_ptr database;
      int status = database.open_v2( baseUri, SQLITE_OPEN_READWRITE, nullptr );
      if ( status != SQLITE_OK )
      {
        errCause = sqlite3_errmsg( database.get() );
      }
      else
      {
        // Remove table
        char *errmsg = nullptr;
        char *sql = sqlite3_mprintf(
                      "DROP table IF EXISTS \"%w\";"
                      "DELETE FROM gpkg_contents WHERE table_name = '%q';"
                      "DELETE FROM gpkg_tile_matrix WHERE table_name = '%q';"
                      "DELETE FROM gpkg_tile_matrix_set WHERE table_name = '%q';",
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData() );
        status = sqlite3_exec(
                   database.get(),               /* An open database */
                   sql,                          /* SQL to be evaluated */
                   nullptr,                      /* Callback function */
                   nullptr,                      /* 1st argument to callback */
                   &errmsg                       /* Error msg written here */
                 );
        sqlite3_free( sql );
        // Remove from optional tables, may silently fail
        QStringList optionalTables;
        optionalTables << QStringLiteral( "gpkg_extensions" )
                       << QStringLiteral( "gpkg_metadata_reference" );
        for ( const QString &tableName : qgis::as_const( optionalTables ) )
        {
          char *sql = sqlite3_mprintf( "DELETE FROM %w WHERE table_name = '%q'",
                                       tableName.toUtf8().constData(),
                                       layerName.toUtf8().constData() );
          ( void )sqlite3_exec(
            database.get(),                      /* An open database */
            sql,                                 /* SQL to be evaluated */
            nullptr,                             /* Callback function */
            nullptr,                             /* 1st argument to callback */
            nullptr                              /* Error msg written here */
          );
          sqlite3_free( sql );
        }
        // Other tables, ignore errors
        {
          char *sql = sqlite3_mprintf( "DELETE FROM gpkg_2d_gridded_coverage_ancillary WHERE tile_matrix_set_name = '%q'",
                                       layerName.toUtf8().constData() );
          ( void )sqlite3_exec(
            database.get(),                      /* An open database */
            sql,                                 /* SQL to be evaluated */
            nullptr,                             /* Callback function */
            nullptr,                             /* 1st argument to callback */
            nullptr                              /* Error msg written here */
          );
          sqlite3_free( sql );
        }
        {
          char *sql = sqlite3_mprintf( "DELETE FROM gpkg_2d_gridded_tile_ancillary WHERE tpudt_name = '%q'",
                                       layerName.toUtf8().constData() );
          ( void )sqlite3_exec(
            database.get(),                      /* An open database */
            sql,                                 /* SQL to be evaluated */
            nullptr,                             /* Callback function */
            nullptr,                             /* 1st argument to callback */
            nullptr                              /* Error msg written here */
          );
          sqlite3_free( sql );
        }

        if ( status == SQLITE_OK )
        {
          result = true;
        }
        else
        {
          errCause = tr( "There was an error deleting the layer %1: %2" ).arg( layerName, QString::fromUtf8( errmsg ) );
        }
        sqlite3_free( errmsg );
      }
    }
  }
  else
  {
    // This should never happen!
    errCause = tr( "Layer URI is empty: layer cannot be deleted!" );
  }
  return result;
}

QgsGeoPackageConnectionItem::QgsGeoPackageConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsGeoPackageCollectionItem( parent, name, path )
{

}

bool QgsGeoPackageConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsGeoPackageConnectionItem *o = qobject_cast<const QgsGeoPackageConnectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;

}

QgsGeoPackageAbstractLayerItem::QgsGeoPackageAbstractLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, QgsLayerItem::LayerType layerType, const QString &providerKey )
  : QgsLayerItem( parent, name, path, uri, layerType, providerKey )
{
  mCapabilities |= Delete;
  mToolTip = uri;
  setState( Populated ); // no children are expected
}

bool QgsGeoPackageAbstractLayerItem::executeDeleteLayer( QString &errCause )
{
  errCause = QObject::tr( "The layer <b>%1</b> cannot be deleted because this feature is not yet implemented for this kind of layers." ).arg( mName );
  return false;
}

static int collect_strings( void *names, int, char **argv, char ** )
{
  *static_cast<QList<QString>*>( names ) << QString::fromUtf8( argv[ 0 ] );
  return 0;
}

QList<QString> QgsGeoPackageAbstractLayerItem::tableNames()
{
  QList<QString> names;
  QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( providerKey(), mUri ) );
  QString baseUri = pieces[QStringLiteral( "path" )].toString();
  if ( !baseUri.isEmpty() )
  {
    char *errmsg = nullptr;
    sqlite3_database_unique_ptr database;
    int status = database.open_v2( baseUri, SQLITE_OPEN_READONLY, nullptr );
    if ( status == SQLITE_OK )
    {
      char *sql = sqlite3_mprintf( "SELECT table_name FROM gpkg_contents;" );
      status = sqlite3_exec(
                 database.get(),              /* An open database */
                 sql,                         /* SQL to be evaluated */
                 collect_strings,             /* Callback function */
                 &names,                      /* 1st argument to callback */
                 &errmsg                      /* Error msg written here */
               );
      sqlite3_free( sql );
      if ( status != SQLITE_OK )
      {
        QgsDebugMsg( QStringLiteral( "There was an error reading tables from GPKG layer %1: %2" ).arg( mUri, QString::fromUtf8( errmsg ) ) );
      }
      sqlite3_free( errmsg );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "There was an error opening GPKG %1" ).arg( mUri ) );
    }
  }
  return  names;
}


QList<QgsMapLayer *> QgsGeoPackageAbstractLayerItem::layersInProject() const
{
  // Check if the layer(s) are in the registry
  QList<QgsMapLayer *> layersList;
  const auto mapLayers( QgsProject::instance()->mapLayers() );
  for ( QgsMapLayer *layer :  mapLayers )
  {
    if ( layer->publicSource() == mUri )
    {
      layersList << layer;
    }
  }
  return layersList;
}

QgsGeoPackageVectorLayerItem::QgsGeoPackageVectorLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, layerType, QStringLiteral( "ogr" ) )
{
  mCapabilities |= Rename;
}


QgsGeoPackageRasterLayerItem::QgsGeoPackageRasterLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, QgsLayerItem::LayerType::Raster, QStringLiteral( "gdal" ) )
{

}

bool QgsGeoPackageRasterLayerItem::executeDeleteLayer( QString &errCause )
{
  return QgsGeoPackageCollectionItem::deleteGeoPackageRasterLayer( mUri, errCause );
}

bool QgsGeoPackageVectorLayerItem::executeDeleteLayer( QString &errCause )
{
  return ::deleteLayer( mUri, errCause );
}

bool QgsGeoPackageVectorLayerItem::rename( const QString &name )
{
  // Checks that name does not exist yet
  if ( tableNames().contains( name ) )
  {
    return false;
  }
  // Check if the layer(s) are in the registry
  const QList<QgsMapLayer *> layersList( layersInProject() );
  if ( ! layersList.isEmpty( ) )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Rename Layer" ), QObject::tr( "The layer <b>%1</b> is loaded in the current project with name <b>%2</b>,"
                                " do you want to remove it from the project and rename it?" ).arg( mName, layersList.at( 0 )->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      return false;
    }
  }
  if ( ! layersList.isEmpty() )
  {
    QgsProject::instance()->removeMapLayers( layersList );
  }

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( mProviderKey, mUri );
  QString errCause;
  if ( parts.empty() || parts.value( QStringLiteral( "path" ) ).isNull() || parts.value( QStringLiteral( "layerName" ) ).isNull() )
  {
    errCause = QObject::tr( "Layer URI %1 is not valid!" ).arg( mUri );
  }
  else
  {
    QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
    const QList<QgsMapLayer *> layersList( layersInProject() );
    if ( ! layersList.isEmpty( ) )
    {
      if ( QMessageBox::question( nullptr, QObject::tr( "Rename Layer" ), QObject::tr( "The layer <b>%1</b> exists in the current project <b>%2</b>,"
                                  " do you want to remove it from the project and rename it?" ).arg( mName, layersList.at( 0 )->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      {
        return false;
      }
    }
    if ( ! layersList.isEmpty() )
    {
      QgsProject::instance()->removeMapLayers( layersList );
    }

    // TODO: maybe an index?
    QString oldName = parts.value( QStringLiteral( "layerName" ) ).toString();

    GDALDatasetH hDS = GDALOpenEx( filePath.toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr );
    if ( hDS )
    {
      QString sql( QStringLiteral( "ALTER TABLE \"%1\" RENAME TO \"%2\"" ).arg( oldName, name ) );
      OGRLayerH ogrLayer( GDALDatasetExecuteSQL( hDS, sql.toUtf8().constData(), nullptr, nullptr ) );
      if ( ogrLayer )
        GDALDatasetReleaseResultSet( hDS, ogrLayer );
      GDALClose( hDS );
      errCause = CPLGetLastErrorMsg( );
    }
    else
    {
      errCause = QObject::tr( "There was an error opening %1!" ).arg( filePath );
    }
  }

  if ( ! errCause.isEmpty() )
    QMessageBox::critical( nullptr, QObject::tr( "Error renaming layer" ), errCause );
  else if ( mParent )
    mParent->refreshConnections();

  return errCause.isEmpty();
}

