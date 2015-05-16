/***************************************************************************
    qgsgrassprovidermodule.cpp -  Data provider for GRASS format
                     -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Gary E.Sherman, Radim Blazek
    email                : sherman@mrcc.com, blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessageoutput.h"
#include "qgsmimedatautils.h"
#include "qgsnewnamedialog.h"
#include "qgsproviderregistry.h"
#include "qgsrasterlayer.h"
#include "qgslogger.h"

#include "qgsgrassprovidermodule.h"
#include "qgsgrassprovider.h"
#include "qgsgrass.h"

#include <QAction>
#include <QFileInfo>
#include <QDir>
#include <QLabel>

//----------------------- QgsGrassLocationItem ------------------------------

QgsGrassLocationItem::QgsGrassLocationItem( QgsDataItem* parent, QString dirPath, QString path )
    : QgsDirectoryItem( parent, "", dirPath, path )
{
  // modify path to distinguish from directory, so that it can be expanded by path in browser
  QDir dir( mDirPath );
  mName = dir.dirName();

  mIconName = "grass_location.png";

  // set Directory type so that when sorted it gets into dirs (after the dir it represents)
  mType = QgsDataItem::Directory;
}

bool QgsGrassLocationItem::isLocation( QString path )
{
  //QgsDebugMsg( "path = " + path );
  return QFile::exists( path + "/" + "PERMANENT" + "/" + "DEFAULT_WIND" );
}

QVector<QgsDataItem*>QgsGrassLocationItem::createChildren()
{
  QVector<QgsDataItem*> mapsets;

  QDir dir( mDirPath );

  QStringList entries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
  foreach ( QString name, entries )
  {
    QString path = dir.absoluteFilePath( name );

    if ( QgsGrassMapsetItem::isMapset( path ) )
    {
      QgsGrassMapsetItem * mapset = new QgsGrassMapsetItem( this, path, mPath + "/" + name );
      mapsets.append( mapset );
    }
  }
  return mapsets;
}

//----------------------- QgsGrassMapsetItem ------------------------------

QList<QgsGrassImport*> QgsGrassMapsetItem::mImports;

QgsGrassMapsetItem::QgsGrassMapsetItem( QgsDataItem* parent, QString dirPath, QString path )
    : QgsDirectoryItem( parent, "", dirPath, path )
{
  QDir dir( mDirPath );
  mName = dir.dirName();
  dir.cdUp();
  mLocation = dir.dirName();
  dir.cdUp();
  mGisdbase = dir.path();

  mIconName = "grass_mapset.png";
}

bool QgsGrassMapsetItem::isMapset( QString path )
{
  return QFile::exists( path + "/WIND" );
}

QVector<QgsDataItem*> QgsGrassMapsetItem::createChildren()
{
  QgsDebugMsg( "Entered" );

  QVector<QgsDataItem*> items;

  QStringList vectorNames = QgsGrass::vectors( mDirPath );

  foreach ( QString name, vectorNames )
  {
    QStringList layerNames = QgsGrass::vectorLayers( mGisdbase, mLocation, mName, name );

    QString mapPath = mPath + "/vector/" + name;

    QgsGrassObject vectorObject( mGisdbase, mLocation, mName, name, QgsGrassObject::Vector );
    QgsGrassVectorItem *map = 0;
    if ( layerNames.size() > 1 )
    {
      //map = new QgsDataCollectionItem( this, name, mapPath );
      //map->setCapabilities( QgsDataItem::NoCapabilities ); // disable fertility
      map = new QgsGrassVectorItem( this, vectorObject, mapPath );
    }
    foreach ( QString layerName, layerNames )
    {
      // don't use QDir::separator(), windows work with '/' and backslash may be lost if
      // somewhere not properly escaped (there was bug in QgsMimeDataUtils for example)
      QString uri = mDirPath + "/" + name + "/" + layerName;
      QgsLayerItem::LayerType layerType = QgsLayerItem::Vector;
      QString typeName = layerName.split( "_" )[1];
      QString baseLayerName = layerName.split( "_" )[0];

      if ( typeName == "point" )
        layerType = QgsLayerItem::Point;
      else if ( typeName == "line" )
        layerType = QgsLayerItem::Line;
      else if ( typeName == "polygon" )
        layerType = QgsLayerItem::Polygon;

      QString layerPath = mapPath + "/" + layerName;
      if ( !map )
      {
        /* This may happen (one layer only) in GRASS 7 with points (no topo layers) */
        QgsLayerItem *layer = new QgsGrassVectorLayerItem( this, vectorObject, name + " " + baseLayerName, layerPath, uri, layerType, true );
        //layer->setState( Populated );
        items.append( layer );
      }
      else
      {
        QgsLayerItem *layer = new QgsGrassVectorLayerItem( map, vectorObject, baseLayerName, layerPath, uri, layerType, false );
        map->addChild( layer );
      }
    }
    if ( map )
    {
      map->setState( Populated );
      items.append( map );
    }
  }

  QStringList rasterNames = QgsGrass::rasters( mDirPath );

  foreach ( QString name, rasterNames )
  {
    QString path = mPath + "/" + "raster" + "/" + name;
    QString uri = mDirPath + "/" + "cellhd" + "/" + name;
    QgsDebugMsg( "uri = " + uri );

    QgsGrassObject rasterObject( mGisdbase, mLocation, mName, name, QgsGrassObject::Raster );
    QgsGrassRasterItem *layer = new QgsGrassRasterItem( this, rasterObject, path, uri );

    items.append( layer );
  }

  QgsGrassObject mapsetObject( mGisdbase, mLocation, mName );
  foreach ( QgsGrassImport* import, mImports )
  {
    if ( !import )
    {
      continue;
    }
    if ( !import->grassObject().mapsetIdentical( mapsetObject ) )
    {
      continue;
    }
    foreach ( QString name, import->names() )
    {
      QString path = mPath + "/" + import->grassObject().elementName() + "/" + name;
      items.append( new QgsGrassImportItem( this, name, path, import ) );
    }
  }

  return items;
}

bool QgsGrassMapsetItem::handleDrop( const QMimeData * data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  QgsGrassObject mapsetObject( mGisdbase, mLocation, mName );
  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGisdbase, mLocation );

  QStringList existingRasters = QgsGrass::rasters( mapsetObject.mapsetPath() );
  // add currently being imported
  foreach ( QgsGrassImport* import, mImports )
  {
    if ( import && import->grassObject().type() == QgsGrassObject::Raster )
    {
      existingRasters.append( import->names() );
    }
  }
  QgsDebugMsg( "existingRasters = " + existingRasters.join( "," ) );

  QStringList errors;
  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  foreach ( const QgsMimeDataUtils::Uri& u, lst )
  {
    if ( u.layerType == "raster" )
    {
      QgsRasterDataProvider* provider = qobject_cast<QgsRasterDataProvider*>( QgsProviderRegistry::instance()->provider( u.providerKey, u.uri ) );
      if ( !provider )
      {
        errors.append( tr( "Cannot create provider %1 : %2" ).arg( u.providerKey ).arg( u.uri ) );
        continue;
      }
      if ( !provider->isValid() )
      {
        errors.append( tr( "Provider is not valid  %1 : %2" ).arg( u.providerKey ).arg( u.uri ) );
        delete provider;
        continue;
      }
#ifdef WIN32
      Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
      Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
      QStringList extensions = QgsGrassRasterImport::extensions( provider );
      QString newName = u.name;
      if ( QgsNewNameDialog::exists( u.name, extensions, existingRasters, cs ) )
      {
        QgsNewNameDialog dialog( u.name, u.name, extensions, existingRasters, QRegExp(), cs );
        if ( dialog.exec() != QDialog::Accepted )
        {
          delete provider;
          continue;
        }
        newName = dialog.name();
      }

      QgsRectangle newExtent = provider->extent();
      int newXSize;
      int newYSize;
      bool useSrcRegion = true;
      if ( provider->capabilities() & QgsRasterInterface::Size )
      {
        newXSize = provider->xSize();
        newYSize = provider->ySize();
      }
      else
      {
        // TODO: open dialog with size options
        // use location default
        QgsDebugMsg( "Unknown size -> using default location region" );
        struct Cell_head window;
        if ( !QgsGrass::defaultRegion( mGisdbase, mLocation, &window ) )
        {
          errors.append( tr( "Cannot get default location region." ) );
          delete provider;
          continue;
        }
        newXSize = window.cols;
        newYSize = window.rows;

        newExtent = QgsGrass::extent( &window );
        useSrcRegion = false;
      }

      QgsRasterPipe* pipe = new QgsRasterPipe();
      pipe->set( provider );

      QgsCoordinateReferenceSystem providerCrs = provider->crs();
      QgsDebugMsg( "providerCrs = " + providerCrs.toWkt() );
      QgsDebugMsg( "mapsetCrs = " + mapsetCrs.toWkt() );
      if ( providerCrs.isValid() && mapsetCrs.isValid() && providerCrs != mapsetCrs )
      {
        QgsRasterProjector * projector = new QgsRasterProjector;
        projector->setCRS( providerCrs, mapsetCrs );
        if ( useSrcRegion )
        {
          projector->destExtentSize( provider->extent(), provider->xSize(), provider->ySize(),
                                     newExtent, newXSize, newYSize );
        }

        pipe->set( projector );
      }
      QgsDebugMsg( "newExtent = " + newExtent.toString() );
      QgsDebugMsg( QString( "newXSize = %1 newYSize = %2" ).arg( newXSize ).arg( newYSize ) );

      QString path = mPath + "/" + "raster" + "/" + u.name;
      QgsGrassObject rasterObject( mGisdbase, mLocation, mName, newName, QgsGrassObject::Raster );
      QgsGrassRasterImport *import = new QgsGrassRasterImport( pipe, rasterObject,
          newExtent, newXSize, newYSize ); // takes pipe ownership
      connect( import, SIGNAL( finished( QgsGrassImport* ) ), SLOT( onImportFinished( QgsGrassImport* ) ) );

      // delete existing files (confirmed before in dialog)
      bool deleteOk = true;
      foreach ( QString name, import->names() )
      {
        QgsGrassObject obj( rasterObject );
        obj.setName( name );
        if ( QgsGrass::objectExists( obj ) )
        {
          QgsDebugMsg( name + " exists -> delete" );
          if ( !QgsGrass::deleteObject( obj ) )
          {
            errors.append( tr( "Cannot delete %1" ).arg( name ) );
            deleteOk = false;
          }
        }
      }
      if ( !deleteOk )
      {
        delete import;
        continue;
      }

      import->importInThread();
      mImports.append( import );
      existingRasters.append( import->names() );
      refresh(); // after each new item so that it is visible if dialog is opened on next item
    }
    else
    {
      errors.append( tr( "%1 layer type not supported" ).arg( u.name ) );
    }
  }

  if ( !errors.isEmpty() )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to GRASS mapset" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + errors.join( "\n" ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

void QgsGrassMapsetItem::onImportFinished( QgsGrassImport* import )
{
  QgsDebugMsg( "entered" );
  if ( !import->error().isEmpty() )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to GRASS mapset failed" ) );
    output->setMessage( tr( "Failed to import %1 to %2: %3" ).arg( import->uri() ).arg( import->grassObject()
                        .mapsetPath() ).arg( import->error() ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  mImports.removeOne( import );
  import->deleteLater();
  refresh();
}

//----------------------- QgsGrassObjectItemBase ------------------------------

QgsGrassObjectItemBase::QgsGrassObjectItemBase( QgsGrassObject grassObject ) :
    mGrassObject( grassObject )
{
}

void QgsGrassObjectItemBase::deleteGrassObject( QgsDataItem* parent )
{
  QgsDebugMsg( "Entered" );

  if ( !QgsGrass::deleteObjectDialog( mGrassObject ) )
    return;

  // Warning if failed is currently in QgsGrass::deleteMap
  if ( QgsGrass::deleteObject( mGrassObject ) )
  {
    // message on success is redundant, like in normal file browser
    if ( parent )
      parent->refresh();
  }
}

QgsGrassObjectItem::QgsGrassObjectItem( QgsDataItem* parent, QgsGrassObject grassObject,
                                        QString name, QString path, QString uri,
                                        LayerType layerType, QString providerKey,
                                        bool deleteAction )
    : QgsLayerItem( parent, name, path, uri, layerType, providerKey )
    , QgsGrassObjectItemBase( grassObject )
    , mDeleteAction( deleteAction )
{
  setState( Populated ); // no children, to show non expandable in browser
}

QList<QAction*> QgsGrassObjectItem::actions()
{
  QList<QAction*> lst;

  if ( mDeleteAction )
  {
    QAction* actionDelete = new QAction( tr( "Delete" ), this );
    connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteGrassObject() ) );
    lst.append( actionDelete );
  }

  return lst;
}

void QgsGrassObjectItem::deleteGrassObject()
{
  QgsGrassObjectItemBase::deleteGrassObject( parent() );
}

//----------------------- QgsGrassVectorItem ------------------------------

QgsGrassVectorItem::QgsGrassVectorItem( QgsDataItem* parent, QgsGrassObject grassObject, QString path ) :
    QgsDataCollectionItem( parent, grassObject.name(), path )
    , QgsGrassObjectItemBase( grassObject )
{
  QgsDebugMsg( "name = " + grassObject.name() + " path = " + path );
  setCapabilities( QgsDataItem::NoCapabilities ); // disable fertility
}

QList<QAction*> QgsGrassVectorItem::actions()
{
  QList<QAction*> lst;

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteGrassObject() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsGrassVectorItem::deleteGrassObject()
{
  QgsGrassObjectItemBase::deleteGrassObject( parent() );
}

//----------------------- QgsGrassVectorLayerItem ------------------------------

QgsGrassVectorLayerItem::QgsGrassVectorLayerItem( QgsDataItem* parent, QgsGrassObject grassObject, QString layerName,
    QString path, QString uri,
    LayerType layerType, bool singleLayer )
    : QgsGrassObjectItem( parent, grassObject, layerName, path, uri, layerType, "grass" )
    , mSingleLayer( singleLayer )
{
}

QString QgsGrassVectorLayerItem::layerName() const
{
  // to get map + layer when added from browser
  return mGrassObject.name() + " " + name();
}

//----------------------- QgsGrassRasterItem ------------------------------

QgsGrassRasterItem::QgsGrassRasterItem( QgsDataItem* parent, QgsGrassObject grassObject,
                                        QString path, QString uri )
    : QgsGrassObjectItem( parent, grassObject, grassObject.name(), path, uri, QgsLayerItem::Raster, "grassraster" )
{
}

//----------------------- QgsGrassImportItem ------------------------------

QgsGrassImportItem::QgsGrassImportItem( QgsDataItem* parent, const QString& name, const QString& path, QgsGrassImport* import )
    : QgsDataItem( QgsDataItem::Layer, parent, name, path )
    , QgsGrassObjectItemBase( import->grassObject() )
{
  setCapabilities( QgsDataItem::NoCapabilities ); // disable fertility
  setState( Populating );
}

//-------------------------------------------------------------------------

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Dir;
}

QGISEXTERN QgsDataItem * dataItem( QString theDirPath, QgsDataItem* parentItem )
{
  if ( QgsGrassLocationItem::isLocation( theDirPath ) )
  {
    QString path;
    QDir dir( theDirPath );
    QString dirName = dir.dirName();
    if ( parentItem )
    {
      path = parentItem->path();
    }
    else
    {
      dir.cdUp();
      path = dir.path();
    }
    path = path + "/" + "grass:" + dirName;
    QgsGrassLocationItem * location = new QgsGrassLocationItem( parentItem, theDirPath, path );
    return location;
  }
  return 0;
}

/**
* Class factory to return a pointer to a newly created
* QgsGrassProvider object
*/
QGISEXTERN QgsGrassProvider * classFactory( const QString *uri )
{
  return new QgsGrassProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return QString( "grass" );
}

/**
* Required description function
*/
QGISEXTERN QString description()
{
  return QString( "GRASS %1 vector provider" ).arg( GRASS_VERSION_MAJOR );
}

/**
* Required isProvider function. Used to determine if this shared library
* is a data provider plugin
*/
QGISEXTERN bool isProvider()
{
  return true;
}
