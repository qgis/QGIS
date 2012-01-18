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
  qDebug("constructor");
  source = QGeoPositionInfoSource::createDefaultSource(this);
  if (source)
  {
    source->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);  //QGeoPositionInfoSource::AllPositioningMethods
    //source->setUpdateInterval(500); // this is the line that gives trouble, so I just don't use it
    source->startUpdates();
    QObject::connect(source, SIGNAL(positionUpdated(QGeoPositionInfo)),
             this, SLOT(parseData()));
 }
 else
 {
   qDebug("No QtLocation Source");
 }
  //HACK
  QTimer::singleShot( 500, this, SLOT( parseData() ) );
}

QgsQtLocationConnection::~QgsQtLocationConnection()
{
  //connection will be closed by base class
  QgsDebugMsg( "entered." );
}


void QgsQtLocationConnection::parseData()
{
  QgsDebugMsg( "parsing!" );
  mStatus = GPSDataReceived;

  if (source){
    QGeoPositionInfo info = source->lastKnownPosition();
    if (info.isValid())
    {
      QgsDebugMsg( "Valid QGeoPositionInfo!" );
   //        t = info.HorizontalAccuracy;
   //        t = info.attribute(QGeoPositionInfo::HorizontalAccuracy);

      mLastGPSInformation.latitude = info.coordinate().latitude();
      mLastGPSInformation.longitude = info.coordinate().longitude() ;
      mLastGPSInformation.elevation = info.coordinate().altitude();
      mLastGPSInformation.speed = info.GroundSpeed * 3.6; // m/s to km/h
      mLastGPSInformation.direction = info.Direction;
      QList<QgsSatelliteInfo> satellitesInView;
      mLastGPSInformation.pdop;
      mLastGPSInformation.hdop;
      mLastGPSInformation.vdop;
      mLastGPSInformation.utcDateTime;
      mLastGPSInformation.fixMode;
      mLastGPSInformation.fixType;
      mLastGPSInformation.quality;      // from GPGGA
      mLastGPSInformation.satellitesUsed; // from GPGGA
      mLastGPSInformation.status;     // from GPRMC A,V
      QList<int>satPrn; // list of SVs in use; needed for QgsSatelliteInfo.inUse and other uses
      mLastGPSInformation.satInfoComplete;  // based on GPGSV sentences - to be used to determine when to graph signal and satellite position
    }
  }
  emit stateChanged( mLastGPSInformation );
}
