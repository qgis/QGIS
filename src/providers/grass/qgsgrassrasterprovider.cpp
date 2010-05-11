/***************************************************************************
  qgsgrassrasterprovider.cpp  -  QGIS Data provider for
                           GRASS rasters
                             -------------------
    begin                : 16 Jan, 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* $Id: qgsgrassrasterprovider.cpp 11522 2009-08-28 14:49:22Z jef $ */

#include "qgslogger.h"
#include "qgsgrass.h"
#include "qgsgrassrasterprovider.h"

#include <math.h>

#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"

#include <QImage>
#include <QSettings>
#include <QColor>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFile>

static QString PROVIDER_KEY = "grassraster";
static QString PROVIDER_DESCRIPTION = "GRASS raster provider";

QgsGrassRasterProvider::QgsGrassRasterProvider( QString const & uri )
    : QgsRasterDataProvider( uri ), mValid( true )
{
  QgsDebugMsg( "QgsGrassRasterProvider: constructing with uri '" + uri + "'." );

  // Parse URI, it is the same like using GDAL, i.e. path to raster cellhd, i.e.
  // /path/to/gisdbase/location/mapset/cellhd/map
  QFileInfo fileInfo( uri );
  mValid = fileInfo.exists(); // then we keep it valid forever
  mMapName = fileInfo.fileName();
  QDir dir = fileInfo.dir();
  QString element = dir.dirName();
  if ( element != "cellhd" )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Groups not yet supported" ) + " (GRASS " + uri + ")" );

    mValid = false;
    return;
  }
  dir.cdUp(); // skip cellhd
  mMapset = dir.dirName();
  dir.cdUp();
  mLocation = dir.dirName();
  dir.cdUp();
  mGisdbase = dir.path();

  QgsDebugMsg( QString( "gisdbase: %1" ).arg( mGisdbase ) );
  QgsDebugMsg( QString( "location: %1" ).arg( mLocation ) );
  QgsDebugMsg( QString( "mapset: %1" ).arg( mMapset ) );
  QgsDebugMsg( QString( "mapName: %1" ).arg( mMapName ) );

  mCrs = QgsGrass::crs( mGisdbase, mLocation );
}

QgsGrassRasterProvider::~QgsGrassRasterProvider()
{
  QgsDebugMsg( "QgsGrassRasterProvider: deconstructing." );
}

QImage* QgsGrassRasterProvider::draw( QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight )
{
  QgsDebugMsg( "pixelWidth = "  + QString::number( pixelWidth ) );
  QgsDebugMsg( "pixelHeight = "  + QString::number( pixelHeight ) );
  QgsDebugMsg( "viewExtent: " + viewExtent.toString() );

  QImage *image = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  image->fill( QColor( Qt::gray ).rgb() );

  QStringList arguments;
  arguments.append( "map=" +  mMapName + "@" + mMapset );

  arguments.append(( QString( "window=%1,%2,%3,%4,%5,%6" )
                     .arg( viewExtent.xMinimum() ).arg( viewExtent.yMinimum() )
                     .arg( viewExtent.xMaximum() ).arg( viewExtent.yMaximum() )
                     .arg( pixelWidth ).arg( pixelHeight ) ) );
  QProcess process( this );
  QString cmd = QgsApplication::pkgDataPath() + "/grass/modules/qgis.d.rast";
  QByteArray data;
  try
  {
    data = QgsGrass::runModule( mGisdbase, mLocation, cmd, arguments );
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot draw raster" ) + "\n"
                          + e.what() );

    // We don't set mValid to false, because the raster can be recreated and work next time
    return image;
  }
  QgsDebugMsg( QString( "%1 bytes read from modules stdout" ).arg( data.size() ) );
  uchar * ptr = image->bits( ) ;
  // byteCount() in Qt >= 4.6
  //int size = image->byteCount() < data.size() ? image->byteCount() : data.size();
  int size = pixelWidth * pixelHeight * 4 < data.size() ? pixelWidth * pixelHeight * 4 : data.size();
  memcpy( ptr, data.data(), size );

  return image;
}

QgsCoordinateReferenceSystem QgsGrassRasterProvider::crs()
{
  QgsDebugMsg( "Entered" );
  return mCrs;
}

QgsRectangle QgsGrassRasterProvider::extent()
{
  // The extend can change of course so we get always fresh, to avoid running always the module
  // we should save mExtent and mLastModified and check if the map was modified

  QgsRectangle rect;
  rect = QgsGrass::extent( mGisdbase, mLocation, mMapset, mMapName, QgsGrass::Raster );
  return rect;
}

bool QgsGrassRasterProvider::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  QgsDebugMsg( "Entered" );
  //theResults["Error"] = tr( "Out of extent" );
  theResults = QgsGrass::query( mGisdbase, mLocation, mMapset, mMapName, QgsGrass::Raster, thePoint.x(), thePoint.y() );
  return true;
}

int QgsGrassRasterProvider::capabilities() const
{
  int capability = QgsRasterDataProvider::Identify;
  return capability;
}

bool QgsGrassRasterProvider::isValid()
{
  return mValid;
}

QString QgsGrassRasterProvider::identifyAsText( const QgsPoint& point )
{
  return  QString( "Not implemented" );
}

QString QgsGrassRasterProvider::identifyAsHtml( const QgsPoint& point )
{
  return  QString( "Not implemented" );
}

QString QgsGrassRasterProvider::lastErrorTitle()
{
  return  QString( "Not implemented" );
}

QString QgsGrassRasterProvider::lastError()
{
  return  QString( "Not implemented" );
}

QString  QgsGrassRasterProvider::name() const
{
  return PROVIDER_KEY;
}

QString  QgsGrassRasterProvider::description() const
{
  return PROVIDER_DESCRIPTION;
}

/**
 * Class factory to return a pointer to a newly created
 * QgsGrassRasterProvider object
 */
QGISEXTERN QgsGrassRasterProvider * classFactory( const QString *uri )
{
  return new QgsGrassRasterProvider( *uri );
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return PROVIDER_KEY;
}
/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return PROVIDER_DESCRIPTION;
}
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

