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
#include "qgsoapifitemsrequest.h"
#include "qgsoapifshareddata.h"
#include "qgsoapifutils.h"
#include "qgsvectordataprovider.h"
#include "qgswfsutils.h"

#include <QEventLoop>

#include "moc_qgsoapiffeaturedownloaderimpl.cpp"

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
  bool hasQueryParam = url.indexOf( QLatin1Char( '?' ) ) > 0;
  if ( maxFeaturesThisRequest > 0 )
  {
    url += ( hasQueryParam ? QLatin1Char( '&' ) : QLatin1Char( '?' ) );
    url += QStringLiteral( "limit=%1" ).arg( maxFeaturesThisRequest );
    hasQueryParam = true;
  }

  // mServerFilter comes from the translation of the uri "filter" parameter
  // mServerExpression comes from the translation of a getFeatures() expression
  if ( !mShared->mServerFilter.isEmpty() )
  {
    url += ( hasQueryParam ? QLatin1Char( '&' ) : QLatin1Char( '?' ) );
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
    url += ( hasQueryParam ? QLatin1Char( '&' ) : QLatin1Char( '?' ) );
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
      url += ( hasQueryParam ? QLatin1Char( '&' ) : QLatin1Char( '?' ) );
      url += QStringLiteral( "bbox=%1,%2,%3,%4" )
               .arg( qgsDoubleToString( rect.xMinimum() ), qgsDoubleToString( rect.yMinimum() ), qgsDoubleToString( rect.xMaximum() ), qgsDoubleToString( rect.yMaximum() ) );

      if ( mShared->mSourceCrs
           != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
        url += QStringLiteral( "&bbox-crs=%1" ).arg( mShared->mSourceCrs.toOgcUri() );
    }
  }

  if ( mShared->mSourceCrs
       != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
    url += QStringLiteral( "&crs=%1" ).arg( mShared->mSourceCrs.toOgcUri() );

  while ( !url.isEmpty() )
  {
    url = mShared->appendExtraQueryParameters( url );

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
