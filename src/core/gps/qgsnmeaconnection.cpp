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

  QgsDebugMsg( "numBytes:" + QString::number( numBytes ) );

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

    dollarIndex = mStringBuffer.indexOf( QLatin1String( "$" ) );
    if ( endSentenceIndex == -1 )
    {
      break;
    }


    if ( endSentenceIndex >= dollarIndex )
    {
      if ( dollarIndex != -1 )
      {
        QString substring = mStringBuffer.mid( dollarIndex, endSentenceIndex );
        QByteArray ba = substring.toLocal8Bit();
        if ( substring.startsWith( QLatin1String( "$GPGGA" ) ) )
        {
          QgsDebugMsg( substring );
          processGgaSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( QStringLiteral( "*******************GPS data received****************" ) );
        }
        else if ( substring.startsWith( QLatin1String( "$GPRMC" ) ) || substring.startsWith( QLatin1String( "$GNRMC" ) ) )
        {
          QgsDebugMsg( substring );
          processRmcSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( QStringLiteral( "*******************GPS data received****************" ) );
        }
        else if ( substring.startsWith( QLatin1String( "$GPGSV" ) ) )
        {
          QgsDebugMsg( substring );
          processGsvSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( QStringLiteral( "*******************GPS data received****************" ) );
        }
        else if ( substring.startsWith( QLatin1String( "$GPVTG" ) ) )
        {
          QgsDebugMsg( substring );
          processVtgSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( QStringLiteral( "*******************GPS data received****************" ) );
        }
        else if ( substring.startsWith( QLatin1String( "$GPGSA" ) ) )
        {
          QgsDebugMsg( substring );
          processGsaSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( QStringLiteral( "*******************GPS data received****************" ) );
        }
        else if ( substring.startsWith( QLatin1String( "$GPGST" ) ) )
        {
          QgsDebugMsg( substring );
          processGstSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( QStringLiteral( "*******************GPS data received****************" ) );
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
    mLastGPSInformation.quality = result.sig;
    mLastGPSInformation.satellitesUsed = result.satinuse;
  }
}

void QgsNmeaConnection::processGstSentence( const char *data, int len )
{
  nmeaGPGST result;
  if ( nmea_parse_GPGST( data, len, &result ) )
  {
    //update mLastGPSInformation
    double sig_lat = result.sig_lat;
    double sig_lon = result.sig_lon;
    double sig_alt = result.sig_alt;

    // Horizontal RMS
    mLastGPSInformation.hacc = sqrt( ( pow( sig_lat, 2 ) + pow( sig_lon, 2 ) ) / 2.0 );
    // Vertical RMS
    mLastGPSInformation.vacc = sig_alt;
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
    mLastGPSInformation.direction = result.direction;
    mLastGPSInformation.status = result.status;  // A,V

    //date and time
    QDate date( result.utc.year + 1900, result.utc.mon + 1, result.utc.day );
    QTime time( result.utc.hour, result.utc.min, result.utc.sec, result.utc.msec ); // added msec part
    if ( date.isValid() && time.isValid() )
    {
      mLastGPSInformation.utcDateTime.setTimeSpec( Qt::UTC );
      mLastGPSInformation.utcDateTime.setDate( date );
      mLastGPSInformation.utcDateTime.setTime( time );
      QgsDebugMsg( QStringLiteral( "utc time:" ) );
      QgsDebugMsg( mLastGPSInformation.utcDateTime.toString() );
      QgsDebugMsg( QStringLiteral( "local time:" ) );
      QgsDebugMsg( mLastGPSInformation.utcDateTime.toLocalTime().toString() );
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
      nmeaSATELLITE currentSatellite = result.sat_data[i];
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
