/***************************************************************************
                             qgsiodevicesensor.cpp
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

#include "qgsiodevicesensor.h"
#include "qgssensorregistry.h"

#include <QDomElement>

#if defined( Q_OS_ANDROID ) || defined( Q_OS_LINUX )
#include <sys/socket.h>
#endif

QgsIODeviceSensor::~QgsIODeviceSensor()
{
}

void QgsIODeviceSensor::initIODevice( QIODevice *device )
{
  mIODevice.reset( device );

  if ( mIODevice )
  {
    connect( mIODevice.get(), &QIODevice::readyRead, this, &QgsIODeviceSensor::parseData );
  }
}

QIODevice *QgsIODeviceSensor::iODevice() const
{
  return mIODevice.get();
}

void QgsIODeviceSensor::parseData()
{
  QgsAbstractSensor::SensorData data;
  data.lastValue = mIODevice->readAll();
  data.lastTimestamp = QDateTime::currentDateTime();
  setData( data );
}

//--------------

QgsTcpSocketSensor::QgsTcpSocketSensor( QObject *parent )
  : QgsIODeviceSensor( parent )
  , mTcpSocket( new QTcpSocket() )
{
  connect( mTcpSocket, &QAbstractSocket::stateChanged, this, &QgsTcpSocketSensor::socketStateChanged );
  connect( mTcpSocket, qOverload<QAbstractSocket::SocketError>( &QAbstractSocket::errorOccurred ), this, &QgsTcpSocketSensor::handleError );

  initIODevice( mTcpSocket );
}

QgsTcpSocketSensor *QgsTcpSocketSensor::create( QObject *parent )
{
  return new QgsTcpSocketSensor( parent );
}

QString QgsTcpSocketSensor::type() const
{
  return QLatin1String( "tcp_socket" );
}

QString QgsTcpSocketSensor::hostName() const
{
  return mHostName;
}

void QgsTcpSocketSensor::setHostName( const QString &hostName )
{
  if ( mHostName == hostName )
    return;

  mHostName = hostName;
}

int QgsTcpSocketSensor::port() const
{
  return mPort;
}

void QgsTcpSocketSensor::setPort( int port )
{
  if ( mPort == port || port < 1 )
    return;

  mPort = port;
}

void QgsTcpSocketSensor::handleConnect()
{
  if ( mHostName.isEmpty() || mPort == 0 )
  {
    setStatus( Qgis::DeviceConnectionStatus::Disconnected );
    return;
  }

  mTcpSocket->connectToHost( mHostName, mPort, QTcpSocket::ReadOnly );
}

void QgsTcpSocketSensor::handleDisconnect()
{
  mTcpSocket->close();
}

void QgsTcpSocketSensor::handleError( QAbstractSocket::SocketError error )
{
  switch ( error )
  {
    case QAbstractSocket::HostNotFoundError:
      mErrorString = tr( "Could not find the remote host" );
      break;
    case QAbstractSocket::NetworkError:
      mErrorString = tr( "Attempt to read or write from socket returned an error" );
      break;
    case QAbstractSocket::ConnectionRefusedError:
      mErrorString = tr( "The connection was refused by the remote host" );
      break;
    default:
      mErrorString = tr( "%1" ).arg( QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey( error ) );
      break;
  }

  emit errorOccurred( mErrorString );
}

void QgsTcpSocketSensor::socketStateChanged( const QAbstractSocket::SocketState socketState )
{
  switch ( socketState )
  {
    case QAbstractSocket::ConnectedState:
    {
      setStatus( Qgis::DeviceConnectionStatus::Connected );
      break;
    }
    case QAbstractSocket::UnconnectedState:
    {
      setStatus( Qgis::DeviceConnectionStatus::Disconnected );
      break;
    }
    default:
      break;
  }
}

bool QgsTcpSocketSensor::writePropertiesToElement( QDomElement &element, QDomDocument & ) const
{
  element.setAttribute( QStringLiteral( "hostName" ), mHostName );
  element.setAttribute( QStringLiteral( "port" ), QString::number( mPort ) );

  return true;
}

bool QgsTcpSocketSensor::readPropertiesFromElement( const QDomElement &element, const QDomDocument & )
{
  mHostName = element.attribute( QStringLiteral( "hostName" ) );
  mPort = element.attribute( QStringLiteral( "port" ) ).toInt();

  return true;
}

//--------------

QgsUdpSocketSensor::QgsUdpSocketSensor( QObject *parent )
  : QgsIODeviceSensor( parent )
  , mUdpSocket( std::make_unique<QUdpSocket>() )
  , mBuffer( new QBuffer() )
{
#if defined( Q_OS_ANDROID ) || defined( Q_OS_LINUX )
  int sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
  int optval = 1;
  setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR,
              ( void * ) &optval, sizeof( optval ) );
  mUdpSocket->setSocketDescriptor( sockfd, QUdpSocket::UnconnectedState );
#endif

  connect( mUdpSocket.get(), &QAbstractSocket::stateChanged, this, &QgsUdpSocketSensor::socketStateChanged );
  connect( mUdpSocket.get(), &QUdpSocket::readyRead, this, [this]()
  {
    QByteArray datagram;
    while ( mUdpSocket->hasPendingDatagrams() )
    {
      datagram.resize( int( mUdpSocket->pendingDatagramSize() ) );
      mUdpSocket->readDatagram( datagram.data(), datagram.size() );

      mBuffer->buffer().clear();
      mBuffer->seek( 0 );
      mBuffer->write( datagram );
      mBuffer->seek( 0 );
    }
  } );

  connect( mUdpSocket.get(), qOverload<QAbstractSocket::SocketError>( &QAbstractSocket::errorOccurred ), this, &QgsUdpSocketSensor::handleError );

  initIODevice( mBuffer );
}

QgsUdpSocketSensor *QgsUdpSocketSensor::create( QObject *parent )
{
  return new QgsUdpSocketSensor( parent );
}

QString QgsUdpSocketSensor::type() const
{
  return QLatin1String( "udp_socket" );
}

QString QgsUdpSocketSensor::hostName() const
{
  return mHostName;
}

void QgsUdpSocketSensor::setHostName( const QString &hostName )
{
  if ( mHostName == hostName )
    return;

  mHostName = hostName;
}

int QgsUdpSocketSensor::port() const
{
  return mPort;
}

void QgsUdpSocketSensor::setPort( int port )
{
  if ( mPort == port || port < 1 )
    return;

  mPort = port;
}

void QgsUdpSocketSensor::handleConnect()
{
  if ( mHostName.isEmpty() || mPort == 0 )
  {
    setStatus( Qgis::DeviceConnectionStatus::Disconnected );
    return;
  }

  mBuffer->open( QIODevice::ReadWrite );
  mUdpSocket->bind( QHostAddress( mHostName ), mPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint );
  mUdpSocket->joinMulticastGroup( QHostAddress( mHostName ) );
}

void QgsUdpSocketSensor::handleDisconnect()
{
  mUdpSocket->close();
  mBuffer->close();
}

void QgsUdpSocketSensor::handleError( QAbstractSocket::SocketError error )
{
  switch ( error )
  {
    case QAbstractSocket::HostNotFoundError:
      mErrorString = tr( "Could not find the remote host" );
      break;
    case QAbstractSocket::NetworkError:
      mErrorString = tr( "Attempt to read or write from socket returned an error" );
      break;
    case QAbstractSocket::ConnectionRefusedError:
      mErrorString = tr( "The connection was refused by the remote host" );
      break;
    default:
      mErrorString = tr( "%1" ).arg( QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey( error ) );
      break;
  }

  emit errorOccurred( mErrorString );
}

void QgsUdpSocketSensor::socketStateChanged( const QAbstractSocket::SocketState socketState )
{
  switch ( socketState )
  {
    case QAbstractSocket::ConnectedState:
    case QAbstractSocket::BoundState:
    {
      setStatus( Qgis::DeviceConnectionStatus::Connected );
      break;
    }
    case QAbstractSocket::UnconnectedState:
    {
      setStatus( Qgis::DeviceConnectionStatus::Disconnected );
      break;
    }
    default:
      break;
  }
}

bool QgsUdpSocketSensor::writePropertiesToElement( QDomElement &element, QDomDocument & ) const
{
  element.setAttribute( QStringLiteral( "hostName" ), mHostName );
  element.setAttribute( QStringLiteral( "port" ), QString::number( mPort ) );

  return true;
}

bool QgsUdpSocketSensor::readPropertiesFromElement( const QDomElement &element, const QDomDocument & )
{
  mHostName = element.attribute( QStringLiteral( "hostName" ) );
  mPort = element.attribute( QStringLiteral( "port" ) ).toInt();

  return true;
}

//--------------

#if defined( HAVE_QTSERIALPORT )
QgsSerialPortSensor::QgsSerialPortSensor( QObject *parent )
  : QgsIODeviceSensor( parent )
  , mSerialPort( new QSerialPort() )
{
  connect( mSerialPort, qOverload<QSerialPort::SerialPortError>( &QSerialPort::errorOccurred ), this, &QgsSerialPortSensor::handleError );

  initIODevice( mSerialPort );
}

QgsSerialPortSensor *QgsSerialPortSensor::create( QObject *parent )
{
  return new QgsSerialPortSensor( parent );
}

QString QgsSerialPortSensor::type() const
{
  return QLatin1String( "serial_port" );
}

QString QgsSerialPortSensor::portName() const
{
  return mPortName;
}

void QgsSerialPortSensor::setPortName( const QString &portName )
{
  if ( mPortName == portName )
    return;

  mPortName = portName;
}

QSerialPort::BaudRate QgsSerialPortSensor::baudRate() const
{
  return mBaudRate;
}

void QgsSerialPortSensor::setBaudRate( const QSerialPort::BaudRate &baudRate )
{
  if ( mBaudRate == baudRate )
    return;

  mBaudRate = baudRate;
}

QByteArray QgsSerialPortSensor::delimiter() const
{
  return mDelimiter;
}

void QgsSerialPortSensor::setDelimiter( const QByteArray &delimiter )
{
  if ( mDelimiter == delimiter )
    return;

  mDelimiter = delimiter;
}


void QgsSerialPortSensor::parseData()
{
  if ( !mDelimiter.isEmpty() )
  {
    if ( mFirstDelimiterHit )
    {
      mDataBuffer += mSerialPort->readAll();
      const auto lastIndex = mDataBuffer.lastIndexOf( mDelimiter );
      if ( lastIndex > -1 )
      {
        QgsAbstractSensor::SensorData data;
        data.lastValue = mDataBuffer.mid( 0, lastIndex );
        mDataBuffer = mDataBuffer.mid( lastIndex + mDelimiter.size() );
        data.lastTimestamp = QDateTime::currentDateTime();
        setData( data );
      }
    }
    else
    {
      QByteArray data = mSerialPort->readAll();
      const auto lastIndex = data.lastIndexOf( mDelimiter );
      if ( lastIndex > -1 )
      {
        mFirstDelimiterHit = true;
        mDataBuffer = data.mid( lastIndex + mDelimiter.size() );
      }
    }
  }
  else
  {
    QgsAbstractSensor::SensorData data;
    data.lastValue = mSerialPort->readAll();
    data.lastTimestamp = QDateTime::currentDateTime();
    setData( data );
  }
}

void QgsSerialPortSensor::handleConnect()
{
  mSerialPort->setPortName( mPortName );
  mSerialPort->setBaudRate( mBaudRate );
  mFirstDelimiterHit = false;

  if ( mSerialPort->open( QIODevice::ReadOnly ) )
  {
    setStatus( Qgis::DeviceConnectionStatus::Connected );
  }
  else
  {
    setStatus( Qgis::DeviceConnectionStatus::Disconnected );
  }
}

void QgsSerialPortSensor::handleDisconnect()
{
  mSerialPort->close();
}

void QgsSerialPortSensor::handleError( QSerialPort::SerialPortError error )
{
  if ( error == QSerialPort::NoError )
  {
    return;
  }

  switch ( error )
  {
    case QSerialPort::DeviceNotFoundError:
      mErrorString = tr( "Could not find the serial port device" );
      break;
    case QSerialPort::ReadError:
      mErrorString = tr( "Attempt to read from the serial port returned an error" );
      break;
    case QSerialPort::PermissionError:
      mErrorString = tr( "The connection was refused due to not having enough permission" );
      break;
    default:
      mErrorString = tr( "%1" ).arg( QMetaEnum::fromType<QSerialPort::SerialPortError>().valueToKey( error ) );
      break;
  }

  emit errorOccurred( mErrorString );
}

bool QgsSerialPortSensor::writePropertiesToElement( QDomElement &element, QDomDocument & ) const
{
  element.setAttribute( QStringLiteral( "portName" ), mPortName );
  element.setAttribute( QStringLiteral( "baudRate" ), static_cast<int>( mBaudRate ) );
  element.setAttribute( QStringLiteral( "delimiter" ), QString( mDelimiter ) );
  return true;
}

bool QgsSerialPortSensor::readPropertiesFromElement( const QDomElement &element, const QDomDocument & )
{
  mPortName = element.attribute( QStringLiteral( "portName" ) );
  mBaudRate = static_cast< QSerialPort::BaudRate >( element.attribute( QStringLiteral( "baudRate" ) ).toInt() );
  mDelimiter = element.attribute( QStringLiteral( "delimiter" ) ).toLocal8Bit();
  return true;
}
#endif
