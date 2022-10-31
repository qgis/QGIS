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

#include <QCoreApplication>
#include <QTime>
#include <QIODevice>
#include <QStringList>
#include <QFileInfo>

#include "info.h"

bool QgsGpsInformation::isValid() const
{
  bool valid = false;
  if ( status == 'V' || fixType == NMEA_FIX_BAD || qualityIndicator == Qgis::GpsQualityIndicator::Invalid ) // some sources say that 'V' indicates position fix, but is below acceptable quality
  {
    valid = false;
  }
  else if ( fixType == NMEA_FIX_2D )
  {
    valid = true;
  }
  else if ( status == 'A' || fixType == NMEA_FIX_3D || ( qualityIndicator != Qgis::GpsQualityIndicator::Invalid ) ) // good
  {
    valid = true;
  }

  valid &= longitude >= -180.0 && longitude <= 180.0 && latitude >= -90.0 && latitude <= 90.0;

  return valid;
}

Qgis::GpsFixStatus QgsGpsInformation::fixStatus() const
{
  Qgis::GpsFixStatus fixStatus = Qgis::GpsFixStatus::NoData;

  // no fix if any of the three report bad; default values are invalid values and won't be changed if the corresponding NMEA msg is not received
  if ( status == 'V' || fixType == NMEA_FIX_BAD || qualityIndicator == Qgis::GpsQualityIndicator::Invalid ) // some sources say that 'V' indicates position fix, but is below acceptable quality
  {
    fixStatus = Qgis::GpsFixStatus::NoFix;
  }
  else if ( fixType == NMEA_FIX_2D ) // 2D indication (from GGA)
  {
    fixStatus = Qgis::GpsFixStatus::Fix2D;
  }
  else if ( status == 'A' || fixType == NMEA_FIX_3D || qualityIndicator != Qgis::GpsQualityIndicator::Invalid ) // good
  {
    fixStatus = Qgis::GpsFixStatus::Fix3D;
  }
  return fixStatus;
}

QString QgsGpsInformation::qualityDescription() const
{
  switch ( qualityIndicator )
  {
    case Qgis::GpsQualityIndicator::Simulation:
      return QCoreApplication::translate( "QgsGpsInformation", "Simulation mode" );

    case Qgis::GpsQualityIndicator::Manual:
      return QCoreApplication::translate( "QgsGpsInformation", "Manual input mode" );

    case Qgis::GpsQualityIndicator::Estimated:
      return QCoreApplication::translate( "QgsGpsInformation", "Estimated" );

    case Qgis::GpsQualityIndicator::FloatRTK:
      return QCoreApplication::translate( "QgsGpsInformation", "Float RTK" );

    case Qgis::GpsQualityIndicator::RTK:
      return QCoreApplication::translate( "QgsGpsInformation", "Fixed RTK" );

    case Qgis::GpsQualityIndicator::PPS:
      return QCoreApplication::translate( "QgsGpsInformation", "PPS" );

    case Qgis::GpsQualityIndicator::DGPS:
      return QCoreApplication::translate( "QgsGpsInformation", "DGPS" );

    case Qgis::GpsQualityIndicator::GPS:
      return QCoreApplication::translate( "QgsGpsInformation", "Autonomous" );

    case Qgis::GpsQualityIndicator::Invalid:
      return QCoreApplication::translate( "QgsGpsInformation", "Invalid" );

    case Qgis::GpsQualityIndicator::Unknown:
    default:
      return QCoreApplication::translate( "QgsGpsInformation", "Unknown (%1)" ).arg( QString::number( quality ) );
  }
}

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
  if ( info.fixStatus() != mLastFixStatus )
  {
    mLastFixStatus = info.fixStatus();
    emit fixStatusChanged( mLastFixStatus );
  }
}

void QgsGpsConnection::clearLastGPSInformation()
{
  mLastGPSInformation = QgsGpsInformation();
}
