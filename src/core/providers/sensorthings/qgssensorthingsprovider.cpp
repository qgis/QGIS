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
#include "qgssetrequestinitiator_p.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsthreadingutils.h"
#include "qgsreadwritelocker.h"
#include "qgssensorthingsfeatureiterator.h"
#include "qgssensorthingsdataitems.h"
#include "qgssensorthingsconnection.h"
#include "qgsmessagelog.h"

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

            // TODO:
            // if we always retrieve feature count, is that less expensive then deferring this till we need it?
            // by retrieving upfront, we can save a lot of requests where we've fetched features from spatial extents
            // as we'll have a way of determining whether we've fetched all features from the source. Otherwise
            // we never know if we've got everything yet, and are forced to re-fetched everything when a non-filtered request
            // comes in...
            ( void ) mSharedData->featureCount();
          }
        }
      }
    }

    if ( !foundMatchingEntity )
    {
      switch ( mSharedData->mEntityType )
      {

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::Location:
        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
        case Qgis::SensorThingsEntity::Observation:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
          appendError( QgsErrorMessage( tr( "Could not find url for %1" ).arg( qgsEnumValueToKey( mSharedData->mEntityType ) ), QStringLiteral( "SensorThings" ) ) );
          QgsMessageLog::logMessage( tr( "Could not find url for %1" ).arg( qgsEnumValueToKey( mSharedData->mEntityType ) ), tr( "SensorThings" ) );
          break;

        case Qgis::SensorThingsEntity::MultiDatastream:
          appendError( QgsErrorMessage( tr( "MultiDatastreams are not supported by this connection" ), QStringLiteral( "SensorThings" ) ) );
          QgsMessageLog::logMessage( tr( "MultiDatastreams are not supported by this connection" ), tr( "SensorThings" ) );
          break;
      }

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

  return new QgsSensorThingsFeatureSource( mSharedData );
}

QgsFeatureIterator QgsSensorThingsProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsSensorThingsFeatureIterator( new QgsSensorThingsFeatureSource( mSharedData ), true, request );
}

Qgis::WkbType QgsSensorThingsProvider::wkbType() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSharedData->mGeometryType;
}

long long QgsSensorThingsProvider::featureCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( ( mReadFlags & QgsDataProvider::SkipFeatureCount ) != 0 )
  {
    return static_cast< long long >( Qgis::FeatureCountState::UnknownCount );
  }

  const long long count = mSharedData->featureCount();
  if ( !mSharedData->error().isEmpty() )
    pushError( mSharedData->error() );

  return count;
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

Qgis::DataProviderFlags QgsSensorThingsProvider::flags() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::DataProviderFlag::FastExtent2D;
}

QgsVectorDataProvider::Capabilities QgsSensorThingsProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsVectorDataProvider::Capabilities c = QgsVectorDataProvider::SelectAtId
                                          | QgsVectorDataProvider::ReadLayerMetadata
                                          | QgsVectorDataProvider::Capability::ReloadData;

  return c;
}

bool QgsSensorThingsProvider::supportsSubsetString() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return true;
}

QString QgsSensorThingsProvider::subsetString() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return mSharedData->subsetString();
}

bool QgsSensorThingsProvider::setSubsetString( const QString &subset, bool )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QString trimmedSubset = subset.trimmed();
  if ( trimmedSubset == mSharedData->subsetString() )
    return true;

  // store this and restore it after the data source is changed,
  // to avoid an unwanted network request to retrieve this again
  const QString baseUri = mSharedData->mEntityBaseUri;

  QgsDataSourceUri uri = dataSourceUri();
  uri.setSql( trimmedSubset );
  setDataSourceUri( uri.uri( false ) );

  mSharedData->mEntityBaseUri = baseUri;

  clearMinMaxCache();

  emit dataChanged();

  return true;
}

void QgsSensorThingsProvider::setDataSourceUri( const QString &uri )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  return mSharedData->extent();
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

bool QgsSensorThingsProvider::renderInPreview( const PreviewContext & )
{
  // be nice to the endpoint and don't make any requests we don't have to!
  return false;
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
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconSensorThings.svg" ) );
}

QList<QgsDataItemProvider *> QgsSensorThingsProviderMetadata::dataItemProviders() const
{
  return { new QgsSensorThingsDataItemProvider() };
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
  if ( !dsUri.username().isEmpty() )
  {
    components.insert( QStringLiteral( "username" ), dsUri.username() );
  }
  if ( !dsUri.password().isEmpty() )
  {
    components.insert( QStringLiteral( "password" ), dsUri.password() );
  }

  // there's two different ways the referer can be set, so we need to check both. Which way it has been
  // set depends on the widget used to create the URI. It's messy, but QgsHttpHeaders has a bunch of logic in
  // it to handle upgrading old referer handling for connections created before QgsHttpHeaders was invented,
  // and if we rely on that entirely then we get multiple "referer" parameters included in the URI, which is
  // both ugly and unnecessary for a provider created post QgsHttpHeaders.
  if ( !dsUri.param( QStringLiteral( "referer" ) ).isEmpty() )
  {
    components.insert( QStringLiteral( "referer" ), dsUri.param( QStringLiteral( "referer" ) ) );
  }
  if ( !dsUri.param( QStringLiteral( "http-header:referer" ) ).isEmpty() )
  {
    components.insert( QStringLiteral( "referer" ), dsUri.param( QStringLiteral( "http-header:referer" ) ) );
  }

  const QString entityParam = dsUri.param( QStringLiteral( "entity" ) );
  Qgis::SensorThingsEntity entity = QgsSensorThingsUtils::entitySetStringToEntity( entityParam );
  if ( entity == Qgis::SensorThingsEntity::Invalid )
    entity = QgsSensorThingsUtils::stringToEntity( entityParam );

  if ( entity != Qgis::SensorThingsEntity::Invalid )
    components.insert( QStringLiteral( "entity" ), qgsEnumValueToKey( entity ) );

  bool ok = false;
  const int maxPageSizeParam = dsUri.param( QStringLiteral( "pageSize" ) ).toInt( &ok );
  if ( ok )
  {
    components.insert( QStringLiteral( "pageSize" ), maxPageSizeParam );
  }

  ok = false;
  const int featureLimitParam = dsUri.param( QStringLiteral( "featureLimit" ) ).toInt( &ok );
  if ( ok )
  {
    components.insert( QStringLiteral( "featureLimit" ), featureLimitParam );
  }

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

  const QStringList bbox = dsUri.param( QStringLiteral( "bbox" ) ).split( ',' );
  if ( bbox.size() == 4 )
  {
    QgsRectangle r;
    bool xminOk = false;
    bool yminOk = false;
    bool xmaxOk = false;
    bool ymaxOk = false;
    r.setXMinimum( bbox[0].toDouble( &xminOk ) );
    r.setYMinimum( bbox[1].toDouble( &yminOk ) );
    r.setXMaximum( bbox[2].toDouble( &xmaxOk ) );
    r.setYMaximum( bbox[3].toDouble( &ymaxOk ) );
    if ( xminOk && yminOk && xmaxOk && ymaxOk )
      components.insert( QStringLiteral( "bounds" ), r );
  }

  if ( !dsUri.sql().isEmpty() )
    components.insert( QStringLiteral( "sql" ), dsUri.sql() );

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
  if ( !parts.value( QStringLiteral( "username" ) ).toString().isEmpty() )
  {
    dsUri.setUsername( parts.value( QStringLiteral( "username" ) ).toString() );
  }
  if ( !parts.value( QStringLiteral( "password" ) ).toString().isEmpty() )
  {
    dsUri.setPassword( parts.value( QStringLiteral( "password" ) ).toString() );
  }
  if ( !parts.value( QStringLiteral( "referer" ) ).toString().isEmpty() )
  {
    dsUri.setParam( QStringLiteral( "referer" ), parts.value( QStringLiteral( "referer" ) ).toString() );
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

  bool ok = false;
  const int maxPageSizeParam = parts.value( QStringLiteral( "pageSize" ) ).toInt( &ok );
  if ( ok )
  {
    dsUri.setParam( QStringLiteral( "pageSize" ), QString::number( maxPageSizeParam ) );
  }

  ok = false;
  const int featureLimitParam = parts.value( QStringLiteral( "featureLimit" ) ).toInt( &ok );
  if ( ok )
  {
    dsUri.setParam( QStringLiteral( "featureLimit" ), QString::number( featureLimitParam ) );
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

  if ( parts.contains( QStringLiteral( "bounds" ) ) && parts.value( QStringLiteral( "bounds" ) ).userType() == QMetaType::type( "QgsRectangle" ) )
  {
    const QgsRectangle bBox = parts.value( QStringLiteral( "bounds" ) ).value< QgsRectangle >();
    dsUri.setParam( QStringLiteral( "bbox" ), QStringLiteral( "%1,%2,%3,%4" ).arg( bBox.xMinimum() ).arg( bBox.yMinimum() ).arg( bBox.xMaximum() ).arg( bBox.yMaximum() ) );
  }

  if ( !parts.value( QStringLiteral( "sql" ) ).toString().isEmpty() )
    dsUri.setSql( parts.value( QStringLiteral( "sql" ) ).toString() );

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

QMap<QString, QgsAbstractProviderConnection *> QgsSensorThingsProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsSensorThingsProviderConnection, QgsSensorThingsProviderConnection>( cached );
}

QgsAbstractProviderConnection *QgsSensorThingsProviderMetadata::createConnection( const QString &name )
{
  return new QgsSensorThingsProviderConnection( name );
}

void QgsSensorThingsProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsSensorThingsProviderConnection>( name );
}

void QgsSensorThingsProviderMetadata::saveConnection( const QgsAbstractProviderConnection *connection, const QString &name )
{
  saveConnectionProtected( connection, name );
}

///@endcond
