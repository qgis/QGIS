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

#include <QEvent>
#include <QMouseEvent>
#include "qgsscrollarea.h"
#include <QScrollBar>

// milliseconds to swallow child wheel events for after a scroll occurs
#define TIMEOUT 1000

QgsScrollArea::QgsScrollArea( QWidget *parent )
  : QScrollArea( parent )
  , mFilter( new ScrollAreaFilter( this, viewport() ) )
{
  viewport()->installEventFilter( mFilter );
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
{}

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
    child->installEventFilter( this );

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
