/***************************************************************************
    qgscredentials.cpp -  interface for requesting credentials
    ----------------------
    begin                : February 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscredentials.h"
#include "qgslogger.h"

#include <QTextIStream>
#include <QTextOStream>

QgsCredentials *QgsCredentials::smInstance = 0;

void QgsCredentials::setInstance( QgsCredentials *theInstance )
{
  if ( smInstance )
  {
    QgsDebugMsg( "already registered an instance of QgsCredentials" );
  }

  smInstance = theInstance;
}

QgsCredentials *QgsCredentials::instance()
{
  if ( smInstance )
    return smInstance;

  return new QgsCredentialsConsole();
}

QgsCredentials::~QgsCredentials()
{
}

bool QgsCredentials::get( QString realm, QString &username, QString &password, QString message )
{
  if ( mCredentialCache.contains( realm ) )
  {
    QPair<QString, QString> credentials = mCredentialCache.take( realm );
    username = credentials.first;
    password = credentials.second;
    QgsDebugMsg( QString( "retrieved realm:%1 username:%2 password:%3" ).arg( realm ).arg( username ).arg( password ) );
    return true;
  }
  else if ( request( realm, username, password, message ) )
  {
    QgsDebugMsg( QString( "requested realm:%1 username:%2 password:%3" ).arg( realm ).arg( username ).arg( password ) );
    return true;
  }
  else
  {
    QgsDebugMsg( QString( "unset realm:%1" ).arg( realm ) );
    return false;
  }
}

void QgsCredentials::put( QString realm, QString username, QString password )
{
  QgsDebugMsg( QString( "inserting realm:%1 username:%2 password:%3" ).arg( realm ).arg( username ).arg( password ) );
  mCredentialCache.insert( realm, QPair<QString, QString>( username, password ) );
}

////////////////////////////////
// QgsCredentialsConsole

QgsCredentialsConsole::QgsCredentialsConsole()
{
  setInstance( this );
}

bool QgsCredentialsConsole::request( QString realm, QString &username, QString &password, QString message )
{
  QTextStream in( stdin, QIODevice::ReadOnly );
  QTextStream out( stdout, QIODevice::WriteOnly );

  out << "credentials for " << realm << endl;
  if ( !message.isEmpty() )
    out << "message: " << message << endl;
  out << "username: ";
  in >> username;
  out << "password: ";
  in >> password;

  return true;
}
