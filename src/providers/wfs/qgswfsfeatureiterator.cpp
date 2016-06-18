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
#include "qgswfsutils.h"
#include "qgscrscache.h"

#include <QDir>
#include <QProgressDialog>
#include <QTimer>
#include <QSettings>
#include <QStyle>

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
  if ( mErrorCode == NoError )
  {
    QByteArray data = response();
    QgsGmlStreamingParser gmlParser( "", "", QgsFields() );
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

QString QgsWFSFeatureHitsAsyncRequest::errorMessageWithReason( const QString& reason )
{
  return tr( "Download of feature count failed: %1" ).arg( reason );
}

// -------------------------

QgsWFSFeatureDownloader::QgsWFSFeatureDownloader( QgsWFSSharedData* shared )
    : QgsWFSRequest( shared->mURI.uri() )
    , mShared( shared )
    , mStop( false )
    , mProgressDialog( nullptr )
    , mProgressDialogShowImmediately( false )
    , mSupportsPaging( shared->mCaps.supportsPaging )
    , mRemoveNSPrefix( false )
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

  if ( mProgressDialog )
    mProgressDialog->deleteLater();
  if ( mTimer )
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

QgsWFSProgressDialog::QgsWFSProgressDialog( const QString & labelText,
    const QString & cancelButtonText,
    int minimum, int maximum, QWidget * parent )
    : QProgressDialog( labelText, cancelButtonText, minimum, maximum, parent )
{
  mCancel = new QPushButton( cancelButtonText, this );
  setCancelButton( mCancel );
  mHide = new QPushButton( tr( "Hide" ), this );
  connect( mHide, SIGNAL( clicked() ), this, SIGNAL( hide() ) );
}

void QgsWFSProgressDialog::resizeEvent( QResizeEvent * ev )
{
  QProgressDialog::resizeEvent( ev );
  // Note: this relies heavily on the details of the layout done in QProgressDialogPrivate::layout()
  // Might be a bit fragile depending on QT versions.
  QRect rect = geometry();
  QRect cancelRect = mCancel->geometry();
  QRect hideRect = mHide->geometry();
  int mtb = style()->pixelMetric( QStyle::PM_DefaultTopLevelMargin );
  int mlr = qMin( width() / 10, mtb );
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
  if ( mStop )
    return;
  Q_ASSERT( !mProgressDialog );
  mProgressDialog = new QgsWFSProgressDialog( tr( "Loading features for layer %1" ).arg( mShared->mURI.typeName() ),
      tr( "Abort" ), 0, mNumberMatched, mMainWindow );
  mProgressDialog->setWindowTitle( tr( "QGIS" ) );
  mProgressDialog->setValue( 0 );
  if ( mProgressDialogShowImmediately )
    mProgressDialog->show();

  connect( mProgressDialog, SIGNAL( canceled() ), this, SLOT( setStopFlag() ), Qt::DirectConnection );
  connect( mProgressDialog, SIGNAL( canceled() ), this, SLOT( stop() ) );
  connect( mProgressDialog, SIGNAL( hide() ), this, SLOT( hideProgressDialog() ) );

  connect( this, SIGNAL( updateProgress( int ) ), mProgressDialog, SLOT( setValue( int ) ) );
}

QString QgsWFSFeatureDownloader::sanitizeFilter( QString filter )
{
  filter = filter.replace( "<fes:ValueReference xmlns:fes=\"http://www.opengis.net/fes/2.0\">", "<fes:ValueReference>" );
  QString nsPrefix( QgsWFSUtils::nameSpacePrefix( mShared->mURI.typeName() ) );
  if ( mRemoveNSPrefix && !nsPrefix.isEmpty() )
    filter = filter.replace( "<fes:ValueReference>" + nsPrefix + ":", "<fes:ValueReference>" );
  return filter;
}

QUrl QgsWFSFeatureDownloader::buildURL( int startIndex, int maxFeatures, bool forHits )
{
  QUrl getFeatureUrl( mShared->mURI.baseURL() );
  getFeatureUrl.addQueryItem( "REQUEST", "GetFeature" );
  getFeatureUrl.addQueryItem( "VERSION",  mShared->mWFSVersion );

  QString typenames;
  if ( mShared->mLayerPropertiesList.isEmpty() )
  {
    typenames = mShared->mURI.typeName();
  }
  else
  {
    Q_FOREACH ( const QgsOgcUtils::LayerProperties layerProperties, mShared->mLayerPropertiesList )
    {
      if ( !typenames.isEmpty() )
        typenames += ",";
      typenames += layerProperties.mName;
    }
  }
  if ( mShared->mWFSVersion.startsWith( "2.0" ) )
    getFeatureUrl.addQueryItem( "TYPENAMES",  typenames );
  else
    getFeatureUrl.addQueryItem( "TYPENAME",  typenames );
  if ( forHits )
  {
    getFeatureUrl.addQueryItem( "RESULTTYPE", "hits" );
  }
  else if ( maxFeatures > 0 )
  {
    if ( mSupportsPaging )
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
  if ( !srsName.isEmpty() && !forHits )
  {
    getFeatureUrl.addQueryItem( "SRSNAME", srsName );
  }

  // In case we must issue a BBOX and we have a filter, we must combine
  // both as a single filter, as both BBOX and FILTER aren't supported together
  if ( !mShared->mRect.isNull() && !mShared->mWFSFilter.isEmpty() )
  {
    double minx = mShared->mRect.xMinimum();
    double miny = mShared->mRect.yMinimum();
    double maxx = mShared->mRect.xMaximum();
    double maxy = mShared->mRect.yMaximum();
    QString filterBbox( QString( "intersects_bbox($geometry, geomFromWKT('LINESTRING(%1 %2,%3 %4)'))" ).
                        arg( minx ).arg( miny ).arg( maxx ).arg( maxy ) );
    QgsExpression bboxExp( filterBbox );
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

    QDomElement andElem = doc.createElement(( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes:And" : "ogc:And" );
    andElem.appendChild( bboxNode );
    andElem.appendChild( filterNode );
    doc.firstChildElement().appendChild( andElem );

    getFeatureUrl.addQueryItem( "FILTER", sanitizeFilter( doc.toString() ) );
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
    QString bbox( QString(( invertAxis ) ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                  .arg( qgsDoubleToString( mShared->mRect.xMinimum() ),
                        qgsDoubleToString( mShared->mRect.yMinimum() ),
                        qgsDoubleToString( mShared->mRect.xMaximum() ),
                        qgsDoubleToString( mShared->mRect.yMaximum() ) ) );
    // Some servers like Geomedia need the srsname to be explictly appended
    // otherwise they are confused and do not interpret it properly
    bbox += "," + mShared->srsName();
    getFeatureUrl.addQueryItem( "BBOX",  bbox );
  }
  else if ( !mShared->mWFSFilter.isEmpty() )
  {
    getFeatureUrl.addQueryItem( "FILTER", sanitizeFilter( mShared->mWFSFilter ) );
  }

  if ( !mShared->mSortBy.isEmpty() && !forHits )
  {
    getFeatureUrl.addQueryItem( "SORTBY", mShared->mSortBy );
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

  if ( !mShared->mHideProgressDialog && maxFeatures != 1 && mShared->supportsHits() )
  {
    Q_FOREACH ( QWidget* widget, qApp->topLevelWidgets() )
    {
      if ( widget->objectName() == "QgisApp" )
      {
        mMainWindow = widget;
        break;
      }
    }
  }

  if ( mMainWindow )
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

  bool interrupted = false;
  bool truncatedResponse = false;
  QSettings s;
  const int maxRetry = s.value( "/qgis/defaultTileMaxRetry", "3" ).toInt();
  int retryIter = 0;
  int lastValidTotalDownloadedFeatureCount = 0;
  int pagingIter = 1;
  QString gmlIdFirstFeatureFirstIter;
  bool disablePaging = false;
  while ( true )
  {
    success = true;
    QgsGmlStreamingParser* parser = mShared->createParser();

    QUrl url( buildURL( mTotalDownloadedFeatureCount,
                        maxFeatures ? maxFeatures : mShared->mMaxFeatures, false ) );

    // Small hack for testing purposes
    if ( retryIter > 0 && url.toString().contains( "fake_qgis_http_endpoint" ) )
    {
      url.addQueryItem( "RETRY", QString::number( retryIter ) );
    }

    sendGET( url,
             false, /* synchronous */
             true, /* forceRefresh */
             false /* cache */ );

    int featureCountForThisResponse = 0;
    while ( true )
    {
      loop.exec( QEventLoop::ExcludeUserInputEvents );
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
        data = mReply->readAll();
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
        if ( mSupportsPaging && mTotalDownloadedFeatureCount == 0 &&
             parser->exceptionText().contains( "Cannot do natural order without a primary key" ) )
        {
          QgsDebugMsg( QString( "Got exception %1. Re-trying with paging disabled" ).arg( parser->exceptionText() ) );
          mSupportsPaging = false;
        }
        // GeoServer doesn't like typenames prefixed by namespace prefix, despite
        // the examples in the WFS 2.0 spec showing that
        else if ( !mRemoveNSPrefix && parser->exceptionText().contains( "more than one feature type" ) )
        {
          QgsDebugMsg( QString( "Got exception %1. Re-trying by removing namespace prefix" ).arg( parser->exceptionText() ) );
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
      if ( !mTimer && maxFeatures != 1 && mMainWindow )
      {
        if ( mNumberMatched < 0 )
        {
          // Some servers, like http://demo.opengeo.org/geoserver/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=ne:ne_10m_admin_0_countries&STARTINDEX=0&COUNT=50&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=-133.04422094925158149,-188.9997780764296067,126.67820349384365386,188.99999458723010548,
          // return numberMatched="unknown" for all pages, except the last one, where
          // this is (erroneously?) the number of features returned
          if ( parser->numberMatched() > 0 && mTotalDownloadedFeatureCount == 0 )
            mNumberMatched = parser->numberMatched();
          // The number returned can only be used if we aren't in paging mode
          else if ( parser->numberReturned() > 0 && !mSupportsPaging )
            mNumberMatched = parser->numberMatched();
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
             mShared->mWFSVersion.startsWith( "1.1" ) &&
             parser->srsName().startsWith( "EPSG:" ) &&
             !parser->layerExtent().isNull() &&
             !mShared->mURI.ignoreAxisOrientation() &&
             !mShared->mURI.invertAxisOrientation() )
        {
          QgsCoordinateReferenceSystem crs = QgsCRSCache::instance()->crsByOgcWmsCrs( parser->srsName() );
          if ( crs.isValid() && crs.axisInverted() &&
               !mShared->mCapabilityExtent.contains( parser->layerExtent() ) )
          {
            QgsRectangle invertedRectangle( parser->layerExtent() );
            invertedRectangle.invert();
            if ( mShared->mCapabilityExtent.contains( invertedRectangle ) )
            {
              mShared->mGetFeatureEPSGDotHonoursEPSGOrder = true;
              QgsDebugMsg( "Server is likely MapServer. Using mGetFeatureEPSGDotHonoursEPSGOrder mode" );
            }
          }
        }

        QVector<QgsWFSFeatureGmlIdPair> featureList;
        for ( int i = 0;i < featurePtrList.size();i++ )
        {
          QgsGmlStreamingParser::QgsGmlFeaturePtrGmlIdPair& featPair = featurePtrList[i];
          QgsFeature& f = *( featPair.first );
          QString gmlId( featPair.second );
          if ( gmlId.isEmpty() )
          {
            // Should normally not happen on sane WFS sources, but can happen with
            // Geomedia
            gmlId = QgsWFSUtils::getMD5( f );
            if ( !mShared->mHasWarnedAboutMissingFeatureId )
            {
              QgsDebugMsg( "Server returns features without fid/gml:id. Computing a fake one using feature attributes" );
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
            QgsDebugMsg( "Server does not seem to properly support paging since it returned the same first feature for 2 different page requests. Disabling paging" );
          }

          if ( mShared->mGetFeatureEPSGDotHonoursEPSGOrder && f.geometry() )
          {
            f.geometry()->transform( QTransform( 0, 1, 1, 0, 0, 0 ) );
          }

          featureList.push_back( QgsWFSFeatureGmlIdPair( f, gmlId ) );
          delete featPair.first;
          if (( i > 0 && ( i % 1000 ) == 0 ) || i + 1 == featurePtrList.size() )
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
        if ( parser->isTruncatedResponse() && !mSupportsPaging )
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

    if ( !mSupportsPaging )
      break;
    if ( maxFeatures == 1 )
      break;
    // Detect if we are at the last page
    if (( mShared->mMaxFeatures > 0 && featureCountForThisResponse < mShared->mMaxFeatures ) || featureCountForThisResponse == 0 )
      break;
    ++ pagingIter;
    if ( disablePaging )
    {
      mSupportsPaging = mShared->mCaps.supportsPaging = false;
      mTotalDownloadedFeatureCount = 0;
      if ( mShared->mMaxFeaturesWasSetFromDefaultForPaging )
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
    , mDownloader( nullptr )
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

void QgsWFSThreadedFeatureDownloader::run()
{
  // We need to construct it in the run() method (i.e. in the new thread)
  mDownloader = new QgsWFSFeatureDownloader( mShared );
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
    , mDownloadFinished( false )
    , mLoop( nullptr )
    , mInterruptionChecker( nullptr )
    , mCounter( 0 )
    , mWriteTransferThreshold( 1024 * 1024 )
    , mWriterFile( nullptr )
    , mWriterStream( nullptr )
    , mReaderFile( nullptr )
    , mReaderStream( nullptr )
    , mFetchGeometry( false )
{
  // Configurable for the purpose of unit tests
  QString threshold( getenv( "QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD" ) );
  if ( !threshold.isEmpty() )
    mWriteTransferThreshold = threshold.toInt();

  int genCounter = ( mShared->mURI.isRestrictedToRequestBBOX() && !request.filterRect().isNull() ) ?
                   mShared->registerToCache( this, request.filterRect() ) : mShared->registerToCache( this );
  mDownloadFinished = genCounter < 0;
  if ( !mShared->mCacheDataProvider )
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

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) ||
       ( mRequest.filterType() == QgsFeatureRequest::FilterExpression && mRequest.filterExpression()->needsGeometry() ) )
  {
    mFetchGeometry = true;
  }

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

    if ( mFetchGeometry )
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

void QgsWFSFeatureIterator::connectSignals( QObject* downloader )
{
  // We want to run the slot for that signal in the same thread as the sender
  // so as to avoid the list of features to accumulate without control in
  // memory
  connect( downloader, SIGNAL( featureReceived( QVector<QgsWFSFeatureGmlIdPair> ) ),
           this, SLOT( featureReceivedSynchronous( QVector<QgsWFSFeatureGmlIdPair> ) ), Qt::DirectConnection );

  connect( downloader, SIGNAL( featureReceived( int ) ),
           this, SLOT( featureReceived( int ) ) );

  connect( downloader, SIGNAL( endOfDownload( bool ) ),
           this, SLOT( endOfDownload( bool ) ) );
}

void QgsWFSFeatureIterator::endOfDownload( bool )
{
  mDownloadFinished = true;
  if ( mLoop )
    mLoop->quit();
}

void QgsWFSFeatureIterator::setInterruptionChecker( QgsInterruptionChecker* interruptionChecker )
{
  mInterruptionChecker = interruptionChecker;
}

// This method will serialize the receive feature list, first into memory, and
// if it goes above a given threshold, on disk
void QgsWFSFeatureIterator::featureReceivedSynchronous( QVector<QgsWFSFeatureGmlIdPair> list )
{
  QgsDebugMsg( QString( "QgsWFSFeatureIterator::featureReceivedSynchronous %1 features" ).arg( list.size() ) );
  QMutexLocker locker( &mMutex );
  if ( !mWriterStream )
  {
    mWriterStream = new QDataStream( &mWriterByteArray, QIODevice::WriteOnly );
  }
  Q_FOREACH ( const QgsWFSFeatureGmlIdPair& pair, list )
  {
    *mWriterStream << pair.first;
  }
  if ( !mWriterFile && mWriterByteArray.size() > mWriteTransferThreshold )
  {
    QString thisStr;
    thisStr.sprintf( "%p", this );
    ++ mCounter;
    mWriterFilename = QDir( QgsWFSUtils::acquireCacheDirectory() ).filePath( QString( "iterator_%1_%2.bin" ).arg( thisStr ).arg( mCounter ) );
    QgsDebugMsg( QString( "Transferring feature iterator cache to %1" ).arg( mWriterFilename ) );
    mWriterFile = new QFile( mWriterFilename );
    if ( !mWriterFile->open( QIODevice::WriteOnly ) )
    {
      QgsDebugMsg( QString( "Cannot open %1 for writing" ).arg( mWriterFilename ) );
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
  //QgsDebugMsg( QString("QgsWFSFeatureIterator::featureReceived %1 features").arg(featureCount) );
  if ( mLoop )
    mLoop->quit();
}

void QgsWFSFeatureIterator::checkInterruption()
{
  //QgsDebugMsg("QgsWFSFeatureIterator::checkInterruption()");

  if ( mInterruptionChecker && mInterruptionChecker->mustStop() )
  {
    mDownloadFinished = true;
    if ( mLoop )
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

    if ( !mShared->mGeometryAttribute.isEmpty() && mFetchGeometry )
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
         ( !constGeom || !constGeom->intersects( mRequest.filterRect() ) ) )
    {
      continue;
    }

    copyFeature( cachedFeature, f );
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
      // Instanciates the reader stream from memory buffer if not empty
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
          QgsDebugMsg( QString( "Cannot open %1" ).arg( mReaderFilename ) );
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
        if ( mInterruptionChecker && mInterruptionChecker->mustStop() )
          return false;

        QgsFeature feat;
        ( *mReaderStream ) >> feat;
        // We need to re-attach fields explictly
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

        const QgsGeometry* constGeom = feat.constGeometry();
        if ( !mRequest.filterRect().isNull() &&
             ( !constGeom || !constGeom->intersects( mRequest.filterRect() ) ) )
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
    if ( mInterruptionChecker && mInterruptionChecker->mustStop() )
      return false;

    //QgsDebugMsg("fetchFeature before loop");
    QEventLoop loop;
    mLoop = &loop;
    QTimer timer( this );
    timer.start( 50 );
    if ( mInterruptionChecker )
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
  QgsDebugMsg( QString( "qgsWFSFeatureIterator::close()" ) );

  iteratorClosed();

  mClosed = true;
  return true;
}


void QgsWFSFeatureIterator::copyFeature( const QgsFeature& srcFeature, QgsFeature& dstFeature )
{
  //copy the geometry
  const QgsGeometry* geometry = srcFeature.constGeometry();
  if ( !mShared->mGeometryAttribute.isEmpty() && geometry && !geometry->isEmpty() )
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
      int idx = srcFeature.fields()->indexFromName( fields.at( i ).name() );
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

