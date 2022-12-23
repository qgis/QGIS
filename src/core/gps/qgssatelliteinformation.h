/***************************************************************************
                          qgssatelliteinformation.h
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

#ifndef QGSSATELLITEINFORMATION_H
#define QGSSATELLITEINFORMATION_H

#include "qgis.h"
#include "qgis_core.h"

#include <QDateTime>
#include <QObject>
#include <QString>

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

    /**
     * satType value from NMEA message $GxGSV, where x:
     * P = GPS; S = SBAS (GPSid> 32 then SBasid = GPSid + 87); N = generic satellite; L = GLONASS; A = GALILEO; B = BEIDOU; Q = QZSS;
     */
    QChar satType;

    /**
     * Returns the GNSS constellation associated with the information.
     *
     * \since QGIS 3.30
     */
    Qgis::GnssConstellation constellation() const { return mConstellation; }

    bool operator==( const QgsSatelliteInfo &other ) const
    {
      return id == other.id &&
             inUse == other.inUse &&
             elevation == other.elevation &&
             azimuth == other.azimuth &&
             signal == other.signal &&
             satType == other.satType &&
             mConstellation == other.mConstellation;
    }

    bool operator!=( const QgsSatelliteInfo &other ) const
    {
      return !operator==( other );
    }

  private:

    Qgis::GnssConstellation mConstellation = Qgis::GnssConstellation::Unknown;

    friend class QgsNmeaConnection;
};

#endif // QGSSATELLITEINFORMATION_H
