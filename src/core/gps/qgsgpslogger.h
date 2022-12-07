/***************************************************************************
  qgsgpslogger.h
   -------------------
  begin                : November 2022
  copyright            : (C) 2022 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSLOGGER_H
#define QGSGPSLOGGER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdistancearea.h"
#include "qgscoordinatetransformcontext.h"
#include "qgswkbtypes.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"

#include <QObject>
#include <QPointer>
#include <QDateTime>
#include "info.h"

class QgsGpsConnection;
class QTimer;
class QgsGpsInformation;


/**
 * \ingroup core
 * \class QgsGpsLogger
 * \brief Base class for objects which log incoming GPS data.
 *
 * This class handles generic logic regarding logging GPS data, such as creation of tracks
 * from incoming GPS location points.
 *
 * \since QGIS 3.30
*/
class CORE_EXPORT QgsGpsLogger : public QObject
{
    Q_OBJECT

  public:

    //! Settings entry for whether storing GPS attributes as geometry M values should be enabled
    static const inline QgsSettingsEntryBool settingsGpsStoreAttributeInMValues = QgsSettingsEntryBool( QStringLiteral( "store-attribute-in-m-values" ), QgsSettings::Prefix::GPS, false, QStringLiteral( "Whether GPS attributes should be stored in geometry m values" ) ) SIP_SKIP;

    //! Settings entry dictating which GPS attribute should be stored in geometry M values
    static const inline QgsSettingsEntryEnumFlag<Qgis::GpsInformationComponent> settingsGpsMValueComponent = QgsSettingsEntryEnumFlag<Qgis::GpsInformationComponent>( QStringLiteral( "m-value-attribute" ), QgsSettings::Prefix::GPS, Qgis::GpsInformationComponent::Timestamp, QStringLiteral( "Which GPS attribute should be stored in geometry m values" ) ) SIP_SKIP;

    /**
     * Constructor for QgsGpsLogger with the specified \a parent object.
     *
     * The logger will automatically record GPS information from the specified \a connection.
     */
    QgsGpsLogger( QgsGpsConnection *connection, QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsGpsLogger() override;

    /**
     * Returns the associated GPS connection.
     *
     * \see setConnection()
     */
    QgsGpsConnection *connection();

    /**
     * Sets the associated GPS connection.
     *
     * \see connection()
     */
    void setConnection( QgsGpsConnection *connection );

    /**
     * Sets the \a ellipsoid which will be used for calculating distances in the log.
     */
    void setEllipsoid( const QString &ellipsoid );

    /**
     * Sets the coordinate transform \a context to be used when transforming
     * GPS coordinates.
     *
     * \see transformContext()
     */
    virtual void setTransformContext( const QgsCoordinateTransformContext &context );

    /**
     * Returns the coordinate transform context to be used when transforming
     * GPS coordinates.
     *
     * \see setTransformContext()
     */
    QgsCoordinateTransformContext transformContext() const;

    /**
     * Returns the distance area calculator which should be used for
     * calculating distances associated with the GPS log.
     */
    const QgsDistanceArea &distanceArea() const;

    /**
     * Returns the recorded points in the current track.
     *
     * These points will always be in WGS84 coordinate reference system.
     */
    QVector<QgsPoint> currentTrack() const;

    /**
     * Returns the current logged GPS positions as a geometry of the specified \a type.
     *
     * The returned geometries will always be in the WGS84 (EPSG:4326) coordinate reference system.
     *
     * \param type desired geometry type
     * \param error Will be set to a user-friendly error if the logged positions could not be converted to an appropriate geometry
     *
     * \returns logged GPS positions as a geometry.
     */
    QgsGeometry currentGeometry( QgsWkbTypes::Type type, QString &error SIP_OUT ) const;

    /**
     * Returns the last recorded position of the device.
     *
     * The returned point will always be in WGS84 coordinate reference system.
     */
    QgsPointXY lastPosition() const;

    /**
     * Returns the last recorded timestamp from the device.
     *
     * The returned time value will respect all user settings regarding GPS time zone
     * handling.
     */
    QDateTime lastTimestamp() const;

    /**
     * Returns the timestamp at which the current track was started.
     *
     * The returned time value will respect all user settings regarding GPS time zone
     * handling.
     */
    QDateTime trackStartTime() const;

    /**
     * Returns the last recorded elevation the device.
     */
    double lastElevation() const;

    /**
     * Returns the last recorded value corresponding to the QgsGpsLogger::settingsGpsMValueComponent setting.
     */
    double lastMValue() const;

    /**
     * Resets the current track, discarding all recorded points.
     */
    void resetTrack();

    /**
     * Returns TRUE if track vertices will be automatically added whenever
     * the GPS position is changed.
     *
     * \see setAutomaticallyAddTrackVertices()
     */
    bool automaticallyAddTrackVertices() const;

    /**
     * Sets whether track vertices will be automatically added whenever
     * the GPS position is changed.
     *
     * \see automaticallyAddTrackVertices()
     */
    void setAutomaticallyAddTrackVertices( bool enabled );

    /**
     * Should be called whenever the QGIS GPS settings are changed.
     */
    void updateGpsSettings();

    /**
     * Returns the total length of the current digitized track (in meters).
     *
     * The returned length is calculated using ellipsoidal calculations.
     */
    double totalTrackLength() const;

    /**
     * Returns the direct length from the first vertex in the track to the last (in meters).
     *
     * The returned length is calculated using ellipsoidal calculations.
     */
    double trackDistanceFromStart() const;

    /**
     * Returns the value of the corresponding GPS information \a component.
     */
    QVariant componentValue( Qgis::GpsInformationComponent component ) const;

  signals:

    /**
     * Emitted whenever the current track changes from being empty to non-empty or vice versa.
     */
    void trackIsEmptyChanged( bool isEmpty );

    /**
     * Emitted whenever the current track is reset.
     */
    void trackReset();

    /**
     * Emitted whenever a new vertex is added to the track.
     *
     * The \a vertex point will be in WGS84 coordinate reference system.
     */
    void trackVertexAdded( const QgsPoint &vertex );

    /**
     * Emitted whenever the associated GPS device state is changed.
     */
    void stateChanged( const QgsGpsInformation &info );

    /**
     * Emitted whenever the distance area used to calculate track distances is changed.
     */
    void distanceAreaChanged();

  protected:

    //! WGS84 coordinate reference system
    QgsCoordinateReferenceSystem mWgs84CRS;

    //! Used to pause logging of incoming GPS messages
    mutable int mBlockGpsStateChanged = 0;

    /**
     * Adds a track vertex at the current GPS location.
     */
    void addTrackVertex();

  private slots:

    void switchAcquisition();
    void gpsStateChanged( const QgsGpsInformation &info );

  private:

    QPointer< QgsGpsConnection > mConnection;

    QgsDistanceArea mDistanceCalculator;

    QgsCoordinateTransformContext mTransformContext;

    QgsPointXY mLastGpsPositionWgs84;
    double mLastElevation = 0.0;

    nmeaPOS mLastNmeaPosition;
    QDateTime mLastTime;

    QDateTime mPreviousTrackPointTime;
    QgsPointXY mPreviousTrackPoint;

    QDateTime mTrackStartTime;

    QVector<QgsPoint> mCaptureListWgs84;

    std::unique_ptr<QTimer> mAcquisitionTimer;
    bool mAcquisitionEnabled = true;
    int mAcquisitionInterval = 0;
    double mDistanceThreshold = 0;

    bool mApplyLeapSettings = false;
    int mLeapSeconds = 0;
    Qt::TimeSpec mTimeStampSpec = Qt::TimeSpec::LocalTime;
    QString mTimeZone;
    int mOffsetFromUtc = 0;

    bool mAutomaticallyAddTrackVertices = true;
    bool mStoreAttributeInMValues = false;
    Qgis::GpsInformationComponent mMValueComponent = Qgis::GpsInformationComponent::Timestamp;
    double mLastMValue = std::numeric_limits<double>::quiet_NaN();

    friend class TestQgsGpsIntegration;

};


#endif // QGSGPSLOGGER_H
