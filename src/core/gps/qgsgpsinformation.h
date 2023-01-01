/***************************************************************************
                          qgsgpsinformation.h
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

#ifndef QGSGPSINFORMATION_H
#define QGSGPSINFORMATION_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgssatelliteinformation.h"

#include <QDateTime>
#include <QObject>
#include <QString>


/**
 * \ingroup core
 * \class QgsGpsInformation
 * \brief Encapsulates information relating to a GPS position fix.
*/
class CORE_EXPORT QgsGpsInformation
{
  public:

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
     * Geoidal separation (in meters).
     *
     * The difference between the WGS-84 Earth ellipsoid and the mean sea level (geoid).
     *
     * Negative values indicate that mean sea level is below the ellipsoid.
     *
     * This value can be added to the elevation value to obtain the geoidal elevation.
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
     * The time at which this position was reported, in UTC time.
     * \since QGIS 3.30
     */
    QTime utcTime;

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
     *
     * \deprecated, use constellationFixStatus() or bestFixStatus() instead.
     */
    int fixType = 0;

    /**
     * Returns a map of GNSS constellation to fix status.
     *
     * \since QGIS 3.30
     */
    QMap< Qgis::GnssConstellation, Qgis::GpsFixStatus > constellationFixStatus() const { return mConstellationFixStatus; }

    /**
     * Returns the best fix status and corresponding constellation.
     *
     * \param constellation will be set to the constellation with best fix status
     * \returns best current fix status
     *
     * \since QGIS 3.30
     */
    Qgis::GpsFixStatus bestFixStatus( Qgis::GnssConstellation &constellation SIP_OUT ) const;

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
     * \deprecated, use constellationFixStatus() or bestFixStatus() instead.
     */
    Q_DECL_DEPRECATED Qgis::GpsFixStatus fixStatus() const SIP_DEPRECATED;

    /**
     * Returns a descriptive string for the signal quality.
     *
     * \since QGIS 3.16
     */
    QString qualityDescription() const;

    /**
     * Returns the value of the corresponding GPS information \a component.
     *
     * \since QGIS 3.30
     */
    QVariant componentValue( Qgis::GpsInformationComponent component ) const;

  private:

    QMap< Qgis::GnssConstellation, Qgis::GpsFixStatus > mConstellationFixStatus;

    friend class QgsNmeaConnection;
    friend class QgsQtLocationConnection;

};

Q_DECLARE_METATYPE( QgsGpsInformation )

#endif // QGSGPSINFORMATION_H
