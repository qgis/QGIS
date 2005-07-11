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
#include <qptrlist.h>
#include <qobjectlist.h>
#include <qapplication.h>

#include "qgslegend.h"
#include "qgslegenditem.h"
#include "qgslegendgroup.h"
#include "qgslegendlayer.h"
#include "qgslegendpropertygroup.h"
#include "qgslegendsymbologygroup.h"
#include "qgslegendlayerfile.h"
#include "qgsmaplayer.h"
#include <iostream>

static const char *const ident_ = "$Id$";

const int AUTOSCROLL_MARGIN = 16;

/**
   @note
 
   set mItemBeingMoved pointer to 0 to prevent SuSE 9.0 crash
*/
QgsLegend::QgsLegend(QgisApp* app, QWidget * parent, const char *name)
    : QListView(parent, name), mApp(app), mMousePressedFlag(false), mItemBeingMoved(0)
{
  connect( this, SIGNAL(selectionChanged(QListViewItem *)),
           this, SLOT(updateLegendItem(QListViewItem *)) );

  mPopupMenu = new QPopupMenu(this);
  mPopupMenu->insertItem("&Add Group", this, SLOT(addGroup()));
  mPopupMenu->insertItem("&Remove", this, SLOT(addGroup()));
  mPopupMenu->insertItem("&Add Group", this, SLOT(addGroup()));
  mPopupMenu->insertItem("&Add Group", this, SLOT(addGroup()));
  mPopupMenu->insertItem("&Add Group", this, SLOT(addGroup()));
  mPopupMenu->insertItem("&Add Group", this, SLOT(addGroup()));

  setSorting(-1);
}



QgsLegend::~QgsLegend()
{}

void QgsLegend::addGroup()
{
  std::cout << "in addGroup" << std::endl << std::flush;
}
void QgsLegend::updateLegendItem( QListViewItem * li )
{
  QgsLegendItem * qli = dynamic_cast<QgsLegendItem*>(li);

  if ( ! qli )
  {
    qDebug( "QgsLegend::updateLegendItem(): couldn't get QgsLegendItem" );
    return;
  }



} // QgsLegend::updateLegendItem




void QgsLegend::removeAll()
{
  clear();
} // QgsLegend::removeAll()

void QgsLegend::removeLayer(QString layer_key)
{
    QListViewItemIterator it(this);

    for (; it.current(); ++it)
    {
	QgsLegendItem *li = dynamic_cast<QgsLegendItem*>(it.current());
	if(li)
	{
	    QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(li);
	    if(llf)
	    {
#ifdef QGISDEBUG
		qWarning("comparing layer strings " + llf->layer()->getLayerID() +" and " + layer_key);
#endif
		if (llf->layer()&&llf->layer()->getLayerID() == layer_key)
		{
		    delete llf;
		    break;
		}
	    }
	}
    }
}

void QgsLegend::contentsMousePressEvent(QMouseEvent * e)
{
  if (e->button() == LeftButton)
  {
    QPoint p(contentsToViewport(e->pos()));
    QListViewItem *i = itemAt(p);
    if (i)
    {
      QgsLegendItem::LEGEND_ITEM_TYPE myTpe =
        dynamic_cast<QgsLegendItem*>(i)->type();
      std::cout << myTpe << std::endl;
      mLastPressPos = e->pos();
      mMousePressedFlag = TRUE;
    }
  }

  else if (e->button() == RightButton)
  {
    //add special behaviours for context menu etc if we want!
    //QPoint p(e->pos());
    //QPoint p(contentsToViewport(e->pos()));
    //QListViewItem *i = itemAt(p);
    //showContextMenu(i,p);
    //if (i)
    //{
      //mLastPressPos = e->pos();
      //mMousePressedFlag = TRUE;
      //}
  }

  QListView::contentsMousePressEvent(e);

}                               // contentsMousePressEvent



void QgsLegend::contentsMouseMoveEvent(QMouseEvent * e)
{
  if (mMousePressedFlag)
  {
    mMousePressedFlag = FALSE;
    // remember item we've pressed as the one being moved
    // and where it was originally
    QListViewItem *item = itemAt(contentsToViewport(mLastPressPos));
    if (item)
    {
      mItemBeingMoved = item;
      mItemBeingMovedOrigPos = getItemPos(mItemBeingMoved);
      setCursor(SizeVerCursor);
    }
  }
  else if (mItemBeingMoved)
  {
    // scroll list if we're near the edge of the viewport
    // code lifted from the poa project: http://poa.berlios.de/
    QPoint p(contentsToViewport(e->pos()));
    mLastPressPos=p;
    if (p.y() < AUTOSCROLL_MARGIN)
    {
      // scroll up
      scrollBy(0, -(AUTOSCROLL_MARGIN - p.y()));
    }
    else if (p.y() > visibleHeight() - AUTOSCROLL_MARGIN)
    {
      // scroll down
      scrollBy(0, (p.y() - (visibleHeight() - AUTOSCROLL_MARGIN)));
    }


    // change the cursor appropriate to if drop is allowed
    QListViewItem *item = itemAt(p);
    QgsLegendItem* origin = dynamic_cast<QgsLegendItem*>(mItemBeingMoved);
    QgsLegendItem* dest = dynamic_cast<QgsLegendItem*>(item);

    if (item && (item != mItemBeingMoved))
    {

      if(dest->accept(origin->type()))
      {
        std::cout << "accept" << std::endl << std::flush;

        setCursor( QCursor(Qt::PointingHandCursor) );
      }
      else if((origin->type() == dest->type()) && (origin->type() == QgsLegendItem::LEGEND_LAYER || origin->type() == QgsLegendItem::LEGEND_GROUP))
      {
        std::cout << "switch" << std::endl << std::flush;

        setCursor( QCursor(Qt::PointingHandCursor) );
      }
      else
      {
        std::cout << "rejeict" << std::endl << std::flush;
        setCursor( QCursor(Qt::ForbiddenCursor) );
      }
    }
  }
}

void QgsLegend::contentsMouseReleaseEvent(QMouseEvent * e)
{
  setCursor(QCursor(Qt::ArrowCursor));
  QListView::contentsMouseReleaseEvent(e);
  if (e->button() == LeftButton)
  {

    if (mItemBeingMoved)
    {
      QListViewItem *item = itemAt(mLastPressPos);
      QListViewItem *destItem = itemAt(e->pos());

      QgsLegendItem* origin = dynamic_cast<QgsLegendItem*>(mItemBeingMoved);
      QgsLegendItem* dest = dynamic_cast<QgsLegendItem*>(destItem);

      if (dest && origin && (origin != dest))
      {
        //  && (origin->parent() != dest->parent()) if you remove this below you can drop groups in other groups
        if(dest->accept(origin->type()) && (origin->parent() != dest->parent()))
        {
          std::cout << "accept" << std::endl << std::flush;
          std::cout << "About to drop onto this node " << std::endl;
          dest->print(dest);

          dest->insertItem(origin);

          if (dest->type()==QgsLegendItem::LEGEND_LAYER)
          {
            dest->sortChildItems(0,true);
          }
        }
        else if((origin->type() == dest->type()) && (origin->type() == QgsLegendItem::LEGEND_LAYER || origin->type() == QgsLegendItem::LEGEND_GROUP))
        {
          std::cout << "About to switch positions" << std::endl;
          QRect rect = itemRect(destItem);
          int height = rect.height();
          int top = rect.top();
          int mid = top + (height / 2);
          if (e->y() < mid)
          {
            // move item we're over now to after one in motion
            // unless it's already there
            if (mItemBeingMoved->nextSibling() != destItem)
            {
              if(origin->parent() != dest->parent())
              {
                dest->parent()->insertItem(origin);
                mItemBeingMoved->moveItem(destItem);
                destItem->moveItem(mItemBeingMoved);
              }
              else
              {
                destItem->moveItem(mItemBeingMoved);
              }
            }
          }
          else
          {
            // move item in motion to after one we're over now
            // unless it's already there
            if (mItemBeingMoved != destItem->nextSibling())
            {
              mItemBeingMoved->moveItem(destItem);
            }
          }
        }
        else
        {

          std::cout << "About to reject drop onto this node " << std::endl;
          dest->print(dest);
          std::cout << "rejeict" << std::endl << std::flush;
        }

	emit zOrderChanged(this);

        // find if we're over the top or bottom half of the item
        /*QRect rect = itemRect(item);
        int height = rect.height();
        int top = rect.top();
        int mid = top + (height / 2);
        if (e->y() < mid)
        {
          // move item we're over now to after one in motion
          // unless it's already there
          if (mItemBeingMoved->nextSibling() != item)
          {
            item->moveItem(mItemBeingMoved);
          }
        }
        else
        {
          // move item in motion to after one we're over now
          // unless it's already there
          if (mItemBeingMoved != item->nextSibling())
          {
            mItemBeingMoved->moveItem(item);
          }
        }*/
      }
    }
  }

  mMousePressedFlag = FALSE;
  mItemBeingMoved = NULL;

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

void QgsLegend::showContextMenu(QListViewItem * lvi, const QPoint & pt)
{
  //if (!mMapCanvas->isDrawing()&&lvi)
  //{
  // get the context menu from the layer and display it
  //  QgsMapLayer *layer = ((QgsLegendItem *) lvi)->layer();
  //  QPopupMenu *mPopupMenu = layer->contextMenu();
  //  if (mPopupMenu)
  //  {
  mPopupMenu->exec(pt);
  //  }
  //}
}

void QgsLegend::addLayer( QgsMapLayer * layer )
{
    QgsLegendGroup * lgroup = new QgsLegendGroup(this,QString("Layer Group"));
    QgsLegendLayer * llayer = new QgsLegendLayer(lgroup,QString("LegendLayer"));
    QgsLegendPropertyGroup * lpgroup = new QgsLegendPropertyGroup(llayer,QString("Properties Group"));
    QgsLegendSymbologyGroup * lsgroup = new QgsLegendSymbologyGroup(llayer,QString("Symbology Group"));
    QgsLegendLayerFile * llfile = new QgsLegendLayerFile(llayer,layer->name(), layer);
    layer->initContextMenu(mApp);

    lgroup->setOpen(true);
    llayer->setOpen(true);
    lpgroup->setOpen(true);
    lsgroup->setOpen(true);
    lsgroup->setOpen(true);
    
}

QgsMapLayer* QgsLegend::currentLayer()
{
    QgsLegendItem* citem=dynamic_cast<QgsLegendItem*>(currentItem());
    if(citem)
    {
	QgsLegendLayerFile* llf=dynamic_cast<QgsLegendLayerFile*>(citem);
	if(llf)
	{
	    return llf->layer();
	}
	else
	{
	    return 0;
	}
    }
    else
    {
	return 0;
    }
}
