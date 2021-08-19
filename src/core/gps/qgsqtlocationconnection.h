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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsgpsconnection.h"

#include <QtCore/QPointer>

#include <QtPositioning/QGeoPositionInfoSource>
#include <QtPositioning/QGeoSatelliteInfo>
#include <QtPositioning/QGeoSatelliteInfoSource>

/**
 * \ingroup core
 * \class QgsQtLocationConnection
 * \brief A GPS connection subclass based on the Qt Location libraries.
 * \note may not be available in Python bindings on all platforms
*/
class CORE_EXPORT QgsQtLocationConnection: public QgsGpsConnection
{
    Q_OBJECT
  public:
    QgsQtLocationConnection();

  protected slots:
    //! Needed to make QtLocation detected
    void broadcastConnectionAvailable();

    //! Parse available data source content
    void parseData() override;

    /**
     * Called when the position updated.
      * \note not available in Python bindings
      */
    void positionUpdated( const QGeoPositionInfo &info ) SIP_SKIP;

    /**
     * Called when the number of satellites in view is updated.
      * \note not available in Python bindings on android
      */
    void satellitesInViewUpdated( const QList<QGeoSatelliteInfo> &satellites );

    /**
     * Called when the number of satellites in use is updated.
      * \note not available in Python bindings on android
      */
    void satellitesInUseUpdated( const QList<QGeoSatelliteInfo> &satellites );

  private:
    void startGPS();
    void startSatelliteMonitor();
    QString mDevice;
    QGeoPositionInfo mInfo;
    QPointer<QGeoPositionInfoSource> locationDataSource;
    QPointer<QGeoSatelliteInfoSource> satelliteInfoSource;

};

#endif // QGSQTLOCATIONCONNECTION_H
