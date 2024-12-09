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

#include "qgsexpressionnodeimpl.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsoapifprovider.h"
#include "moc_qgsoapifprovider.cpp"
#include "qgsoapiflandingpagerequest.h"
#include "qgsoapifapirequest.h"
#include "qgsoapifcollection.h"
#include "qgsoapifconformancerequest.h"
#include "qgsoapifcreatefeaturerequest.h"
#include "qgsoapifcql2textexpressioncompiler.h"
#include "qgsoapifdeletefeaturerequest.h"
#include "qgsoapifpatchfeaturerequest.h"
#include "qgsoapifputfeaturerequest.h"
#include "qgsoapifitemsrequest.h"
#include "qgsoapifoptionsrequest.h"
#include "qgsoapifqueryablesrequest.h"
#include "qgsoapifsingleitemrequest.h"
#include "qgswfsconstants.h"
#include "qgswfsutils.h" // for isCompatibleType()

#include <algorithm>
#include <QIcon>
#include <QUrlQuery>

const QString QgsOapifProvider::OAPIF_PROVIDER_KEY = QStringLiteral( "OAPIF" );
const QString QgsOapifProvider::OAPIF_PROVIDER_DESCRIPTION = QStringLiteral( "OGC API - Features data provider" );

const QString QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS = QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" );

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

  mShared->mCollectionUrl = landingPageRequest.collectionsUrl() + QStringLiteral( "/" ) + mShared->mURI.typeName();
  std::unique_ptr<QgsOapifCollectionRequest> collectionRequest = std::make_unique<QgsOapifCollectionRequest>( mShared->mURI.uri(), mShared->appendExtraQueryParameters( mShared->mCollectionUrl ) );
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
  if ( !conformanceUrl.isEmpty() )
  {
    QgsOapifConformanceRequest conformanceRequest( mShared->mURI.uri() );
    const QStringList conformanceClasses = conformanceRequest.conformanceClasses( conformanceUrl );
    implementsPart2 = conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/ogcapi-features-2/1.0/conf/crs" ) );

    const bool implementsCql2Text = ( conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/0.0/conf/cql2-text" ) ) || conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/1.0/conf/cql2-text" ) ) );
    mShared->mServerSupportsFilterCql2Text = ( conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/0.0/conf/basic-cql2" ) ) || conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/1.0/conf/basic-cql2" ) ) ) && ( conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/ogcapi-features-3/0.0/conf/filter" ) ) || conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/filter" ) ) ) && ( conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/ogcapi-features-3/0.0/conf/features-filter" ) ) || conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/features-filter" ) ) ) && implementsCql2Text;
    mShared->mServerSupportsLikeBetweenIn = ( conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/0.0/conf/advanced-comparison-operators" ) ) || conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/1.0/conf/advanced-comparison-operators" ) ) );
    mShared->mServerSupportsCaseI = ( conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/0.0/conf/case-insensitive-comparison" ) ) || conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/1.0/conf/case-insensitive-comparison" ) ) );
    mShared->mServerSupportsBasicSpatialOperators = ( conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/0.0/conf/basic-spatial-operators" ) ) || conformanceClasses.contains( QLatin1String( "http://www.opengis.net/spec/cql2/1.0/conf/basic-spatial-operators" ) ) );
  }

  mLayerMetadata = collectionRequest->collection().mLayerMetadata;

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
    // WORKAROUND: Recreate a CRS object with fromOgcWmsCrs because when copying the
    // CRS his mPj pointer gets deleted and it is impossible to create a transform
    mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mLayerMetadata.crs().authid() );
    mShared->mSourceCrs.setCoordinateEpoch( mLayerMetadata.crs().coordinateEpoch() );
  }
  else
  {
    mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs(
      QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS
    );
  }
  mShared->mCapabilityExtent = collectionRequest->collection().mBbox;

  // Reproject extent of /collection request to the layer CRS
  if ( !mShared->mCapabilityExtent.isNull() && collectionRequest->collection().mBboxCrs != mShared->mSourceCrs )
  {
    QgsCoordinateTransform ct( collectionRequest->collection().mBboxCrs, mShared->mSourceCrs, transformContext() );
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
    const QString queryablesUrl = mShared->mCollectionUrl + QStringLiteral( "/queryables" );
    QgsOapifQueryablesRequest queryablesRequest( mShared->mURI.uri() );
    mShared->mQueryables = queryablesRequest.queryables( queryablesUrl );
  }

  mShared->mItemsUrl = mShared->mCollectionUrl + QStringLiteral( "/items" );

  QgsOapifItemsRequest itemsRequest( mShared->mURI.uri(), mShared->appendExtraQueryParameters( mShared->mItemsUrl + QStringLiteral( "?limit=10" ) ) );
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
        QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS
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
  mShared->mWKBType = itemsRequest.wkbType();
  mShared->mFoundIdTopLevel = itemsRequest.foundIdTopLevel();
  mShared->mFoundIdInProperties = itemsRequest.foundIdInProperties();

  computeCapabilities( itemsRequest );

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
    QString url = mShared->mItemsUrl;
    url += QLatin1String( "?limit=1" );
    url = mShared->appendExtraQueryParameters( url );

    if ( !mShared->mServerFilter.isEmpty() )
    {
      url += QLatin1Char( '&' );
      url += mShared->mServerFilter;
    }

    QgsOapifItemsRequest itemsRequest( mShared->mURI.uri(), url );
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
  if ( supportedOptions.contains( QLatin1String( "POST" ) ) )
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
      testId = QStringLiteral( "unknown_id" );
    }
    QgsOapifOptionsRequest optionsOneItemRequest( uri );
    QString url( mShared->mItemsUrl );
    url += QLatin1Char( '/' );
    url += testId;
    supportedOptions = optionsOneItemRequest.sendOPTIONS( url );
    if ( supportedOptions.contains( QLatin1String( "PUT" ) ) )
    {
      mCapabilities |= Qgis::VectorProviderCapability::ChangeAttributeValues;
      mCapabilities |= Qgis::VectorProviderCapability::ChangeGeometries;
    }
    if ( supportedOptions.contains( QLatin1String( "DELETE" ) ) )
    {
      mCapabilities |= Qgis::VectorProviderCapability::DeleteFeatures;
    }
    if ( supportedOptions.contains( QLatin1String( "PATCH" ) ) )
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

bool QgsOapifProvider::setSubsetString( const QString &filter, bool updateFeatureCount )
{
  QgsDebugMsgLevel( QStringLiteral( "filter = '%1'" ).arg( filter ), 4 );

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
  return QStringLiteral( "https://portal.ogc.org/files/96288#cql-core" );
}

bool QgsOapifProvider::supportsSubsetString() const
{
  return true;
}

QgsOapifProvider::FilterTranslationState QgsOapifProvider::filterTranslatedState() const
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

bool QgsOapifProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  QgsDataSourceUri uri( mShared->mURI.uri() );
  QStringList jsonIds;
  QString contentCrs;
  if ( mShared->mSourceCrs
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS ) )
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
      QgsOapifSingleItemRequest itemRequest( mShared->mURI.uri(), mShared->appendExtraQueryParameters( mShared->mItemsUrl + QString( QStringLiteral( "/" ) + id ) ) );
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
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS ) )
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
      pushError( QStringLiteral( "Cannot identify feature of id %1" ).arg( qgisFid ) );
      return false;
    }

    if ( mSupportsPatch )
    {
      // Push to server
      QgsOapifPatchFeatureRequest req( uri );
      if ( !req.patchFeature( mShared.get(), jsonId, geomIt.value(), contentCrs, hasAxisInverted ) )
      {
        pushError( QStringLiteral( "Cannot modify feature of id %1" ).arg( qgisFid ) );
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
        pushError( QStringLiteral( "Cannot retrieve feature of id %1" ).arg( qgisFid ) );
        return false;
      }

      // Patch it with new geometry
      f.setGeometry( geomIt.value() );

      // Push to server
      QgsOapifPutFeatureRequest req( uri );
      if ( !req.putFeature( mShared.get(), jsonId, f, contentCrs, hasAxisInverted ) )
      {
        pushError( QStringLiteral( "Cannot modify feature of id %1" ).arg( qgisFid ) );
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
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS ) )
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
      pushError( QStringLiteral( "Cannot identify feature of id %1" ).arg( qgisFid ) );
      return false;
    }

    if ( mSupportsPatch )
    {
      // Push to server
      QgsOapifPatchFeatureRequest req( uri );
      if ( !req.patchFeature( mShared.get(), jsonId, attIt.value() ) )
      {
        pushError( QStringLiteral( "Cannot modify feature of id %1" ).arg( qgisFid ) );
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
        pushError( QStringLiteral( "Cannot retrieve feature of id %1" ).arg( qgisFid ) );
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
        pushError( QStringLiteral( "Cannot modify feature of id %1" ).arg( qgisFid ) );
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
      pushError( QStringLiteral( "Cannot identify feature of id %1" ).arg( id ) );
      return false;
    }

    QgsOapifDeleteFeatureRequest req( uri );
    QUrl url( mShared->mItemsUrl + QString( QStringLiteral( "/" ) + jsonId ) );
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

QgsOapifSharedData::QgsOapifSharedData( const QString &uri )
  : QgsBackgroundCachedSharedData( "oapif", tr( "OAPIF" ) )
  , mURI( uri )
{
  mHideProgressDialog = mURI.hideDownloadProgressDialog();
}

QgsOapifSharedData::~QgsOapifSharedData()
{
  QgsDebugMsgLevel( QStringLiteral( "~QgsOapifSharedData()" ), 4 );

  cleanup();
}

QString QgsOapifSharedData::appendExtraQueryParameters( const QString &url ) const
{
  if ( mExtraQueryParameters.isEmpty() || url.indexOf( mExtraQueryParameters ) > 0 )
    return url;
  const int nPos = url.indexOf( '?' );
  if ( nPos < 0 )
    return url + '?' + mExtraQueryParameters;
  return url + '&' + mExtraQueryParameters;
}

bool QgsOapifSharedData::isRestrictedToRequestBBOX() const
{
  return mURI.isRestrictedToRequestBBOX();
}


std::unique_ptr<QgsFeatureDownloaderImpl> QgsOapifSharedData::newFeatureDownloaderImpl( QgsFeatureDownloader *downloader, bool requestMadeFromMainThread )
{
  return std::unique_ptr<QgsFeatureDownloaderImpl>( new QgsOapifFeatureDownloaderImpl( this, downloader, requestMadeFromMainThread ) );
}


void QgsOapifSharedData::invalidateCacheBaseUnderLock()
{
}

QgsOapifSharedData *QgsOapifSharedData::clone() const
{
  QgsOapifSharedData *copy = new QgsOapifSharedData( mURI.uri( true ) );
  copy->mWKBType = mWKBType;
  copy->mPageSize = mPageSize;
  copy->mExtraQueryParameters = mExtraQueryParameters;
  copy->mCollectionUrl = mCollectionUrl;
  copy->mItemsUrl = mItemsUrl;
  copy->mServerFilter = mServerFilter;
  copy->mFoundIdTopLevel = mFoundIdTopLevel;
  copy->mFoundIdInProperties = mFoundIdInProperties;
  copy->mSimpleQueryables = mSimpleQueryables;
  copy->mServerSupportsFilterCql2Text = mServerSupportsFilterCql2Text;
  copy->mServerSupportsLikeBetweenIn = mServerSupportsLikeBetweenIn;
  copy->mServerSupportsCaseI = mServerSupportsCaseI;
  copy->mServerSupportsBasicSpatialOperators = mServerSupportsBasicSpatialOperators;
  copy->mQueryables = mQueryables;
  QgsBackgroundCachedSharedData::copyStateToClone( copy );

  return copy;
}

static QDateTime getDateTimeValue( const QVariant &v )
{
  if ( v.userType() == QMetaType::Type::QString )
    return QDateTime::fromString( v.toString(), Qt::ISODateWithMs );
  else if ( v.userType() == QMetaType::Type::QDateTime )
    return v.toDateTime();
  return QDateTime();
}

static bool isDateTime( const QVariant &v )
{
  return getDateTimeValue( v ).isValid();
}

static QString getDateTimeValueAsString( const QVariant &v )
{
  if ( v.userType() == QMetaType::Type::QString )
    return v.toString();
  else if ( v.userType() == QMetaType::Type::QDateTime )
    return v.toDateTime().toOffsetFromUtc( 0 ).toString( Qt::ISODateWithMs );
  return QString();
}

static bool isDateTimeField( const QgsFields &fields, const QString &fieldName )
{
  const int idx = fields.indexOf( fieldName );
  if ( idx >= 0 )
  {
    const auto type = fields.at( idx ).type();
    return type == QMetaType::Type::QDateTime || type == QMetaType::Type::QDate;
  }
  return false;
}

static QString getEncodedQueryParam( const QString &key, const QString &value )
{
  QUrlQuery query;
  query.addQueryItem( key, value );
  return query.toString( QUrl::FullyEncoded );
}

static void collectTopLevelAndNodes( const QgsExpressionNode *node, std::vector<const QgsExpressionNode *> &topAndNodes )
{
  if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator )
  {
    const auto binNode = static_cast<const QgsExpressionNodeBinaryOperator *>( node );
    const auto op = binNode->op();
    if ( op == QgsExpressionNodeBinaryOperator::boAnd )
    {
      collectTopLevelAndNodes( binNode->opLeft(), topAndNodes );
      collectTopLevelAndNodes( binNode->opRight(), topAndNodes );
      return;
    }
  }
  topAndNodes.push_back( node );
}

QString QgsOapifSharedData::compileExpressionNodeUsingPart1(
  const QgsExpressionNode *rootNode,
  QgsOapifProvider::FilterTranslationState &translationState,
  QString &untranslatedPart
) const
{
  std::vector<const QgsExpressionNode *> topAndNodes;
  collectTopLevelAndNodes( rootNode, topAndNodes );
  QDateTime minDate;
  QDateTime maxDate;
  QString minDateStr;
  QString maxDateStr;
  QStringList equalityComparisons;
  bool hasTranslatedParts = false;
  for ( size_t i = 0; i < topAndNodes.size(); /* do not increment here */ )
  {
    bool removeMe = false;
    const auto node = topAndNodes[i];
    if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator )
    {
      const auto binNode = static_cast<const QgsExpressionNodeBinaryOperator *>( node );
      const auto op = binNode->op();
      if ( binNode->opLeft()->nodeType() == QgsExpressionNode::ntColumnRef && binNode->opRight()->nodeType() == QgsExpressionNode::ntLiteral )
      {
        const auto left = static_cast<const QgsExpressionNodeColumnRef *>( binNode->opLeft() );
        const auto right = static_cast<const QgsExpressionNodeLiteral *>( binNode->opRight() );
        if ( isDateTimeField( mFields, left->name() ) && isDateTime( right->value() ) )
        {
          if ( op == QgsExpressionNodeBinaryOperator::boGE || op == QgsExpressionNodeBinaryOperator::boGT || op == QgsExpressionNodeBinaryOperator::boEQ )
          {
            removeMe = true;
            if ( !minDate.isValid() || getDateTimeValue( right->value() ) > minDate )
            {
              minDate = getDateTimeValue( right->value() );
              minDateStr = getDateTimeValueAsString( right->value() );
            }
          }
          if ( op == QgsExpressionNodeBinaryOperator::boLE || op == QgsExpressionNodeBinaryOperator::boLT || op == QgsExpressionNodeBinaryOperator::boEQ )
          {
            removeMe = true;
            if ( !maxDate.isValid() || getDateTimeValue( right->value() ) < maxDate )
            {
              maxDate = getDateTimeValue( right->value() );
              maxDateStr = getDateTimeValueAsString( right->value() );
            }
          }
        }
        else if ( op == QgsExpressionNodeBinaryOperator::boEQ && mFields.indexOf( left->name() ) >= 0 )
        {
          // Filtering based on Part 1 /rec/core/fc-filters recommendation.
          const auto iter = mSimpleQueryables.find( left->name() );
          if ( iter != mSimpleQueryables.end() )
          {
            if ( iter->mType == QLatin1String( "string" ) && right->value().userType() == QMetaType::Type::QString )
            {
              equalityComparisons << getEncodedQueryParam( left->name(), right->value().toString() );
              removeMe = true;
            }
            else if ( ( iter->mType == QLatin1String( "integer" ) || iter->mType == QLatin1String( "number" ) ) && right->value().userType() == QMetaType::Type::Int )
            {
              equalityComparisons << getEncodedQueryParam( left->name(), QString::number( right->value().toInt() ) );
              removeMe = true;
            }
            else if ( iter->mType == QLatin1String( "number" ) && right->value().userType() == QMetaType::Type::Double )
            {
              equalityComparisons << getEncodedQueryParam( left->name(), QString::number( right->value().toDouble() ) );
              removeMe = true;
            }
            else if ( iter->mType == QLatin1String( "boolean" ) && right->value().userType() == QMetaType::Type::Bool )
            {
              equalityComparisons << getEncodedQueryParam( left->name(), right->value().toBool() ? QLatin1String( "true" ) : QLatin1String( "false" ) );
              removeMe = true;
            }
          }
        }
      }
    }
    if ( removeMe )
    {
      hasTranslatedParts = true;
      topAndNodes.erase( topAndNodes.begin() + i );
    }
    else
      ++i;
  }

  QString ret;
  if ( minDate.isValid() && maxDate.isValid() )
  {
    if ( minDate == maxDate )
    {
      ret = QStringLiteral( "datetime=" ) + minDateStr;
    }
    else
    {
      ret = QStringLiteral( "datetime=" ) + minDateStr + QStringLiteral( "%2F" ) + maxDateStr;
    }
  }
  else if ( minDate.isValid() )
  {
    // TODO: use ellipsis '..' instead of dummy upper bound once more servers are compliant
    ret = QStringLiteral( "datetime=" ) + minDateStr + QStringLiteral( "%2F9999-12-31T00:00:00Z" );
  }
  else if ( maxDate.isValid() )
  {
    // TODO: use ellipsis '..' instead of dummy upper bound once more servers are compliant
    ret = QStringLiteral( "datetime=0000-01-01T00:00:00Z%2F" ) + maxDateStr;
  }

  for ( const QString &equalityComparison : equalityComparisons )
  {
    if ( !ret.isEmpty() )
      ret += QLatin1Char( '&' );
    ret += equalityComparison;
  }

  if ( !hasTranslatedParts )
  {
    untranslatedPart = rootNode->dump();
    translationState = QgsOapifProvider::FilterTranslationState::FULLY_CLIENT;
  }
  else if ( topAndNodes.empty() )
  {
    untranslatedPart.clear();
    translationState = QgsOapifProvider::FilterTranslationState::FULLY_SERVER;
  }
  else
  {
    translationState = QgsOapifProvider::FilterTranslationState::PARTIAL;

    // Collect part(s) of the filter to be evaluated on client-side
    if ( topAndNodes.size() == 1 )
    {
      untranslatedPart = topAndNodes[0]->dump();
    }
    else
    {
      for ( size_t i = 0; i < topAndNodes.size(); ++i )
      {
        if ( i == 0 )
          untranslatedPart = QStringLiteral( "(" );
        else
          untranslatedPart += QLatin1String( " AND (" );
        untranslatedPart += topAndNodes[i]->dump();
        untranslatedPart += QLatin1Char( ')' );
      }
    }
  }

  return ret;
}

bool QgsOapifSharedData::computeFilter( const QgsExpression &expr, QgsOapifProvider::FilterTranslationState &translationState, QString &serverSideParameters, QString &clientSideFilterExpression ) const
{
  const auto rootNode = expr.rootNode();
  if ( !rootNode )
    return false;

  if ( mServerSupportsFilterCql2Text )
  {
    const bool invertAxisOrientation = mSourceCrs.hasAxisInverted();
    QgsOapifCql2TextExpressionCompiler compiler(
      mQueryables, mServerSupportsLikeBetweenIn, mServerSupportsCaseI,
      mServerSupportsBasicSpatialOperators, invertAxisOrientation
    );
    QgsOapifCql2TextExpressionCompiler::Result res = compiler.compile( &expr );
    if ( res == QgsOapifCql2TextExpressionCompiler::Fail )
    {
      clientSideFilterExpression = expr.rootNode()->dump();
      translationState = QgsOapifProvider::FilterTranslationState::FULLY_CLIENT;
      return true;
    }
    serverSideParameters = getEncodedQueryParam( QStringLiteral( "filter" ), compiler.result() );
    serverSideParameters += QLatin1String( "&filter-lang=cql2-text" );
    if ( compiler.geometryLiteralUsed() )
    {
      if ( mSourceCrs
           != QgsCoordinateReferenceSystem::fromOgcWmsCrs( QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS ) )
      {
        serverSideParameters += QStringLiteral( "&filter-crs=%1" ).arg( mSourceCrs.toOgcUri() );
      }
    }

    clientSideFilterExpression.clear();
    if ( res == QgsOapifCql2TextExpressionCompiler::Partial )
    {
      translationState = QgsOapifProvider::FilterTranslationState::PARTIAL;
    }
    else
    {
      translationState = QgsOapifProvider::FilterTranslationState::FULLY_SERVER;
    }
    return true;
  }

  serverSideParameters = compileExpressionNodeUsingPart1( rootNode, translationState, clientSideFilterExpression );
  return true;
}

bool QgsOapifSharedData::computeServerFilter( QString &errorMsg )
{
  errorMsg.clear();
  mClientSideFilterExpression = mURI.filter();
  mServerFilter.clear();
  if ( mClientSideFilterExpression.isEmpty() )
  {
    mFilterTranslationState = QgsOapifProvider::FilterTranslationState::FULLY_SERVER;
    return true;
  }

  const QgsExpression expr( mClientSideFilterExpression );
  bool ret = computeFilter( expr, mFilterTranslationState, mServerFilter, mClientSideFilterExpression );
  if ( ret )
  {
    if ( mFilterTranslationState == QgsOapifProvider::FilterTranslationState::PARTIAL )
    {
      QgsDebugMsgLevel( QStringLiteral( "Part of the filter will be evaluated on client-side: %1" ).arg( mClientSideFilterExpression ), 2 );
    }
    else if ( mFilterTranslationState == QgsOapifProvider::FilterTranslationState::FULLY_CLIENT )
    {
      QgsDebugMsgLevel( QStringLiteral( "Whole filter will be evaluated on client-side" ), 2 );
    }
  }
  return ret;
}

QString QgsOapifSharedData::computedExpression( const QgsExpression &expression ) const
{
  if ( !expression.isValid() )
    return QString();
  QgsOapifProvider::FilterTranslationState translationState;
  QString serverParameters;
  QString clientSideFilterExpression;
  computeFilter( expression, translationState, serverParameters, clientSideFilterExpression );
  return serverParameters;
}

void QgsOapifSharedData::pushError( const QString &errorMsg ) const
{
  QgsMessageLog::logMessage( errorMsg, tr( "OAPIF" ) );
  emit raiseError( errorMsg );
}

// ---------------------------------

QgsOapifFeatureDownloaderImpl::QgsOapifFeatureDownloaderImpl( QgsOapifSharedData *shared, QgsFeatureDownloader *downloader, bool requestMadeFromMainThread )
  : QgsFeatureDownloaderImpl( shared, downloader ), mShared( shared )
{
  QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS( requestMadeFromMainThread );
}

QgsOapifFeatureDownloaderImpl::~QgsOapifFeatureDownloaderImpl()
{
}

void QgsOapifFeatureDownloaderImpl::createProgressTask()
{
  QgsFeatureDownloaderImpl::createProgressTask( mNumberMatched );
  CONNECT_PROGRESS_TASK( QgsOapifFeatureDownloaderImpl );
}

void QgsOapifFeatureDownloaderImpl::run( bool serializeFeatures, long long maxFeatures )
{
  QEventLoop loop;
  connect( this, &QgsOapifFeatureDownloaderImpl::doStop, &loop, &QEventLoop::quit );

  const bool useProgressDialog = ( !mShared->mHideProgressDialog && maxFeatures != 1 );

  long long maxTotalFeatures = 0;
  if ( maxFeatures > 0 && mShared->mMaxFeatures > 0 )
  {
    maxTotalFeatures = std::min( maxFeatures, mShared->mMaxFeatures );
  }
  else if ( maxFeatures > 0 )
  {
    maxTotalFeatures = maxFeatures;
  }
  else
  {
    maxTotalFeatures = mShared->mMaxFeatures;
  }

  long long totalDownloadedFeatureCount = 0;
  bool interrupted = false;
  bool success = true;
  QString errorMessage;
  QString url;

  long long maxFeaturesThisRequest = maxTotalFeatures;
  if ( mShared->mPageSize > 0 )
  {
    if ( maxFeaturesThisRequest > 0 )
    {
      maxFeaturesThisRequest = std::min( maxFeaturesThisRequest, mShared->mPageSize );
    }
    else
    {
      maxFeaturesThisRequest = mShared->mPageSize;
    }
  }
  url = mShared->mItemsUrl;
  bool hasQueryParam = false;
  if ( maxFeaturesThisRequest > 0 )
  {
    url += QStringLiteral( "?limit=%1" ).arg( maxFeaturesThisRequest );
    hasQueryParam = true;
  }

  // mServerFilter comes from the translation of the uri "filter" parameter
  // mServerExpression comes from the translation of a getFeatures() expression
  if ( !mShared->mServerFilter.isEmpty() )
  {
    url += ( hasQueryParam ? QStringLiteral( "&" ) : QStringLiteral( "?" ) );
    if ( !mShared->mServerExpression.isEmpty() )
    {
      // Combine mServerFilter and mServerExpression
      QStringList components1 = mShared->mServerFilter.split( QLatin1Char( '&' ) );
      QStringList components2 = mShared->mServerExpression.split( QLatin1Char( '&' ) );
      Q_ASSERT( components1[0].startsWith( QLatin1String( "filter=" ) ) );
      Q_ASSERT( components2[0].startsWith( QLatin1String( "filter=" ) ) );
      url += QLatin1String( "filter=" );
      url += '(';
      url += components1[0].mid( static_cast<int>( strlen( "filter=" ) ) );
      url += QLatin1String( ") AND (" );
      url += components2[0].mid( static_cast<int>( strlen( "filter=" ) ) );
      url += ')';
      // Add components1 extra parameters: filter-lang and filter-crs
      for ( int i = 1; i < components1.size(); ++i )
      {
        url += '&';
        url += components1[i];
      }
      // Add components2 extra parameters, not already included in components1
      for ( int i = 1; i < components2.size(); ++i )
      {
        if ( !components1.contains( components2[i] ) )
        {
          url += '&';
          url += components1[i];
        }
      }
    }
    else
    {
      url += mShared->mServerFilter;
    }
    hasQueryParam = true;
  }
  else if ( !mShared->mServerExpression.isEmpty() )
  {
    url += ( hasQueryParam ? QStringLiteral( "&" ) : QStringLiteral( "?" ) );
    url += mShared->mServerExpression;
    hasQueryParam = true;
  }

  QgsRectangle rect = mShared->currentRect();
  if ( !rect.isNull() )
  {
    if ( mShared->mSourceCrs.isGeographic() )
    {
      // Clamp to avoid server errors.
      rect.setXMinimum( std::max( -180.0, rect.xMinimum() ) );
      rect.setYMinimum( std::max( -90.0, rect.yMinimum() ) );
      rect.setXMaximum( std::min( 180.0, rect.xMaximum() ) );
      rect.setYMaximum( std::min( 90.0, rect.yMaximum() ) );
      if ( rect.xMinimum() > 180.0 || rect.yMinimum() > 90.0 || rect.xMaximum() < -180.0 || rect.yMaximum() < -90.0 )
      {
        // completely out of range. Servers could error out
        url.clear();
        rect = QgsRectangle();
      }
    }

    if ( mShared->mSourceCrs.hasAxisInverted() )
      rect.invert();

    if ( !rect.isNull() )
    {
      url += ( hasQueryParam ? QStringLiteral( "&" ) : QStringLiteral( "?" ) );
      url += QStringLiteral( "bbox=%1,%2,%3,%4" )
               .arg( qgsDoubleToString( rect.xMinimum() ), qgsDoubleToString( rect.yMinimum() ), qgsDoubleToString( rect.xMaximum() ), qgsDoubleToString( rect.yMaximum() ) );

      if ( mShared->mSourceCrs
           != QgsCoordinateReferenceSystem::fromOgcWmsCrs( QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS ) )
        url += QStringLiteral( "&bbox-crs=%1" ).arg( mShared->mSourceCrs.toOgcUri() );
    }
  }

  if ( mShared->mSourceCrs
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS ) )
    url += QStringLiteral( "&crs=%1" ).arg( mShared->mSourceCrs.toOgcUri() );

  while ( !url.isEmpty() )
  {
    url = mShared->appendExtraQueryParameters( url );

    if ( maxTotalFeatures > 0 && totalDownloadedFeatureCount >= maxTotalFeatures )
    {
      break;
    }

    QgsOapifItemsRequest itemsRequest( mShared->mURI.uri(), url );
    connect( &itemsRequest, &QgsOapifItemsRequest::gotResponse, &loop, &QEventLoop::quit );
    itemsRequest.request( false /* synchronous*/, true /* forceRefresh */ );
    loop.exec( QEventLoop::ExcludeUserInputEvents );
    if ( mStop )
    {
      interrupted = true;
      success = false;
      break;
    }
    if ( itemsRequest.errorCode() != QgsBaseNetworkRequest::NoError )
    {
      errorMessage = itemsRequest.errorMessage();
      success = false;
      break;
    }
    if ( itemsRequest.features().empty() )
    {
      break;
    }
    url = itemsRequest.nextUrl();

    // Consider if we should display a progress dialog
    // We can only do that if we know how many features will be downloaded
    if ( mNumberMatched < 0 && !mTimer && useProgressDialog && itemsRequest.numberMatched() > 0 )
    {
      mNumberMatched = itemsRequest.numberMatched();
      CREATE_PROGRESS_TASK( QgsOapifFeatureDownloaderImpl );
    }

    totalDownloadedFeatureCount += itemsRequest.features().size();
    if ( !mStop )
    {
      emit updateProgress( totalDownloadedFeatureCount );
    }

    QVector<QgsFeatureUniqueIdPair> featureList;
    size_t i = 0;
    const QgsFields srcFields = itemsRequest.fields();
    const QgsFields dstFields = mShared->fields();
    for ( const auto &pair : itemsRequest.features() )
    {
      // In the case the features of the current page have not the same schema
      // as the layer, convert them
      const QgsFeature &f = pair.first;
      QgsFeature dstFeat( dstFields, f.id() );
      if ( f.hasGeometry() )
      {
        QgsGeometry g = f.geometry();
        if ( mShared->mSourceCrs.hasAxisInverted() )
          g.get()->swapXy();
        dstFeat.setGeometry( g );
      }
      const auto srcAttrs = f.attributes();
      for ( int j = 0; j < dstFields.size(); j++ )
      {
        const int srcIdx = srcFields.indexOf( dstFields[j].name() );
        if ( srcIdx >= 0 )
        {
          const QVariant &v = srcAttrs.value( srcIdx );
          const auto dstFieldType = dstFields.at( j ).type();
          if ( QgsVariantUtils::isNull( v ) )
            dstFeat.setAttribute( j, QgsVariantUtils::createNullVariant( dstFieldType ) );
          else if ( QgsWFSUtils::isCompatibleType( static_cast<QMetaType::Type>( v.userType() ), dstFieldType ) )
            dstFeat.setAttribute( j, v );
          else
            dstFeat.setAttribute( j, QgsVectorDataProvider::convertValue( dstFieldType, v.toString() ) );
        }
      }

      QString uniqueId( pair.second );
      if ( uniqueId.isEmpty() )
      {
        uniqueId = QgsBackgroundCachedSharedData::getMD5( f );
      }

      featureList.push_back( QgsFeatureUniqueIdPair( dstFeat, uniqueId ) );

      if ( ( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == itemsRequest.features().size() )
      {
        // We call it directly to avoid asynchronous signal notification, and
        // as serializeFeatures() can modify the featureList to remove features
        // that have already been cached, so as to avoid to notify them several
        // times to subscribers
        if ( serializeFeatures )
          mShared->serializeFeatures( featureList );

        if ( !featureList.isEmpty() )
        {
          emitFeatureReceived( featureList );
          emitFeatureReceived( featureList.size() );
        }

        featureList.clear();
      }
      i++;
    }

    if ( mShared->mPageSize <= 0 )
    {
      break;
    }
  }

  endOfRun( serializeFeatures, success, totalDownloadedFeatureCount, false /* truncatedResponse */, interrupted, errorMessage );
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
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconWfs.svg" ) );
}
