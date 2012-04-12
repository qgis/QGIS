/***************************************************************************
                          QgsQtLocationConnection.h  -  description
                          -------------------
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

#ifndef QGSQTLOCATIONCONNECTION_H
#define QGSQTLOCATIONCONNECTION_H

#include "qgsgpsconnection.h"
#include <QtCore/QPointer>
#include <QtLocation/QGeoPositionInfoSource>
#include <QtLocation/QGeoSatelliteInfo>
#include <QtLocation/QGeoSatelliteInfoSource>

QTM_USE_NAMESPACE

class CORE_EXPORT QgsQtLocationConnection: public QgsGPSConnection
{
    Q_OBJECT
  public:
    QgsQtLocationConnection();
    ~QgsQtLocationConnection();

  protected slots:
    /**Needed to make QtLocation detected*/
    void broadcastConnectionAvailable( );

    /**Parse available data source content*/
    void parseData();

    /**Called when the position updated.*/
    void positionUpdated( const QGeoPositionInfo &info );

    /**Called when the number of satellites in view is updated.*/
    void satellitesInViewUpdated( const QList<QGeoSatelliteInfo>& satellites );

    /**Called when the number of satellites in use is updated.*/
    void satellitesInUseUpdated( const QList<QGeoSatelliteInfo>& satellites );

  private:
    void startGPS();
    void startSatelliteMonitor();
    QString mDevice;
    QGeoPositionInfo mInfo;
    QPointer<QGeoPositionInfoSource> locationDataSource;
    QPointer<QGeoSatelliteInfoSource> satelliteInfoSource;

};

#endif // QGSQTLOCATIONCONNECTION_H
