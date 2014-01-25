/***************************************************************************
                          qgsgpsudconnection.cpp  -  description
                          ---------------------
    begin                : January 22nd, 2014
    copyright            : (C) 2010 by Jürgen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpsudpconnection.h"
#include "qgslogger.h"

#include <QUdpSocket>
#include <QTimer>

// QgsGpsUdpConnection::QgsGpsUdpConnection( QString host, qint16 port, 
// QString device )
//     : QgsGPSConnection( new QUdpSocket() )
//     , mDevice( device )
QgsGpsUdpConnection::QgsGpsUdpConnection(qint16 port)
    : QgsGPSConnection( new QUdpSocket() )
    , mDevice()

{
    QUdpSocket *socket = qobject_cast< QUdpSocket * >( mSource );
 
    QObject::connect( socket, SIGNAL( readyRead() ), this, SLOT( parseData() ) );

    QObject::connect( socket, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( error( QAbstractSocket::SocketError ) ) );
    QgsDebugMsg("Listening to UDP port: " + port );
    socket->bind( port );


}

QgsGpsUdpConnection::~QgsGpsUdpConnection()
{
  //connection will be closed by base class
  QgsDebugMsg( "entered." );
}


void QgsGpsUdpConnection::error( QAbstractSocket::SocketError socketError )
{
#if QGISDEBUG
  QUdpSocket *socket = qobject_cast< QUdpSocket * >( mSource );
  QgsDebugMsg( QString( "error: %1 %2" ).arg( socketError ).arg( socket->errorString() ) );
#else
  Q_UNUSED( socketError );
#endif
}

void QgsGpsUdpConnection::parseData()
{
    QgsDebugMsg("Parsing UDP Data");
    QByteArray datagram;
    int datagramSize;
    QgsGpsUdpConnection::timer.start();
    
    
    QUdpSocket *socket = qobject_cast< QUdpSocket * >( mSource );

    do {
        
       
        datagramSize = socket->pendingDatagramSize();
        datagram.resize( datagramSize );
        socket->readDatagram( datagram.data(), datagram.size() );
        QgsDebugMsg( datagram );
        //not sure if this has to be done
        if ( datagramSize == -1 ){
            mStatus=NotConnected;
        }
        else if ( datagramSize >= 6 ){
            QgsDebugMsg( "Datagram size OK!!" );
            QgsDebugMsg( datagram.data() );
            mStringBuffer = datagram;
            mStringBuffer.append( datagram );
            // If the datagram does not have line end, we add it.
            // So we take advantage of the existing processStringBuffer method
            if ( mStringBuffer.indexOf( "\r\n" ) == -1 ){
                mStringBuffer.append( "\r\n" );
            }
            processStringBuffer();
            emit stateChanged( mLastGPSInformation );
        }
         
    } while ( socket->hasPendingDatagrams() );
}

void QgsGpsUdpConnection::processStringBuffer()
{
  int endSentenceIndex = 0;
  int dollarIndex;

  while (( endSentenceIndex = mStringBuffer.indexOf( "\r\n" ) ) && endSentenceIndex != -1 )
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
          QgsDebugMsg( substring );
          processGGASentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        else if ( substring.startsWith( "$GPRMC" ) )
        {
          QgsDebugMsg( substring );
          processRMCSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        else if ( substring.startsWith( "$GPGSV" ) )
        {
          QgsDebugMsg( substring );
          processGSVSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        else if ( substring.startsWith( "$GPVTG" ) )
        {
          QgsDebugMsg( substring );
          processVTGSentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        else if ( substring.startsWith( "$GPGSA" ) )
        {
          QgsDebugMsg( substring );
          processGSASentence( ba.data(), ba.length() );
          mStatus = GPSDataReceived;
          QgsDebugMsg( "*******************GPS data received****************" );
        }
        emit nmeaSentenceReceived( substring );  // added to be able to save raw data
      }
    }
    mStringBuffer.remove( 0, endSentenceIndex + 2 );
  }
}

void QgsGpsUdpConnection::processGGASentence( const char* data, int len )
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

void QgsGpsUdpConnection::processRMCSentence( const char* data, int len )
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
      QgsDebugMsg( "utc time:" );
      QgsDebugMsg( mLastGPSInformation.utcDateTime.toString() );
      QgsDebugMsg( "local time:" );
      QgsDebugMsg( mLastGPSInformation.utcDateTime.toLocalTime().toString() );
    }
  }
}

void QgsGpsUdpConnection::processGSVSentence( const char* data, int len )
{
  nmeaGPGSV result;
  if ( nmea_parse_GPGSV( data, len, &result ) )
  {
    //clear satellite informations when a new series of packs arrives
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

void QgsGpsUdpConnection::processVTGSentence( const char* data, int len )
{
  nmeaGPVTG result;
  if ( nmea_parse_GPVTG( data, len, &result ) )
  {
    mLastGPSInformation.speed = result.spk;
  }
}

void QgsGpsUdpConnection::processGSASentence( const char* data, int len )
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

