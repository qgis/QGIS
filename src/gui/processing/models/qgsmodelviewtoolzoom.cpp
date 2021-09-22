/***************************************************************************
                             qgsmodelviewtoolzoom.cpp
                             -------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelviewtoolzoom.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelviewrubberband.h"
#include "qgsrectangle.h"
#include "qgsapplication.h"
#include <QScrollBar>

QgsModelViewToolZoom::QgsModelViewToolZoom( QgsModelGraphicsView *view )
  : QgsModelViewTool( view, tr( "Pan" ) )
{
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
  mRubberBand.reset( new QgsModelViewRectangularRubberBand( view ) );
  mRubberBand->setBrush( QBrush( QColor( 70, 50, 255, 25 ) ) );
  mRubberBand->setPen( QPen( QBrush( QColor( 70, 50, 255, 100 ) ), 0 ) );
}

void QgsModelViewToolZoom::modelPressEvent( QgsModelViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
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

    visibleRect.scale( scaleFactor, event->modelPoint().x(), event->modelPoint().y() );
    const QRectF boundsRect = visibleRect.toRectF();

    //zoom view to fit desired bounds
    view()->fitInView( boundsRect, Qt::KeepAspectRatio );
  }
  else
  {
    //zoom in action
    startMarqueeZoom( event->modelPoint() );
  }
}

void QgsModelViewToolZoom::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  if ( !mMarqueeZoom )
  {
    event->ignore();
    return;
  }

  mRubberBand->update( event->modelPoint(), Qt::KeyboardModifiers() );
}

void QgsModelViewToolZoom::modelReleaseEvent( QgsModelViewMouseEvent *event )
{
  if ( !mMarqueeZoom || event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mMarqueeZoom = false;
  QRectF newBoundsRect = mRubberBand->finish( event->modelPoint() );

  // click? or click-and-drag?
  if ( !isClickAndDrag( mMousePressStartPos, event->pos() ) )
  {
    //just a click, so zoom to clicked point and recenter
    const double scaleFactor = 0.5;
    //get current visible part of scene
    const QRect viewportRect( 0, 0, view()->viewport()->width(), view()->viewport()->height() );
    QgsRectangle visibleRect = QgsRectangle( view()->mapToScene( viewportRect ).boundingRect() );

    visibleRect.scale( scaleFactor, event->modelPoint().x(), event->modelPoint().y() );
    newBoundsRect = visibleRect.toRectF();
  }

  //zoom view to fit desired bounds
  view()->fitInView( newBoundsRect, Qt::KeepAspectRatio );
}

void QgsModelViewToolZoom::keyPressEvent( QKeyEvent *event )
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

void QgsModelViewToolZoom::keyReleaseEvent( QKeyEvent *event )
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

void QgsModelViewToolZoom::deactivate()
{
  if ( mMarqueeZoom )
  {
    mMarqueeZoom = false;
    mRubberBand->finish();
  }
  QgsModelViewTool::deactivate();
}

void QgsModelViewToolZoom::startMarqueeZoom( QPointF scenePoint )
{
  mMarqueeZoom = true;

  mRubberBandStartPos = scenePoint;
  mRubberBand->start( scenePoint, Qt::KeyboardModifiers() );
}
