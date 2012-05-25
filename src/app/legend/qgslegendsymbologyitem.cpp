/***************************************************************************
    qgslegendsymbologyitem.cpp
    ---------------------
    begin                : January 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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

void QgsLegendSymbologyItem::setLegend( QgsLegend* theLegend )
{
  mLegend = theLegend;
  if ( mLegend )
  {
    mLegend->addPixmapWidthValue( mPixmapWidth );
    mLegend->addPixmapHeightValue( mPixmapHeight );
  }
}

