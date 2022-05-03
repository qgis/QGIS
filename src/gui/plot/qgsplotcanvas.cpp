/***************************************************************************
                          qgsplotcanvas.cpp
                          -----------------
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

#include "qgsplotcanvas.h"
#include "qgsplotmouseevent.h"
#include "qgsplottool.h"
#include "qgslogger.h"
#include "qgsplottransienttools.h"
#include "qgssettings.h"

#include <QMenu>
#include <QKeyEvent>
#include <QGestureEvent>

QgsPlotCanvas::QgsPlotCanvas( QWidget *parent )
  : QGraphicsView( parent )
{
  setObjectName( QStringLiteral( "PlotCanvas" ) );
  mScene = new QGraphicsScene( this );
  setScene( mScene );

  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );

  setFocusPolicy( Qt::StrongFocus );

  mSpacePanTool = new QgsPlotToolTemporaryKeyPan( this );
  mMidMouseButtonPanTool = new QgsPlotToolTemporaryMousePan( this );
  mSpaceZoomTool = new QgsPlotToolTemporaryKeyZoom( this );
}

QgsPlotCanvas::~QgsPlotCanvas()
{
  if ( mTool )
  {
    mTool->deactivate();
    mTool = nullptr;
  }
  emit willBeDeleted();

  QgsPlotCanvas::cancelJobs();

  // WARNING WARNING WARNING
  // QgsMapCanvas deletes all items in the destructor. But for some absolutely INSANE WTF reason
  // if we uncomment this code below then we get random crashes in QGraphicsScene EVEN IF WE NEVER EVER CREATE A QgsPlotCanvas
  // object and this code is NEVER EVEN CALLED ONCE. Like, WTAF??!?!?!?!?!

  // change this if you want to waste days of your life only. I don't, so I just made the scene parented to this canvas, and let's see what fallout ensures...
#if 0

  // delete canvas items prior to deleting the canvas
  // because they might try to update canvas when it's
  // already being destructed, ends with segfault
  qDeleteAll( mScene->items() );

  mScene->deleteLater();
#endif
}

void QgsPlotCanvas::cancelJobs()
{

}

void QgsPlotCanvas::refresh()
{

}

void QgsPlotCanvas::showContextMenu( QgsPlotMouseEvent *event )
{
  QMenu menu;

  if ( mTool )
  {
    mTool->populateContextMenuWithEvent( &menu, event );
  }

  emit contextMenuAboutToShow( &menu, event );

  if ( !menu.isEmpty() )
    menu.exec( event->globalPos() );
}

void QgsPlotCanvas::keyPressEvent( QKeyEvent *event )
{
  if ( mTool )
  {
    mTool->keyPressEvent( event );
  }
  if ( mTool && event->isAccepted() )
    return;

  if ( event->key() == Qt::Key_Space && ! event->isAutoRepeat() )
  {
    if ( !( event->modifiers() & Qt::ControlModifier ) )
    {
      // Pan layout with space bar
      setTool( mSpacePanTool );
    }
    else
    {
      //ctrl+space pressed, so switch to temporary keyboard based zoom tool
      setTool( mSpaceZoomTool );
    }
    event->accept();
  }
}

void QgsPlotCanvas::keyReleaseEvent( QKeyEvent *event )
{
  if ( mTool )
  {
    mTool->keyReleaseEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::keyReleaseEvent( event );
}

void QgsPlotCanvas::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, event ) );
    mTool->plotDoubleClickEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseDoubleClickEvent( event );
}

void QgsPlotCanvas::mousePressEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, event ) );
    mTool->plotPressEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
  {
    if ( event->button() == Qt::MiddleButton )
    {
      // Pan layout with middle mouse button
      setTool( mMidMouseButtonPanTool );
      event->accept();
    }
    else if ( event->button() == Qt::RightButton && mTool->flags() & Qgis::PlotToolFlag::ShowContextMenu )
    {
      std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, event ) );
      showContextMenu( me.get() );
      event->accept();
      return;
    }
    else
    {
      QGraphicsView::mousePressEvent( event );
    }
  }
}

void QgsPlotCanvas::mouseReleaseEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, event ) );
    mTool->plotReleaseEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseReleaseEvent( event );
}

void QgsPlotCanvas::resizeEvent( QResizeEvent *e )
{
  QGraphicsView::resizeEvent( e );
}

void QgsPlotCanvas::wheelEvent( QWheelEvent *event )
{
  if ( mTool )
  {
    mTool->wheelEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
  {
    event->accept();
    wheelZoom( event );
  }
}

void QgsPlotCanvas::mouseMoveEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, event ) );
    mTool->plotMoveEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseMoveEvent( event );
}

void QgsPlotCanvas::setTool( QgsPlotTool *tool )
{
  if ( mTool )
  {
    mTool->deactivate();
  }

  if ( tool )
  {
    // activate new tool before setting it - gives tools a chance
    // to respond to whatever the current tool is
    tool->activate();
  }

  mTool = tool;
  emit toolChanged( mTool );
}

void QgsPlotCanvas::unsetTool( QgsPlotTool *tool )
{
  if ( mTool && mTool == tool )
  {
    mTool->deactivate();
    emit toolChanged( nullptr );
    setCursor( Qt::ArrowCursor );
  }
}

QgsPlotTool *QgsPlotCanvas::tool()
{
  return mTool;
}

QgsCoordinateReferenceSystem QgsPlotCanvas::crs() const
{
  return QgsCoordinateReferenceSystem();
}

QgsPoint QgsPlotCanvas::toMapCoordinates( const QgsPointXY & ) const
{
  return QgsPoint();
}

QgsPointXY QgsPlotCanvas::toCanvasCoordinates( const QgsPoint & ) const
{
  return QgsPointXY();
}

void QgsPlotCanvas::panContentsBy( double, double )
{

}

void QgsPlotCanvas::centerPlotOn( double, double )
{

}

void QgsPlotCanvas::scalePlot( double )
{

}

void QgsPlotCanvas::zoomToRect( const QRectF & )
{

}

QgsPointXY QgsPlotCanvas::snapToPlot( QPoint )
{
  return QgsPointXY();
}

bool QgsPlotCanvas::viewportEvent( QEvent *event )
{
  if ( event->type() == QEvent::ToolTip && mTool && mTool->canvasToolTipEvent( qgis::down_cast<QHelpEvent *>( event ) ) )
  {
    return true;
  }
  return QGraphicsView::viewportEvent( event );
}

void QgsPlotCanvas::wheelZoom( QWheelEvent * )
{

}

bool QgsPlotCanvas::event( QEvent *e )
{
  if ( !QTouchDevice::devices().empty() )
  {
    if ( e->type() == QEvent::Gesture )
    {
      // call handler of current map tool
      if ( mTool )
      {
        return mTool->gestureEvent( static_cast<QGestureEvent *>( e ) );
      }
    }
  }

  // pass other events to base class
  return QGraphicsView::event( e );
}
