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
#include "qgsogcutils.h"
#include "qgswfsconstants.h"
#include "qgswfsfeatureiterator.h"
#include "qgswfsprovider.h"
#include "qgswfsshareddata.h"

#include <QTimer>
#include <QProgressDialog>


QgsWFSFeatureHitsAsyncRequest::QgsWFSFeatureHitsAsyncRequest( QgsWFSDataSourceURI& uri )
    : QgsWFSRequest( uri.uri() )
    , mNumberMatched( -1 )
{
  connect( this, SIGNAL( downloadFinished() ), this, SLOT( hitsReplyFinished() ) );
}

QgsWFSFeatureHitsAsyncRequest::~QgsWFSFeatureHitsAsyncRequest()
{
}

void QgsWFSFeatureHitsAsyncRequest::launch( const QUrl& url )
{
  sendGET( url,
           false, /* synchronous */
           true, /* forceRefresh */
           false /* cache */ );
}

void QgsWFSFeatureHitsAsyncRequest::hitsReplyFinished()
{
  QByteArray data = response();
  QgsGmlStreamingParser gmlParser( "", "", QgsFields() );
  if ( gmlParser.processData( data, true ) )
  {
    mNumberMatched = ( gmlParser.numberMatched() >= 0 ) ? gmlParser.numberMatched() :
                     gmlParser.numberReturned();
  }
  emit gotHitsResponse();
}

QString QgsWFSFeatureHitsAsyncRequest::errorMessageWithReason( const QString& reason )
{
  return tr( "Download of feature count failed: %1" ).arg( reason );
}

// -------------------------

QgsWFSFeatureDownloader::QgsWFSFeatureDownloader( QgsWFSSharedData* shared,
    QString filter )
    : QgsWFSRequest( shared->mURI.uri() )
    , mShared( shared )
    , mWFSFilter( filter )
    , mStop( false )
    , mProgressDialog( nullptr )
    , mProgressDialogShowImmediately( false )
    , mNumberMatched( -1 )
    , mMainWindow( nullptr )
    , mTimer( nullptr )
    , mFeatureHitsAsyncRequest( shared->mURI )
    , mTotalDownloadedFeatureCount( 0 )
{
  // Needed because used by a signal
  qRegisterMetaType< QVector<QgsWFSFeatureGmlIdPair> >( "QVector<QgsWFSFeatureGmlIdPair>" );
}

QgsWFSFeatureDownloader::~QgsWFSFeatureDownloader()
{
  stop();

  if ( mProgressDialog != nullptr )
    mProgressDialog->deleteLater();
  if ( mTimer != nullptr )
    mTimer->deleteLater();
}

void QgsWFSFeatureDownloader::stop()
{
  QgsDebugMsg( "QgsWFSFeatureDownloader::stop()" );
  mStop = true;
  emit doStop();
}

void QgsWFSFeatureDownloader::setStopFlag()
{
  QgsDebugMsg( "QgsWFSFeatureDownloader::setStopFlag()" );
  mStop = true;
}

// Called from GUI thread
void QgsWFSFeatureDownloader::createProgressDialog()
{
  if ( mStop )
    return;
  Q_ASSERT( mProgressDialog == nullptr );
  mProgressDialog = new QProgressDialog( tr( "Loading features for layer %1" ).arg( mShared->mURI.typeName() ),
                                         tr( "Abort" ), 0, mNumberMatched, mMainWindow );
  mProgressDialog->setWindowTitle( tr( "QGIS" ) );
  mProgressDialog->setValue( 0 );
  if ( mProgressDialogShowImmediately )
    mProgressDialog->show();

  connect( mProgressDialog, SIGNAL( canceled() ), this, SLOT( setStopFlag() ), Qt::DirectConnection );
  connect( mProgressDialog, SIGNAL( canceled() ), this, SLOT( stop() ) );

  connect( this, SIGNAL( updateProgress( int ) ), mProgressDialog, SLOT( setValue( int ) ) );
}

QUrl QgsWFSFeatureDownloader::buildURL( int startIndex, int maxFeatures, bool forHits )
{
  QUrl getFeatureUrl( mShared->mURI.baseURL() );
  getFeatureUrl.addQueryItem( "REQUEST", "GetFeature" );
  getFeatureUrl.addQueryItem( "VERSION",  mShared->mWFSVersion );
  if ( mShared->mWFSVersion.startsWith( "2.0" ) )
    getFeatureUrl.addQueryItem( "TYPENAMES",  mShared->mURI.typeName() );
  else
    getFeatureUrl.addQueryItem( "TYPENAME",  mShared->mURI.typeName() );
  if ( forHits )
  {
    getFeatureUrl.addQueryItem( "RESULTTYPE", "hits" );
  }
  else if ( maxFeatures > 0 )
  {
    if ( mShared->mSupportsPaging )
    {
      // Note: always include the STARTINDEX, even for zero, has some (likely buggy)
      // implementations do not return the same results if STARTINDEX=0 is specified
      // or not.
      // For example http://demo.opengeo.org/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=ne:ne_10m_admin_0_countries&STARTINDEX=0&COUNT=253
      // doesn't include ne_10m_admin_0_countries.99, as expected since it is
      // at index 254.
      getFeatureUrl.addQueryItem( "STARTINDEX", QString::number( startIndex ) );
    }
    if ( mShared->mWFSVersion.startsWith( "2.0" ) )
      getFeatureUrl.addQueryItem( "COUNT", QString::number( maxFeatures ) );
    else
      getFeatureUrl.addQueryItem( "MAXFEATURES", QString::number( maxFeatures ) );
  }
  QString srsName( mShared->srsName() );
  if ( !srsName.isEmpty() )
  {
    getFeatureUrl.addQueryItem( "SRSNAME", srsName );
  }

  // In case we must issue a BBOX and we have a filter, we must combine
  // both as a single filter, as both BBOX and FILTER aren't supported together
  if ( !mShared->mRect.isNull() && !mWFSFilter.isEmpty() )
  {
    double minx = mShared->mRect.xMinimum();
    double miny = mShared->mRect.yMinimum();
    double maxx = mShared->mRect.xMaximum();
    double maxy = mShared->mRect.yMaximum();
    QString filter( QString( "intersects_bbox($geometry, geomFromWKT('LINESTRING(%1 %2,%3 %4)'))" ).
                    arg( minx ).arg( miny ).arg( maxx ).arg( maxy ) );
    QgsExpression bboxExp( filter );
    QgsOgcUtils::GMLVersion gmlVersion;
    QgsOgcUtils::FilterVersion filterVersion;
    bool honourAxisOrientation = false;
    if ( mShared->mWFSVersion.startsWith( "1.0" ) )
    {
      gmlVersion = QgsOgcUtils::GML_2_1_2;
      filterVersion = QgsOgcUtils::FILTER_OGC_1_0;
    }
    else if ( mShared->mWFSVersion.startsWith( "1.1" ) )
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
    QDomElement bboxElem = QgsOgcUtils::expressionToOgcFilter( bboxExp, doc,
                           gmlVersion, filterVersion, mShared->mGeometryAttribute, mShared->srsName(),
                           honourAxisOrientation, mShared->mURI.invertAxisOrientation() );
    doc.appendChild( bboxElem );
    QDomNode bboxNode = bboxElem.firstChildElement();
    bboxNode = bboxElem.removeChild( bboxNode );

    QDomDocument filterDoc;
    ( void )filterDoc.setContent( mWFSFilter, true );
    QDomNode filterNode = filterDoc.firstChildElement().firstChildElement();
    filterNode = filterDoc.firstChildElement().removeChild( filterNode );

    QDomElement andElem = doc.createElement(( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes:And" : "ogc:And" );
    andElem.appendChild( bboxNode );
    andElem.appendChild( filterNode );
    doc.firstChildElement().appendChild( andElem );

    getFeatureUrl.addQueryItem( "FILTER", doc.toString() );
  }
  else if ( !mShared->mRect.isNull() )
  {
    bool invertAxis = false;
    if ( !mShared->mWFSVersion.startsWith( "1.0" ) && !mShared->mURI.ignoreAxisOrientation() &&
         mShared->mSourceCRS.axisInverted() )
    {
      invertAxis = true;
    }
    if ( mShared->mURI.invertAxisOrientation() )
    {
      invertAxis = !invertAxis;
    }
    getFeatureUrl.addQueryItem( "BBOX",  QString(( invertAxis ) ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                                .arg( qgsDoubleToString( mShared->mRect.xMinimum() ),
                                      qgsDoubleToString( mShared->mRect.yMinimum() ),
                                      qgsDoubleToString( mShared->mRect.xMaximum() ),
                                      qgsDoubleToString( mShared->mRect.yMaximum() ) ) );
  }
  else if ( !mWFSFilter.isEmpty() )
  {
    getFeatureUrl.addQueryItem( "FILTER", mWFSFilter );
  }

  return getFeatureUrl;
}

// Called when we get the response of the asynchronous RESULTTYPE=hits request
void QgsWFSFeatureDownloader::gotHitsResponse()
{
  mNumberMatched = mFeatureHitsAsyncRequest.numberMatched();
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
    connect( &mFeatureHitsAsyncRequest, SIGNAL( gotHitsResponse() ), this, SLOT( gotHitsResponse() ) );
    mFeatureHitsAsyncRequest.launch( buildURL( 0, -1, true ) );
  }
}

void QgsWFSFeatureDownloader::run( bool serializeFeatures, int maxFeatures )
{
  bool success = true;

  QEventLoop loop;
  connect( this, SIGNAL( doStop() ), &loop, SLOT( quit() ) );
  connect( this, SIGNAL( downloadFinished() ), &loop, SLOT( quit() ) );
  connect( this, SIGNAL( downloadProgress( qint64, qint64 ) ), &loop, SLOT( quit() ) );

  QTimer timerForHits;

  Q_FOREACH ( QWidget* widget, qApp->topLevelWidgets() )
  {
    if ( widget->objectName() == "QgisApp" )
    {
      mMainWindow = widget;
      break;
    }
  }

  if ( mMainWindow != nullptr && maxFeatures != 1 && mShared->supportsHits() )
  {
    // In case the header of the GetFeature response doesn't contain the total
    // number of features, or we don't get it within 4 seconds, we will issue
    // an explicit RESULTTYPE=hits request.
    timerForHits.setInterval( 4 * 1000 );
    timerForHits.setSingleShot( true );
    timerForHits.start();
    connect( &timerForHits, SIGNAL( timeout() ), this, SLOT( startHitsRequest() ) );
    connect( &mFeatureHitsAsyncRequest, SIGNAL( downloadFinished() ), &loop, SLOT( quit() ) );
  }

  while ( true )
  {
    QgsGmlStreamingParser parser( mShared->mURI.typeName(),
                                  mShared->mGeometryAttribute,
                                  mShared->mFields );

    sendGET( buildURL( mTotalDownloadedFeatureCount,
                       maxFeatures ? maxFeatures : mShared->mMaxFeatures, false ),
             false, /* synchronous */
             true, /* forceRefresh */
             false /* cache */ );

    bool retry = false;
    int featureCount = 0;
    while ( true )
    {
      loop.exec( QEventLoop::ExcludeUserInputEvents );
      if ( mStop )
      {
        success = false;
        break;
      }

      QByteArray data;
      bool finished = false;
      if ( mReply )
      {
        data = mReply->readAll();
      }
      else
      {
        data = mResponse;
        finished = true;
      }
      // Parse the received chunk of data
      if ( !parser.processData( data, finished ) )
      {
        success = false;
        mErrorMessage = tr( "Error when parsing GetFeature response" );
        QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
        if ( mProgressDialog != nullptr )
        {
          mProgressDialog->deleteLater();
          mProgressDialog = nullptr;
        }
        break;
      }
      if ( parser.isException() && finished )
      {
        // Some GeoServer instances in WFS 2.0 with paging throw an exception
        // e.g. http://ows.region-bretagne.fr/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=rb:etudes&STARTINDEX=0&COUNT=1
        // Disabling paging helps in those cases
        if ( mShared->mSupportsPaging && mTotalDownloadedFeatureCount == 0 &&
             parser.exceptionText().contains( "Cannot do natural order without a primary key" ) )
        {
          QgsDebugMsg( QString( "Got exception %1. Re-trying with paging disabled" ).arg( parser.exceptionText() ) );
          mShared->mSupportsPaging = false;
          retry = true;
          break;
        }

        success = false;
        mErrorMessage = tr( "Server generated an exception in GetFeature response" ) + ": " + parser.exceptionText();
        QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
        if ( mProgressDialog != nullptr )
        {
          mProgressDialog->deleteLater();
          mProgressDialog = nullptr;
        }
        break;
      }

      // Consider if we should display a progress dialog
      // We can only do that if we know how many features will be downloaded
      if ( mTimer == nullptr && maxFeatures != 1 && mMainWindow != nullptr )
      {
        if ( mNumberMatched < 0 )
        {
          // Some servers, like http://demo.opengeo.org/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=ne:ne_10m_admin_0_countries&STARTINDEX=0&COUNT=50&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=-133.04422094925158149,-188.9997780764296067,126.67820349384365386,188.99999458723010548,
          // return numberMatched="unknown" for all pages, except the last one, where
          // this is (erroneously?) the number of features returned
          if ( parser.numberMatched() > 0 && mTotalDownloadedFeatureCount == 0 )
            mNumberMatched = parser.numberMatched();
          // The number returned can only be used if we aren't in paging mode
          else if ( parser.numberReturned() > 0 && !mShared->mSupportsPaging )
            mNumberMatched = parser.numberMatched();
          // We can only use the layer feature count if we don't apply a BBOX
          else if ( mShared->isFeatureCountExact() && mShared->mRect.isNull() )
            mNumberMatched = mShared->getFeatureCount( false );

          // If we didn't get a valid mNumberMatched, we will possibly issue
          // a explicit RESULTTYPE=hits request 4 second after the beginning of
          // the download
        }

        if ( mNumberMatched > 0 )
        {
          if ( mShared->supportsHits() )
            disconnect( &timerForHits, SIGNAL( timeout() ), this, SLOT( startHitsRequest() ) );

          // This is a bit tricky. We want the createProgressDialog()
          // method to be run into the GUI thread
          mTimer = new QTimer();
          mTimer->setSingleShot( true );

          // Direct connection, since we want createProgressDialog()
          // to be invoked from the same thread as timer, and not in the
          // thread of this
          connect( mTimer, SIGNAL( timeout() ), this, SLOT( createProgressDialog() ), Qt::DirectConnection );

          mTimer->moveToThread( mMainWindow->thread() );
          QMetaObject::invokeMethod( mTimer, "start", Qt::QueuedConnection );
        }
      }

      QVector<QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair> featurePtrList =
        parser.getAndStealReadyFeatures();

      featureCount += featurePtrList.size();
      mTotalDownloadedFeatureCount += featurePtrList.size();

      if ( !mStop )
      {
        emit updateProgress( mTotalDownloadedFeatureCount );
      }

      if ( featurePtrList.size() != 0 )
      {
        QVector<QgsWFSFeatureGmlIdPair> featureList;
        Q_FOREACH ( QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair featPair, featurePtrList )
        {
          featureList.push_back( QgsWFSFeatureGmlIdPair( *( featPair.first ), featPair.second ) );
          delete featPair.first;
        }

        // We call it directly to avoid asynchronous signal notification, and
        // as serializeFeatures() can modify the featureList to remove features
        // that have already been cached, so as to avoid to notify them several
        // times to subscribers
        if ( serializeFeatures )
          mShared->serializeFeatures( featureList );

        if ( !featureList.isEmpty() )
          emit featureReceived( featureList );
      }

      if ( finished )
        break;
    }
    if ( retry )
      continue;
    if ( !success )
      break;
    if ( !mShared->mSupportsPaging )
      break;
    if ( maxFeatures == 1 )
      break;
    // Detect if we are at the last page
    if (( mShared->mMaxFeatures > 0 && featureCount < mShared->mMaxFeatures ) || featureCount == 0 )
      break;
  }

  mStop = true;

  if ( serializeFeatures )
    mShared->endOfDownload( success, mTotalDownloadedFeatureCount );

  // We must emit the signal *AFTER* the previous call to mShared->endOfDownload()
  // to avoid issues with iterators that would start just now, wouldn't detect
  // that the downloader has finished, would register to itself, but would never
  // receive the endOfDownload signal. This is not just a theoretical problem.
  // If you switch both calls, it happens quite easily in Release mode with the
  // test suite.
  emit endOfDownload( success );

  if ( mProgressDialog != nullptr )
  {
    mProgressDialog->deleteLater();
    mProgressDialog = nullptr;
  }
  if ( mTimer != nullptr )
  {
    mTimer->deleteLater();
    mTimer = nullptr;
  }
  // explictly abort here so that mReply is destroyed within the right thread
  // otherwise will deadlock because deleteLayer() will not have a valid thread to post
  abort();
  mFeatureHitsAsyncRequest.abort();
}

QString QgsWFSFeatureDownloader::errorMessageWithReason( const QString& reason )
{
  return tr( "Download of features failed: %1" ).arg( reason );
}

QgsWFSThreadedFeatureDownloader::QgsWFSThreadedFeatureDownloader( QgsWFSSharedData* shared )
    : mShared( shared )
    , mWFSFilter( mShared->WFSFilter() )
    , mDownloader( nullptr )
{
}

QgsWFSThreadedFeatureDownloader::~QgsWFSThreadedFeatureDownloader()
{
  stop();
}

void QgsWFSThreadedFeatureDownloader::stop()
{
  if ( mDownloader != nullptr )
  {
    mDownloader->stop();
    wait();
    delete mDownloader;
    mDownloader = nullptr;
  }
}

void QgsWFSThreadedFeatureDownloader::run()
{
  // We need to construct it in the run() method (i.e. in the new thread)
  mDownloader = new QgsWFSFeatureDownloader( mShared, mWFSFilter );
  emit ready();
  mDownloader->run( true, /* serialize features */
                    0 /* user max features */ );
}

// -------------------------

QgsWFSFeatureIterator::QgsWFSFeatureIterator( QgsWFSFeatureSource* source,
    bool ownSource,
    const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsWFSFeatureSource>( source, ownSource, request )
    , mShared( source->mShared )
    , mCurFeatureIdx( 0 )
    , mDownloadFinished( false )
    , mLoop( nullptr )
    , mInterruptionChecker( nullptr )
{
  int genCounter = ( mShared->mURI.isRestrictedToRequestBBOX() && !request.filterRect().isNull() ) ?
                   mShared->registerToCache( this, request.filterRect() ) : mShared->registerToCache( this );
  mDownloadFinished = genCounter < 0;
  if ( mShared->mCacheDataProvider == nullptr )
    return;

  QgsDebugMsg( QString( "QgsWFSFeatureIterator::constructor(): genCounter=%1 " ).arg( genCounter ) );

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

  requestCache.setFilterRect( mRequest.filterRect() );

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    const QgsFields & dataProviderFields = mShared->mCacheDataProvider->fields();
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
      Q_FOREACH ( const QString& field, mRequest.filterExpression()->referencedColumns() )
      {
        int idx = dataProviderFields.indexFromName( field );
        if ( idx >= 0 && !cacheSubSet.contains( idx ) )
          cacheSubSet.append( idx );
        idx = mShared->mFields.indexFromName( field );
        if ( idx >= 0  && !mSubSetAttributes.contains( idx ) )
          mSubSetAttributes.append( idx );
      }
    }

    if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
    {
      int hexwkbGeomIdx = dataProviderFields.indexFromName( QgsWFSConstants::FIELD_HEXWKB_GEOM );
      Q_ASSERT( hexwkbGeomIdx >= 0 );
      cacheSubSet.append( hexwkbGeomIdx );
    }
    requestCache.setSubsetOfAttributes( cacheSubSet );
  }

  mCacheIterator = mShared->mCacheDataProvider->getFeatures( requestCache );
}

QgsWFSFeatureIterator::~QgsWFSFeatureIterator()
{
  QgsDebugMsg( QString( "qgsWFSFeatureIterator::~QgsWFSFeatureIterator()" ) );

  close();
}

void QgsWFSFeatureIterator::connectSignals( QObject* downloader )
{
  connect( downloader, SIGNAL( featureReceived( QVector<QgsWFSFeatureGmlIdPair> ) ),
           this, SLOT( featureReceived( QVector<QgsWFSFeatureGmlIdPair> ) ) );
  connect( downloader, SIGNAL( endOfDownload( bool ) ),
           this, SLOT( endOfDownload( bool ) ) );
}

void QgsWFSFeatureIterator::endOfDownload( bool )
{
  mDownloadFinished = true;
  if ( mLoop != nullptr )
    mLoop->quit();
}

void QgsWFSFeatureIterator::setInterruptionChecker( QgsInterruptionChecker* interruptionChecker )
{
  mInterruptionChecker = interruptionChecker;
}

void QgsWFSFeatureIterator::featureReceived( QVector<QgsWFSFeatureGmlIdPair> list )
{
  QString msg;
  msg.sprintf( "QgsWFSFeatureIterator::featureReceived %p %d", QThread::currentThread(), list.size() );
  QgsDebugMsg( msg );
  mFeatureList += list;
  if ( mLoop != nullptr )
    mLoop->quit();
}

void QgsWFSFeatureIterator::checkInterruption()
{
  //QgsDebugMsg("QgsWFSFeatureIterator::checkInterruption()");

  if ( mInterruptionChecker && mInterruptionChecker->mustStop() )
  {
    mDownloadFinished = true;
    if ( mLoop != nullptr )
      mLoop->quit();
  }
}

bool QgsWFSFeatureIterator::fetchFeature( QgsFeature& f )
{
  f.setValid( false );

  if ( mClosed )
    return false;

  // First step is to iterate over the on-disk cache
  QgsFeature cachedFeature;
  while ( mCacheIterator.nextFeature( cachedFeature ) )
  {
    if ( mInterruptionChecker && mInterruptionChecker->mustStop() )
      return false;

    //QgsDebugMsg(QString("QgsWFSSharedData::fetchFeature() : mCacheIterator.nextFeature(cachedFeature)") );

    if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
    {
      int idx = cachedFeature.fields()->indexFromName( QgsWFSConstants::FIELD_HEXWKB_GEOM );
      Q_ASSERT( idx >= 0 );

      const QVariant &v = cachedFeature.attributes().value( idx );
      if ( !v.isNull() && v.type() == QVariant::String )
      {
        QByteArray wkbGeom( QByteArray::fromHex( v.toString().toAscii() ) );
        QgsGeometry *g = new QgsGeometry();
        unsigned char* wkbClone = new unsigned char[wkbGeom.size()];
        memcpy( wkbClone, wkbGeom.data(), wkbGeom.size() );
        try
        {
          g->fromWkb( wkbClone, wkbGeom.size() );
          cachedFeature.setGeometry( g );
        }
        catch ( const QgsWkbException& )
        {
          QgsDebugMsg( QString( "Invalid WKB for cached feature %1" ).arg( cachedFeature.id() ) );
          delete[] wkbClone;
          cachedFeature.setGeometry( nullptr );
        }
      }
      else
      {
        cachedFeature.setGeometry( nullptr );
      }
    }
    else
    {
      cachedFeature.setGeometry( nullptr );
    }

    const QgsGeometry* constGeom = cachedFeature.constGeometry();
    if ( !mRequest.filterRect().isNull() &&
         ( constGeom == nullptr || !constGeom->intersects( mRequest.filterRect() ) ) )
    {
      continue;
    }

    copyFeature( cachedFeature, f );
    return true;
  }

  // Second step is to wait for features to be notified by the downloader
  while ( true )
  {
    //QgsDebugMsg(QString("QgsWFSSharedData::fetchFeature() : mCurFeatureIdx=%1, mFeatureList=%2").arg(mCurFeatureIdx).arg(mFeatureList.size()) );
    while ( mCurFeatureIdx < mFeatureList.size() )
    {
      if ( mInterruptionChecker && mInterruptionChecker->mustStop() )
        return false;

      if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
      {
        if ( mFeatureList[mCurFeatureIdx].first.id() != mRequest.filterFid() )
        {
          mCurFeatureIdx ++;
          continue;
        }
      }
      else if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
      {
        if ( !mRequest.filterFids().contains( mFeatureList[mCurFeatureIdx].first.id() ) )
        {
          mCurFeatureIdx ++;
          continue;
        }
      }

      const QgsGeometry* constGeom = mFeatureList[mCurFeatureIdx].first.constGeometry();
      if ( !mRequest.filterRect().isNull() &&
           ( constGeom == nullptr || !constGeom->intersects( mRequest.filterRect() ) ) )
      {
        mCurFeatureIdx ++;
        continue;
      }

      copyFeature( mFeatureList[mCurFeatureIdx].first, f );
      mCurFeatureIdx ++;
      return true;
    }

    if ( mDownloadFinished )
      return false;
    if ( mInterruptionChecker && mInterruptionChecker->mustStop() )
      return false;

    mCurFeatureIdx = 0;
    mFeatureList.clear();

    //QgsDebugMsg("fetchFeature before loop");
    QEventLoop loop;
    mLoop = &loop;
    QTimer timer( this );
    timer.start( 50 );
    if ( mInterruptionChecker != nullptr )
      connect( &timer, SIGNAL( timeout() ), this, SLOT( checkInterruption() ) );
    loop.exec( QEventLoop::ExcludeUserInputEvents );
    mLoop = nullptr;
    //QgsDebugMsg("fetchFeature after loop");
  }
}

bool QgsWFSFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  mCurFeatureIdx = 0;
  mFeatureList.clear();
  QgsFeatureRequest requestCache;
  int genCounter = mShared->getUpdatedCounter();
  if ( genCounter >= 0 )
    requestCache.setFilterExpression( QString( QgsWFSConstants::FIELD_GEN_COUNTER + " <= %1" ).arg( genCounter ) );
  else
    mDownloadFinished = true;
  if ( mShared->mCacheDataProvider != nullptr )
    mCacheIterator = mShared->mCacheDataProvider->getFeatures( requestCache );
  return true;
}

bool QgsWFSFeatureIterator::close()
{
  if ( mClosed )
    return false;
  QgsDebugMsg( QString( "qgsWFSFeatureIterator::close()" ) );

  iteratorClosed();

  mClosed = true;
  return true;
}


void QgsWFSFeatureIterator::copyFeature( const QgsFeature& srcFeature, QgsFeature& dstFeature )
{
  //copy the geometry
  const QgsGeometry* geometry = srcFeature.constGeometry();
  if ( geometry )
  {
    const unsigned char *geom = geometry->asWkb();
    int geomSize = geometry->wkbSize();
    unsigned char* copiedGeom = new unsigned char[geomSize];
    memcpy( copiedGeom, geom, geomSize );

    QgsGeometry *g = new QgsGeometry();
    g->fromWkb( copiedGeom, geomSize );
    dstFeature.setGeometry( g );
  }
  else
  {
    dstFeature.setGeometry( nullptr );
  }

  //and the attributes
  QgsFields& fields = mShared->mFields;
  dstFeature.initAttributes( fields.size() );

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    Q_FOREACH ( int i, mSubSetAttributes )
    {
      int idx = srcFeature.fields()->indexFromName( fields.at( i ).name() );
      if ( idx >= 0 )
      {
        const QVariant &v = srcFeature.attributes().value( idx );
        if ( v.type() != fields.at( i ).type() )
          dstFeature.setAttribute( i, QgsVectorDataProvider::convertValue( fields.at( i ).type(), v.toString() ) );
        else
          dstFeature.setAttribute( i, v );
      }
    }
  }
  else
  {
    for ( int i = 0; i < fields.size(); i++ )
    {
      int idx = srcFeature.fields()->indexFromName( fields.at( i ).name() );
      if ( idx >= 0 )
      {
        const QVariant &v = srcFeature.attributes().value( idx );
        if ( v.type() != fields.at( i ).type() )
          dstFeature.setAttribute( i, QgsVectorDataProvider::convertValue( fields.at( i ).type(), v.toString() ) );
        else
          dstFeature.setAttribute( i, v );
      }
    }
  }

  //id and valid
  dstFeature.setValid( true );
  dstFeature.setFeatureId( srcFeature.id() );
  dstFeature.setFields( fields ); // allow name-based attribute lookups
}


// -------------------------

QgsWFSFeatureSource::QgsWFSFeatureSource( const QgsWFSProvider* p )
    : mShared( p->mShared )
{
}

QgsWFSFeatureSource::~QgsWFSFeatureSource()
{

}

QgsFeatureIterator QgsWFSFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsWFSFeatureIterator( this, false, request ) );
}

