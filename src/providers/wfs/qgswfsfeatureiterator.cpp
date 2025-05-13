/***************************************************************************
    qgswfsfeatureiterator.cpp
    ---------------------
    begin                : January 2013
    copyright            : (C) 2013 by Marco Hugentobler
                           (C) 2016 by Even Rouault
    email                : marco dot hugentobler at sourcepole dot ch
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessagelog.h"
#include "qgsgeometry.h"
#include "qgsgml.h"
#include "qgsgeometrycollection.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgsogcutils.h"
#include "qgswfsfeatureiterator.h"
#include "moc_qgswfsfeatureiterator.cpp"
#include "qgswfsshareddata.h"
#include "qgswfsutils.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <algorithm>
#include <QDir>
#include <QTimer>
#include <QUrlQuery>
#include <QTransform>

QgsWFSFeatureHitsAsyncRequest::QgsWFSFeatureHitsAsyncRequest( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
  , mNumberMatched( -1 )
{
  connect( this, &QgsWfsRequest::downloadFinished, this, &QgsWFSFeatureHitsAsyncRequest::hitsReplyFinished );
}

void QgsWFSFeatureHitsAsyncRequest::launchGet( const QUrl &url )
{
  sendGET( url,
           QString(), // content-type
           false,     /* synchronous */
           true,      /* forceRefresh */
           false /* cache */ );
}

void QgsWFSFeatureHitsAsyncRequest::launchPost( const QUrl &url, const QByteArray &data )
{
  sendPOST( url, QStringLiteral( "application/xml; charset=utf-8" ), data, false, /* synchronous */
            { QNetworkReply::RawHeaderPair { "Accept", "application/xml" } } );
}

void QgsWFSFeatureHitsAsyncRequest::hitsReplyFinished()
{
  if ( mErrorCode == NoError )
  {
    QByteArray data = response();
    QgsGmlStreamingParser gmlParser( ( QString() ), ( QString() ), QgsFields() );
    QString errorMsg;
    if ( gmlParser.processData( data, true, errorMsg ) )
    {
      mNumberMatched = ( gmlParser.numberMatched() >= 0 ) ? gmlParser.numberMatched() : gmlParser.numberReturned();
    }
    else
    {
      QgsMessageLog::logMessage( errorMsg, tr( "WFS" ) );
    }
  }
  emit gotHitsResponse();
}

QString QgsWFSFeatureHitsAsyncRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature count failed: %1" ).arg( reason );
}

// -------------------------

QgsWFSFeatureDownloaderImpl::QgsWFSFeatureDownloaderImpl( QgsWFSSharedData *shared, QgsFeatureDownloader *downloader, bool requestMadeFromMainThread )
  : QgsWfsRequest( shared->mURI )
  , QgsFeatureDownloaderImpl( shared, downloader )
  , mShared( shared )
  , mPageSize( shared->mPageSize )
  , mFeatureHitsAsyncRequest( shared->mURI )
{
  QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS( requestMadeFromMainThread );
}

QgsWFSFeatureDownloaderImpl::~QgsWFSFeatureDownloaderImpl()
{
  stop();

  if ( mTimer )
    mTimer->deleteLater();
}

QString QgsWFSFeatureDownloaderImpl::sanitizeFilter( QString filter )
{
  filter = filter.replace( QLatin1String( "<fes:ValueReference xmlns:fes=\"http://www.opengis.net/fes/2.0\">" ), QLatin1String( "<fes:ValueReference>" ) );
  QString nsPrefix( QgsWFSUtils::nameSpacePrefix( mShared->mURI.typeName() ) );
  if ( mRemoveNSPrefix && !nsPrefix.isEmpty() )
    filter = filter.replace( "<fes:ValueReference>" + nsPrefix + ":", QLatin1String( "<fes:ValueReference>" ) );
  return filter;
}

std::pair<QString, QString> QgsWFSFeatureDownloaderImpl::determineTypeNames() const
{
  QString typenames;
  QString namespaces;
  if ( mShared->mLayerPropertiesList.isEmpty() )
  {
    typenames = mShared->mURI.typeName();
    namespaces = mShared->mCaps.getNamespaceParameterValue( mShared->mWFSVersion, typenames );
  }
  else
  {
    QSet<QString> setNamespaces;
    for ( const QgsOgcUtils::LayerProperties &layerProperties : std::as_const( mShared->mLayerPropertiesList ) )
    {
      if ( !typenames.isEmpty() )
        typenames += QLatin1Char( ',' );
      typenames += layerProperties.mName;
      const QString lNamespace = mShared->mCaps.getNamespaceParameterValue( mShared->mWFSVersion, layerProperties.mName );
      if ( !lNamespace.isEmpty() && !setNamespaces.contains( lNamespace ) )
      {
        if ( !namespaces.isEmpty() )
          namespaces += QLatin1Char( ',' );
        namespaces += lNamespace;
        setNamespaces.insert( lNamespace );
      }
    }
  }
  return std::make_pair( typenames, namespaces );
}

bool QgsWFSFeatureDownloaderImpl::useInvertedAxis() const
{
  bool invertAxis = false;
  if ( !mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) && !mShared->mURI.ignoreAxisOrientation() )
  {
    // cloned branches are intentional here for improved readability
    // NOLINTBEGIN(bugprone-branch-clone)

    // This is a bit nasty, but if the server reports OGC::CRS84
    // mSourceCrs will report hasAxisInverted() == false, but srsName()
    // will be urn:ogc:def:crs:EPSG::4326, so axis inversion is needed...
    if ( mShared->srsName() == QLatin1String( "urn:ogc:def:crs:EPSG::4326" ) )
    {
      invertAxis = true;
    }
    else if ( mShared->mSourceCrs.hasAxisInverted() )
    {
      invertAxis = true;
    }

    // NOLINTEND(bugprone-branch-clone)
  }
  if ( mShared->mURI.invertAxisOrientation() )
  {
    invertAxis = !invertAxis;
  }
  return invertAxis;
}

QUrl QgsWFSFeatureDownloaderImpl::buildURL( qint64 startIndex, long long maxFeatures, bool forHits )
{
  QUrl getFeatureUrl( mShared->mURI.requestUrl( QStringLiteral( "GetFeature" ), mShared->mHttpMethod ) );
  QUrlQuery query( getFeatureUrl );
  query.addQueryItem( QStringLiteral( "VERSION" ), mShared->mWFSVersion );

  QString typenames;
  QString namespaces;
  std::tie( typenames, namespaces ) = determineTypeNames();
  if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
  {
    query.addQueryItem( QStringLiteral( "TYPENAMES" ), typenames );
  }
  else
  {
    query.addQueryItem( QStringLiteral( "TYPENAME" ), typenames );
  }

  if ( forHits )
  {
    query.addQueryItem( QStringLiteral( "RESULTTYPE" ), QStringLiteral( "hits" ) );
  }
  else if ( maxFeatures > 0 )
  {
    if ( mPageSize > 0 )
    {
      // Note: always include the STARTINDEX, even for zero, has some (likely buggy)
      // implementations do not return the same results if STARTINDEX=0 is specified
      // or not.
      // For example http://demo.opengeo.org/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=ne:ne_10m_admin_0_countries&STARTINDEX=0&COUNT=253
      // doesn't include ne_10m_admin_0_countries.99, as expected since it is
      // at index 254.
      query.addQueryItem( QStringLiteral( "STARTINDEX" ), QString::number( startIndex ) );
    }
    if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
      query.addQueryItem( QStringLiteral( "COUNT" ), QString::number( maxFeatures ) );
    else
      query.addQueryItem( QStringLiteral( "MAXFEATURES" ), QString::number( maxFeatures ) );
  }
  QString srsName( mShared->srsName() );
  if ( !srsName.isEmpty() && !forHits )
  {
    query.addQueryItem( QStringLiteral( "SRSNAME" ), srsName );
  }

  // In case we must issue a BBOX and we have a filter, we must combine
  // both as a single filter, as both BBOX and FILTER aren't supported together
  std::vector<QString> filters;
  if ( !mShared->mServerExpression.isEmpty() )
    filters.push_back( mShared->mServerExpression );

  const QgsRectangle &rect = mShared->currentRect();
  if ( !rect.isNull() && ( !mShared->mWFSFilter.isEmpty() || !mShared->mServerExpression.isEmpty() || !mShared->mWFSGeometryTypeFilter.isEmpty() ) )
  {
    QgsOgcUtils::GMLVersion gmlVersion;
    QgsOgcUtils::FilterVersion filterVersion;
    bool honourAxisOrientation = false;
    if ( mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
    {
      gmlVersion = QgsOgcUtils::GML_2_1_2;
      filterVersion = QgsOgcUtils::FILTER_OGC_1_0;
    }
    else if ( mShared->mWFSVersion.startsWith( QLatin1String( "1.1" ) ) )
    {
      honourAxisOrientation = !mShared->mURI.ignoreAxisOrientation();
      gmlVersion = QgsOgcUtils::GML_3_1_0;
      filterVersion = QgsOgcUtils::FILTER_OGC_1_1;
    }
    else
    {
      honourAxisOrientation = !mShared->mURI.ignoreAxisOrientation();
      gmlVersion = QgsOgcUtils::GML_3_2_1;
      filterVersion = QgsOgcUtils::FILTER_FES_2_0;
    }
    QString geometryAttribute( mShared->mGeometryAttribute );
    if ( mShared->mLayerPropertiesList.size() > 1 )
      geometryAttribute = mShared->mURI.typeName() + "/" + geometryAttribute;


    double minx = rect.xMinimum();
    double miny = rect.yMinimum();
    double maxx = rect.xMaximum();
    double maxy = rect.yMaximum();
    QString filterBbox( QStringLiteral( "intersects_bbox($geometry, geomFromWKT('LINESTRING(%1 %2,%3 %4)'))" ).arg( minx ).arg( miny ).arg( maxx ).arg( maxy ) );
    QgsExpression bboxExp( filterBbox );
    QDomDocument bboxDoc;

    QMap<QString, QString> fieldNameToXPathMap;
    if ( !mShared->mFieldNameToXPathAndIsNestedContentMap.isEmpty() )
    {
      for ( auto iterFieldName = mShared->mFieldNameToXPathAndIsNestedContentMap.constBegin(); iterFieldName != mShared->mFieldNameToXPathAndIsNestedContentMap.constEnd(); ++iterFieldName )
      {
        const QString &fieldName = iterFieldName.key();
        const auto &value = iterFieldName.value();
        fieldNameToXPathMap[fieldName] = value.first;
      }
    }

    QDomElement bboxElem = QgsOgcUtils::expressionToOgcFilter( bboxExp, bboxDoc, gmlVersion, filterVersion, mShared->mLayerPropertiesList.size() == 1 ? mShared->mLayerPropertiesList[0].mNamespacePrefix : QString(), mShared->mLayerPropertiesList.size() == 1 ? mShared->mLayerPropertiesList[0].mNamespaceURI : QString(), geometryAttribute, mShared->srsName(), honourAxisOrientation, mShared->mURI.invertAxisOrientation(), nullptr, fieldNameToXPathMap, mShared->mNamespacePrefixToURIMap );
    bboxDoc.appendChild( bboxElem );

    filters.push_back( bboxDoc.toString() );
  }
  if ( !mShared->mWFSFilter.isEmpty() )
    filters.push_back( mShared->mWFSFilter );
  if ( !mShared->mWFSGeometryTypeFilter.isEmpty() )
    filters.push_back( mShared->mWFSGeometryTypeFilter );

  if ( filters.size() >= 2 )
  {
    query.addQueryItem( QStringLiteral( "FILTER" ), sanitizeFilter( mShared->combineWFSFilters( filters ) ) );
  }
  else if ( !rect.isNull() )
  {
    const bool invertAxis = useInvertedAxis();
    QString bbox( QString( ( invertAxis ) ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                    .arg( qgsDoubleToString( rect.xMinimum() ), qgsDoubleToString( rect.yMinimum() ), qgsDoubleToString( rect.xMaximum() ), qgsDoubleToString( rect.yMaximum() ) ) );
    // Some servers like Geomedia need the srsname to be explicitly appended
    // otherwise they are confused and do not interpret it properly
    if ( !mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
    {
      // but it is illegal in WFS 1.0 and some servers definitely not like
      // it. See #15464
      bbox += "," + mShared->srsName();
    }
    query.addQueryItem( QStringLiteral( "BBOX" ), bbox );
  }
  else if ( !mShared->mWFSFilter.isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "FILTER" ), sanitizeFilter( mShared->mWFSFilter ) );
  }
  else if ( !mShared->mServerExpression.isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "FILTER" ), sanitizeFilter( mShared->mServerExpression ) );
  }
  else if ( !mShared->mWFSGeometryTypeFilter.isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "FILTER" ), sanitizeFilter( mShared->mWFSGeometryTypeFilter ) );
  }


  if ( !mShared->mSortBy.isEmpty() && !forHits )
  {
    query.addQueryItem( QStringLiteral( "SORTBY" ), mShared->mSortBy );
  }

  if ( !forHits && !mShared->mURI.outputFormat().isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "OUTPUTFORMAT" ), mShared->mURI.outputFormat() );
  }
  else if ( !forHits && mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    for ( const QString &format : WFS1FORMATS )
    {
      if ( mShared->mCaps.outputFormats.contains( format ) )
      {
        query.addQueryItem( QStringLiteral( "OUTPUTFORMAT" ), format );
        break;
      }
    }
  }

  if ( !namespaces.isEmpty() )
  {
    if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
      query.addQueryItem( QStringLiteral( "NAMESPACES" ), namespaces );
    query.addQueryItem( QStringLiteral( "NAMESPACE" ), namespaces );
  }

  getFeatureUrl.setQuery( query );
  QgsDebugMsgLevel( QStringLiteral( "WFS GetFeature URL: %1" ).arg( getFeatureUrl.toDisplayString() ), 2 );
  return getFeatureUrl;
}

std::pair<QUrl, QByteArray> QgsWFSFeatureDownloaderImpl::buildPostRequest( qint64 startIndex, long long maxFeatures, bool forHits )
{
  QUrl postUrl( mShared->mURI.requestUrl( QStringLiteral( "GetFeature" ), mShared->mHttpMethod ) );

  QString typenames;
  QString namespaces;
  std::tie( typenames, namespaces ) = determineTypeNames();

  const QStringList typeNames = typenames.split( ',' );

  QDomDocument postDocument = createPostDocument();
  QDomElement getFeatureElement = createRootPostElement(
    mShared->mCaps,
    mShared->mWFSVersion,
    postDocument,
    QStringLiteral( "wfs:GetFeature" ),
    typeNames
  );

  if ( forHits )
  {
    getFeatureElement.setAttribute( QStringLiteral( "resultType" ), QStringLiteral( "hits" ) );
  }
  else if ( maxFeatures > 0 )
  {
    if ( mPageSize > 0 )
    {
      getFeatureElement.setAttribute( QStringLiteral( "startIndex" ), QString::number( startIndex ) );
    }

    if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
    {
      getFeatureElement.setAttribute( QStringLiteral( "count" ), QString::number( maxFeatures ) );
    }
    else
    {
      getFeatureElement.setAttribute( QStringLiteral( "maxFeatures" ), QString::number( maxFeatures ) );
    }
  }

  // Add output format if specified
  if ( !forHits && !mShared->mURI.outputFormat().isEmpty() )
  {
    getFeatureElement.setAttribute( QStringLiteral( "outputFormat" ), mShared->mURI.outputFormat() );
  }
  else if ( !forHits && mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    for ( const QString &format : WFS1FORMATS )
    {
      if ( mShared->mCaps.outputFormats.contains( format ) )
      {
        getFeatureElement.setAttribute( QStringLiteral( "outputFormat" ), format );
        break;
      }
    }
  }

  auto buildQueryElement = [&postDocument, forHits, this]( const QString &typeName, const QString &geometryAttribute ) -> QDomElement {
    QDomElement queryElement = postDocument.createElement( QStringLiteral( "wfs:Query" ) );

    const bool useVersion2 = mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) );
    if ( useVersion2 )
    {
      queryElement.setAttribute( QStringLiteral( "typeNames" ), typeName );
    }
    else
    {
      queryElement.setAttribute( QStringLiteral( "typeName" ), typeName );
    }

    // Add srsName if specified
    QString srsName( mShared->srsName() );
    if ( !srsName.isEmpty() && !forHits )
    {
      queryElement.setAttribute( QStringLiteral( "srsName" ), srsName );
    }

    std::vector<QString> filters;
    if ( !mShared->mServerExpression.isEmpty() )
    {
      filters.push_back( mShared->mServerExpression );
    }
    if ( !mShared->mWFSFilter.isEmpty() )
    {
      filters.push_back( mShared->mWFSFilter );
    }
    if ( !mShared->mWFSGeometryTypeFilter.isEmpty() )
    {
      filters.push_back( mShared->mWFSGeometryTypeFilter );
    }

    const QgsRectangle &rect = mShared->currentRect();
    if ( !rect.isNull() )
    {
      QDomDocument bboxDoc;
      QDomElement bboxElement = useVersion2 ? bboxDoc.createElement( QStringLiteral( "fes:BBOX" ) ) : bboxDoc.createElement( QStringLiteral( "ogc:BBOX" ) );

      if ( useVersion2 )
      {
        QDomElement valueRefElement = postDocument.createElement( QStringLiteral( "fes:ValueReference" ) );
        valueRefElement.appendChild( postDocument.createTextNode( geometryAttribute ) );
        bboxElement.appendChild( valueRefElement );
      }
      else
      {
        QDomElement propertyNameElement = postDocument.createElement( QStringLiteral( "ogc:PropertyName" ) );
        propertyNameElement.appendChild( postDocument.createTextNode( geometryAttribute ) );
        bboxElement.appendChild( propertyNameElement );
      }

      const bool invertAxis = useInvertedAxis();
      if ( mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
      {
        QDomElement boxElement = QgsOgcUtils::rectangleToGMLBox( &rect, postDocument, srsName, invertAxis );
        bboxElement.appendChild( boxElement );
      }
      else
      {
        // WFS 1.1, 2.0 use GML Envelope
        const QDomElement envelopeElement = QgsOgcUtils::rectangleToGMLEnvelope( &rect, postDocument, srsName, invertAxis );
        bboxElement.appendChild( envelopeElement );
      }

      if ( !filters.empty() )
      {
        QDomElement filterElement = bboxDoc.createElement( useVersion2 ? QStringLiteral( "fes:Filter" ) : QStringLiteral( "ogc:Filter" ) );
        filterElement.appendChild( bboxElement );
        bboxDoc.appendChild( filterElement );
      }
      else
      {
        bboxDoc.appendChild( bboxElement );
      }

      filters.push_back( bboxDoc.toString() );
    }

    if ( !filters.empty() )
    {
      QDomDocument filterDoc;
      QString filter = sanitizeFilter( mShared->combineWFSFilters( filters ) );
      filter = filter.replace( QLatin1String( "<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">" ), QLatin1String( "<fes:Filter>" ) );
      if ( filterDoc.setContent( filter ) )
      {
        queryElement.appendChild( filterDoc.documentElement() );
      }
    }

    if ( !mShared->mSortBy.isEmpty() && !forHits )
    {
      QDomElement sortByElement = postDocument.createElement( useVersion2 ? QStringLiteral( "fes:SortBy" ) : QStringLiteral( "ogc:SortBy" ) );

      const QStringList sortColumns = mShared->mSortBy.split( QLatin1Char( ',' ) );
      for ( const QString &sortColumn : sortColumns )
      {
        const QStringList sortComponents = sortColumn.split( QLatin1Char( ' ' ) );
        const QString propertyName = sortComponents[0];
        const QString sortOrder = ( sortComponents.size() > 1 && sortComponents[1].startsWith( 'D', Qt::CaseInsensitive ) ) ? QStringLiteral( "DESC" ) : QStringLiteral( "ASC" );

        QDomElement sortPropertyElement = postDocument.createElement( useVersion2 ? QStringLiteral( "fes:SortProperty" ) : QStringLiteral( "ogc:SortProperty" ) );

        QDomElement propertyElement = postDocument.createElement( useVersion2 ? QStringLiteral( "fes:ValueReference" ) : QStringLiteral( "ogc:PropertyName" ) );
        propertyElement.appendChild( postDocument.createTextNode( propertyName ) );
        sortPropertyElement.appendChild( propertyElement );

        QDomElement sortOrderElement = postDocument.createElement( useVersion2 ? QStringLiteral( "fes:SortOrder" ) : QStringLiteral( "ogc:SortOrder" ) );
        sortOrderElement.appendChild( postDocument.createTextNode( sortOrder ) );
        sortPropertyElement.appendChild( sortOrderElement );

        sortByElement.appendChild( sortPropertyElement );
      }

      queryElement.appendChild( sortByElement );
    }

    return queryElement;
  };

  for ( const QString &typeName : typeNames )
  {
    QString geometryAttribute = mShared->mGeometryAttribute;
    if ( mShared->mLayerPropertiesList.size() > 1 )
    {
      geometryAttribute = typeName + "/" + geometryAttribute;
    }

    getFeatureElement.appendChild( buildQueryElement( typenames, geometryAttribute ) );
  }

  return std::make_pair( postUrl, postDocument.toByteArray() );
}

// Called when we get the response of the asynchronous RESULTTYPE=hits request
void QgsWFSFeatureDownloaderImpl::gotHitsResponse()
{
  mNumberMatched = mFeatureHitsAsyncRequest.numberMatched();
  if ( mShared->mMaxFeatures > 0 )
  {
    mNumberMatched = std::min( mNumberMatched, mShared->mMaxFeatures );
  }
  if ( mNumberMatched >= 0 )
  {
    // If the request didn't include any BBOX, then we can update the layer
    // feature count
    if ( mShared->currentRect().isNull() )
      mShared->setFeatureCount( mNumberMatched, true );
  }
}

// Starts an asynchronous RESULTTYPE=hits request
void QgsWFSFeatureDownloaderImpl::startHitsRequest()
{
  // Do a last minute check in case the feature count would have been known in-between
  if ( mShared->isFeatureCountExact() && mShared->currentRect().isNull() )
    mNumberMatched = mShared->getFeatureCount( false );
  if ( mNumberMatched < 0 )
  {
    connect( &mFeatureHitsAsyncRequest, &QgsWFSFeatureHitsAsyncRequest::gotHitsResponse, this, &QgsWFSFeatureDownloaderImpl::gotHitsResponse );
    switch ( mUri.httpMethod() )
    {
      case Qgis::HttpMethod::Get:
      {
        mFeatureHitsAsyncRequest.launchGet( buildURL( 0, -1, true ) );
        break;
      }
      case Qgis::HttpMethod::Post:
      {
        QByteArray data;
        QUrl url;
        std::tie( url, data ) = buildPostRequest( 0, -1, true );
        mFeatureHitsAsyncRequest.launchPost( url, data );
        break;
      }
      case Qgis::HttpMethod::Head:
      case Qgis::HttpMethod::Put:
      case Qgis::HttpMethod::Delete:
        break;
    }
  }
}

void QgsWFSFeatureDownloaderImpl::createProgressTask()
{
  QgsFeatureDownloaderImpl::createProgressTask( mNumberMatched );
  CONNECT_PROGRESS_TASK( QgsWFSFeatureDownloaderImpl );
}

void QgsWFSFeatureDownloaderImpl::run( bool serializeFeatures, long long maxFeatures )
{
  bool success = true;

  QEventLoop loop;
  connect( this, &QgsWFSFeatureDownloaderImpl::doStop, &loop, &QEventLoop::quit );
  connect( this, &QgsWfsRequest::downloadFinished, &loop, &QEventLoop::quit );
  connect( this, &QgsWfsRequest::downloadProgress, &loop, &QEventLoop::quit );

  QTimer timerForHits;

  const bool useProgressDialog = ( !mShared->mHideProgressDialog && maxFeatures != 1 && mShared->supportsFastFeatureCount() );
  if ( useProgressDialog )
  {
    // In case the header of the GetFeature response doesn't contain the total
    // number of features, or we don't get it within 4 seconds, we will issue
    // an explicit RESULTTYPE=hits request.
    timerForHits.setInterval( 4 * 1000 );
    timerForHits.setSingleShot( true );
    timerForHits.start();
    connect( &timerForHits, &QTimer::timeout, this, &QgsWFSFeatureDownloaderImpl::startHitsRequest );
    connect( &mFeatureHitsAsyncRequest, &QgsWfsRequest::downloadFinished, &loop, &QEventLoop::quit );
  }

  bool interrupted = false;
  bool truncatedResponse = false;
  QgsSettings s;
  const int maxRetry = s.value( QStringLiteral( "qgis/defaultTileMaxRetry" ), "3" ).toInt();
  int retryIter = 0;
  int lastValidTotalDownloadedFeatureCount = 0;
  int pagingIter = 1;
  QString gmlIdFirstFeatureFirstIter;
  bool disablePaging = false;
  qint64 maxTotalFeatures = 0;
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
  // Top level loop to do feature paging in WFS 2.0
  while ( true )
  {
    success = true;
    QgsGmlStreamingParser *parser = mShared->createParser();

    if ( maxTotalFeatures > 0 && mTotalDownloadedFeatureCount >= maxTotalFeatures )
    {
      break;
    }
    long long maxFeaturesThisRequest = maxTotalFeatures - mTotalDownloadedFeatureCount;
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

    QUrl url;
    switch ( mShared->mHttpMethod )
    {
      case Qgis::HttpMethod::Get:
      {
        url = buildURL( mTotalDownloadedFeatureCount, maxFeaturesThisRequest, false );

        // Small hack for testing purposes
        if ( retryIter > 0 && url.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
        {
          QUrlQuery query( url );
          query.addQueryItem( QStringLiteral( "RETRY" ), QString::number( retryIter ) );
          url.setQuery( query );
        }

        sendGET( url,
                 QString(), // content-type
                 false,     /* synchronous */
                 true,      /* forceRefresh */
                 false /* cache */ );
        break;
      }

      case Qgis::HttpMethod::Post:
      {
        QByteArray data;
        std::tie( url, data ) = buildPostRequest( mTotalDownloadedFeatureCount, maxFeaturesThisRequest, false );

        // Small hack for testing purposes
        if ( retryIter > 0 && url.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
        {
          QUrlQuery query( url );
          query.addQueryItem( QStringLiteral( "RETRY" ), QString::number( retryIter ) );
          url.setQuery( query );
        }

        sendPOST( url, QStringLiteral( "application/xml; charset=utf-8" ), data, false, { QNetworkReply::RawHeaderPair { "Accept", "application/xml" } } );
        break;
      }

      case Qgis::HttpMethod::Head:
      case Qgis::HttpMethod::Put:
      case Qgis::HttpMethod::Delete:
        // not possible!
        break;
    }

    long long featureCountForThisResponse = 0;
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
        data = mReply->read( 10 * 1024 * 1024 );
        bytesStillAvailableInReply = mReply->bytesAvailable() > 0;
      }
      else
      {
        data = mResponse;
        finished = true;
      }
      // Parse the received chunk of data
      QString gmlProcessErrorMsg;
      if ( !parser->processData( data, finished, gmlProcessErrorMsg ) )
      {
        success = false;
        // Only add an error message if no general networking related error has been
        // previously reported by QgsWfsRequest logic.
        // We indeed make processData() run even if an error has been reported,
        // so that we have a chance to parse XML errors (isException() case below)
        if ( mErrorCode == NoError )
        {
          mErrorMessage = tr( "Error when parsing GetFeature response" ) + " : " + gmlProcessErrorMsg;
          QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
        }
        break;
      }
      else if ( parser->isException() )
      {
        // Only process the exception report if we get the full error response.
        if ( !finished )
          continue;
        success = false;

        // Some GeoServer instances in WFS 2.0 with paging throw an exception
        // e.g. http://ows.region-bretagne.fr/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=rb:etudes&STARTINDEX=0&COUNT=1
        // Disabling paging helps in those cases
        if ( mPageSize > 0 && mTotalDownloadedFeatureCount == 0 && parser->exceptionText().contains( QLatin1String( "Cannot do natural order without a primary key" ) ) )
        {
          QgsDebugError( QStringLiteral( "Got exception %1. Re-trying with paging disabled" ).arg( parser->exceptionText() ) );
          mPageSize = 0;
          mShared->mPageSize = 0;
        }
        // GeoServer doesn't like typenames prefixed by namespace prefix, despite
        // the examples in the WFS 2.0 spec showing that
        else if ( !mRemoveNSPrefix && parser->exceptionText().contains( QLatin1String( "more than one feature type" ) ) )
        {
          QgsDebugError( QStringLiteral( "Got exception %1. Re-trying by removing namespace prefix" ).arg( parser->exceptionText() ) );
          mRemoveNSPrefix = true;
        }

        {
          mErrorMessage = tr( "Server generated an exception in GetFeature response" ) + ": " + parser->exceptionText();
          QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
        }
        break;
      }
      // Test error code only after having let a chance to the parser to process the ExceptionReport
      else if ( mErrorCode != NoError )
      {
        success = false;
        break;
      }

      // Consider if we should display a progress dialog
      // We can only do that if we know how many features will be downloaded
      if ( !mTimer && maxFeatures != 1 && useProgressDialog )
      {
        if ( mNumberMatched < 0 )
        {
          // Some servers, like http://demo.opengeo.org/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=ne:ne_10m_admin_0_countries&STARTINDEX=0&COUNT=50&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=-133.04422094925158149,-188.9997780764296067,126.67820349384365386,188.99999458723010548,
          // return numberMatched="unknown" for all pages, except the last one, where
          // this is (erroneously?) the number of features returned
          if ( parser->numberMatched() > 0 && mTotalDownloadedFeatureCount == 0 )
            mNumberMatched = parser->numberMatched();
          // The number returned can only be used if we aren't in paging mode
          else if ( parser->numberReturned() > 0 && mPageSize == 0 )
            mNumberMatched = parser->numberMatched();
          // We can only use the layer feature count if we don't apply a BBOX
          else if ( mShared->isFeatureCountExact() && mShared->currentRect().isNull() )
            mNumberMatched = mShared->getFeatureCount( false );
          if ( mNumberMatched > 0 && mShared->mMaxFeatures > 0 )
          {
            mNumberMatched = std::min( mNumberMatched, mShared->mMaxFeatures );
          }

          // If we didn't get a valid mNumberMatched, we will possibly issue
          // a explicit RESULTTYPE=hits request 4 second after the beginning of
          // the download
        }

        if ( mNumberMatched > 0 )
        {
          if ( mShared->supportsFastFeatureCount() )
            disconnect( &timerForHits, &QTimer::timeout, this, &QgsWFSFeatureDownloaderImpl::startHitsRequest );

          CREATE_PROGRESS_TASK( QgsWFSFeatureDownloaderImpl );
        }
      }

      QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> featurePtrList = parser->getAndStealReadyFeatures();

      mTotalDownloadedFeatureCount += featurePtrList.size();

      if ( !mStop )
      {
        emit updateProgress( mTotalDownloadedFeatureCount );
      }

      if ( featurePtrList.size() != 0 )
      {
        // Heuristics to try to detect MapServer WFS 1.1 that honours EPSG axis order, but returns
        // EPSG:XXXX srsName and not EPSG urns
        if ( pagingIter == 1 && featureCountForThisResponse == 0 && mShared->mWFSVersion.startsWith( QLatin1String( "1.1" ) ) && parser->srsName().startsWith( QLatin1String( "EPSG:" ) ) && !parser->layerExtent().isNull() && !mShared->mURI.ignoreAxisOrientation() && !mShared->mURI.invertAxisOrientation() )
        {
          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( parser->srsName() );
          if ( crs.isValid() && crs.hasAxisInverted() && !mShared->mCapabilityExtent.contains( parser->layerExtent() ) )
          {
            QgsRectangle invertedRectangle( parser->layerExtent() );
            invertedRectangle.invert();
            if ( mShared->mCapabilityExtent.contains( invertedRectangle ) )
            {
              mShared->mGetFeatureEPSGDotHonoursEPSGOrder = true;
              QgsDebugMsgLevel( QStringLiteral( "Server is likely MapServer. Using mGetFeatureEPSGDotHonoursEPSGOrder mode" ), 2 );
            }
          }
        }

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
              QgsDebugError( QStringLiteral( "Server returns features without fid/gml:id. Computing a fake one using feature attributes" ) );
              mShared->mHasWarnedAboutMissingFeatureId = true;
            }
          }
          if ( pagingIter == 1 && featureCountForThisResponse == 0 )
          {
            gmlIdFirstFeatureFirstIter = gmlId;
          }
          else if ( pagingIter == 2 && featureCountForThisResponse == 0 && gmlIdFirstFeatureFirstIter == gmlId )
          {
            disablePaging = true;
            QgsDebugError( QStringLiteral( "Server does not seem to properly support paging since it returned the same first feature for 2 different page requests. Disabling paging" ) );
          }

          if ( mShared->mGetFeatureEPSGDotHonoursEPSGOrder && f.hasGeometry() )
          {
            QgsGeometry g = f.geometry();
            g.transform( QTransform( 0, 1, 1, 0, 0, 0 ) );
            f.setGeometry( g );
          }

          // If receiving a geometry collection, but expecting a multipoint/...,
          // then try to convert it
          if ( f.hasGeometry() && f.geometry().wkbType() == Qgis::WkbType::GeometryCollection && ( mShared->mWKBType == Qgis::WkbType::MultiPoint || mShared->mWKBType == Qgis::WkbType::MultiLineString || mShared->mWKBType == Qgis::WkbType::MultiPolygon ) )
          {
            Qgis::WkbType singleType = QgsWkbTypes::singleType( mShared->mWKBType );
            const QgsAbstractGeometry *g = f.geometry().constGet();
            if ( const QgsGeometryCollection *gc = qgsgeometry_cast<const QgsGeometryCollection *>( g ) )
            {
              bool allExpectedType = true;
              for ( int i = 0; i < gc->numGeometries(); ++i )
              {
                if ( gc->geometryN( i )->wkbType() != singleType )
                {
                  allExpectedType = false;
                  break;
                }
              }
              if ( allExpectedType )
              {
                std::unique_ptr< QgsGeometryCollection > newGC;
                if ( mShared->mWKBType == Qgis::WkbType::MultiPoint )
                {
                  newGC = std::make_unique< QgsMultiPoint >();
                }
                else if ( mShared->mWKBType == Qgis::WkbType::MultiLineString )
                {
                  newGC = std::make_unique< QgsMultiLineString >();
                }
                else
                {
                  newGC = std::make_unique< QgsMultiPolygon >();
                }
                newGC->reserve( gc->numGeometries() );
                for ( int i = 0; i < gc->numGeometries(); ++i )
                {
                  newGC->addGeometry( gc->geometryN( i )->clone() );
                }
                f.setGeometry( std::move( newGC ) );
              }
            }
          }
          else if ( f.hasGeometry() && !mShared->mWFSGeometryTypeFilter.isEmpty() && QgsWkbTypes::flatType( f.geometry().wkbType() ) != mShared->mWKBType )
          {
            QgsGeometry g = f.geometry();
            g.convertToCurvedMultiType();
            f.setGeometry( g );
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

          featureCountForThisResponse++;
        }
      }

      if ( finished )
      {
        if ( parser->isTruncatedResponse() && mPageSize == 0 )
        {
          // e.g: http://services.cuzk.cz/wfs/inspire-cp-wfs.asp?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=cp:CadastralParcel
          truncatedResponse = true;
        }
        break;
      }
    }

    delete parser;

    if ( mStop )
      break;
    if ( !success )
    {
      if ( ++retryIter <= maxRetry )
      {
        QgsMessageLog::logMessage( tr( "Retrying request %1: %2/%3" ).arg( url.toString() ).arg( retryIter ).arg( maxRetry ), tr( "WFS" ) );
        featureCountForThisResponse = 0;
        mTotalDownloadedFeatureCount = lastValidTotalDownloadedFeatureCount;
        continue;
      }

      break;
    }

    retryIter = 0;
    lastValidTotalDownloadedFeatureCount = mTotalDownloadedFeatureCount;

    if ( mPageSize == 0 )
      break;
    if ( maxFeatures == 1 )
      break;
    // Detect if we are at the last page
    if ( ( mShared->mPageSize > 0 && featureCountForThisResponse < mShared->mPageSize ) || featureCountForThisResponse == 0 )
      break;
    ++pagingIter;
    if ( disablePaging )
    {
      mShared->mPageSize = mPageSize = 0;
      mTotalDownloadedFeatureCount = 0;
      mShared->mPageSize = 0;
      if ( mShared->mMaxFeatures == mShared->mURI.maxNumFeatures() )
      {
        mShared->mMaxFeatures = 0;
      }
    }
  }

  endOfRun( serializeFeatures, success, mTotalDownloadedFeatureCount, truncatedResponse, interrupted, mErrorMessage );

  // explicitly abort here so that mReply is destroyed within the right thread
  // otherwise will deadlock because deleteLayer() will not have a valid thread to post
  abort();
  mFeatureHitsAsyncRequest.abort();
}

QString QgsWFSFeatureDownloaderImpl::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of features failed: %1" ).arg( reason );
}
