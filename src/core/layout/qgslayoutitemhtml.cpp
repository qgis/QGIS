/***************************************************************************
                              qgslayoutitemhtml.cpp
    ------------------------------------------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemhtml.h"
#include "qgslayoutframe.h"
#include "qgslayout.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmessagelog.h"
#include "qgsexpression.h"
#include "qgslogger.h"
#include "qgsnetworkcontentfetcher.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsdistancearea.h"
#include "qgsjsonutils.h"
#include "qgsmapsettings.h"
#include "qgswebpage.h"
#include "qgswebframe.h"
#include "qgslayoutitemmap.h"

#include <QCoreApplication>
#include <QPainter>
#include <QImage>
#include <QNetworkReply>
#include <QThread>
#include <QUrl>

// clazy:excludeall=lambda-in-connect

QgsLayoutItemHtml::QgsLayoutItemHtml( QgsLayout *layout )
  : QgsLayoutMultiFrame( layout )
{
  mHtmlUnitsToLayoutUnits = htmlUnitsToLayoutUnits();

  // only possible on the main thread!
  if ( QThread::currentThread() == QApplication::instance()->thread() )
  {
    mWebPage = std::make_unique< QgsWebPage >();
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Cannot load HTML content in background threads" ) );
  }

  if ( mWebPage )
  {
    mWebPage->setIdentifier( tr( "Layout HTML item" ) );
    mWebPage->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    mWebPage->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );

    //This makes the background transparent. Found on http://blog.qt.digia.com/blog/2009/06/30/transparent-qwebview-or-qwebpage/
    QPalette palette = mWebPage->palette();
    palette.setBrush( QPalette::Base, Qt::transparent );
    mWebPage->setPalette( palette );

    mWebPage->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
  }

  //a html item added to a layout needs to have the initial expression context set,
  //otherwise fields in the html aren't correctly evaluated until atlas preview feature changes (#9457)
  setExpressionContext( mLayout->reportContext().feature(), mLayout->reportContext().layer() );

  connect( &mLayout->reportContext(), &QgsLayoutReportContext::changed, this, &QgsLayoutItemHtml::refreshExpressionContext );

  mFetcher = new QgsNetworkContentFetcher();
}

QgsLayoutItemHtml::~QgsLayoutItemHtml()
{
  mFetcher->deleteLater();
}

int QgsLayoutItemHtml::type() const
{
  return QgsLayoutItemRegistry::LayoutHtml;
}

QIcon QgsLayoutItemHtml::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemHtml.svg" ) );
}

QgsLayoutItemHtml *QgsLayoutItemHtml::create( QgsLayout *layout )
{
  return new QgsLayoutItemHtml( layout );
}

void QgsLayoutItemHtml::setUrl( const QUrl &url )
{
  if ( !mWebPage )
  {
    return;
  }

  mUrl = url;
  loadHtml( true );
  emit changed();
}

void QgsLayoutItemHtml::setHtml( const QString &html )
{
  mHtml = html;
  //TODO - this signal should be emitted, but without changing the signal which sets the html
  //to an equivalent of editingFinished it causes a lot of problems. Need to investigate
  //ways of doing this using QScintilla widgets.
  //emit changed();
}

void QgsLayoutItemHtml::setEvaluateExpressions( bool evaluateExpressions )
{
  mEvaluateExpressions = evaluateExpressions;
  loadHtml( true );
  emit changed();
}

void QgsLayoutItemHtml::loadHtml( const bool useCache, const QgsExpressionContext *context )
{
  if ( !mWebPage )
  {
    return;
  }

  const QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  QString loadedHtml;
  switch ( mContentMode )
  {
    case QgsLayoutItemHtml::Url:
    {

      QString currentUrl = mUrl.toString();

      //data defined url set?
      bool ok = false;
      currentUrl = mDataDefinedProperties.valueAsString( QgsLayoutObject::SourceUrl, *evalContext, currentUrl, &ok );
      if ( ok )
      {
        currentUrl = currentUrl.trimmed();
        QgsDebugMsg( QStringLiteral( "exprVal Source Url:%1" ).arg( currentUrl ) );
      }
      if ( currentUrl.isEmpty() )
      {
        return;
      }
      if ( !( useCache && currentUrl == mLastFetchedUrl ) )
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
    case QgsLayoutItemHtml::ManualHtml:
      loadedHtml = mHtml;
      break;
  }

  //evaluate expressions
  if ( mEvaluateExpressions )
  {
    loadedHtml = QgsExpression::replaceExpressionText( loadedHtml, evalContext, &mDistanceArea );
  }

  bool loaded = false;

  QEventLoop loop;
  connect( mWebPage.get(), &QWebPage::loadFinished, &loop, [&loaded, &loop ] { loaded = true; loop.quit(); } );
  connect( mFetcher, &QgsNetworkContentFetcher::finished, &loop, [&loaded, &loop ] { loaded = true; loop.quit(); } );

  //reset page size. otherwise viewport size increases but never decreases again
  mWebPage->setViewportSize( QSize( maxFrameWidth() * mHtmlUnitsToLayoutUnits, 0 ) );

  //set html, using the specified url as base if in Url mode or the project file if in manual mode
  const QUrl baseUrl = mContentMode == QgsLayoutItemHtml::Url ?
                       QUrl( mActualFetchedUrl ) :
                       QUrl::fromLocalFile( mLayout->project()->absoluteFilePath() );

  mWebPage->mainFrame()->setHtml( loadedHtml, baseUrl );

  //set user stylesheet
  QWebSettings *settings = mWebPage->settings();
  if ( mEnableUserStylesheet && ! mUserStylesheet.isEmpty() )
  {
    QByteArray ba;
    ba.append( mUserStylesheet.toUtf8() );
    const QUrl cssFileURL = QUrl( QString( "data:text/css;charset=utf-8;base64," + ba.toBase64() ) );
    settings->setUserStyleSheetUrl( cssFileURL );
  }
  else
  {
    settings->setUserStyleSheetUrl( QUrl() );
  }

  if ( !loaded )
    loop.exec( QEventLoop::ExcludeUserInputEvents );

  //inject JSON feature
  if ( !mAtlasFeatureJSON.isEmpty() )
  {
    JavascriptExecutorLoop jsLoop;

    mWebPage->mainFrame()->addToJavaScriptWindowObject( QStringLiteral( "loop" ), &jsLoop );
    mWebPage->mainFrame()->evaluateJavaScript( QStringLiteral( "if ( typeof setFeature === \"function\" ) { try{ setFeature(%1); } catch (err) { loop.reportError(err.message); } }; loop.done();" ).arg( mAtlasFeatureJSON ) );

    jsLoop.execIfNotDone();
  }

  recalculateFrameSizes();
  //trigger a repaint
  emit contentsChanged();
}

double QgsLayoutItemHtml::maxFrameWidth() const
{
  double maxWidth = 0;
  for ( QgsLayoutFrame *frame : mFrameItems )
  {
    maxWidth = std::max( maxWidth, static_cast< double >( frame->boundingRect().width() ) );
  }

  return maxWidth;
}

void QgsLayoutItemHtml::recalculateFrameSizes()
{
  if ( frameCount() < 1 )
    return;

  if ( !mWebPage )
    return;

  QSize contentsSize = mWebPage->mainFrame()->contentsSize();

  //find maximum frame width
  const double maxWidth = maxFrameWidth();
  //set content width to match maximum frame width
  contentsSize.setWidth( maxWidth * mHtmlUnitsToLayoutUnits );

  mWebPage->setViewportSize( contentsSize );
  mSize.setWidth( contentsSize.width() / mHtmlUnitsToLayoutUnits );
  mSize.setHeight( contentsSize.height() / mHtmlUnitsToLayoutUnits );
  if ( contentsSize.isValid() )
  {
    renderCachedImage();
  }
  QgsLayoutMultiFrame::recalculateFrameSizes();
  emit changed();
}

void QgsLayoutItemHtml::renderCachedImage()
{
  if ( !mWebPage )
    return;

  //render page to cache image
  mRenderedPage = QImage( mWebPage->viewportSize(), QImage::Format_ARGB32 );
  if ( mRenderedPage.isNull() )
  {
    return;
  }
  mRenderedPage.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &mRenderedPage );
  mWebPage->mainFrame()->render( &painter );
  painter.end();
}

QString QgsLayoutItemHtml::fetchHtml( const QUrl &url )
{
  //pause until HTML fetch
  bool loaded = false;
  QEventLoop loop;
  connect( mFetcher, &QgsNetworkContentFetcher::finished, &loop, [&loaded, &loop ] { loaded = true; loop.quit(); } );
  mFetcher->fetchContent( url );

  if ( !loaded )
    loop.exec( QEventLoop::ExcludeUserInputEvents );

  mFetchedHtml = mFetcher->contentAsString();
  mActualFetchedUrl = mFetcher->reply()->url().toString();
  return mFetchedHtml;
}

QSizeF QgsLayoutItemHtml::totalSize() const
{
  return mSize;
}

void QgsLayoutItemHtml::render( QgsLayoutItemRenderContext &context, const QRectF &renderExtent, const int )
{
  if ( !mWebPage )
    return;

  QPainter *painter = context.renderContext().painter();
  const QgsScopedQPainterState painterState( painter );
  // painter is scaled to dots, so scale back to layout units
  painter->scale( context.renderContext().scaleFactor() / mHtmlUnitsToLayoutUnits, context.renderContext().scaleFactor() / mHtmlUnitsToLayoutUnits );
  painter->translate( 0.0, -renderExtent.top() * mHtmlUnitsToLayoutUnits );
  mWebPage->mainFrame()->render( painter, QRegion( renderExtent.left(), renderExtent.top() * mHtmlUnitsToLayoutUnits, renderExtent.width() * mHtmlUnitsToLayoutUnits, renderExtent.height() * mHtmlUnitsToLayoutUnits ) );
}

double QgsLayoutItemHtml::htmlUnitsToLayoutUnits()
{
  if ( !mLayout )
  {
    return 1.0;
  }

  return mLayout->convertToLayoutUnits( QgsLayoutMeasurement( mLayout->renderContext().dpi() / 72.0, QgsUnitTypes::LayoutMillimeters ) ); //webkit seems to assume a standard dpi of 96
}

bool candidateSort( QPair<int, int> c1, QPair<int, int> c2 )
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

double QgsLayoutItemHtml::findNearbyPageBreak( double yPos )
{
  if ( !mWebPage || mRenderedPage.isNull() || !mUseSmartBreaks )
  {
    return yPos;
  }

  //convert yPos to pixels
  const int idealPos = yPos * htmlUnitsToLayoutUnits();

  //if ideal break pos is past end of page, there's nothing we need to do
  if ( idealPos >= mRenderedPage.height() )
  {
    return yPos;
  }

  const int maxSearchDistance = mMaxBreakDistance * htmlUnitsToLayoutUnits();

  //loop through all lines just before ideal break location, up to max distance
  //of maxSearchDistance
  int changes = 0;
  QRgb currentColor;
  bool currentPixelTransparent = false;
  bool previousPixelTransparent = false;
  QRgb pixelColor;
  QList< QPair<int, int> > candidates;
  const int minRow = std::max( idealPos - maxSearchDistance, 0 );
  for ( int candidateRow = idealPos; candidateRow >= minRow; --candidateRow )
  {
    changes = 0;
    currentColor = qRgba( 0, 0, 0, 0 );
    //check all pixels in this line
    for ( int col = 0; col < mRenderedPage.width(); ++col )
    {
      //count how many times the pixels change color in this row
      //eventually, we select a row to break at with the minimum number of color changes
      //since this is likely a line break, or gap between table cells, etc
      //but very unlikely to be midway through a text line or picture
      pixelColor = mRenderedPage.pixel( col, candidateRow );
      currentPixelTransparent = qAlpha( pixelColor ) == 0;
      if ( pixelColor != currentColor && !( currentPixelTransparent && previousPixelTransparent ) )
      {
        //color has changed
        currentColor = pixelColor;
        changes++;
      }
      previousPixelTransparent = currentPixelTransparent;
    }
    candidates.append( qMakePair( candidateRow, changes ) );
  }

  //sort candidate rows by number of changes ascending, row number descending
  std::sort( candidates.begin(), candidates.end(), candidateSort );
  //first candidate is now the largest row with smallest number of changes

  //OK, now take the mid point of the best candidate position
  //we do this so that the spacing between text lines is likely to be split in half
  //otherwise the html will be broken immediately above a line of text, which
  //looks a little messy
  const int maxCandidateRow = candidates[0].first;
  int minCandidateRow = maxCandidateRow + 1;
  const int minCandidateChanges = candidates[0].second;

  QList< QPair<int, int> >::iterator it;
  for ( it = candidates.begin(); it != candidates.end(); ++it )
  {
    if ( ( *it ).second != minCandidateChanges || ( *it ).first != minCandidateRow - 1 )
    {
      //no longer in a consecutive block of rows of minimum pixel color changes
      //so return the row mid-way through the block
      //first converting back to mm
      return ( minCandidateRow + ( maxCandidateRow - minCandidateRow ) / 2 ) / htmlUnitsToLayoutUnits();
    }
    minCandidateRow = ( *it ).first;
  }

  //above loop didn't work for some reason
  //return first candidate converted to mm
  return candidates[0].first / htmlUnitsToLayoutUnits();
}

void QgsLayoutItemHtml::setUseSmartBreaks( bool useSmartBreaks )
{
  mUseSmartBreaks = useSmartBreaks;
  recalculateFrameSizes();
  emit changed();
}

void QgsLayoutItemHtml::setMaxBreakDistance( double maxBreakDistance )
{
  mMaxBreakDistance = maxBreakDistance;
  recalculateFrameSizes();
  emit changed();
}

void QgsLayoutItemHtml::setUserStylesheet( const QString &stylesheet )
{
  mUserStylesheet = stylesheet;
  //TODO - this signal should be emitted, but without changing the signal which sets the css
  //to an equivalent of editingFinished it causes a lot of problems. Need to investigate
  //ways of doing this using QScintilla widgets.
  //emit changed();
}

void QgsLayoutItemHtml::setUserStylesheetEnabled( const bool stylesheetEnabled )
{
  if ( mEnableUserStylesheet != stylesheetEnabled )
  {
    mEnableUserStylesheet = stylesheetEnabled;
    loadHtml( true );
    emit changed();
  }
}

QString QgsLayoutItemHtml::displayName() const
{
  return tr( "<HTML frame>" );
}

bool QgsLayoutItemHtml::writePropertiesToElement( QDomElement &htmlElem, QDomDocument &, const QgsReadWriteContext & ) const
{
  htmlElem.setAttribute( QStringLiteral( "contentMode" ), QString::number( static_cast< int >( mContentMode ) ) );
  htmlElem.setAttribute( QStringLiteral( "url" ), mUrl.toString() );
  htmlElem.setAttribute( QStringLiteral( "html" ), mHtml );
  htmlElem.setAttribute( QStringLiteral( "evaluateExpressions" ), mEvaluateExpressions ? "true" : "false" );
  htmlElem.setAttribute( QStringLiteral( "useSmartBreaks" ), mUseSmartBreaks ? "true" : "false" );
  htmlElem.setAttribute( QStringLiteral( "maxBreakDistance" ), QString::number( mMaxBreakDistance ) );
  htmlElem.setAttribute( QStringLiteral( "stylesheet" ), mUserStylesheet );
  htmlElem.setAttribute( QStringLiteral( "stylesheetEnabled" ), mEnableUserStylesheet ? "true" : "false" );
  return true;
}

bool QgsLayoutItemHtml::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext & )
{
  bool contentModeOK;
  mContentMode = static_cast< QgsLayoutItemHtml::ContentMode >( itemElem.attribute( QStringLiteral( "contentMode" ) ).toInt( &contentModeOK ) );
  if ( !contentModeOK )
  {
    mContentMode = QgsLayoutItemHtml::Url;
  }
  mEvaluateExpressions = itemElem.attribute( QStringLiteral( "evaluateExpressions" ), QStringLiteral( "true" ) ) == QLatin1String( "true" );
  mUseSmartBreaks = itemElem.attribute( QStringLiteral( "useSmartBreaks" ), QStringLiteral( "true" ) ) == QLatin1String( "true" );
  mMaxBreakDistance = itemElem.attribute( QStringLiteral( "maxBreakDistance" ), QStringLiteral( "10" ) ).toDouble();
  mHtml = itemElem.attribute( QStringLiteral( "html" ) );
  mUserStylesheet = itemElem.attribute( QStringLiteral( "stylesheet" ) );
  mEnableUserStylesheet = itemElem.attribute( QStringLiteral( "stylesheetEnabled" ), QStringLiteral( "false" ) ) == QLatin1String( "true" );

  //finally load the set url
  const QString urlString = itemElem.attribute( QStringLiteral( "url" ) );
  if ( !urlString.isEmpty() )
  {
    mUrl = urlString;
  }
  loadHtml( true );

  //since frames had to be created before, we need to emit a changed signal to refresh the widget
  emit changed();
  return true;
}

void QgsLayoutItemHtml::setExpressionContext( const QgsFeature &feature, QgsVectorLayer *layer )
{
  mExpressionFeature = feature;
  mExpressionLayer = layer;

  //setup distance area conversion
  if ( layer )
  {
    mDistanceArea.setSourceCrs( layer->crs(), mLayout->project()->transformContext() );
  }
  else if ( mLayout )
  {
    //set to composition's mapsettings' crs
    QgsLayoutItemMap *referenceMap = mLayout->referenceMap();
    if ( referenceMap )
      mDistanceArea.setSourceCrs( referenceMap->crs(), mLayout->project()->transformContext() );
  }
  if ( mLayout )
  {
    mDistanceArea.setEllipsoid( mLayout->project()->ellipsoid() );
  }

  if ( feature.isValid() )
  {
    // create JSON representation of feature
    QgsJsonExporter exporter( layer );
    exporter.setIncludeRelated( true );
    mAtlasFeatureJSON = exporter.exportFeature( feature );
  }
  else
  {
    mAtlasFeatureJSON.clear();
  }
}

void QgsLayoutItemHtml::refreshExpressionContext()
{
  QgsVectorLayer *vl = nullptr;
  QgsFeature feature;

  if ( mLayout )
  {
    vl = mLayout->reportContext().layer();
    feature = mLayout->reportContext().feature();
  }

  setExpressionContext( feature, vl );
  loadHtml( true );
}

void QgsLayoutItemHtml::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  const QgsExpressionContext context = createExpressionContext();

  //updates data defined properties and redraws item to match
  if ( property == QgsLayoutObject::SourceUrl || property == QgsLayoutObject::AllProperties )
  {
    loadHtml( true, &context );
  }
}

//JavascriptExecutorLoop
///@cond PRIVATE

void JavascriptExecutorLoop::done()
{
  mDone = true;
  quit();
}

void JavascriptExecutorLoop::execIfNotDone()
{
  if ( !mDone )
    exec( QEventLoop::ExcludeUserInputEvents );

  // gross, but nothing else works, so f*** it.. it's not worth spending a day trying to find a non-hacky way
  // to force the web page to update following the js execution
  for ( int i = 0; i < 100; i++ )
    qApp->processEvents();
}

void JavascriptExecutorLoop::reportError( const QString &error )
{
  mDone = true;
  QgsMessageLog::logMessage( tr( "HTML setFeature function error: %1" ).arg( error ), tr( "Layout" ) );
  quit();
}

///@endcond
