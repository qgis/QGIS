/***************************************************************************
                          qgslegendview.h 
 Subclassed QListView that handles drag-n-drop for changing layer order
                             -------------------
    begin                : 2004-02-12
    copyright            : (C) 2004 by Steve Halasz
    email                : <stevehalasz at users.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
#include <qlistview.h>

class QgsLegendView : public QListView
{
	Q_OBJECT
	
public:
	QgsLegendView( QWidget *parent=0, const char *name=0 );
	
protected:
	// override these to handle layer order manipulation
	void contentsMouseMoveEvent( QMouseEvent *e );
	void contentsMousePressEvent( QMouseEvent *e );
	void contentsMouseReleaseEvent( QMouseEvent *e );
	
signals:
	// broadcast that the stacking order has changed
	// so the map canvas can be redrawn
	void zOrderChanged( QgsLegendView *lv );
	
private:
	// location of mouse press
	QPoint presspos;
	// keep track of if the mouse is pressed or not
	bool mousePressed;
	// keep track of the Item being dragged
	QListViewItem *movingItem;
	// keep track of the original position of the Item being dragged
	int movingItemOrigPos;
	// return position of item in the list
	int getItemPos(QListViewItem *item);
};
