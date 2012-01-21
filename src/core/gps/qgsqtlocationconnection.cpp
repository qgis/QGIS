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

QgsQtLocationConnection::QgsQtLocationConnection( ): QgsGPSConnection( new QLocalSocket() )
{
  QgsDebugMsg("Creating QtLocation GPS connection");
  startGPS();
  startSatelliteMonitor();
  //HACK
  QTimer::singleShot( 500, this, SLOT( parseData() ) );
}

QgsQtLocationConnection::~QgsQtLocationConnection()
{
  //connection will be closed by base class
  QgsDebugMsg( "entered." );
}

/*Needed to make connection detectable (half HACK)*/
void QgsQtLocationConnection::parseData()
{
  if (locationDataSource){
    mStatus = GPSDataReceived;
    emit stateChanged( mLastGPSInformation );
  }
}

void QgsQtLocationConnection::positionUpdated(const QtMobility::QGeoPositionInfo &info)
{
  if (locationDataSource){
    mStatus = GPSDataReceived;
    //QGeoPositionInfo info = locationDataSource->lastKnownPosition();

    QgsDebugMsg( "Parsing locationDataSource" );

    qDebug() << info;

    if (info.isValid())
    {
      QgsDebugMsg( "Valid QGeoPositionInfo, parsing" );
      // info.HorizontalAccuracy;

      mLastGPSInformation.latitude = info.coordinate().latitude();
      mLastGPSInformation.longitude = info.coordinate().longitude() ;
      mLastGPSInformation.elevation = info.coordinate().altitude();
      mLastGPSInformation.speed = info.GroundSpeed * 3.6; // m/s to km/h
      mLastGPSInformation.direction = info.Direction;
      mLastGPSInformation.utcDateTime = info.timestamp();
      mLastGPSInformation.pdop;     //< Dilution of precision
      mLastGPSInformation.hdop;     //< Horizontal dilution of precision
      mLastGPSInformation.vdop;     //< Vertical dilution of precision
      mLastGPSInformation.fixMode;  //< Mode (M = Manual, forced to operate in 2D or 3D; A = Automatic, 3D/2D)
      mLastGPSInformation.fixType = info.coordinate().type();  //< Type, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D)
      mLastGPSInformation.quality;  //< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive)
      mLastGPSInformation.status;   //< Status (A = active or V = void)
      mLastGPSInformation.satInfoComplete = true;  // based on GPGSV sentences - to be used to determine when to graph signal and satellite position
      emit stateChanged( mLastGPSInformation );
      QgsDebugMsg("positionUpdated");
    }
  }
}

void QgsQtLocationConnection::satellitesInViewUpdated(
  const QList<QGeoSatelliteInfo>& satellites)
{
  // The number of satellites in view is updated
  mLastGPSInformation.satellitesInView.clear();
  for (int i = 0; i < satellites.size(); ++i)
  {
    QGeoSatelliteInfo currentSatellite = satellites.at(i);
    QgsSatelliteInfo satelliteInfo;
    satelliteInfo.azimuth = currentSatellite.Azimuth;
    satelliteInfo.elevation = currentSatellite.Elevation;
    satelliteInfo.id = currentSatellite.prnNumber();
    satelliteInfo.signal = currentSatellite.signalStrength();
    mLastGPSInformation.satellitesInView.append( satelliteInfo );
  }
  QgsDebugMsg("satellitesInViewUpdated");
  emit stateChanged( mLastGPSInformation );
}

void QgsQtLocationConnection::satellitesInUseUpdated(
        const QList<QGeoSatelliteInfo>& satellites) {
  // The number of satellites in use is updated
  mLastGPSInformation.satellitesUsed = QString::number(satellites.count()).toInt();

  mLastGPSInformation.satPrn.clear();
  for (int i = 0; i < satellites.size(); ++i)
  {
    QGeoSatelliteInfo currentSatellite = satellites.at(i);
    //add pnr to mLastGPSInformation.satPrn
    mLastGPSInformation.satPrn.append(currentSatellite.prnNumber());

    //set QgsSatelliteInfo.inuse to true for the satellites in use
    for (int i = 0; i < mLastGPSInformation.satellitesInView.size(); ++i)
    {
      QgsSatelliteInfo satInView = mLastGPSInformation.satellitesInView.at(i);
      if ( satInView.id == currentSatellite.prnNumber() )
      {
        satInView.inUse = true;
        break;
      }
    }
  }
  QgsDebugMsg("satellitesInUseUpdated");
  emit stateChanged( mLastGPSInformation );
}


void QgsQtLocationConnection::startGPS()
{
    // Obtain the location data source if it is not obtained already
    if (!locationDataSource)
    {
      locationDataSource = QGeoPositionInfoSource::createDefaultSource(this);
      if (locationDataSource)
      {
        locationDataSource->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);  //QGeoPositionInfoSource::AllPositioningMethods
        locationDataSource->setUpdateInterval(2000); // this is the line that gives trouble, so I just don't use it

        // Whenever the location data source signals that the current
        // position is updated, the positionUpdated function is called.
        QObject::connect(locationDataSource,
                         SIGNAL(positionUpdated(QGeoPositionInfo)),
                         this,
                         SLOT(positionUpdated(QGeoPositionInfo)));
        // Start listening for position updates
        locationDataSource->startUpdates();
      }
      else
      {
          // Not able to obtain the location data source
         QgsDebugMsg("No QtLocation Position Source");
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
  if (!satelliteInfoSource)
  {
    satelliteInfoSource = QGeoSatelliteInfoSource::createDefaultSource(this);
    if (satelliteInfoSource)
    {
      // Whenever the satellite info source signals that the number of
      // satellites in use is updated, the satellitesInUseUpdated function
      // is called
      QObject::connect(satelliteInfoSource,
                       SIGNAL(satellitesInUseUpdated(
                               const QList<QGeoSatelliteInfo>&)),
                       this,
                       SLOT(satellitesInUseUpdated(
                               const QList<QGeoSatelliteInfo>&)));

      // Whenever the satellite info source signals that the number of
      // satellites in view is updated, the satellitesInViewUpdated function
      // is called
      QObject::connect(satelliteInfoSource,
                       SIGNAL(satellitesInViewUpdated(
                               const QList<QGeoSatelliteInfo>&)),
                       this,
                       SLOT(satellitesInViewUpdated(
                               const QList<QGeoSatelliteInfo>&)));

      // Start listening for satellite updates
      satelliteInfoSource->startUpdates();
    }
    else
    {
      // Not able to obtain the Satellite data source
      QgsDebugMsg("No QtLocation Satellite Source");
    }
  }
  else
  {
    // Start listening for position updates
    satelliteInfoSource->startUpdates();
  }
}
