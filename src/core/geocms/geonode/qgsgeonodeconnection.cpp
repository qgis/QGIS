/***************************************************************************
    qgsgeonodeconnection.cpp
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgssettings.h"
#include "qgsgeonodeconnection.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsowsconnection.h"

const QString QgsGeoNodeConnectionUtils::sPathGeoNodeConnection = QStringLiteral( "qgis/connections-geonode" );
const QString QgsGeoNodeConnectionUtils::sPathGeoNodeConnectionDetails = QStringLiteral( "qgis/GeoNode" );

QgsGeoNodeConnection::QgsGeoNodeConnection( const QString &name )
  : mConnName( name )
{
  QgsSettings settings;

//  settings.Section
  QString key = settingsKey();
  QString credentialsKey = QgsGeoNodeConnectionUtils::pathGeoNodeConnectionDetails() + QStringLiteral( "/" ) + mConnName;

  mUri.setParam( QStringLiteral( "url" ), settings.value( key + QStringLiteral( "/url" ), QString() ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + QStringLiteral( "/username" ), QString() ).toString();
  QString password = settings.value( credentialsKey + QStringLiteral( "/password" ), QString() ).toString();
  if ( !username.isEmpty() )
  {
    mUri.setUsername( username );
    mUri.setPassword( password );
  }

  QString authcfg = settings.value( credentialsKey + QStringLiteral( "/authcfg" ), QString() ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setAuthConfigId( authcfg );
  }

  QgsDebugMsgLevel( QStringLiteral( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ), 4 );
}

QgsDataSourceUri QgsGeoNodeConnection::uri() const
{
  return mUri;
}

QString QgsGeoNodeConnection::connectionName() const
{
  return mConnName;
}

void QgsGeoNodeConnection::setConnectionName( const QString &connName )
{
  mConnName = connName;
}

void QgsGeoNodeConnection::setUri( const QgsDataSourceUri &uri )
{
  mUri = uri;
}

QgsDataSourceUri &QgsGeoNodeConnection::addWmsConnectionSettings( QgsDataSourceUri &uri ) const
{
  return QgsOwsConnection::addWmsWcsConnectionSettings( uri, settingsKey() + QStringLiteral( "/wms" ) );
}

QgsDataSourceUri &QgsGeoNodeConnection::addWfsConnectionSettings( QgsDataSourceUri &uri ) const
{
  return QgsOwsConnection::addWfsConnectionSettings( uri, settingsKey() + QStringLiteral( "/wfs" ) );
}

QString QgsGeoNodeConnection::settingsKey() const
{
  return QgsGeoNodeConnectionUtils::pathGeoNodeConnection() + QStringLiteral( "/" ) + mConnName;
}


//
// QgsGeoNodeConnectionUtils
//


QStringList QgsGeoNodeConnectionUtils::connectionList()
{
  QgsSettings settings;
  // Add Section manually
  settings.beginGroup( QStringLiteral( "qgis/connections-geonode" ) );
  return settings.childGroups();
}

void QgsGeoNodeConnectionUtils::deleteConnection( const QString &name )
{
  QgsOwsConnection::deleteConnection( QStringLiteral( "GEONODE" ), name );
}

QString QgsGeoNodeConnectionUtils::pathGeoNodeConnection()
{
  return sPathGeoNodeConnection;
}

QString QgsGeoNodeConnectionUtils::pathGeoNodeConnectionDetails()
{
  return sPathGeoNodeConnectionDetails;
}
