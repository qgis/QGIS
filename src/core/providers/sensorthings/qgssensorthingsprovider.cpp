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
#include "qgssensorthingsutils.h"
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsthreadingutils.h"
#include "qgsreadwritelocker.h"

#include <QIcon>
#include <QNetworkRequest>
#include <nlohmann/json.hpp>

///@cond PRIVATE

QgsSensorThingsProvider::QgsSensorThingsProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  mSharedData = std::make_shared< QgsSensorThingsSharedData >( uri );

  const QUrl url( QgsSensorThingsSharedData::parseUrl( mSharedData->mRootUri ) );

  QNetworkRequest request = QNetworkRequest( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsSensorThingsProvider" ) )
  mSharedData->mHeaders.updateNetworkRequest( request );

  QgsBlockingNetworkRequest networkRequest;
  networkRequest.setAuthCfg( mSharedData->mAuthCfg );

  switch ( networkRequest.get( request ) )
  {
    case QgsBlockingNetworkRequest::NoError:
      break;

    case QgsBlockingNetworkRequest::NetworkError:
    case QgsBlockingNetworkRequest::TimeoutError:
    case QgsBlockingNetworkRequest::ServerExceptionError:
      appendError( QgsErrorMessage( tr( "Connection failed: %1" ).arg( networkRequest.errorMessage() ), QStringLiteral( "SensorThings" ) ) );
      return;
  }

  const QgsNetworkReplyContent content = networkRequest.reply();

  try
  {
    auto rootContent = json::parse( content.content().toStdString() );
    if ( !rootContent.contains( "value" ) )
    {
      appendError( QgsErrorMessage( tr( "No 'value' array in response" ), QStringLiteral( "SensorThings" ) ) );
      return;
    }

    bool foundMatchingEntity = false;
    for ( const auto &valueJson : rootContent["value"] )
    {
      if ( valueJson.contains( "name" ) && valueJson.contains( "url" ) )
      {
        const QString name = QString::fromStdString( valueJson["name"].get<std::string>() );
        Qgis::SensorThingsEntity entityType = QgsSensorThingsUtils::entitySetStringToEntity( name );
        if ( entityType == mSharedData->mEntityType )
        {
          const QString url = QString::fromStdString( valueJson["url"].get<std::string>() );
          if ( !url.isEmpty() )
          {
            foundMatchingEntity = true;
            mSharedData->mEntityBaseUri = url;
          }
        }
      }
    }

    if ( !foundMatchingEntity )
    {
      appendError( QgsErrorMessage( tr( "Could not find url for %1" ).arg( qgsEnumValueToKey( mSharedData->mEntityType ) ), QStringLiteral( "SensorThings" ) ) );
      return;
    }
  }
  catch ( const json::parse_error &ex )
  {
    appendError( QgsErrorMessage( tr( "Error parsing response: %1" ).arg( ex.what() ), QStringLiteral( "SensorThings" ) ) );
    return;
  }

  mValid = true;
}

QString QgsSensorThingsProvider::storageType() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "OGC SensorThings API" );
}

QgsAbstractFeatureSource *QgsSensorThingsProvider::featureSource() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

#if 0
  return new QgsSensorThingsFeatureSource( mSharedData );
#endif
}

QgsFeatureIterator QgsSensorThingsProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

#if 0
  return new QgsAfsFeatureIterator( new QgsAfsFeatureSource( mSharedData ), true, request );
#endif
}

Qgis::WkbType QgsSensorThingsProvider::wkbType() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSharedData->mGeometryType;
}

long long QgsSensorThingsProvider::featureCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

#if 0
  return mSharedData->featureCount();
#endif

  return static_cast< long long >( Qgis::FeatureCountState::UnknownCount );
}

QgsFields QgsSensorThingsProvider::fields() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSharedData->mFields;
}

QgsLayerMetadata QgsSensorThingsProvider::layerMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLayerMetadata;
}

QString QgsSensorThingsProvider::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString metadata;

  QgsReadWriteLocker locker( mSharedData->mReadWriteLock, QgsReadWriteLocker::Read );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Entity Type" ) % QStringLiteral( "</td><td>%1" ).arg( qgsEnumValueToKey( mSharedData->mEntityType ) ) % QStringLiteral( "</td></tr>\n" );
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Endpoint" ) % QStringLiteral( "</td><td><a href=\"%1\">%1</a>" ).arg( mSharedData->mEntityBaseUri ) % QStringLiteral( "</td></tr>\n" );

  return metadata;
}

QgsVectorDataProvider::Capabilities QgsSensorThingsProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsVectorDataProvider::Capabilities c = QgsVectorDataProvider::SelectAtId
                                          | QgsVectorDataProvider::ReadLayerMetadata
                                          | QgsVectorDataProvider::Capability::ReloadData;

  return c;
}

void QgsSensorThingsProvider::setDataSourceUri( const QString &uri )
{
  mSharedData = std::make_shared< QgsSensorThingsSharedData >( uri );
  QgsDataProvider::setDataSourceUri( uri );
}

QgsCoordinateReferenceSystem QgsSensorThingsProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSharedData->mSourceCRS;
}

QgsRectangle QgsSensorThingsProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

#if 0
  return mSharedData->extent();
#endif

  return QgsRectangle();
}

QString QgsSensorThingsProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

  const QString entityParam = dsUri.param( QStringLiteral( "entity" ) );
  Qgis::SensorThingsEntity entity = QgsSensorThingsUtils::entitySetStringToEntity( entityParam );
  if ( entity == Qgis::SensorThingsEntity::Invalid )
    entity = QgsSensorThingsUtils::stringToEntity( entityParam );

  if ( entity != Qgis::SensorThingsEntity::Invalid )
    components.insert( QStringLiteral( "entity" ), qgsEnumValueToKey( entity ) );

  switch ( QgsWkbTypes::geometryType( dsUri.wkbType() ) )
  {
    case Qgis::GeometryType::Point:
      if ( QgsWkbTypes::isMultiType( dsUri.wkbType() ) )
        components.insert( QStringLiteral( "geometryType" ), QStringLiteral( "multipoint" ) );
      else
        components.insert( QStringLiteral( "geometryType" ), QStringLiteral( "point" ) );
      break;
    case Qgis::GeometryType::Line:
      components.insert( QStringLiteral( "geometryType" ), QStringLiteral( "line" ) );
      break;
    case Qgis::GeometryType::Polygon:
      components.insert( QStringLiteral( "geometryType" ), QStringLiteral( "polygon" ) );
      break;

    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      break;
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

  Qgis::SensorThingsEntity entity = QgsSensorThingsUtils::entitySetStringToEntity(
                                      parts.value( QStringLiteral( "entity" ) ).toString() );
  if ( entity == Qgis::SensorThingsEntity::Invalid )
    entity = QgsSensorThingsUtils::stringToEntity( parts.value( QStringLiteral( "entity" ) ).toString() );

  if ( entity != Qgis::SensorThingsEntity::Invalid )
  {
    dsUri.setParam( QStringLiteral( "entity" ),
                    qgsEnumValueToKey( entity ) );
  }

  const QString geometryType = parts.value( QStringLiteral( "geometryType" ) ).toString();
  if ( geometryType.compare( QLatin1String( "point" ), Qt::CaseInsensitive ) == 0 )
  {
    dsUri.setWkbType( Qgis::WkbType::PointZ );
  }
  else if ( geometryType.compare( QLatin1String( "multipoint" ), Qt::CaseInsensitive ) == 0 )
  {
    dsUri.setWkbType( Qgis::WkbType::MultiPointZ );
  }
  else if ( geometryType.compare( QLatin1String( "line" ), Qt::CaseInsensitive ) == 0 )
  {
    dsUri.setWkbType( Qgis::WkbType::MultiLineStringZ );
  }
  else if ( geometryType.compare( QLatin1String( "polygon" ), Qt::CaseInsensitive ) == 0 )
  {
    dsUri.setWkbType( Qgis::WkbType::MultiPolygonZ );
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

///@endcond
