/***************************************************************************
  qgsquickpositionkit.cpp
  --------------------------------------
  Date                 : Dec. 2017
  Copyright            : (C) 2017 Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include "qgsquickpositionkit.h"
#include "qgsquickutils.h"
#include "qgsquicksimulatedpositionsource.h"

QgsQuickPositionKit::QgsQuickPositionKit( QObject *parent )
  : QObject( parent )
  , mAccuracy( -1 )
  , mScreenAccuracy( 2 )
  , mAccuracyUnits( QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::DistanceMeters ) )
  , mDirection( -1 )
  , mHasPosition( false )
  , mIsSimulated( false )
{
  connect( this, &QgsQuickPositionKit::simulatePositionLongLatRadChanged, this, &QgsQuickPositionKit::onSimulatePositionLongLatRadChanged );
  connect( this, &QgsQuickPositionKit::positionChanged, this, &QgsQuickPositionKit::updateScreenPosition );
  connect( this, &QgsQuickPositionKit::positionChanged, this, &QgsQuickPositionKit::updateProjectedPosition );

  use_gps_location();
}

QGeoPositionInfoSource  *QgsQuickPositionKit::gpsSource()
{
  // this should give us "true" position source
  // on Linux it comes from Geoclue library
  QGeoPositionInfoSource *source = QGeoPositionInfoSource::createDefaultSource( this );
  if ( source->error() != QGeoPositionInfoSource::NoError )
  {
    QgsMessageLog::logMessage( QStringLiteral( "%1 (%2)" )
                               .arg( tr( "Unable to create default GPS Position Source" ) )
                               .arg( QString::number( ( long )source->error() ) )
                               , QStringLiteral( "QgsQuick" )
                               , Qgis::Warning );
    delete source;
    return nullptr;
  }
  else
  {
    return source;
  }
}

QGeoPositionInfoSource  *QgsQuickPositionKit::simulatedSource( double longitude, double latitude, double radius )
{
  return new QgsQuickSimulatedPositionSource( this, longitude, latitude, radius );
}

void QgsQuickPositionKit::use_simulated_location( double longitude, double latitude, double radius )
{
  QGeoPositionInfoSource *source = simulatedSource( longitude, latitude, radius );
  mIsSimulated = true;
  replacePositionSource( source );
}

void QgsQuickPositionKit::updateScreenPosition()
{
  if ( !mMapSettings )
    return;

  mScreenPosition = mapSettings()->coordinateToScreen( projectedPosition() );

  emit screenPositionChanged();
}

QString QgsQuickPositionKit::sourceAccuracyLabel()
{
  if ( hasPosition() && accuracy() > 0 )
  {
    return QgsQuickUtils::distanceToString( accuracy(), 0 ); // e.g 1 km or 15 m or 500 mm
  }
  else
  {
    return QStringLiteral( "" );
  }
}

QString QgsQuickPositionKit::sourcePositionLabel( int precision )
{
  if ( hasPosition() )
  {
    return QgsQuickUtils::qgsPointToString( position(), precision ); // e.g -2.243, 45.441
  }
  else
  {
    return QStringLiteral( "" );
  }
}

void QgsQuickPositionKit::use_gps_location()
{
  QGeoPositionInfoSource *source = gpsSource();
  mIsSimulated = false;
  replacePositionSource( source );
}

void QgsQuickPositionKit::replacePositionSource( QGeoPositionInfoSource *source )
{
  if ( mSource.get() == source )
    return;

  if ( mSource )
  {
    mSource->disconnect();
  }

  mSource.reset( source );

  if ( mSource )
  {
    connect( mSource.get(), &QGeoPositionInfoSource::positionUpdated, this, &QgsQuickPositionKit::positionUpdated );
    connect( mSource.get(), &QGeoPositionInfoSource::updateTimeout, this,  &QgsQuickPositionKit::onUpdateTimeout );
    mSource->startUpdates();

    QgsDebugMsg( QStringLiteral( "Position source changed: %1" ).arg( mSource->sourceName() ) );
  }
}

QgsQuickMapSettings *QgsQuickPositionKit::mapSettings() const
{
  return mMapSettings;
}

void QgsQuickPositionKit::updateProjectedPosition()
{
  if ( !mMapSettings )
    return;

  QgsPointXY srcPoint = QgsPointXY( mPosition.x(), mPosition.y() );
  QgsPointXY projectedPositionXY = QgsQuickUtils::transformPoint( QgsQuickUtils::coordinateReferenceSystemFromEpsgId( 4326 ), mMapSettings->destinationCrs(), mMapSettings->transformContext(), srcPoint );
  mProjectedPosition = QgsPoint( projectedPositionXY.x(), projectedPositionXY.y() );
  mProjectedPosition.addZValue( mPosition.z() );
}

void QgsQuickPositionKit::positionUpdated( const QGeoPositionInfo &info )
{
  if ( !info.coordinate().isValid() )
  {
    // keep last valid position
    mHasPosition = false;
    emit hasPositionChanged();
  }


  mPosition = QgsPoint( info.coordinate().longitude(),
                        info.coordinate().latitude(),
                        info.coordinate().altitude() ); // can be NaN

  if ( info.hasAttribute( QGeoPositionInfo::HorizontalAccuracy ) )
    mAccuracy = info.attribute( QGeoPositionInfo::HorizontalAccuracy );
  else
    mAccuracy = -1;
  mScreenAccuracy = calculateScreenAccuracy();

  if ( info.hasAttribute( QGeoPositionInfo::Direction ) )
    mDirection = info.attribute( QGeoPositionInfo::Direction );
  else
    mDirection = -1;

  emit positionChanged();

  if ( !mHasPosition )
  {
    mHasPosition = true;
    emit hasPositionChanged();
  }
}
void QgsQuickPositionKit::onSimulatePositionLongLatRadChanged( QVector<double> simulatePositionLongLatRad )
{
  if ( simulatePositionLongLatRad.size() > 2 )
  {
    double longitude = simulatePositionLongLatRad[0];
    double latitude = simulatePositionLongLatRad[1];
    double radius = simulatePositionLongLatRad[2];
    QgsDebugMsg( QStringLiteral( "Use simulated position around longlat: %1, %2, %3" ).arg( longitude ).arg( latitude ).arg( radius ) );
    use_simulated_location( longitude, latitude, radius );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Unable to set simulated position due to the input errors." ) );
    use_gps_location();
  }
}

double QgsQuickPositionKit::calculateScreenAccuracy()
{
  if ( !mMapSettings )
    return 2.0;

  if ( accuracy() > 0 )
  {
    double scpm = QgsQuickUtils::screenUnitsToMeters( mMapSettings, 1 );
    if ( scpm > 0 )
      return 2 * ( accuracy() / scpm );
    else
      return 2.0;
  }
  return 2.0;
}

void QgsQuickPositionKit::onUpdateTimeout()
{
  if ( mHasPosition )
  {
    mHasPosition = false;
    emit hasPositionChanged();
  }
}

QPointF QgsQuickPositionKit::screenPosition() const
{
  return mScreenPosition;
}

void QgsQuickPositionKit::setScreenPosition( const QPointF &screenPosition )
{
  mScreenPosition = screenPosition;
}

double QgsQuickPositionKit::screenAccuracy() const
{
  return mScreenAccuracy;
}

QVector<double> QgsQuickPositionKit::simulatePositionLongLatRad() const
{
  return mSimulatePositionLongLatRad;
}

void QgsQuickPositionKit::setSimulatePositionLongLatRad( const QVector<double> &simulatePositionLongLatRad )
{
  mSimulatePositionLongLatRad = simulatePositionLongLatRad;
  emit simulatePositionLongLatRadChanged( simulatePositionLongLatRad );
}

QgsPoint QgsQuickPositionKit::projectedPosition() const
{
  return mProjectedPosition;
}

bool QgsQuickPositionKit::hasPosition() const
{
  return mHasPosition;
}

QgsPoint QgsQuickPositionKit::position() const
{
  return mPosition;
}

qreal QgsQuickPositionKit::accuracy() const
{
  return mAccuracy;
}

QString QgsQuickPositionKit::accuracyUnits() const
{
  return mAccuracyUnits;
}

qreal QgsQuickPositionKit::direction() const
{
  return mDirection;
}

bool QgsQuickPositionKit::simulated() const
{
  return mIsSimulated;
}

void QgsQuickPositionKit::setMapSettings( QgsQuickMapSettings *mapSettings )
{
  if ( mMapSettings == mapSettings )
    return;

  if ( mMapSettings )
  {
    mMapSettings->disconnect();
  }

  mMapSettings = mapSettings;

  if ( mMapSettings )
  {
    connect( mMapSettings, &QgsQuickMapSettings::extentChanged, this, &QgsQuickPositionKit::updateScreenPosition );
    connect( mMapSettings, &QgsQuickMapSettings::destinationCrsChanged, this, &QgsQuickPositionKit::updateScreenPosition );
    connect( mMapSettings, &QgsQuickMapSettings::mapUnitsPerPixelChanged, this, &QgsQuickPositionKit::updateScreenPosition );
    connect( mMapSettings, &QgsQuickMapSettings::visibleExtentChanged, this, &QgsQuickPositionKit::updateScreenPosition );
    connect( mMapSettings, &QgsQuickMapSettings::outputSizeChanged, this, &QgsQuickPositionKit::updateScreenPosition );
    connect( mMapSettings, &QgsQuickMapSettings::outputDpiChanged, this, &QgsQuickPositionKit::updateScreenPosition );
  }

  emit mapSettingsChanged();
}
