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
#include <q3header.h>
#include <qstring.h>
#include <qpainter.h>
#include <qlabel.h>
#include <q3vbox.h>
#include <q3listview.h>
#include <q3ptrlist.h>
#include <qobject.h>
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
#include "qgsproject.h"
#include "qgsrasterlayerproperties.h"
//Added by qt3to4:
#include <QPixmap>
#include <Q3PopupMenu>
#include <QMouseEvent>
#include <iostream>
#include <qlayout.h>

static const char *const ident_ = "$Id$";

const int AUTOSCROLL_MARGIN = 16;

/**
   @note
 
   set mItemBeingMoved pointer to 0 to prevent SuSE 9.0 crash
*/
QgsLegend::QgsLegend(QgisApp* app, QWidget * parent, const char *name)
    : Q3ListView(parent, name), mApp(app), mMousePressedFlag(false), mItemBeingMoved(0), mMapCanvas(0)
{
  connect( this, SIGNAL(selectionChanged(Q3ListViewItem *)),
           this, SLOT(updateLegendItem(Q3ListViewItem *)) );

  connect( this, SIGNAL(rightButtonPressed(Q3ListViewItem *, const QPoint &, int)),
	   this, SLOT(handleRightClickEvent(Q3ListViewItem*, const QPoint&)));

  connect( this, SIGNAL(doubleClicked(Q3ListViewItem *, const QPoint &, int)),
	   this, SLOT(handleDoubleClickEvent(Q3ListViewItem*)));

  connect( this, SIGNAL(expanded(Q3ListViewItem*)), this, SLOT(placeCheckBoxes()));
  connect( this, SIGNAL(collapsed(Q3ListViewItem*)), this, SLOT(placeCheckBoxes()));
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

void QgsLegend::updateLegendItem( Q3ListViewItem * li )
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
    mCheckBoxes.clear();
    clear();
}

void QgsLegend::removeLayer(QString layer_key)
{
    Q3ListViewItemIterator it(this);

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
    mLastPressPos = e->pos();
    mMousePressedFlag = true;
  }
  Q3ListView::contentsMousePressEvent(e);
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
	Q3ListViewItem *item = itemAt(contentsToViewport(mLastPressPos));
	if (item)
	{
	    mItemBeingMoved = item;
	    mItemBeingMovedOrigPos = getItemPos(mItemBeingMoved);

	    //store information to insert the item back to the original position
	    storeInitialPosition(mItemBeingMoved);
 
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
	Q3ListViewItem *item = itemAt(p);
	QgsLegendItem* origin = dynamic_cast<QgsLegendItem*>(mItemBeingMoved);
	QgsLegendItem* dest = dynamic_cast<QgsLegendItem*>(item);

	if (item && (item != mItemBeingMoved))
	{

	    QgsLegendItem::DRAG_ACTION action = dest->accept(origin);
	    if(action == QgsLegendItem::REORDER)
	    {
	      if(!yCoordAboveCenter(dest, e->y())) //over bottom of item
	      {
		  if (mItemBeingMoved->nextSibling() != dest)
		  {
		      if(origin->parent() != dest->parent())
		      {
			  dest->parent()->insertItem(origin);
			  mItemBeingMoved->moveItem(dest);
			  dest->moveItem(mItemBeingMoved);
		      }
		      else
		      {
			  dest->moveItem(mItemBeingMoved);
		      }
		  }
	      }
	      else //over top of item
	      {
		  if (mItemBeingMoved != dest->nextSibling())
		  {
		      mItemBeingMoved->moveItem(dest);
		  } 
	      }
	      placeCheckBoxes();
	    }
	    else if(action == QgsLegendItem::INSERT)
	    {
	      setCursor( QCursor(Qt::PointingHandCursor) );
	      dest->insert(origin, false);
	      placeCheckBoxes();
	    }
	    else//no action
	    {
	      resetToInitialPosition(mItemBeingMoved);
	      setCursor( QCursor(Qt::ForbiddenCursor) );
	    }
	}     
    }
}


void QgsLegend::contentsMouseReleaseEvent(QMouseEvent * e)
{
  setCursor(QCursor(Qt::ArrowCursor));
  Q3ListView::contentsMouseReleaseEvent(e);

  if (mItemBeingMoved)
  {
      Q3ListViewItem *destItem = itemAt(e->pos());
      
      QgsLegendItem* origin = dynamic_cast<QgsLegendItem*>(mItemBeingMoved);
      QgsLegendItem* dest = dynamic_cast<QgsLegendItem*>(destItem);

      if(!dest || !origin)
      {
	  return;
      }

      if(dest && origin && getItemPos(dest) != mItemBeingMovedOrigPos)
      {
	QgsLegendItem::LEGEND_ITEM_TYPE originType = origin->type();
	QgsLegendItem::LEGEND_ITEM_TYPE destType = dest->type();

	if(originType == QgsLegendItem::LEGEND_LAYER_FILE && destType == QgsLegendItem::LEGEND_LAYER_FILE_GROUP)
	  {
	    QgsMapLayer* origLayer = ((QgsLegendLayerFile*)(origin))->layer();
	    
	    if(origLayer->legendSymbologyGroupParent() != dest->nextSibling())
	      {
		origLayer->setLegendSymbologyGroupParent((QgsLegendSymbologyGroup*)(dest->nextSibling()));
		if(dest->childCount() > 1)
		  {
		    //find the first layer in the legend layer group != origLayer and copy its settings
		    Q3ListViewItem* currentItem = dest->firstChild();
		    while(currentItem)
		      {
			if(currentItem != origin)
			  {
			    QgsMapLayer* origLayer = ((QgsLegendLayerFile*)(origin))->layer();
			    QgsMapLayer* currentLayer = ((QgsLegendLayerFile*)(currentItem))->layer();
			    origLayer->copySymbologySettings(*currentLayer);
			    break;
			  }
			currentItem = currentItem->nextSibling();
		      }
		  }
	      }
	  }
	else if(originType == QgsLegendItem::LEGEND_LAYER_FILE && destType == QgsLegendItem::LEGEND_LAYER_FILE)
	  {
	    QgsMapLayer* origLayer = ((QgsLegendLayerFile*)(origin))->layer();
	    QgsMapLayer* destLayer = ((QgsLegendLayerFile*)(dest))->layer();
	    if(dest == origin)//origin item has been moved in mouseMoveEvent such that it is under the mouse cursor now
	      {
		if(origLayer->legendSymbologyGroupParent() != origin->parent())
		  {
		    origLayer->setLegendSymbologyGroupParent((QgsLegendSymbologyGroup*)(origin->parent()->nextSibling()));
		    if(origin->parent()->childCount() > 1)
		      {
			//find the first layer in the legend layer group != origLayer and copy its settings
			Q3ListViewItem* currentItem = dest->parent()->firstChild();
			while(currentItem)
			  {
			    if(currentItem != origin)
			      {
				QgsMapLayer* origLayer = ((QgsLegendLayerFile*)(origin))->layer();
				QgsMapLayer* currentLayer = ((QgsLegendLayerFile*)(currentItem))->layer();
				origLayer->copySymbologySettings(*currentLayer);
				break;
			      }
			    currentItem = currentItem->nextSibling();
			  }
		      }
		  }
	      }
	    else
	      {
		QgsMapLayer* origLayer = ((QgsLegendLayerFile*)(origin))->layer();
		QgsMapLayer* destLayer = ((QgsLegendLayerFile*)(dest))->layer();
		origLayer->copySymbologySettings(*destLayer);
		origLayer->setLegendSymbologyGroupParent((QgsLegendSymbologyGroup*)(dest->parent()->nextSibling()));
	      }
	  }
	//placeCheckBoxes()
	emit zOrderChanged(this);
      }
  }
  mMousePressedFlag = false;
  mItemBeingMoved = NULL; 
}

void QgsLegend::handleDoubleClickEvent(Q3ListViewItem* item)
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

void QgsLegend::handleRightClickEvent(Q3ListViewItem* item, const QPoint& position)
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
	    Q3PopupMenu pm;
	    pm.insertItem(QIcon(QPixmap(iconsPath+QString("folder_new.png"))), tr("&Add group"), this, SLOT(addGroup()));
	    pm.insertItem(QIcon(QPixmap(iconsPath+QString("expand_tree.png"))), tr("&Expand all"), this, SLOT(expandAll()));
	    pm.insertItem(QIcon(QPixmap(iconsPath+QString("collapse_tree.png"))), tr("&Collapse all"), this, SLOT(collapseAll()));
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
		       Q3PopupMenu *mPopupMenu = ml->contextMenu();
		       if (mPopupMenu)
		       {
			   mPopupMenu->exec(position);
		       }
		   }
		}
	    }
	    else if(li->type() == QgsLegendItem::LEGEND_LAYER)
	    {
		Q3PopupMenu pm;
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("remove.png"))), tr("&Remove"), this, SLOT(legendLayerRemove()));
		pm.insertItem(tr("&Properties"), this, SLOT(legendLayerShowProperties()));
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("inoverview.png"))), tr("&Add to overview"), this, SLOT(legendLayerAddToOverview()));
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("remove_from_overview.png"))), tr("&Remove from overview"), this, SLOT(legendLayerRemoveFromOverview()));
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("folder_new.png"))), tr("&Add group"), this, SLOT(addGroup()));
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("expand_tree.png"))), tr("&Expand all"), this, SLOT(expandAll()));
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("collapse_tree.png"))), tr("&Collapse all"), this, SLOT(collapseAll()));
		pm.exec(position);
	    }
	    else if(li->type() == QgsLegendItem::LEGEND_GROUP)
	    {
		Q3PopupMenu pm;
		pm.insertItem(tr("&Remove"), this, SLOT(legendGroupRemove()));
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("folder_new.png"))), tr("&Add group"), this, SLOT(addGroup()));
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("expand_tree.png"))), tr("&Expand all"), this, SLOT(expandAll()));
		pm.insertItem(QIcon(QPixmap(iconsPath+QString("collapse_tree.png"))), tr("&Collapse all"), this, SLOT(collapseAll()));
		pm.exec(position);
	    }
	}
    }
}

int QgsLegend::getItemPos(Q3ListViewItem * item)
{
  Q3ListViewItemIterator it(this);
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
    QgsLegendLayer * llayer = new QgsLegendLayer(/*lgroup*/this,QString(layer->name()));
    llayer->setRenameEnabled(0, true);
    QgsLegendPropertyGroup * lpgroup = new QgsLegendPropertyGroup(llayer,QString("Properties"));
    QgsLegendSymbologyGroup * lsgroup = new QgsLegendSymbologyGroup(llayer,QString("Symbology"));
    QgsLegendLayerFileGroup * llfgroup = new QgsLegendLayerFileGroup(llayer,QString("Files"));
    layer->setLegendSymbologyGroupParent(lsgroup);
    QgsLegendLayerFile * llfile = new QgsLegendLayerFile(llfgroup, QgsLegendLayerFile::nameFromLayer(layer), layer);
    layer->setLegendLayerFile(llfile);
    layer->initContextMenu(mApp);

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

void QgsLegend::registerCheckBox(Q3ListViewItem* item, QCheckBox* cbox)
{
    mCheckBoxes.insert(std::make_pair(item, cbox) );
    placeCheckBox(item, cbox);
}

void QgsLegend::unregisterCheckBox(Q3ListViewItem* item)
{
    mCheckBoxes.erase(item);
}

void QgsLegend::placeCheckBoxes()
{
    updateContents();
    for(std::map<Q3ListViewItem*, QCheckBox*>::iterator it = mCheckBoxes.begin(); it!=mCheckBoxes.end(); ++it)
    {
	placeCheckBox(it->first, it->second);
    }
}

void QgsLegend::placeCheckBox(Q3ListViewItem* litem, QCheckBox* cbox)
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
	    cbox->setGeometry(2*treeStepSize()-14, ypos, 14, 14);
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
	Q3ListViewItem * child = lg->firstChild();
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
    Q3ListViewItemIterator it(this);
    while(it.current()) 
    {
	Q3ListViewItem *item = it.current();
	item->setOpen(true);
	++it;
    }
}

void QgsLegend::collapseAll()
{
   Q3ListViewItemIterator it(this);
    while(it.current()) 
    {
	Q3ListViewItem *item = it.current();
	item->setOpen(false);
	++it;
    } 
}

bool QgsLegend::writeXML( QDomNode & layer_node, QDomDocument & document )
{
    QDomElement legendnode = document.createElement("legend");
    layer_node.appendChild(legendnode);

    QDomElement tmplegendnode = legendnode; /*copy of the legendnode*/
    QDomElement legendgroupnode;
    QDomElement legendlayernode;
    QDomElement layerfilegroupnode;
    QDomElement legendsymbolnode;
    QDomElement legendpropertynode;
    QDomElement legendlayerfilenode;
    QgsLegendLayerFile* llf;

    Q3ListViewItemIterator it(this);
    while(it.current()) 
    {
	QgsLegendItem *item = dynamic_cast<QgsLegendItem*>(it.current());
	if(item)
	{
	    switch(item->type())
	      {
	        case QgsLegendItem::LEGEND_GROUP:
		//make sure the legendnode is 'legend' again after a legend group
		if(!(item->parent()))
		    {
			legendnode = tmplegendnode;
		    }
		    legendgroupnode = document.createElement("legendgroup");
		    if(item->isOpen())
		    {
			legendgroupnode.setAttribute("open","true");
		    }
		    else
		    {
			legendgroupnode.setAttribute("open","false");
		    }
		    legendgroupnode.setAttribute("name",item->text(0));
		    legendnode.appendChild(legendgroupnode);
		    tmplegendnode =  legendnode;
		    legendnode = legendgroupnode;
		    break;

		case QgsLegendItem::LEGEND_LAYER:
		    //make sure the legendnode is 'legend' again after a legend group
		    if(!(item->parent()))
		    {
			legendnode = tmplegendnode;
		    }
		    legendlayernode = document.createElement("legendlayer");
		    if(item->isOpen())
		    {
			legendlayernode.setAttribute("open","true");
		    }
		    else
		    {
			legendlayernode.setAttribute("open","false");
		    }
		    legendlayernode.setAttribute("name", item->text(0));
		    legendnode.appendChild(legendlayernode);
		    break;

		case QgsLegendItem::LEGEND_PROPERTY_GROUP:
		    legendpropertynode = document.createElement("propertygroup");
		    if(item->isOpen())
		    {
			legendpropertynode.setAttribute("open","true");	
		    }
		    else
		    {
			legendpropertynode.setAttribute("open","false");
		    }
		    legendlayernode.appendChild(legendpropertynode);
		    break;

		case QgsLegendItem::LEGEND_SYMBOL_GROUP:
		    legendsymbolnode = document.createElement("symbolgroup");
		    if(item->isOpen())
		    {
			legendsymbolnode.setAttribute("open", "true");
		    }
		    else
		    {
			legendsymbolnode.setAttribute("open", "false");
		    }
		    legendlayernode.appendChild(legendsymbolnode);
		    break;
		
  
		case QgsLegendItem::LEGEND_LAYER_FILE_GROUP:
		    layerfilegroupnode = document.createElement("filegroup");
		    if(item->isOpen())
		    {
			layerfilegroupnode.setAttribute("open", "true");
		    }
		    else
		    {
			layerfilegroupnode.setAttribute("open", "false");
		    }
		    legendlayernode.appendChild(layerfilegroupnode);
		    break;
	    
	      case QgsLegendItem::LEGEND_LAYER_FILE:
		legendlayerfilenode = document.createElement("legendlayerfile");
		llf = dynamic_cast<QgsLegendLayerFile*>(item);
		if(llf)
		  {
		    legendlayerfilenode.setAttribute("layerid", llf->layer()->getLayerID());
		    layerfilegroupnode.appendChild(legendlayerfilenode);
		  }
		break;

		default: //do nothing for the leaf nodes
		    break;
	    }
	}
	++it;
    }
    return true;
}

bool QgsLegend::readXML(QDomNode& legendnode)
{
    QString open;
    Q3ListViewItemIterator it(this);
    Q3ListViewItem* theItem = firstChild(); //first level hierarchy items
    Q3ListViewItem* prevchild = 0; //store last value of theItem because of legend group
    Q3ListViewItem* secondLevelItem = 0; //second level item
    QgsLegendGroup* group = 0; //pointer to the last inserted legend group

    QDomNode child = legendnode.firstChild();
    if(!child.isNull())
    {
	do //iterate over legend layers/ legend groups
	{
	  if(!theItem)
	    {
	      break;
	    }
	    
	    QDomElement legendlayerelem = child.toElement();
	    
	    if(legendlayerelem.tagName()=="legendgroup")
	    {
		group = new QgsLegendGroup(this, legendlayerelem.attribute("name")); 
		group->setRenameEnabled(0, true);
		open = legendlayerelem.attribute("open");
		if(open == "true")
		{
		    group->setOpen(true);
		}
		if(prevchild)
		  {
		    group->moveItem(prevchild);
		  }
		theItem = group->nextSibling();
		if(!child.firstChild().isNull())
		  {
		    child = child.firstChild();//go one hierarchy step down
		  }
		else
		  {
		    child = child.nextSibling();
		  }
		continue;
	    }

	    open = legendlayerelem.attribute("open");
	    if(child.parentNode().toElement().tagName()=="legendgroup")
	    {
	      group->insertItem(theItem);
	      Q3ListViewItem* currentChild = group->firstChild();
	      if(!currentChild)
		{
		  group->insertItem(theItem);//insert the first child in the group
		}
	      else //find the last child and insert the new one after it
		{
		  while(currentChild->nextSibling() != 0)
		    {
		      currentChild = currentChild->nextSibling();
		    }
		  group->insertItem(theItem);
		  theItem->moveItem(currentChild);
		}
	    }
	    if(open == "true")
	    {
		theItem->setOpen(true);
	    }
	    theItem->setText(0, legendlayerelem.attribute("name"));

	    //file group
	    secondLevelItem = theItem->firstChild();
	    QDomNode filegroupnode = child.firstChild();
	    QDomElement filegroupelem = filegroupnode.toElement();
	    open = filegroupelem.attribute("open");
	    if(open == "true")
	    {
		secondLevelItem->setOpen(true);
	    }
	    QDomNode layerfilenode = filegroupnode.firstChild();
	    QDomElement layerfileelem = layerfilenode.toElement();
     
	    //remove the existing legendlayerfile and insert the one(s) according to the entries in the XML file
	    if(secondLevelItem->firstChild())
	      {
		QgsLegendLayerFile* llfdelete = dynamic_cast<QgsLegendLayerFile*>(secondLevelItem->firstChild());
		if(llfdelete)
		  {
		    unregisterCheckBox(llfdelete);
		    delete llfdelete;
		    placeCheckBoxes();
		  }
	      }
	    
	    //if there are several legend layer files in this group, create the additional items
	    std::map<QString,QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
	    while(true)
	      {
		if(layerfilenode.isNull())
		  {
		    break;
		  }
		
		std::map<QString,QgsMapLayer*>::iterator it = layers.find(layerfileelem.attribute("layerid"));
		if(it != layers.end())
		  {
		    QgsMapLayer* newlayer = it->second;
		    QgsLegendLayerFile* newfile = new QgsLegendLayerFile(secondLevelItem, QgsLegendLayerFile::nameFromLayer(newlayer), newlayer); 
		    newlayer->setLegendLayerFile(newfile);
		    newlayer->initContextMenu(mApp);

		    //move newfile as the last child of the legendlayerfilegroup
		    Q3ListViewItem* curitem = newfile;
		    while(curitem = newfile->nextSibling())
		      {
			newfile->moveItem(curitem);
		      }
		  }
		layerfilenode = layerfilenode.nextSibling();
		layerfileelem = layerfilenode.toElement();
	      }

	    //symbology group
	    secondLevelItem = secondLevelItem->nextSibling();
	    QDomNode symbologygroupnode = filegroupnode.nextSibling();
	    QDomElement symbologygroupelem = symbologygroupnode.toElement();
	    open = symbologygroupelem.attribute("open");
	    if(open == "true")
	    {
		secondLevelItem->setOpen(true);
	    }

	    //property group
	    secondLevelItem = secondLevelItem->nextSibling();
	    QDomNode propertygroupnode = symbologygroupnode.nextSibling();
	    QDomElement propertygroupelem = propertygroupnode.toElement();
	    open = propertygroupelem.attribute("open");
	    {
		secondLevelItem->setOpen(true);
	    }

	    if(child.nextSibling().isNull() && !child.parentNode().isNull()) //go one hierarchy step up
	      {
		child = child.parentNode();
	      }
	    if(theItem->nextSibling() == 0)
	      {
		theItem = theItem->parent();
	      }

	    if(!theItem)
	      {
		break;
	      }

	    prevchild = theItem;
	    theItem = theItem->nextSibling();
	    child = child.nextSibling();
	    if(!theItem)
	      {
		break; //reached the end
	      }
	}
	while(!(child.isNull()));
    }
    placeCheckBoxes();
    return true;
}

void QgsLegend::saveToProject()
{
  int toplayeridx=0;
  int lgroupidx=0;
  int llayeridx=0;
  QString groupstring; //string which have to be prepended if an element is into a group

  Q3ListViewItemIterator it(this);
  while(it.current()) 
    {
      QgsLegendItem *item = dynamic_cast<QgsLegendItem*>(it.current());
      if(item)
	{
	  switch(item->type())
	    {
	    case QgsLegendItem::LEGEND_GROUP:
	      ++lgroupidx;
	      groupstring = "/LegendGroup"+QString::number(lgroupidx);
	      QgsProject::instance()->writeEntry("Legend","/LegendGroup"+QString::number(lgroupidx)+"/Name",item->text(0));
	      ++toplayeridx;
	      QgsProject::instance()->writeEntry("Legend","/LegendGroup"+QString::number(lgroupidx)+"/TopLayerIndex",toplayeridx);
	      if(item->isOpen())
		{
		  QgsProject::instance()->writeEntry("Legend","/LegendGroup"+QString::number(lgroupidx)+"/Open",true);	
		}
	      else
		{
		  QgsProject::instance()->writeEntry("Legend","/LegendGroup"+QString::number(lgroupidx)+"/Open",true);
		}
	      break;
		    
	    case QgsLegendItem::LEGEND_LAYER:
	      ++llayeridx;
	      if(item->parent()==0)//legend layer is not in a group
		{
		  groupstring="";
		  ++toplayeridx;
		 QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/TopLayerIndex",toplayeridx); 
		}
	      else //set TopLayerIndex -1 if a LegendLayer is not a toplevel item
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/TopLayerIndex",-1);
		}
	      QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/Name",item->text(0));
	      if(item->isOpen())
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/Open",true);
		}
	      else
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/Open",false);
		}
	      break;
	      
	    case QgsLegendItem::LEGEND_PROPERTY_GROUP:
	      if(item->isOpen())
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/LegendPropertyGroup/Open", true);
		}
	      else
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/LegendPropertyGroup/Open", false);
		}
	      break;
	      
	    case QgsLegendItem::LEGEND_SYMBOL_GROUP:
	      if(item->isOpen())
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/LegendSymbolGroup/Open", true);
		}
	      else
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/LegendSymbolGroup/Open", false);
		}
	      break;
	      
	    case QgsLegendItem::LEGEND_LAYER_FILE_GROUP:
	      if(item->isOpen())
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/LegendLayerFileGroup/Open", true);
		}
	      else
		{
		  QgsProject::instance()->writeEntry("Legend",groupstring+"/LegendLayer"+QString::number(llayeridx)+"/LegendLayerFileGroup/Open", false);
		}
	      break;
	      
	    default: //do nothing for the leaf nodes
	      break;	
	    }
	}
      ++it;
    }
}

void QgsLegend::restoreFromProject()
{
  int legendlayercount = 0;
  int legendgroupcount = 0;
  QStringList legendlayerlist;
  QStringList legendgrouplist;
  bool ok;

  legendlayerlist = QgsProject::instance()->readListEntry("Legend","/LegendLayer"+QString::number(legendlayercount+1), &ok);
  if(ok)
    {
      ++legendlayercount;
      for(QStringList::Iterator it = legendlayerlist.begin(); it != legendlayerlist.end(); ++it)
	{
	  qWarning(*it);
	}
    }

  legendgrouplist = QgsProject::instance()->readListEntry("Legend","/LegendGroup"+QString::number(legendgroupcount+1), &ok);
  if(ok)
    {
      ++legendgroupcount;
      for(QStringList::Iterator it = legendgrouplist.begin(); it != legendgrouplist.end(); ++it)
	{
	  qWarning(*it);
	}
    }
}

void QgsLegend::storeInitialPosition(Q3ListViewItem* li)
{
  if(li == firstChild()) //the item is the first item in the list view
    {
      mRestoreInformation = FIRST_ITEM;
      mRestoreItem = 0;
    }
  else if(li->parent() == 0) //li is a toplevel item, but not the first one
    {
      mRestoreInformation = YOUNGER_SIBLING;
      mRestoreItem = ((QgsLegendItem*)(li))->findYoungerSibling();
    }
  else if(li == li->parent()->firstChild())//li is not a toplevel item, but the first child
    {
      mRestoreInformation = FIRST_CHILD;
      mRestoreItem = li->parent();
    }
  else
    {
      mRestoreInformation = YOUNGER_SIBLING;
      mRestoreItem = ((QgsLegendItem*)(li))->findYoungerSibling();
    }
}

void QgsLegend::resetToInitialPosition(Q3ListViewItem* li)
{
  if(mRestoreInformation == FIRST_ITEM)
    {
      insertItem(li);
    }
  else if(mRestoreInformation == FIRST_CHILD)
    {
      mRestoreItem->insertItem(li);
    }
  else if(mRestoreInformation == YOUNGER_SIBLING)
    {
      li->moveItem(mRestoreItem);
    }
}

bool QgsLegend::yCoordAboveCenter(QgsLegendItem* it, int ycoord)
{
  QRect rect = itemRect(it);
  int height = rect.height();
  int top = rect.top();
  int mid = top + (height / 2);
  if (ycoord < mid) //bottom
    {
      return false;
    }
  else//top
    {
      return true;
    }
}
