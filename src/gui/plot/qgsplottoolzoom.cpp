/***************************************************************************
                          qgsplottoolzoom.cpp
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsplottoolzoom.h"
#include "qgsapplication.h"
#include "qgsplotmouseevent.h"
#include "qgsplotcanvas.h"
#include "qgsplotrubberband.h"

QgsPlotToolZoom::QgsPlotToolZoom( QgsPlotCanvas *canvas )
  : QgsPlotTool( canvas, tr( "Zoom" ) )
{
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
  mRubberBand.reset( new QgsPlotRectangularRubberBand( canvas ) );
  mRubberBand->setBrush( QBrush( QColor( 70, 50, 255, 25 ) ) );
  mRubberBand->setPen( QPen( QBrush( QColor( 70, 50, 255, 100 ) ), 0 ) );
}

QgsPlotToolZoom::~QgsPlotToolZoom() = default;

void QgsPlotToolZoom::plotPressEvent( QgsPlotMouseEvent *event )
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
    zoomOutClickOn( event->pos() );
  }
  else
  {
    //zoom in action
    startMarqueeZoom( event->pos() );
  }
}

void QgsPlotToolZoom::plotMoveEvent( QgsPlotMouseEvent *event )
{
  if ( !mMarqueeZoom )
  {
    event->ignore();
    return;
  }

  mRubberBand->update( constrainMovePoint( event->pos() ), Qt::KeyboardModifiers() );
}

void QgsPlotToolZoom::plotReleaseEvent( QgsPlotMouseEvent *event )
{
  if ( !mMarqueeZoom || event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mMarqueeZoom = false;
  // click? or click-and-drag?
  if ( !isClickAndDrag( mMousePressStartPos, event->pos() ) )
  {
    zoomInClickOn( event->pos() );
  }
  else
  {
    QRectF newBoundsRect = mRubberBand->finish( constrainMovePoint( event->pos() ) );

    //zoom view to fit desired bounds
    canvas()->zoomToRect( constrainBounds( newBoundsRect ) );
  }
}

void QgsPlotToolZoom::keyPressEvent( QKeyEvent *event )
{
  //respond to changes in the alt key status and update cursor accordingly
  if ( !event->isAutoRepeat() )
  {

    canvas()->viewport()->setCursor( ( event->modifiers() & Qt::AltModifier ) ?
                                     QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomOut ) :
                                     QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
  }
  event->ignore();
}

void QgsPlotToolZoom::keyReleaseEvent( QKeyEvent *event )
{
  //respond to changes in the alt key status and update cursor accordingly
  if ( !event->isAutoRepeat() )
  {

    canvas()->viewport()->setCursor( ( event->modifiers() & Qt::AltModifier ) ?
                                     QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomOut ) :
                                     QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
  }
  event->ignore();
}

void QgsPlotToolZoom::deactivate()
{
  if ( mMarqueeZoom )
  {
    mMarqueeZoom = false;
    mRubberBand->finish();
  }
  QgsPlotTool::deactivate();
}

QPointF QgsPlotToolZoom::constrainStartPoint( QPointF scenePoint ) const
{
  return scenePoint;
}

QPointF QgsPlotToolZoom::constrainMovePoint( QPointF scenePoint ) const
{
  return scenePoint;
}

QRectF QgsPlotToolZoom::constrainBounds( const QRectF &sceneBounds ) const
{
  return sceneBounds;
}

void QgsPlotToolZoom::zoomOutClickOn( QPointF scenePoint )
{
  //just a click, so zoom to clicked point and recenter
  canvas()->centerPlotOn( scenePoint.x(), scenePoint.y() );
  canvas()->scalePlot( 0.5 );
}

void QgsPlotToolZoom::zoomInClickOn( QPointF scenePoint )
{
  //just a click, so zoom to clicked point and recenter
  canvas()->centerPlotOn( scenePoint.x(), scenePoint.y() );
  canvas()->scalePlot( 2.0 );
}

void QgsPlotToolZoom::startMarqueeZoom( QPointF scenePoint )
{
  mMarqueeZoom = true;

  mRubberBandStartPos = scenePoint;
  mRubberBand->start( constrainStartPoint( scenePoint ), Qt::KeyboardModifiers() );
}
