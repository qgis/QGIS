/***************************************************************************
                             qgsiodevicesensor.h
                             ---------------------------
    begin                : March 2023
    copyright            : (C) 2023 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSIODEVICESENSOR_H
#define QGSIODEVICESENSOR_H

#include "qgsconfig.h"

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractsensor.h"

#include <QBuffer>
#include <QDomElement>
#include <QTcpSocket>
#include <QUdpSocket>

#if defined( HAVE_QTSERIALPORT )
#include <QSerialPort>
#endif

/**
 * \ingroup core
 * \class QgsIODeviceSensor
 * \brief An abstract class QIODevice-based sensor classes
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsIODeviceSensor : public QgsAbstractSensor
{

    Q_OBJECT

  public:

    /**
     * Constructor for a abstract QIODevice-based sensor, bound to the specified \a parent.
     */
    explicit QgsIODeviceSensor( QObject *parent = nullptr ) : QgsAbstractSensor( parent ) {}
    ~QgsIODeviceSensor() override;

    /**
     * Returns the I/O device.
     */
    QIODevice *iODevice() const;

  protected:

    /**
     * Initiates the I/O \a device.
     * \note Takes ownership of the device.
     */
    void initIODevice( QIODevice *device SIP_TRANSFER );

  protected slots:

    /**
     * Parses the data read from the device when available.
     */
    virtual void parseData();

  private:

    std::unique_ptr<QIODevice> mIODevice;

};

/**
 * \ingroup core
 * \class QgsTcpSocketSensor
 * \brief A TCP socket sensor class
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsTcpSocketSensor : public QgsIODeviceSensor
{

    Q_OBJECT

  public:

    /**
     * Constructor for a TCP socket sensor, bound to the specified \a parent.
     */
    explicit QgsTcpSocketSensor( QObject *parent = nullptr );
    ~QgsTcpSocketSensor() override = default;

    /**
     * Returns a new TCP socket sensor.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsTcpSocketSensor *create( QObject *parent ) SIP_FACTORY;

    QString type() const override;

    /**
     * Returns the host name the socket connects to.
     */
    QString hostName() const;

    /**
     * Sets the host name the socket connects to.
     * \param hostName the host name string (a domain name or an IP address)
     */
    void setHostName( const QString &hostName );

    /**
     * Returns the port the socket connects to.
     */
    int port() const;

    /**
     * Sets the \a port the socket connects to.
     */
    void setPort( int port );

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document ) override;

  protected:

    void handleConnect() override;
    void handleDisconnect() override;

  private slots:

    void socketStateChanged( const QAbstractSocket::SocketState socketState );
    void handleError( QAbstractSocket::SocketError error );

  private:

    QTcpSocket *mTcpSocket = nullptr;

    QString mHostName;
    int mPort = 0;

};

/**
 * \ingroup core
 * \class QgsUdpSocketSensor
 * \brief A UDP socket sensor class
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsUdpSocketSensor : public QgsIODeviceSensor
{

    Q_OBJECT

  public:

    /**
     * Constructor for a UDP socket sensor, bound to the specified \a parent.
     */
    explicit QgsUdpSocketSensor( QObject *parent = nullptr );
    ~QgsUdpSocketSensor() override = default;

    /**
     * Returns a new UDP socket sensor.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsUdpSocketSensor *create( QObject *parent ) SIP_FACTORY;

    QString type() const override;

    /**
     * Returns the host name the socket connects to.
     */
    QString hostName() const;

    /**
     * Sets the host name the socket connects to.
     * \param hostName the host name string (a domain name or an IP address)
     */
    void setHostName( const QString &hostName );

    /**
     * Returns the port the socket connects to.
     */
    int port() const;

    /**
     * Sets the \a port the socket connects to.
     */
    void setPort( int port );

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document ) override;

  protected:

    void handleConnect() override;
    void handleDisconnect() override;

  private slots:

    void socketStateChanged( const QAbstractSocket::SocketState socketState );
    void handleError( QAbstractSocket::SocketError error );

  private:

    std::unique_ptr<QUdpSocket> mUdpSocket;
    QBuffer *mBuffer = nullptr;

    QString mHostName;
    int mPort = 0;

};

#if defined( HAVE_QTSERIALPORT )
SIP_IF_FEATURE( HAVE_QTSERIALPORT )

/**
 * \ingroup core
 * \class QgsSerialPortSensor
 * \brief A serial port sensor class
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsSerialPortSensor : public QgsIODeviceSensor
{

    Q_OBJECT

  public:

    /**
     * Constructor for a serial port sensor, bound to the specified \a parent.
     */
    explicit QgsSerialPortSensor( QObject *parent = nullptr );
    ~QgsSerialPortSensor() override = default;

    /**
     * Returns a new serial port sensor.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsSerialPortSensor *create( QObject *parent ) SIP_FACTORY;

    QString type() const override;

    /**
     * Returns the serial port the sensor connects to.
     */
    QString portName() const;

    /**
    * Sets the serial port the sensor connects to.
    * \param portName the port name (e.g. COM4)
    */
    void setPortName( const QString &portName );

    /**
     * Returns the baudrate of the serial port the sensor connects to.
     * \since QGIS 3.36
     */
    QSerialPort::BaudRate baudRate() const;

    /**
     * Sets the baudrate of the serial port the sensor connects to.
     * \param baudRate the baudrate (e.g. 9600)
     * \since QGIS 3.36
     */
    void setBaudRate( const QSerialPort::BaudRate &baudRate );

    /**
     * Returns the current delimiter used to separate data frames. If empty,
     * each serial port data update will be considered a data frame.
     * \since QGIS 3.38
     */
    QByteArray delimiter() const;

    /**
     * Sets the delimiter used to identify data frames out of the data received
     * from the serial port. If empty, each serial port data update will be
     * considered a data frame.
     * \param delimiter Character used to identify data frames
     * \since QGIS 3.38
     */
    void setDelimiter( const QByteArray &delimiter );

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document ) override;

  protected:

    void handleConnect() override;
    void handleDisconnect() override;

  protected slots:

    void parseData() override;

  private slots:

    void handleError( QSerialPort::SerialPortError error );

  private:

    QSerialPort *mSerialPort = nullptr;

    QString mPortName;
    QSerialPort::BaudRate mBaudRate = QSerialPort::Baud9600;
    QByteArray mDelimiter;
    bool mFirstDelimiterHit = false;
    QByteArray mDataBuffer;

};
SIP_END
#endif

#endif //QGSIODEVICESENSOR_H



