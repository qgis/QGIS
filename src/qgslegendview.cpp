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
//
#include <qapplication.h>
#include <qdragobject.h>
#include <qlistview.h>
#include <qpoint.h>

#include "qgslegendview.h"

QgsLegendView::QgsLegendView( QWidget *parent, const char *name ):QListView( parent, name ), mousePressed( FALSE )
{
	setAcceptDrops( TRUE );
	viewport()->setAcceptDrops( TRUE );
}

void QgsLegendView::contentsDragEnterEvent( QDragEnterEvent* e )
{
	QString text;
	if ( QTextDrag::decode(e, text) ) {
		// verify text is 'legend-drag' to avoid
		// handling regular text drops
		if ( text.compare("legend-drag") == 0 ) {
			e->accept( TRUE );
		}
	}
}

void QgsLegendView::contentsDropEvent(QDropEvent* e)
{
	QString text;
	if ( QTextDrag::decode(e, text) ) {
		// verify text is 'legend-drag' to avoid
		// handling regular text drops
		if ( text.compare("legend-drag") == 0 ) {
			QListViewItem *item = itemAt( contentsToViewport(e->pos()) );
			// insert item in motion after one it was dropped on
			movingItem->moveItem( item );
			// avoid having more than one layer appear selected after drop
			setCurrentItem(movingItem);
			emit zOrderChanged(this);
		}
	} else {
		e->ignore();
		return;
	}
}
		
void QgsLegendView::contentsMousePressEvent( QMouseEvent* e )
{
	QListView::contentsMousePressEvent( e );
	QPoint p( contentsToViewport( e->pos() ) );
	QListViewItem *i = itemAt( p );
	if ( i ) {
		presspos = e->pos();
		mousePressed = TRUE;
	}
}

void QgsLegendView::contentsMouseMoveEvent( QMouseEvent* e )
{
	if ( mousePressed && ( presspos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
		mousePressed = FALSE;
		QListViewItem *item = itemAt( contentsToViewport(presspos) );
		if ( item ) {
			movingItem = item;
			// create drag object with some useless text
			QDragObject *d = new QTextDrag( "legend-drag", this );
			d->drag();
		}
	}
}

void QgsLegendView::contentsMouseReleaseEvent( QMouseEvent * )
{
	mousePressed = FALSE;
}
