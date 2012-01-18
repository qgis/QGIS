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
    QgsGPSInformation info;
    info.latitude = 40;
    info.longitude = 10;
    mStatus = QgsGPSConnection::GPSDataReceived;
    emit stateChanged( info );
 }
 else
 {
   qDebug("No Source");
 }
// char buffer [50];
// sprintf(buffer, "Update int:%d",source->updateInterval());
// qDebug(buffer);
}

QgsQtLocationConnection::~QgsQtLocationConnection()
{
  //connection will be closed by base class
  QgsDebugMsg( "entered." );
}


void QgsQtLocationConnection::connected()
{
  QgsDebugMsg( "connected!" );
}

void QgsQtLocationConnection::error(  )
{
}

void QgsQtLocationConnection::parseData()
{
  QgsDebugMsg( "parsing!" );
  QGeoPositionInfo info = source->lastKnownPosition();
  if (info.isValid())
  {
    QgsDebugMsg( "Valid QGeoPositionInfo!" );
 //        counts++;
 //        qDebug() << info;

 //        double t;

 //        t = info.coordinate().latitude();
 //        t = info.coordinate().altitude();
 //        t = info.coordinate().longitude();
 //        t = info.GroundSpeed;
 //        t = info.HorizontalAccuracy;
 //        t = info.attribute(QGeoPositionInfo::HorizontalAccuracy);

  }
}
