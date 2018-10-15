/***************************************************************************
    qgswfsfeatureiterator.cpp
    ---------------------
    begin                : Januar 2013
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
#include "qgswfsconstants.h"
#include "qgswfsfeatureiterator.h"
#include "qgswfsprovider.h"
#include "qgswfsshareddata.h"
#include "qgswfsutils.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsexception.h"
#include "qgsfeedback.h"

#include <algorithm>
#include <QDir>
#include <QProgressDialog>
#include <QTimer>
#include <QStyle>

QgsWFSFeatureHitsAsyncRequest::QgsWFSFeatureHitsAsyncRequest( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
  , mNumberMatched( -1 )
{
  connect( this, &QgsWfsRequest::downloadFinished, this, &QgsWFSFeatureHitsAsyncRequest::hitsReplyFinished );
}

void QgsWFSFeatureHitsAsyncRequest::launch( const QUrl &url )
{
  sendGET( url,
           false, /* synchronous */
           true, /* forceRefresh */
           false /* cache */ );
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
      mNumberMatched = ( gmlParser.numberMatched() >= 0 ) ? gmlParser.numberMatched() :
                       gmlParser.numberReturned();
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

QgsWFSFeatureDownloader::QgsWFSFeatureDownloader( QgsWFSSharedData *shared )
  : QgsWfsRequest( shared->mURI )
  , mShared( shared )
  , mStop( false )
  , mProgressDialogShowImmediately( false )
  , mPageSize( shared->mPageSize )
  , mRemoveNSPrefix( false )
  , mNumberMatched( -1 )
  , mFeatureHitsAsyncRequest( shared->mURI )
  , mTotalDownloadedFeatureCount( 0 )
{
  // Needed because used by a signal
  qRegisterMetaType< QVector<QgsWFSFeatureGmlIdPair> >( "QVector<QgsWFSFeatureGmlIdPair>" );
}

QgsWFSFeatureDownloader::~QgsWFSFeatureDownloader()
{
  stop();

  if ( mProgressDialog )
    mProgressDialog->deleteLater();
  if ( mTimer )
    mTimer->deleteLater();
}

void QgsWFSFeatureDownloader::stop()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsWFSFeatureDownloader::stop()" ), 4 );
  mStop = true;
  emit doStop();
}

void QgsWFSFeatureDownloader::setStopFlag()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsWFSFeatureDownloader::setStopFlag()" ), 4 );
  mStop = true;
}

QgsWFSProgressDialog::QgsWFSProgressDialog( const QString &labelText,
    const QString &cancelButtonText,
    int minimum, int maximum, QWidget *parent )
  : QProgressDialog( labelText, cancelButtonText, minimum, maximum, parent )
{
  mCancel = new QPushButton( cancelButtonText, this );
  setCancelButton( mCancel );
  mHide = new QPushButton( tr( "Hide" ), this );
  connect( mHide, &QAbstractButton::clicked, this, &QgsWFSProgressDialog::hideRequest );
}

void QgsWFSProgressDialog::resizeEvent( QResizeEvent *ev )
{
  QProgressDialog::resizeEvent( ev );
  // Note: this relies heavily on the details of the layout done in QProgressDialogPrivate::layout()
  // Might be a bit fragile depending on QT versions.
  QRect rect = geometry();
  QRect cancelRect = mCancel->geometry();
  QRect hideRect = mHide->geometry();
  int mtb = style()->pixelMetric( QStyle::PM_DefaultTopLevelMargin );
  int mlr = std::min( width() / 10, mtb );
  if ( rect.width() - cancelRect.x() - cancelRect.width() > mlr )
  {
    // Force right alighnment of cancel button
    cancelRect.setX( rect.width() - cancelRect.width() - mlr );
    mCancel->setGeometry( cancelRect );
  }
  mHide->setGeometry( rect.width() - cancelRect.x() - cancelRect.width(),
                      cancelRect.y(), hideRect.width(), cancelRect.height() );
}

void QgsWFSFeatureDownloader::hideProgressDialog()
{
  mShared->mHideProgressDialog = true;
  mProgressDialog->deleteLater();
  mProgressDialog = nullptr;
}

// Called from GUI thread
void QgsWFSFeatureDownloader::createProgressDialog()
{
  Q_ASSERT( qApp->thread() == QThread::currentThread() );

  if ( mStop )
    return;
  Q_ASSERT( !mProgressDialog );

  if ( !mMainWindow )
  {
    const QWidgetList widgets = qApp->topLevelWidgets();
    for ( QWidget *widget : widgets )
    {
      if ( widget->objectName() == QLatin1String( "QgisApp" ) )
      {
        mMainWindow = widget;
        break;
      }
    }
  }

  if ( !mMainWindow )
    return;

  mProgressDialog = new QgsWFSProgressDialog( tr( "Loading features for layer %1" ).arg( mShared->mURI.typeName() ),
      tr( "Abort" ), 0, mNumberMatched, mMainWindow );
  mProgressDialog->setWindowTitle( tr( "QGIS" ) );
  mProgressDialog->setValue( 0 );
  if ( mProgressDialogShowImmediately )
    mProgressDialog->show();

  connect( mProgressDialog, &QProgressDialog::canceled, this, &QgsWFSFeatureDownloader::setStopFlag, Qt::DirectConnection );
  connect( mProgressDialog, &QProgressDialog::canceled, this, &QgsWFSFeatureDownloader::stop );
  connect( mProgressDialog, &QgsWFSProgressDialog::hideRequest, this, &QgsWFSFeatureDownloader::hideProgressDialog );

  // Make sure the progress dialog has not been deleted by another thread
  if ( mProgressDialog )
  {
    connect( this, &QgsWFSFeatureDownloader::updateProgress, mProgressDialog, &QProgressDialog::setValue );
  }
}

QString QgsWFSFeatureDownloader::sanitizeFilter( QString filter )
{
  filter = filter.replace( QLatin1String( "<fes:ValueReference xmlns:fes=\"http://www.opengis.net/fes/2.0\">" ), QLatin1String( "<fes:ValueReference>" ) );
  QString nsPrefix( QgsWFSUtils::nameSpacePrefix( mShared->mURI.typeName() ) );
  if ( mRemoveNSPrefix && !nsPrefix.isEmpty() )
    filter = filter.replace( "<fes:ValueReference>" + nsPrefix + ":", QLatin1String( "<fes:ValueReference>" ) );
  return filter;
}

QUrl QgsWFSFeatureDownloader::buildURL( qint64 startIndex, int maxFeatures, bool forHits )
{
  QUrl getFeatureUrl( mShared->mURI.requestUrl( QStringLiteral( "GetFeature" ) ) );
  getFeatureUrl.addQueryItem( QStringLiteral( "VERSION" ),  mShared->mWFSVersion );

  QString typenames;
  QString namespaces;
  if ( mShared->mLayerPropertiesList.isEmpty() )
  {
    typenames = mShared->mURI.typeName();

    // Add NAMESPACES parameter for server that declare a namespace in the FeatureType of GetCapabilities response
    // See https://issues.qgis.org/issues/14685
    Q_FOREACH ( const QgsWfsCapabilities::FeatureType &f, mShared->mCaps.featureTypes )
    {
      if ( f.name == typenames )
      {
        if ( !f.nameSpace.isEmpty() && f.name.contains( ':' ) )
        {
          QString prefixOfTypename = f.name.section( ':', 0, 0 );
          namespaces = "xmlns(" + prefixOfTypename + "," + f.nameSpace + ")";
        }
        break;
      }
    }

  }
  else
  {
    Q_FOREACH ( const QgsOgcUtils::LayerProperties layerProperties, mShared->mLayerPropertiesList )
    {
      if ( !typenames.isEmpty() )
        typenames += QLatin1String( "," );
      typenames += layerProperties.mName;
    }
  }
  if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
    getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAMES" ),  typenames );
  else
    getFeatureUrl.addQueryItem( QStringLiteral( "TYPENAME" ),  typenames );
  if ( forHits )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "RESULTTYPE" ), QStringLiteral( "hits" ) );
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
      getFeatureUrl.addQueryItem( QStringLiteral( "STARTINDEX" ), QString::number( startIndex ) );
    }
    if ( mShared->mWFSVersion.startsWith( QLatin1String( "2.0" ) ) )
      getFeatureUrl.addQueryItem( QStringLiteral( "COUNT" ), QString::number( maxFeatures ) );
    else
      getFeatureUrl.addQueryItem( QStringLiteral( "MAXFEATURES" ), QString::number( maxFeatures ) );
  }
  QString srsName( mShared->srsName() );
  if ( !srsName.isEmpty() && !forHits )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "SRSNAME" ), srsName );
  }

  // In case we must issue a BBOX and we have a filter, we must combine
  // both as a single filter, as both BBOX and FILTER aren't supported together
  if ( !mShared->mRect.isNull() && !mShared->mWFSFilter.isEmpty() )
  {
    double minx = mShared->mRect.xMinimum();
    double miny = mShared->mRect.yMinimum();
    double maxx = mShared->mRect.xMaximum();
    double maxy = mShared->mRect.yMaximum();
    QString filterBbox( QStringLiteral( "intersects_bbox($geometry, geomFromWKT('LINESTRING(%1 %2,%3 %4)'))" ).
                        arg( minx ).arg( miny ).arg( maxx ).arg( maxy ) );
    QgsExpression bboxExp( filterBbox );
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
    QDomDocument doc;
    QString geometryAttribute( mShared->mGeometryAttribute );
    if ( mShared->mLayerPropertiesList.size() > 1 )
      geometryAttribute = mShared->mURI.typeName() + "/" + geometryAttribute;
    QDomElement bboxElem = QgsOgcUtils::expressionToOgcFilter( bboxExp, doc,
                           gmlVersion, filterVersion, geometryAttribute, mShared->srsName(),
                           honourAxisOrientation, mShared->mURI.invertAxisOrientation() );
    doc.appendChild( bboxElem );
    QDomNode bboxNode = bboxElem.firstChildElement();
    bboxNode = bboxElem.removeChild( bboxNode );

    QDomDocument filterDoc;
    ( void )filterDoc.setContent( mShared->mWFSFilter, true );
    QDomNode filterNode = filterDoc.firstChildElement().firstChildElement();
    filterNode = filterDoc.firstChildElement().removeChild( filterNode );

    QDomElement andElem = doc.createElement( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes:And" : "ogc:And" );
    andElem.appendChild( bboxNode );
    andElem.appendChild( filterNode );
    doc.firstChildElement().appendChild( andElem );

    getFeatureUrl.addQueryItem( QStringLiteral( "FILTER" ), sanitizeFilter( doc.toString() ) );
  }
  else if ( !mShared->mRect.isNull() )
  {
    bool invertAxis = false;
    if ( !mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) &&
         !mShared->mURI.ignoreAxisOrientation() )
    {
      // This is a bit nasty, but if the server reports OGC::CRS84
      // mSourceCRS will report hasAxisInverted() == false, but srsName()
      // will be urn:ogc:def:crs:EPSG::4326, so axis inversion is needed...
      if ( mShared->srsName() == QLatin1String( "urn:ogc:def:crs:EPSG::4326" ) )
      {
        invertAxis = true;
      }
      else if ( mShared->mSourceCRS.hasAxisInverted() )
      {
        invertAxis = true;
      }
    }
    if ( mShared->mURI.invertAxisOrientation() )
    {
      invertAxis = !invertAxis;
    }
    QString bbox( QString( ( invertAxis ) ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                  .arg( qgsDoubleToString( mShared->mRect.xMinimum() ),
                        qgsDoubleToString( mShared->mRect.yMinimum() ),
                        qgsDoubleToString( mShared->mRect.xMaximum() ),
                        qgsDoubleToString( mShared->mRect.yMaximum() ) ) );
    // Some servers like Geomedia need the srsname to be explicitly appended
    // otherwise they are confused and do not interpret it properly
    if ( !mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
    {
      // but it is illegal in WFS 1.0 and some servers definitely not like
      // it. See #15464
      bbox += "," + mShared->srsName();
    }
    getFeatureUrl.addQueryItem( QStringLiteral( "BBOX" ),  bbox );
  }
  else if ( !mShared->mWFSFilter.isEmpty() )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "FILTER" ), sanitizeFilter( mShared->mWFSFilter ) );
  }

  if ( !mShared->mSortBy.isEmpty() && !forHits )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "SORTBY" ), mShared->mSortBy );
  }

  if ( !forHits && !mShared->mURI.outputFormat().isEmpty() )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "OUTPUTFORMAT" ), mShared->mURI.outputFormat() );
  }
  else if ( !forHits && mShared->mWFSVersion.startsWith( QLatin1String( "1.0" ) ) )
  {
    QStringList list;
    list << QStringLiteral( "text/xml; subtype=gml/3.2.1" );
    list << QStringLiteral( "application/gml+xml; version=3.2" );
    list << QStringLiteral( "text/xml; subtype=gml/3.1.1" );
    list << QStringLiteral( "application/gml+xml; version=3.1" );
    list << QStringLiteral( "text/xml; subtype=gml/3.0.1" );
    list << QStringLiteral( "application/gml+xml; version=3.0" );
    list << QStringLiteral( "GML3" );
    Q_FOREACH ( const QString &format, list )
    {
      if ( mShared->mCaps.outputFormats.contains( format ) )
      {
        getFeatureUrl.addQueryItem( QStringLiteral( "OUTPUTFORMAT" ),
                                    format );
        break;
      }
    }
  }

  if ( !namespaces.isEmpty() )
  {
    getFeatureUrl.addQueryItem( QStringLiteral( "NAMESPACES" ),
                                namespaces );
  }

  QgsDebugMsgLevel( QStringLiteral( "WFS GetFeature URL: %1" ).arg( getFeatureUrl.toDisplayString( ) ), 2 );

  return getFeatureUrl;
}

// Called when we get the response of the asynchronous RESULTTYPE=hits request
void QgsWFSFeatureDownloader::gotHitsResponse()
{
  mNumberMatched = mFeatureHitsAsyncRequest.numberMatched();
  if ( mShared->mMaxFeatures > 0 )
  {
    mNumberMatched = std::min( mNumberMatched, mShared->mMaxFeatures );
  }
  if ( mNumberMatched >= 0 )
  {
    if ( mTotalDownloadedFeatureCount == 0 )
    {
      // We get at this point after the 4 second delay to emit the hits request
      // and the delay to get its response. If we don't still have downloaded
      // any feature at this point, it is high time to give some visual feedback
      mProgressDialogShowImmediately = true;
    }
    // If the request didn't include any BBOX, then we can update the layer
    // feature count
    if ( mShared->mRect.isNull() )
      mShared->setFeatureCount( mNumberMatched );
  }
}

// Starts an asynchronous RESULTTYPE=hits request
void QgsWFSFeatureDownloader::startHitsRequest()
{
  // Do a last minute check in case the feature count would have been known in-between
  if ( mShared->isFeatureCountExact() && mShared->mRect.isNull() )
    mNumberMatched = mShared->getFeatureCount( false );
  if ( mNumberMatched < 0 )
  {
    connect( &mFeatureHitsAsyncRequest, &QgsWFSFeatureHitsAsyncRequest::gotHitsResponse, this, &QgsWFSFeatureDownloader::gotHitsResponse );
    mFeatureHitsAsyncRequest.launch( buildURL( 0, -1, true ) );
  }
}

void QgsWFSFeatureDownloader::run( bool serializeFeatures, int maxFeatures )
{
  bool success = true;

  QEventLoop loop;
  connect( this, &QgsWFSFeatureDownloader::doStop, &loop, &QEventLoop::quit );
  connect( this, &QgsWfsRequest::downloadFinished, &loop, &QEventLoop::quit );
  connect( this, &QgsWfsRequest::downloadProgress, &loop, &QEventLoop::quit );

  QTimer timerForHits;

  if ( !mShared->mHideProgressDialog && maxFeatures != 1 && mShared->supportsHits() )
  {
    mUseProgressDialog = true;
  }

  if ( mUseProgressDialog )
  {
    // In case the header of the GetFeature response doesn't contain the total
    // number of features, or we don't get it within 4 seconds, we will issue
    // an explicit RESULTTYPE=hits request.
    timerForHits.setInterval( 4 * 1000 );
    timerForHits.setSingleShot( true );
    timerForHits.start();
    connect( &timerForHits, &QTimer::timeout, this, &QgsWFSFeatureDownloader::startHitsRequest );
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
    int maxFeaturesThisRequest = static_cast<int>(
                                   std::min( maxTotalFeatures - mTotalDownloadedFeatureCount,
                                       static_cast<qint64>( std::numeric_limits<int>::max() ) ) );
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
    QUrl url( buildURL( mTotalDownloadedFeatureCount,
                        maxFeaturesThisRequest, false ) );

    // Small hack for testing purposes
    if ( retryIter > 0 && url.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
    {
      url.addQueryItem( QStringLiteral( "RETRY" ), QString::number( retryIter ) );
    }

    sendGET( url,
             false, /* synchronous */
             true, /* forceRefresh */
             false /* cache */ );

    int featureCountForThisResponse = 0;
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
      if ( mErrorCode != NoError )
      {
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
        mErrorMessage = tr( "Error when parsing GetFeature response" ) + " : " + gmlProcessErrorMsg;
        QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
        break;
      }
      if ( parser->isException() && finished )
      {
        success = false;

        // Some GeoServer instances in WFS 2.0 with paging throw an exception
        // e.g. http://ows.region-bretagne.fr/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=rb:etudes&STARTINDEX=0&COUNT=1
        // Disabling paging helps in those cases
        if ( mPageSize > 0 && mTotalDownloadedFeatureCount == 0 &&
             parser->exceptionText().contains( QLatin1String( "Cannot do natural order without a primary key" ) ) )
        {
          QgsDebugMsg( QStringLiteral( "Got exception %1. Re-trying with paging disabled" ).arg( parser->exceptionText() ) );
          mPageSize = 0;
        }
        // GeoServer doesn't like typenames prefixed by namespace prefix, despite
        // the examples in the WFS 2.0 spec showing that
        else if ( !mRemoveNSPrefix && parser->exceptionText().contains( QLatin1String( "more than one feature type" ) ) )
        {
          QgsDebugMsg( QStringLiteral( "Got exception %1. Re-trying by removing namespace prefix" ).arg( parser->exceptionText() ) );
          mRemoveNSPrefix = true;
        }

        {
          mErrorMessage = tr( "Server generated an exception in GetFeature response" ) + ": " + parser->exceptionText();
          QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
        }
        break;
      }

      // Consider if we should display a progress dialog
      // We can only do that if we know how many features will be downloaded
      if ( !mTimer && maxFeatures != 1 && mUseProgressDialog )
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
          else if ( mShared->isFeatureCountExact() && mShared->mRect.isNull() )
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
          if ( mShared->supportsHits() )
            disconnect( &timerForHits, &QTimer::timeout, this, &QgsWFSFeatureDownloader::startHitsRequest );

          // This is a bit tricky. We want the createProgressDialog()
          // method to be run into the GUI thread
          mTimer = new QTimer();
          mTimer->setSingleShot( true );

          // Direct connection, since we want createProgressDialog()
          // to be invoked from the same thread as timer, and not in the
          // thread of this
          connect( mTimer, &QTimer::timeout, this, &QgsWFSFeatureDownloader::createProgressDialog, Qt::DirectConnection );

          mTimer->moveToThread( qApp->thread() );
          QMetaObject::invokeMethod( mTimer, "start", Qt::QueuedConnection );
        }
      }

      QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> featurePtrList =
        parser->getAndStealReadyFeatures();

      mTotalDownloadedFeatureCount += featurePtrList.size();

      if ( !mStop )
      {
        emit updateProgress( mTotalDownloadedFeatureCount );
      }

      if ( featurePtrList.size() != 0 )
      {
        // Heuristics to try to detect MapServer WFS 1.1 that honours EPSG axis order, but returns
        // EPSG:XXXX srsName and not EPSG urns
        if ( pagingIter == 1 && featureCountForThisResponse == 0 &&
             mShared->mWFSVersion.startsWith( QLatin1String( "1.1" ) ) &&
             parser->srsName().startsWith( QLatin1String( "EPSG:" ) ) &&
             !parser->layerExtent().isNull() &&
             !mShared->mURI.ignoreAxisOrientation() &&
             !mShared->mURI.invertAxisOrientation() )
        {
          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( parser->srsName() );
          if ( crs.isValid() && crs.hasAxisInverted() &&
               !mShared->mCapabilityExtent.contains( parser->layerExtent() ) )
          {
            QgsRectangle invertedRectangle( parser->layerExtent() );
            invertedRectangle.invert();
            if ( mShared->mCapabilityExtent.contains( invertedRectangle ) )
            {
              mShared->mGetFeatureEPSGDotHonoursEPSGOrder = true;
              QgsDebugMsg( QStringLiteral( "Server is likely MapServer. Using mGetFeatureEPSGDotHonoursEPSGOrder mode" ) );
            }
          }
        }

        QVector<QgsWFSFeatureGmlIdPair> featureList;
        for ( int i = 0; i < featurePtrList.size(); i++ )
        {
          QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair &featPair = featurePtrList[i];
          QgsFeature &f = *( featPair.first );
          QString gmlId( featPair.second );
          if ( gmlId.isEmpty() )
          {
            // Should normally not happen on sane WFS sources, but can happen with
            // Geomedia
            gmlId = QgsWFSUtils::getMD5( f );
            if ( !mShared->mHasWarnedAboutMissingFeatureId )
            {
              QgsDebugMsg( QStringLiteral( "Server returns features without fid/gml:id. Computing a fake one using feature attributes" ) );
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
            QgsDebugMsg( QStringLiteral( "Server does not seem to properly support paging since it returned the same first feature for 2 different page requests. Disabling paging" ) );
          }

          if ( mShared->mGetFeatureEPSGDotHonoursEPSGOrder && f.hasGeometry() )
          {
            QgsGeometry g = f.geometry();
            g.transform( QTransform( 0, 1, 1, 0, 0, 0 ) );
            f.setGeometry( g );
          }

          // If receiving a geometry collection, but expecting a multipoint/...,
          // then try to convert it
          if ( f.hasGeometry() &&
               f.geometry().wkbType() == QgsWkbTypes::GeometryCollection &&
               ( mShared->mWKBType == QgsWkbTypes::MultiPoint ||
                 mShared->mWKBType == QgsWkbTypes::MultiLineString ||
                 mShared->mWKBType == QgsWkbTypes::MultiPolygon ) )
          {
            QgsWkbTypes::Type singleType = QgsWkbTypes::singleType( mShared->mWKBType );
            auto g = f.geometry().constGet();
            auto gc = qgsgeometry_cast<QgsGeometryCollection *>( g );
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
              QgsGeometryCollection *newGC;
              if ( mShared->mWKBType == QgsWkbTypes::MultiPoint )
              {
                newGC =  new QgsMultiPoint();
              }
              else if ( mShared->mWKBType == QgsWkbTypes::MultiLineString )
              {
                newGC = new QgsMultiLineString();
              }
              else
              {
                newGC = new QgsMultiPolygon();
              }
              for ( int i = 0; i < gc->numGeometries(); ++i )
              {
                newGC->addGeometry( gc->geometryN( i )->clone() );
              }
              f.setGeometry( QgsGeometry( newGC ) );
            }
          }

          featureList.push_back( QgsWFSFeatureGmlIdPair( f, gmlId ) );
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
              emit featureReceived( featureList );
              emit featureReceived( featureList.size() );
            }

            featureList.clear();
          }

          featureCountForThisResponse ++;
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
    ++ pagingIter;
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

  mStop = true;

  if ( serializeFeatures )
    mShared->endOfDownload( success, mTotalDownloadedFeatureCount, truncatedResponse, interrupted, mErrorMessage );

  // We must emit the signal *AFTER* the previous call to mShared->endOfDownload()
  // to avoid issues with iterators that would start just now, wouldn't detect
  // that the downloader has finished, would register to itself, but would never
  // receive the endOfDownload signal. This is not just a theoretical problem.
  // If you switch both calls, it happens quite easily in Release mode with the
  // test suite.
  emit endOfDownload( success );

  if ( mProgressDialog )
  {
    mProgressDialog->deleteLater();
    mProgressDialog = nullptr;
  }
  if ( mTimer )
  {
    mTimer->deleteLater();
    mTimer = nullptr;
  }
  // explicitly abort here so that mReply is destroyed within the right thread
  // otherwise will deadlock because deleteLayer() will not have a valid thread to post
  abort();
  mFeatureHitsAsyncRequest.abort();
}

QString QgsWFSFeatureDownloader::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of features failed: %1" ).arg( reason );
}

QgsWFSThreadedFeatureDownloader::QgsWFSThreadedFeatureDownloader( QgsWFSSharedData *shared )
  : mShared( shared )
{
}

QgsWFSThreadedFeatureDownloader::~QgsWFSThreadedFeatureDownloader()
{
  stop();
}

void QgsWFSThreadedFeatureDownloader::stop()
{
  if ( mDownloader )
  {
    mDownloader->stop();
    wait();
    delete mDownloader;
    mDownloader = nullptr;
  }
}

void QgsWFSThreadedFeatureDownloader::startAndWait()
{
  start();

  QMutexLocker locker( &mWaitMutex );
  while ( !mDownloader )
  {
    mWaitCond.wait( &mWaitMutex );
  }
}

void QgsWFSThreadedFeatureDownloader::run()
{
  // We need to construct it in the run() method (i.e. in the new thread)
  mDownloader = new QgsWFSFeatureDownloader( mShared );
  {
    QMutexLocker locker( &mWaitMutex );
    mWaitCond.wakeOne();
  }
  mDownloader->run( true, /* serialize features */
                    mShared->requestLimit() /* user max features */ );
}

// -------------------------

QgsWFSFeatureIterator::QgsWFSFeatureIterator( QgsWFSFeatureSource *source,
    bool ownSource,
    const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsWFSFeatureSource>( source, ownSource, request )
  , mShared( source->mShared )
  , mDownloadFinished( false )
  , mCounter( 0 )
  , mWriteTransferThreshold( 1024 * 1024 )
  , mFetchGeometry( false )
{
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }

  // Configurable for the purpose of unit tests
  QString threshold( getenv( "QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD" ) );
  if ( !threshold.isEmpty() )
    mWriteTransferThreshold = threshold.toInt();

  // Particular case: if a single feature is requested by Fid and we already
  // have it in cache, no need to interrupt any download
  if ( mShared->mCacheDataProvider &&
       mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    QgsFeatureRequest requestCache = buildRequestCache( -1 );
    QgsFeature f;
    if ( mShared->mCacheDataProvider->getFeatures( requestCache ).nextFeature( f ) )
    {
      mCacheIterator = mShared->mCacheDataProvider->getFeatures( requestCache );
      mDownloadFinished = true;
      return;
    }
  }

  int genCounter = ( mShared->mURI.isRestrictedToRequestBBOX() && !mFilterRect.isNull() ) ?
                   mShared->registerToCache( this, static_cast<int>( mRequest.limit() ), mFilterRect ) :
                   mShared->registerToCache( this, static_cast<int>( mRequest.limit() ) );
  mDownloadFinished = genCounter < 0;
  if ( !mShared->mCacheDataProvider )
    return;

  QgsDebugMsgLevel( QStringLiteral( "QgsWFSFeatureIterator::constructor(): genCounter=%1 " ).arg( genCounter ), 4 );

  mCacheIterator = mShared->mCacheDataProvider->getFeatures( buildRequestCache( genCounter ) );
}

QgsFeatureRequest QgsWFSFeatureIterator::buildRequestCache( int genCounter )
{
  QgsFeatureRequest requestCache;
  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid ||
       mRequest.filterType() == QgsFeatureRequest::FilterFids )
    requestCache = mRequest;
  else
  {
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
    {
      requestCache.setFilterExpression( mRequest.filterExpression()->expression() );
    }
    if ( genCounter >= 0 )
    {
      requestCache.combineFilterExpression( QString( QgsWFSConstants::FIELD_GEN_COUNTER + " <= %1" ).arg( genCounter ) );
    }
  }

  requestCache.setFilterRect( mFilterRect );

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) ||
       ( mRequest.filterType() == QgsFeatureRequest::FilterExpression && mRequest.filterExpression()->needsGeometry() ) )
  {
    mFetchGeometry = true;
  }

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    QgsFields dataProviderFields = mShared->mCacheDataProvider->fields();
    QgsAttributeList cacheSubSet;
    Q_FOREACH ( int i, mRequest.subsetOfAttributes() )
    {
      int idx = dataProviderFields.indexFromName( mShared->mFields.at( i ).name() );
      if ( idx >= 0 )
        cacheSubSet.append( idx );
      idx = mShared->mFields.indexFromName( mShared->mFields.at( i ).name() );
      if ( idx >= 0 )
        mSubSetAttributes.append( idx );
    }

    // ensure that all attributes required for expression filter are being fetched
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
    {
      Q_FOREACH ( const QString &field, mRequest.filterExpression()->referencedColumns() )
      {
        int idx = dataProviderFields.indexFromName( field );
        if ( idx >= 0 && !cacheSubSet.contains( idx ) )
          cacheSubSet.append( idx );
        idx = mShared->mFields.indexFromName( field );
        if ( idx >= 0  && !mSubSetAttributes.contains( idx ) )
          mSubSetAttributes.append( idx );
      }
    }

    // also need attributes required by order by
    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && !mRequest.orderBy().isEmpty() )
    {
      Q_FOREACH ( const QString &attr, mRequest.orderBy().usedAttributes() )
      {
        int idx = dataProviderFields.indexFromName( attr );
        if ( idx >= 0 && !cacheSubSet.contains( idx ) )
          cacheSubSet.append( idx );

        idx = mShared->mFields.indexFromName( attr );
        if ( idx >= 0  && !mSubSetAttributes.contains( idx ) )
          mSubSetAttributes.append( idx );
      }
    }

    if ( mFetchGeometry )
    {
      int hexwkbGeomIdx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_HEXWKB_GEOM );
      Q_ASSERT( hexwkbGeomIdx >= 0 );
      cacheSubSet.append( hexwkbGeomIdx );
    }
    requestCache.setSubsetOfAttributes( cacheSubSet );
  }

  return requestCache;
}

QgsWFSFeatureIterator::~QgsWFSFeatureIterator()
{
  QgsDebugMsgLevel( QStringLiteral( "qgsWFSFeatureIterator::~QgsWFSFeatureIterator()" ), 4 );

  close();

  QMutexLocker locker( &mMutex );
  if ( mWriterStream )
  {
    delete mWriterStream;
    delete mWriterFile;
    if ( !mWriterFilename.isEmpty() )
      QFile::remove( mWriterFilename );
  }
  if ( mReaderStream )
  {
    delete mReaderStream;
    delete mReaderFile;
    if ( !mReaderFilename.isEmpty() )
      QFile::remove( mReaderFilename );
  }
}

void QgsWFSFeatureIterator::connectSignals( QgsWFSFeatureDownloader *downloader )
{
  // We want to run the slot for that signal in the same thread as the sender
  // so as to avoid the list of features to accumulate without control in
  // memory
  connect( downloader, static_cast<void ( QgsWFSFeatureDownloader::* )( QVector<QgsWFSFeatureGmlIdPair> )>( &QgsWFSFeatureDownloader::featureReceived ),
           this, &QgsWFSFeatureIterator::featureReceivedSynchronous, Qt::DirectConnection );

  connect( downloader, static_cast<void ( QgsWFSFeatureDownloader::* )( int )>( &QgsWFSFeatureDownloader::featureReceived ),
           this, &QgsWFSFeatureIterator::featureReceived );

  connect( downloader, &QgsWFSFeatureDownloader::endOfDownload,
           this, &QgsWFSFeatureIterator::endOfDownload );
}

void QgsWFSFeatureIterator::endOfDownload( bool )
{
  mDownloadFinished = true;
  if ( mLoop )
    mLoop->quit();
}

void QgsWFSFeatureIterator::setInterruptionChecker( QgsFeedback *interruptionChecker )
{
  mInterruptionChecker = interruptionChecker;
}

// This method will serialize the receive feature list, first into memory, and
// if it goes above a given threshold, on disk
void QgsWFSFeatureIterator::featureReceivedSynchronous( const QVector<QgsWFSFeatureGmlIdPair> &list )
{
  QgsDebugMsgLevel( QStringLiteral( "QgsWFSFeatureIterator::featureReceivedSynchronous %1 features" ).arg( list.size() ), 4 );
  QMutexLocker locker( &mMutex );
  if ( !mWriterStream )
  {
    mWriterStream = new QDataStream( &mWriterByteArray, QIODevice::WriteOnly );
  }
  Q_FOREACH ( const QgsWFSFeatureGmlIdPair &pair, list )
  {
    *mWriterStream << pair.first;
  }
  if ( !mWriterFile && mWriterByteArray.size() > mWriteTransferThreshold )
  {
    QString thisStr;
    thisStr.sprintf( "%p", this );
    ++ mCounter;
    mWriterFilename = QDir( QgsWFSUtils::acquireCacheDirectory() ).filePath( QStringLiteral( "iterator_%1_%2.bin" ).arg( thisStr ).arg( mCounter ) );
    QgsDebugMsgLevel( QStringLiteral( "Transferring feature iterator cache to %1" ).arg( mWriterFilename ), 4 );
    mWriterFile = new QFile( mWriterFilename );
    if ( !mWriterFile->open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot open %1 for writing" ).arg( mWriterFilename ) );
      delete mWriterFile;
      mWriterFile = nullptr;
      return;
    }
    mWriterFile->write( mWriterByteArray );
    mWriterByteArray.clear();
    mWriterStream->setDevice( mWriterFile );
  }
}

// This will invoked asynchronously, in the thread of QgsWFSFeatureIterator
// hence it is safe to quite the loop
void QgsWFSFeatureIterator::featureReceived( int /*featureCount*/ )
{
  //QgsDebugMsg( QStringLiteral("QgsWFSFeatureIterator::featureReceived %1 features").arg(featureCount) );
  if ( mLoop )
    mLoop->quit();
}

void QgsWFSFeatureIterator::checkInterruption()
{
  //QgsDebugMsg("QgsWFSFeatureIterator::checkInterruption()");

  if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
  {
    mDownloadFinished = true;
    if ( mLoop )
      mLoop->quit();
  }
}

void QgsWFSFeatureIterator::timeout()
{
  mTimeoutOccurred = true;
  mDownloadFinished = true;
  if ( mLoop )
    mLoop->quit();
}

bool QgsWFSFeatureIterator::fetchFeature( QgsFeature &f )
{
  f.setValid( false );

  if ( mClosed )
    return false;

  // First step is to iterate over the on-disk cache
  QgsFeature cachedFeature;
  while ( mCacheIterator.nextFeature( cachedFeature ) )
  {
    if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
      return false;

    if ( mTimeoutOccurred )
      return false;

    //QgsDebugMsg(QString("QgsWFSSharedData::fetchFeature() : mCacheIterator.nextFeature(cachedFeature)") );

    if ( !mShared->mGeometryAttribute.isEmpty() && mFetchGeometry )
    {
      int idx = cachedFeature.fields().indexFromName( QgsWFSConstants::FIELD_HEXWKB_GEOM );
      Q_ASSERT( idx >= 0 );

      const QVariant &v = cachedFeature.attributes().value( idx );
      if ( !v.isNull() && v.type() == QVariant::String )
      {
        QByteArray wkbGeom( QByteArray::fromHex( v.toString().toLatin1() ) );
        QgsGeometry g;
        unsigned char *wkbClone = new unsigned char[wkbGeom.size()];
        memcpy( wkbClone, wkbGeom.data(), wkbGeom.size() );
        try
        {
          g.fromWkb( wkbClone, wkbGeom.size() );
          cachedFeature.setGeometry( g );
        }
        catch ( const QgsWkbException & )
        {
          QgsDebugMsg( QStringLiteral( "Invalid WKB for cached feature %1" ).arg( cachedFeature.id() ) );
          delete[] wkbClone;
          cachedFeature.clearGeometry();
        }
      }
      else
      {
        cachedFeature.clearGeometry();
      }
    }
    else
    {
      cachedFeature.clearGeometry();
    }

    QgsGeometry constGeom = cachedFeature.geometry();
    if ( !mFilterRect.isNull() &&
         ( constGeom.isNull() || !constGeom.intersects( mFilterRect ) ) )
    {
      continue;
    }

    copyFeature( cachedFeature, f );
    geometryToDestinationCrs( f, mTransform );
    return true;
  }

  // Second step is to wait for features to be notified by the downloader
  while ( true )
  {
    // Initialize a reader stream if there's a writer stream available
    if ( !mReaderStream )
    {
      {
        QMutexLocker locker( &mMutex );
        if ( mWriterStream )
        {
          // Transfer writer variables to the reader
          delete mWriterStream;
          delete mWriterFile;
          mWriterStream = nullptr;
          mWriterFile = nullptr;
          mReaderByteArray = mWriterByteArray;
          mWriterByteArray.clear();
          mReaderFilename = mWriterFilename;
          mWriterFilename.clear();
        }
      }
      // Instantiates the reader stream from memory buffer if not empty
      if ( !mReaderByteArray.isEmpty() )
      {
        mReaderStream = new QDataStream( &mReaderByteArray, QIODevice::ReadOnly );
      }
      // Otherwise from the on-disk file
      else if ( !mReaderFilename.isEmpty() )
      {
        mReaderFile = new QFile( mReaderFilename );
        if ( !mReaderFile->open( QIODevice::ReadOnly ) )
        {
          QgsDebugMsg( QStringLiteral( "Cannot open %1" ).arg( mReaderFilename ) );
          delete mReaderFile;
          mReaderFile = nullptr;
          return false;
        }
        mReaderStream = new QDataStream( mReaderFile );
      }
    }

    // Read from the stream
    if ( mReaderStream )
    {
      while ( !mReaderStream->atEnd() )
      {
        if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
          return false;

        QgsFeature feat;
        ( *mReaderStream ) >> feat;
        // We need to re-attach fields explicitly
        feat.setFields( mShared->mFields );

        if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
        {
          if ( feat.id() != mRequest.filterFid() )
          {
            continue;
          }
        }
        else if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
        {
          if ( !mRequest.filterFids().contains( feat.id() ) )
          {
            continue;
          }
        }

        QgsGeometry constGeom = feat.geometry();
        if ( !mFilterRect.isNull() &&
             ( constGeom.isNull() || !constGeom.intersects( mFilterRect ) ) )
        {
          continue;
        }

        copyFeature( feat, f );
        return true;
      }

      // When the stream is finished, cleanup
      delete mReaderStream;
      mReaderStream = nullptr;
      delete mReaderFile;
      mReaderFile = nullptr;
      mReaderByteArray.clear();
      if ( !mReaderFilename.isEmpty() )
      {
        QFile::remove( mReaderFilename );
        mReaderFilename.clear();
      }

      // And try again to check if there's a new output stream to read from
      continue;
    }

    if ( mDownloadFinished )
      return false;
    if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
      return false;

    //QgsDebugMsg("fetchFeature before loop");
    QEventLoop loop;
    mLoop = &loop;
    QTimer timer( this );
    timer.start( 50 );
    QTimer requestTimeout( this );
    if ( mRequest.timeout() > 0 )
    {
      connect( &requestTimeout, &QTimer::timeout, this, &QgsWFSFeatureIterator::timeout );
      requestTimeout.start( mRequest.timeout() );
    }
    if ( mInterruptionChecker )
      connect( &timer, &QTimer::timeout, this, &QgsWFSFeatureIterator::checkInterruption );
    loop.exec( QEventLoop::ExcludeUserInputEvents );
    mLoop = nullptr;
    //QgsDebugMsg("fetchFeature after loop");
  }
}

bool QgsWFSFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( mReaderStream )
  {
    delete mReaderStream;
    mReaderStream = nullptr;
    delete mReaderFile;
    mReaderFile = nullptr;
    mReaderByteArray.clear();
    if ( !mReaderFilename.isEmpty() )
    {
      QFile::remove( mReaderFilename );
      mReaderFilename.clear();
    }
  }

  QgsFeatureRequest requestCache;
  int genCounter = mShared->getUpdatedCounter();
  if ( genCounter >= 0 )
    requestCache.setFilterExpression( QString( QgsWFSConstants::FIELD_GEN_COUNTER + " <= %1" ).arg( genCounter ) );
  else
    mDownloadFinished = true;
  if ( mShared->mCacheDataProvider )
    mCacheIterator = mShared->mCacheDataProvider->getFeatures( requestCache );
  return true;
}

bool QgsWFSFeatureIterator::close()
{
  if ( mClosed )
    return false;
  QgsDebugMsgLevel( QStringLiteral( "qgsWFSFeatureIterator::close()" ), 4 );

  iteratorClosed();

  mClosed = true;
  return true;
}


void QgsWFSFeatureIterator::copyFeature( const QgsFeature &srcFeature, QgsFeature &dstFeature )
{
  //copy the geometry
  QgsGeometry geometry = srcFeature.geometry();
  if ( !mShared->mGeometryAttribute.isEmpty() && !geometry.isNull() )
  {
    QgsGeometry g;
    g.fromWkb( geometry.asWkb() );
    dstFeature.setGeometry( g );
  }
  else
  {
    dstFeature.clearGeometry();
  }

  //and the attributes
  QgsFields &fields = mShared->mFields;
  dstFeature.initAttributes( fields.size() );

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    Q_FOREACH ( int i, mSubSetAttributes )
    {
      int idx = srcFeature.fields().indexFromName( fields.at( i ).name() );
      if ( idx >= 0 )
      {
        const QVariant &v = srcFeature.attributes().value( idx );
        if ( v.type() == fields.at( i ).type() )
          dstFeature.setAttribute( i, v );
        else if ( fields.at( i ).type() == QVariant::DateTime && !v.isNull() )
          dstFeature.setAttribute( i, QVariant( QDateTime::fromMSecsSinceEpoch( v.toLongLong() ) ) );
        else
          dstFeature.setAttribute( i, QgsVectorDataProvider::convertValue( fields.at( i ).type(), v.toString() ) );
      }
    }
  }
  else
  {
    for ( int i = 0; i < fields.size(); i++ )
    {
      int idx = srcFeature.fields().indexFromName( fields.at( i ).name() );
      if ( idx >= 0 )
      {
        const QVariant &v = srcFeature.attributes().value( idx );
        if ( v.type() == fields.at( i ).type() )
          dstFeature.setAttribute( i, v );
        else if ( fields.at( i ).type() == QVariant::DateTime && !v.isNull() )
          dstFeature.setAttribute( i, QVariant( QDateTime::fromMSecsSinceEpoch( v.toLongLong() ) ) );
        else
          dstFeature.setAttribute( i, QgsVectorDataProvider::convertValue( fields.at( i ).type(), v.toString() ) );
      }
    }
  }

  //id and valid
  dstFeature.setValid( true );
  dstFeature.setId( srcFeature.id() );
  dstFeature.setFields( fields ); // allow name-based attribute lookups
}


// -------------------------

QgsWFSFeatureSource::QgsWFSFeatureSource( const QgsWFSProvider *p )
  : mShared( p->mShared )
  , mCrs( p->crs() )
{
}

QgsFeatureIterator QgsWFSFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsWFSFeatureIterator( this, false, request ) );
}

