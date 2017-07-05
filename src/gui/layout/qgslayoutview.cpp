/***************************************************************************
                             qgslayoutview.cpp
                             -----------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutviewtool.h"
#include "qgslayoutviewmouseevent.h"
#include "qgssettings.h"
#include "qgsrectangle.h"
#include <memory>

#define MIN_VIEW_SCALE 0.05
#define MAX_VIEW_SCALE 1000.0

QgsLayoutView::QgsLayoutView( QWidget *parent )
  : QGraphicsView( parent )
{
  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );
}

QgsLayout *QgsLayoutView::currentLayout()
{
  return qobject_cast<QgsLayout *>( scene() );
}

void QgsLayoutView::setCurrentLayout( QgsLayout *layout )
{
  setScene( layout );

  //emit layoutSet, so that designer dialogs can update for the new layout
  emit layoutSet( layout );
}

QgsLayoutViewTool *QgsLayoutView::tool()
{
  return mTool;
}

void QgsLayoutView::setTool( QgsLayoutViewTool *tool )
{
  if ( !tool )
    return;

  if ( mTool )
  {
    mTool->deactivate();
  }

  // set new tool and activate it
  mTool = tool;
  mTool->activate();

  emit toolSet( mTool );
}

void QgsLayoutView::unsetTool( QgsLayoutViewTool *tool )
{
  if ( mTool && mTool == tool )
  {
    mTool->deactivate();
    emit toolSet( nullptr );
    setCursor( Qt::ArrowCursor );
  }
}

void QgsLayoutView::scaleSafe( double scale )
{
  double currentScale = transform().m11();
  scale *= currentScale;
  scale = qBound( MIN_VIEW_SCALE, scale, MAX_VIEW_SCALE );
  setTransform( QTransform::fromScale( scale, scale ) );
}

void QgsLayoutView::mousePressEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event ) );
    mTool->layoutPressEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mousePressEvent( event );
}

void QgsLayoutView::mouseReleaseEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event ) );
    mTool->layoutReleaseEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseReleaseEvent( event );
}

void QgsLayoutView::mouseMoveEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event ) );
    mTool->layoutMoveEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseMoveEvent( event );
}

void QgsLayoutView::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event ) );
    mTool->layoutDoubleClickEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseDoubleClickEvent( event );
}

void QgsLayoutView::wheelEvent( QWheelEvent *event )
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

void QgsLayoutView::keyPressEvent( QKeyEvent *event )
{
  if ( mTool )
  {
    mTool->keyPressEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::keyPressEvent( event );
}

void QgsLayoutView::keyReleaseEvent( QKeyEvent *event )
{
  if ( mTool )
  {
    mTool->keyReleaseEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::keyReleaseEvent( event );
}

void QgsLayoutView::wheelZoom( QWheelEvent *event )
{
  //get mouse wheel zoom behavior settings
  QgsSettings settings;
  double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * qAbs( event->angleDelta().y() );

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  //calculate zoom scale factor
  bool zoomIn = event->angleDelta().y() > 0;
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

  //transform the mouse pos to scene coordinates
  QPointF scenePoint = mapToScene( event->pos() );

  //adjust view center
  QgsPointXY oldCenter( visibleRect.center() );
  QgsPointXY newCenter( scenePoint.x() + ( ( oldCenter.x() - scenePoint.x() ) * scaleFactor ),
                        scenePoint.y() + ( ( oldCenter.y() - scenePoint.y() ) * scaleFactor ) );
  centerOn( newCenter.x(), newCenter.y() );

  //zoom layout
  if ( zoomIn )
  {
    scaleSafe( zoomFactor );
  }
  else
  {
    scaleSafe( 1 / zoomFactor );
  }

  //update layout for new zoom
#if 0 // TODO
  emit zoomLevelChanged();
  updateRulers();
#endif
  update();
}
