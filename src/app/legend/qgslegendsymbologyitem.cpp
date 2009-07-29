/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton   *
 *   aps02ts@macbuntu   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "qgslegendsymbologyitem.h"

QgsLegendSymbologyItem::QgsLegendSymbologyItem( QTreeWidgetItem * theItem, QString theString, int pixmapWidth, int pixmapHeight )
    : QgsLegendItem( theItem, theString ),
    mPixmapWidth( pixmapWidth ),
    mPixmapHeight( pixmapHeight ),
    mLegend( 0 )
{
  mType = LEGEND_SYMBOL_ITEM;
  setFlags( Qt::ItemIsEnabled );
}

QgsLegendSymbologyItem::QgsLegendSymbologyItem( int pixmapWidth, int pixmapHeight )
    : QgsLegendItem(),
    mPixmapWidth( pixmapWidth ),
    mPixmapHeight( pixmapHeight ),
    mLegend( 0 )
{
  mType = LEGEND_SYMBOL_ITEM;
  setFlags( Qt::ItemIsEnabled );
}

QgsLegendSymbologyItem::~QgsLegendSymbologyItem()
{
  if ( mLegend )
  {
    mLegend->removePixmapWidthValue( mPixmapWidth );
    mLegend->removePixmapHeightValue( mPixmapHeight );
  }
}

QgsLegendItem::DRAG_ACTION QgsLegendSymbologyItem::accept( LEGEND_ITEM_TYPE type )
{
  return NO_ACTION;
}

QgsLegendItem::DRAG_ACTION QgsLegendSymbologyItem::accept( const QgsLegendItem* li ) const
{
  return NO_ACTION;
}

void QgsLegendSymbologyItem::setLegend( QgsLegend* theLegend )
{
  mLegend = theLegend;
  if ( mLegend )
  {
    mLegend->addPixmapWidthValue( mPixmapWidth );
    mLegend->addPixmapHeightValue( mPixmapHeight );
  }
}

