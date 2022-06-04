/***************************************************************************
                          qgsnmeaconnection.cpp  -  description
                          ---------------------
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

#include "qgsnmeaconnection.h"
#include "qgslogger.h"

#include <QIODevice>
#include <QApplication>
#include <QStringList>


//from libnmea
#include "parse.h"
#include "gmath.h"
#include "info.h"

// for sqrt
#include <math.h>

#define KNOTS_TO_KMH 1.852

QgsNmeaConnection::QgsNmeaConnection( QIODevice *device )
  : QgsGpsConnection( device )
{
}

void QgsNmeaConnection::parseData()
{
  if ( !mSource )
  {
    return;
  }

  //print out the data as a test
  qint64 numBytes = 0;
  if ( ! mSource->isSequential() ) //necessary because of a bug in QExtSerialPort   //SLM - bytesAvailable() works on Windows, so I reversed the logic (added ! ); this is what QIODevice docs say to do; the orig impl of win_qextserialport had an (unsigned int)-1 return on error - it should be (qint64)-1, which was fixed by ?
  {
    numBytes = mSource->size();
  }
  else
  {
    numBytes = mSource->bytesAvailable();
  }

  QgsDebugMsgLevel( "numBytes:" + QString::number( numBytes ), 2 );

  if ( numBytes >= 6 )
  {
    if ( mStatus != GPSDataReceived )
    {
      mStatus = DataReceived;
    }

    //append new data to the remaining results from last parseData() call
    mStringBuffer.append( mSource->read( numBytes ) );
    processStringBuffer();
    emit stateChanged( mLastGPSInformation );
  }
}

void QgsNmeaConnection::processStringBuffer()
{
  int endSentenceIndex = 0;
  int dollarIndex;

  while ( ( endSentenceIndex = mStringBuffer.indexOf( QLatin1String( "\r\n" ) ) ) && endSentenceIndex != -1 )
  {
    endSentenceIndex = mStringBuffer.indexOf( QLatin1String( "\r\n" ) );

    dollarIndex = mStringBuffer.indexOf( QLatin1Char( '$' ) );
    if ( endSentenceIndex == -1 )
    {
      break;
    }


    if ( endSentenceIndex >= dollarIndex )
    {
      if ( dollarIndex != -1 )
      {
        const QString substring = mStringBuffer.mid( dollarIndex, endSentenceIndex );
        QByteArray ba = substring.toLocal8Bit();
        if ( substring.startsWith( QLatin1String( "$GPGGA" ) ) || substring.startsWith( QLatin1String( "$GNGGA" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processGgaSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else if ( substring.startsWith( QLatin1String( "$GPRMC" ) ) || substring.startsWith( QLatin1String( "$GNRMC" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processRmcSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else if ( substring.startsWith( QLatin1String( "$GPGSV" ) ) || substring.startsWith( QLatin1String( "$GNGSV" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processGsvSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else if ( substring.startsWith( QLatin1String( "$GPVTG" ) ) || substring.startsWith( QLatin1String( "$GNVTG" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processVtgSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else if ( substring.startsWith( QLatin1String( "$GPGSA" ) ) || substring.startsWith( QLatin1String( "$GNGSA" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processGsaSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else if ( substring.startsWith( QLatin1String( "$GPGST" ) ) || substring.startsWith( QLatin1String( "$GNGST" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processGstSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else if ( substring.startsWith( QLatin1String( "$GPHDT" ) ) || substring.startsWith( QLatin1String( "$GNHDT" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processHdtSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else if ( substring.startsWith( QLatin1String( "$HCHDG" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processHchdgSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else if ( substring.startsWith( QLatin1String( "$HCHDT" ) ) )
        {
          QgsDebugMsgLevel( substring, 2 );
          processHchdtSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsgLevel( QStringLiteral( "*******************GPS data received****************" ), 2 );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "unknown nmea sentence: %1" ).arg( substring ), 2 );
        }
        emit nmeaSentenceReceived( substring );  // added to be able to save raw data
      }
    }
    mStringBuffer.remove( 0, endSentenceIndex + 2 );
  }
}

void QgsNmeaConnection::processGgaSentence( const char *data, int len )
{
  nmeaGPGGA result;
  if ( nmea_parse_GPGGA( data, len, &result ) )
  {
    //update mLastGPSInformation
    double longitude = result.lon;
    if ( result.ew == 'W' )
    {
      longitude = -longitude;
    }
    double latitude = result.lat;
    if ( result.ns == 'S' )
    {
      latitude = -latitude;
    }

    mLastGPSInformation.longitude = nmea_ndeg2degree( longitude );
    mLastGPSInformation.latitude = nmea_ndeg2degree( latitude );
    mLastGPSInformation.elevation = result.elv;
    mLastGPSInformation.elevation_diff = result.diff;

    mLastGPSInformation.quality = result.sig;
    if ( result.sig >= 0 && result.sig <= 8 )
    {
      mLastGPSInformation.qualityIndicator = static_cast<Qgis::GpsQualityIndicator>( result.sig );
    }
    else
    {
      mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::Unknown;
    }

    mLastGPSInformation.satellitesUsed = result.satinuse;
  }
}

void QgsNmeaConnection::processGstSentence( const char *data, int len )
{
  nmeaGPGST result;
  if ( nmea_parse_GPGST( data, len, &result ) )
  {
    //update mLastGPSInformation
    const double sig_lat = result.sig_lat;
    const double sig_lon = result.sig_lon;
    const double sig_alt = result.sig_alt;

    // Horizontal RMS
    mLastGPSInformation.hacc = sqrt( ( pow( sig_lat, 2 ) + pow( sig_lon, 2 ) ) / 2.0 );
    // Vertical RMS
    mLastGPSInformation.vacc = sig_alt;
    // 3D RMS
    mLastGPSInformation.hvacc = sqrt( ( pow( sig_lat, 2 ) + pow( sig_lon, 2 ) + pow( sig_alt, 2 ) ) / 3.0 );
  }
}

void QgsNmeaConnection::processHdtSentence( const char *data, int len )
{
  nmeaGPHDT result;
  if ( nmea_parse_GPHDT( data, len, &result ) )
  {
    mLastGPSInformation.direction = result.heading;
  }
}

void QgsNmeaConnection::processHchdgSentence( const char *data, int len )
{
  nmeaHCHDG result;
  if ( nmea_parse_HCHDG( data, len, &result ) )
  {
    mLastGPSInformation.direction = result.mag_heading;
    if ( result.ew_variation == 'E' )
      mLastGPSInformation.direction += result.mag_variation;
    else
      mLastGPSInformation.direction -= result.mag_variation;
  }
}

void QgsNmeaConnection::processHchdtSentence( const char *data, int len )
{
  nmeaHCHDT result;
  if ( nmea_parse_HCHDT( data, len, &result ) )
  {
    mLastGPSInformation.direction = result.direction;
  }
}

void QgsNmeaConnection::processRmcSentence( const char *data, int len )
{
  nmeaGPRMC result;
  if ( nmea_parse_GPRMC( data, len, &result ) )
  {
    double longitude = result.lon;
    if ( result.ew == 'W' )
    {
      longitude = -longitude;
    }
    double latitude = result.lat;
    if ( result.ns == 'S' )
    {
      latitude = -latitude;
    }
    mLastGPSInformation.longitude = nmea_ndeg2degree( longitude );
    mLastGPSInformation.latitude = nmea_ndeg2degree( latitude );
    mLastGPSInformation.speed = KNOTS_TO_KMH * result.speed;
    if ( !std::isnan( result.direction ) )
      mLastGPSInformation.direction = result.direction;
    mLastGPSInformation.status = result.status;  // A,V

    //date and time
    const QDate date( result.utc.year + 1900, result.utc.mon + 1, result.utc.day );
    const QTime time( result.utc.hour, result.utc.min, result.utc.sec, result.utc.msec ); // added msec part
    if ( date.isValid() && time.isValid() )
    {
      mLastGPSInformation.utcDateTime.setTimeSpec( Qt::UTC );
      mLastGPSInformation.utcDateTime.setDate( date );
      mLastGPSInformation.utcDateTime.setTime( time );
      QgsDebugMsgLevel( QStringLiteral( "utc time:" ), 2 );
      QgsDebugMsgLevel( mLastGPSInformation.utcDateTime.toString(), 2 );
      QgsDebugMsgLevel( QStringLiteral( "local time:" ), 2 );
      QgsDebugMsgLevel( mLastGPSInformation.utcDateTime.toLocalTime().toString(), 2 );
    }

    // convert mode to signal (aka quality) indicator
    // (see https://gitlab.com/fhuberts/nmealib/-/blob/master/src/info.c#L27)
    if ( result.status == 'A' )
    {
      if ( result.mode == 'A' )
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::GPS );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::GPS;
      }
      else if ( result.mode == 'D' )
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::DGPS );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::DGPS;
      }
      else if ( result.mode == 'P' )
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::PPS );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::PPS;
      }
      else if ( result.mode == 'R' )
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::RTK );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::RTK;
      }
      else if ( result.mode == 'F' )
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::FloatRTK );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::FloatRTK;
      }
      else if ( result.mode == 'E' )
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::Estimated );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::Estimated;
      }
      else if ( result.mode == 'M' )
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::Manual );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::Manual;
      }
      else if ( result.mode == 'S' )
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::Simulation );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::Simulation;
      }
      else
      {
        mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::Unknown );
        mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::Unknown;
      }
    }
    else
    {
      mLastGPSInformation.quality = static_cast<int>( Qgis::GpsQualityIndicator::Invalid );
      mLastGPSInformation.qualityIndicator = Qgis::GpsQualityIndicator::Invalid;
    }
  }
}

void QgsNmeaConnection::processGsvSentence( const char *data, int len )
{
  nmeaGPGSV result;
  if ( nmea_parse_GPGSV( data, len, &result ) )
  {
    //clear satellite information when a new series of packs arrives
    if ( result.pack_index == 1 )
    {
      mLastGPSInformation.satellitesInView.clear();
    }

    // for determining when to graph sat info
    mLastGPSInformation.satInfoComplete = ( result.pack_index == result.pack_count );

    for ( int i = 0; i < NMEA_SATINPACK; ++i )
    {
      const nmeaSATELLITE currentSatellite = result.sat_data[i];
      QgsSatelliteInfo satelliteInfo;
      satelliteInfo.azimuth = currentSatellite.azimuth;
      satelliteInfo.elevation = currentSatellite.elv;
      satelliteInfo.id = currentSatellite.id;
      satelliteInfo.inUse = currentSatellite.in_use; // the GSA processing below does NOT set the sats in use
      satelliteInfo.signal = currentSatellite.sig;
      mLastGPSInformation.satellitesInView.append( satelliteInfo );
    }

  }
}

void QgsNmeaConnection::processVtgSentence( const char *data, int len )
{
  nmeaGPVTG result;
  if ( nmea_parse_GPVTG( data, len, &result ) )
  {
    mLastGPSInformation.speed = result.spk;
  }
}

void QgsNmeaConnection::processGsaSentence( const char *data, int len )
{
  nmeaGPGSA result;
  if ( nmea_parse_GPGSA( data, len, &result ) )
  {
    mLastGPSInformation.satPrn.clear();
    mLastGPSInformation.hdop = result.HDOP;
    mLastGPSInformation.pdop = result.PDOP;
    mLastGPSInformation.vdop = result.VDOP;
    mLastGPSInformation.fixMode = result.fix_mode;
    mLastGPSInformation.fixType = result.fix_type;
    for ( int i = 0; i < NMEA_MAXSAT; i++ )
    {
      mLastGPSInformation.satPrn.append( result.sat_prn[ i ] );
    }
  }
}
