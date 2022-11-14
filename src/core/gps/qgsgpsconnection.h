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
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgspoint.h"

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

    //! Settings entry GPS connection type
    static const inline QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType> settingsGpsConnectionType = QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType>( QStringLiteral( "gps-connection-type" ), QgsSettings::Prefix::GPS, Qgis::GpsConnectionType::Automatic, QStringLiteral( "GPS connection type" ) ) SIP_SKIP;

    //! Settings entry GPSD host name
    static const inline QgsSettingsEntryString settingsGpsdHostName = QgsSettingsEntryString( QStringLiteral( "gpsd-host-name" ), QgsSettings::Prefix::GPS, QString(), QStringLiteral( "GPSD connection host name" ) ) SIP_SKIP;

    //! Settings entry GPSD port number
    static const inline QgsSettingsEntryInteger settingsGpsdPortNumber = QgsSettingsEntryInteger( QStringLiteral( "gpsd-port" ), QgsSettings::Prefix::GPS, 2947, QStringLiteral( "GPSD port number" ) ) SIP_SKIP;

    //! Settings entry GPSD device name
    static const inline QgsSettingsEntryString settingsGpsdDeviceName = QgsSettingsEntryString( QStringLiteral( "gpsd-device-name" ), QgsSettings::Prefix::GPS, QString(), QStringLiteral( "GPSD connection device name" ) ) SIP_SKIP;

    //! Settings entry GPS serial device name
    static const inline QgsSettingsEntryString settingsGpsSerialDevice = QgsSettingsEntryString( QStringLiteral( "gpsd-serial-device" ), QgsSettings::Prefix::GPS, QString(), QStringLiteral( "GPS serial device name" ) ) SIP_SKIP;

    //! Settings entry GPS track point acquisition interval
    static const inline QgsSettingsEntryInteger settingGpsAcquisitionInterval = QgsSettingsEntryInteger( QStringLiteral( "acquisition-interval" ), QgsSettings::Prefix::GPS, 0, QStringLiteral( "GPS track point acquisition interval" ) ) SIP_SKIP;

    //! Settings entry GPS track point distance threshold
    static const inline QgsSettingsEntryDouble settingGpsDistanceThreshold = QgsSettingsEntryDouble( QStringLiteral( "distance-threshold" ), QgsSettings::Prefix::GPS, 0, QStringLiteral( "GPS track point distance threshold" ) ) SIP_SKIP;

    //! Settings entry GPS calculate bearing from travel direction
    static const inline QgsSettingsEntryBool settingGpsBearingFromTravelDirection = QgsSettingsEntryBool( QStringLiteral( "calculate-bearing-from-travel" ), QgsSettings::Prefix::GPS, false, QStringLiteral( "Calculate GPS bearing from travel direction" ) ) SIP_SKIP;

    //! Settings entry GPS apply leap seconds correction
    static const inline QgsSettingsEntryBool settingGpsApplyLeapSecondsCorrection = QgsSettingsEntryBool( QStringLiteral( "apply-leap-seconds-correction" ), QgsSettings::Prefix::GPS, true, QStringLiteral( "Whether leap seconds corrections should be applied to GPS timestamps" ) ) SIP_SKIP;

    //! Settings entry GPS leap seconds correction amount (in seconds)
    static const inline QgsSettingsEntryInteger settingGpsLeapSeconds = QgsSettingsEntryInteger( QStringLiteral( "leap-seconds" ), QgsSettings::Prefix::GPS, 18, QStringLiteral( "Leap seconds correction amount (in seconds)" ) ) SIP_SKIP;

    //! Settings entry time specification for GPS time stamps
    static const inline QgsSettingsEntryEnumFlag<Qt::TimeSpec> settingsGpsTimeStampSpecification = QgsSettingsEntryEnumFlag<Qt::TimeSpec>( QStringLiteral( "timestamp-time-spec" ), QgsSettings::Prefix::GPS, Qt::TimeSpec::LocalTime, QStringLiteral( "GPS time stamp specification" ) ) SIP_SKIP;

    //! Settings entry GPS time stamp time zone
    static const inline QgsSettingsEntryString settingsGpsTimeStampTimeZone = QgsSettingsEntryString( QStringLiteral( "timestamp-time-zone" ), QgsSettings::Prefix::GPS, QString(), QStringLiteral( "GPS time stamp time zone" ) ) SIP_SKIP;

    //! Settings entry GPS time offset from UTC in seconds
    static const inline QgsSettingsEntryInteger settingsGpsTimeStampOffsetFromUtc = QgsSettingsEntryInteger( QStringLiteral( "timestamp-offset-from-utc" ), QgsSettings::Prefix::GPS, 0, QStringLiteral( "GPS time stamp offset from UTC (in seconds)" ) ) SIP_SKIP;

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

    /**
     * Returns the last valid location obtained by the device.
     *
     * \since QGIS 3.30
     */
    QgsPoint lastValidLocation() const { return mLastLocation; }

  signals:

    /**
     * Emitted whenever the GPS state is changed.
     */
    void stateChanged( const QgsGpsInformation &info );

    // TODO QGIS 4.0 -- move to QgsNmeaConnection, it makes no sense in the base class

    /**
     * Emitted whenever the GPS device receives a raw NMEA sentence.
     */
    void nmeaSentenceReceived( const QString &substring );

    /**
     * Emitted when the GPS device fix status is changed.
     *
     * \since QGIS 3.30
     */
    void fixStatusChanged( Qgis::GpsFixStatus status );

    /**
     * Emitted when the GPS position changes.
     *
     * This signal is only emitted when the new GPS location is considered valid (see QgsGpsInformation::isValid()).
     *
     * \since QGIS 3.30
     */
    void positionChanged( const QgsPoint &point );

  protected:
    //! Data source (e.g. serial device, socket, file,...)
    std::unique_ptr< QIODevice > mSource;
    //! Last state of the gps related variables (e.g. position, time, ...)
    QgsGpsInformation mLastGPSInformation;
    //! Connection status
    Status mStatus = NotConnected;

  private slots:

    void onStateChanged( const QgsGpsInformation &info );

  private:
    //! Closes and deletes mSource
    void cleanupSource();
    void clearLastGPSInformation();

  protected slots:
    //! Parse available data source content
    virtual void parseData() = 0;

  private:

    //! Last fix status
    Qgis::GpsFixStatus mLastFixStatus = Qgis::GpsFixStatus::NoData;

    //! Last recorded valid location
    QgsPoint mLastLocation;
};

Q_DECLARE_METATYPE( QgsGpsInformation )

#endif // QGSGPSCONNECTION_H
