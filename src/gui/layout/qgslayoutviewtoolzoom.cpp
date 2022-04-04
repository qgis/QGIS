/***************************************************************************
                             qgslayoutviewtoolzoom.cpp
                             -------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutviewtoolzoom.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include "qgslayoutviewrubberband.h"
#include "qgsrectangle.h"
#include <QScrollBar>

QgsLayoutViewToolZoom::QgsLayoutViewToolZoom( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Pan" ) )
{
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
  mRubberBand.reset( new QgsLayoutViewRectangularRubberBand( view ) );
  mRubberBand->setBrush( QBrush( QColor( 70, 50, 255, 25 ) ) );
  mRubberBand->setPen( QPen( QBrush( QColor( 70, 50, 255, 100 ) ), 0 ) );
}

void QgsLayoutViewToolZoom::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    if ( mMarqueeZoom )
    {
      mMarqueeZoom = false;
      mRubberBand->finish();
    }
    event->ignore();
    return;
  }

  mMousePressStartPos = event->pos();
  if ( event->modifiers() & Qt::AltModifier )
  {
    //zoom out action, so zoom out and recenter on clicked point
    const double scaleFactor = 2;
    //get current visible part of scene
    const QRect viewportRect( 0, 0, view()->viewport()->width(), view()->viewport()->height() );
    QgsRectangle visibleRect = QgsRectangle( view()->mapToScene( viewportRect ).boundingRect() );

    visibleRect.scale( scaleFactor, event->layoutPoint().x(), event->layoutPoint().y() );
    const QRectF boundsRect = visibleRect.toRectF();

    //zoom view to fit desired bounds
    view()->fitInView( boundsRect, Qt::KeepAspectRatio );
    view()->emitZoomLevelChanged();
    view()->viewChanged();
  }
  else
  {
    //zoom in action
    startMarqueeZoom( event->layoutPoint() );
  }
}

void QgsLayoutViewToolZoom::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( !mMarqueeZoom )
  {
    event->ignore();
    return;
  }

  mRubberBand->update( event->layoutPoint(), Qt::KeyboardModifiers() );
}

void QgsLayoutViewToolZoom::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( !mMarqueeZoom || event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mMarqueeZoom = false;
  QRectF newBoundsRect = mRubberBand->finish( event->layoutPoint() );

  // click? or click-and-drag?
  if ( !isClickAndDrag( mMousePressStartPos, event->pos() ) )
  {
    //just a click, so zoom to clicked point and recenter
    const double scaleFactor = 0.5;
    //get current visible part of scene
    const QRect viewportRect( 0, 0, view()->viewport()->width(), view()->viewport()->height() );
    QgsRectangle visibleRect = QgsRectangle( view()->mapToScene( viewportRect ).boundingRect() );

    visibleRect.scale( scaleFactor, event->layoutPoint().x(), event->layoutPoint().y() );
    newBoundsRect = visibleRect.toRectF();
  }

  //zoom view to fit desired bounds
  view()->fitInView( newBoundsRect, Qt::KeepAspectRatio );
  view()->emitZoomLevelChanged();
  view()->viewChanged();
}

void QgsLayoutViewToolZoom::keyPressEvent( QKeyEvent *event )
{
  //respond to changes in the alt key status and update cursor accordingly
  if ( !event->isAutoRepeat() )
  {

    view()->viewport()->setCursor( ( event->modifiers() & Qt::AltModifier ) ?
                                   QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomOut ) :
                                   QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
  }
  event->ignore();
}

void QgsLayoutViewToolZoom::keyReleaseEvent( QKeyEvent *event )
{
  //respond to changes in the alt key status and update cursor accordingly
  if ( !event->isAutoRepeat() )
  {

    view()->viewport()->setCursor( ( event->modifiers() & Qt::AltModifier ) ?
                                   QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomOut ) :
                                   QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
  }
  event->ignore();
}

void QgsLayoutViewToolZoom::deactivate()
{
  if ( mMarqueeZoom )
  {
    mMarqueeZoom = false;
    mRubberBand->finish();
  }
  QgsLayoutViewTool::deactivate();
}

void QgsLayoutViewToolZoom::startMarqueeZoom( QPointF scenePoint )
{
  mMarqueeZoom = true;

  mRubberBandStartPos = scenePoint;
  mRubberBand->start( scenePoint, Qt::KeyboardModifiers() );
}
