/***************************************************************************
      qgssensorthingsprovider.cpp
      ----------------
    begin                : November 2023
    copyright            : (C) 2013 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingsprovider.h"
#include "qgsapplication.h"

#include <QIcon>

QgsSensorThingsProvider::QgsSensorThingsProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  mSharedData = std::make_shared< QgsSensorThingsSharedData >( QgsDataSourceUri( uri ) );

  mValid = true;
}

QString QgsSensorThingsProvider::storageType() const
{
  return QStringLiteral( "OGC SensorThings API" );
}

QgsAbstractFeatureSource *QgsSensorThingsProvider::featureSource() const
{
#if 0
  return new QgsSensorThingsFeatureSource( mSharedData );
#endif
}

QgsFeatureIterator QgsSensorThingsProvider::getFeatures( const QgsFeatureRequest &request ) const
{
#if 0
  return new QgsAfsFeatureIterator( new QgsAfsFeatureSource( mSharedData ), true, request );
#endif
}

Qgis::WkbType QgsSensorThingsProvider::wkbType() const
{
  return mSharedData->mGeometryType;
}

long long QgsSensorThingsProvider::featureCount() const
{
#if 0
  return mSharedData->featureCount();
#endif
}

QgsFields QgsSensorThingsProvider::fields() const
{
  return mSharedData->mFields;
}

QgsLayerMetadata QgsSensorThingsProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QgsVectorDataProvider::Capabilities QgsSensorThingsProvider::capabilities() const
{
  QgsVectorDataProvider::Capabilities c = QgsVectorDataProvider::SelectAtId
                                          | QgsVectorDataProvider::ReadLayerMetadata
                                          | QgsVectorDataProvider::Capability::ReloadData;

#if 0
  if ( mServerSupportsCurves )
    c |= QgsVectorDataProvider::CircularGeometries;
#endif

  return c;
}

void QgsSensorThingsProvider::setDataSourceUri( const QString &uri )
{
  mSharedData->mDataSource = QgsDataSourceUri( uri );
  QgsDataProvider::setDataSourceUri( uri );
}

QgsCoordinateReferenceSystem QgsSensorThingsProvider::crs() const
{
  return mSharedData->mSourceCRS;
}

QgsRectangle QgsSensorThingsProvider::extent() const
{
#if 0
  return mSharedData->extent();
#endif
}

QString QgsSensorThingsProvider::name() const
{
  return SENSORTHINGS_PROVIDER_KEY;
}

QString QgsSensorThingsProvider::providerKey()
{
  return SENSORTHINGS_PROVIDER_KEY;
}

void QgsSensorThingsProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  mSharedData = qobject_cast<QgsSensorThingsProvider *>( source )->mSharedData;
}

QString QgsSensorThingsProvider::description() const
{
  return SENSORTHINGS_PROVIDER_DESCRIPTION;
}

QString QgsSensorThingsProvider::dataComment() const
{
#if 0
  return mLayerDescription;
#endif
}

void QgsSensorThingsProvider::reloadProviderData()
{
#if 0
  mSharedData->clearCache();
#endif
}

//
// QgsSensorThingsProviderMetadata
//

QgsSensorThingsProviderMetadata::QgsSensorThingsProviderMetadata():
  QgsProviderMetadata( QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_DESCRIPTION )
{
}

QIcon QgsSensorThingsProviderMetadata::icon() const
{
  // TODO
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconAfs.svg" ) );
}

QList<QgsDataItemProvider *> QgsSensorThingsProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;

#if 0
  providers
      << new QgsArcGisRestDataItemProvider;
#endif

  return providers;
}

QVariantMap QgsSensorThingsProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );

  if ( !dsUri.authConfigId().isEmpty() )
  {
    components.insert( QStringLiteral( "authcfg" ), dsUri.authConfigId() );
  }
  return components;
}

QString QgsSensorThingsProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "url" ), parts.value( QStringLiteral( "url" ) ).toString() );

  if ( !parts.value( QStringLiteral( "authcfg" ) ).toString().isEmpty() )
  {
    dsUri.setAuthConfigId( parts.value( QStringLiteral( "authcfg" ) ).toString() );
  }
  return dsUri.uri( false );
}

QgsSensorThingsProvider *QgsSensorThingsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsSensorThingsProvider( uri, options, flags );
}

QList<Qgis::LayerType> QgsSensorThingsProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}
