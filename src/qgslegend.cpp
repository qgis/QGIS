/***************************************************************************
                          qgslegend.cpp  -  description
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
 /* $Id$ */
#include <map>


#include <qcursor.h>
#include <qstring.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qlistview.h>


#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgslegend.h"


static const char* const ident_ 
 = "$Id$";


QgsLegend::QgsLegend(QWidget * parent, const char *name):QListView(parent, name), mousePressed(FALSE)
{
  // set movingItem pointer to 0 to prevent SuSE 9.0 crash
  movingItem = 0;
}

QgsLegend::~QgsLegend()
{
}


void QgsLegend::setMapCanvas(QgsMapCanvas * canvas)
{
  map = canvas;
}

QgsMapLayer *QgsLegend::currentLayer()
{
  QgsLegendItem *li = (QgsLegendItem *) currentItem();

  if (li)
    return li->layer();
  else
    return 0;


}

QString QgsLegend::currentLayerName()
{
  QListViewItem *li = currentItem();
  if (li)
    return li->text(0);

  else
    return 0;
}

void QgsLegend::update()
{
  // clear the legend
  clear();
  
  std::list < QString >::iterator zi = map->zOrder.begin();
  while (zi != map->zOrder.end())
    {
      QgsMapLayer *lyr = map->layerByName(*zi);
      QgsLegendItem *lvi = new QgsLegendItem(lyr, this);  // lyr->name(), QCheckListItem::CheckBox );
      lyr->setLegendItem(lvi);
      lvi->setPixmap(0, *lyr->legendPixmap());
      zi++;
    }

  // highlight the current item

  setSelected( currentItem(), true );


  emit currentChanged(currentItem());

// Get the list of layers in order from the
// map canvas and add legenditems to the legend

/*
	for (int idx = 0; idx < map->layerCount(); idx++) {
		QgsMapLayer *lyr = map->getZpos(idx);
		if(lyr)
			QgsLegendItem *lvi = new QgsLegendItem(lyr, listView);	// lyr->name(), QCheckListItem::CheckBox );
		//lvi->setOn(lyr->visible());
//  QgsLegendItem *li = new QgsLegendItem(lyr, legendContainer);
		//addChild(li,0,idx*60);
		int foo = 1;
		//repaint();
	}
	*/
}

void QgsLegend::contentsMousePressEvent(QMouseEvent * e)
{
  if (e->button() == LeftButton)
    {
      QPoint p(contentsToViewport(e->pos()));
      QListViewItem *i = itemAt(p);
      if (i)
        {
          presspos = e->pos();
          mousePressed = TRUE;
        }
    }
  QListView::contentsMousePressEvent(e);
}

void QgsLegend::contentsMouseMoveEvent(QMouseEvent * e)
{
  if (mousePressed)
    {
      mousePressed = FALSE;
      // remember item we've pressed as the one being moved
      // and where it was originally
      QListViewItem *item = itemAt(contentsToViewport(presspos));
      if (item)
        {
          movingItem = item;
          movingItemOrigPos = getItemPos(movingItem);
          setCursor(SizeVerCursor);
        }
  } else if (movingItem)
    {
      // move item in list if we're dragging over another item
      QListViewItem *item = itemAt(e->pos());
      if (item && (item != movingItem))
        {
          // find if we're over the top or bottom half of the item
          QRect rect = itemRect(item);
          int height = rect.height();
          int top = rect.top();
          int mid = top + (height / 2);
          if (e->y() < mid)
            {
              // move item we're over now to after one in motion
              // unless it's already there
              if (movingItem->nextSibling() != item)
                {
                  item->moveItem(movingItem);
                }
          } else
            {
              // move item in motion to after one we're over now
              // unless it's already there
              if (movingItem != item->nextSibling())
                {
                  movingItem->moveItem(item);
                }
            }
        }
    }
}

void QgsLegend::contentsMouseReleaseEvent(QMouseEvent * e)
{
  QListView::contentsMouseReleaseEvent(e);
  if (e->button() == LeftButton)
    {
      mousePressed = FALSE;
      unsetCursor();
      if (movingItem)
        {
          if (getItemPos(movingItem) != movingItemOrigPos)
            {
              movingItem = NULL;
              // tell qgsmapcanvas to reset layer order using the legend order
              emit zOrderChanged(this);
          } else
            {
              movingItem = NULL;
            }
        }
    }
}

int QgsLegend::getItemPos(QListViewItem * item)
{
  QListViewItemIterator it(this);
  int index = 0;
  while (it.current())
    {
      QgsLegendItem *li = (QgsLegendItem *) it.current();
      if (item == li)
        {
          break;
        }
      ++it;
      ++index;
    }
  return index;
}
