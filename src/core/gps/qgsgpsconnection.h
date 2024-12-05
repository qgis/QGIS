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
#include "qgspoint.h"
#include "qgsgpsinformation.h"

class QIODevice;

class QgsSettingsEntryBool;
class QgsSettingsEntryDouble;
class QgsSettingsEntryInteger;
class QgsSettingsEntryString;
template<class T> class QgsSettingsEntryEnumFlag;


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
    //SIP_TYPEHEADER_INCLUDE( "qgsgpsdconnection.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsnmeaconnection.h" );


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

#ifndef SIP_RUN
    //! Settings entry GPS connection type
    static const QgsSettingsEntryEnumFlag<Qgis::GpsConnectionType> *settingsGpsConnectionType;

    //! Settings entry GPSD host name
    static const QgsSettingsEntryString *settingsGpsdHostName;

    //! Settings entry GPSD port number
    static const QgsSettingsEntryInteger *settingsGpsdPortNumber;

    //! Settings entry GPSD device name
    static const QgsSettingsEntryString *settingsGpsdDeviceName;

    //! Settings entry GPS serial device name
    static const QgsSettingsEntryString *settingsGpsSerialDevice;

    //! Settings entry GPS track point acquisition interval
    static const QgsSettingsEntryInteger *settingGpsAcquisitionInterval;

    //! Settings entry GPS track point distance threshold
    static const QgsSettingsEntryDouble *settingGpsDistanceThreshold;

    //! Settings entry GPS calculate bearing from travel direction
    static const QgsSettingsEntryBool *settingGpsBearingFromTravelDirection;

    //! Settings entry GPS apply leap seconds correction
    static const QgsSettingsEntryBool *settingGpsApplyLeapSecondsCorrection;

    //! Settings entry GPS leap seconds correction amount (in seconds)
    static const QgsSettingsEntryInteger *settingGpsLeapSeconds;

    //! Settings entry time specification for GPS time stamps
    static const QgsSettingsEntryEnumFlag<Qt::TimeSpec> *settingsGpsTimeStampSpecification;

    //! Settings entry GPS time stamp time zone
    static const QgsSettingsEntryString *settingsGpsTimeStampTimeZone;

    //! Settings entry GPS time offset from UTC in seconds
    static const QgsSettingsEntryInteger *settingsGpsTimeStampOffsetFromUtc;
#endif

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
    virtual void parseData() = 0;  // cppcheck-suppress pureVirtualCall

  private:

    //! Last fix status
    Qgis::GpsFixStatus mLastFixStatus = Qgis::GpsFixStatus::NoData;

    //! Last recorded valid location
    QgsPoint mLastLocation;
};

#endif // QGSGPSCONNECTION_H
