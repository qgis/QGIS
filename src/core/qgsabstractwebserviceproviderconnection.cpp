/***************************************************************************
  qgsabstractwebserviceproviderconnection.cpp - QgsAbstractWebServiceProviderConnection

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
#include "qgsabstractwebserviceproviderconnection.h"
#include "qgssettings.h"

QgsAbstractWebServiceProviderConnection::QgsAbstractWebServiceProviderConnection( const QString &name ):
  QgsAbstractProviderConnection( name )
{
  // Note: concrete classes must implement the logic to read the configuration from the settings
  //       and create mUri
}

QgsAbstractWebServiceProviderConnection::QgsAbstractWebServiceProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractProviderConnection( uri, configuration )
{
  // Note: concrete classes must implement the logic to read the configuration from the settings
  //       and parse the uri
}

QgsAbstractWebServiceProviderConnection::Capabilities QgsAbstractWebServiceProviderConnection::capabilities() const
{
  return mCapabilities;
}

QList<QgsAbstractWebServiceProviderConnection::LayerProperty> QgsAbstractWebServiceProviderConnection::layers( const QgsAbstractWebServiceProviderConnection::LayerFlags &flags ) const
{
  Q_UNUSED( flags )
  checkCapability( Capability::Layers );
  return QList<QgsAbstractWebServiceProviderConnection::LayerProperty>();
}

QList<QgsAbstractWebServiceProviderConnection::LayerProperty> QgsAbstractWebServiceProviderConnection::layersInt( const int flags ) const
{
  return layers( static_cast<QgsAbstractWebServiceProviderConnection::LayerFlags>( flags ) );
}

QgsAbstractWebServiceProviderConnection::LayerProperty QgsAbstractWebServiceProviderConnection::layer( const QString &layerName ) const
{
  checkCapability( Capability::Layers );
  const QList<QgsAbstractWebServiceProviderConnection::LayerProperty> constLayers { layers( ) };
  for ( const auto &l : constLayers )
  {
    if ( l.layerName() == layerName )
    {
      return l;
    }
  }
  throw QgsProviderConnectionException( QObject::tr( "Layer '%1' was not found" )
                                        .arg( layerName ) );

}

QString QgsAbstractWebServiceProviderConnection::layerUri( const QString &layerName ) const
{
  Q_UNUSED( layerName )
  throw QgsProviderConnectionException( QObject::tr( "Operation 'layerUri' is not supported" ) );
}

///@cond PRIVATE
void QgsAbstractWebServiceProviderConnection::checkCapability( QgsAbstractWebServiceProviderConnection::Capability capability ) const
{
  if ( ! mCapabilities.testFlag( capability ) )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<QgsAbstractWebServiceProviderConnection::Capability>();
    const QString capName { metaEnum.valueToKey( capability ) };
    throw QgsProviderConnectionException( QObject::tr( "Operation '%1' is not supported for this connection" ).arg( capName ) );
  }
}
///@endcond


bool QgsAbstractWebServiceProviderConnection::layerExists( const QString &name ) const
{
  checkCapability( Capability::LayerExists );
  try
  {
    layer( name );
    return true;
  }
  catch ( QgsProviderConnectionException & )
  {
    return false;
  }
}

QgsCoordinateReferenceSystem QgsAbstractWebServiceProviderConnection::LayerProperty::crs() const
{
  return mCrs;
}

QString QgsAbstractWebServiceProviderConnection::LayerProperty::layerName() const
{
  return mLayerName;
}

void QgsAbstractWebServiceProviderConnection::LayerProperty::setLayerName( const QString &name )
{
  mLayerName = name;
}

QgsAbstractWebServiceProviderConnection::LayerFlags QgsAbstractWebServiceProviderConnection::LayerProperty::flags() const
{
  return mFlags;
}

void QgsAbstractWebServiceProviderConnection::LayerProperty::setFlags( const LayerFlags &flags )
{
  mFlags = flags;
}

QString QgsAbstractWebServiceProviderConnection::LayerProperty::comment() const
{
  return mComment;
}

void QgsAbstractWebServiceProviderConnection::LayerProperty::setComment( const QString &comment )
{
  mComment = comment;
}

QVariantMap QgsAbstractWebServiceProviderConnection::LayerProperty::info() const
{
  return mInfo;
}

void QgsAbstractWebServiceProviderConnection::LayerProperty::setInfo( const QVariantMap &info )
{
  mInfo = info;
}

void QgsAbstractWebServiceProviderConnection::LayerProperty::setFlag( const QgsAbstractWebServiceProviderConnection::LayerFlag &flag )
{
  mFlags.setFlag( flag );
}

void QgsAbstractWebServiceProviderConnection::LayerProperty::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

QgsWkbTypes::Type QgsAbstractWebServiceProviderConnection::LayerProperty::wkbType() const
{
  return mWkbType;
}

void QgsAbstractWebServiceProviderConnection::LayerProperty::setWkbType( const QgsWkbTypes::Type &wkbType )
{
  mWkbType = wkbType;
}

void QgsAbstractWebServiceProviderConnection::store( const QString &name ) const
{
  QgsSettings settings;
  QgsDataSourceUri dsUri( uri( ) );
  settings.beginGroup( QStringLiteral( "qgis/connections-%1/%2" )
                       .arg( mServiceName.toLower() )
                       .arg( name ) );
  settings.setValue( QStringLiteral( "url" ), dsUri.param( "url" ) );
  settings.setValue( QStringLiteral( "referer" ), dsUri.param( "referer" ) );
  settings.endGroup();

  settings.beginGroup( QStringLiteral( "qgis/%1" )
                       .arg( mServiceName.toUpper() ) );
  settings.setValue( QStringLiteral( "username" ), dsUri.username() );
  settings.setValue( QStringLiteral( "password" ), dsUri.password() );
  settings.setValue( QStringLiteral( "authcfg" ), dsUri.authConfigId() );
}

void QgsAbstractWebServiceProviderConnection::remove( const QString &name ) const
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "qgis/connections-%1/%2" )
                   .arg( mServiceName.toLower() )
                   .arg( name ) );
  settings.remove( QStringLiteral( "qgis/%1" )
                   .arg( mServiceName.toUpper() ) );
}
