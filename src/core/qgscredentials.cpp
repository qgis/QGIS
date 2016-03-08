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

#include <QTextStream>

QgsCredentials *QgsCredentials::smInstance = nullptr;

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

  return new QgsCredentialsNone();
}

QgsCredentials::QgsCredentials()
{
}

QgsCredentials::~QgsCredentials()
{
}

bool QgsCredentials::get( const QString& realm, QString &username, QString &password, const QString& message )
{
  if ( mCredentialCache.contains( realm ) )
  {
    QPair<QString, QString> credentials = mCredentialCache.take( realm );
    username = credentials.first;
    password = credentials.second;
    QgsDebugMsg( QString( "retrieved realm:%1 username:%2 password:%3" ).arg( realm, username, password ) );

    if ( !password.isNull() )
      return true;
  }

  if ( request( realm, username, password, message ) )
  {
    QgsDebugMsg( QString( "requested realm:%1 username:%2 password:%3" ).arg( realm, username, password ) );
    return true;
  }
  else
  {
    QgsDebugMsg( QString( "unset realm:%1" ).arg( realm ) );
    return false;
  }
}

void QgsCredentials::put( const QString& realm, const QString& username, const QString& password )
{
  QgsDebugMsg( QString( "inserting realm:%1 username:%2 password:%3" ).arg( realm, username, password ) );
  mCredentialCache.insert( realm, QPair<QString, QString>( username, password ) );
}

bool QgsCredentials::getMasterPassword( QString &password , bool stored )
{
  if ( requestMasterPassword( password, stored ) )
  {
    QgsDebugMsg( "requested master password" );
    return true;
  }
  return false;
}

void QgsCredentials::lock()
{
  mMutex.lock();
}

void QgsCredentials::unlock()
{
  mMutex.unlock();
}


////////////////////////////////
// QgsCredentialsNone

QgsCredentialsNone::QgsCredentialsNone()
{
  setInstance( this );
}

bool QgsCredentialsNone::request( const QString& realm, QString &username, QString &password, const QString& message )
{
  Q_UNUSED( realm );
  Q_UNUSED( username );
  Q_UNUSED( password );
  Q_UNUSED( message );
  return false;
}

bool QgsCredentialsNone::requestMasterPassword( QString &password, bool stored )
{
  Q_UNUSED( password );
  Q_UNUSED( stored );
  return false;
}

////////////////////////////////
// QgsCredentialsConsole

QgsCredentialsConsole::QgsCredentialsConsole()
{
  setInstance( this );
}

bool QgsCredentialsConsole::request( const QString& realm, QString &username, QString &password, const QString& message )
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

bool QgsCredentialsConsole::requestMasterPassword( QString &password, bool stored )
{
  Q_UNUSED( stored );

  QTextStream in( stdin, QIODevice::ReadOnly );
  QTextStream out( stdout, QIODevice::WriteOnly );

  QString msg( stored ? "Master password for authentication configs: " : "Set master password for authentication configs: " );

  out << msg;
  in >> password;

  return true;
}
