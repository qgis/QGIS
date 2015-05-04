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

#include "qgsgrassprovidermodule.h"
#include "qgsgrassprovider.h"
#include "qgsgrass.h"
#include "qgslogger.h"

#include <QAction>
#include <QFileInfo>
#include <QDir>

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

  return items;
}

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

QgsGrassVectorLayerItem::QgsGrassVectorLayerItem( QgsDataItem* parent, QgsGrassObject grassObject, QString layerName,
    QString path, QString uri,
    LayerType layerType, bool singleLayer )
    : QgsGrassObjectItem( parent, grassObject, layerName, path, uri, layerType, "grass", singleLayer )
{
}

QString QgsGrassVectorLayerItem::layerName() const
{
  // to get map + layer when added from browser
  return mGrassObject.name() + " " + name();
}

QgsGrassRasterItem::QgsGrassRasterItem( QgsDataItem* parent, QgsGrassObject grassObject,
                                        QString path, QString uri )
    : QgsGrassObjectItem( parent, grassObject, grassObject.name(), path, uri, QgsLayerItem::Raster, "grassraster" )
{
}

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
