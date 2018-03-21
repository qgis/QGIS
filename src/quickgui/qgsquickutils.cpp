/***************************************************************************
  qgsquickutils.cpp
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QThread>

#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include "qgsquickmapsettings.h"
#include "qgsquickutils.h"


QgsQuickUtils *QgsQuickUtils::sInstance = 0;

QgsQuickUtils *QgsQuickUtils::instance()
{
  if ( !sInstance )
  {
    QgsDebugMsg( QString( "QgsQuickUtils created: %1" ).arg( long( QThread::currentThreadId() ) ) );
    sInstance = new QgsQuickUtils();
  }
  return sInstance;
}

QgsQuickUtils::QgsQuickUtils( QObject *parent )
  : QObject( parent )
{

  // calculate screen density for calculation of real pixel sizes from density-independent pixels
  int dpiX = QApplication::desktop()->physicalDpiX();
  int dpiY = QApplication::desktop()->physicalDpiY();
  int dpi = dpiX < dpiY ? dpiX : dpiY; // In case of asymmetrical DPI. Improbable
  mScreenDensity = dpi / 160.;  // 160 DPI is baseline for density-independent pixels in Android
}

QgsQuickUtils::~QgsQuickUtils()
{
}

QgsCoordinateReferenceSystem QgsQuickUtils::coordinateReferenceSystemFromEpsgId( long epsg ) const
{
  return QgsCoordinateReferenceSystem::fromEpsgId( epsg );
}

QgsPointXY QgsQuickUtils::pointXYFactory( double x, double y ) const
{
  return QgsPointXY( x, y );
}

QgsPoint QgsQuickUtils::pointFactory( double x, double y ) const
{
  return QgsPoint( x, y );
}

QgsPoint QgsQuickUtils::coordinateToPoint( const QGeoCoordinate &coor ) const
{
  return QgsPoint( coor.longitude(), coor.latitude(), coor.altitude() );
}

QgsPointXY QgsQuickUtils::transformPoint( const QgsCoordinateReferenceSystem &srcCrs,
    const QgsCoordinateReferenceSystem &destCrs,
    const QgsCoordinateTransformContext &context,
    const QgsPointXY &srcPoint ) const
{
  QgsCoordinateTransform mTransform( srcCrs, destCrs, context );
  QgsPointXY pt = mTransform.transform( srcPoint );
  return pt;
}

bool QgsQuickUtils::hasValidGeometry( QgsVectorLayer *layer, const QgsFeature &feat )
{
  if ( !layer )
    return false;

  if ( !feat.hasGeometry() )
    return false;

  if ( feat.geometry().type() != layer->geometryType() )
    return false;

  if ( QgsWkbTypes::hasZ( layer->wkbType() ) != QgsWkbTypes::hasZ( feat.geometry().wkbType() ) )
    return false;

  return true;
}

double QgsQuickUtils::screenUnitsToMeters( QgsQuickMapSettings *mapSettings, int baseLengthPixels ) const
{
  if ( mapSettings == 0 ) return 0;

  QgsDistanceArea mDistanceArea;
  mDistanceArea.setEllipsoid( "WGS84" );
  mDistanceArea.setSourceCrs( mapSettings->destinationCrs(), mapSettings->transformContext() );

  // calculate the geographic distance from the central point of extent
  // to the specified number of points on the right side
  QSize s = mapSettings->outputSize();
  QPoint pointCenter( s.width() / 2, s.height() / 2 );
  QgsPointXY p1 = mapSettings->screenToCoordinate( pointCenter );
  QgsPointXY p2 = mapSettings->screenToCoordinate( pointCenter + QPoint( baseLengthPixels, 0 ) );
  return mDistanceArea.measureLine( p1, p2 );
}

bool QgsQuickUtils::fileExists( QString path )
{
  QFileInfo check_file( path );
  // check if file exists and if yes: Is it really a file and no directory?
  return ( check_file.exists() && check_file.isFile() );
}

void QgsQuickUtils::copyFile( QString sourcePath, QString targetPath )
{
  if ( !fileExists( sourcePath ) )
  {
    QgsDebugMsg( QString( "Source file does not exist! %1" ).arg( sourcePath ) );
    return;
  }

  if ( !QDir::root().mkpath( targetPath ) )
  {
    QgsApplication::messageLog()->logMessage( tr( "Could not create folder %1" ).arg( targetPath ), "QgsQuick", Qgis::Critical );
    return;
  }

  QDir dir( targetPath );
  QString filename( QFile( sourcePath ).fileName() );

  if ( !QFile( sourcePath ).rename( dir.absoluteFilePath( filename ) ) )
  {
    QgsDebugMsg( QString( "Couldn't rename file! Trying to copy instead! %1" ).arg( filename ) );
    if ( !QFile( sourcePath ).copy( dir.absoluteFilePath( filename ) ) )
    {
      QgsApplication::messageLog()->logMessage( tr( "File %1 could not be copied to folder %2.", "QgsQuick", Qgis::Critical ).arg( sourcePath, targetPath ) );
      return;
    }
  }
}

void QgsQuickUtils::remove( QString path )
{
  QFile::remove( path );
}

QString QgsQuickUtils::getFileName( QString path )
{
  QFileInfo fileInfo( path );
  QString filename( fileInfo.fileName() );
  return filename;
}

void QgsQuickUtils::logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level )
{
  QgsMessageLog::logMessage( message, tag, level );
}

QUrl QgsQuickUtils::getThemeIcon( const QString &name )
{
  QString extension( ".svg" );
  QString path = "qrc:/" + name + extension;
  QgsDebugMsg( QString( "Using icon %1 from %2" ).arg( name ).arg( path ) );
  return QUrl( path );
}

QString QgsQuickUtils::qgsPointToString( const QgsPoint &point, int decimals )
{
  QString label;
  label += QString::number( point.x(), 'f', decimals );
  label += ", ";
  label += QString::number( point.y(), 'f', decimals );
  return label;
}

QString QgsQuickUtils::distanceToString( qreal dist, int decimals )
{
  if ( dist < 0 )
  {
    return "0 m";
  }

  QString label;
  if ( dist > 1000 )
  {
    label += QString::number( dist / 1000.0, 'f', decimals );
    label += QString( " km" );
  }
  else
  {
    if ( dist > 1 )
    {
      label += QString::number( dist, 'f', decimals );
      label += QString( " m" );
    }
    else
    {
      label += QString::number( dist * 1000, 'f', decimals );
      label += QString( " mm" );
    }
  }
  return label;
}

QString QgsQuickUtils::dumpScreenInfo() const
{
  QRect rec = QApplication::desktop()->screenGeometry();
  int dpiX = QApplication::desktop()->physicalDpiX();
  int dpiY = QApplication::desktop()->physicalDpiY();
  int height = rec.height();
  int width = rec.width();
  double sizeX = ( double ) width / dpiX * 25.4;
  double sizeY = ( double ) height / dpiY * 25.4;

  QString msg;
  msg += "screen resolution: " + QString::number( width ) + "x" + QString::number( height ) + " px\n";
  msg += "screen DPI: " + QString::number( dpiX ) + "x" + QString::number( dpiY ) + "\n";
  msg += "screen size: " + QString::number( sizeX, 'f', 0 ) + "x" + QString::number( sizeY, 'f', 0 ) + " mm\n";
  msg += "screen density: " + QString::number( mScreenDensity );
  return msg;
}

qreal QgsQuickUtils::screenDensity() const
{
  return mScreenDensity;
}
