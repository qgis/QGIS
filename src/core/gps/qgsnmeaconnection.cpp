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
#include "qextserialport.h"
#include "qgslogger.h"

#include <QIODevice>
#include <QApplication>
#include <QStringList>


//from libnmea
#include "parse.h"
#include "gmath.h"

#define KNOTS_TO_KMH 1.852

QgsNMEAConnection::QgsNMEAConnection( QIODevice* dev, int pollInterval ): QgsGPSConnection( dev, pollInterval )
{

}

QgsNMEAConnection::~QgsNMEAConnection()
{
  //connection will be closed by base class
}

void QgsNMEAConnection::parseData()
{
  if ( !mSource )
  {
    return;
  }

  //print out the data as a test
  int numBytes = 0;
  if ( mSource->isSequential() ) //necessary because of a bug in QExtSerialPort
  {
    numBytes = mSource->size();
  }
  else
  {
    numBytes = mSource->bytesAvailable();
  }

  QgsDebugMsg( "numBytes" );
  QgsDebugMsg( QString::number( numBytes ) );



  if ( numBytes > 0 )
  {
    if ( mStatus != GPSDataReceived )
    {
      mStatus = DataReceived;
    }

    //append new data to the remaining results from last parseData() call
    mStringBuffer.append( mSource->read( numBytes ) );
    processStringBuffer();
    emit stateChanged( mLastGPSInformation );
    qWarning( mStringBuffer.toLocal8Bit().data() );
  }
}

void QgsNMEAConnection::processStringBuffer()
{
  int endSentenceIndex = 0;
  int dollarIndex;

  while ( endSentenceIndex = mStringBuffer.indexOf( "\r\n" ) && endSentenceIndex != -1 )
  {
    endSentenceIndex = mStringBuffer.indexOf( "\r\n" );

    dollarIndex = mStringBuffer.indexOf( "$" );
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
        if ( substring.startsWith( "$GPGGA" ) )
        {
          qWarning( substring.toLocal8Bit().data() );
          processGGASentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        else if ( substring.startsWith( "$GPRMC" ) )
        {
          qWarning( substring.toLocal8Bit().data() );
          processRMCSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        else if ( substring.startsWith( "$GPGSV" ) )
        {
          qWarning( substring.toLocal8Bit().data() );
          processGSVSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        else if ( substring.startsWith( "$GPVTG" ) )
        {
          qWarning( substring.toLocal8Bit().data() );
          processVTGSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        else if ( substring.startsWith( "$GPGSA" ) )
        {
          qWarning( substring.toLocal8Bit().data() );
          processGSASentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
      }
    }
    mStringBuffer.remove( 0, endSentenceIndex + 2 );
  }
}

void QgsNMEAConnection::processGGASentence( const char* data, int len )
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
  }
}

void QgsNMEAConnection::processRMCSentence( const char* data, int len )
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
  }
}

void QgsNMEAConnection::processGSVSentence( const char* data, int len )
{
  nmeaGPGSV result;
  if ( nmea_parse_GPGSV( data, len, &result ) )
  {
    //clear satellite informations when a new series of packs arrives
    if ( result.pack_index == 1 )
    {
      mLastGPSInformation.satellitesInView.clear();
    }

    for ( int i = 0; i < NMEA_SATINPACK; ++i )
    {
      nmeaSATELLITE currentSatellite = result.sat_data[i];
      QgsSatelliteInfo satelliteInfo;
      satelliteInfo.azimuth = currentSatellite.azimuth;
      satelliteInfo.elevation = currentSatellite.elv;
      satelliteInfo.id = currentSatellite.id;
      satelliteInfo.inUse = currentSatellite.in_use;
      satelliteInfo.signal = currentSatellite.sig;
      mLastGPSInformation.satellitesInView.append( satelliteInfo );
    }

  }
}

void QgsNMEAConnection::processVTGSentence( const char* data, int len )
{
  nmeaGPVTG result;
  if ( nmea_parse_GPVTG( data, len, &result ) )
  {
    mLastGPSInformation.speed = result.spk;
  }
}

void QgsNMEAConnection::processGSASentence( const char* data, int len )
{
  nmeaGPGSA result;
  if ( nmea_parse_GPGSA( data, len, &result ) )
  {
    mLastGPSInformation.hdop = result.HDOP;
    mLastGPSInformation.pdop = result.PDOP;
    mLastGPSInformation.vdop = result.VDOP;
  }
}
