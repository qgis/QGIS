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

#include <QMenu>
#include <QKeyEvent>
#include <QGestureEvent>

QgsPlotCanvas::QgsPlotCanvas( QWidget *parent )
  : QGraphicsView( parent )
{
  mScene = new QGraphicsScene();
  setScene( mScene );

  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setMouseTracking( true );
  setFocusPolicy( Qt::StrongFocus );

  setInteractive( false );

  refresh();
}

QgsPlotCanvas::~QgsPlotCanvas()
{
  if ( mTool )
  {
    mTool->deactivate();
    mTool = nullptr;
  }

  cancelJobs();

  // delete canvas items prior to deleting the canvas
  // because they might try to update canvas when it's
  // already being destructed, ends with segfault
  qDeleteAll( mScene->items() );

  mScene->deleteLater();
}

void QgsPlotCanvas::cancelJobs()
{

}

void QgsPlotCanvas::waitWhileRendering()
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

  menu.exec( event->globalPos() );
}

void QgsPlotCanvas::keyPressEvent( QKeyEvent *e )
{
  if ( mMouseButtonDown )
  {
    emit keyPressed( e );
    return;
  }
  // Don't want to interfer with mouse events
  else if ( !mMouseButtonDown )
  {
    e->ignore();
    if ( mTool )
    {
      mTool->keyPressEvent( e );
      if ( e->isAccepted() ) // map tool consumed event
        return;
    }

    switch ( e->key() )
    {
      default:
        // Pass it on
        if ( !mTool )
        {
          e->ignore();
        }
    }
  }

  emit keyPressed( e );
}

void QgsPlotCanvas::keyReleaseEvent( QKeyEvent *e )
{
  switch ( e->key() )
  {
    default:
      // Pass it on
      if ( mTool )
      {
        mTool->keyReleaseEvent( e );
      }
      else
      {
        e->ignore();

      }
  }
  emit keyReleased( e );
}

void QgsPlotCanvas::mouseDoubleClickEvent( QMouseEvent *e )
{
  if ( mTool )
  {
    std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, e ) );
    mTool->plotDoubleClickEvent( me.get() );
  }
}

void QgsPlotCanvas::mousePressEvent( QMouseEvent *e )
{
  if ( false )
  {

  }
  else
  {
    // call handler of current map tool
    if ( mTool )
    {
      if ( mTool->flags() & Qgis::PlotToolFlag::ShowContextMenu && e->button() == Qt::RightButton )
      {
        std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, e ) );
        showContextMenu( me.get() );
        return;
      }
      else
      {
        std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, e ) );
        mTool->plotPressEvent( me.get() );
      }
    }
  }

  mMouseButtonDown = true;
}

void QgsPlotCanvas::mouseReleaseEvent( QMouseEvent *e )
{
  if ( false )
  {

  }
  else
  {
    // call handler of current tool
    if ( mTool )
    {
      std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, e ) );
      mTool->plotReleaseEvent( me.get() );
    }
  }

  mMouseButtonDown = false;
}

void QgsPlotCanvas::resizeEvent( QResizeEvent *e )
{
  QGraphicsView::resizeEvent( e );
}

void QgsPlotCanvas::wheelEvent( QWheelEvent *e )
{
  // Zoom the map canvas in response to a mouse wheel event. Moving the
  // wheel forward (away) from the user zooms in

  e->ignore();
  if ( mTool )
  {
    mTool->wheelEvent( e );
    if ( e->isAccepted() )
      return;
  }

  if ( e->angleDelta().y() == 0 )
  {
    e->accept();
    return;
  }

  e->accept();
}

void QgsPlotCanvas::mouseMoveEvent( QMouseEvent *e )
{
  if ( false )
  {

  }
  else
  {
    // call handler of current map tool
    if ( mTool )
    {
      std::unique_ptr<QgsPlotMouseEvent> me( new QgsPlotMouseEvent( this, e ) );
      mTool->plotMoveEvent( me.get() );
    }
  }
}

void QgsPlotCanvas::setTool( QgsPlotTool *tool, bool clean )
{
  if ( !tool )
    return;

  if ( mTool )
  {
    if ( clean )
      mTool->clean();

    disconnect( mTool, &QObject::destroyed, this, &QgsPlotCanvas::toolDestroyed );
    mTool->deactivate();
  }

  QgsPlotTool *oldTool = mTool;

  // set new tool and activate it
  mTool = tool;
  emit toolChanged( mTool, oldTool );
  if ( mTool )
  {
    connect( mTool, &QObject::destroyed, this, &QgsPlotCanvas::toolDestroyed );
    mTool->activate();
  }
}

void QgsPlotCanvas::unsetTool( QgsPlotTool *tool )
{
  if ( mTool && mTool == tool )
  {
    disconnect( mTool, &QObject::destroyed, this, &QgsPlotCanvas::toolDestroyed );
    QgsPlotTool *oldTool = mTool;
    mTool = nullptr;
    oldTool->deactivate();
    emit toolChanged( nullptr, oldTool );
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

bool QgsPlotCanvas::viewportEvent( QEvent *event )
{
  if ( event->type() == QEvent::ToolTip && mTool && mTool->canvasToolTipEvent( qgis::down_cast<QHelpEvent *>( event ) ) )
  {
    return true;
  }
  return QGraphicsView::viewportEvent( event );
}

void QgsPlotCanvas::toolDestroyed()
{
  QgsDebugMsgLevel( QStringLiteral( "tool destroyed" ), 2 );
  mTool = nullptr;
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
