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

QTM_USE_NAMESPACE


QgsQtLocationConnection::QgsQtLocationConnection( ): QgsGPSConnection( new QLocalSocket() )
{
  qDebug("constr");
//    source = QGeoPositionInfoSource::createDefaultSource(this);
//    if (source) {
//        source->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);  //QGeoPositionInfoSource::AllPositioningMethods
//        //source->setUpdateInterval(500); // this is the line that gives trouble, so I just don't use it
//   }
//    source->startUpdates();
//    QObject::connect(source, SIGNAL(positionUpdated(QGeoPositionInfo)),
//             this, SLOT(parseData(QGeoPositionInfo)));
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

void QgsQtLocationConnection::positionUpdated(const QtMobility::QGeoPositionInfo &info)
{
  QgsDebugMsg( "Position Updated!" );
//  static int counts=0;

//  if (info.isValid())
//  {
//        counts++;
//        qDebug() << info;

//        double t;

//        t = info.coordinate().latitude();
//        t = info.coordinate().altitude();
//        t = info.coordinate().longitude();
//        t = info.GroundSpeed;
//        t = info.HorizontalAccuracy;
//        t = info.attribute(QGeoPositionInfo::HorizontalAccuracy);

//   }
}

void QgsQtLocationConnection::parseData()
{
  QgsDebugMsg( "parsing!" );
}
