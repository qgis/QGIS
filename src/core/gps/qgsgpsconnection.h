/***************************************************************************
                          qgsgpsconnection.h  -  description
                          -------------------
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

#ifndef QGSGPSCONNECTION_H
#define QGSGPSCONNECTION_H

#include <QDateTime>
#include "qgis.h"
#include <QObject>
#include <QString>

#include "qgis_core.h"

class QIODevice;

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsgpsconnection.h"
% End
#endif

/**
 * \ingroup core
 * \class QgsSatelliteInfo
 * \brief Encapsulates information relating to a GPS satellite.
*/
class CORE_EXPORT QgsSatelliteInfo
{
  public:

    /**
     * Contains the satellite identifier number.
     *
     * The satellite identifier number can be used to identify a satellite inside the satellite system.
     * For satellite system GPS the satellite identifier number represents the PRN (Pseudo-random noise)
     * number. For satellite system GLONASS the satellite identifier number represents the slot number.
     */
    int id = 0;

    /**
     * TRUE if satellite was used in obtaining the position fix.
     */
    bool inUse = false;

#ifndef SIP_RUN

    /**
     * Elevation of the satellite, in degrees.
     */
    double elevation = std::numeric_limits< double >::quiet_NaN();
#else

    /**
     * Elevation of the satellite, in degrees.
     */
    double elevation;
#endif

#ifndef SIP_RUN

    /**
     * The azimuth of the satellite to true north, in degrees.
     */
    double azimuth = std::numeric_limits< double >::quiet_NaN();
#else

    /**
     * The azimuth of the satellite to true north, in degrees.
     */
    double azimuth;
#endif

    /**
     * Signal strength (0-99dB), or -1 if not available.
     */
    int signal = -1;
};

/**
 * \ingroup core
 * \class QgsGpsInformation
 * \brief Encapsulates information relating to a GPS position fix.
*/
class CORE_EXPORT QgsGpsInformation
{
  public:

    /**
     * GPS fix status
     * \since QGIS 3.10
     */
    enum FixStatus
    {
      NoData,
      NoFix,
      Fix2D,
      Fix3D
    };

    /**
     * Latitude in decimal degrees, using the WGS84 datum. A positive value indicates the Northern Hemisphere, and
     * a negative value indicates the Southern Hemisphere.
     */
    double latitude = 0;

    /**
     * Longitude in decimal degrees, using the WGS84 datum. A positive value indicates the Eastern Hemisphere, and
     * a negative value indicates the Western Hemisphere.
     */
    double longitude = 0;

    /**
     * Altitude (in meters) above or below the mean sea level.
     */
    double elevation = 0;

    /**
     * Geoidal separation (Diff. between WGS-84 earth ellipsoid and
     * mean sea level.
     *
     * \since QGIS 3.18
     */
    double elevation_diff = 0;

    /**
     * Ground speed, in km/h.
     */
    double speed = 0;

#ifndef SIP_RUN

    /**
     * The bearing measured in degrees clockwise from true north to the direction of travel.
     */
    double direction = std::numeric_limits< double >::quiet_NaN();
#else

    /**
     * The bearing measured in degrees clockwise from true north to the direction of travel.
     */
    double direction;
#endif

    /**
     * Contains a list of information relating to the current satellites in view.
     */
    QList<QgsSatelliteInfo> satellitesInView;

    /**
     * Dilution of precision.
     */
    double pdop = 0;

    /**
     * Horizontal dilution of precision.
     */
    double hdop = 0;

    /**
     * Vertical dilution of precision.
     */
    double vdop = 0;

#ifndef SIP_RUN
    //! Horizontal accuracy in meters
    double hacc = std::numeric_limits< double >::quiet_NaN();
    //! Vertical accuracy in meters
    double vacc = std::numeric_limits< double >::quiet_NaN();

    /**
     * 3D RMS
     * \since QGIS 3.18
     */
    double hvacc = std::numeric_limits< double >::quiet_NaN();
#else
    //! Horizontal accuracy in meters
    double hacc;
    //! Vertical accuracy in meters
    double vacc;

    /**
     * 3D RMS
     * \since QGIS 3.18
     */
    double hvacc;
#endif

    /**
     * The date and time at which this position was reported, in UTC time.
     */
    QDateTime utcDateTime;

    /**
     * Fix mode (where M = Manual, forced to operate in 2D or 3D or A = Automatic, 3D/2D)
     */
    QChar fixMode;

    /**
     * Contains the fix type, where 1 = no fix, 2 = 2d fix, 3 = 3d fix
     */
    int fixType = 0;

    /**
     * GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive, etc.)
     * \deprecated use qualityIndicator instead
     */
    int quality = -1;

    /**
     * Returns the signal quality indicator
     * \since QGIS 3.22.6
     */
    Qgis::GpsQualityIndicator qualityIndicator = Qgis::GpsQualityIndicator::Unknown;

    /**
     * Count of satellites used in obtaining the fix.
     */
    int satellitesUsed = 0;

    /**
     * Status (A = active or V = void)
     */
    QChar status;

    /**
     * IDs of satellites used in the position fix.
     */
    QList<int> satPrn;

    /**
     * TRUE if satellite information is complete.
     */
    bool satInfoComplete = false;

    /**
     * Returns whether the connection information is valid
     * \since QGIS 3.10
     */
    bool isValid() const;

    /**
     * Returns the fix status
     * \since QGIS 3.10
     */
    FixStatus fixStatus() const;

    /**
     * Returns a descriptive string for the signal quality.
     *
     * \since QGIS 3.16
     */
    QString qualityDescription() const;
};

/**
 * \ingroup core
 * \brief Abstract base class for connection to a GPS device
*/
class CORE_EXPORT QgsGpsConnection : public QObject
{
#ifdef SIP_RUN
#include <qgsgpsdconnection.h>
#include <qgsnmeaconnection.h>
#endif


#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->inherits( "QgsGpsdConnection" ) )
      sipType = sipType_QgsGpsdConnection;
    else if ( sipCpp->inherits( "QgsNmeaConnection" ) )
      sipType = sipType_QgsNmeaConnection;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT
  public:

    enum Status
    {
      NotConnected,
      Connected,
      DataReceived,
      GPSDataReceived
    };

    /**
     * Constructor
     * \param dev input device for the connection (e.g. serial device). The class takes ownership of the object
     */
    QgsGpsConnection( QIODevice *dev SIP_TRANSFER );
    ~QgsGpsConnection() override;
    //! Opens connection to device
    bool connect();
    //! Closes connection to device
    bool close();

    //! Sets the GPS source. The class takes ownership of the device class
    void setSource( QIODevice *source SIP_TRANSFER );

    //! Returns the status. Possible state are not connected, connected, data received
    Status status() const { return mStatus; }

    //! Returns the current gps information (lat, lon, etc.)
    QgsGpsInformation currentGPSInformation() const { return mLastGPSInformation; }

  signals:
    void stateChanged( const QgsGpsInformation &info );
    void nmeaSentenceReceived( const QString &substring ); // added to capture 'raw' data

  protected:
    //! Data source (e.g. serial device, socket, file,...)
    std::unique_ptr< QIODevice > mSource;
    //! Last state of the gps related variables (e.g. position, time, ...)
    QgsGpsInformation mLastGPSInformation;
    //! Connection status
    Status mStatus = NotConnected;

  private:
    //! Closes and deletes mSource
    void cleanupSource();
    void clearLastGPSInformation();

  protected slots:
    //! Parse available data source content
    virtual void parseData() = 0;
};

#endif // QGSGPSCONNECTION_H
