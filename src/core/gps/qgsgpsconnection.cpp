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

#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"

#include <QIODevice>

#include "moc_qgsgpsconnection.cpp"

const QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType> *QgsGpsConnection::settingsGpsConnectionType = new QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType>( u"gps-connection-type"_s, QgsSettingsTree::sTreeGps, Qgis::GpsConnectionType::Automatic, u"GPS connection type"_s ) SIP_SKIP;

const QgsSettingsEntryEnumFlag<Qt::TimeSpec> *QgsGpsConnection::settingsGpsTimeStampSpecification = new QgsSettingsEntryEnumFlag<Qt::TimeSpec>( u"timestamp-time-spec"_s, QgsSettingsTree::sTreeGps, Qt::TimeSpec::LocalTime, u"GPS time stamp specification"_s ) SIP_SKIP;

const QgsSettingsEntryString *QgsGpsConnection::settingsGpsdHostName = new QgsSettingsEntryString( u"gpsd-host-name"_s, QgsSettingsTree::sTreeGps, QString(), u"GPSD connection host name"_s ) SIP_SKIP;

const QgsSettingsEntryInteger *QgsGpsConnection::settingsGpsdPortNumber = new QgsSettingsEntryInteger( u"gpsd-port"_s, QgsSettingsTree::sTreeGps, 2947, u"GPSD port number"_s ) SIP_SKIP;

const QgsSettingsEntryString *QgsGpsConnection::settingsGpsdDeviceName = new QgsSettingsEntryString( u"gpsd-device-name"_s, QgsSettingsTree::sTreeGps, QString(), u"GPSD connection device name"_s ) SIP_SKIP;

const QgsSettingsEntryString *QgsGpsConnection::settingsGpsSerialDevice = new QgsSettingsEntryString( u"gpsd-serial-device"_s, QgsSettingsTree::sTreeGps, QString(), u"GPS serial device name"_s ) SIP_SKIP;

const QgsSettingsEntryInteger *QgsGpsConnection::settingGpsAcquisitionInterval = new QgsSettingsEntryInteger( u"acquisition-interval"_s, QgsSettingsTree::sTreeGps, 0, u"GPS track point acquisition interval"_s ) SIP_SKIP;

const QgsSettingsEntryDouble *QgsGpsConnection::settingGpsDistanceThreshold = new QgsSettingsEntryDouble( u"distance-threshold"_s, QgsSettingsTree::sTreeGps, 0, u"GPS track point distance threshold"_s ) SIP_SKIP;

const QgsSettingsEntryBool *QgsGpsConnection::settingGpsBearingFromTravelDirection = new QgsSettingsEntryBool( u"calculate-bearing-from-travel"_s, QgsSettingsTree::sTreeGps, false, u"Calculate GPS bearing from travel direction"_s ) SIP_SKIP;

const QgsSettingsEntryBool *QgsGpsConnection::settingGpsApplyLeapSecondsCorrection = new QgsSettingsEntryBool( u"apply-leap-seconds-correction"_s, QgsSettingsTree::sTreeGps, true, u"Whether leap seconds corrections should be applied to GPS timestamps"_s ) SIP_SKIP;

const QgsSettingsEntryInteger *QgsGpsConnection::settingGpsLeapSeconds = new QgsSettingsEntryInteger( u"leap-seconds"_s, QgsSettingsTree::sTreeGps, 18, u"Leap seconds correction amount (in seconds)"_s ) SIP_SKIP;

const QgsSettingsEntryString *QgsGpsConnection::settingsGpsTimeStampTimeZone = new QgsSettingsEntryString( u"timestamp-time-zone"_s, QgsSettingsTree::sTreeGps, QString(), u"GPS time stamp time zone"_s ) SIP_SKIP;

const QgsSettingsEntryInteger *QgsGpsConnection::settingsGpsTimeStampOffsetFromUtc = new QgsSettingsEntryInteger( u"timestamp-offset-from-utc"_s, QgsSettingsTree::sTreeGps, 0, u"GPS time stamp offset from UTC (in seconds)"_s ) SIP_SKIP;

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
