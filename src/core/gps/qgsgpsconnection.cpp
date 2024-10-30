/***************************************************************************
                          qgsgpsconnection.cpp  -  description
                          --------------------
    begin                : November 30th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpsconnection.h"
#include "moc_qgsgpsconnection.cpp"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"


#include <QIODevice>


const QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType> *QgsGpsConnection::settingsGpsConnectionType = new QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType>( QStringLiteral( "gps-connection-type" ), QgsSettingsTree::sTreeGps, Qgis::GpsConnectionType::Automatic, QStringLiteral( "GPS connection type" ) ) SIP_SKIP;

const QgsSettingsEntryEnumFlag<Qt::TimeSpec> *QgsGpsConnection::settingsGpsTimeStampSpecification = new QgsSettingsEntryEnumFlag<Qt::TimeSpec>( QStringLiteral( "timestamp-time-spec" ), QgsSettingsTree::sTreeGps, Qt::TimeSpec::LocalTime, QStringLiteral( "GPS time stamp specification" ) ) SIP_SKIP;

const QgsSettingsEntryString *QgsGpsConnection::settingsGpsdHostName = new QgsSettingsEntryString( QStringLiteral( "gpsd-host-name" ), QgsSettingsTree::sTreeGps, QString(), QStringLiteral( "GPSD connection host name" ) ) SIP_SKIP;

const QgsSettingsEntryInteger *QgsGpsConnection::settingsGpsdPortNumber = new QgsSettingsEntryInteger( QStringLiteral( "gpsd-port" ), QgsSettingsTree::sTreeGps, 2947, QStringLiteral( "GPSD port number" ) ) SIP_SKIP;

const QgsSettingsEntryString *QgsGpsConnection::settingsGpsdDeviceName = new QgsSettingsEntryString( QStringLiteral( "gpsd-device-name" ), QgsSettingsTree::sTreeGps, QString(), QStringLiteral( "GPSD connection device name" ) ) SIP_SKIP;

const QgsSettingsEntryString *QgsGpsConnection::settingsGpsSerialDevice = new QgsSettingsEntryString( QStringLiteral( "gpsd-serial-device" ), QgsSettingsTree::sTreeGps, QString(), QStringLiteral( "GPS serial device name" ) ) SIP_SKIP;

const QgsSettingsEntryInteger *QgsGpsConnection::settingGpsAcquisitionInterval = new QgsSettingsEntryInteger( QStringLiteral( "acquisition-interval" ), QgsSettingsTree::sTreeGps, 0, QStringLiteral( "GPS track point acquisition interval" ) ) SIP_SKIP;

const QgsSettingsEntryDouble *QgsGpsConnection::settingGpsDistanceThreshold = new QgsSettingsEntryDouble( QStringLiteral( "distance-threshold" ), QgsSettingsTree::sTreeGps, 0, QStringLiteral( "GPS track point distance threshold" ) ) SIP_SKIP;

const QgsSettingsEntryBool *QgsGpsConnection::settingGpsBearingFromTravelDirection = new QgsSettingsEntryBool( QStringLiteral( "calculate-bearing-from-travel" ), QgsSettingsTree::sTreeGps, false, QStringLiteral( "Calculate GPS bearing from travel direction" ) ) SIP_SKIP;

const QgsSettingsEntryBool *QgsGpsConnection::settingGpsApplyLeapSecondsCorrection = new QgsSettingsEntryBool( QStringLiteral( "apply-leap-seconds-correction" ), QgsSettingsTree::sTreeGps, true, QStringLiteral( "Whether leap seconds corrections should be applied to GPS timestamps" ) ) SIP_SKIP;

const QgsSettingsEntryInteger *QgsGpsConnection::settingGpsLeapSeconds = new QgsSettingsEntryInteger( QStringLiteral( "leap-seconds" ), QgsSettingsTree::sTreeGps, 18, QStringLiteral( "Leap seconds correction amount (in seconds)" ) ) SIP_SKIP;

const QgsSettingsEntryString *QgsGpsConnection::settingsGpsTimeStampTimeZone = new QgsSettingsEntryString( QStringLiteral( "timestamp-time-zone" ), QgsSettingsTree::sTreeGps, QString(), QStringLiteral( "GPS time stamp time zone" ) ) SIP_SKIP;

const QgsSettingsEntryInteger *QgsGpsConnection::settingsGpsTimeStampOffsetFromUtc = new QgsSettingsEntryInteger( QStringLiteral( "timestamp-offset-from-utc" ), QgsSettingsTree::sTreeGps, 0, QStringLiteral( "GPS time stamp offset from UTC (in seconds)" ) ) SIP_SKIP;

QgsGpsConnection::QgsGpsConnection( QIODevice *dev )
  : QObject( nullptr )
  , mSource( dev )
{
  if ( mSource )
    QObject::connect( mSource.get(), &QIODevice::readyRead, this, &QgsGpsConnection::parseData );

  QObject::connect( this, &QgsGpsConnection::stateChanged, this, &QgsGpsConnection::onStateChanged );
}

QgsGpsConnection::~QgsGpsConnection()
{
  cleanupSource();
}

bool QgsGpsConnection::connect()
{
  if ( !mSource )
  {
    return false;
  }

  const bool connected = mSource->open( QIODevice::ReadWrite | QIODevice::Unbuffered );
  if ( connected )
  {
    mStatus = Connected;
  }
  return connected;
}

bool QgsGpsConnection::close()
{
  if ( !mSource )
  {
    return false;
  }

  mSource->close();

  if ( mLastFixStatus != Qgis::GpsFixStatus::NoData )
  {
    mLastFixStatus = Qgis::GpsFixStatus::NoData;
    emit fixStatusChanged( mLastFixStatus );
  }

  return true;
}

void QgsGpsConnection::cleanupSource()
{
  if ( mSource )
  {
    mSource->close();
  }
  mSource.reset();

  if ( mLastFixStatus != Qgis::GpsFixStatus::NoData )
  {
    mLastFixStatus = Qgis::GpsFixStatus::NoData;
    emit fixStatusChanged( mLastFixStatus );
  }
}

void QgsGpsConnection::setSource( QIODevice *source )
{
  cleanupSource();
  mSource.reset( source );
  QObject::connect( mSource.get(), &QIODevice::readyRead, this, &QgsGpsConnection::parseData );

  clearLastGPSInformation();
}

void QgsGpsConnection::onStateChanged( const QgsGpsInformation &info )
{
  if ( info.isValid() )
  {
    const QgsPoint oldPosition = mLastLocation;
    mLastLocation = QgsPoint( info.longitude, info.latitude, info.elevation );
    if ( mLastLocation != oldPosition )
    {
      emit positionChanged( mLastLocation );
    }
  }

  Qgis::GnssConstellation bestFixConstellation = Qgis::GnssConstellation::Unknown;
  Qgis::GpsFixStatus bestFix = info.bestFixStatus( bestFixConstellation );
  if ( bestFix != mLastFixStatus )
  {
    mLastFixStatus = bestFix;
    emit fixStatusChanged( mLastFixStatus );
  }
}

void QgsGpsConnection::clearLastGPSInformation()
{
  mLastGPSInformation = QgsGpsInformation();
}
