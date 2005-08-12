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

#include <qcheckbox.h>
#include <qcursor.h>
#include <qheader.h>
#include <qstring.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qobjectlist.h>
#include <qapplication.h>

#include "qgslegend.h"
#include "qgslegendgroup.h"
#include "qgslegendlayer.h"
#include "qgslegendpropertygroup.h"
#include "qgslegendsymbologygroup.h"
#include "qgslegendlayerfile.h"
#include "qgslegendlayerfilegroup.h"
#include "qgsmaplayer.h"
#include <iostream>

static const char *const ident_ = "$Id$";

const int AUTOSCROLL_MARGIN = 16;

/**
   @note
 
   set mItemBeingMoved pointer to 0 to prevent SuSE 9.0 crash
*/
QgsLegend::QgsLegend(QgisApp* app, QWidget * parent, const char *name)
    : QListView(parent, name), mApp(app), mMouseDragType(QgsLegendItem::NO_DRAG), mMousePressedFlag(false), mItemBeingMoved(0), mMapCanvas(0)
{
  connect( this, SIGNAL(selectionChanged(QListViewItem *)),
           this, SLOT(updateLegendItem(QListViewItem *)) );

  connect( this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
	   this, SLOT(distributeRightClickEvent(QListViewItem*, const QPoint&)));

  connect( this, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)),
	   this, SLOT(distributeDoubleClickEvent(QListViewItem*)));

  connect( this, SIGNAL(expanded(QListViewItem*)), this, SLOT(placeCheckBoxes()));
  connect( this, SIGNAL(collapsed(QListViewItem*)), this, SLOT(placeCheckBoxes()));
  connect( this, SIGNAL(contentsMoving(int, int)), this, SLOT(placeCheckBoxes()));

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
		if (llf->layer()&&llf->layer()->getLayerID() == layer_key)
		{
		    //unregister listview/checkbox pair
		    //deleting is done by the destructor of QgsLegendLayerFile
		    unregisterCheckBox(llf);
		    delete llf;
		    placeCheckBoxes();
		    break;
		}
	    }
	}
    }
}

void QgsLegend::contentsMousePressEvent(QMouseEvent * e)
{
  if (e->button() == Qt::LeftButton || e->button() == Qt::MidButton)
  {
    QPoint p(contentsToViewport(e->pos()));
    QListViewItem *i = itemAt(p);
    if (i)
    {
      QgsLegendItem::LEGEND_ITEM_TYPE myTpe =
        dynamic_cast<QgsLegendItem*>(i)->type();
      std::cout << myTpe << std::endl;
      mLastPressPos = e->pos();
      
      if( e->button() == Qt::LeftButton )
      {
	  mMouseDragType = QgsLegendItem::REORDER;
      }
      else if( e->button() == Qt::MidButton )
      {
	  mMouseDragType = QgsLegendItem::INSERT;
      }
      else
      {
	  mMouseDragType = QgsLegendItem::NO_DRAG;
      }
    }
    mMousePressedFlag = true;
  }
  QListView::contentsMousePressEvent(e);
}                               // contentsMousePressEvent


void QgsLegend::contentsMouseMoveEvent(QMouseEvent * e)
{
    if(mMousePressedFlag)
    {
	//set the flag back such that the else if(mItemBeingMoved)
	//code part is passed during the next mouse moves
	mMousePressedFlag = false;

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

      if(dest->accept(mMouseDragType, origin->type()))
      {
        std::cout << "accept" << std::endl << std::flush;

        setCursor( QCursor(Qt::PointingHandCursor) );
      }
      //else if((origin->type() == dest->type()) && (origin->type() == QgsLegendItem::LEGEND_LAYER || origin->type() == QgsLegendItem::LEGEND_GROUP))
      //{
      //std::cout << "switch" << std::endl << std::flush;

      //setCursor( QCursor(Qt::PointingHandCursor) );
      //}
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

  if (mItemBeingMoved)
  {
      QListViewItem *item = itemAt(mLastPressPos);
      QListViewItem *destItem = itemAt(e->pos());
      
      QgsLegendItem* origin = dynamic_cast<QgsLegendItem*>(mItemBeingMoved);
      QgsLegendItem* dest = dynamic_cast<QgsLegendItem*>(destItem);

      if (dest && origin && (origin != dest))
      {
	  if( mMouseDragType == QgsLegendItem::INSERT )
	  {
	      if(dest->accept(mMouseDragType, origin->type()))
	      {
		  dest->insert(origin);
		  placeCheckBoxes();
	      }
	  }
	  else if( mMouseDragType == QgsLegendItem::REORDER)
	  {
	      if(dest->accept(mMouseDragType, origin->type()))
	      {
		  //find out, if mouse is over the top or the bottom of the item
		  QRect rect = itemRect(dest);
		  int height = rect.height();
		  int top = rect.top();
		  int mid = top + (height / 2);
		  if (e->y() < mid) //bottom
		  {
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
		  else //top
		  {
		     if (mItemBeingMoved != destItem->nextSibling())
		     {
			 mItemBeingMoved->moveItem(destItem);
		     } 
		  }
		  placeCheckBoxes();
	      }
	  }
	
	  /*if(dest->accept(mMousePressedFlag, origin->type()) && (origin->parent() != dest->parent()))
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
	  }*/

	emit zOrderChanged(this);
      }
  }

  mMouseDragType = QgsLegendItem::NO_DRAG;
  mMousePressedFlag = false;
  mItemBeingMoved = NULL;

}

void QgsLegend::distributeDoubleClickEvent(QListViewItem* item)
{
    QgsLegendItem* li = dynamic_cast<QgsLegendItem*>(item);
    if(li)
    {
	li->handleDoubleClickEvent();
    }
}

void QgsLegend::distributeRightClickEvent(QListViewItem* item, const QPoint& position)
{
    if(!mMapCanvas->isDrawing())
    {
	QgsLegendItem* li = dynamic_cast<QgsLegendItem*>(item);
	if(li)
	{
	    li->handleRightClickEvent(position);
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
    lgroup->setRenameEnabled(0, true);
    QgsLegendLayer * llayer = new QgsLegendLayer(lgroup,QString(layer->name()));
    llayer->setRenameEnabled(0, true);
    QgsLegendPropertyGroup * lpgroup = new QgsLegendPropertyGroup(llayer,QString("Properties"));
    QgsLegendSymbologyGroup * lsgroup = new QgsLegendSymbologyGroup(llayer,QString("Symbology"));
    QgsLegendLayerFileGroup * llfgroup = new QgsLegendLayerFileGroup(llayer,QString("Files"));
    layer->setLegendSymbologyGroupParent(lsgroup);
    QString sourcename = layer->source();
    if(sourcename.startsWith("host", false))
    {
	//this layer is a database layer
	//modify source string such that password is not visible
	sourcename = layer->name();
    }
    else
    {
	//modify source name such that only the file is visible
	sourcename = layer->source().section('/',-1,-1);
    }
    QgsLegendLayerFile * llfile = new QgsLegendLayerFile(llfgroup, sourcename, layer);
    layer->setLegendLayerFile(llfile);
    layer->initContextMenu(mApp);

    lgroup->setOpen(true);
    llayer->setOpen(false);
    lpgroup->setOpen(false);
    lsgroup->setOpen(false); 
    llfgroup->setOpen(false);
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

void QgsLegend::registerCheckBox(QListViewItem* item, QCheckBox* cbox)
{
    mCheckBoxes.insert(std::make_pair(item, cbox) );
    placeCheckBox(item, cbox);
}

void QgsLegend::unregisterCheckBox(QListViewItem* item)
{
    mCheckBoxes.erase(item);
}

void QgsLegend::placeCheckBoxes()
{
    for(std::map<QListViewItem*, QCheckBox*>::iterator it = mCheckBoxes.begin(); it!=mCheckBoxes.end(); ++it)
    {
	placeCheckBox(it->first, it->second);
    }
}

void QgsLegend::placeCheckBox(QListViewItem* litem, QCheckBox* cbox)
{
    QRect itempos = itemRect(litem);
    if(itempos.isValid())//rectangle is not valid if the item is not visible
    { 
	int yoffset = viewport()->geometry().top();
	int headersheight = header()->height();
	int ypos = itempos.top()+5+yoffset;
#ifdef QGISDEBUG
	qWarning("yoffset: "+QString::number(yoffset));
	qWarning("headersheight: "+QString::number(headersheight));
	qWarning("ypos: "+QString::number(ypos));
	qWarning("itemtext: "+litem->text(0));
#endif
	//test, if item intersects the header
	if(ypos < headersheight)
	{
	    cbox->hide();
	}
	else
	{
	    cbox->setGeometry(3*treeStepSize()-14, ypos, 14, 14);
	    cbox->show();
	}
    }
    else
    {
	cbox->hide();
    }
}
