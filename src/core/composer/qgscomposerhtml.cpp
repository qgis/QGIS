/***************************************************************************
                              qgscomposerhtml.cpp
    ------------------------------------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgscomposition.h"
#include "qgsaddremovemultiframecommand.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmessagelog.h"
#include "qgsexpression.h"
#include "qgslogger.h"
#include "qgsnetworkcontentfetcher.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"

#include <QCoreApplication>
#include <QPainter>
#include <QWebFrame>
#include <QWebPage>
#include <QImage>
#include <QNetworkReply>

QgsComposerHtml::QgsComposerHtml( QgsComposition* c, bool createUndoCommands )
    : QgsComposerMultiFrame( c, createUndoCommands )
    , mContentMode( QgsComposerHtml::Url )
    , mWebPage( 0 )
    , mLoaded( false )
    , mHtmlUnitsToMM( 1.0 )
    , mRenderedPage( 0 )
    , mEvaluateExpressions( true )
    , mUseSmartBreaks( true )
    , mMaxBreakDistance( 10 )
    , mExpressionFeature( 0 )
    , mExpressionLayer( 0 )
    , mDistanceArea( 0 )
    , mEnableUserStylesheet( false )
    , mFetcher( 0 )
{
  mDistanceArea = new QgsDistanceArea();
  mHtmlUnitsToMM = htmlUnitsToMM();
  mWebPage = new QWebPage();
  mWebPage->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
  mWebPage->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );

  //This makes the background transparent. Found on http://blog.qt.digia.com/blog/2009/06/30/transparent-qwebview-or-qwebpage/
  QPalette palette = mWebPage->palette();
  palette.setBrush( QPalette::Base, Qt::transparent );
  mWebPage->setPalette( palette );

  mWebPage->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
  QObject::connect( mWebPage, SIGNAL( loadFinished( bool ) ), this, SLOT( frameLoaded( bool ) ) );
  if ( mComposition )
  {
    QObject::connect( mComposition, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SLOT( handleFrameRemoval( QgsComposerItem* ) ) );
  }

  // data defined strings
  mDataDefinedNames.insert( QgsComposerObject::SourceUrl, QString( "dataDefinedSourceUrl" ) );

  if ( mComposition && mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //a html item added while atlas preview is enabled needs to have the expression context set,
    //otherwise fields in the html aren't correctly evaluated until atlas preview feature changes (#9457)
    setExpressionContext( mComposition->atlasComposition().currentFeature(), mComposition->atlasComposition().coverageLayer() );
  }

  //connect to atlas feature changes
  //to update the expression context
  connect( &mComposition->atlasComposition(), SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( refreshExpressionContext() ) );

  mFetcher = new QgsNetworkContentFetcher();
  connect( mFetcher, SIGNAL( finished() ), this, SLOT( frameLoaded() ) );

}

QgsComposerHtml::QgsComposerHtml()
    : QgsComposerMultiFrame( 0, false )
    , mContentMode( QgsComposerHtml::Url )
    , mWebPage( 0 )
    , mLoaded( false )
    , mHtmlUnitsToMM( 1.0 )
    , mRenderedPage( 0 )
    , mUseSmartBreaks( true )
    , mMaxBreakDistance( 10 )
    , mExpressionFeature( 0 )
    , mExpressionLayer( 0 )
    , mDistanceArea( 0 )
    , mFetcher( 0 )
{
  mDistanceArea = new QgsDistanceArea();
  mFetcher = new QgsNetworkContentFetcher();
  connect( mFetcher, SIGNAL( finished() ), this, SLOT( frameLoaded() ) );
}

QgsComposerHtml::~QgsComposerHtml()
{
  delete mDistanceArea;
  delete mWebPage;
  delete mRenderedPage;
  mFetcher->deleteLater();
}

void QgsComposerHtml::setUrl( const QUrl& url )
{
  if ( !mWebPage )
  {
    return;
  }

  mUrl = url;
  loadHtml();
  emit changed();
}

void QgsComposerHtml::setHtml( const QString html )
{
  mHtml = html;
  //TODO - this signal should be emitted, but without changing the signal which sets the html
  //to an equivalent of editingFinished it causes a lot of problems. Need to investigate
  //ways of doing this using QScintilla widgets.
  //emit changed();
}

void QgsComposerHtml::setEvaluateExpressions( bool evaluateExpressions )
{
  mEvaluateExpressions = evaluateExpressions;
  loadHtml();
  emit changed();
}

void QgsComposerHtml::loadHtml()
{
  if ( !mWebPage )
  {
    return;
  }

  QString loadedHtml;
  switch ( mContentMode )
  {
    case QgsComposerHtml::Url:
    {

      QString currentUrl = mUrl.toString();

      //data defined url set?
      QVariant exprVal;
      if ( dataDefinedEvaluate( QgsComposerObject::SourceUrl, exprVal ) )
      {
        currentUrl = exprVal.toString().trimmed();;
        QgsDebugMsg( QString( "exprVal Source Url:%1" ).arg( currentUrl ) );
      }
      if ( currentUrl.isEmpty() )
      {
        return;
      }
      if ( currentUrl != mLastFetchedUrl )
      {
        loadedHtml = fetchHtml( QUrl( currentUrl ) );
        mLastFetchedUrl = currentUrl;
      }
      else
      {
        loadedHtml = mFetchedHtml;
      }

      break;
    }
    case QgsComposerHtml::ManualHtml:
      loadedHtml = mHtml;
      break;
  }

  //evaluate expressions
  if ( mEvaluateExpressions )
  {
    loadedHtml = QgsExpression::replaceExpressionText( loadedHtml, mExpressionFeature, mExpressionLayer, 0, mDistanceArea );
  }

  mLoaded = false;

  //reset page size. otherwise viewport size increases but never decreases again
  mWebPage->setViewportSize( QSize( maxFrameWidth() * mHtmlUnitsToMM, 0 ) );

  //set html, using the specified url as base if in Url mode
  mWebPage->mainFrame()->setHtml( loadedHtml, mContentMode == QgsComposerHtml::Url ? QUrl( mActualFetchedUrl ) : QUrl() );

  //set user stylesheet
  QWebSettings* settings = mWebPage->settings();
  if ( mEnableUserStylesheet && ! mUserStylesheet.isEmpty() )
  {
    QByteArray ba;
    ba.append( mUserStylesheet.toUtf8() );
    QUrl cssFileURL = QUrl( "data:text/css;charset=utf-8;base64," + ba.toBase64() );
    settings->setUserStyleSheetUrl( cssFileURL );
  }
  else
  {
    settings->setUserStyleSheetUrl( QUrl() );
  }

  while ( !mLoaded )
  {
    qApp->processEvents();
  }

  recalculateFrameSizes();
  //trigger a repaint
  emit contentsChanged();
}

void QgsComposerHtml::frameLoaded( bool ok )
{
  Q_UNUSED( ok );
  mLoaded = true;
}

double QgsComposerHtml::maxFrameWidth() const
{
  double maxWidth = 0;
  QList<QgsComposerFrame*>::const_iterator frameIt = mFrameItems.constBegin();
  for ( ; frameIt != mFrameItems.constEnd(); ++frameIt )
  {
    maxWidth = qMax( maxWidth, ( *frameIt )->boundingRect().width() );
  }

  return maxWidth;
}

void QgsComposerHtml::recalculateFrameSizes()
{
  if ( frameCount() < 1 ) return;

  QSize contentsSize = mWebPage->mainFrame()->contentsSize();

  //find maximum frame width
  double maxWidth = maxFrameWidth();
  //set content width to match maximum frame width
  contentsSize.setWidth( maxWidth * mHtmlUnitsToMM );

  mWebPage->setViewportSize( contentsSize );
  mSize.setWidth( contentsSize.width() / mHtmlUnitsToMM );
  mSize.setHeight( contentsSize.height() / mHtmlUnitsToMM );
  if ( contentsSize.isValid() )
  {
    renderCachedImage();
  }
  QgsComposerMultiFrame::recalculateFrameSizes();
  emit changed();
}

void QgsComposerHtml::renderCachedImage()
{
  //render page to cache image
  if ( mRenderedPage )
  {
    delete mRenderedPage;
  }
  mRenderedPage = new QImage( mWebPage->viewportSize(), QImage::Format_ARGB32 );
  if ( mRenderedPage->isNull() )
  {
    return;
  }
  QPainter painter;
  painter.begin( mRenderedPage );
  mWebPage->mainFrame()->render( &painter );
  painter.end();
}

QString QgsComposerHtml::fetchHtml( QUrl url )
{
  //pause until HTML fetch
  mLoaded = false;
  mFetcher->fetchContent( url );

  while ( !mLoaded )
  {
    qApp->processEvents();
  }

  mFetchedHtml = mFetcher->contentAsString();
  mActualFetchedUrl = mFetcher->reply()->url().toString();
  return mFetchedHtml;
}

QSizeF QgsComposerHtml::totalSize() const
{
  return mSize;
}

void QgsComposerHtml::render( QPainter* p, const QRectF& renderExtent, const int frameIndex )
{
  Q_UNUSED( frameIndex );

  if ( !mWebPage )
  {
    return;
  }

  p->save();
  p->setRenderHint( QPainter::Antialiasing );
  p->scale( 1.0 / mHtmlUnitsToMM, 1.0 / mHtmlUnitsToMM );
  p->translate( 0.0, -renderExtent.top() * mHtmlUnitsToMM );
  mWebPage->mainFrame()->render( p, QRegion( renderExtent.left(), renderExtent.top() * mHtmlUnitsToMM, renderExtent.width() * mHtmlUnitsToMM, renderExtent.height() * mHtmlUnitsToMM ) );
  p->restore();
}

double QgsComposerHtml::htmlUnitsToMM()
{
  if ( !mComposition )
  {
    return 1.0;
  }

  return ( mComposition->printResolution() / 72.0 ); //webkit seems to assume a standard dpi of 96
}

void QgsComposerHtml::addFrame( QgsComposerFrame* frame, bool recalcFrameSizes )
{
  mFrameItems.push_back( frame );
  QObject::connect( frame, SIGNAL( sizeChanged() ), this, SLOT( recalculateFrameSizes() ) );
  if ( mComposition )
  {
    mComposition->addComposerHtmlFrame( this, frame );
  }

  if ( recalcFrameSizes )
  {
    recalculateFrameSizes();
  }
}

bool candidateSort( const QPair<int, int> &c1, const QPair<int, int> &c2 )
{
  if ( c1.second < c2.second )
    return true;
  else if ( c1.second > c2.second )
    return false;
  else if ( c1.first > c2.first )
    return true;
  else
    return false;
}

double QgsComposerHtml::findNearbyPageBreak( double yPos )
{
  if ( !mWebPage || !mRenderedPage || !mUseSmartBreaks )
  {
    return yPos;
  }

  //convert yPos to pixels
  int idealPos = yPos * htmlUnitsToMM();

  //if ideal break pos is past end of page, there's nothing we need to do
  if ( idealPos >= mRenderedPage->height() )
  {
    return yPos;
  }

  int maxSearchDistance = mMaxBreakDistance * htmlUnitsToMM();

  //loop through all lines just before ideal break location, up to max distance
  //of maxSearchDistance
  int changes = 0;
  QRgb currentColor;
  QRgb pixelColor;
  QList< QPair<int, int> > candidates;
  int minRow = qMax( idealPos - maxSearchDistance, 0 );
  for ( int candidateRow = idealPos; candidateRow >= minRow; --candidateRow )
  {
    changes = 0;
    currentColor = qRgba( 0, 0, 0, 0 );
    //check all pixels in this line
    for ( int col = 0; col < mRenderedPage->width(); ++col )
    {
      //count how many times the pixels change color in this row
      //eventually, we select a row to break at with the minimum number of color changes
      //since this is likely a line break, or gap between table cells, etc
      //but very unlikely to be midway through a text line or picture
      pixelColor = mRenderedPage->pixel( col, candidateRow );
      if ( pixelColor != currentColor )
      {
        //color has changed
        currentColor = pixelColor;
        changes++;
      }
    }
    candidates.append( qMakePair( candidateRow, changes ) );
  }

  //sort candidate rows by number of changes ascending, row number descending
  qSort( candidates.begin(), candidates.end(), candidateSort );
  //first candidate is now the largest row with smallest number of changes

  //ok, now take the mid point of the best candidate position
  //we do this so that the spacing between text lines is likely to be split in half
  //otherwise the html will be broken immediately above a line of text, which
  //looks a little messy
  int maxCandidateRow = candidates[0].first;
  int minCandidateRow = maxCandidateRow + 1;
  int minCandidateChanges = candidates[0].second;

  QList< QPair<int, int> >::iterator it;
  for ( it = candidates.begin(); it != candidates.end(); ++it )
  {
    if (( *it ).second != minCandidateChanges || ( *it ).first != minCandidateRow - 1 )
    {
      //no longer in a consecutive block of rows of minimum pixel color changes
      //so return the row mid-way through the block
      //first converting back to mm
      return ( minCandidateRow + ( maxCandidateRow - minCandidateRow ) / 2 ) / htmlUnitsToMM();
    }
    minCandidateRow = ( *it ).first;
  }

  //above loop didn't work for some reason
  //return first candidate converted to mm
  return candidates[0].first / htmlUnitsToMM();
}

void QgsComposerHtml::setUseSmartBreaks( bool useSmartBreaks )
{
  mUseSmartBreaks = useSmartBreaks;
  recalculateFrameSizes();
  emit changed();
}

void QgsComposerHtml::setMaxBreakDistance( double maxBreakDistance )
{
  mMaxBreakDistance = maxBreakDistance;
  recalculateFrameSizes();
  emit changed();
}

void QgsComposerHtml::setUserStylesheet( const QString stylesheet )
{
  mUserStylesheet = stylesheet;
  //TODO - this signal should be emitted, but without changing the signal which sets the css
  //to an equivalent of editingFinished it causes a lot of problems. Need to investigate
  //ways of doing this using QScintilla widgets.
  //emit changed();
}

void QgsComposerHtml::setUserStylesheetEnabled( const bool stylesheetEnabled )
{
  if ( mEnableUserStylesheet != stylesheetEnabled )
  {
    mEnableUserStylesheet = stylesheetEnabled;
    loadHtml();
    emit changed();
  }
}

QString QgsComposerHtml::displayName() const
{
  return tr( "<html frame>" );
}

bool QgsComposerHtml::writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames ) const
{
  QDomElement htmlElem = doc.createElement( "ComposerHtml" );
  htmlElem.setAttribute( "contentMode", QString::number(( int ) mContentMode ) );
  htmlElem.setAttribute( "url", mUrl.toString() );
  htmlElem.setAttribute( "html", mHtml );
  htmlElem.setAttribute( "evaluateExpressions", mEvaluateExpressions ? "true" : "false" );
  htmlElem.setAttribute( "useSmartBreaks", mUseSmartBreaks ? "true" : "false" );
  htmlElem.setAttribute( "maxBreakDistance", QString::number( mMaxBreakDistance ) );
  htmlElem.setAttribute( "stylesheet", mUserStylesheet );
  htmlElem.setAttribute( "stylesheetEnabled", mEnableUserStylesheet ? "true" : "false" );

  bool state = _writeXML( htmlElem, doc, ignoreFrames );
  elem.appendChild( htmlElem );
  return state;
}

bool QgsComposerHtml::readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames )
{
  if ( !ignoreFrames )
  {
    deleteFrames();
  }

  //first create the frames
  if ( !_readXML( itemElem, doc, ignoreFrames ) )
  {
    return false;
  }

  bool contentModeOK;
  mContentMode = ( QgsComposerHtml::ContentMode )itemElem.attribute( "contentMode" ).toInt( &contentModeOK );
  if ( !contentModeOK )
  {
    mContentMode = QgsComposerHtml::Url;
  }
  mEvaluateExpressions = itemElem.attribute( "evaluateExpressions", "true" ) == "true" ? true : false;
  mUseSmartBreaks = itemElem.attribute( "useSmartBreaks", "true" ) == "true" ? true : false;
  mMaxBreakDistance = itemElem.attribute( "maxBreakDistance", "10" ).toDouble();
  mHtml = itemElem.attribute( "html" );
  mUserStylesheet = itemElem.attribute( "stylesheet" );
  mEnableUserStylesheet = itemElem.attribute( "stylesheetEnabled", "false" ) == "true" ? true : false;

  //finally load the set url
  QString urlString = itemElem.attribute( "url" );
  if ( !urlString.isEmpty() )
  {
    mUrl = urlString;
  }
  loadHtml();

  //since frames had to be created before, we need to emit a changed signal to refresh the widget
  emit changed();
  return true;
}

void QgsComposerHtml::setExpressionContext( QgsFeature* feature, QgsVectorLayer* layer )
{
  mExpressionFeature = feature;
  mExpressionLayer = layer;

  //setup distance area conversion
  if ( layer )
  {
    mDistanceArea->setSourceCrs( layer->crs().srsid() );
  }
  else if ( mComposition )
  {
    //set to composition's mapsettings' crs
    mDistanceArea->setSourceCrs( mComposition->mapSettings().destinationCrs().srsid() );
  }
  if ( mComposition )
  {
    mDistanceArea->setEllipsoidalMode( mComposition->mapSettings().hasCrsTransformEnabled() );
  }
  mDistanceArea->setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
}

void QgsComposerHtml::refreshExpressionContext()
{
  QgsVectorLayer * vl = 0;
  QgsFeature* feature = 0;

  if ( mComposition->atlasComposition().enabled() )
  {
    vl = mComposition->atlasComposition().coverageLayer();
  }
  if ( mComposition->atlasMode() != QgsComposition::AtlasOff )
  {
    feature = mComposition->atlasComposition().currentFeature();
  }

  setExpressionContext( feature, vl );
  loadHtml();
}

void QgsComposerHtml::refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property )
{
  //updates data defined properties and redraws item to match
  if ( property == QgsComposerObject::SourceUrl || property == QgsComposerObject::AllProperties )
  {
    loadHtml();
  }
  QgsComposerObject::refreshDataDefinedProperty( property );
}
