/***************************************************************************
    qgsscrollarea.cpp
    -----------------
    begin                : March 2017
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

#include "qgsscrollarea.h"

#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QAbstractItemView>

// milliseconds to swallow child wheel events for after a scroll occurs
#define TIMEOUT 1000

QgsScrollArea::QgsScrollArea( QWidget *parent )
  : QScrollArea( parent )
  , mFilter( new ScrollAreaFilter( this, viewport() ) )
{
  viewport()->installEventFilter( mFilter );
  setMouseTracking( true );
}

void QgsScrollArea::wheelEvent( QWheelEvent *e )
{
  //scroll occurred, reset timer
  scrollOccurred();
  QScrollArea::wheelEvent( e );
}

void QgsScrollArea::resizeEvent( QResizeEvent *event )
{
  if ( mVerticalOnly && widget() )
    widget()->setFixedWidth( event->size().width() );
  QScrollArea::resizeEvent( event );
}

void QgsScrollArea::scrollOccurred()
{
  mTimer.setSingleShot( true );
  mTimer.start( TIMEOUT );
}

bool QgsScrollArea::hasScrolled() const
{
  return mTimer.isActive();
}

void QgsScrollArea::resetHasScrolled()
{
  mTimer.stop();
}

void QgsScrollArea::setVerticalOnly( bool verticalOnly )
{
  mVerticalOnly = verticalOnly;
  if ( mVerticalOnly )
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  if ( mVerticalOnly && widget() )
    widget()->setFixedWidth( size().width() );
}

///@cond PRIVATE

ScrollAreaFilter::ScrollAreaFilter( QgsScrollArea *parent, QWidget *viewPort )
  : QObject( parent )
  , mScrollAreaWidget( parent )
  , mViewPort( viewPort )
{
  QFontMetrics fm( parent->font() );
  mMoveDistanceThreshold = fm.horizontalAdvance( 'X' );
}

bool ScrollAreaFilter::eventFilter( QObject *obj, QEvent *event )
{
  switch ( event->type() )
  {
    case QEvent::ChildAdded:
    {
      // need to install filter on all child widgets as well
      QChildEvent *ce = static_cast<QChildEvent *>( event );
      addChild( ce->child() );
      break;
    }

    case QEvent::ChildRemoved:
    {
      QChildEvent *ce = static_cast<QChildEvent *>( event );
      removeChild( ce->child() );
      break;
    }

    case QEvent::MouseMove:
    {
      if ( obj == mViewPort )
      {
        const QPoint mouseDelta = QCursor::pos() - mPreviousViewportCursorPos;
        if ( mouseDelta.manhattanLength() > mMoveDistanceThreshold )
        {
          // release time based child widget constraint -- user moved the mouse over the viewport (and not just an accidental "wiggle")
          // so we no longer are in the 'possible unwanted mouse wheel event going to child widget mid-scroll' state
          mScrollAreaWidget->resetHasScrolled();
        }
        mPreviousViewportCursorPos = QCursor::pos();
      }
      break;
    }

    case QEvent::Wheel:
    {
      if ( obj == mViewPort )
      {
        // scrolling scroll area - kick off the timer to block wheel events in children
        mScrollAreaWidget->scrollOccurred();
      }
      else
      {
        if ( mScrollAreaWidget->hasScrolled() )
        {
          // swallow wheel events for children shortly after scroll occurs
          return true;
        }
      }
      break;
    }

    default:
      break;
  }
  return QObject::eventFilter( obj, event );
}

void ScrollAreaFilter::addChild( QObject *child )
{
  if ( child && child->isWidgetType() )
  {
    if ( qobject_cast< QScrollArea * >( child ) || qobject_cast< QAbstractItemView * >( child ) )
      return;

    child->installEventFilter( this );
    if ( QWidget *w = qobject_cast< QWidget * >( child ) )
      w->setMouseTracking( true );

    // also install filter on existing children
    const auto constChildren = child->children();
    for ( QObject *c : constChildren )
    {
      addChild( c );
    }
  }
}

void ScrollAreaFilter::removeChild( QObject *child )
{
  if ( child && child->isWidgetType() )
  {
    if ( qobject_cast< QScrollArea * >( child ) || qobject_cast< QAbstractItemView * >( child ) )
      return;

    child->removeEventFilter( this );

    // also remove filter on existing children
    const auto constChildren = child->children();
    for ( QObject *c : constChildren )
    {
      removeChild( c );
    }
  }
}

///@endcond PRIVATE
