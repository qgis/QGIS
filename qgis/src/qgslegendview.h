//
//
// C++ Interface: $MODULE$
//
// Description: Subclassed QListView that handles drag-n-drop for changing layer order
//
//
// Author: Steve Halasz <stevehalasz at users.sourceforge.net>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
