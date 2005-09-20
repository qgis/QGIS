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
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayerproperties.h"
#include <iostream>
#include <qlayout.h>

static const char *const ident_ = "$Id$";

const int AUTOSCROLL_MARGIN = 16;

/**
   @note
 
   set mItemBeingMoved pointer to 0 to prevent SuSE 9.0 crash
*/
QgsLegend::QgsLegend(QgisApp* app, QWidget * parent, const char *name)
    : QListView(parent, name), mApp(app), mMousePressedFlag(false), mItemBeingMoved(0), mMapCanvas(0)
{
  connect( this, SIGNAL(selectionChanged(QListViewItem *)),
           this, SLOT(updateLegendItem(QListViewItem *)) );

  connect( this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
	   this, SLOT(handleRightClickEvent(QListViewItem*, const QPoint&)));

  connect( this, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)),
	   this, SLOT(handleDoubleClickEvent(QListViewItem*)));

  connect( this, SIGNAL(expanded(QListViewItem*)), this, SLOT(placeCheckBoxes()));
  connect( this, SIGNAL(collapsed(QListViewItem*)), this, SLOT(placeCheckBoxes()));
  connect( this, SIGNAL(contentsMoving(int, int)), this, SLOT(placeCheckBoxes()));

  setSorting(-1);
}



QgsLegend::~QgsLegend()
{}

void QgsLegend::addGroup()
{
    QgsLegendGroup* group = new QgsLegendGroup(this, tr("group"));
    group->setRenameEnabled(0, true);
    group->setOpen(true);
    placeCheckBoxes();
}

void QgsLegend::updateLegendItem( QListViewItem * li )
{
  QgsLegendItem * qli = dynamic_cast<QgsLegendItem*>(li);

  if ( ! qli )
  {
    qDebug( "QgsLegend::updateLegendItem(): couldn't get QgsLegendItem" );
    return;
  }

} 

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
    //update the overview canvas
    mApp->setOverviewZOrder(this);
}

void QgsLegend::contentsMousePressEvent(QMouseEvent * e)
{
#ifdef QGISDEBUG
    qWarning("this message comes from within QgsLegend::contentsMousePressEvent");
#endif
  if (e->button() == Qt::LeftButton || e->button() == Qt::MidButton)
  {
    QPoint p(contentsToViewport(e->pos()));
    QListViewItem *i = itemAt(p);
    mLastPressPos = e->pos();
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
	    setCursor(Qt::SizeVerCursor);
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

	    QgsLegendItem::DRAG_ACTION action = dest->accept(origin->type());
	    if(action == QgsLegendItem::REORDER)
	    {
		setCursor( QCursor(Qt::SizeVerCursor) );
	    }
	    else if(action == QgsLegendItem::INSERT)
	    {
		setCursor( QCursor(Qt::PointingHandCursor) );
	    }
	    else
	    {
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

      QgsLegendItem::DRAG_ACTION daction= dest->accept(origin->type());

      if (dest && origin && (origin != dest))
      {
	  if( daction == QgsLegendItem::INSERT )
	  {
	      dest->insert(origin);
	      placeCheckBoxes();
	  }
	  else if( daction == QgsLegendItem::REORDER)
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
	emit zOrderChanged(this);
      }
  }
  mMousePressedFlag = false;
  mItemBeingMoved = NULL;
}

void QgsLegend::handleDoubleClickEvent(QListViewItem* item)
{
    QgsLegendItem* li = dynamic_cast<QgsLegendItem*>(item);
    if(li)
    {
	if(li->type() == QgsLegendItem::LEGEND_LAYER_FILE)
	{
	    QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(li);
	    if(llf)
	    {
		QgsMapLayer* ml = llf->layer();
		if (ml && ml->type() == QgsMapLayer::RASTER)
		{
		    QgsRasterLayerProperties *rlp = new QgsRasterLayerProperties(ml);
		    if (rlp->exec())
		    {
			delete rlp;
			qApp->processEvents();
		    }
		}
		else if(ml) //vector
		{
		    ml->showLayerProperties();
		}
	    } 
	}
    }
}

void QgsLegend::handleRightClickEvent(QListViewItem* item, const QPoint& position)
{
    if(!mMapCanvas->isDrawing())
    {
#if defined(Q_OS_MACX) || defined(WIN32)
    QString iconsPath(qApp->applicationDirPath()+QString("/share/qgis"));
#else
    QString iconsPath(PKGDATAPATH);
#endif
    iconsPath += QString("/images/icons/");
	if(!item)//show list view popup menu
	{
	    QPopupMenu pm;
	    pm.insertItem(QIconSet(QPixmap(iconsPath+QString("folder_new.png"))), tr("&Add group"), this, SLOT(addGroup()));
	    pm.insertItem(QIconSet(QPixmap(iconsPath+QString("expand_tree.png"))), tr("&Expand all"), this, SLOT(expandAll()));
	    pm.insertItem(QIconSet(QPixmap(iconsPath+QString("collapse_tree.png"))), tr("&Collapse all"), this, SLOT(collapseAll()));
	    pm.exec(position);
	    return;
	}
	QgsLegendItem* li = dynamic_cast<QgsLegendItem*>(item);
	if(li)
	{
	    if(li->type() == QgsLegendItem::LEGEND_LAYER_FILE)
	    {
		QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(li);
		if(llf)
		{
		   QgsMapLayer* ml = llf->layer();
		   if(ml)
		   {
		       QPopupMenu *mPopupMenu = ml->contextMenu();
		       if (mPopupMenu)
		       {
			   mPopupMenu->exec(position);
		       }
		   }
		}
	    }
	    else if(li->type() == QgsLegendItem::LEGEND_LAYER)
	    {
		QPopupMenu pm;
		pm.insertItem(tr("&Remove"), this, SLOT(legendLayerRemove()));
		pm.insertItem(tr("&Properties"), this, SLOT(legendLayerShowProperties()));
		pm.insertItem(tr("&Add to overview"), this, SLOT(legendLayerAddToOverview()));
		pm.insertItem(tr("&Remove from overview"), this, SLOT(legendLayerRemoveFromOverview()));
		pm.insertItem(QIconSet(QPixmap(iconsPath+QString("folder_new.png"))), tr("&Add group"), this, SLOT(addGroup()));
		pm.insertItem(QIconSet(QPixmap(iconsPath+QString("expand_tree.png"))), tr("&Expand all"), this, SLOT(expandAll()));
		pm.insertItem(QIconSet(QPixmap(iconsPath+QString("collapse_tree.png"))), tr("&Collapse all"), this, SLOT(collapseAll()));
		pm.exec(position);
	    }
	    else if(li->type() == QgsLegendItem::LEGEND_GROUP)
	    {
		QPopupMenu pm;
		pm.insertItem(tr("&Remove"), this, SLOT(legendGroupRemove()));
		pm.insertItem(QIconSet(QPixmap(iconsPath+QString("folder_new.png"))), tr("&Add group"), this, SLOT(addGroup()));
		pm.insertItem(QIconSet(QPixmap(iconsPath+QString("expand_tree.png"))), tr("&Expand all"), this, SLOT(expandAll()));
		pm.insertItem(QIconSet(QPixmap(iconsPath+QString("collapse_tree.png"))), tr("&Collapse all"), this, SLOT(collapseAll()));
		pm.exec(position);
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

void QgsLegend::addLayer( QgsMapLayer * layer )
{
    //QgsLegendGroup * lgroup = new QgsLegendGroup(this,QString("Layer Group"));
    //lgroup->setRenameEnabled(0, true);
    QgsLegendLayer * llayer = new QgsLegendLayer(/*lgroup*/this,QString(layer->name()));
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

    /*lgroup->setOpen(true);*/
    llayer->setOpen(false);
    lpgroup->setOpen(false);
    lsgroup->setOpen(false); 
    llfgroup->setOpen(false);
    placeCheckBoxes();
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
	    QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer*>(citem);
	    if(ll)
	    {
		return ll->firstMapLayer();
	    }
	    else
	    {
		return 0;
	    }
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
    updateContents();
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
# ifdef QGISDEBUG
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
	    cbox->setGeometry(/*3*/2*treeStepSize()-14, ypos, 14, 14);
	    cbox->show();
	}
    }
    else
    {
	cbox->hide();
    }
}

void QgsLegend::legendGroupRemove()
{
    QgsLegendGroup* lg = dynamic_cast<QgsLegendGroup*>(currentItem());
    if(lg)
    {
	//delete the legend layers first
	QListViewItem * child = lg->firstChild();
        while(child) 
	{
	    setCurrentItem(child);
	    legendLayerRemove();
            child = lg->firstChild();
        }
	delete lg;
    }
}

void QgsLegend::legendLayerRemove()
{
    //remove all layers of the current legendLayer
   QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer*>(currentItem());
   if(!ll)
   {
       return;
   }
   std::list<QgsMapLayer*> maplayers = ll->mapLayers();

   for(std::list<QgsMapLayer*>::iterator it = maplayers.begin(); it!=maplayers.end(); ++it)
   {
       //remove the layer
       if(*it)
       {
	   QgsMapLayerRegistry::instance()->removeMapLayer((*it)->getLayerID());
       }
   }

   if(maplayers.size()>0)
   {
       mMapCanvas->removeDigitizingLines();
       mMapCanvas->clear();
       mMapCanvas->render();
   }
   delete ll;
   placeCheckBoxes();
   //update the overview canvas
   mApp->setOverviewZOrder(this);
}

void QgsLegend::legendLayerAddToOverview()
{
   //add or remove all layers to/ from overview
   QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer*>(currentItem());
   if(!ll)
   {
       return;
   }

   std::list<QgsMapLayer*> maplayers = ll->mapLayers();
   for(std::list<QgsMapLayer*>::iterator it = maplayers.begin(); it!=maplayers.end(); ++it)
   {
       if(*it)
       {
	       (*it)->inOverview(true);
       }
   }

   if(maplayers.size()>0)
   {
       //update the overview canvas
       mApp->setOverviewZOrder(this);
   } 
}

void QgsLegend::legendLayerRemoveFromOverview()
{
    //add or remove all layers to/ from overview
   QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer*>(currentItem());
   if(!ll)
   {
       return;
   }
   std::list<QgsMapLayer*> maplayers = ll->mapLayers();

   for(std::list<QgsMapLayer*>::iterator it = maplayers.begin(); it!=maplayers.end(); ++it)
   {
       if(*it)
       {
	   (*it)->inOverview(false); //else
       }
   }

   if(maplayers.size()>0)
   {
       //update the overview canvas
       mApp->setOverviewZOrder(this);
   } 
}

void QgsLegend::legendLayerShowProperties()
{
   QgsLegendItem* citem=dynamic_cast<QgsLegendItem*>(currentItem());
    if(!citem)
    {
	return;
    }
    QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer*>(citem);
    if(!ll)
    {
	return;
    }
    QgsMapLayer* ml = ll->firstMapLayer(); 
    if(!ml)
    {
	return;
    }
    ml->showLayerProperties();
}

void QgsLegend::expandAll()
{
    QListViewItemIterator it(this);
    while(it.current()) 
    {
	QListViewItem *item = it.current();
	item->setOpen(true);
	++it;
    }
}

void QgsLegend::collapseAll()
{
   QListViewItemIterator it(this);
    while(it.current()) 
    {
	QListViewItem *item = it.current();
	item->setOpen(false);
	++it;
    } 
}
