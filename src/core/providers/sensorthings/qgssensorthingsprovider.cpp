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

#include <nlohmann/json.hpp>

#include "qgsapplication.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsmessagelog.h"
#include "qgsreadwritelocker.h"
#include "qgssensorthingsconnection.h"
#include "qgssensorthingsdataitems.h"
#include "qgssensorthingsfeatureiterator.h"
#include "qgssensorthingsutils.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsthreadingutils.h"

#include <QIcon>
#include <QNetworkRequest>

#include "moc_qgssensorthingsprovider.cpp"

///@cond PRIVATE

QgsSensorThingsProvider::QgsSensorThingsProvider( const QString &uri, const ProviderOptions &options, Qgis::DataProviderReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  mSharedData = std::make_shared< QgsSensorThingsSharedData >( uri );

  const QUrl url( QgsSensorThingsSharedData::parseUrl( mSharedData->mRootUri ) );

  QNetworkRequest request = QNetworkRequest( url );
  QgsSetRequestInitiatorClass( request, u"QgsSensorThingsProvider"_s )
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
      appendError( QgsErrorMessage( tr( "Connection failed: %1" ).arg( networkRequest.errorMessage() ), u"SensorThings"_s ) );
      return;
  }

  const QgsNetworkReplyContent content = networkRequest.reply();

  try
  {
    auto rootContent = json::parse( content.content().toStdString() );
    if ( !rootContent.contains( "value" ) )
    {
      appendError( QgsErrorMessage( tr( "No 'value' array in response" ), u"SensorThings"_s ) );
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
          appendError( QgsErrorMessage( tr( "Could not find url for %1" ).arg( qgsEnumValueToKey( mSharedData->mEntityType ) ), u"SensorThings"_s ) );
          QgsMessageLog::logMessage( tr( "Could not find url for %1" ).arg( qgsEnumValueToKey( mSharedData->mEntityType ) ), tr( "SensorThings" ) );
          break;

        case Qgis::SensorThingsEntity::MultiDatastream:
          appendError( QgsErrorMessage( tr( "MultiDatastreams are not supported by this connection" ), u"SensorThings"_s ) );
          QgsMessageLog::logMessage( tr( "MultiDatastreams are not supported by this connection" ), tr( "SensorThings" ) );
          break;
      }

      return;
    }
  }
  catch ( const json::parse_error &ex )
  {
    appendError( QgsErrorMessage( tr( "Error parsing response: %1" ).arg( ex.what() ), u"SensorThings"_s ) );
    return;
  }

  mValid = true;
}

QString QgsSensorThingsProvider::storageType() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return u"OGC SensorThings API"_s;
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

  if ( ( mReadFlags & Qgis::DataProviderReadFlag::SkipFeatureCount ) != 0 )
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

  metadata += u"<tr><td class=\"highlight\">"_s % tr( "Entity Type" ) % u"</td><td>%1"_s.arg( qgsEnumValueToKey( mSharedData->mEntityType ) ) % u"</td></tr>\n"_s;
  metadata += u"<tr><td class=\"highlight\">"_s % tr( "Endpoint" ) % u"</td><td><a href=\"%1\">%1</a>"_s.arg( mSharedData->mEntityBaseUri ) % u"</td></tr>\n"_s;

  return metadata;
}

Qgis::DataProviderFlags QgsSensorThingsProvider::flags() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::DataProviderFlag::FastExtent2D;
}

Qgis::VectorProviderCapabilities QgsSensorThingsProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Qgis::VectorProviderCapabilities c = Qgis::VectorProviderCapability::SelectAtId
                                       | Qgis::VectorProviderCapability::ReadLayerMetadata
                                       | Qgis::VectorProviderCapability::ReloadData;

  return c;
}

bool QgsSensorThingsProvider::supportsSubsetString() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return true;
}

QString QgsSensorThingsProvider::subsetStringDialect() const
{
  return tr( "OGC SensorThings filter" );
}

QString QgsSensorThingsProvider::subsetStringHelpUrl() const
{
  return u"https://docs.ogc.org/is/18-088/18-088.html#filter"_s;
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
  return QgsApplication::getThemeIcon( u"mIconSensorThings.svg"_s );
}

QList<QgsDataItemProvider *> QgsSensorThingsProviderMetadata::dataItemProviders() const
{
  return { new QgsSensorThingsDataItemProvider() };
}

QVariantMap QgsSensorThingsProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( u"url"_s, dsUri.param( u"url"_s ) );

  if ( !dsUri.authConfigId().isEmpty() )
  {
    components.insert( u"authcfg"_s, dsUri.authConfigId() );
  }
  if ( !dsUri.username().isEmpty() )
  {
    components.insert( u"username"_s, dsUri.username() );
  }
  if ( !dsUri.password().isEmpty() )
  {
    components.insert( u"password"_s, dsUri.password() );
  }

  // there's two different ways the referer can be set, so we need to check both. Which way it has been
  // set depends on the widget used to create the URI. It's messy, but QgsHttpHeaders has a bunch of logic in
  // it to handle upgrading old referer handling for connections created before QgsHttpHeaders was invented,
  // and if we rely on that entirely then we get multiple "referer" parameters included in the URI, which is
  // both ugly and unnecessary for a provider created post QgsHttpHeaders.
  if ( !dsUri.param( u"referer"_s ).isEmpty() )
  {
    components.insert( u"referer"_s, dsUri.param( u"referer"_s ) );
  }
  if ( !dsUri.param( u"http-header:referer"_s ).isEmpty() )
  {
    components.insert( u"referer"_s, dsUri.param( u"http-header:referer"_s ) );
  }

  const QString entityParam = dsUri.param( u"entity"_s );
  Qgis::SensorThingsEntity entity = QgsSensorThingsUtils::entitySetStringToEntity( entityParam );
  if ( entity == Qgis::SensorThingsEntity::Invalid )
    entity = QgsSensorThingsUtils::stringToEntity( entityParam );

  if ( entity != Qgis::SensorThingsEntity::Invalid )
    components.insert( u"entity"_s, qgsEnumValueToKey( entity ) );

  const QStringList expandToParam = dsUri.param( u"expandTo"_s ).split( ';', Qt::SkipEmptyParts );
  if ( !expandToParam.isEmpty() )
  {
    QVariantList expandParts;
    for ( const QString &expandString : expandToParam )
    {
      const QgsSensorThingsExpansionDefinition definition = QgsSensorThingsExpansionDefinition::fromString( expandString );
      if ( definition.isValid() )
      {
        expandParts.append( QVariant::fromValue( definition ) );
      }
    }
    if ( !expandParts.isEmpty() )
    {
      components.insert( u"expandTo"_s, expandParts );
    }
  }

  bool ok = false;
  const int maxPageSizeParam = dsUri.param( u"pageSize"_s ).toInt( &ok );
  if ( ok )
  {
    components.insert( u"pageSize"_s, maxPageSizeParam );
  }

  ok = false;
  const int featureLimitParam = dsUri.param( u"featureLimit"_s ).toInt( &ok );
  if ( ok )
  {
    components.insert( u"featureLimit"_s, featureLimitParam );
  }

  switch ( QgsWkbTypes::geometryType( dsUri.wkbType() ) )
  {
    case Qgis::GeometryType::Point:
      if ( QgsWkbTypes::isMultiType( dsUri.wkbType() ) )
        components.insert( u"geometryType"_s, u"multipoint"_s );
      else
        components.insert( u"geometryType"_s, u"point"_s );
      break;
    case Qgis::GeometryType::Line:
      components.insert( u"geometryType"_s, u"line"_s );
      break;
    case Qgis::GeometryType::Polygon:
      components.insert( u"geometryType"_s, u"polygon"_s );
      break;

    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      break;
  }

  const QStringList bbox = dsUri.param( u"bbox"_s ).split( ',' );
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
      components.insert( u"bounds"_s, r );
  }

  if ( !dsUri.sql().isEmpty() )
    components.insert( u"sql"_s, dsUri.sql() );

  return components;
}

QString QgsSensorThingsProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( u"url"_s, parts.value( u"url"_s ).toString() );

  if ( !parts.value( u"authcfg"_s ).toString().isEmpty() )
  {
    dsUri.setAuthConfigId( parts.value( u"authcfg"_s ).toString() );
  }
  if ( !parts.value( u"username"_s ).toString().isEmpty() )
  {
    dsUri.setUsername( parts.value( u"username"_s ).toString() );
  }
  if ( !parts.value( u"password"_s ).toString().isEmpty() )
  {
    dsUri.setPassword( parts.value( u"password"_s ).toString() );
  }
  if ( !parts.value( u"referer"_s ).toString().isEmpty() )
  {
    dsUri.setParam( u"referer"_s, parts.value( u"referer"_s ).toString() );
  }

  Qgis::SensorThingsEntity entity = QgsSensorThingsUtils::entitySetStringToEntity(
                                      parts.value( u"entity"_s ).toString() );
  if ( entity == Qgis::SensorThingsEntity::Invalid )
    entity = QgsSensorThingsUtils::stringToEntity( parts.value( u"entity"_s ).toString() );

  if ( entity != Qgis::SensorThingsEntity::Invalid )
  {
    dsUri.setParam( u"entity"_s,
                    qgsEnumValueToKey( entity ) );
  }

  const QVariantList expandToParam = parts.value( u"expandTo"_s ).toList();
  if ( !expandToParam.isEmpty() )
  {
    QStringList expandToStringList;
    for ( const QVariant &expansion : expandToParam )
    {
      const QgsSensorThingsExpansionDefinition expansionDefinition = expansion.value< QgsSensorThingsExpansionDefinition >();
      if ( !expansionDefinition.isValid() )
        continue;

      expandToStringList.append( expansionDefinition.toString() );
    }
    if ( !expandToStringList.isEmpty() )
    {
      dsUri.setParam( u"expandTo"_s, expandToStringList.join( ';' ) );
    }
  }

  bool ok = false;
  const int maxPageSizeParam = parts.value( u"pageSize"_s ).toInt( &ok );
  if ( ok )
  {
    dsUri.setParam( u"pageSize"_s, QString::number( maxPageSizeParam ) );
  }

  ok = false;
  const int featureLimitParam = parts.value( u"featureLimit"_s ).toInt( &ok );
  if ( ok )
  {
    dsUri.setParam( u"featureLimit"_s, QString::number( featureLimitParam ) );
  }

  const QString geometryType = parts.value( u"geometryType"_s ).toString();
  if ( geometryType.compare( "point"_L1, Qt::CaseInsensitive ) == 0 )
  {
    dsUri.setWkbType( Qgis::WkbType::PointZ );
  }
  else if ( geometryType.compare( "multipoint"_L1, Qt::CaseInsensitive ) == 0 )
  {
    dsUri.setWkbType( Qgis::WkbType::MultiPointZ );
  }
  else if ( geometryType.compare( "line"_L1, Qt::CaseInsensitive ) == 0 )
  {
    dsUri.setWkbType( Qgis::WkbType::MultiLineStringZ );
  }
  else if ( geometryType.compare( "polygon"_L1, Qt::CaseInsensitive ) == 0 )
  {
    dsUri.setWkbType( Qgis::WkbType::MultiPolygonZ );
  }

  if ( parts.contains( u"bounds"_s ) && parts.value( u"bounds"_s ).userType() == qMetaTypeId<QgsRectangle>() )
  {
    const QgsRectangle bBox = parts.value( u"bounds"_s ).value< QgsRectangle >();
    dsUri.setParam( u"bbox"_s, u"%1,%2,%3,%4"_s.arg( bBox.xMinimum() ).arg( bBox.yMinimum() ).arg( bBox.xMaximum() ).arg( bBox.yMaximum() ) );
  }

  if ( !parts.value( u"sql"_s ).toString().isEmpty() )
    dsUri.setSql( parts.value( u"sql"_s ).toString() );

  return dsUri.uri( false );
}

QgsSensorThingsProvider *QgsSensorThingsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
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
