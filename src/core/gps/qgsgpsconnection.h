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
#include "qgsgpsinformation.h"

class QIODevice;

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsgpsconnection.h"
% End
#endif

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
    static const inline QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType> settingsGpsConnectionType = QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType>( QStringLiteral( "gps-connection-type" ), QgsSettings::sTreeGps, Qgis::GpsConnectionType::Automatic, QStringLiteral( "GPS connection type" ) ) SIP_SKIP;

    //! Settings entry GPSD host name
    static const inline QgsSettingsEntryString *settingsGpsdHostName = new QgsSettingsEntryString( QStringLiteral( "gpsd-host-name" ), QgsSettings::sTreeGps, QString(), QStringLiteral( "GPSD connection host name" ) ) SIP_SKIP;

    //! Settings entry GPSD port number
    static const inline QgsSettingsEntryInteger *settingsGpsdPortNumber = new QgsSettingsEntryInteger( QStringLiteral( "gpsd-port" ), QgsSettings::sTreeGps, 2947, QStringLiteral( "GPSD port number" ) ) SIP_SKIP;

    //! Settings entry GPSD device name
    static const inline QgsSettingsEntryString *settingsGpsdDeviceName = new QgsSettingsEntryString( QStringLiteral( "gpsd-device-name" ), QgsSettings::sTreeGps, QString(), QStringLiteral( "GPSD connection device name" ) ) SIP_SKIP;

    //! Settings entry GPS serial device name
    static const inline QgsSettingsEntryString *settingsGpsSerialDevice = new QgsSettingsEntryString( QStringLiteral( "gpsd-serial-device" ), QgsSettings::sTreeGps, QString(), QStringLiteral( "GPS serial device name" ) ) SIP_SKIP;

    //! Settings entry GPS track point acquisition interval
    static const inline QgsSettingsEntryInteger *settingGpsAcquisitionInterval = new QgsSettingsEntryInteger( QStringLiteral( "acquisition-interval" ), QgsSettings::sTreeGps, 0, QStringLiteral( "GPS track point acquisition interval" ) ) SIP_SKIP;

    //! Settings entry GPS track point distance threshold
    static const inline QgsSettingsEntryDouble *settingGpsDistanceThreshold = new QgsSettingsEntryDouble( QStringLiteral( "distance-threshold" ), QgsSettings::sTreeGps, 0, QStringLiteral( "GPS track point distance threshold" ) ) SIP_SKIP;

    //! Settings entry GPS calculate bearing from travel direction
    static const inline QgsSettingsEntryBool *settingGpsBearingFromTravelDirection = new QgsSettingsEntryBool( QStringLiteral( "calculate-bearing-from-travel" ), QgsSettings::sTreeGps, false, QStringLiteral( "Calculate GPS bearing from travel direction" ) ) SIP_SKIP;

    //! Settings entry GPS apply leap seconds correction
    static const inline QgsSettingsEntryBool *settingGpsApplyLeapSecondsCorrection = new QgsSettingsEntryBool( QStringLiteral( "apply-leap-seconds-correction" ), QgsSettings::sTreeGps, true, QStringLiteral( "Whether leap seconds corrections should be applied to GPS timestamps" ) ) SIP_SKIP;

    //! Settings entry GPS leap seconds correction amount (in seconds)
    static const inline QgsSettingsEntryInteger *settingGpsLeapSeconds = new QgsSettingsEntryInteger( QStringLiteral( "leap-seconds" ), QgsSettings::sTreeGps, 18, QStringLiteral( "Leap seconds correction amount (in seconds)" ) ) SIP_SKIP;

    //! Settings entry time specification for GPS time stamps
    static const inline QgsSettingsEntryEnumFlag<Qt::TimeSpec> settingsGpsTimeStampSpecification = QgsSettingsEntryEnumFlag<Qt::TimeSpec>( QStringLiteral( "timestamp-time-spec" ), QgsSettings::sTreeGps, Qt::TimeSpec::LocalTime, QStringLiteral( "GPS time stamp specification" ) ) SIP_SKIP;

    //! Settings entry GPS time stamp time zone
    static const inline QgsSettingsEntryString *settingsGpsTimeStampTimeZone = new QgsSettingsEntryString( QStringLiteral( "timestamp-time-zone" ), QgsSettings::sTreeGps, QString(), QStringLiteral( "GPS time stamp time zone" ) ) SIP_SKIP;

    //! Settings entry GPS time offset from UTC in seconds
    static const inline QgsSettingsEntryInteger *settingsGpsTimeStampOffsetFromUtc = new QgsSettingsEntryInteger( QStringLiteral( "timestamp-offset-from-utc" ), QgsSettings::sTreeGps, 0, QStringLiteral( "GPS time stamp offset from UTC (in seconds)" ) ) SIP_SKIP;

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

#endif // QGSGPSCONNECTION_H
