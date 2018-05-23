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
#include <QDesktopWidget>
#include <QString>

#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include "qgsquickfeaturelayerpair.h"
#include "qgsquickmapsettings.h"
#include "qgsquickutils.h"
#include "qgsunittypes.h"


QgsQuickUtils::QgsQuickUtils( QObject *parent )
  : QObject( parent )
  , mScreenDensity( calculateScreenDensity() )
{
}

/**
 * Makes QgsCoordinateReferenceSystem::fromEpsgId accessable for QML components
 */
QgsCoordinateReferenceSystem QgsQuickUtils::coordinateReferenceSystemFromEpsgId( long epsg )
{
  return QgsCoordinateReferenceSystem::fromEpsgId( epsg );
}

QgsPointXY QgsQuickUtils::pointXYFactory( double x, double y ) const
{
  return QgsPointXY( x, y );
}

QgsPoint QgsQuickUtils::pointFactory( double x, double y, double z, double m ) const
{
  return QgsPoint( x, y, z, m );
}

QgsPoint QgsQuickUtils::coordinateToPoint( const QGeoCoordinate &coor ) const
{
  return QgsPoint( coor.longitude(), coor.latitude(), coor.altitude() );
}

QgsPointXY QgsQuickUtils::transformPoint( const QgsCoordinateReferenceSystem &srcCrs,
    const QgsCoordinateReferenceSystem &destCrs,
    const QgsCoordinateTransformContext &context,
    const QgsPointXY &srcPoint )
{
  QgsCoordinateTransform mTransform( srcCrs, destCrs, context );
  QgsPointXY pt = mTransform.transform( srcPoint );
  return pt;
}

double QgsQuickUtils::screenUnitsToMeters( QgsQuickMapSettings *mapSettings, int baseLengthPixels )
{
  if ( mapSettings == nullptr ) return 0.0;

  QgsDistanceArea mDistanceArea;
  mDistanceArea.setEllipsoid( QStringLiteral( "WGS84" ) );
  mDistanceArea.setSourceCrs( mapSettings->destinationCrs(), mapSettings->transformContext() );

  // calculate the geographic distance from the central point of extent
  // to the specified number of points on the right side
  QSize s = mapSettings->outputSize();
  QPoint pointCenter( s.width() / 2, s.height() / 2 );
  QgsPointXY p1 = mapSettings->screenToCoordinate( pointCenter );
  QgsPointXY p2 = mapSettings->screenToCoordinate( pointCenter + QPoint( baseLengthPixels, 0 ) );
  return mDistanceArea.measureLine( p1, p2 );
}

void QgsQuickUtils::logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level )
{
  QgsMessageLog::logMessage( message, tag, level );
}

QgsQuickFeatureLayerPair QgsQuickUtils::featureFactory( const QgsFeature &feature, QgsVectorLayer *layer ) const
{
  return QgsQuickFeatureLayerPair( feature, layer );
}

QUrl QgsQuickUtils::getThemeIcon( const QString &name )
  QString path = QStringLiteral( "qrc:/%1.svg" ).arg( name );
  QgsDebugMsg( QStringLiteral( "Using icon %1 from %2" ).arg( name, path ) );
  return QUrl( path );
}

QString QgsQuickUtils::qgsPointToString( const QgsPoint &point, int decimals )
{
  return QString( "%1, %2" ).arg( QString::number( point.x(), 'f', decimals ) ).arg( QString::number( point.y(), 'f', decimals ) );
}

QString QgsQuickUtils::distanceToString( double distance, QgsUnitTypes::DistanceUnit units, int decimals )
{
  double dist = distance * QgsUnitTypes::fromUnitToUnitFactor( units, QgsUnitTypes::DistanceMeters );

  if ( dist < 0 )
  {
    return QStringLiteral( "0 %1" ).arg( QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::DistanceMeters ) );
  }

  if ( dist > 1000 )
  {
    return QStringLiteral( "%1 %2" ).arg( QString::number( dist / 1000.0, 'f', decimals ) ).arg( QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::DistanceKilometers ) );
  }
  else
  {
    if ( dist > 1 )
    {
      return QStringLiteral( "%1 %2" ).arg( QString::number( dist, 'f', decimals ) ).arg( QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::DistanceMeters ) );
    }
    else
    {
      return QStringLiteral( "%1 %2" ).arg( QString::number( dist * 1000, 'f', decimals ) ).arg( QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::DistanceMillimeters ) );
    }
  }
}

QString QgsQuickUtils::dumpScreenInfo() const
{
  QRect rec = QApplication::desktop()->screenGeometry();
  int dpiX = QApplication::desktop()->physicalDpiX();
  int dpiY = QApplication::desktop()->physicalDpiY();
  int height = rec.height();
  int width = rec.width();
  double sizeX = static_cast<double>( width ) / dpiX * 25.4;
  double sizeY = static_cast<double>( height ) / dpiY * 25.4;

  QString msg;
  msg += tr( "screen resolution: %1x%2 px\n" ).arg( width ).arg( height );
  msg += tr( "screen DPI: %1x%2\n" ).arg( dpiX ).arg( dpiY );
  msg += tr( "screen size: %1x%2 mm\n" ).arg( QString::number( sizeX, 'f', 0 ), QString::number( sizeY, 'f', 0 ) );
  msg += tr( "screen density: %1" ).arg( mScreenDensity );
  return msg;
}

qreal QgsQuickUtils::screenDensity() const
{
  return mScreenDensity;
}

qreal QgsQuickUtils::calculateScreenDensity()
{
  // calculate screen density for calculation of real pixel sizes from density-independent pixels
  int dpiX = QApplication::desktop()->physicalDpiX();
  int dpiY = QApplication::desktop()->physicalDpiY();
  int dpi = dpiX < dpiY ? dpiX : dpiY; // In case of asymmetrical DPI. Improbable
  return dpi / 160.;  // 160 DPI is baseline for density-independent pixels in Android
}
