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

void QgsLayoutView::mousePressEvent( QMouseEvent *event )
{
  if ( mTool )
    mTool->layoutPressEvent( event );
  else
    QGraphicsView::mousePressEvent( event );
}

void QgsLayoutView::mouseReleaseEvent( QMouseEvent *event )
{
  if ( mTool )
    mTool->layoutReleaseEvent( event );
  else
    QGraphicsView::mouseReleaseEvent( event );
}

void QgsLayoutView::mouseMoveEvent( QMouseEvent *event )
{
  if ( mTool )
    mTool->layoutMoveEvent( event );
  else
    QGraphicsView::mouseMoveEvent( event );
}

void QgsLayoutView::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( mTool )
    mTool->layoutDoubleClickEvent( event );
  else
    QGraphicsView::mouseDoubleClickEvent( event );
}

void QgsLayoutView::wheelEvent( QWheelEvent *event )
{
  if ( mTool )
    mTool->wheelEvent( event );
  else
    QGraphicsView::wheelEvent( event );
}

void QgsLayoutView::keyPressEvent( QKeyEvent *event )
{
  if ( mTool )
    mTool->keyPressEvent( event );
  else
    QGraphicsView::keyPressEvent( event );
}

void QgsLayoutView::keyReleaseEvent( QKeyEvent *event )
{
  if ( mTool )
    mTool->keyReleaseEvent( event );
  else
    QGraphicsView::keyReleaseEvent( event );
}
