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


#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgslegend.h"


static const char* const ident_ 
 = "$Id$";


/**
   @note

   set movingItem pointer to 0 to prevent SuSE 9.0 crash
*/
QgsLegend::QgsLegend(QWidget * parent, const char *name)
    : QListView(parent, name), mousePressed(false), movingItem(0)
{}

QgsLegend::~QgsLegend()
{}


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
  {
    return li->text(0);
  }
  else
  {
      return 0;
  }
}




/**
   Find the QgsLegendItem that corresponds to a map layer with the given name.

   @return pointer to QgsLegendItem with that name, otherwise null.
*/
static
QgsLegendItem *
findLegendItem_( QgsLegend * legend, QString const & name )
{
  QListViewItemIterator it(legend);

  while (it.current())
  {
      QgsLegendItem * li = dynamic_cast<QgsLegendItem*>(it.current());

      Q_CHECK_PTR( li );

      if ( li )
      {
          std::cerr << "comparing " << li->layerID() 
                    << " and " << name << "\n"; 
          if ( li->layerID() == name )
          {
              return li;
          }
      }

      ++it;
  }

  return 0x0;
    
} // findLegendItem_( QString const & name )



#ifdef DEPRECATED
void QgsLegend::update()
{
    // if there are already items in the legend, then we need to be careful to
    // only add new items, and to keep the items that are selected selected
    if ( childCount() )
    { // we have legend items, so add ONLY NEW ONES leaving original items
      // alone
                                // current map layer
        QgsMapLayer * currentMapLayer = 0x0;

        // grind through all the layers in "Z" order (i.e., front to back)
        for ( int currentLayer = 0;
              currentLayer < map->zOrders().size();
              ++currentLayer )
        {
            currentMapLayer = map->getZpos( currentLayer );

            // if the map layer isn't in the legend, add it
            if ( ! findLegendItem_( this, currentMapLayer->name() ) )
            {
                std::cerr << __FILE__ << ":" << __LINE__
                          << " didn't find " <<  currentMapLayer->name() << "'s legend item ... adding new item\n";

                QgsLegendItem *lvi = 
                    new QgsLegendItem(currentMapLayer, this);
                currentMapLayer->setLegendItem(lvi);
                lvi->setPixmap(0, *currentMapLayer->legendPixmap());
            }
            else
            {
                std::cerr << __FILE__ << ":" << __LINE__
                          << " found " << currentMapLayer->name() << "'s legend item ... skipping\n";
            }
            ++currentLayer;
        }
    }
    else
    { // no layers loaded, so just load 'em all up

        std::list < QString >::iterator zi = map->zOrders().begin();
        while (zi != map->zOrders().end())
        {
            QgsMapLayer *lyr = map->layerByName(*zi);
            QgsLegendItem *lvi = new QgsLegendItem(lyr, this);  // lyr->name(), QCheckListItem::CheckBox );
            lyr->setLegendItem(lvi);
            lvi->setPixmap(0, *lyr->legendPixmap());
            zi++;
        }

        // highlight the top item

        setCurrentItem( firstChild() );
        setSelected( firstChild(), true );
    }

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


} // QgsLegend::update()
#endif // DEPRECATED



/* slot */
void QgsLegend::addLayer( QgsMapLayer * layer )
{
    // XXX check for duplicates first?

    Q_CHECK_PTR( layer );

    if ( ! layer )
    { return; }

    QgsLegendItem * legend_item = new QgsLegendItem( layer, this );

    // done in QgsLegendItem ctor legend_item->setPixmap( 0, *layer->legendPixmap() );

    // XXX we could probably make map layers ignorant of corresponding legend
    // XXX item through use of signals/slots
    layer->setLegendItem( legend_item );

    // if the newly added legend item is the first one, then go ahead and
    // highlight it indicating it is selected
    if ( 1 == childCount() )
    {
        setCurrentItem( firstChild() );
        setSelected( firstChild(), true ); // shouldn't have to do this,
                                // but for some reason making it current
                                // item doesn't select it
    }
    
} // QgsLegend::addLayer



/* slot */
void QgsLegend::removeLayer( QString layer_key )
{
    // There are three possible starting legend states when this is invoked.
    //
    // 1. there is currently only one layer
    // 2. there is currently more than one layer, and this layer isn't selected
    // 3. there is currently more than one layer, and this layer is selected

    // For #1, we just remove the layer.  For #2, we also just remove the
    // layer.  #3 is more complicated in that if it's the _only_ selected
    // layer, we should reset what is the current layer and toggle on its
    // selection; if there're more than one selected layer, then just remove
    // this layer.  We arbitrarily make firstChild() the new selected layer,
    // in the case of new selected layers after deleting this one.  (Because
    // it's easy.)

                                // first find the layer
    
    QListViewItemIterator it(this);

    for ( ; it.current(); ++it )
    {
        QgsLegendItem *li = (QgsLegendItem *) it.current();
        
        if ( li->layerID() == layer_key )
        { break; }
    }

    if ( it.current() )         // will be non-NULL if found layer
    {
        bool makeNewCurrentItem(false);

        // if the current item is about to be deleted, make a note of it so
        // that we can select a new one later
        if ( it.current() == currentItem() )
        {
            makeNewCurrentItem = true;
        }

        delete it.current();    // remove from list

        map->remove( layer_key ); // tell canvas map to remove the layer; note
                                  // that a revese "removeLayer" signal will
                                  // call this function -- which will be fine
                                  // because the second invocation won't find
                                  // the layer and just exit

                                // if we find a selected layer, do nothing; if
                                // we don't then arbitrarily set firstChild()
                                // to be the current layer

        // not Qt 3.1.2 compatible QListViewItemIterator select_it(this, QListViewItemIterator::Selected);
        QListViewItemIterator select_it(this);

        for ( ; select_it.current(); ++select_it )
        {
            if ( it.current()->isSelected() )
            {
                // if we even find ONE SELECTED ITEM, we're done since there's
                // still at least one other layer that's selected

                // Wellll, ok, we'll have to possibly set the current item
                if ( makeNewCurrentItem )
                {
                    setCurrentItem( select_it.current() );
                    emit currentChanged( firstChild() );
                }

                break;
            }
        }

        if ( ! select_it.current() )
        {
            // if there're NO CURRENTLY SELECTED ITEMS, arbitrarily make
            // firstChild() the current selected item

            setCurrentItem( firstChild() );
            emit currentChanged( firstChild() );
        }

        emit layerRemoved( layer_key );
    }
    else
    {
        // NOP -- this might actually be ok since it's possible for this
        // function to be invoked twice because map canvas signal
        // "removedLayer" will call this slot; but the second go round the
        // layer will already be gone, so it'll just return.  

        // XXX this still seems kludgy to me; but it's better than what was
        // XXX here before; we could probably correct by working out better
        // XXX signal/slot system network
    }

} // removeLayer



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

} // contentsMousePressEvent



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
       // scroll view if we're near the edge
       QPoint p(contentsToViewport(e->pos()));
       if (p.y() < 15)
         {
           scrollBy(0, -(15 - p.y()));
       } else if (p.y() > visibleHeight() - 15)
		  {
		    scrollBy(0, p.y() - visibleHeight() - 15)
		  }
      
		
      // move item in list if we're dragging over another item
      QListViewItem *item = itemAt(p);
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
