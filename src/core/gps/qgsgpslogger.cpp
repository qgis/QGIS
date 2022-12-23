/***************************************************************************
  qgsgpslogger.cpp
   -------------------
  begin                : November 2022
  copyright            : (C) 2022 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpslogger.h"
#include "qgsgpsconnection.h"
#include "gmath.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgspolygon.h"

#include <QTimer>
#include <QTimeZone>

QgsGpsLogger::QgsGpsLogger( QgsGpsConnection *connection, QObject *parent )
  : QObject( parent )
  , mWgs84CRS( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) )
{
  setConnection( connection );

  mLastNmeaPosition.lat = nmea_degree2radian( 0.0 );
  mLastNmeaPosition.lon = nmea_degree2radian( 0.0 );

  mAcquisitionTimer = std::unique_ptr<QTimer>( new QTimer( this ) );
  mAcquisitionTimer->setSingleShot( true );

  updateGpsSettings();

  connect( mAcquisitionTimer.get(), &QTimer::timeout,
           this, &QgsGpsLogger::switchAcquisition );
}

QgsGpsLogger::~QgsGpsLogger()
{

}

QgsGpsConnection *QgsGpsLogger::connection()
{
  return mConnection;
}

void QgsGpsLogger::setConnection( QgsGpsConnection *connection )
{
  if ( mConnection )
  {
    disconnect( mConnection, &QgsGpsConnection::stateChanged, this, &QgsGpsLogger::gpsStateChanged );
  }

  mConnection = connection;

  if ( mConnection )
  {
    connect( mConnection, &QgsGpsConnection::stateChanged, this, &QgsGpsLogger::gpsStateChanged );
  }
}

void QgsGpsLogger::setEllipsoid( const QString &ellipsoid )
{
  mDistanceCalculator.setEllipsoid( ellipsoid );
  emit distanceAreaChanged();
}

void QgsGpsLogger::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
  mDistanceCalculator.setSourceCrs( mWgs84CRS, mTransformContext );
  emit distanceAreaChanged();
}

QgsCoordinateTransformContext QgsGpsLogger::transformContext() const
{
  return mTransformContext;
}

const QgsDistanceArea &QgsGpsLogger::distanceArea() const
{
  return mDistanceCalculator;
}

QVector<QgsPoint> QgsGpsLogger::currentTrack() const
{
  return mCaptureListWgs84;
}

QgsGeometry QgsGpsLogger::currentGeometry( QgsWkbTypes::Type type, QString &error ) const
{
  const QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::geometryType( type );
  const QVector< QgsPoint > captureListWgs84 = currentTrack();
  if ( geometryType == QgsWkbTypes::LineGeometry && captureListWgs84.size() < 2 )
  {
    error = tr( "Creating a line feature requires a track with at least two vertices." );
    return QgsGeometry();
  }
  else if ( geometryType == QgsWkbTypes::PolygonGeometry && captureListWgs84.size() < 3 )
  {
    error = tr( "Creating a polygon feature requires a track with at least three vertices." );
    return QgsGeometry();
  }

  const bool is3D = QgsWkbTypes::hasZ( type );
  const bool isMeasure = QgsWkbTypes::hasM( type );
  switch ( geometryType )
  {
    case QgsWkbTypes::PointGeometry:
    {
      const QgsPointXY pointXYWgs84 = lastPosition();

      QgsGeometry g;
      if ( is3D )
        g = QgsGeometry( new QgsPoint( pointXYWgs84.x(), pointXYWgs84.y(), lastElevation() ) );
      else
        g = QgsGeometry::fromPointXY( pointXYWgs84 );

      if ( isMeasure )
        g.get()->addMValue( lastMValue() );

      if ( QgsWkbTypes::isMultiType( type ) )
        g.convertToMultiType();

      return g;
    }

    case QgsWkbTypes::LineGeometry:
    case QgsWkbTypes::PolygonGeometry:
    {
      QgsGeometry g;

      std::unique_ptr<QgsLineString> ringWgs84( new QgsLineString( captureListWgs84 ) );
      if ( !is3D )
        ringWgs84->dropZValue();
      if ( !isMeasure )
        ringWgs84->dropMValue();

      if ( geometryType == QgsWkbTypes::LineGeometry )
      {
        g = QgsGeometry( ringWgs84.release() );
        if ( QgsWkbTypes::isMultiType( type ) )
          g.convertToMultiType();
      }
      else if ( geometryType == QgsWkbTypes::PolygonGeometry )
      {
        ringWgs84->close();
        std::unique_ptr<QgsPolygon> polygon( new QgsPolygon() );
        polygon->setExteriorRing( ringWgs84.release() );

        g = QgsGeometry( polygon.release() );

        if ( QgsWkbTypes::isMultiType( type ) )
          g.convertToMultiType();
      }
      return g;
    }

    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      break;
  }
  return QgsGeometry();
}

QgsPointXY QgsGpsLogger::lastPosition() const
{
  return mLastGpsPositionWgs84;
}

double QgsGpsLogger::lastElevation() const
{
  return mLastElevation;
}

double QgsGpsLogger::lastMValue() const
{
  return mLastMValue;
}

void QgsGpsLogger::resetTrack()
{
  mBlockGpsStateChanged++;

  const bool trackWasEmpty = mCaptureListWgs84.isEmpty();
  mCaptureListWgs84.clear();
  mBlockGpsStateChanged--;
  mTrackStartTime = QDateTime();

  if ( !trackWasEmpty )
    emit trackIsEmptyChanged( true );

  emit trackReset();
}

void QgsGpsLogger::updateGpsSettings()
{
  int acquisitionInterval = 0;
  if ( QgsGpsConnection::settingsGpsTimeStampSpecification.exists() )
  {
    acquisitionInterval = static_cast< int >( QgsGpsConnection::settingGpsAcquisitionInterval.value() );
    mDistanceThreshold = QgsGpsConnection::settingGpsDistanceThreshold.value();
    mApplyLeapSettings = QgsGpsConnection::settingGpsApplyLeapSecondsCorrection.value();
    mLeapSeconds = static_cast< int >( QgsGpsConnection::settingGpsLeapSeconds.value() );
    mTimeStampSpec = QgsGpsConnection::settingsGpsTimeStampSpecification.value();
    mTimeZone = QgsGpsConnection::settingsGpsTimeStampTimeZone.value();
    mOffsetFromUtc = static_cast< int >( QgsGpsConnection::settingsGpsTimeStampOffsetFromUtc.value() );

    mStoreAttributeInMValues = settingsGpsStoreAttributeInMValues.value();
    mMValueComponent = settingsGpsMValueComponent.value();
  }
  else
  {
    // legacy settings
    QgsSettings settings;

    acquisitionInterval = settings.value( QStringLiteral( "acquisitionInterval" ), 0, QgsSettings::Gps ).toInt();
    mDistanceThreshold = settings.value( QStringLiteral( "distanceThreshold" ), 0, QgsSettings::Gps ).toDouble();
    mApplyLeapSettings = settings.value( QStringLiteral( "applyLeapSeconds" ), true, QgsSettings::Gps ).toBool();
    mLeapSeconds = settings.value( QStringLiteral( "leapSecondsCorrection" ), 18, QgsSettings::Gps ).toInt();

    switch ( settings.value( QStringLiteral( "timeStampFormat" ), Qt::LocalTime, QgsSettings::Gps ).toInt() )
    {
      case 0:
        mTimeStampSpec = Qt::TimeSpec::LocalTime;
        break;

      case 1:
        mTimeStampSpec = Qt::TimeSpec::UTC;
        break;

      case 2:
        mTimeStampSpec = Qt::TimeSpec::TimeZone;
        break;
    }
    mTimeZone = settings.value( QStringLiteral( "timestampTimeZone" ), QVariant(), QgsSettings::Gps ).toString();
  }

  mAcquisitionInterval = acquisitionInterval * 1000;
  if ( mAcquisitionTimer->isActive() )
    mAcquisitionTimer->stop();
  mAcquisitionEnabled = true;

  switchAcquisition();
}

double QgsGpsLogger::totalTrackLength() const
{
  QVector<QgsPointXY> points;
  QgsGeometry::convertPointList( mCaptureListWgs84, points );
  return mDistanceCalculator.measureLine( points );
}

double QgsGpsLogger::trackDistanceFromStart() const
{
  if ( mCaptureListWgs84.empty() )
    return 0;

  return mDistanceCalculator.measureLine( { QgsPointXY( mCaptureListWgs84.constFirst() ), QgsPointXY( mCaptureListWgs84.constLast() )} );
}

QVariant QgsGpsLogger::componentValue( Qgis::GpsInformationComponent component ) const
{
  if ( !mConnection )
    return QVariant();

  switch ( component )
  {
    case Qgis::GpsInformationComponent::Location:
    case Qgis::GpsInformationComponent::Altitude:
    case Qgis::GpsInformationComponent::GeoidalSeparation:
    case Qgis::GpsInformationComponent::EllipsoidAltitude:
    case Qgis::GpsInformationComponent::GroundSpeed:
    case Qgis::GpsInformationComponent::Bearing:
    case Qgis::GpsInformationComponent::Pdop:
    case Qgis::GpsInformationComponent::Hdop:
    case Qgis::GpsInformationComponent::Vdop:
    case Qgis::GpsInformationComponent::HorizontalAccuracy:
    case Qgis::GpsInformationComponent::VerticalAccuracy:
    case Qgis::GpsInformationComponent::HvAccuracy:
    case Qgis::GpsInformationComponent::SatellitesUsed:
      return mConnection->currentGPSInformation().componentValue( component );

    case Qgis::GpsInformationComponent::Timestamp:
      return lastTimestamp();

    case Qgis::GpsInformationComponent::TotalTrackLength:
      return totalTrackLength();
    case Qgis::GpsInformationComponent::TrackDistanceFromStart:
      return trackDistanceFromStart();
    case Qgis::GpsInformationComponent::TrackStartTime:
      return trackStartTime();
    case Qgis::GpsInformationComponent::TrackEndTime:
      return lastTimestamp();

    case Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint:
      return mPreviousTrackPoint.isEmpty() ? QVariant() : distanceArea().measureLine( mPreviousTrackPoint, lastPosition() );
    case Qgis::GpsInformationComponent::TrackTimeSinceLastPoint:
      return mPreviousTrackPointTime.isValid() ? static_cast< double >( mPreviousTrackPointTime.msecsTo( lastTimestamp() ) ) / 1000 : QVariant();
  }
  BUILTIN_UNREACHABLE
}

void QgsGpsLogger::switchAcquisition()
{
  if ( mAcquisitionInterval > 0 )
  {
    if ( mAcquisitionEnabled )
      mAcquisitionTimer->start( mAcquisitionInterval );
    else
      //wait only acquisitionInterval/10 for new valid data
      mAcquisitionTimer->start( mAcquisitionInterval / 10 );
    // anyway switch to enabled / disabled acquisition
    mAcquisitionEnabled = !mAcquisitionEnabled;
  }
}

void QgsGpsLogger::gpsStateChanged( const QgsGpsInformation &info )
{
  if ( mBlockGpsStateChanged )
    return;

  const bool validFlag = info.isValid();
  QgsPointXY newLocationWgs84;
  nmeaPOS newNmeaPosition;
  double newAlt = 0.0;
  if ( validFlag )
  {
    newLocationWgs84 = QgsPointXY( info.longitude, info.latitude );
    newNmeaPosition.lat = nmea_degree2radian( info.latitude );
    newNmeaPosition.lon = nmea_degree2radian( info.longitude );
    newAlt = info.elevation;

    if ( info.utcDateTime.isValid() )
    {
      mLastTime = info.utcDateTime;
    }

    switch ( mMValueComponent )
    {
      case Qgis::GpsInformationComponent::Altitude:
      case Qgis::GpsInformationComponent::GroundSpeed:
      case Qgis::GpsInformationComponent::Bearing:
      case Qgis::GpsInformationComponent::Pdop:
      case Qgis::GpsInformationComponent::Hdop:
      case Qgis::GpsInformationComponent::Vdop:
      case Qgis::GpsInformationComponent::HorizontalAccuracy:
      case Qgis::GpsInformationComponent::VerticalAccuracy:
      case Qgis::GpsInformationComponent::HvAccuracy:
      case Qgis::GpsInformationComponent::SatellitesUsed:
      case Qgis::GpsInformationComponent::GeoidalSeparation:
      case Qgis::GpsInformationComponent::EllipsoidAltitude:
      {
        const QVariant value = info.componentValue( mMValueComponent );
        mLastMValue = value.isValid() ? info.componentValue( mMValueComponent ).toDouble() : std::numeric_limits< double >::quiet_NaN();
        break;
      }

      case Qgis::GpsInformationComponent::Timestamp:
        mLastMValue = static_cast< double >( info.utcDateTime.toMSecsSinceEpoch() );
        break;

      case Qgis::GpsInformationComponent::Location:
      case Qgis::GpsInformationComponent::TotalTrackLength:
      case Qgis::GpsInformationComponent::TrackDistanceFromStart:
      case Qgis::GpsInformationComponent::TrackStartTime:
      case Qgis::GpsInformationComponent::TrackEndTime:
      case Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint:
      case Qgis::GpsInformationComponent::TrackTimeSinceLastPoint:
        // not possible
        break;
    }
  }
  else
  {
    newLocationWgs84 = mLastGpsPositionWgs84;
    newNmeaPosition = mLastNmeaPosition;
    newAlt = mLastElevation;
  }
  if ( !mAcquisitionEnabled || ( nmea_distance( &newNmeaPosition, &mLastNmeaPosition ) < mDistanceThreshold ) )
  {
    // do not update position if update is disabled by timer or distance is under threshold
    newLocationWgs84 = mLastGpsPositionWgs84;

  }
  if ( validFlag && mAcquisitionEnabled )
  {
    // position updated by valid data, reset timer
    switchAcquisition();
  }

  // Avoid adding track vertices when we haven't moved
  if ( mLastGpsPositionWgs84 != newLocationWgs84 )
  {
    mLastGpsPositionWgs84 = newLocationWgs84;
    mLastNmeaPosition = newNmeaPosition;
    mLastElevation = newAlt;

    if ( mAutomaticallyAddTrackVertices )
    {
      addTrackVertex();
    }
  }

  emit stateChanged( info );

  mPreviousTrackPointTime = lastTimestamp();
  mPreviousTrackPoint = mLastGpsPositionWgs84;
}

void QgsGpsLogger::addTrackVertex()
{
  QgsPoint pointWgs84 = QgsPoint( mLastGpsPositionWgs84.x(), mLastGpsPositionWgs84.y(), mLastElevation );

  if ( mStoreAttributeInMValues )
  {
    pointWgs84.addMValue( mLastMValue );
  }

  const bool trackWasEmpty = mCaptureListWgs84.empty();
  mCaptureListWgs84.push_back( pointWgs84 );

  emit trackVertexAdded( pointWgs84 );

  if ( trackWasEmpty )
  {
    mTrackStartTime = lastTimestamp();
    emit trackIsEmptyChanged( false );
  }
}

bool QgsGpsLogger::automaticallyAddTrackVertices() const
{
  return mAutomaticallyAddTrackVertices;
}

void QgsGpsLogger::setAutomaticallyAddTrackVertices( bool enabled )
{
  mAutomaticallyAddTrackVertices = enabled;
}

QDateTime QgsGpsLogger::lastTimestamp() const
{
  if ( !mLastTime.isValid() )
    return QDateTime();

  QDateTime time = mLastTime;

  // Time from GPS is UTC time
  time.setTimeSpec( Qt::UTC );
  // Apply leap seconds correction
  if ( mApplyLeapSettings && mLeapSeconds != 0 )
  {
    time = time.addSecs( mLeapSeconds );
  }
  // Desired format
  if ( mTimeStampSpec != Qt::TimeSpec::OffsetFromUTC )
    time = time.toTimeSpec( mTimeStampSpec );

  if ( mTimeStampSpec == Qt::TimeSpec::TimeZone )
  {
    // Get timezone from the combo
    const QTimeZone destTz( mTimeZone.toUtf8() );
    if ( destTz.isValid() )
    {
      time = time.toTimeZone( destTz );
    }
  }
  else if ( mTimeStampSpec == Qt::TimeSpec::LocalTime )
  {
    time = time.toLocalTime();
  }
  else if ( mTimeStampSpec == Qt::TimeSpec::OffsetFromUTC )
  {
    time = time.toOffsetFromUtc( mOffsetFromUtc );
  }
  else if ( mTimeStampSpec == Qt::TimeSpec::UTC )
  {
    // Do nothing: we are already in UTC
  }

  return time;
}

QDateTime QgsGpsLogger::trackStartTime() const
{
  return mTrackStartTime;
}
