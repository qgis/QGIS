#include "qgsamsproviderconnection.h"
#include "qgsowsconnection.h"
#include "qgssettings.h"

QgsAmsProviderConnection::QgsAmsProviderConnection( const QString &name )
  : QgsAbstractWebServiceProviderConnection( name )
{
  mServiceName = QStringLiteral( "arcgismapserver" );
  // Load from settings, note that this is totally wrong:
  // auth info for a connection named FEMA is stored as
  // [qgis]
  // ARCGISMAPSERVER\FEMA\authcfg=r7sp589
  // ARCGISMAPSERVER\FEMA\password=
  //
  // while connection info is under group:
  // connections-arcgismapserver\FEMA\referer=
  // connections-arcgismapserver\FEMA\url=https://hazards.fema.gov/gis/nfhl/rest/services/public/NFHL/MapServer/28

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

  setDefaultCapabilities();
}

QgsAmsProviderConnection::QgsAmsProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractWebServiceProviderConnection( uri, configuration )
{
  mServiceName = QStringLiteral( "arcgismapserver" );
  setDefaultCapabilities();
}

QString QgsAmsProviderConnection::layerUri( const QString &layerName ) const
{
  // TODO
  return QString();
}

QList<QgsAbstractWebServiceProviderConnection::LayerProperty> QgsAmsProviderConnection::layers( const QgsAbstractWebServiceProviderConnection::LayerFlags &flags ) const
{
  QList<QgsAbstractWebServiceProviderConnection::LayerProperty> result;
  // TODO
  return result;
}

void QgsAmsProviderConnection::setDefaultCapabilities()
{
  mCapabilities =
  {
    Capability::Layers,
    Capability::LayerExists,
    Capability::Spatial
  };
}
