/***************************************************************************
                         qgslegendlayerfilegroup.cpp  -  description
                             -------------------
    begin                : Juli 2005
    copyright            : (C) 2005 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegendlayer.h"
#include "qgslegendlayerfilegroup.h"
#include "qgslegendlayerfile.h"
#include "qgslegendsymbologygroup.h"
#include "qgsmaplayer.h"
#include "qgslogger.h"

QgsLegendLayerFileGroup::QgsLegendLayerFileGroup( QTreeWidgetItem* theItem, QString theString ): QgsLegendItem( theItem, theString )
{
  mType = LEGEND_LAYER_FILE_GROUP;
  setText( 0, theString );
}

QgsLegendItem::DRAG_ACTION QgsLegendLayerFileGroup::accept( LEGEND_ITEM_TYPE type )
{
  if ( type == LEGEND_LAYER_FILE )
  {
    return INSERT; //there should be a way to already test, if the layers are symbology compatible
  }
  else
  {
    return NO_ACTION;
  }
}

QgsLegendItem::DRAG_ACTION QgsLegendLayerFileGroup::accept( const QgsLegendItem* li ) const
{
  QgsDebugMsg( "entered." );
  if ( li )
  {
    LEGEND_ITEM_TYPE type = li->type();
    if ( type == LEGEND_LAYER_FILE /*&& this != li->parent()*/ )
    {
      if ( child( 0 ) == 0 )
      {
        return INSERT;
      }
      else
      {
        QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile *>( child( 0 ) );
        if ( llf )
        {
          QgsMapLayer* childlayer = llf->layer();
          const QgsMapLayer* newlayer = qobject_cast<const QgsLegendLayerFile *>( li )->layer();
          if ( newlayer->hasCompatibleSymbology( *childlayer ) )
          {
            return INSERT;
          }
        }
      }
    }
  }
  return NO_ACTION;
}

bool QgsLegendLayerFileGroup::insert( QgsLegendItem* newItem )
{
  if ( newItem->type() == LEGEND_LAYER_FILE )
  {
    QgsLegendItem* oldItem = firstChild();

    if ( !oldItem )//this item is the first child
    {
      insertChild( 0, newItem );
      return true;
    }
    //there are already legend layer files

    //find the lowest sibling
    while ( oldItem->nextSibling() != 0 )
    {
      oldItem = oldItem->nextSibling();
    }
    QgsLegendLayerFile* thefile = qobject_cast<QgsLegendLayerFile *>( oldItem );

    if ( !thefile )
    {
      return false;
    }
    QgsMapLayer* thelayer = thefile->layer();
    if ( !thelayer )
    {
      return false;
    }
    QgsMapLayer* newLayer = qobject_cast<QgsLegendLayerFile *>( newItem )->layer();
    if ( newLayer->hasCompatibleSymbology( *thelayer ) )
    {
      insertChild( childCount(), newItem );
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool QgsLegendLayerFileGroup::containsLegendLayerFile( const QgsLegendLayerFile* llf ) const
{
  bool result = false;
  for ( int i = 0; i < childCount(); ++i )
  {
    if ( llf == child( i ) )
    {
      result = true;
      break;
    }
  }
  return result;
}

void QgsLegendLayerFileGroup::receive( QgsLegendItem* newChild )
{
  if ( newChild->type() == LEGEND_LAYER_FILE )
  {
    QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer *>( parent() );
    if ( ll )
    {
      ll->updateIcon();
      ll->updateCheckState();
    }
  }
}

void QgsLegendLayerFileGroup::release( QgsLegendItem* formerChild )
{
  QgsDebugMsg( "entered." );
  if ( formerChild->type() == LEGEND_LAYER_FILE )
  {
    QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer *>( parent() );
    if ( ll )
    {
      ll->updateIcon();
      ll->updateCheckState();
    }
  }
}
