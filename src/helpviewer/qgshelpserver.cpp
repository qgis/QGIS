/***************************************************************************
                           qgshelpserver.cpp
    Receive help context numbers from client process for help viewer
                             -------------------
    begin                : 2005-07-07
    copyright            : (C) 2005 by Tom Elwertowski
    email                : telwertowski at comcast.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgshelpserver.h"

// The communications technique used here has been adapted from Qt Assistant.
// See qt/tools/assistant/ main.cpp and lib/qassistantclient.cpp (Qt 3.3.4).

QgsHelpContextServer::QgsHelpContextServer( QObject *parent ) :
    QTcpServer( parent )
{
  listen( QHostAddress::LocalHost, 0 );
}

QgsHelpContextServer::~QgsHelpContextServer()
{
  // Socket is automatically deleted here because it is a QQbject child
}

void QgsHelpContextServer::incomingConnection( int socket )
{
  // Create socket in response to new connection
  QgsHelpContextSocket *helpSocket = new QgsHelpContextSocket( socket, this );
  // Pass context from socket upwards
  connect( helpSocket, SIGNAL( setContext( const QString& ) ),
           SIGNAL( setContext( const QString& ) ) );
  emit newConnection();
}

QgsHelpContextSocket::QgsHelpContextSocket( int socket, QObject *parent ) :
    QTcpSocket( parent )
{
  connect( this, SIGNAL( readyRead() ), SLOT( readClient() ) );
  setSocketDescriptor( socket );
}

QgsHelpContextSocket::~QgsHelpContextSocket()
{
}

void QgsHelpContextSocket::readClient()
{
  // Read context numbers (one per line) and pass upwards
  QString contextId;
  while ( canReadLine() )
  {
    contextId = readLine();
    contextId.remove( '\n' );
    emit setContext( contextId );
  }
}
