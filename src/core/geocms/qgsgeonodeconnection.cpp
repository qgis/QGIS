/***************************************************************************
    qgsgeonodeconnection.cpp
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

#include "qgsgeonodeconnection.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsgeocmsconnection.h"
#include "qgsexception.h"

#include <QMultiMap>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QJsonDocument>
#include <QDebug>
#include <QUrl>
#include <QDomDocument>


const QString QgsGeoNodeConnection::pathGeoNodeConnection = "qgis/connections-geonode";
const QString QgsGeoNodeConnection::pathGeoNodeConnectionDetails = "qgis/GeoNode";

QgsGeoNodeConnection::QgsGeoNodeConnection( const QString &connName )
  : QgsGeoCmsConnection( QStringLiteral( "GeoNode" ), connName )
{
  QgsDebugMsg( "theConnName = " + connName );
  QgsDebugMsg( QString( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsGeoNodeConnection::~QgsGeoNodeConnection()
{

}

QStringList QgsGeoNodeConnection::connectionList()
{
  return QgsGeoCmsConnection::connectionList( QStringLiteral( "GeoNode" ) );
}

void QgsGeoNodeConnection::deleteConnection( const QString &name )
{
  return QgsGeoCmsConnection::deleteConnection( QStringLiteral( "GeoNode" ), name );
}

QString QgsGeoNodeConnection::selectedConnection()
{
  return QgsGeoCmsConnection::selectedConnection( QStringLiteral( "GeoNode" ) );
}

void QgsGeoNodeConnection::setSelectedConnection( const QString &name )
{
  return QgsGeoCmsConnection::setSelectedConnection( QStringLiteral( "GeoNode" ), name );
}
