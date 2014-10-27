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

QgsGrassLocationItem::QgsGrassLocationItem( QgsDataItem* parent, QString path )
    : QgsDataCollectionItem( parent, "", path )
{
  QFileInfo fi( path );
  mName = fi.baseName();
  mIcon = QIcon( QgsApplication::getThemePixmap( "grass_location.png" ) );
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

  QDir dir( mPath );

  QStringList entries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
  foreach ( QString name, entries )
  {
    QString path = dir.absoluteFilePath( name );

    if ( QgsGrassMapsetItem::isMapset( path ) )
    {
      QgsGrassMapsetItem * mapset = new QgsGrassMapsetItem( this, path );
      mapsets.append( mapset );
    }
  }
  return mapsets;
}

QgsGrassMapsetItem::QgsGrassMapsetItem( QgsDataItem* parent, QString path )
    : QgsDataCollectionItem( parent, "", path )
{
  QDir dir( path );
  mName = dir.dirName();
  dir.cdUp();
  mLocation = dir.dirName();
  dir.cdUp();
  mGisdbase = dir.path();

  mIcon = QIcon( QgsApplication::getThemePixmap( "grass_mapset.png" ) );
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

  QStringList vectorNames = QgsGrass::vectors( mPath );

  foreach ( QString name, vectorNames )
  {
    QStringList layerNames = QgsGrass::vectorLayers( mGisdbase, mLocation, mName, name );

    QString path = mPath + QDir::separator() + "vector" + QDir::separator() + name;

    QgsDataCollectionItem *map = 0;
    if ( layerNames.size() != 1 )
      map = new QgsDataCollectionItem( this, name );
    foreach ( QString layerName, layerNames )
    {
      QString uri = mPath + QDir::separator() + name + QDir::separator() + layerName;
      QgsLayerItem::LayerType layerType = QgsLayerItem::Vector;
      QString typeName = layerName.split( "_" )[1];
      QString baseLayerName = layerName.split( "_" )[0];

      if ( typeName == "point" )
        layerType = QgsLayerItem::Point;
      else if ( typeName == "line" )
        layerType = QgsLayerItem::Line;
      else if ( typeName == "polygon" )
        layerType = QgsLayerItem::Polygon;

      if ( layerNames.size() == 1 )
      {
        QgsLayerItem *layer = new QgsLayerItem( this, name + " " + baseLayerName, path, uri, layerType, "grass" );
        items.append( layer );
      }
      else
      {
        QgsLayerItem *layer = new QgsLayerItem( map, baseLayerName, path, uri, layerType, "grass" );
        map->addChild( layer );
      }
    }
    if ( layerNames.size() != 1 )
      items.append( map );
  }

  QStringList rasterNames = QgsGrass::rasters( mPath );

  foreach ( QString name, rasterNames )
  {
    QString uri = mPath + QDir::separator() + "cellhd" + QDir::separator() + name;
    QgsDebugMsg( "uri = " + uri );

    QgsLayerItem *layer = new QgsLayerItem( this, name, uri, uri, QgsLayerItem::Raster, "grassraster" );

    items.append( layer );
  }

  return items;
}

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Dir;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  if ( QgsGrassLocationItem::isLocation( thePath ) )
  {
    QgsGrassLocationItem * location = new QgsGrassLocationItem( parentItem, thePath );
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
