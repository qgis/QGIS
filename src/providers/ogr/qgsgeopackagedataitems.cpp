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

#include "sqlite3.h"

#include "qgsgeopackagedataitems.h"
#include "qgsgeopackageconnection.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsogrprovider.h"
#include "qgsnewgeopackagelayerdialog.h"
#include "qgsmessageoutput.h"
#include "qgsvectorlayerexporter.h"
#include "gdal.h"

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

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

QgsGeoPackageRootItem::QgsGeoPackageRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mGeoPackage.svg" );
  populate();
}

QgsGeoPackageRootItem::~QgsGeoPackageRootItem()
{

}

QVector<QgsDataItem *> QgsGeoPackageRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsGeoPackageConnection::connectionList() )
  {
    QgsGeoPackageConnection connection( connName );
    QgsDataItem *conn = new QgsGeoPackageConnectionItem( this, connection.name(), connection.uri().encodedUri() );

    connections.append( conn );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsGeoPackageRootItem::actions()
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, &QAction::triggered, this, &QgsGeoPackageRootItem::newConnection );
  lst.append( actionNew );

  QAction *actionCreateDatabase = new QAction( tr( "Create Database..." ), this );
  connect( actionCreateDatabase, &QAction::triggered, this, &QgsGeoPackageRootItem::createDatabase );
  lst.append( actionCreateDatabase );

  return lst;
}

QWidget *QgsGeoPackageRootItem::paramWidget()
{
  return nullptr;
}
#endif

void QgsGeoPackageRootItem::connectionsChanged()
{
  refresh();
}

void QgsGeoPackageRootItem::newConnection()
{
  // TODO use QgsFileWidget
  QString path = QFileDialog::getOpenFileName( nullptr, tr( "Open GeoPackage" ), "", tr( "GeoPackage Database (*.gpkg)" ) );
  storeConnection( path );
}


#ifdef HAVE_GUI
void QgsGeoPackageRootItem::createDatabase()
{
  QgsNewGeoPackageLayerDialog dialog( nullptr );
  dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
  if ( dialog.exec() == QDialog::Accepted )
  {
    storeConnection( dialog.databasePath() );
  }
}
#endif

bool QgsGeoPackageRootItem::storeConnection( const QString &path )
{
  QFileInfo fileInfo( path );
  QString connName = fileInfo.fileName();
  if ( ! path.isEmpty() )
  {
    bool ok = true;
    while ( ok && ! QgsGeoPackageConnection( connName ).path( ).isEmpty( ) )
    {

      connName = QInputDialog::getText( nullptr, tr( "Cannot add connection '%1'" ).arg( connName ),
                                        tr( "A connection with the same name already exists,\nplease provide a new name:" ), QLineEdit::Normal,
                                        QLatin1String( "" ), &ok );
    }
    if ( ok && ! connName.isEmpty() )
    {
      QgsGeoPackageConnection connection( connName );
      connection.setPath( path );
      connection.save();
      refreshConnections();
      return true;
    }
  }
  return false;
}


QgsGeoPackageConnectionItem::QgsGeoPackageConnectionItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
  , mPath( path )
{
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsGeoPackageConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  // Vector layers
  QgsVectorLayer layer( mPath, QStringLiteral( "ogr_tmp" ), QStringLiteral( "ogr" ) );
  if ( ! layer.isValid( ) )
  {
    QgsDebugMsgLevel( tr( "Layer is not a valid GeoPackage Vector layer %1" ).arg( mPath ), 3 );
  }
  else
  {
    // Collect mixed-geom layers
    QMultiMap<int, QStringList> subLayers;
    Q_FOREACH ( const QString &descriptor, layer.dataProvider()->subLayers( ) )
    {
      QStringList pieces = descriptor.split( ':' );
      subLayers.insert( pieces[0].toInt(), pieces );
    }
    int prevIdx = -1;
    Q_FOREACH ( const int &idx, subLayers.keys( ) )
    {
      if ( idx == prevIdx )
      {
        continue;
      }
      prevIdx = idx;
      QList<QStringList> values = subLayers.values( idx );
      for ( int i = 0; i < values.size(); ++i )
      {
        QStringList pieces = values.at( i );
        QString layerId = pieces[0];
        QString name = pieces[1];
        // QString featuresCount = pieces[2]; // Not used
        QString geometryType = pieces[3];
        QgsLayerItem::LayerType layerType;
        layerType = layerTypeFromDb( geometryType );
        // example URI for mixed-geoms geoms:    '/path/gdal_sample_v1.2_no_extensions.gpkg|layerid=7|geometrytype=Point'
        // example URI for mixed-geoms attr table:    '/path/gdal_sample_v1.2_no_extensions.gpkg|layername=MyLayer|layerid=7'
        // example URI for single geoms:    '/path/gdal_sample_v1.2_no_extensions.gpkg|layerid=6'
        QString uri;
        // Check if it's a mixed geometry type
        if ( i == 0 && values.size() > 1 )
        {
          uri = QStringLiteral( "%1|layerid=%2|layername=%3" ).arg( mPath, layerId, name );
          QgsGeoPackageVectorLayerItem *item = new QgsGeoPackageVectorLayerItem( this, name, mPath, uri, QgsLayerItem::LayerType::TableLayer );
          children.append( item );
        }
        if ( layerType != QgsLayerItem::LayerType::NoType )
        {
          if ( geometryType.contains( QStringLiteral( "Collection" ), Qt::CaseInsensitive ) )
          {
            QgsDebugMsgLevel( QStringLiteral( "Layer %1 is a geometry collection: skipping %2" ).arg( name, mPath ), 3 );
          }
          else
          {
            if ( values.size() > 1 )
            {
              uri = QStringLiteral( "%1|layerid=%2|geometrytype=%3" ).arg( mPath, layerId, geometryType );
            }
            else
            {
              uri = QStringLiteral( "%1|layerid=%2" ).arg( mPath, layerId );
            }
            QgsGeoPackageVectorLayerItem *item = new QgsGeoPackageVectorLayerItem( this, name, mPath, uri, layerType );
            QgsDebugMsgLevel( QStringLiteral( "Adding GeoPackage Vector item %1 %2 %3" ).arg( name, uri, geometryType ), 3 );
            children.append( item );
          }
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "Layer type is not a supported GeoPackage Vector layer %1" ).arg( mPath ), 3 );
        }
        QgsDebugMsgLevel( QStringLiteral( "Adding GeoPackage Vector item %1 %2 %3" ).arg( name, uri, geometryType ), 3 );
      }
    }
  }
  // Raster layers
  QgsRasterLayer rlayer( mPath, QStringLiteral( "gdal_tmp" ), QStringLiteral( "gdal" ), false );
  if ( rlayer.dataProvider()->subLayers( ).size() > 0 )
  {
    Q_FOREACH ( const QString &uri, rlayer.dataProvider()->subLayers( ) )
    {
      QStringList pieces = uri.split( ':' );
      QString name = pieces.value( pieces.length() - 1 );
      QgsDebugMsgLevel( QStringLiteral( "Adding GeoPackage Raster item %1 %2 %3" ).arg( name, uri ), 3 );
      QgsGeoPackageRasterLayerItem *item = new QgsGeoPackageRasterLayerItem( this, name, mPath, uri );
      children.append( item );
    }
  }
  else if ( rlayer.isValid( ) )
  {
    // Get the identifier
    GDALAllRegister();
    // do not print errors, but write to debug
    CPLPushErrorHandler( CPLQuietErrorHandler );
    CPLErrorReset();
    GDALDatasetH hDS = GDALOpen( mPath.toUtf8().constData(), GA_ReadOnly );
    CPLPopErrorHandler();

    if ( ! hDS )
    {
      QgsDebugMsg( QString( "GDALOpen error # %1 : %2 " ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ) );

    }
    else
    {
      QString uri( QStringLiteral( "GPKG:%1" ).arg( mPath ) );
      QString name = GDALGetMetadataItem( hDS, "IDENTIFIER", NULL );
      GDALClose( hDS );
      // Fallback: will not be able to delete the table
      if ( name.isEmpty() )
      {
        name = QFileInfo( mPath ).fileName();
      }
      else
      {
        uri += QStringLiteral( ":%1" ).arg( name );
      }

      QgsDebugMsgLevel( QStringLiteral( "Adding GeoPackage Raster item %1 %2 %3" ).arg( name, mPath ), 3 );
      QgsGeoPackageRasterLayerItem *item = new QgsGeoPackageRasterLayerItem( this, name, mPath, uri );
      children.append( item );
    }
  }
  return children;

}

bool QgsGeoPackageConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsGeoPackageConnectionItem *o = dynamic_cast<const QgsGeoPackageConnectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;

}

#ifdef HAVE_GUI

QList<QAction *> QgsGeoPackageConnectionItem::actions()
{
  QList<QAction *> lst;

  QAction *actionDeleteConnection = new QAction( tr( "Remove connection" ), this );
  connect( actionDeleteConnection, &QAction::triggered, this, &QgsGeoPackageConnectionItem::deleteConnection );
  lst.append( actionDeleteConnection );

  // Add table to existing DB
  QAction *actionAddTable = new QAction( tr( "Create a new layer or table..." ), this );
  connect( actionAddTable, &QAction::triggered, this, &QgsGeoPackageConnectionItem::addTable );
  lst.append( actionAddTable );


  return lst;
}
#endif



bool QgsGeoPackageConnectionItem::handleDrop( const QMimeData *data, Qt::DropAction )
{

  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  QString uri;

  QStringList importResults;
  bool hasError = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  Q_FOREACH ( const QgsMimeDataUtils::Uri &u, lst )
  {
    if ( u.layerType == QStringLiteral( "vector" ) )
    {
      // Check that we are not copying over self
      if ( u.uri.startsWith( mPath ) )
      {
        importResults.append( tr( "You cannot import layer %1 over itself!" ).arg( u.name ) );
        hasError = true;

      }
      else
      {
        // open the source layer
        bool owner;
        QString error;
        QgsVectorLayer *srcLayer = u.vectorLayer( owner, error );
        if ( !srcLayer )
        {
          importResults.append( tr( "%1: %2" ).arg( u.name ).arg( error ) );
          hasError = true;
          continue;
        }

        if ( srcLayer->isValid() )
        {
          uri = mPath;
          QgsDebugMsgLevel( "URI " + uri, 3 );

          // check if the destination layer already exists
          bool exists = false;
          // Q_FOREACH won't detach ...
          for ( const auto child : children() )
          {
            if ( child->name() == u.name )
            {
              exists = true;
            }
          }
          if ( ! exists || QMessageBox::question( nullptr, tr( "Overwrite Layer" ),
                                                  tr( "Destination layer <b>%1</b> already exists. Do you want to overwrite it?" ).arg( u.name ), QMessageBox::Yes |  QMessageBox::No ) == QMessageBox::Yes )
          {

            QVariantMap options;
            options.insert( QStringLiteral( "driverName" ), QStringLiteral( "GPKG" ) );
            options.insert( QStringLiteral( "update" ), true );
            options.insert( QStringLiteral( "overwrite" ), true );
            options.insert( QStringLiteral( "layerName" ), u.name );

            std::unique_ptr< QgsVectorLayerExporterTask > exportTask( new QgsVectorLayerExporterTask( srcLayer, uri, QStringLiteral( "ogr" ), srcLayer->crs(), options, owner ) );

            // when export is successful:
            connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
            {
              // this is gross - TODO - find a way to get access to messageBar from data items
              QMessageBox::information( nullptr, tr( "Import to GeoPackage database" ), tr( "Import was successful." ) );
              refreshConnections();
            } );

            // when an error occurs:
            connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( int error, const QString & errorMessage )
            {
              if ( error != QgsVectorLayerExporter::ErrUserCanceled )
              {
                QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
                output->setTitle( tr( "Import to GeoPackage database" ) );
                output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
                output->showMessage();
              }
            } );

            QgsApplication::taskManager()->addTask( exportTask.release() );
          }
        }
        else
        {
          importResults.append( tr( "%1: Not a valid layer!" ).arg( u.name ) );
          hasError = true;
        }
      }
    }
    else
    {
      // TODO: implement raster import
      QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
      output->setTitle( tr( "Import to GeoPackage database failed" ) );
      output->setMessage( tr( "Failed to import some layers!\n\n" ) + QStringLiteral( "Raster import is not yet implemented!\n" ), QgsMessageOutput::MessageText );
      output->showMessage();
    }

  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to GeoPackage database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}


QgsLayerItem::LayerType QgsGeoPackageConnectionItem::layerTypeFromDb( const QString &geometryType )
{
  if ( geometryType.contains( QStringLiteral( "Point" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Point;
  }
  else if ( geometryType.contains( QStringLiteral( "Polygon" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Polygon;
  }
  else if ( geometryType.contains( QStringLiteral( "LineString" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Line;
  }
  else if ( geometryType.contains( QStringLiteral( "Collection" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Vector;
  }
  // To be moved in a parent class that would also work for gdal and rasters
  else if ( geometryType.contains( QStringLiteral( "Raster" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Raster;
  }
  return QgsLayerItem::LayerType::TableLayer;
}

void QgsGeoPackageConnectionItem::deleteConnection()
{
  QgsGeoPackageConnection::deleteConnection( name() );
  mParent->refreshConnections();
}

#ifdef HAVE_GUI
void QgsGeoPackageConnectionItem::addTable()
{
  QgsNewGeoPackageLayerDialog dialog( nullptr );
  QFileInfo fileInfo( mPath );
  QString connName = fileInfo.fileName();
  QgsGeoPackageConnection connection( connName );
  if ( ! connection.path().isEmpty() )
  {
    dialog.setDatabasePath( connection.path() );
    dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
    if ( dialog.exec() == QMessageBox::Ok )
    {
      mParent->refreshConnections();
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Cannot add Table: connection %1 does not exists or the path is empy!" ).arg( connName ) );
  }
}
#endif

#ifdef HAVE_GUI
QList<QAction *> QgsGeoPackageAbstractLayerItem::actions()
{
  QList<QAction *> lst;
  QAction *actionDeleteLayer = new QAction( tr( "Delete layer '%1'..." ).arg( mName ), this );
  connect( actionDeleteLayer, &QAction::triggered, this, &QgsGeoPackageAbstractLayerItem::deleteLayer );
  lst.append( actionDeleteLayer );
  return lst;
}
#endif

void QgsGeoPackageAbstractLayerItem::deleteLayer()
{
  // Check if the layer is in the registry
  const QgsMapLayer *projectLayer = nullptr;
  Q_FOREACH ( const QgsMapLayer *layer, QgsProject::instance()->mapLayers() )
  {
    if ( layer->publicSource() == mUri )
    {
      projectLayer = layer;
    }
  }
  if ( ! projectLayer )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Layer" ),
                                QObject::tr( "Are you sure you want to delete layer <b>%1</b> from GeoPackage?" ).arg( mName ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;

    QString errCause;
    bool res = executeDeleteLayer( errCause );
    if ( !res )
    {
      QMessageBox::warning( nullptr, tr( "Delete Layer" ), errCause );
    }
    else
    {
      QMessageBox::information( nullptr, tr( "Delete Layer" ), tr( "Layer <b>%1</b> deleted successfully." ).arg( mName ) );
      if ( mParent )
        mParent->refresh();
    }
  }
  else
  {
    QMessageBox::warning( nullptr, QObject::tr( "Delete Layer" ), QObject::tr( "The layer <b>%1</b> cannot be deleted because it is in the current project as <b>%2</b>,"
                          " remove it from the project and retry." ).arg( mName, projectLayer->name() ) );
  }

}

QgsGeoPackageAbstractLayerItem::QgsGeoPackageAbstractLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, QgsLayerItem::LayerType layerType, QString providerKey )
  : QgsLayerItem( parent, name, path, uri, layerType, providerKey )
{
  setState( Populated ); // no children are expected
}

bool QgsGeoPackageAbstractLayerItem::executeDeleteLayer( QString &errCause )
{
  errCause = QObject::tr( "The layer <b>%1</b> cannot be deleted because the this feature is not yet implemented for this kind of layers." ).arg( mName );
  return false;
}


QgsGeoPackageVectorLayerItem::QgsGeoPackageVectorLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, LayerType layerType )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, layerType, QStringLiteral( "ogr" ) )
{

}


QgsGeoPackageRasterLayerItem::QgsGeoPackageRasterLayerItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, QgsLayerItem::LayerType::Raster, QStringLiteral( "gdal" ) )
{

}

bool QgsGeoPackageRasterLayerItem::executeDeleteLayer( QString &errCause )
{
  bool result = false;
  // Better safe than sorry
  if ( ! mUri.isEmpty( ) )
  {
    QStringList pieces( mUri.split( ':' ) );
    if ( pieces.size() != 3 )
    {
      errCause = QStringLiteral( "Layer URI is malformed: layer <b>%1</b> cannot be deleted!" ).arg( mName );
    }
    else
    {
      QString baseUri = pieces.at( 1 );
      QString layerName = pieces.at( 2 );
      sqlite3 *handle;
      int status = sqlite3_open_v2( baseUri.toUtf8().constData(), &handle, SQLITE_OPEN_READWRITE, NULL );
      if ( status != SQLITE_OK )
      {
        errCause = sqlite3_errmsg( handle );
      }
      else
      {
        // Remove table
        char *errmsg = nullptr;
        char *sql = sqlite3_mprintf(
                      "DROP table %w;"
                      "DELETE FROM gpkg_contents WHERE table_name = '%q';"
                      "DELETE FROM gpkg_tile_matrix WHERE table_name = '%q';"
                      "DELETE FROM gpkg_tile_matrix_set WHERE table_name = '%q';",
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData() );
        status = sqlite3_exec(
                   handle,                              /* An open database */
                   sql,                                 /* SQL to be evaluated */
                   NULL,                                /* Callback function */
                   NULL,                                /* 1st argument to callback */
                   &errmsg                              /* Error msg written here */
                 );
        sqlite3_free( sql );
        // Remove from optional tables, may silently fail
        QStringList optionalTables;
        optionalTables << QStringLiteral( "gpkg_extensions" )
                       << QStringLiteral( "gpkg_metadata_reference" );
        for ( const auto tableName : optionalTables )
        {
          char *sql = sqlite3_mprintf( "DELETE FROM table %w WHERE table_name = '%q",
                                       tableName.toUtf8().constData(),
                                       layerName.toUtf8().constData() );
          sqlite3_exec(
            handle,                              /* An open database */
            sql,                                 /* SQL to be evaluated */
            NULL,                                /* Callback function */
            NULL,                                /* 1st argument to callback */
            NULL                                 /* Error msg written here */
          );
          sqlite3_free( sql );
        }
        if ( status == SQLITE_OK )
        {
          result = true;
        }
        else
        {
          errCause = tr( "There was an error deleting the layer: %1" ).arg( QString::fromUtf8( errmsg ) );
        }
        sqlite3_free( errmsg );
      }
      sqlite3_close( handle );
    }
  }
  else
  {
    // This should never happen!
    errCause = QStringLiteral( "Layer URI is empty: layer <b>%1</b> cannot be deleted!" ).arg( mName );
  }
  return result;
}


bool QgsGeoPackageVectorLayerItem::executeDeleteLayer( QString &errCause )
{
  return ::deleteLayer( mUri, errCause );
}

