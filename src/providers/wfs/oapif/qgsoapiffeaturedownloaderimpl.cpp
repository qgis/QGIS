/***************************************************************************
    qgsoapiffeaturedownloaderimpl.cpp
    ---------------------------------
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

#include "qgsoapiffeaturedownloaderimpl.h"

#include "qgsfeaturedownloader.h"
#include "qgsgeometrycollection.h"
#include "qgsgml.h"
#include "qgsmessagelog.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgsoapifitemsrequest.h"
#include "qgsoapifshareddata.h"
#include "qgsoapifutils.h"
#include "qgssettings.h"
#include "qgsvectordataprovider.h"
#include "qgswfsutils.h"

#include <QEventLoop>

#include "moc_qgsoapiffeaturedownloaderimpl.cpp"

QgsOapifFeatureDownloaderImpl::QgsOapifFeatureDownloaderImpl( QgsOapifSharedData *shared, QgsFeatureDownloader *downloader, bool requestMadeFromMainThread )
  : QgsBaseNetworkRequest( shared->mURI.auth(), tr( "OAPIF" ) ), QgsFeatureDownloaderImpl( shared, downloader ), mShared( shared )
{
  QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS( requestMadeFromMainThread );
}

QgsOapifFeatureDownloaderImpl::~QgsOapifFeatureDownloaderImpl()
{
  stop();
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
  connect( this, &QgsBaseNetworkRequest::downloadFinished, &loop, &QEventLoop::quit );
  connect( this, &QgsBaseNetworkRequest::downloadProgress, &loop, &QEventLoop::quit );

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

  QString url = ( !mShared->mBulkDownloadGmlUrl.isEmpty() ) ? mShared->mBulkDownloadGmlUrl : mShared->mItemsUrl;
  bool hasQueryParam = url.indexOf( '?'_L1 ) > 0;
  if ( maxFeaturesThisRequest > 0 && mShared->mBulkDownloadGmlUrl.isEmpty() )
  {
    url += ( hasQueryParam ? '&'_L1 : '?'_L1 );
    url += u"limit=%1"_s.arg( maxFeaturesThisRequest );
    hasQueryParam = true;
  }

  // mServerFilter comes from the translation of the uri "filter" parameter
  // mServerExpression comes from the translation of a getFeatures() expression
  if ( !mShared->mServerFilter.isEmpty() )
  {
    url += ( hasQueryParam ? '&'_L1 : '?'_L1 );
    if ( !mShared->mServerExpression.isEmpty() )
    {
      // Combine mServerFilter and mServerExpression
      QStringList components1 = mShared->mServerFilter.split( '&'_L1 );
      QStringList components2 = mShared->mServerExpression.split( '&'_L1 );
      Q_ASSERT( components1[0].startsWith( "filter="_L1 ) );
      Q_ASSERT( components2[0].startsWith( "filter="_L1 ) );
      url += "filter="_L1;
      url += '(';
      url += components1[0].mid( static_cast<int>( strlen( "filter=" ) ) );
      url += ") AND ("_L1;
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
    url += ( hasQueryParam ? '&'_L1 : '?'_L1 );
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
      url += ( hasQueryParam ? '&'_L1 : '?'_L1 );
      url += u"bbox=%1,%2,%3,%4"_s
               .arg( qgsDoubleToString( rect.xMinimum() ), qgsDoubleToString( rect.yMinimum() ), qgsDoubleToString( rect.xMaximum() ), qgsDoubleToString( rect.yMaximum() ) );

      if ( mShared->mSourceCrs
           != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
        url += u"&bbox-crs=%1"_s.arg( mShared->mSourceCrs.toOgcUri() );
    }
  }

  if ( mShared->mSourceCrs
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
    url += u"&crs=%1"_s.arg( mShared->mSourceCrs.toOgcUri() );

  url = mShared->appendExtraQueryParameters( url );

  if ( mShared->mFeatureFormat.startsWith( "application/gml+xml"_L1 ) )
  {
    runGmlDownload( loop, url, serializeFeatures, maxTotalFeatures, useProgressDialog );
  }
  else
  {
    runGenericDownload( loop, url, serializeFeatures, maxTotalFeatures, useProgressDialog );
  }
}

void QgsOapifFeatureDownloaderImpl::runGmlDownload( QEventLoop &loop, QString url, bool serializeFeatures, long long maxTotalFeatures, bool useProgressDialog )
{
  QgsGmlStreamingParser::AxisOrientationLogic axisOrientationLogic( QgsGmlStreamingParser::Honour_EPSG_if_urn );
  if ( mShared->mURI.ignoreAxisOrientation() )
  {
    axisOrientationLogic = QgsGmlStreamingParser::Ignore_EPSG;
  }

  if ( mShared->mBulkDownloadGmlUrl.isEmpty() )
    mFakeResponseHasHeaders = true;

  bool interrupted = false;
  bool success = false;
  QgsSettings s;
  const int maxRetry = s.value( u"qgis/defaultTileMaxRetry"_s, "3" ).toInt();
  int retryIter = 0;
  long long totalDownloadedFeatureCount = 0;
  long long lastValidTotalDownloadedFeatureCount = 0;

  while ( !url.isEmpty() )
  {
    success = true;
    sendGET( url, mShared->mFeatureFormat, false, /* synchronous */
             true,                                /* forceRefresh */
             false /* cache */ );

    QgsGmlStreamingParser gmlParser( mShared->mURI.typeName(), mShared->mGeometryAttribute, mShared->mFields, axisOrientationLogic, mShared->mURI.invertAxisOrientation() );
    if ( !mShared->mFieldNameToXPathAndIsNestedContentMap.isEmpty() )
    {
      gmlParser.setFieldsXPath( mShared->mFieldNameToXPathAndIsNestedContentMap, mShared->mNamespacePrefixToURIMap );
    }

    QString nextUrl;
    bool bytesStillAvailableInReply = false;
    // Loop until there is no data coming from the current request
    while ( true )
    {
      if ( !bytesStillAvailableInReply )
      {
        loop.exec( QEventLoop::ExcludeUserInputEvents );
      }
      if ( mStop )
      {
        interrupted = true;
        success = false;
        break;
      }

      QByteArray data;
      bool finished = false;
      if ( mReply )
      {
        // Limit the number of bytes to process at once, to avoid the GML parser to
        // create too many objects.
        constexpr int CHUNK_SIZE = 10 * 1024 * 1024;
        data = mReply->read( CHUNK_SIZE );
        bytesStillAvailableInReply = mReply->bytesAvailable() > 0;
      }
      else
      {
        data = mResponse;
        finished = true;
      }

      if ( nextUrl.isEmpty() )
      {
        nextUrl = QgsOAPIFGetNextLinkFromResponseHeader( mResponseHeaders, mShared->mFeatureFormat );
        if ( !nextUrl.isEmpty() )
          nextUrl = mShared->appendExtraQueryParameters( nextUrl );
      }

      // Parse the received chunk of data
      QString gmlProcessErrorMsg;
      if ( !gmlParser.processData( data, finished, gmlProcessErrorMsg ) )
      {
        success = false;
        // Only add an error message if no general networking related error has been
        // previously reported by QgsWfsRequest logic.
        // We indeed make processData() run even if an error has been reported,
        // so that we have a chance to parse XML errors (isException() case below)
        if ( mErrorCode == NoError )
        {
          mErrorMessage = tr( "Error when parsing GetFeature response" ) + " : " + gmlProcessErrorMsg;
          QgsMessageLog::logMessage( mErrorMessage, tr( "OAPIF" ) );
        }
        break;
      }
      else if ( gmlParser.isException() )
      {
        // Only process the exception report if we get the full error response.
        if ( !finished )
          continue;
        success = false;

        mErrorMessage = tr( "Server generated an exception in GetFeature response" ) + ": " + gmlParser.exceptionText();
        QgsMessageLog::logMessage( mErrorMessage, tr( "OAPI" ) );

        break;
      }
      // Test error code only after having let a chance to the parser to process the ExceptionReport
      else if ( mErrorCode != NoError )
      {
        success = false;
        break;
      }

      if ( totalDownloadedFeatureCount == 0 && useProgressDialog && !mResponseHeaders.empty() && !mTimer && mNumberMatched < 0 )
      {
        for ( const auto &headerKeyValue : mResponseHeaders )
        {
          if ( headerKeyValue.first.compare( QByteArray( "OGC-NumberMatched" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 )
          {
            bool ok = false;
            const long long val = QString::fromUtf8( headerKeyValue.second ).toLongLong( &ok );
            if ( ok )
            {
              mNumberMatched = val;

              CREATE_PROGRESS_TASK( QgsOapifFeatureDownloaderImpl );
            }
          }
        }
      }


      QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> featurePtrList = gmlParser.getAndStealReadyFeatures();

      totalDownloadedFeatureCount += featurePtrList.size();

      if ( !mStop )
      {
        emit updateProgress( totalDownloadedFeatureCount );
      }

      if ( featurePtrList.size() != 0 )
      {
        QVector<QgsFeatureUniqueIdPair> featureList;
        for ( int i = 0; i < featurePtrList.size(); i++ )
        {
          QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair &featPair = featurePtrList[i];
          QgsFeature &f = *( featPair.first );
          QString gmlId( featPair.second );
          if ( gmlId.isEmpty() )
          {
            // Should normally not happen on sane WFS sources, but can happen with
            // Geomedia
            gmlId = QgsBackgroundCachedSharedData::getMD5( f );
            if ( !mShared->mHasWarnedAboutMissingFeatureId )
            {
              QgsDebugError( u"Server returns features without fid/gml:id. Computing a fake one using feature attributes"_s );
              mShared->mHasWarnedAboutMissingFeatureId = true;
            }
          }

          // Coerce geometry type if needed
          if ( f.hasGeometry() && QgsWkbTypes::flatType( f.geometry().wkbType() ) != QgsWkbTypes::flatType( mShared->mWKBType ) )
          {
            QVector< QgsGeometry > coercedGeoms = f.geometry().coerceToType( mShared->mWKBType );
            if ( coercedGeoms.size() == 1 )
              f.setGeometry( coercedGeoms[0] );
          }

          featureList.push_back( QgsFeatureUniqueIdPair( f, gmlId ) );
          delete featPair.first;
          if ( ( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == featurePtrList.size() )
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
        }
      }

      if ( maxTotalFeatures > 0 && totalDownloadedFeatureCount >= maxTotalFeatures )
      {
        url.clear();
        break;
      }
      if ( finished )
      {
        break;
      }
    }

    if ( mStop )
      break;

    if ( !success )
    {
      if ( ++retryIter <= maxRetry )
      {
        QgsMessageLog::logMessage( tr( "Retrying request %1: %2/%3" ).arg( url ).arg( retryIter ).arg( maxRetry ), tr( "OAPIF" ) );
        totalDownloadedFeatureCount = lastValidTotalDownloadedFeatureCount;
        continue;
      }

      break;
    }

    url = nextUrl;
    retryIter = 0;
    lastValidTotalDownloadedFeatureCount = totalDownloadedFeatureCount;
  }

  endOfRun( serializeFeatures, success, totalDownloadedFeatureCount, false /* truncatedResponse */, interrupted, mErrorMessage );

  // explicitly abort here so that mReply is destroyed within the right thread
  // otherwise will deadlock because deleteLayer() will not have a valid thread to post
  abort();
}

void QgsOapifFeatureDownloaderImpl::runGenericDownload( QEventLoop &loop, QString url, bool serializeFeatures, long long maxTotalFeatures, bool useProgressDialog )
{
  long long totalDownloadedFeatureCount = 0;
  bool interrupted = false;
  bool success = true;
  QString errorMessage;

  while ( !url.isEmpty() )
  {
    if ( maxTotalFeatures > 0 && totalDownloadedFeatureCount >= maxTotalFeatures )
    {
      break;
    }

    QgsOapifItemsRequest itemsRequest( mShared->mURI.uri(), url, mShared->mFeatureFormat );
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
    if ( !url.isEmpty() )
      url = mShared->appendExtraQueryParameters( url );

    // Consider if we should display a progress dialog
    // We can only do that if we know how many features will be downloaded
    if ( mNumberMatched < 0 && !mTimer && useProgressDialog && itemsRequest.numberMatched() > 0 )
    {
      mNumberMatched = itemsRequest.numberMatched();
      CREATE_PROGRESS_TASK( QgsOapifFeatureDownloaderImpl );
    }

    totalDownloadedFeatureCount += static_cast<long long>( itemsRequest.features().size() );
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

        // Promote single geometries to multipart if necessary
        if ( QgsWkbTypes::flatType( g.wkbType() ) == Qgis::WkbType::Point && mShared->mWKBType == Qgis::WkbType::MultiPoint )
          g = g.convertToType( Qgis::GeometryType::Point, /* destMultipart = */ true );
        else if ( QgsWkbTypes::flatType( g.wkbType() ) == Qgis::WkbType::LineString && mShared->mWKBType == Qgis::WkbType::MultiLineString )
          g = g.convertToType( Qgis::GeometryType::Line, /* destMultipart = */ true );
        else if ( QgsWkbTypes::flatType( g.wkbType() ) == Qgis::WkbType::Polygon && mShared->mWKBType == Qgis::WkbType::MultiPolygon )
          g = g.convertToType( Qgis::GeometryType::Polygon, /* destMultipart = */ true );

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

QString QgsOapifFeatureDownloaderImpl::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of features failed: %1" ).arg( reason );
}
