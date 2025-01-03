/***************************************************************************
    qgsdecoratedscrollbar.cpp
     --------------------------------------
    Date                 : May 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecoratedscrollbar.h"
#include "moc_qgsdecoratedscrollbar.cpp"
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QPainter>
#include <QEvent>
#include <QStyleOptionSlider>
#include <cmath>

///@cond PRIVATE

//
// QgsScrollBarHighlightOverlay
//

QgsScrollBarHighlightOverlay::QgsScrollBarHighlightOverlay( QgsScrollBarHighlightController *scrollBarController )
  : QWidget( scrollBarController->scrollArea() )
  , mHighlightController( scrollBarController )
{
  setAttribute( Qt::WA_TransparentForMouseEvents );
  scrollBar()->parentWidget()->installEventFilter( this );
  doResize();
  doMove();
  setVisible( scrollBar()->isVisible() );
}

void QgsScrollBarHighlightOverlay::doResize()
{
  resize( scrollBar()->size() );
}

void QgsScrollBarHighlightOverlay::doMove()
{
  move( parentWidget()->mapFromGlobal( scrollBar()->mapToGlobal( scrollBar()->pos() ) ) );
}

void QgsScrollBarHighlightOverlay::scheduleUpdate()
{
  if ( mIsCacheUpdateScheduled )
    return;

  mIsCacheUpdateScheduled = true;
// silence false positive leak warning
#ifndef __clang_analyzer__
  QMetaObject::invokeMethod( this, qOverload<>( &QWidget::update ), Qt::QueuedConnection );
#endif
}

void QgsScrollBarHighlightOverlay::paintEvent( QPaintEvent *paintEvent )
{
  QWidget::paintEvent( paintEvent );

  updateCache();

  if ( mHighlightCache.isEmpty() )
    return;

  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing, false );

  const QRect &gRect = overlayRect();
  const QRect &hRect = handleRect();

  constexpr int marginX = 3;
  constexpr int marginH = -2 * marginX + 1;
  const QRect aboveHandleRect = QRect( gRect.x() + marginX, gRect.y(), gRect.width() + marginH, hRect.y() - gRect.y() );
  const QRect handleRect = QRect( gRect.x() + marginX, hRect.y(), gRect.width() + marginH, hRect.height() );
  const QRect belowHandleRect = QRect( gRect.x() + marginX, hRect.y() + hRect.height(), gRect.width() + marginH, gRect.height() - hRect.height() + gRect.y() - hRect.y() );

  const int aboveValue = scrollBar()->value();
  const int belowValue = scrollBar()->maximum() - scrollBar()->value();
  const int sizeDocAbove = int( aboveValue * mHighlightController->lineHeight() );
  const int sizeDocBelow = int( belowValue * mHighlightController->lineHeight() );
  const int sizeDocVisible = int( mHighlightController->visibleRange() );

  const int scrollBarBackgroundHeight = aboveHandleRect.height() + belowHandleRect.height();
  const int sizeDocInvisible = sizeDocAbove + sizeDocBelow;
  const double backgroundRatio = sizeDocInvisible
                                   ? ( ( double ) scrollBarBackgroundHeight / sizeDocInvisible )
                                   : 0;


  if ( aboveValue )
  {
    drawHighlights( &painter, 0, sizeDocAbove, backgroundRatio, 0, aboveHandleRect );
  }

  if ( belowValue )
  {
    // This is the hypothetical handle height if the handle would
    // be stretched using the background ratio.
    const double handleVirtualHeight = sizeDocVisible * backgroundRatio;
    // Skip the doc above and visible part.
    const int offset = static_cast<int>( std::round( aboveHandleRect.height() + handleVirtualHeight ) );

    drawHighlights( &painter, sizeDocAbove + sizeDocVisible, sizeDocBelow, backgroundRatio, offset, belowHandleRect );
  }

  const double handleRatio = sizeDocVisible
                               ? ( ( double ) handleRect.height() / sizeDocVisible )
                               : 0;

  // This is the hypothetical handle position if the background would
  // be stretched using the handle ratio.
  const double aboveVirtualHeight = sizeDocAbove * handleRatio;

  // This is the accurate handle position (double)
  const double accurateHandlePos = sizeDocAbove * backgroundRatio;
  // The correction between handle position (int) and accurate position (double)
  const double correction = aboveHandleRect.height() - accurateHandlePos;
  // Skip the doc above and apply correction
  const int offset = static_cast<int>( std::round( aboveVirtualHeight + correction ) );

  drawHighlights( &painter, sizeDocAbove, sizeDocVisible, handleRatio, offset, handleRect );
}

void QgsScrollBarHighlightOverlay::drawHighlights( QPainter *painter, int docStart, int docSize, double docSizeToHandleSizeRatio, int handleOffset, const QRect &viewport )
{
  if ( docSize <= 0 )
    return;

  painter->save();
  painter->setClipRect( viewport );

  const double lineHeight = mHighlightController->lineHeight();

  for ( const QMap<QRgb, QMap<int, int>> &colors : std::as_const( mHighlightCache ) )
  {
    const auto itColorEnd = colors.constEnd();
    for ( auto itColor = colors.constBegin(); itColor != itColorEnd; ++itColor )
    {
      const QColor color = itColor.key();
      const QMap<int, int> &positions = itColor.value();
      const auto itPosEnd = positions.constEnd();
      const auto firstPos = int( docStart / lineHeight );
      auto itPos = positions.upperBound( firstPos );
      if ( itPos != positions.constBegin() )
        --itPos;
      while ( itPos != itPosEnd )
      {
        const double posStart = itPos.key() * lineHeight;
        const double posEnd = ( itPos.value() + 1 ) * lineHeight;
        if ( posEnd < docStart )
        {
          ++itPos;
          continue;
        }
        if ( posStart > docStart + docSize )
          break;

        const int height = std::max( static_cast<int>( std::round( ( posEnd - posStart ) * docSizeToHandleSizeRatio ) ), 1 );
        const int top = static_cast<int>( std::round( posStart * docSizeToHandleSizeRatio ) - handleOffset + viewport.y() );

        const QRect rect( viewport.left(), top, viewport.width(), height );
        painter->fillRect( rect, color );
        ++itPos;
      }
    }
  }
  painter->restore();
}

bool QgsScrollBarHighlightOverlay::eventFilter( QObject *object, QEvent *event )
{
  switch ( event->type() )
  {
    case QEvent::Move:
      doMove();
      break;
    case QEvent::Resize:
      doResize();
      break;
    case QEvent::ZOrderChange:
      raise();
      break;
    case QEvent::Show:
      show();
      break;
    case QEvent::Hide:
      hide();
      break;
    default:
      break;
  }
  return QWidget::eventFilter( object, event );
}

static void insertPosition( QMap<int, int> *map, int position )
{
  auto itNext = map->upperBound( position );

  bool gluedWithPrev = false;
  if ( itNext != map->begin() )
  {
    auto itPrev = std::prev( itNext );
    const int keyStart = itPrev.key();
    const int keyEnd = itPrev.value();
    if ( position >= keyStart && position <= keyEnd )
      return; // pos is already included

    if ( keyEnd + 1 == position )
    {
      // glue with prev
      ( *itPrev )++;
      gluedWithPrev = true;
    }
  }

  if ( itNext != map->end() && itNext.key() == position + 1 )
  {
    const int keyEnd = itNext.value();
    itNext = map->erase( itNext );
    if ( gluedWithPrev )
    {
      // glue with prev and next
      auto itPrev = std::prev( itNext );
      *itPrev = keyEnd;
    }
    else
    {
      // glue with next
      itNext = map->insert( itNext, position, keyEnd );
    }
    return; // glued
  }

  if ( gluedWithPrev )
    return; // glued

  map->insert( position, position );
}

void QgsScrollBarHighlightOverlay::updateCache()
{
  if ( !mIsCacheUpdateScheduled )
    return;

  mHighlightCache.clear();

  const QHash<int, QVector<QgsScrollBarHighlight>> highlightsForId = mHighlightController->highlights();
  for ( const QVector<QgsScrollBarHighlight> &highlights : highlightsForId )
  {
    for ( const QgsScrollBarHighlight &highlight : highlights )
    {
      QMap<int, int> &highlightMap = mHighlightCache[highlight.priority][highlight.color.rgba()];
      insertPosition( &highlightMap, highlight.position );
    }
  }

  mIsCacheUpdateScheduled = false;
}

QRect QgsScrollBarHighlightOverlay::overlayRect() const
{
  QStyleOptionSlider opt = qt_qscrollbarStyleOption( scrollBar() );
  return scrollBar()->style()->subControlRect( QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, scrollBar() );
}

QRect QgsScrollBarHighlightOverlay::handleRect() const
{
  QStyleOptionSlider opt = qt_qscrollbarStyleOption( scrollBar() );
  return scrollBar()->style()->subControlRect( QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, scrollBar() );
}

///@endcond PRIVATE

//
// QgsScrollBarHighlight
//

QgsScrollBarHighlight::QgsScrollBarHighlight( int category, int position, const QColor &color, QgsScrollBarHighlight::Priority priority )
  : category( category )
  , position( position )
  , color( color )
  , priority( priority )
{
}


//
// QgsScrollBarHighlightController
//

QgsScrollBarHighlightController::QgsScrollBarHighlightController() = default;

QgsScrollBarHighlightController::~QgsScrollBarHighlightController()
{
  if ( mOverlay )
    delete mOverlay;
}

QScrollBar *QgsScrollBarHighlightController::scrollBar() const
{
  if ( mScrollArea )
    return mScrollArea->verticalScrollBar();

  return nullptr;
}

QAbstractScrollArea *QgsScrollBarHighlightController::scrollArea() const
{
  return mScrollArea;
}

void QgsScrollBarHighlightController::setScrollArea( QAbstractScrollArea *scrollArea )
{
  if ( mScrollArea == scrollArea )
    return;

  if ( mOverlay )
  {
    delete mOverlay;
    mOverlay = nullptr;
  }

  mScrollArea = scrollArea;

  if ( mScrollArea )
  {
    mOverlay = new QgsScrollBarHighlightOverlay( this );
    mOverlay->scheduleUpdate();
  }
}

double QgsScrollBarHighlightController::lineHeight() const
{
  return std::ceil( mLineHeight );
}

void QgsScrollBarHighlightController::setLineHeight( double lineHeight )
{
  mLineHeight = lineHeight;
}

double QgsScrollBarHighlightController::visibleRange() const
{
  return mVisibleRange;
}

void QgsScrollBarHighlightController::setVisibleRange( double visibleRange )
{
  mVisibleRange = visibleRange;
}

double QgsScrollBarHighlightController::margin() const
{
  return mMargin;
}

void QgsScrollBarHighlightController::setMargin( double margin )
{
  mMargin = margin;
}

QHash<int, QVector<QgsScrollBarHighlight>> QgsScrollBarHighlightController::highlights() const
{
  return mHighlights;
}

void QgsScrollBarHighlightController::addHighlight( const QgsScrollBarHighlight &highlight )
{
  if ( !mOverlay )
    return;

  mHighlights[highlight.category] << highlight;
  mOverlay->scheduleUpdate();
}

void QgsScrollBarHighlightController::removeHighlights( int category )
{
  if ( !mOverlay )
    return;

  mHighlights.remove( category );
  mOverlay->scheduleUpdate();
}

void QgsScrollBarHighlightController::removeAllHighlights()
{
  if ( !mOverlay )
    return;

  mHighlights.clear();
  mOverlay->scheduleUpdate();
}
