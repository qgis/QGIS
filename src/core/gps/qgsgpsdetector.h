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

/**
 * \ingroup core
 * \brief Class to detect the GPS port
 */
class CORE_EXPORT QgsGpsDetector : public QObject
{
    Q_OBJECT
  public:
    QgsGpsDetector( const QString &portName );

#if defined( HAVE_QTSERIALPORT )
    static const QgsSettingsEntryEnumFlag<QSerialPort::StopBits> *settingsGpsStopBits SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<QSerialPort::DataBits> *settingsGpsDataBits SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<QSerialPort::Parity> *settingsGpsParity SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<QSerialPort::FlowControl> *settingsGpsFlowControl SIP_SKIP;
#endif

    ~QgsGpsDetector() override;

    static QList< QPair<QString, QString> > availablePorts();

  public slots:
    void advance();
    void detected( const QgsGpsInformation & );
    void connDestroyed( QObject * );

  signals:

    // TODO QGIS 4.0 - this is horrible, fragile, leaky and crash prone API.
    // don't transfer ownership with this signal, and add an explicit takeConnection member!

    /**
     * Emitted when the GPS connection has been detected. A single connection must listen for this signal and
     * immediately take ownership of the \a connection object.
     */
    void detected( QgsGpsConnection *connection );

    void detectionFailed();

  private:
    int mPortIndex = 0;
    int mBaudIndex = -1;
    QList< QPair< QString, QString > > mPortList;
    QList<qint32> mBaudList;

    std::unique_ptr< QgsGpsConnection > mConn;
};

#endif // QGSGPSDETECTOR_H
