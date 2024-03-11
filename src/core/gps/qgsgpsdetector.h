/***************************************************************************
                          qgsgpsdetector.h  -  description
                          -------------------
    begin                : January 13th, 2009
    copyright            : (C) 2009 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSDETECTOR_H
#define QGSGPSDETECTOR_H

#include "qgsconfig.h"

#include <QObject>
#include <QList>
#include <QPair>
#if defined( HAVE_QTSERIALPORT )
#include <QSerialPort>
#endif
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"

#ifndef SIP_RUN
template<class T>
class QgsSettingsEntryEnumFlag;
#endif

class QgsGpsConnection;
class QgsGpsInformation;
class QTimer;

/**
 * \ingroup core
 * \brief Class to detect the GPS port
 */
class CORE_EXPORT QgsGpsDetector : public QObject
{
    Q_OBJECT
  public:

    // TODO QGIS 4.0 -- remove useUnsafeSignals option

    /**
     * Constructor for QgsGpsDetector.
     *
     * If \a portName is specified, then only devices from the given port will be scanned. Otherwise
     * all connection types will be attempted (including internal GPS devices).
     *
     * Since QGIS 3.38, the \a useUnsafeSignals parameter can be set to FALSE to avoid emitting the
     * dangerous and fragile detected() signal. This is highly recommended, but is opt-in to avoid
     * breaking stable QGIS 3.x API. If \a useUnsafeSignals is set to FALSE, only the safe connectionDetected() signal
     * will be emitted and clients must manually take ownership of the detected connection via a call
     * to takeConnection().
     */
    QgsGpsDetector( const QString &portName = QString(), bool useUnsafeSignals = true );

#if defined( HAVE_QTSERIALPORT )
    static const QgsSettingsEntryEnumFlag<QSerialPort::StopBits> *settingsGpsStopBits SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<QSerialPort::DataBits> *settingsGpsDataBits SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<QSerialPort::Parity> *settingsGpsParity SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<QSerialPort::FlowControl> *settingsGpsFlowControl SIP_SKIP;
#endif

    ~QgsGpsDetector() override;

    /**
     * Returns the detected GPS connection, and removes it from the detector.
     *
     * The caller takes ownership of the connection. Only the first call to this
     * method following a connectionDetected() signal will be able to retrieve the
     * detected connection -- subsequent calls will return NULLPTR.
     *
     * \warning Do not call this method if the useUnsafeSignals option in the
     * QgsGpsDetector constructor was set to TRUE.
     *
     * \since QGIS 3.38
     */
    QgsGpsConnection *takeConnection() SIP_TRANSFERBACK;

    static QList< QPair<QString, QString> > availablePorts();

  public slots:
    void advance();
    void detected( const QgsGpsInformation & );
    void connDestroyed( QObject * );

  signals:

    /**
     * Emitted when a GPS connection is successfully detected.
     *
     * Call takeConnection() to take ownership of the detected connection.
     *
     * \since QGIS 3.38
     */
    void connectionDetected();

    /**
     * Emitted when the GPS connection has been detected. A single connection must listen for this signal and
     * immediately take ownership of the \a connection object.
     *
     * \deprecated This signal is dangerous and extremely unsafe! It is recommended to instead set the \a useUnsafeSignals parameter to FALSE in the QgsGpsDetector constructor and use the safe connectionDetected() signal instead.
     */
    Q_DECL_DEPRECATED void detected( QgsGpsConnection *connection ) SIP_DEPRECATED;

    /**
     * Emitted when the detector could not find a valid GPS connection.
     */
    void detectionFailed();

  private slots:

    void connectionTimeout();

  private:
    bool mUseUnsafeSignals = true;
    int mPortIndex = 0;
    int mBaudIndex = -1;
    QList< QPair< QString, QString > > mPortList;
    QList<qint32> mBaudList;

    std::unique_ptr< QgsGpsConnection > mConn;
    QTimer *mTimeoutTimer = nullptr;
};

#endif // QGSGPSDETECTOR_H
