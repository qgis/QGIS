/***************************************************************************
  qgsafsproviderconnection.cpp - QgsAfsProviderConnection

 ---------------------
 begin                : 21.11.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsafsproviderconnection.h"
#include "qgsowsconnection.h"
#include "qgssettings.h"

QgsAfsProviderConnection::QgsAfsProviderConnection( const QString &name )
  : QgsAbstractWebServiceProviderConnection( name )
{
  mServiceName = QStringLiteral( "arcgisfeatureserver" );
  // Load from settings, note that this is totally wrong:
  // auth info for a connection named FEMA is stored as
  // [qgis]
  // ARCGISFEATURESERVER\FEMA\authcfg=r7sp589
  // ARCGISFEATURESERVER\FEMA\password=
  //
  // while connection info is under group:
  // connections-arcgisfeatureserver\FEMA\referer=
  // connections-arcgisfeatureserver\FEMA\url=https://hazards.fema.gov/gis/nfhl/rest/services/public/NFHL/MapServer/28

  QgsDataSourceUri dsUri;
  QgsSettings settings;

  // Read auth info
  settings.beginGroup( QStringLiteral( "qgis" ) );
  settings.beginGroup( mServiceName.toUpper() );
  settings.beginGroup( name );
  dsUri.setUsername( settings.value( QStringLiteral( "username" ) ).toString() );
  dsUri.setPassword( settings.value( QStringLiteral( "password" ) ).toString() );
  dsUri.setAuthConfigId( settings.value( QStringLiteral( "authcfg" ) ).toString() );
  settings.endGroup();
  settings.endGroup();

  // Read connection info
  settings.beginGroup( QStringLiteral( "connections-%1" ).arg( mServiceName.toLower() ) );
  settings.beginGroup( name );
  dsUri.setParam( QStringLiteral( "url" ), settings.value( QStringLiteral( "url" ) ).toString() );
  dsUri.setParam( QStringLiteral( "referer" ), settings.value( QStringLiteral( "referer" ) ).toString() );

  setUri( dsUri.uri( ) );
}

QgsAfsProviderConnection::QgsAfsProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractWebServiceProviderConnection( uri, configuration )
{
  mServiceName = QStringLiteral( "arcgisfeatureserver" );
  // TODO: Fix/clear URI strip away unneded parts if URI comes from a layer uri (if possible)
  mCapabilities =
  {
    Capability::Layers,
    Capability::LayerExists,
    Capability::Spatial
  };
}

QString QgsAfsProviderConnection::layerUri( const QString &layerName ) const
{
  // TODO
}

QList<QgsAbstractWebServiceProviderConnection::LayerProperty> QgsAfsProviderConnection::layers( const QgsAbstractWebServiceProviderConnection::LayerFlags &flags ) const
{
  // TODO
}
