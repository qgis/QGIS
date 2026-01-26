/***************************************************************************
                          qgsgpsdconnection.cpp  -  description
                          ---------------------
    begin                : October 4th, 2010
    copyright            : (C) 2010 by Jürgen E. Fischer, norBIT GmbH
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

#include "qgsgpsdconnection.h"

#include "qgslogger.h"

#include <QTcpSocket>

#include "moc_qgsgpsdconnection.cpp"

QgsGpsdConnection::QgsGpsdConnection( const QString &host, qint16 port, const QString &device )
  : QgsNmeaConnection( new QTcpSocket() )
  , mDevice( device )
{
  QTcpSocket *socket = qobject_cast< QTcpSocket * >( mSource.get() );

  QObject::connect( socket, &QTcpSocket::connected, this, &QgsGpsdConnection::connected );
  QObject::connect( socket, &QTcpSocket::errorOccurred, this, &QgsGpsdConnection::errorOccurred );
  socket->connectToHost( host, port );
}

void QgsGpsdConnection::connected()
{
  QgsDebugMsgLevel( u"connected!"_s, 2 );
  QTcpSocket *socket = qobject_cast< QTcpSocket * >( mSource.get() );
  socket->write( u"?WATCH={\"enable\":true,\"nmea\":true,\"raw\":true%1};"_s.arg( mDevice.isEmpty() ? mDevice : u",\"device\":%1"_s.arg( mDevice ) ).toUtf8() );
}

void QgsGpsdConnection::errorOccurred( QAbstractSocket::SocketError socketError )
{
#ifdef QGISDEBUG
  QTcpSocket *socket = qobject_cast< QTcpSocket * >( mSource.get() );
  QgsDebugError( u"error: %1 %2"_s.arg( socketError ).arg( socket->errorString() ) );
#else
  Q_UNUSED( socketError )
#endif
}
