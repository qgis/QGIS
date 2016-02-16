/***************************************************************************
                          QgsQtLocationConnection.cpp  -  description
                          ---------------------
    begin                : December 7th, 2011
    copyright            : (C) 2011 by Marco Bernasocchi, Bernawebdesign.ch
    email                : marco at bernawebdesign dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsqtlocationconnection.h"
#include "qgslogger.h"

#include <QLocalSocket>
#include <QTimer>
#include <QMetaType>

QgsQtLocationConnection::QgsQtLocationConnection(): QgsGPSConnection( new QLocalSocket() )
{
  //needed to fix https://sourceforge.net/p/necessitas/tickets/146/
  qRegisterMetaType< QList<QGeoSatelliteInfo> >( "QList<QGeoSatelliteInfo>" );

  startSatelliteMonitor();
  startGPS();

  //HACK to signal the gpsinformationwidget that we have a QtLocationConnection
  QTimer::singleShot( 500, this, SLOT( broadcastConnectionAvailable() ) );
}

QgsQtLocationConnection::~QgsQtLocationConnection()
{
  //connection will be closed by base class
  QgsDebugMsg( "entered." );
}

//Needed to make connection detectable (half HACK)
//this signals that the device has started the GPS sucessfully,
//not that it has a fix yet.
void QgsQtLocationConnection::broadcastConnectionAvailable()
{
  if ( locationDataSource )
  {
    mStatus = GPSDataReceived;
    emit stateChanged( mLastGPSInformation );
  }
}

//TODO: Temporarely needed to workaround https://sourceforge.net/p/necessitas/tickets/147/
void QgsQtLocationConnection::positionUpdated( const QGeoPositionInfo &info )
{
  mInfo = info;
  parseData();
}

void QgsQtLocationConnection::parseData()
{
  if ( locationDataSource )
  {
    mStatus = GPSDataReceived;
    //const QGeoPositionInfo &info = locationDataSource->lastKnownPosition();
    if ( mInfo.isValid() )
    {
      // mInfo.HorizontalAccuracy;
      mLastGPSInformation.latitude = mInfo.coordinate().latitude();
      mLastGPSInformation.longitude = mInfo.coordinate().longitude();
      mLastGPSInformation.elevation = mInfo.coordinate().altitude();
      mLastGPSInformation.speed = mInfo.attribute( QGeoPositionInfo::GroundSpeed ) * 3.6; // m/s to km/h
      mLastGPSInformation.direction = mInfo.attribute( QGeoPositionInfo::Direction );
      mLastGPSInformation.utcDateTime = mInfo.timestamp();
      mLastGPSInformation.fixType = mInfo.coordinate().type() + 1;
      //< fixType, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D)
      //< coordinate().type(), returns 0 = Fix not available; 1 = 2D; 2 = 3D)
      mLastGPSInformation.hacc = mInfo.attribute( QGeoPositionInfo::HorizontalAccuracy );   //< Horizontal dilution of precision
      mLastGPSInformation.vacc = mInfo.attribute( QGeoPositionInfo::VerticalAccuracy );   //< Vertical dilution of precision

      //TODO implement dop maybe by getting a
      //http://developer.android.com/reference/android/location/GpsStatus.NmeaListener.html
      //http://doc.qt.nokia.com/qtmobility-1.1/qnmeapositioninfosource.html
      //into QtLocation and subclass QgsNMEAConnection directly?
      mLastGPSInformation.pdop;     //< Dilution of precision
      mLastGPSInformation.hdop;     //< Horizontal dilution of precision
      mLastGPSInformation.vdop;     //< Vertical dilution of precision

      mLastGPSInformation.fixMode;  //< Mode (M = Manual, forced to operate in 2D or 3D; A = Automatic, 3D/2D)
      mLastGPSInformation.quality;  //< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive)
      mLastGPSInformation.status;   //< Status (A = active or V = void)

      emit stateChanged( mLastGPSInformation );
      QgsDebugMsg( "Valid QGeoPositionInfo, positionUpdated" );
    }
  }
}

void QgsQtLocationConnection::satellitesInViewUpdated(
  const QList<QGeoSatelliteInfo>& satellites )
{
  // The number of satellites in view is updated
  mLastGPSInformation.satellitesInView.clear();
  for ( int i = 0; i < satellites.size(); ++i )
  {
    QGeoSatelliteInfo currentSatellite = satellites.at( i );
    QgsSatelliteInfo satelliteInfo;
    satelliteInfo.azimuth = currentSatellite.attribute( QGeoSatelliteInfo::Azimuth );
    satelliteInfo.elevation = currentSatellite.attribute( QGeoSatelliteInfo::Elevation );
#if defined(HAVE_QT_MOBILITY_LOCATION )
    satelliteInfo.id = currentSatellite.prnNumber();
#else // QtPositioning
    satelliteInfo.id = currentSatellite.satelliteIdentifier();
#endif
    satelliteInfo.signal = currentSatellite.signalStrength();
    mLastGPSInformation.satellitesInView.append( satelliteInfo );
  }
  mLastGPSInformation.satInfoComplete = true;  //to be used to determine when to graph signal and satellite position
  emit stateChanged( mLastGPSInformation );
  QgsDebugMsg( "satellitesInViewUpdated" );
}

void QgsQtLocationConnection::satellitesInUseUpdated(
  const QList<QGeoSatelliteInfo>& satellites )
{
  // The number of satellites in use is updated
  mLastGPSInformation.satellitesUsed = QString::number( satellites.count() ).toInt();

  mLastGPSInformation.satPrn.clear();
  for ( int i = 0; i < satellites.size(); ++i )
  {
    QGeoSatelliteInfo currentSatellite = satellites.at( i );
    //add pnr to mLastGPSInformation.satPrn
#if defined(HAVE_QT_MOBILITY_LOCATION )
    mLastGPSInformation.satPrn.append( currentSatellite.prnNumber() );
#else // QtPositioning
    mLastGPSInformation.satPrn.append( currentSatellite.satelliteIdentifier() );
#endif

    //set QgsSatelliteInfo.inuse to true for the satellites in use
    for ( int i = 0; i < mLastGPSInformation.satellitesInView.size(); ++i )
    {
      QgsSatelliteInfo satInView = mLastGPSInformation.satellitesInView.at( i );
#if defined(HAVE_QT_MOBILITY_LOCATION )
      if ( satInView.id == currentSatellite.prnNumber() )
#else // QtPositioning
      if ( satInView.id == currentSatellite.satelliteIdentifier() )
#endif
      {
        satInView.inUse = true;
        break;
      }
    }
  }
  mLastGPSInformation.satInfoComplete = true;  //to be used to determine when to graph signal and satellite position
  emit stateChanged( mLastGPSInformation );
  QgsDebugMsg( "satellitesInUseUpdated" );
}

void QgsQtLocationConnection::startGPS()
{
  QgsDebugMsg( "Starting GPS QtLocation connection" );
  // Obtain the location data source if it is not obtained already
  if ( !locationDataSource )
  {
    locationDataSource = QGeoPositionInfoSource::createDefaultSource( this );
    if ( locationDataSource )
    {
      locationDataSource->setPreferredPositioningMethods( QGeoPositionInfoSource::SatellitePositioningMethods );  //QGeoPositionInfoSource::AllPositioningMethods
      locationDataSource->setUpdateInterval( 1000 );
      // Whenever the location data source signals that the current
      // position is updated, the positionUpdated function is called.
      QObject::connect( locationDataSource,
                        SIGNAL( positionUpdated( QGeoPositionInfo ) ),
                        this,
                        SLOT( positionUpdated( QGeoPositionInfo ) ) );
      // Start listening for position updates
      locationDataSource->startUpdates();
    }
    else
    {
      // Not able to obtain the location data source
      QgsDebugMsg( "No QtLocation Position Source" );
    }
  }
  else
  {
    // Start listening for position updates
    locationDataSource->startUpdates();
  }
}

void QgsQtLocationConnection::startSatelliteMonitor()
{
  QgsDebugMsg( "Starting GPS QtLocation satellite monitor" );

  if ( !satelliteInfoSource )
  {
    satelliteInfoSource = QGeoSatelliteInfoSource::createDefaultSource( this );
    if ( satelliteInfoSource )
    {
      QgsDebugMsg( "satelliteMonitor started" );
      // Whenever the satellite info source signals that the number of
      // satellites in use is updated, the satellitesInUseUpdated function
      // is called
      QObject::connect( satelliteInfoSource,
                        SIGNAL( satellitesInUseUpdated(
                                  const QList<QGeoSatelliteInfo>& ) ),
                        this,
                        SLOT( satellitesInUseUpdated(
                                const QList<QGeoSatelliteInfo>& ) ) );

      // Whenever the satellite info source signals that the number of
      // satellites in view is updated, the satellitesInViewUpdated function
      // is called
      QObject::connect( satelliteInfoSource,
                        SIGNAL( satellitesInViewUpdated(
                                  const QList<QGeoSatelliteInfo>& ) ),
                        this,
                        SLOT( satellitesInViewUpdated(
                                const QList<QGeoSatelliteInfo>& ) ) );

      // Start listening for satellite updates
      satelliteInfoSource->startUpdates();
    }
    else
    {
      // Not able to obtain the Satellite data source
      QgsDebugMsg( "No QtLocation Satellite Source" );
    }
  }
  else
  {
    // Start listening for position updates
    satelliteInfoSource->startUpdates();
  }
}
