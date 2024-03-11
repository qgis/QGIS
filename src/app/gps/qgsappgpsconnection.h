/***************************************************************************
    qgsappgpsconnection.h
    -------------------------
    begin                : October 2022
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
#ifndef QGSAPPGPSCONNECTION_H
#define QGSAPPGPSCONNECTION_H

#include "qgis_app.h"
#include "qgis.h"

#include <QObject>
#include <QPointer>

class QgsGpsConnection;
class QgsGpsInformation;
class QgsPoint;
class QgsMessageBarItem;
class QgsGpsDetector;

/**
 * Manages a single "canonical" GPS connection for use in the QGIS app, eg for displaying GPS
 * coordinates and recording GPS tracks.
 *
 * While the underlying core API for GPS connections is designed to support multiple connections,
 * the QGIS app interface (and all related GPS widgets) only allow for a single user-defined GPS connection
 * at a time. This class tracks this canonical connection, and advises clients when the user-set
 * GPS devices has state changes.
 */
class APP_EXPORT QgsAppGpsConnection : public QObject
{
    Q_OBJECT

  public:

    QgsAppGpsConnection( QObject *parent );

    ~QgsAppGpsConnection() override;

    /**
     * Returns the associated GPS connection, or NULLPTR if not connected.
     */
    QgsGpsConnection *connection();

    /**
     * Returns TRUE if the GPS device is currently connected.
     */
    bool isConnected() const;

    /**
     * Sets a GPS \a connection to use within QGIS app.
     *
     * Any existing GPS connection used by the app will be disconnected and replaced with this connection. The connection
     * is automatically registered within the QgsApplication::gpsConnectionRegistry().
     */
    void setConnection( QgsGpsConnection *connection );

    /**
     * Returns the last recorded GPS position.
     */
    QgsPoint lastValidLocation() const;

    /**
     * Returns the last received GPS information.
     */
    QgsGpsInformation lastInformation() const;

  public slots:

    /**
     * Starts a connection to the user-specified GPS device.
     */
    void connectGps();

    /**
     * Disconnects from the user-specified GPS device.
     */
    void disconnectGps();

  signals:

    /**
     * Emitted when a connection attempt starts.
     */
    void connecting();

    /**
     * Emitted when an \a error occurs during the connection.
     */
    void connectionError( const QString &error );

    /**
     * Emitted when the GPS connection has been successfully made.
     */
    void connected();

    /**
     * Emitted when the connection status changes.
     */
    void statusChanged( Qgis::DeviceConnectionStatus status );

    /**
     * Emitted when the GPS device has been disconnected.
     */
    void disconnected();

    /**
     * Emitted when a connection timeout occurs.
     */
    void connectionTimedOut();

    /**
     * Emitted when the GPS fix status is changed
     */
    void fixStatusChanged( Qgis::GpsFixStatus status );

    /**
     * Emitted when the state of the associated GPS device changes.
     */
    void stateChanged( const QgsGpsInformation &info );

    /**
     * Emitted when the associated GPS device receives an NMEA sentence.
     */
    void nmeaSentenceReceived( const QString &substring );

    /**
     * Emitted when the GPS position changes.
     */
    void positionChanged( const QgsPoint &point );

  private slots:

    void onTimeOut();
    void onConnectionDetected();

    void setConnectionPrivate( QgsGpsConnection *connection );

  private:

    void showStatusBarMessage( const QString &msg );

    void showGpsConnectFailureWarning( const QString &message );
    void showMessage( Qgis::MessageLevel level, const QString &message );

    QPointer< QgsGpsDetector > mDetector;
    QgsGpsConnection *mConnection = nullptr;
    QPointer< QgsMessageBarItem > mConnectionMessageItem;
};


#endif // QGSAPPGPSCONNECTION_H
