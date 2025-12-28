/***************************************************************************
    qgsoapifprovider.cpp
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoapifprovider.h"

#include <algorithm>

#include "qgsbackgroundcachedfeatureiterator.h"
#include "qgsbackgroundcachedfeaturesource.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsoapifapirequest.h"
#include "qgsoapifcollection.h"
#include "qgsoapifconformancerequest.h"
#include "qgsoapifcreatefeaturerequest.h"
#include "qgsoapifdeletefeaturerequest.h"
#include "qgsoapifitemsrequest.h"
#include "qgsoapiflandingpagerequest.h"
#include "qgsoapifoptionsrequest.h"
#include "qgsoapifpatchfeaturerequest.h"
#include "qgsoapifputfeaturerequest.h"
#include "qgsoapifqueryablesrequest.h"
#include "qgsoapifschemarequest.h"
#include "qgsoapifshareddata.h"
#include "qgsoapifsingleitemrequest.h"
#include "qgsoapifutils.h"
#include "qgswfsconstants.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgsxmlschemaanalyzer.h"

#include <QIcon>

#include "moc_qgsoapifprovider.cpp"

const QString QgsOapifProvider::OAPIF_PROVIDER_KEY = u"OAPIF"_s;
const QString QgsOapifProvider::OAPIF_PROVIDER_DESCRIPTION = u"OGC API - Features data provider"_s;

QgsOapifProvider::QgsOapifProvider( const QString &uri, const ProviderOptions &options, Qgis::DataProviderReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags ), mShared( new QgsOapifSharedData( uri ) )
{
  connect( mShared.get(), &QgsOapifSharedData::raiseError, this, &QgsOapifProvider::pushErrorSlot );
  connect( mShared.get(), &QgsOapifSharedData::extentUpdated, this, &QgsOapifProvider::fullExtentCalculated );

  if ( uri.isEmpty() )
  {
    mValid = false;
    return;
  }

  mSubsetString = mShared->mURI.filter();

  if ( !init() )
  {
    mValid = false;
    return;
  }

  QString errorMsg;
  if ( !mShared->computeServerFilter( errorMsg ) )
  {
    QgsMessageLog::logMessage( errorMsg, tr( "OAPIF" ) );
    mValid = false;
    return;
  }
}

QgsOapifProvider::~QgsOapifProvider()
{
}

bool QgsOapifProvider::init()
{
  const bool synchronous = true;
  const bool forceRefresh = false;

  const QString url = QgsDataSourceUri( mShared->mURI.uri() ).param( QgsWFSConstants::URI_PARAM_URL );
  const int pos = url.indexOf( '?' );
  if ( pos >= 0 )
  {
    mShared->mExtraQueryParameters = url.mid( pos + 1 );
  }

  QgsOapifLandingPageRequest landingPageRequest( mShared->mURI.uri() );
  if ( !landingPageRequest.request( synchronous, forceRefresh ) )
    return false;
  if ( landingPageRequest.errorCode() != QgsBaseNetworkRequest::NoError )
    return false;

  QgsOapifApiRequest apiRequest( mShared->mURI.uri(), mShared->appendExtraQueryParameters( landingPageRequest.apiUrl() ) );
  if ( !apiRequest.request( synchronous, forceRefresh ) )
    return false;
  if ( apiRequest.errorCode() != QgsBaseNetworkRequest::NoError )
    return false;

  const auto &collectionProperties = apiRequest.collectionProperties();
  const auto thisCollPropertiesIter = collectionProperties.find( mShared->mURI.typeName() );
  if ( thisCollPropertiesIter != collectionProperties.end() )
  {
    mShared->mSimpleQueryables = thisCollPropertiesIter->mSimpleQueryables;
  }

  mShared->mServerMaxFeatures = apiRequest.maxLimit();

  const bool pagingEnabled = mShared->mURI.pagingStatus() != QgsWFSDataSourceURI::PagingStatus::DISABLED;
  if ( mShared->mURI.maxNumFeatures() > 0 && mShared->mServerMaxFeatures > 0 && !pagingEnabled )
  {
    mShared->mMaxFeatures = std::min( mShared->mURI.maxNumFeatures(), mShared->mServerMaxFeatures );
  }
  else if ( mShared->mURI.maxNumFeatures() > 0 )
  {
    mShared->mMaxFeatures = mShared->mURI.maxNumFeatures();
  }
  else if ( mShared->mServerMaxFeatures > 0 && !pagingEnabled )
  {
    mShared->mMaxFeatures = mShared->mServerMaxFeatures;
  }

  if ( pagingEnabled && mShared->mURI.pageSize() > 0 )
  {
    if ( mShared->mServerMaxFeatures > 0 )
    {
      mShared->mPageSize = std::min( mShared->mURI.pageSize(), mShared->mServerMaxFeatures );
    }
    else
    {
      mShared->mPageSize = mShared->mURI.pageSize();
    }
  }
  else if ( pagingEnabled )
  {
    if ( apiRequest.defaultLimit() > 0 && apiRequest.maxLimit() > 0 )
    {
      // Use the default, but if it is below 1000, aim for 1000
      // but clamp to the maximum limit
      mShared->mPageSize = std::min( std::max( 1000, apiRequest.defaultLimit() ), apiRequest.maxLimit() );
    }
    else if ( apiRequest.defaultLimit() > 0 )
      mShared->mPageSize = std::max( 1000, apiRequest.defaultLimit() );
    else if ( apiRequest.maxLimit() > 0 )
      mShared->mPageSize = apiRequest.maxLimit();
    else
      mShared->mPageSize = 100; // fallback to arbitrary page size
  }

  mShared->mCollectionUrl = landingPageRequest.collectionsUrl() + u"/"_s + mShared->mURI.typeName();
  auto collectionRequest = std::make_unique<QgsOapifCollectionRequest>( mShared->mURI.uri(), mShared->appendExtraQueryParameters( mShared->mCollectionUrl ) );
  if ( !collectionRequest->request( synchronous, forceRefresh ) || collectionRequest->errorCode() != QgsBaseNetworkRequest::NoError )
  {
    // Retry with a trailing slash. Works around a bug with
    // https://geoserveis.ide.cat/servei/catalunya/inspire/ogc/features/collections/inspire:AD.Address not working
    // but https://geoserveis.ide.cat/servei/catalunya/inspire/ogc/features/collections/inspire:AD.Address/ working
    mShared->mCollectionUrl += QLatin1Char( '/' );
    collectionRequest = std::make_unique<QgsOapifCollectionRequest>( mShared->mURI.uri(), mShared->appendExtraQueryParameters( mShared->mCollectionUrl ) );
    if ( !collectionRequest->request( synchronous, forceRefresh ) || collectionRequest->errorCode() != QgsBaseNetworkRequest::NoError )
    {
      return false;
    }
  }

  bool implementsPart2 = false;
  const QString &conformanceUrl = landingPageRequest.conformanceUrl();
  bool implementsSchemas = false;
  if ( !conformanceUrl.isEmpty() )
  {
    QgsOapifConformanceRequest conformanceRequest( mShared->mURI.uri() );
    const QStringList conformanceClasses = conformanceRequest.conformanceClasses( conformanceUrl );
    implementsPart2 = conformanceClasses.contains( "http://www.opengis.net/spec/ogcapi-features-2/1.0/conf/crs"_L1 );

    const bool implementsCql2Text = ( conformanceClasses.contains( "http://www.opengis.net/spec/cql2/0.0/conf/cql2-text"_L1 ) || conformanceClasses.contains( "http://www.opengis.net/spec/cql2/1.0/conf/cql2-text"_L1 ) );
    mShared->mServerSupportsFilterCql2Text = ( conformanceClasses.contains( "http://www.opengis.net/spec/cql2/0.0/conf/basic-cql2"_L1 ) || conformanceClasses.contains( "http://www.opengis.net/spec/cql2/1.0/conf/basic-cql2"_L1 ) ) && ( conformanceClasses.contains( "http://www.opengis.net/spec/ogcapi-features-3/0.0/conf/filter"_L1 ) || conformanceClasses.contains( "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/filter"_L1 ) ) && ( conformanceClasses.contains( "http://www.opengis.net/spec/ogcapi-features-3/0.0/conf/features-filter"_L1 ) || conformanceClasses.contains( "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/features-filter"_L1 ) ) && implementsCql2Text;
    mShared->mServerSupportsLikeBetweenIn = ( conformanceClasses.contains( "http://www.opengis.net/spec/cql2/0.0/conf/advanced-comparison-operators"_L1 ) || conformanceClasses.contains( "http://www.opengis.net/spec/cql2/1.0/conf/advanced-comparison-operators"_L1 ) );
    mShared->mServerSupportsCaseI = ( conformanceClasses.contains( "http://www.opengis.net/spec/cql2/0.0/conf/case-insensitive-comparison"_L1 ) || conformanceClasses.contains( "http://www.opengis.net/spec/cql2/1.0/conf/case-insensitive-comparison"_L1 ) );
    mShared->mServerSupportsBasicSpatialFunctions = ( conformanceClasses.contains( "http://www.opengis.net/spec/cql2/1.0/conf/basic-spatial-functions"_L1 ) ||
                                                      // Two below names are deprecated
                                                      conformanceClasses.contains( "http://www.opengis.net/spec/cql2/0.0/conf/basic-spatial-operators"_L1 ) || conformanceClasses.contains( "http://www.opengis.net/spec/cql2/1.0/conf/basic-spatial-operators"_L1 ) );
    implementsSchemas = conformanceClasses.contains( "http://www.opengis.net/spec/ogcapi-features-5/1.0/conf/schemas"_L1 );
  }

  const QgsOapifCollection &collectionDesc = collectionRequest->collection();

  mLayerMetadata = collectionDesc.mLayerMetadata;
  mFeatureCount = collectionDesc.mFeatureCount;

  QString srsName = mShared->mURI.SRSName();
  if ( implementsPart2 && !srsName.isEmpty() )
  {
    // Use URI SRSName parameter if defined
    mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( srsName );
    if ( mLayerMetadata.crs().isValid() && mShared->mSourceCrs.authid() == mLayerMetadata.crs().authid() )
      mShared->mSourceCrs.setCoordinateEpoch( mLayerMetadata.crs().coordinateEpoch() );
  }
  else if ( implementsPart2 && mLayerMetadata.crs().isValid() )
  {
    // Recreate a CRS object with fromOgcWmsCrs  because its mPj pointer got
    // deleted because it got acquired by a thread that has now disappeared
    // and without it, it is impossible to create a transform
    mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mLayerMetadata.crs().authid() );
    mShared->mSourceCrs.setCoordinateEpoch( mLayerMetadata.crs().coordinateEpoch() );
  }
  else
  {
    mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs(
      OAPIF_PROVIDER_DEFAULT_CRS
    );
  }
  mShared->mCapabilityExtent = collectionDesc.mBbox;

  // Reproject extent of /collection request to the layer CRS
  if ( !mShared->mCapabilityExtent.isNull() && collectionDesc.mBboxCrs != mShared->mSourceCrs )
  {
    // Recreate a CRS object with fromOgcWmsCrs because its mPj pointer got
    // deleted because it got acquired by a thread that has now disappeared
    // and without it, it is impossible to create a transform
    QgsCoordinateReferenceSystem bboxCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( collectionDesc.mBboxCrs.authid() );
    bboxCrs.setCoordinateEpoch( collectionDesc.mBboxCrs.coordinateEpoch() );

    QgsCoordinateTransform ct( bboxCrs, mShared->mSourceCrs, transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    QgsDebugMsgLevel( "before ext:" + mShared->mCapabilityExtent.toString(), 4 );
    try
    {
      mShared->mCapabilityExtent = ct.transformBoundingBox( mShared->mCapabilityExtent );
      QgsDebugMsgLevel( "after ext:" + mShared->mCapabilityExtent.toString(), 4 );
    }
    catch ( const QgsCsException &e )
    {
      QgsMessageLog::logMessage( tr( "Cannot compute layer extent: %1" ).arg( e.what() ), tr( "OAPIF" ) );
      mShared->mCapabilityExtent = QgsRectangle();
    }
  }

  // Merge contact info from /api
  mLayerMetadata.setContacts( apiRequest.metadata().contacts() );

  if ( mShared->mServerSupportsFilterCql2Text )
  {
    const QString queryablesUrl = mShared->mCollectionUrl + u"/queryables"_s;
    QgsOapifQueryablesRequest queryablesRequest( mShared->mURI.uri() );
    mShared->mQueryables = queryablesRequest.queryables( queryablesUrl );
  }

  mShared->mItemsUrl = mShared->mCollectionUrl + u"/items"_s;

  mShared->mFeatureFormat = mShared->mURI.outputFormat();
  if ( !mShared->mFeatureFormat.isEmpty() )
  {
    auto it = collectionDesc.mMapFeatureFormatToUrl.find( mShared->mFeatureFormat );
    if ( it != collectionDesc.mMapFeatureFormatToUrl.end() )
    {
      mShared->mItemsUrl = *it;

      if ( mShared->mFeatureFormat.startsWith( "application/gml+xml"_L1 ) )
      {
        auto it2 = collectionDesc.mMapFeatureFormatToBulkDownloadUrl.find( mShared->mFeatureFormat );
        if ( it2 != collectionDesc.mMapFeatureFormatToBulkDownloadUrl.end() )
        {
          mShared->mBulkDownloadGmlUrl = *it2;
        }
      }
    }
    else
    {
      mShared->mFeatureFormat.clear();
    }
  }

  QString tenFeaturesRequestUrl = mShared->mItemsUrl;
  if ( tenFeaturesRequestUrl.indexOf( QLatin1Char( '?' ) ) < 0 )
    tenFeaturesRequestUrl += QLatin1Char( '?' );
  else
    tenFeaturesRequestUrl += QLatin1Char( '&' );
  tenFeaturesRequestUrl += "limit=10"_L1;
  if ( mShared->mSourceCrs
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
    tenFeaturesRequestUrl += u"&crs=%1"_s.arg( mShared->mSourceCrs.toOgcUri() );

  QgsOapifItemsRequest itemsRequest( mShared->mURI.uri(), mShared->appendExtraQueryParameters( tenFeaturesRequestUrl ), mShared->mFeatureFormat );
  if ( mShared->mCapabilityExtent.isNull() )
  {
    itemsRequest.setComputeBbox();
  }
  if ( !itemsRequest.request( synchronous, forceRefresh ) )
    return false;
  if ( itemsRequest.errorCode() != QgsBaseNetworkRequest::NoError )
    return false;

  if ( itemsRequest.numberMatched() >= 0 )
  {
    mShared->mHasNumberMatched = true;
    if ( mSubsetString.isEmpty() )
    {
      mShared->setFeatureCount( itemsRequest.numberMatched(), true );
    }
  }

  if ( mShared->mCapabilityExtent.isNull() )
  {
    mShared->mCapabilityExtent = itemsRequest.bbox();
    if ( !mShared->mCapabilityExtent.isNull() )
    {
      QgsCoordinateReferenceSystem defaultCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs(
        OAPIF_PROVIDER_DEFAULT_CRS
      );
      if ( defaultCrs != mShared->mSourceCrs )
      {
        QgsCoordinateTransform ct( defaultCrs, mShared->mSourceCrs, transformContext() );
        ct.setBallparkTransformsAreAppropriate( true );
        QgsDebugMsgLevel( "before ext:" + mShared->mCapabilityExtent.toString(), 4 );
        try
        {
          mShared->mCapabilityExtent = ct.transformBoundingBox( mShared->mCapabilityExtent );
          QgsDebugMsgLevel( "after ext:" + mShared->mCapabilityExtent.toString(), 4 );
        }
        catch ( const QgsCsException &e )
        {
          QgsMessageLog::logMessage( tr( "Cannot compute layer extent: %1" ).arg( e.what() ), tr( "OAPIF" ) );
          mShared->mCapabilityExtent = QgsRectangle();
        }
      }
    }
  }

  mShared->mFields = itemsRequest.fields();
  mShared->mGeometryAttribute = itemsRequest.geometryColumnName();
  mShared->mWKBType = itemsRequest.wkbType();
  mShared->mFoundIdTopLevel = itemsRequest.foundIdTopLevel();
  mShared->mFoundIdInProperties = itemsRequest.foundIdInProperties();

  computeCapabilities( itemsRequest );

  bool trySchemasLink = true;

  // If we got a link to an XML schema and that the feature format is GML,
  // try to analyze this XML schema to get the fields
  if ( !collectionDesc.mXmlSchemaUrl.isEmpty() && mShared->mFeatureFormat.startsWith( "application/gml+xml"_L1 ) )
  {
    QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI );
    if ( !describeFeatureType.sendGET( collectionDesc.mXmlSchemaUrl, QString(), true, false ) )
    {
      QgsMessageLog::logMessage( QObject::tr( "Network request to get XML schema failed for url %1: %2" ).arg( collectionDesc.mXmlSchemaUrl, describeFeatureType.errorMessage() ), QObject::tr( "OAPIF" ) );
    }
    else
    {
      QByteArray response = describeFeatureType.response();
      QDomDocument describeFeatureDocument;
      bool geometryMaybeMissing = false;
      bool metadataRetrievalCanceled = false;
      QString errorMsg;
      const bool hasZ = QgsWkbTypes::hasZ( mShared->mWKBType );
      if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
      {
        QgsDebugMsgLevel( response, 4 );
        QgsMessageLog::logMessage( QObject::tr( "Cannot parse XML schema for url %1: %2" ).arg( collectionDesc.mXmlSchemaUrl, errorMsg ), QObject::tr( "OAPIF" ) );
      }
      else if ( !QgsXmlSchemaAnalyzer::readAttributesFromSchema(
                  QObject::tr( "OAPIF" ), mShared.get(), mCapabilities,
                  describeFeatureDocument, response, /* singleLayerContext = */ true,
                  mShared->mURI.typeName(), mShared->mGeometryAttribute,
                  mShared->mFields, mShared->mWKBType, geometryMaybeMissing,
                  errorMsg, metadataRetrievalCanceled
                ) )
      {
        QgsMessageLog::logMessage( QObject::tr( "Analysis of XML schema failed for url %1: %2" ).arg( collectionDesc.mXmlSchemaUrl, errorMsg ), QObject::tr( "OAPIF" ) );
      }
      else
      {
        // GML schema analysis cannot infer if Z is used. So if the sampling
        // of the initial features has detected Z, add it back.
        if ( hasZ )
          mShared->mWKBType = QgsWkbTypes::addZ( mShared->mWKBType );
        trySchemasLink = false;
      }
    }
  }

  if ( implementsSchemas && trySchemasLink )
  {
    QString schemaUrl;
    // Find a link for rel=http://www.opengis.net/def/rel/ogc/1.0/schema (or [ogc-rel:schema])
    // whose mime type is in priority "application/schema+json"
    // Also accept "application/json" as lower priority
    for ( const QgsAbstractMetadataBase::Link &link : mLayerMetadata.links() )
    {
      if ( link.name == "http://www.opengis.net/def/rel/ogc/1.0/schema"_L1 || link.name == "[ogc-rel:schema]"_L1 )
      {
        if ( link.mimeType == "application/schema+json" )
        {
          schemaUrl = link.url;
          break;
        }
        else if ( link.mimeType == "application/json" )
        {
          schemaUrl = link.url;
          // Go on in case there is a later mimeType == "application/schema+json"
        }
      }
    }
    if ( !schemaUrl.isEmpty() )
    {
      handleGetSchemaRequest( schemaUrl );
    }
  }

  return true;
}

void QgsOapifProvider::pushErrorSlot( const QString &errorMsg )
{
  pushError( errorMsg );
}

QgsAbstractFeatureSource *QgsOapifProvider::featureSource() const
{
  return new QgsBackgroundCachedFeatureSource( mShared );
}

QgsFeatureIterator QgsOapifProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsBackgroundCachedFeatureIterator( new QgsBackgroundCachedFeatureSource( mShared ), true, mShared, request ) );
}

Qgis::WkbType QgsOapifProvider::wkbType() const
{
  return mShared->mWKBType;
}

long long QgsOapifProvider::featureCount() const
{
  // If no filter is set try the fast way of retrieving the feature count
  if ( mSubsetString.isEmpty() )
  {
    if ( mShared->mServerFilter.isEmpty() && mFeatureCount >= 0 )
    {
      return mFeatureCount;
    }

    QString url = mShared->mItemsUrl;
    if ( url.indexOf( QLatin1Char( '?' ) ) < 0 )
      url += QLatin1Char( '?' );
    else
      url += QLatin1Char( '&' );
    url += "limit=1"_L1;
    url = mShared->appendExtraQueryParameters( url );

    if ( !mShared->mServerFilter.isEmpty() )
    {
      url += QLatin1Char( '&' );
      url += mShared->mServerFilter;
    }

    QgsOapifItemsRequest itemsRequest( mShared->mURI.uri(), url, mShared->mFeatureFormat );
    if ( !itemsRequest.request( true, false ) )
      return -1;
    if ( itemsRequest.errorCode() != QgsBaseNetworkRequest::NoError )
      return -1;

    const long long featureCount = itemsRequest.numberMatched();
    if ( featureCount >= 0 )
    {
      mShared->setFeatureCount( featureCount, true );
      return featureCount;
    }
  }

  // Retry the slow way by active filter or numberMatched parameter not sent from server
  if ( mUpdateFeatureCountAtNextFeatureCountRequest )
  {
    mUpdateFeatureCountAtNextFeatureCountRequest = false;

    QgsFeature f;
    QgsFeatureRequest request;
    request.setNoAttributes();
    constexpr int MAX_FEATURES = 1000;
    request.setLimit( MAX_FEATURES + 1 );
    auto iter = getFeatures( request );
    long long count = 0;
    bool countExact = true;
    while ( iter.nextFeature( f ) )
    {
      if ( count == MAX_FEATURES ) // to avoid too long processing time
      {
        countExact = false;
        break;
      }
      count++;
    }

    mShared->setFeatureCount( count, countExact );
  }

  return mShared->getFeatureCount();
}

QgsFields QgsOapifProvider::fields() const
{
  return mShared->mFields;
}

QgsCoordinateReferenceSystem QgsOapifProvider::crs() const
{
  return mShared->mSourceCrs;
}

QgsRectangle QgsOapifProvider::extent() const
{
  return mShared->consolidatedExtent();
}

void QgsOapifProvider::reloadProviderData()
{
  mUpdateFeatureCountAtNextFeatureCountRequest = true;
  mShared->invalidateCache();
}

bool QgsOapifProvider::isValid() const
{
  return mValid;
}

void QgsOapifProvider::computeCapabilities( const QgsOapifItemsRequest &itemsRequest )
{
  mCapabilities = Qgis::VectorProviderCapability::SelectAtId | Qgis::VectorProviderCapability::ReadLayerMetadata | Qgis::VectorProviderCapability::ReloadData;

  // Determine edition capabilities: create (POST on /items),
  // update (PUT on /items/some_id) and delete (DELETE on /items/some_id)
  // by issuing a OPTIONS HTTP request.
  QgsDataSourceUri uri( mShared->mURI.uri() );
  QgsOapifOptionsRequest optionsItemsRequest( uri );
  QStringList supportedOptions = optionsItemsRequest.sendOPTIONS( mShared->mItemsUrl );
  if ( supportedOptions.contains( "POST"_L1 ) )
  {
    mCapabilities |= Qgis::VectorProviderCapability::AddFeatures;

    const auto &features = itemsRequest.features();
    QString testId;
    if ( !features.empty() )
    {
      testId = features[0].second;
    }
    else
    {
      // If there is no existing feature, it is not obvious to know if the
      // server supports PUT and DELETE on items. Attempt to request OPTIONS
      // on a fake object...
      testId = u"unknown_id"_s;
    }
    QgsOapifOptionsRequest optionsOneItemRequest( uri );
    QString url( mShared->mItemsUrl );
    url += QLatin1Char( '/' );
    url += testId;
    supportedOptions = optionsOneItemRequest.sendOPTIONS( url );
    if ( supportedOptions.contains( "PUT"_L1 ) )
    {
      mCapabilities |= Qgis::VectorProviderCapability::ChangeAttributeValues;
      mCapabilities |= Qgis::VectorProviderCapability::ChangeGeometries;
    }
    if ( supportedOptions.contains( "DELETE"_L1 ) )
    {
      mCapabilities |= Qgis::VectorProviderCapability::DeleteFeatures;
    }
    if ( supportedOptions.contains( "PATCH"_L1 ) )
    {
      mSupportsPatch = true;
    }
  }
}

Qgis::VectorProviderCapabilities QgsOapifProvider::capabilities() const
{
  return mCapabilities;
}

bool QgsOapifProvider::empty() const
{
  if ( subsetString().isEmpty() && mShared->isFeatureCountExact() )
  {
    return mShared->getFeatureCount( false ) == 0;
  }

  QgsFeature f;
  QgsFeatureRequest request;
  request.setNoAttributes();
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );

  // Whoops, the provider returns an empty iterator when we are using
  // a setLimit call in combination with a subsetString.
  // Remove this method (and default to the QgsVectorDataProvider one)
  // once this is fixed
#if 0
  request.setLimit( 1 );
#endif
  return !getFeatures( request ).nextFeature( f );
};

QString QgsOapifProvider::geometryColumnName() const { return mShared->mGeometryAttribute; }

bool QgsOapifProvider::setSubsetString( const QString &filter, bool updateFeatureCount )
{
  QgsDebugMsgLevel( u"filter = '%1'"_s.arg( filter ), 4 );

  if ( filter == mSubsetString )
    return true;

  if ( !filter.isEmpty() )
  {
    const QgsExpression filterExpression( filter );
    if ( !filterExpression.isValid() )
    {
      QgsMessageLog::logMessage( filterExpression.parserErrorString(), tr( "OAPIF" ) );
      return false;
    }
  }

  disconnect( mShared.get(), &QgsOapifSharedData::raiseError, this, &QgsOapifProvider::pushErrorSlot );
  disconnect( mShared.get(), &QgsOapifSharedData::extentUpdated, this, &QgsOapifProvider::fullExtentCalculated );

  // We must not change the subset string of the shared data used in another iterator/data provider ...
  mShared.reset( mShared->clone() );

  connect( mShared.get(), &QgsOapifSharedData::raiseError, this, &QgsOapifProvider::pushErrorSlot );
  connect( mShared.get(), &QgsOapifSharedData::extentUpdated, this, &QgsOapifProvider::fullExtentCalculated );

  mSubsetString = filter;
  clearMinMaxCache();

  // update URI
  mShared->mURI.setFilter( filter );
  setDataSourceUri( mShared->mURI.uri() );
  QString errorMsg;
  if ( !mShared->computeServerFilter( errorMsg ) )
    QgsMessageLog::logMessage( errorMsg, tr( "OAPIF" ) );


  if ( updateFeatureCount )
  {
    reloadData();
  }
  else
  {
    mShared->invalidateCache();
    emit dataChanged();
  }

  return true;
}

QString QgsOapifProvider::subsetStringDialect() const
{
  return tr( "OGC API - Features filter" );
}

QString QgsOapifProvider::subsetStringHelpUrl() const
{
  return u"https://portal.ogc.org/files/96288#cql-core"_s;
}

bool QgsOapifProvider::supportsSubsetString() const
{
  return true;
}

QgsOapifFilterTranslationState QgsOapifProvider::filterTranslatedState() const
{
  return mShared->mFilterTranslationState;
}

const QString &QgsOapifProvider::clientSideFilterExpression() const
{
  return mShared->mClientSideFilterExpression;
}

void QgsOapifProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  mShared = qobject_cast<QgsOapifProvider *>( source )->mShared;
}

void QgsOapifProvider::handleGetSchemaRequest( const QString &schemaUrl )
{
  QgsOapifSchemaRequest schemaRequest( mShared->mURI.uri() );
  const QgsOapifSchemaRequest::Schema schema = schemaRequest.schema( schemaUrl );
  if ( schemaRequest.errorCode() == QgsBaseNetworkRequest::NoError )
  {
    mShared->mFields = schema.mFields;
    mShared->mGeometryAttribute = schema.mGeometryColumnName;
    if ( schema.mWKBType != Qgis::WkbType::Unknown )
      mShared->mWKBType = schema.mWKBType;
  }
}

bool QgsOapifProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  QgsDataSourceUri uri( mShared->mURI.uri() );
  QStringList jsonIds;
  QString contentCrs;
  if ( mShared->mSourceCrs
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
  {
    contentCrs = mShared->mSourceCrs.toOgcUri();
  }
  const bool hasAxisInverted = mShared->mSourceCrs.hasAxisInverted();
  const int idFieldIdx = mShared->mFields.indexOf( "id" );
  for ( QgsFeature &f : flist )
  {
    QgsOapifCreateFeatureRequest req( uri );
    const QString id = req.createFeature( mShared.get(), f, contentCrs, hasAxisInverted );
    if ( id.isEmpty() )
    {
      pushError( tr( "Feature creation failed: %1" ).arg( req.errorMessage() ) );
      return false;
    }
    jsonIds.append( id );

    // If there's no feature["properties"]["id"] field in the JSON returned by the
    // /items request, but there's a "id" field, it means that feature["id"]
    // is non-numeric. Thus set the one returned by the createFeature() request
    if ( !( flags & QgsFeatureSink::FastInsert ) && !mShared->mFoundIdInProperties && idFieldIdx >= 0 )
    {
      f.setAttribute( idFieldIdx, id );
    }

    // Refresh the feature content with its content from the server with a
    // /items/{id} request.
    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      QgsOapifSingleItemRequest itemRequest( mShared->mURI.uri(), mShared->appendExtraQueryParameters( mShared->mItemsUrl + QString( u"/"_s + id ) ) );
      if ( itemRequest.request( /*synchronous=*/true, /*forceRefresh=*/true ) && itemRequest.errorCode() == QgsBaseNetworkRequest::NoError )
      {
        const QgsFeature &updatedFeature = itemRequest.feature();
        if ( updatedFeature.isValid() )
        {
          int updatedFieldIdx = 0;
          for ( const QgsField &updatedField : itemRequest.fields() )
          {
            const int srcFieldIdx = mShared->mFields.indexOf( updatedField.name() );
            if ( srcFieldIdx >= 0 )
            {
              f.setAttribute( srcFieldIdx, updatedFeature.attribute( updatedFieldIdx ) );
            }
            updatedFieldIdx++;
          }
        }
      }
    }
  }

  QStringList::const_iterator idIt = jsonIds.constBegin();
  QgsFeatureList::iterator featureIt = flist.begin();

  QVector<QgsFeatureUniqueIdPair> serializedFeatureList;
  for ( ; idIt != jsonIds.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
  {
    serializedFeatureList.push_back( QgsFeatureUniqueIdPair( *featureIt, *idIt ) );
  }
  mShared->serializeFeatures( serializedFeatureList );

  if ( !( flags & QgsFeatureSink::FastInsert ) )
  {
    // And now set the feature id from the one got from the database
    QMap<QString, QgsFeatureId> map;
    for ( int idx = 0; idx < serializedFeatureList.size(); idx++ )
      map[serializedFeatureList[idx].second] = serializedFeatureList[idx].first.id();

    idIt = jsonIds.constBegin();
    featureIt = flist.begin();
    for ( ; idIt != jsonIds.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
    {
      if ( map.find( *idIt ) != map.end() )
        featureIt->setId( map[*idIt] );
    }
  }

  return true;
}

bool QgsOapifProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  QgsDataSourceUri uri( mShared->mURI.uri() );
  QString contentCrs;
  if ( mShared->mSourceCrs
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
  {
    contentCrs = mShared->mSourceCrs.toOgcUri();
  }
  const bool hasAxisInverted = mShared->mSourceCrs.hasAxisInverted();
  QgsGeometryMap::const_iterator geomIt = geometry_map.constBegin();
  for ( ; geomIt != geometry_map.constEnd(); ++geomIt )
  {
    const QgsFeatureId qgisFid = geomIt.key();
    //find out feature id
    QString jsonId = mShared->findUniqueId( qgisFid );
    if ( jsonId.isEmpty() )
    {
      pushError( u"Cannot identify feature of id %1"_s.arg( qgisFid ) );
      return false;
    }

    if ( mSupportsPatch )
    {
      // Push to server
      QgsOapifPatchFeatureRequest req( uri );
      if ( !req.patchFeature( mShared.get(), jsonId, geomIt.value(), contentCrs, hasAxisInverted ) )
      {
        pushError( u"Cannot modify feature of id %1"_s.arg( qgisFid ) );
        return false;
      }
    }
    else
    {
      // Fetch existing feature
      QgsFeatureRequest request;
      request.setFilterFid( qgisFid );
      QgsFeatureIterator featureIterator = getFeatures( request );
      QgsFeature f;
      if ( !featureIterator.nextFeature( f ) )
      {
        pushError( u"Cannot retrieve feature of id %1"_s.arg( qgisFid ) );
        return false;
      }

      // Patch it with new geometry
      f.setGeometry( geomIt.value() );

      // Push to server
      QgsOapifPutFeatureRequest req( uri );
      if ( !req.putFeature( mShared.get(), jsonId, f, contentCrs, hasAxisInverted ) )
      {
        pushError( u"Cannot modify feature of id %1"_s.arg( qgisFid ) );
        return false;
      }
    }
  }

  mShared->changeGeometryValues( geometry_map );
  return true;
}

bool QgsOapifProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  QgsDataSourceUri uri( mShared->mURI.uri() );
  QString contentCrs;
  if ( mShared->mSourceCrs
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
  {
    contentCrs = mShared->mSourceCrs.toOgcUri();
  }
  const bool hasAxisInverted = mShared->mSourceCrs.hasAxisInverted();
  QgsChangedAttributesMap::const_iterator attIt = attr_map.constBegin();
  for ( ; attIt != attr_map.constEnd(); ++attIt )
  {
    const QgsFeatureId qgisFid = attIt.key();
    //find out feature id
    QString jsonId = mShared->findUniqueId( qgisFid );
    if ( jsonId.isEmpty() )
    {
      pushError( u"Cannot identify feature of id %1"_s.arg( qgisFid ) );
      return false;
    }

    if ( mSupportsPatch )
    {
      // Push to server
      QgsOapifPatchFeatureRequest req( uri );
      if ( !req.patchFeature( mShared.get(), jsonId, attIt.value() ) )
      {
        pushError( u"Cannot modify feature of id %1"_s.arg( qgisFid ) );
        return false;
      }
    }
    else
    {
      // Fetch existing feature
      QgsFeatureRequest request;
      request.setFilterFid( qgisFid );
      QgsFeatureIterator featureIterator = getFeatures( request );
      QgsFeature f;
      if ( !featureIterator.nextFeature( f ) )
      {
        pushError( u"Cannot retrieve feature of id %1"_s.arg( qgisFid ) );
        return false;
      }

      // Patch it with new attribute values
      QgsAttributeMap::const_iterator attMapIt = attIt.value().constBegin();
      for ( ; attMapIt != attIt.value().constEnd(); ++attMapIt )
      {
        f.setAttribute( attMapIt.key(), attMapIt.value() );
      }

      // Push to server
      QgsOapifPutFeatureRequest req( uri );
      if ( !req.putFeature( mShared.get(), jsonId, f, contentCrs, hasAxisInverted ) )
      {
        pushError( u"Cannot modify feature of id %1"_s.arg( qgisFid ) );
        return false;
      }
    }
  }

  mShared->changeAttributeValues( attr_map );
  return true;
}

bool QgsOapifProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  if ( ids.isEmpty() )
  {
    return true;
  }

  QgsDataSourceUri uri( mShared->mURI.uri() );
  for ( const QgsFeatureId &id : ids )
  {
    //find out feature id
    QString jsonId = mShared->findUniqueId( id );
    if ( jsonId.isEmpty() )
    {
      pushError( u"Cannot identify feature of id %1"_s.arg( id ) );
      return false;
    }

    QgsOapifDeleteFeatureRequest req( uri );
    QUrl url( mShared->mItemsUrl + QString( u"/"_s + jsonId ) );
    if ( !req.sendDELETE( url ) )
    {
      pushError( tr( "Feature deletion failed: %1" ).arg( req.errorMessage() ) );
      return false;
    }
  }

  mShared->deleteFeatures( ids );
  return true;
}

QString QgsOapifProvider::name() const
{
  return OAPIF_PROVIDER_KEY;
}

QString QgsOapifProvider::providerKey()
{
  return OAPIF_PROVIDER_KEY;
}

QString QgsOapifProvider::description() const
{
  return OAPIF_PROVIDER_DESCRIPTION;
}

// ---------------------------------

QgsOapifProvider *QgsOapifProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsOapifProvider( uri, options, flags );
}

QList<Qgis::LayerType> QgsOapifProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}

QgsOapifProviderMetadata::QgsOapifProviderMetadata()
  : QgsProviderMetadata( QgsOapifProvider::OAPIF_PROVIDER_KEY, QgsOapifProvider::OAPIF_PROVIDER_DESCRIPTION ) {}

QIcon QgsOapifProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconWfs.svg"_s );
}
