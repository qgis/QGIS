//
//
// C++ Implementation: $MODULE$
//
// Description: Subclassed QListView that handles drag-n-drop for changing layer order
//
//
// Author: Steve Halasz <stevehalasz at users.sourceforge.net>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
// $Id$
#include <qapplication.h>
#include <qcursor.h>
#include <qlistview.h>
#include <qpoint.h>
#include <qrect.h>
//#include <iostream>

#include "qgslegendview.h"

QgsLegendView::QgsLegendView( QWidget *parent, const char *name ):QListView( parent, name ), mousePressed( FALSE )
{
  // set movingItem pointer to 0 to prevent SuSE 9.0 crash
  movingItem = 0;
}

void QgsLegendView::contentsMousePressEvent( QMouseEvent* e )
{
	//std::cout << "contentsMousePressEvent" << std::endl;	
	if (e->button() == LeftButton) {
		//std::cout << "leftButton" << std::endl;
		QPoint p( contentsToViewport( e->pos() ) );
		QListViewItem *i = itemAt( p );
		if ( i ) {
			presspos = e->pos();
			mousePressed = TRUE;
			//std::cout << "mousePressed = TRUE" << std::endl;
		}
	}
	QListView::contentsMousePressEvent( e );
}

void QgsLegendView::contentsMouseMoveEvent( QMouseEvent* e )
{
	//std::cout << "contentsMouseMoveEvent" << std::endl;
	if ( mousePressed ) {
		//std::cout << "mousePressed" << std::endl;
		mousePressed = FALSE;
		//std::cout << "mousePressed = FALSE" << std::endl;
		// remember item we've pressed as the one being moved
		QListViewItem *item = itemAt( contentsToViewport(presspos) );
		if ( item ) {
			movingItem = item;
			setCursor( SizeVerCursor );
			//std::cout << "movingItem = item" << std::endl;
		}
	} else if ( movingItem ) {
		//std::cout << "movingItem" << std::endl;
		// move item in list if we're dragging over another item
		QListViewItem *item = itemAt( e->pos() );
		if ( item && ( item != movingItem ) ) {
			// find if we're over the top or bottom half of the item
			QRect rect = itemRect( item );
			int height = rect.height();
			int top = rect.top();
			int mid = top + ( height / 2 );
			if ( e->y() < mid) {
				// move item we're over now to after one in motion
				// unless it's already there
				if ( movingItem->nextSibling() != item ) {
					item->moveItem( movingItem );
				}
			} else {
				// move item in motion to after one we're over now
				// unless it's already there
				if ( movingItem != item->nextSibling() ) {
					movingItem->moveItem( item );
				}
			}
		}
	}
}

void QgsLegendView::contentsMouseReleaseEvent( QMouseEvent* e)
{
	//std::cout << "contentsMouseReleaseEvent" << std::endl;
	QListView::contentsMouseReleaseEvent( e );
	if (e->button() == LeftButton) {
		//std::cout << "leftButton" << std::endl;
		mousePressed = FALSE;
		//std::cout << "mousePressed = FALSE" << std::endl;
		unsetCursor();
		if ( movingItem ) {
			//std::cout << "movingItem" << std::endl;
			movingItem = NULL;
			//std::cout << "movingItem = NULL" << std::endl;
			// tell qgsmapcanvas to reset layer order using the legend order
			emit zOrderChanged(this);
		}
	}
}
