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
QgsGrassLocationItem::~QgsGrassLocationItem() {}

bool QgsGrassLocationItem::isLocation( QString path )
{
  //QgsDebugMsg( "path = " + path );
  return QFile::exists( path + QDir::separator() + "PERMANENT" + QDir::separator() + "DEFAULT_WIND" );
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
      QgsGrassMapsetItem * mapset = new QgsGrassMapsetItem( this, path, mPath + QDir::separator() + name );
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

QgsGrassMapsetItem::~QgsGrassMapsetItem() {}

bool QgsGrassMapsetItem::isMapset( QString path )
{
  return QFile::exists( path + QDir::separator() + "WIND" );
}

QVector<QgsDataItem*> QgsGrassMapsetItem::createChildren()
{
  QgsDebugMsg( "Entered" );

  QVector<QgsDataItem*> items;

  QStringList vectorNames = QgsGrass::vectors( mDirPath );

  foreach ( QString name, vectorNames )
  {
    QStringList layerNames = QgsGrass::vectorLayers( mGisdbase, mLocation, mName, name );

    QString mapPath = mPath + QDir::separator() + "vector" + QDir::separator() + name;

    QgsDataCollectionItem *map = 0;
    if ( layerNames.size() > 1 )
    {
      map = new QgsDataCollectionItem( this, name, mapPath );
      map->setCapabilities( QgsDataItem::NoCapabilities ); // disable fertility
    }
    foreach ( QString layerName, layerNames )
    {
      QString uri = mDirPath + QDir::separator() + name + QDir::separator() + layerName;
      QgsLayerItem::LayerType layerType = QgsLayerItem::Vector;
      QString typeName = layerName.split( "_" )[1];
      QString baseLayerName = layerName.split( "_" )[0];

      if ( typeName == "point" )
        layerType = QgsLayerItem::Point;
      else if ( typeName == "line" )
        layerType = QgsLayerItem::Line;
      else if ( typeName == "polygon" )
        layerType = QgsLayerItem::Polygon;

      QString layerPath = mapPath + QDir::separator() + layerName;
      if ( layerNames.size() == 1 )
      {
        /* This may happen (one layer only) in GRASS 7 with points (no topo layers) */
        QgsLayerItem *layer = new QgsLayerItem( this, name + " " + baseLayerName, layerPath, uri, layerType, "grass" );
        layer->setState( Populated );
        items.append( layer );
      }
      else if ( map )
      {
        QgsLayerItem *layer = new QgsGrassVectorLayerItem( map, name, baseLayerName, layerPath, uri, layerType, "grass" );
        map->addChild( layer );
      }
    }
    if ( layerNames.size() != 1 )
      items.append( map );
  }

  QStringList rasterNames = QgsGrass::rasters( mDirPath );

  foreach ( QString name, rasterNames )
  {
    QString path = mPath + QDir::separator() + "raster" + QDir::separator() + name;
    QString uri = mDirPath + QDir::separator() + "cellhd" + QDir::separator() + name;
    QgsDebugMsg( "uri = " + uri );

    QgsLayerItem *layer = new QgsLayerItem( this, name, path, uri, QgsLayerItem::Raster, "grassraster" );
    layer->setState( Populated );

    items.append( layer );
  }

  return items;
}

QgsGrassVectorLayerItem::QgsGrassVectorLayerItem( QgsDataItem* parent, QString mapName, QString layerName, QString path, QString uri, LayerType layerType, QString providerKey )
    : QgsLayerItem( parent, layerName, path, uri, layerType, providerKey )
    , mMapName( mapName )
{
  setState( Populated ); // no children, to show non expandable in browser
}

QString QgsGrassVectorLayerItem::layerName() const
{
  // to get map + layer when added from browser
  return mMapName + " " + name();
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
    path = path + QDir::separator() + "grass:" + dirName;
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
  return QString( "GRASS data provider" );
}

/**
* Required isProvider function. Used to determine if this shared library
* is a data provider plugin
*/
QGISEXTERN bool isProvider()
{
  return true;
}
