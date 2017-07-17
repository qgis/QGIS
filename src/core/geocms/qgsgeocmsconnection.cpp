/***************************************************************************
    qgsgeocmsconnection.cpp
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeocmsconnection.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"

#include <QMultiMap>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QJsonDocument>
#include <QDebug>
#include <QUrl>
#include <QDomDocument>


QgsGeoCmsConnection::QgsGeoCmsConnection( const QString &geoCMSName, const QString &connName )
  : mGeoCMSName( geoCMSName )
  , mConnName( connName )
{
  QgsDebugMsg( "Connection name = " + connName );

  QgsSettings settings;

  QString key = "qgis/connections-" + mGeoCMSName.toLower() + '/' + mConnName;
  QString credentialsKey = "qgis/" + mGeoCMSName + '/' + mConnName;

  QStringList connStringParts;

  mUri.setParam( QStringLiteral( "url" ), settings.value( key + "/url" ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "username" ), username );
    mUri.setParam( QStringLiteral( "password" ), password );
  }

  QString authcfg = settings.value( credentialsKey + "/authcfg" ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "authcfg" ), authcfg );
  }

  QgsDebugMsg( QString( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsGeoCmsConnection::~QgsGeoCmsConnection()
{

}

QgsDataSourceUri QgsGeoCmsConnection::uri()
{
  return mUri;
}

QStringList QgsGeoCmsConnection::connectionList( const QString &geoCMSName )
{
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-" + geoCMSName.toLower() );
  return settings.childGroups();
}

void QgsGeoCmsConnection::deleteConnection( const QString &geoCMSName, const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-" + geoCMSName.toLower() + '/' + name );
  settings.remove( "qgis/" + geoCMSName + '/' + name );
}

QString QgsGeoCmsConnection::selectedConnection( const QString &geoCMSName )
{
  QgsSettings settings;
  return settings.value( "qgis/connections-" + geoCMSName.toLower() + "/selected" ).toString();
}

void QgsGeoCmsConnection::setSelectedConnection( const QString &geoCMSName, const QString &name )
{
  QgsSettings settings;
  settings.setValue( "qgis/connections-" + geoCMSName.toLower() + "/selected", name );
}

QString QgsGeoCmsConnection::geoCMSName() const
{
  return mGeoCMSName;
}

void QgsGeoCmsConnection::setGeoCMSName( const QString &geoCMSName )
{
  mGeoCMSName = geoCMSName;
}

QString QgsGeoCmsConnection::connName() const
{
  return mConnName;
}

void QgsGeoCmsConnection::setConnName( const QString &connName )
{
  mConnName = connName;
}

void QgsGeoCmsConnection::setUri( const QgsDataSourceUri &uri )
{
  mUri = uri;
}
