/***************************************************************************
                          qgslegend.h  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
               Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#ifndef QGSLEGEND_H
#define QGSLEGEND_H
#include <qlistview.h>
class QgsMapCanvas;
class QgsMapLayer;
class QListView;

class QPainter;
/**
* \class QgsLegend
* \brief Map legend
*
* The map legend is a Subclassed QListView that controls the display of legend items.
	*@author Gary E.Sherman
	*/

class QgsLegend:public QListView
{
  Q_OBJECT
	
public:
  /*! Constructor.
   * @param parent Parent widget
   * @param name Name of the widget
   */
  QgsLegend(QWidget * parent = 0, const char *name = 0);
  //! Destructor
   ~QgsLegend();
  //! Set the pointer to the map canvas
  void setMapCanvas(QgsMapCanvas * canvas);
  //! Update the legend
  void update();
  QString currentLayerName();

  QgsMapLayer *currentLayer();

protected:
  // override these to handle layer order manipulation
  void contentsMouseMoveEvent(QMouseEvent * e);
  void contentsMousePressEvent(QMouseEvent * e);
  void contentsMouseReleaseEvent(QMouseEvent * e);

private:
  // the map canvas this legend refers to
  QgsMapCanvas * map;
  // location of mouse press
  QPoint presspos;
  // keep track of if the mouse is pressed or not
  bool mousePressed;
  // keep track of the Item being dragged
  QListViewItem *movingItem;
  // keep track of the original position of the Item being dragged
  int movingItemOrigPos;
  // return position of item in the list
  int getItemPos(QListViewItem * item);

signals:
  // broadcast that the stacking order has changed
  // so the map canvas can be redrawn
  void zOrderChanged(QgsLegend * lv);
};
#endif
