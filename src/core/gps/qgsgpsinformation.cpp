/***************************************************************************
                          qgsgpsinformation.cpp
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

#include "qgsgpsinformation.h"
#include "info.h"
#include "qgspointxy.h"

#include <QCoreApplication>

Qgis::GpsFixStatus QgsGpsInformation::bestFixStatus( Qgis::GnssConstellation &constellation ) const
{
  constellation = Qgis::GnssConstellation::Unknown;
  Qgis::GpsFixStatus bestStatus = Qgis::GpsFixStatus::NoData;
  for ( auto it = mConstellationFixStatus.begin(); it != mConstellationFixStatus.end(); ++it )
  {
    if ( it.value() == Qgis::GpsFixStatus::Fix3D
         || ( it.value() == Qgis::GpsFixStatus::Fix2D && bestStatus != Qgis::GpsFixStatus::Fix3D )
         || ( it.value() == Qgis::GpsFixStatus::NoFix && bestStatus == Qgis::GpsFixStatus::NoData )
       )
    {
      bestStatus = it.value();
      constellation = it.key();
    }
  }
  return bestStatus;
}

bool QgsGpsInformation::isValid() const
{
  bool valid = false;
  Qgis::GnssConstellation constellation = Qgis::GnssConstellation::Unknown;
  const Qgis::GpsFixStatus bestFix = bestFixStatus( constellation );
  if ( status == 'V'
       || bestFix == Qgis::GpsFixStatus::NoFix
       || qualityIndicator == Qgis::GpsQualityIndicator::Invalid ) // some sources say that 'V' indicates position fix, but is below acceptable quality
  {
    valid = false;
  }
  else if ( status == 'A'
            || bestFix == Qgis::GpsFixStatus::Fix2D
            || bestFix == Qgis::GpsFixStatus::Fix3D
            || ( qualityIndicator != Qgis::GpsQualityIndicator::Invalid ) ) // good
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
      return QCoreApplication::translate( "QgsGpsInformation", "Unknown (%1)" ).arg( QString::number( quality ) );
  }
  BUILTIN_UNREACHABLE
}

QVariant QgsGpsInformation::componentValue( Qgis::GpsInformationComponent component ) const
{
  if ( !isValid() )
    return QVariant();

  switch ( component )
  {
    case Qgis::GpsInformationComponent::Location:
      return QgsPointXY( longitude, latitude );

    case Qgis::GpsInformationComponent::Altitude:
      return std::isnan( elevation ) ? QVariant() : elevation;

    case Qgis::GpsInformationComponent::GeoidalSeparation:
      return std::isnan( elevation_diff ) ? QVariant() : elevation_diff;

    case Qgis::GpsInformationComponent::EllipsoidAltitude:
      return std::isnan( elevation ) || std::isnan( elevation_diff ) ? QVariant() : elevation + elevation_diff;

    case Qgis::GpsInformationComponent::GroundSpeed:
      return speed;
    case Qgis::GpsInformationComponent::Bearing:
      return std::isnan( direction ) ? QVariant() : direction;

    case Qgis::GpsInformationComponent::Pdop:
      return pdop;
    case Qgis::GpsInformationComponent::Hdop:
      return hdop;
    case Qgis::GpsInformationComponent::Vdop:
      return vdop;
    case Qgis::GpsInformationComponent::HorizontalAccuracy:
      return hacc;
    case Qgis::GpsInformationComponent::VerticalAccuracy:
      return vacc;
    case Qgis::GpsInformationComponent::HvAccuracy:
      return hvacc;
    case Qgis::GpsInformationComponent::SatellitesUsed:
      return satellitesUsed;

    case Qgis::GpsInformationComponent::Timestamp:
      return utcDateTime;

    case Qgis::GpsInformationComponent::TotalTrackLength:
    case Qgis::GpsInformationComponent::TrackDistanceFromStart:
    case Qgis::GpsInformationComponent::TrackStartTime:
    case Qgis::GpsInformationComponent::TrackEndTime:
    case Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint:
    case Qgis::GpsInformationComponent::TrackTimeSinceLastPoint:
      return QVariant(); // not available
  }
  BUILTIN_UNREACHABLE
}

