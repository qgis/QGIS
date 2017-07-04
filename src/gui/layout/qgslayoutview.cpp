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

void QgsLayoutView::mousePressEvent( QMouseEvent *event )
{
  QGraphicsView::mousePressEvent( event );
}

void QgsLayoutView::mouseReleaseEvent( QMouseEvent *event )
{
  QGraphicsView::mouseReleaseEvent( event );
}

void QgsLayoutView::mouseMoveEvent( QMouseEvent *event )
{
  QGraphicsView::mouseMoveEvent( event );
}

void QgsLayoutView::mouseDoubleClickEvent( QMouseEvent *event )
{
  QGraphicsView::mouseDoubleClickEvent( event );
}
