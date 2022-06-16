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
#include "qgsoapiflandingpagerequest.h"
#include "qgsoapifapirequest.h"
#include "qgsoapifcollection.h"
#include "qgsoapifitemsrequest.h"
#include "qgswfsconstants.h"
#include "qgswfsutils.h" // for isCompatibleType()

#include <algorithm>
#include <QIcon>

const QString QgsOapifProvider::OAPIF_PROVIDER_KEY = QStringLiteral( "OAPIF" );
const QString QgsOapifProvider::OAPIF_PROVIDER_DESCRIPTION = QStringLiteral( "OGC API - Features data provider" );


QgsOapifProvider::QgsOapifProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags ),
    mShared( new QgsOapifSharedData( uri ) )
{

  connect( mShared.get(), &QgsOapifSharedData::raiseError, this, &QgsOapifProvider::pushErrorSlot );
  connect( mShared.get(), &QgsOapifSharedData::extentUpdated, this, &QgsOapifProvider::fullExtentCalculated );

  if ( uri.isEmpty() )
  {
    mValid = false;
    return;
  }

  mShared->mSourceCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );

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

  mShared->mServerMaxFeatures = apiRequest.maxLimit();

  if ( mShared->mURI.maxNumFeatures() > 0 && mShared->mServerMaxFeatures > 0 && !mShared->mURI.pagingEnabled() )
  {
    mShared->mMaxFeatures = std::min( mShared->mURI.maxNumFeatures(), mShared->mServerMaxFeatures );
  }
  else if ( mShared->mURI.maxNumFeatures() > 0 )
  {
    mShared->mMaxFeatures = mShared->mURI.maxNumFeatures();
  }
  else if ( mShared->mServerMaxFeatures > 0 && !mShared->mURI.pagingEnabled() )
  {
    mShared->mMaxFeatures = mShared->mServerMaxFeatures;
  }

  if ( mShared->mURI.pagingEnabled() && mShared->mURI.pageSize() > 0 )
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
  else if ( mShared->mURI.pagingEnabled() )
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

  mShared->mCollectionUrl =
    landingPageRequest.collectionsUrl() + QStringLiteral( "/" ) + mShared->mURI.typeName();
  QgsOapifCollectionRequest collectionRequest( mShared->mURI.uri(), mShared->appendExtraQueryParameters( mShared->mCollectionUrl ) );
  if ( !collectionRequest.request( synchronous, forceRefresh ) )
    return false;
  if ( collectionRequest.errorCode() != QgsBaseNetworkRequest::NoError )
    return false;

  mShared->mCapabilityExtent = collectionRequest.collection().mBbox;

  mLayerMetadata = collectionRequest.collection().mLayerMetadata;
  // Merge contact info from /api
  mLayerMetadata.setContacts( apiRequest.metadata().contacts() );

  mShared->mItemsUrl = mShared->mCollectionUrl +  QStringLiteral( "/items" );

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
  }

  mShared->mFields = itemsRequest.fields();
  mShared->mWKBType = itemsRequest.wkbType();

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

QgsWkbTypes::Type QgsOapifProvider::wkbType() const
{
  return mShared->mWKBType;
}

long long QgsOapifProvider::featureCount() const
{
  if ( mUpdateFeatureCountAtNextFeatureCountRequest )
  {
    mUpdateFeatureCountAtNextFeatureCountRequest = false;

    QgsFeature f;
    QgsFeatureRequest request;
    request.setNoAttributes();
    auto iter = getFeatures( request );
    long long count = 0;
    bool countExact = true;
    while ( iter.nextFeature( f ) )
    {
      if ( count == 1000 ) // to avoid too long processing time
      {
        countExact = false;
        break;
      }
      count ++;
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

QgsVectorDataProvider::Capabilities QgsOapifProvider::capabilities() const
{
  return QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::ReadLayerMetadata | QgsVectorDataProvider::Capability::ReloadData;
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
  request.setFlags( QgsFeatureRequest::NoGeometry );

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
  QgsBackgroundCachedSharedData::copyStateToClone( copy );

  return copy;
}

static QDateTime getDateTimeValue( const QVariant &v )
{
  if ( v.type() == QVariant::String )
    return QDateTime::fromString( v.toString(), Qt::ISODateWithMs );
  else if ( v.type() == QVariant::DateTime )
    return v.toDateTime();
  return QDateTime();
}

static bool isDateTime( const QVariant &v )
{
  return getDateTimeValue( v ).isValid();
}

static QString getDateTimeValueAsString( const QVariant &v )
{
  if ( v.type() == QVariant::String )
    return v.toString();
  else if ( v.type() == QVariant::DateTime )
    return v.toDateTime().toString( Qt::ISODateWithMs );
  return QString();
}

static bool isDateTimeField( const QgsFields &fields, const QString &fieldName )
{
  const int idx = fields.indexOf( fieldName );
  if ( idx >= 0 )
  {
    const auto type = fields.at( idx ).type();
    return type == QVariant::DateTime || type == QVariant::Date;
  }
  return false;
}

static void collectTopLevelAndNodes( const QgsExpressionNode *node,
                                     std::vector<const QgsExpressionNode *> &topAndNodes )
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

QString QgsOapifSharedData::translateNodeToServer(
  const QgsExpressionNode *rootNode,
  QgsOapifProvider::FilterTranslationState &translationState,
  QString &untranslatedPart )
{
  std::vector<const QgsExpressionNode *> topAndNodes;
  collectTopLevelAndNodes( rootNode, topAndNodes );
  QDateTime minDate;
  QDateTime maxDate;
  QString minDateStr;
  QString maxDateStr;
  bool hasTranslatedParts = false;
  for ( size_t i = 0; i < topAndNodes.size(); /* do not increment here */ )
  {
    bool removeMe = false;
    const auto node = topAndNodes[i];
    if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator )
    {
      const auto binNode = static_cast<const QgsExpressionNodeBinaryOperator *>( node );
      const auto op = binNode->op();
      if ( binNode->opLeft()->nodeType() == QgsExpressionNode::ntColumnRef &&
           binNode->opRight()->nodeType() == QgsExpressionNode::ntLiteral )
      {
        const auto left = static_cast<const QgsExpressionNodeColumnRef *>( binNode->opLeft() );
        const auto right = static_cast<const QgsExpressionNodeLiteral *>( binNode->opRight() );
        if ( isDateTimeField( mFields, left->name() ) &&
             isDateTime( right->value() ) )
        {
          if ( op == QgsExpressionNodeBinaryOperator::boGE ||
               op == QgsExpressionNodeBinaryOperator::boGT ||
               op == QgsExpressionNodeBinaryOperator::boEQ )
          {
            removeMe = true;
            if ( !minDate.isValid() || getDateTimeValue( right->value() ) > minDate )
            {
              minDate = getDateTimeValue( right->value() );
              minDateStr = getDateTimeValueAsString( right->value() );
            }
          }
          if ( op == QgsExpressionNodeBinaryOperator::boLE ||
               op == QgsExpressionNodeBinaryOperator::boLT ||
               op == QgsExpressionNodeBinaryOperator::boEQ )
          {
            removeMe = true;
            if ( !maxDate.isValid() || getDateTimeValue( right->value() ) < maxDate )
            {
              maxDate = getDateTimeValue( right->value() );
              maxDateStr = getDateTimeValueAsString( right->value() );
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
  const auto rootNode = expr.rootNode();
  if ( !rootNode )
    return false;
  mServerFilter = translateNodeToServer( rootNode, mFilterTranslationState, mClientSideFilterExpression );
  if ( mFilterTranslationState == QgsOapifProvider::FilterTranslationState::PARTIAL )
  {
    QgsDebugMsg( QStringLiteral( "Part of the filter will be evaluated on client-side: %1" ).arg( mClientSideFilterExpression ) );
  }
  else if ( mFilterTranslationState == QgsOapifProvider::FilterTranslationState::FULLY_CLIENT )
  {
    QgsDebugMsg( "Whole filter will be evaluated on client-side" );
  }

  return true;
}

void QgsOapifSharedData::pushError( const QString &errorMsg ) const
{
  QgsMessageLog::logMessage( errorMsg, tr( "OAPIF" ) );
  emit raiseError( errorMsg );
}

// ---------------------------------

QgsOapifFeatureDownloaderImpl::QgsOapifFeatureDownloaderImpl( QgsOapifSharedData *shared, QgsFeatureDownloader *downloader, bool requestMadeFromMainThread ):
  QgsFeatureDownloaderImpl( shared, downloader ),
  mShared( shared )
{
  QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS( requestMadeFromMainThread );
}

QgsOapifFeatureDownloaderImpl::~QgsOapifFeatureDownloaderImpl()
{
}

void QgsOapifFeatureDownloaderImpl::createProgressDialog()
{
  QgsFeatureDownloaderImpl::createProgressDialog( mNumberMatched );
  CONNECT_PROGRESS_DIALOG( QgsOapifFeatureDownloaderImpl );
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

  if ( !mShared->mServerFilter.isEmpty() )
  {
    url += ( hasQueryParam ? QStringLiteral( "&" ) : QStringLiteral( "?" ) );
    url += mShared->mServerFilter;
    hasQueryParam = true;
  }

  const QgsRectangle &rect = mShared->currentRect();
  if ( !rect.isNull() )
  {
    // Clamp to avoid server errors.
    const double minx = std::max( -180.0, rect.xMinimum() );
    const double miny = std::max( -90.0, rect.yMinimum() );
    const double maxx = std::min( 180.0, rect.xMaximum() );
    const double maxy = std::min( 90.0, rect.yMaximum() );
    if ( minx > 180.0 || miny > 90.0 || maxx  < -180.0 || maxy < -90.0 )
    {
      // completely out of range. Servers could error out
      url.clear();
    }
    else if ( minx > -180.0 || miny > -90.0 || maxx < 180.0 || maxy < 90.0 )
    {
      url += ( hasQueryParam ? QStringLiteral( "&" ) : QStringLiteral( "?" ) );
      url += QStringLiteral( "bbox=%1,%2,%3,%4" )
             .arg( qgsDoubleToString( minx ),
                   qgsDoubleToString( miny ),
                   qgsDoubleToString( maxx ),
                   qgsDoubleToString( maxy ) );
    }
  }

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
      CREATE_PROGRESS_DIALOG( QgsOapifFeatureDownloaderImpl );
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
      dstFeat.setGeometry( f.geometry() );
      const auto srcAttrs = f.attributes();
      for ( int j = 0; j < dstFields.size(); j++ )
      {
        const int srcIdx = srcFields.indexOf( dstFields[j].name() );
        if ( srcIdx >= 0 )
        {
          const QVariant &v = srcAttrs.value( srcIdx );
          const auto dstFieldType = dstFields.at( j ).type();
          if ( v.isNull() )
            dstFeat.setAttribute( j, QVariant( dstFieldType ) );
          else if ( QgsWFSUtils::isCompatibleType( v.type(), dstFieldType ) )
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

QgsOapifProvider *QgsOapifProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsOapifProvider( uri, options, flags );
}

QList<QgsMapLayerType> QgsOapifProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::VectorLayer };
}

QgsOapifProviderMetadata::QgsOapifProviderMetadata():
  QgsProviderMetadata( QgsOapifProvider::OAPIF_PROVIDER_KEY, QgsOapifProvider::OAPIF_PROVIDER_DESCRIPTION ) {}

QIcon QgsOapifProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconWfs.svg" ) );
}
