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

#include <nlohmann/json.hpp>

#include "qgsdistancearea.h"
#include "qgsexpression.h"
#include "qgsjsonutils.h"
#include "qgslayout.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayoututils.h"
#include "qgslogger.h"
#include "qgsmapsettings.h"
#include "qgsnetworkcontentfetcher.h"
#include "qgspainting.h"
#include "qgspicturerasterizer.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgswebenginepage.h"

#include <QString>

#if defined( HAVE_WEBENGINE ) && defined( HAVE_PDF4QT )
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#endif

#include <QAbstractTextDocumentLayout>
#include <QCoreApplication>
#include <QImage>
#include <QNetworkReply>
#include <QPainter>
#include <QString>
#include <QThread>
#include <QUrl>
#include <QtConcurrentRun>
#include <QTimer>

#include "moc_qgslayoutitemhtml.cpp"

using namespace Qt::StringLiterals;

// clazy:excludeall=lambda-in-connect

QgsLayoutItemHtml::QgsLayoutItemHtml( QgsLayout *layout )
  : QgsLayoutMultiFrame( layout )
  , mPictureRasterizer( new QgsPictureRasterizer( this ) )
{
  mPictureRasterizer->setMaxCacheSize( 10000 );

  mBackgroundUpdateTimer = new QTimer( this );
  mBackgroundUpdateTimer->setSingleShot( true );
  connect( mBackgroundUpdateTimer, &QTimer::timeout, this, &QgsLayoutItemHtml::recreateCachedImageInBackground );

  connect( mPictureRasterizer, &QgsPictureRasterizer::rendered, this, [this] {
    updateFrames();
    emit previewRefreshed();
  } );

  //a html item added to a layout needs to have the initial expression context set,
  //otherwise fields in the html aren't correctly evaluated until atlas preview feature changes (#9457)
  setExpressionContext( mLayout->reportContext().feature(), mLayout->reportContext().layer() );

  connect( mLayout, &QgsLayout::refreshed, this, &QgsLayoutItemHtml::invalidateCache );
  connect( &mLayout->reportContext(), &QgsLayoutReportContext::changed, this, &QgsLayoutItemHtml::refreshExpressionContext );

  mFetcher = new QgsNetworkContentFetcher( this );
  connect( mFetcher, &QgsNetworkContentFetcher::finished, this, &QgsLayoutItemHtml::onHtmlFetched );
}

QgsLayoutItemHtml::~QgsLayoutItemHtml()
{
  mBackgroundUpdateTimer->stop();
  cancelPendingFetch();
  cancelRunningRenderJob( true );
  emit backgroundTaskCountChanged( 0 );
}

int QgsLayoutItemHtml::type() const
{
  return QgsLayoutItemRegistry::LayoutHtml;
}

QIcon QgsLayoutItemHtml::icon() const
{
  return QgsApplication::getThemeIcon( u"/mLayoutItemHtml.svg"_s );
}

QgsLayoutItemHtml *QgsLayoutItemHtml::create( QgsLayout *layout )
{
  return new QgsLayoutItemHtml( layout );
}

QgsLayoutItemHtml *QgsLayoutItemHtml::createFromLabel( QgsLayoutItemLabel *label )
{
  QgsLayoutItemHtml *html = new QgsLayoutItemHtml( label->layout() );
  QgsLayoutFrame *frame = new QgsLayoutFrame( label->layout(), html );
  frame->setVisible( label->isVisible() );
  frame->setLocked( label->isLocked() );
  frame->setItemOpacity( label->itemOpacity() );
  frame->setRotation( label->rotation() );
  frame->setReferencePoint( label->referencePoint() );
  frame->attemptMove( label->positionWithUnits() );
  frame->attemptResize( label->sizeWithUnits() );
  frame->setZValue( label->zValue() );
  frame->setParentGroup( label->parentGroup() );
  frame->setBackgroundColor( label->backgroundColor() );
  frame->setFrameEnabled( label->frameEnabled() );
  frame->setFrameJoinStyle( label->frameJoinStyle() );
  frame->setFrameStrokeWidth( label->frameStrokeWidth() );
  frame->setFrameStrokeColor( label->frameStrokeColor() );
  html->addFrame( frame );
  html->setContentMode( QgsLayoutItemHtml::ManualHtml );
  html->setHtml( label->currentText() );
  html->setUserStylesheetEnabled( true );
  html->setUserStylesheet( label->createStylesheet() );
  html->loadHtml();
  return html;
}

void QgsLayoutItemHtml::setUrl( const QUrl &url )
{
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
      currentUrl = mDataDefinedProperties.valueAsString( QgsLayoutObject::DataDefinedProperty::SourceUrl, *evalContext, currentUrl, &ok );
      if ( ok )
      {
        currentUrl = currentUrl.trimmed();
        QgsDebugMsgLevel( u"exprVal Source Url:%1"_s.arg( currentUrl ), 2 );
      }
      if ( currentUrl.isEmpty() )
      {
        cancelPendingFetch();
        finalizeLoadedHtml( QString(), QUrl(), evalContext );
        return;
      }

      if ( useCache && mHasFetchedHtml && currentUrl == mLastFetchedUrl )
      {
        cancelPendingFetch();
        finalizeLoadedHtml( mFetchedHtml, QUrl( mActualFetchedUrl ), evalContext );
      }
      else
      {
        if ( !( useCache && mFetching && currentUrl == mPendingFetchUrl ) )
        {
          cancelPendingFetch();
          mBackgroundUpdateTimer->stop();
          cancelRunningRenderJob( false );

          mPendingFetchUrl = currentUrl;
          mPendingExpressionContext = context ? std::optional< QgsExpressionContext >( *context ) : std::nullopt;
          mCacheInvalidated = true;
          startFetchHtml( QUrl( currentUrl ) );
          updateFrames();
        }
        else if ( context )
        {
          mPendingExpressionContext = *context;
        }

        if ( !mLayout || !mLayout->renderContext().isPreviewRender() )
        {
          // if not a preview render, we HAVE to wait till we've loaded content
          waitForPendingFetch();
        }
      }

      break;
    }
    case QgsLayoutItemHtml::ManualHtml:
      cancelPendingFetch();
      const QUrl baseUrl = ( mLayout && mLayout->project() ) ? QUrl::fromLocalFile( mLayout->project()->absoluteFilePath() ) : QUrl();
      finalizeLoadedHtml( mHtml, baseUrl, evalContext );
      break;
  }
}

void QgsLayoutItemHtml::startFetchHtml( const QUrl &url )
{
  emit backgroundTaskCountChanged( 1 );
  mFetching = true;
  mFetcher->fetchContent( url );
}

void QgsLayoutItemHtml::cancelPendingFetch()
{
  if ( !mFetching )
    return;

  mFetching = false;
  mPendingFetchUrl.clear();
  mPendingExpressionContext.reset();

  mFetcher->disconnect( this );
  mFetcher->cancel();
  mFetcher->deleteLater();
  mFetcher = new QgsNetworkContentFetcher( this );
  connect( mFetcher, &QgsNetworkContentFetcher::finished, this, &QgsLayoutItemHtml::onHtmlFetched );
  emit backgroundTaskCountChanged( 0 );
}

void QgsLayoutItemHtml::waitForPendingFetch()
{
  if ( !mFetching )
    return;

  QEventLoop loop;
  connect( mFetcher, &QgsNetworkContentFetcher::finished, &loop, &QEventLoop::quit );
  if ( mFetching )
  {
    loop.exec( QEventLoop::ExcludeUserInputEvents );
  }
}

void QgsLayoutItemHtml::onHtmlFetched()
{
  if ( !mFetching )
    return;

  emit backgroundTaskCountChanged( 0 );
  mFetching = false;
  mFetchedHtml = mFetcher->contentAsString();
  mActualFetchedUrl = mFetcher->reply() ? mFetcher->reply()->url().toString() : mPendingFetchUrl;
  mLastFetchedUrl = mPendingFetchUrl;
  mHasFetchedHtml = true;
  mPendingFetchUrl.clear();

  const std::optional< QgsExpressionContext > savedContext = std::move( mPendingExpressionContext );
  mPendingExpressionContext = {};

  const QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = savedContext.has_value() ? &*savedContext : &scopedContext;
  finalizeLoadedHtml( mFetchedHtml, QUrl( mActualFetchedUrl ), evalContext );
}

void QgsLayoutItemHtml::finalizeLoadedHtml( const QString &rawHtml, const QUrl &baseUrl, const QgsExpressionContext *context )
{
  QString loadedHtml = rawHtml;
  if ( mEvaluateExpressions && !loadedHtml.isEmpty() )
  {
    loadedHtml = QgsExpression::replaceExpressionText( loadedHtml, context, &mDistanceArea );
  }

  mEvaluatedHtml = loadedHtml;
  mResolvedBaseUrl = baseUrl;
  mCacheInvalidated = true;

  if ( !mLayout || !mLayout->renderContext().isPreviewRender() )
  {
    mBackgroundUpdateTimer->stop();
    cancelRunningRenderJob( false );
    renderHtmlSynchronously();
    emit backgroundTaskCountChanged( 0 );
  }
  else
  {
    // render HTML in a background thread, so we don't block the user interface
    mBackgroundUpdateTimer->start( 1 );
  }
}

void QgsLayoutItemHtml::recreateCachedImageInBackground()
{
  if ( mFetching )
    return;

  cancelRunningRenderJob( false );

  const double frameWidthLayoutUnits = maxFrameWidth();
  if ( frameWidthLayoutUnits <= 0 || mEvaluatedHtml.isEmpty() )
  {
    mCacheInvalidated = false;
    mLastRenderedWidth = frameWidthLayoutUnits;
    mSize = QSizeF( 0, 0 );
    mRenderedPicture = QPicture();
    mRenderedPage = QImage();
    mPictureRasterizer->clear();
    recalculateFrameSizes();
    emit contentsChanged();
    updateFrames();
    emit previewRefreshed();
    emit backgroundTaskCountChanged( 0 );
    return;
  }

  emit backgroundTaskCountChanged( 1 );
  mCacheInvalidated = false;
  mRenderJob = std::make_unique<
    QgsLayoutItemHtmlRenderJob >( mEvaluatedHtml, mResolvedBaseUrl, mEnableUserStylesheet ? mUserStylesheet : QString(), mAtlasFeatureJSON, frameWidthLayoutUnits, layoutUnitsPerCssPixel() );
  connect( mRenderJob.get(), &QgsLayoutItemHtmlRenderJob::finished, this, &QgsLayoutItemHtml::htmlRenderFinished );
  mRenderJob->start();
}

void QgsLayoutItemHtml::htmlRenderFinished()
{
  if ( !mRenderJob )
    return;

  if ( !mRenderJob->success() && !mRenderJob->errorMessage().isEmpty() )
  {
    QgsDebugError( mRenderJob->errorMessage() );
  }

  mLastRenderedWidth = mRenderJob->frameWidthLayoutUnits();
  mSize = mRenderJob->size();
  mRenderedPicture = QgsPainting::deepCopyQPicture( mRenderJob->picture() );
  mRenderedPage = mRenderJob->renderedPage();
  mRenderJob.reset();

  mPictureRasterizer->setPicture( mRenderedPicture );

  emit backgroundTaskCountChanged( 0 );

  recalculateFrameSizes();
  emit contentsChanged();
  updateFrames();
  emit previewRefreshed();
}

void QgsLayoutItemHtml::renderHtmlSynchronously()
{
  if ( mFetching )
  {
    waitForPendingFetch();
    return;
  }

  const double frameWidthLayoutUnits = maxFrameWidth();
  mLastRenderedWidth = frameWidthLayoutUnits;
  mCacheInvalidated = false;

  if ( frameWidthLayoutUnits <= 0 || mEvaluatedHtml.isEmpty() )
  {
    mSize = QSizeF( 0, 0 );
    mRenderedPicture = QPicture();
    mRenderedPage = QImage();
    mPictureRasterizer->clear();
    recalculateFrameSizes();
    emit contentsChanged();
    return;
  }

  const double unitsPerCssPx = layoutUnitsPerCssPixel();
  const double widthCssPx = frameWidthLayoutUnits / unitsPerCssPx;

  QString errorMessage;
  mRenderedPicture = QPicture();
  QPainter painter( &mRenderedPicture );
  painter.setRenderHint( QPainter::Antialiasing, true );
  painter.setRenderHint( QPainter::TextAntialiasing, true );
  painter.setRenderHint( QPainter::SmoothPixmapTransform, true );

  QSizeF renderedSizeCssPx;

  QgsHtmlRenderSettings htmlRenderSettings;
  htmlRenderSettings.setBaseUrl( mResolvedBaseUrl );
  if ( mEnableUserStylesheet )
  {
    htmlRenderSettings.setUserStylesheet( mUserStylesheet );
  }
  htmlRenderSettings.setFeatureJson( mAtlasFeatureJSON );

#if defined( HAVE_WEBENGINE ) && defined( HAVE_PDF4QT )
  if ( !QgsWebEnginePage::renderBlocking( mEvaluatedHtml, &painter, QRectF( 0, 0, widthCssPx, 9999 ), errorMessage, renderedSizeCssPx, htmlRenderSettings, nullptr ) )
  {
    QgsDebugError( errorMessage );
  }
#endif
  painter.end();

  const double heightCssPx = renderedSizeCssPx.height();
  mRenderedPicture.setBoundingRect( QRect( 0, 0, static_cast< int >( std::ceil( widthCssPx ) ), static_cast< int >( std::ceil( heightCssPx ) ) ) );

  mSize = QSizeF( frameWidthLayoutUnits, heightCssPx * unitsPerCssPx );

  const QSize pageSizeCssPx( static_cast< int >( std::ceil( widthCssPx ) ), static_cast< int >( std::ceil( heightCssPx ) ) );
  if ( mUseSmartBreaks && !pageSizeCssPx.isEmpty() )
  {
    mRenderedPage = QImage( pageSizeCssPx, QImage::Format_ARGB32 );
    mRenderedPage.fill( Qt::transparent );
    QPainter imagePainter( &mRenderedPage );
    QgsPainting::drawPicture( &imagePainter, QPointF( 0, 0 ), mRenderedPicture );
    imagePainter.end();
  }
  else
  {
    mRenderedPage = QImage();
  }

  mPictureRasterizer->setPicture( mRenderedPicture );

  recalculateFrameSizes();
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

  if ( !mFetching )
  {
    const double currentWidth = maxFrameWidth();
    if ( !qgsDoubleNear( currentWidth, mLastRenderedWidth ) && !mEvaluatedHtml.isEmpty() )
    {
      mCacheInvalidated = true;
      if ( !mLayout || !mLayout->renderContext().isPreviewRender() )
      {
        mBackgroundUpdateTimer->stop();
        cancelRunningRenderJob( false );
        renderHtmlSynchronously();
        return;
      }
      else
      {
        mBackgroundUpdateTimer->start( 1 );
      }
    }
  }

  QgsLayoutMultiFrame::recalculateFrameSizes();
  emit changed();
}

QSizeF QgsLayoutItemHtml::totalSize() const
{
  return mSize;
}

void QgsLayoutItemHtml::render( QgsLayoutItemRenderContext &context, const QRectF &renderExtent, const int frameIndex )
{
#if defined( HAVE_WEBENGINE ) && defined( HAVE_PDF4QT )
  QPainter *painter = context.renderContext().painter();
  if ( !painter )
    return;

  if ( mLayout->renderContext().isPreviewRender() )
  {
    const double currentScaleFactor = context.renderContext().scaleFactor();

    if ( !mFetching && ( mCacheInvalidated || !qgsDoubleNear( maxFrameWidth(), mLastRenderedWidth ) ) && !mRenderJob && !mBackgroundUpdateTimer->isActive() && !mEvaluatedHtml.isEmpty() )
    {
      mBackgroundUpdateTimer->start( 1 );
    }

    // main thread preview render ONLY draws the rasterized QImage from QgsPictureRasterizer
    const QSize targetPixelSize = ( mSize * currentScaleFactor ).toSize();
    const QImage previewImage = mPictureRasterizer->image( targetPixelSize );

    if ( !previewImage.isNull() && !mSize.isEmpty() )
    {
      const QgsScopedQPainterState painterState( painter );
      painter->scale( currentScaleFactor, currentScaleFactor );
      painter->translate( -renderExtent.left(), -renderExtent.top() );
      painter->setClipRect( renderExtent, Qt::IntersectClip );
      painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
      painter->drawImage( QRectF( QPointF( 0, 0 ), mSize ), previewImage );
    }
    else if ( mFetching || mRenderJob || mBackgroundUpdateTimer->isActive() || mPictureRasterizer->isRendering() )
    {
      if ( QgsLayoutFrame *currentFrame = frame( frameIndex ) )
      {
        const QgsScopedQPainterState painterState( painter );
        painter->scale( currentScaleFactor, currentScaleFactor );
        const QRectF frameRect( 0, 0, currentFrame->rect().width(), currentFrame->rect().height() );
        painter->setBrush( QBrush( QColor( 125, 125, 125, 125 ) ) );
        painter->setPen( Qt::NoPen );
        painter->drawRect( frameRect );
        painter->setBrush( Qt::NoBrush );

        QFont messageFont;
        messageFont.setPointSize( 12 );
        painter->setFont( messageFont );
        painter->setPen( QColor( 255, 255, 255, 255 ) );
        painter->drawText( frameRect, Qt::AlignCenter, tr( "Rendering HTML" ) );
      }
    }
    return;
  }

  if ( mFetching )
  {
    waitForPendingFetch();
  }

  if ( mCacheInvalidated || mRenderJob || !qgsDoubleNear( maxFrameWidth(), mLastRenderedWidth ) )
  {
    mBackgroundUpdateTimer->stop();
    cancelRunningRenderJob( false );
    renderHtmlSynchronously();
  }

  const QgsScopedQPainterState painterState( painter );
  painter->setRenderHint( QPainter::Antialiasing );
  painter->setRenderHint( QPainter::SmoothPixmapTransform );

  // painter is scaled to dots; scale back to layout units
  painter->scale( context.renderContext().scaleFactor(), context.renderContext().scaleFactor() );
  painter->translate( -renderExtent.left(), -renderExtent.top() );
  painter->setClipRect( renderExtent, Qt::IntersectClip );
  // scale from CSS pixels (mRenderedPicture's coordinate space) to layout units
  const double unitsPerCssPx = layoutUnitsPerCssPixel();
  painter->scale( unitsPerCssPx, unitsPerCssPx );

  QgsPainting::drawPicture( painter, QPointF( 0, 0 ), mRenderedPicture );
#else
  Q_UNUSED( renderExtent )
  if ( mLayout->renderContext().isPreviewRender() )
  {
    if ( QgsLayoutFrame *currentFrame = frame( frameIndex ) )
    {
      QPainter *painter = context.renderContext().painter();

      // painter is scaled to dots, so scale back to layout units
      const QRectF painterRect = QRectF(
        currentFrame->rect().left() * context.renderContext().scaleFactor(),
        currentFrame->rect().top() * context.renderContext().scaleFactor(),
        currentFrame->rect().width() * context.renderContext().scaleFactor(),
        currentFrame->rect().height() * context.renderContext().scaleFactor()
      );

      painter->setBrush( QBrush( QColor( 255, 125, 125, 125 ) ) );
      painter->setPen( Qt::NoPen );
      painter->drawRect( painterRect );
      painter->setBrush( Qt::NoBrush );

      painter->setPen( QColor( 200, 0, 0, 255 ) );
      QTextDocument td;
      td.setTextWidth( painterRect.width() );
      td.setHtml(
        u"<span style=\"color: rgb(200,0,0);\"><b>%1</b><br>%2</span>"_s.arg( tr( "WebKit not available!" ), tr( "The item cannot be rendered because this QGIS install was built without WebKit support." ) )
      );
      painter->setClipRect( painterRect );
      QAbstractTextDocumentLayout::PaintContext ctx;
      td.documentLayout()->draw( painter, ctx );
    }
  }
#endif
}

double QgsLayoutItemHtml::layoutUnitsPerCssPixel() const
{
  if ( !mLayout )
    return 25.4 / 96.0;

  return mLayout->convertToLayoutUnits( QgsLayoutMeasurement( 1.0, Qgis::LayoutUnit::Inches ) ) / 96.0;
}

void QgsLayoutItemHtml::cancelRunningRenderJob( bool block )
{
  if ( !mRenderJob )
    return;

  disconnect( mRenderJob.get(), &QgsLayoutItemHtmlRenderJob::finished, this, &QgsLayoutItemHtml::htmlRenderFinished );
  if ( block )
  {
    mRenderJob->cancelAndWait();
    mRenderJob.reset();
  }
  else
  {
    QgsLayoutItemHtmlRenderJob *oldJob = mRenderJob.release();
    connect( oldJob, &QgsLayoutItemHtmlRenderJob::finished, oldJob, &QObject::deleteLater );
    oldJob->cancelWithoutBlocking();
  }
}

void QgsLayoutItemHtml::updateFrames()
{
  for ( QgsLayoutFrame *frame : std::as_const( mFrameItems ) )
  {
    if ( frame )
      frame->update();
  }
}

void QgsLayoutItemHtml::invalidateCache()
{
  mPictureRasterizer->invalidate();
  updateFrames();
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
  if ( mRenderedPage.isNull() || !mUseSmartBreaks )
  {
    return yPos;
  }

  const double unitsPerCssPx = layoutUnitsPerCssPixel();
  //convert yPos to pixels
  const int idealPos = static_cast< int >( yPos / unitsPerCssPx );

  //if ideal break pos is past end of page, there's nothing we need to do
  if ( idealPos >= mRenderedPage.height() )
  {
    return yPos;
  }

  const int maxSearchDistance = static_cast< int >( mMaxBreakDistance / unitsPerCssPx );

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
      //first converting back to layout units
      return ( minCandidateRow + ( maxCandidateRow - minCandidateRow ) / 2 ) * unitsPerCssPx;
    }
    minCandidateRow = ( *it ).first;
  }

  //above loop didn't work for some reason
  //return first candidate converted to layout units
  return candidates[0].first * unitsPerCssPx;
}

void QgsLayoutItemHtml::setUseSmartBreaks( bool useSmartBreaks )
{
  if ( mUseSmartBreaks == useSmartBreaks )
    return;

  mUseSmartBreaks = useSmartBreaks;
  if ( mUseSmartBreaks && mRenderedPage.isNull() && !mRenderedPicture.isNull() && !mSize.isEmpty() )
  {
    const double unitsPerCssPx = layoutUnitsPerCssPixel();
    const QSize pageSizeCssPx( static_cast< int >( std::ceil( mSize.width() / unitsPerCssPx ) ), static_cast< int >( std::ceil( mSize.height() / unitsPerCssPx ) ) );
    if ( !pageSizeCssPx.isEmpty() )
    {
      mRenderedPage = QImage( pageSizeCssPx, QImage::Format_ARGB32 );
      mRenderedPage.fill( Qt::transparent );
      QPainter imagePainter( &mRenderedPage );
      QgsPainting::drawPicture( &imagePainter, QPointF( 0, 0 ), mRenderedPicture );
      imagePainter.end();
    }
  }

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
  htmlElem.setAttribute( u"contentMode"_s, QString::number( static_cast< int >( mContentMode ) ) );
  htmlElem.setAttribute( u"url"_s, mUrl.toString() );
  htmlElem.setAttribute( u"html"_s, mHtml );
  htmlElem.setAttribute( u"evaluateExpressions"_s, mEvaluateExpressions ? "true" : "false" );
  htmlElem.setAttribute( u"useSmartBreaks"_s, mUseSmartBreaks ? "true" : "false" );
  htmlElem.setAttribute( u"maxBreakDistance"_s, QString::number( mMaxBreakDistance ) );
  htmlElem.setAttribute( u"stylesheet"_s, mUserStylesheet );
  htmlElem.setAttribute( u"stylesheetEnabled"_s, mEnableUserStylesheet ? "true" : "false" );
  return true;
}

bool QgsLayoutItemHtml::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext & )
{
  bool contentModeOK;
  mContentMode = static_cast< QgsLayoutItemHtml::ContentMode >( itemElem.attribute( u"contentMode"_s ).toInt( &contentModeOK ) );
  if ( !contentModeOK )
  {
    mContentMode = QgsLayoutItemHtml::Url;
  }
  mEvaluateExpressions = itemElem.attribute( u"evaluateExpressions"_s, u"true"_s ) == "true"_L1;
  mUseSmartBreaks = itemElem.attribute( u"useSmartBreaks"_s, u"true"_s ) == "true"_L1;
  mMaxBreakDistance = itemElem.attribute( u"maxBreakDistance"_s, u"10"_s ).toDouble();
  mHtml = itemElem.attribute( u"html"_s );
  mUserStylesheet = itemElem.attribute( u"stylesheet"_s );
  mEnableUserStylesheet = itemElem.attribute( u"stylesheetEnabled"_s, u"false"_s ) == "true"_L1;

  //finally load the set url
  const QString urlString = itemElem.attribute( u"url"_s );
  if ( !urlString.isEmpty() )
  {
    mUrl = urlString;
  }

  //since frames had to be created before, we need to emit a changed signal to refresh the widget
  emit changed();
  return true;
}

bool QgsLayoutItemHtml::renderInProgress() const
{
  return mFetching || mRenderJob || mBackgroundUpdateTimer->isActive() || ( mPictureRasterizer && mPictureRasterizer->isRendering() );
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
    mAtlasFeatureJSON = QgsJsonUtils::jsonToVariant( exporter.exportFeatureToJsonObject( feature ) ).toMap();
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
  if ( property == QgsLayoutObject::DataDefinedProperty::SourceUrl || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    loadHtml( true, &context );
  }
}


//
// QgsLayoutItemHtmlRenderJob
//
///@cond PRIVATE
QgsLayoutItemHtmlRenderJob::QgsLayoutItemHtmlRenderJob(
  const QString &html, const QUrl &baseUrl, const QString &userStylesheet, const QVariantMap &featureJson, double frameWidthLayoutUnits, double unitsPerCssPx
)
  : mHtml( html )
  , mBaseUrl( baseUrl )
  , mUserStylesheet( userStylesheet )
  , mFeatureJson( featureJson )
  , mFrameWidthLayoutUnits( frameWidthLayoutUnits )
  , mUnitsPerCssPx( unitsPerCssPx )
{
  connect( &mWatcher, &QFutureWatcher< void >::finished, this, &QgsLayoutItemHtmlRenderJob::finished );
}

void QgsLayoutItemHtmlRenderJob::start()
{
  mWatcher.setFuture( QtConcurrent::run( [this]() { run(); } ) );
}

void QgsLayoutItemHtmlRenderJob::cancelWithoutBlocking()
{
  mFeedback.cancel();
}

void QgsLayoutItemHtmlRenderJob::cancelAndWait()
{
  mFeedback.cancel();
  mWatcher.waitForFinished();
}

bool QgsLayoutItemHtmlRenderJob::isCanceled() const
{
  return mFeedback.isCanceled();
}

void QgsLayoutItemHtmlRenderJob::run()
{
#if defined( HAVE_WEBENGINE ) && defined( HAVE_PDF4QT )
  const double widthCssPx = mFrameWidthLayoutUnits / mUnitsPerCssPx;

  QPainter painter( &mPicture );
  QSizeF renderedSizeCssPx;

  QgsHtmlRenderSettings htmlRenderSettings;
  htmlRenderSettings.setBaseUrl( mBaseUrl );
  htmlRenderSettings.setUserStylesheet( mUserStylesheet );
  htmlRenderSettings.setFeatureJson( mFeatureJson );

  mSuccess = QgsWebEnginePage::renderBlocking( mHtml, &painter, QRectF( 0, 0, widthCssPx, 9999 ), mErrorMessage, renderedSizeCssPx, htmlRenderSettings, &mFeedback );
  painter.end();

  if ( mFeedback.isCanceled() )
    return;

  const double heightCssPx = renderedSizeCssPx.height();
  mPicture.setBoundingRect( QRect( 0, 0, static_cast< int >( std::ceil( widthCssPx ) ), static_cast< int >( std::ceil( heightCssPx ) ) ) );

  mSize = QSizeF( mFrameWidthLayoutUnits, heightCssPx * mUnitsPerCssPx );

  const QSize pageSizeCssPx( static_cast< int >( std::ceil( widthCssPx ) ), static_cast< int >( std::ceil( heightCssPx ) ) );
  if ( !pageSizeCssPx.isEmpty() )
  {
    mRenderedPage = QImage( pageSizeCssPx, QImage::Format_ARGB32 );
    mRenderedPage.fill( Qt::transparent );
    QPainter imagePainter( &mRenderedPage );
    QgsPainting::drawPicture( &imagePainter, QPointF( 0, 0 ), mPicture );
    imagePainter.end();
  }
#else
  mSuccess = false;
  mErrorMessage = tr( "Rendering HTML requires a QGIS build with WebEngine and PDF4QT support" );
#endif
}
///@endcond
