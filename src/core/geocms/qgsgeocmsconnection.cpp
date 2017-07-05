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


QgsGeoCMSConnection::QgsGeoCMSConnection( const QString &geoCMSName, const QString &connName )
  : mGeoCMSName( geoCMSName )
  , mConnName( connName )
{
  QgsDebugMsg( "theConnName = " + connName );

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

QgsGeoCMSConnection::~QgsGeoCMSConnection()
{

}

QgsDataSourceUri QgsGeoCMSConnection::uri()
{
  return mUri;
}

QStringList QgsGeoCMSConnection::connectionList( const QString &geoCMSName )
{
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-" + geoCMSName.toLower() );
  return settings.childGroups();
}

void QgsGeoCMSConnection::deleteConnection( const QString &geoCMSName, const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-" + geoCMSName.toLower() + '/' + name );
  settings.remove( "qgis/" + geoCMSName + '/' + name );
}

QString QgsGeoCMSConnection::selectedConnection( const QString &geoCMSName )
{
  QgsSettings settings;
  return settings.value( "qgis/connections-" + geoCMSName.toLower() + "/selected" ).toString();
}

void QgsGeoCMSConnection::setSelectedConnection( const QString &geoCMSName, const QString &name )
{
  QgsSettings settings;
  settings.setValue( "qgis/connections-" + geoCMSName.toLower() + "/selected", name );
}
