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

QgsGpsdConnection::QgsGpsdConnection( QString host, qint16 port, QString device )
    : QgsNMEAConnection( new QTcpSocket() )
    , mDevice( device )
{
  QTcpSocket *socket = qobject_cast< QTcpSocket * >( mSource );

  QObject::connect( socket, SIGNAL( connected() ), this, SLOT( connected() ) );
  QObject::connect( socket, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( error( QAbstractSocket::SocketError ) ) );
  socket->connectToHost( host, port );
}

QgsGpsdConnection::~QgsGpsdConnection()
{
  //connection will be closed by base class
  QgsDebugMsg( "entered." );
}

void QgsGpsdConnection::connected()
{
  QgsDebugMsg( "connected!" );
  QTcpSocket *socket = qobject_cast< QTcpSocket * >( mSource );
  socket->write( QString( "?WATCH={\"enable\":true,\"raw\":true%1};" ).arg( mDevice.isEmpty() ? mDevice : QString( ",\"device\":%1" ).arg( mDevice ) ).toUtf8() );
}

void QgsGpsdConnection::error( QAbstractSocket::SocketError socketError )
{
#if QGISDEBUG
  QTcpSocket *socket = qobject_cast< QTcpSocket * >( mSource );
  QgsDebugMsg( QString( "error: %1 %2" ).arg( socketError ).arg( socket->errorString() ) );
#endif
}
