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
#include <QIODevice>

QgsCredentials *QgsCredentials::sInstance = nullptr;

void QgsCredentials::setInstance( QgsCredentials *instance )
{
  if ( sInstance )
  {
    QgsDebugMsg( QStringLiteral( "already registered an instance of QgsCredentials" ) );
  }

  sInstance = instance;
}

QgsCredentials *QgsCredentials::instance()
{
  if ( sInstance )
    return sInstance;

  return new QgsCredentialsNone();
}

bool QgsCredentials::get( const QString &realm, QString &username, QString &password, const QString &message )
{
  {
    const QMutexLocker locker( &mCacheMutex );
    if ( mCredentialCache.contains( realm ) )
    {
      const QPair<QString, QString> credentials = mCredentialCache.take( realm );
      username = credentials.first;
      password = credentials.second;
      QgsDebugMsgLevel( QStringLiteral( "retrieved realm:%1 username:%2" ).arg( realm, username ), 2 );

      if ( !password.isNull() )
        return true;
    }
  }

  if ( request( realm, username, password, message ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "requested realm:%1 username:%2" ).arg( realm, username ), 2 );
    return true;
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "unset realm:%1" ).arg( realm ), 4 );
    return false;
  }
}

void QgsCredentials::put( const QString &realm, const QString &username, const QString &password )
{
  const QMutexLocker locker( &mCacheMutex );
  QgsDebugMsgLevel( QStringLiteral( "inserting realm:%1 username:%2" ).arg( realm, username ), 2 );
  mCredentialCache.insert( realm, QPair<QString, QString>( username, password ) );
}

bool QgsCredentials::getMasterPassword( QString &password, bool stored )
{
  if ( requestMasterPassword( password, stored ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "requested master password" ), 2 );
    return true;
  }
  return false;
}

void QgsCredentials::lock()
{
  mAuthMutex.lock();
}

void QgsCredentials::unlock()
{
  mAuthMutex.unlock();
}


////////////////////////////////
// QgsCredentialsNone

QgsCredentialsNone::QgsCredentialsNone()
{
  setInstance( this );
}

bool QgsCredentialsNone::request( const QString &realm, QString &username, QString &password, const QString &message )
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

bool QgsCredentialsConsole::request( const QString &realm, QString &username, QString &password, const QString &message )
{
  QTextStream in( stdin, QIODevice::ReadOnly );
  QTextStream out( stdout, QIODevice::WriteOnly );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  out << "credentials for " << realm << endl;
#else
  out << "credentials for " << realm << Qt::endl;
#endif
  if ( !message.isEmpty() )
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    out << "message: " << message << endl;
#else
    out << "message: " << message << Qt::endl;
#endif
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

  const QString msg( stored ? "Master password for authentication configs: " : "Set master password for authentication configs: " );

  out << msg;
  in >> password;

  return true;
}
