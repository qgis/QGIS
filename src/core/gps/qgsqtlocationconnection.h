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

#ifndef SIP_RUN
#if defined(HAVE_QT_MOBILITY_LOCATION )
#include <QtLocation/QGeoPositionInfoSource>
#include <QtLocation/QGeoSatelliteInfo>
#include <QtLocation/QGeoSatelliteInfoSource>

QTM_USE_NAMESPACE
#else // Using QtPositioning
#include <QtPositioning/QGeoPositionInfoSource>
#include <QtPositioning/QGeoSatelliteInfo>
#include <QtPositioning/QGeoSatelliteInfoSource>
#endif
#endif

SIP_FEATURE( MOBILITY_LOCATION )

SIP_IF_FEATURE( MOBILITY_LOCATION )

/**
 * \ingroup core
 * \class QgsQtLocationConnection
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

#ifdef SIP_RUN
    SIP_IF_FEATURE( !ANDROID )
#endif

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

#ifdef SIP_RUN
    SIP_END
#endif

  private:
    void startGPS();
    void startSatelliteMonitor();
    QString mDevice;
    QGeoPositionInfo mInfo;
    QPointer<QGeoPositionInfoSource> locationDataSource;
    QPointer<QGeoSatelliteInfoSource> satelliteInfoSource;

};

SIP_END // MOBILITY_LOCATION

#endif // QGSQTLOCATIONCONNECTION_H
