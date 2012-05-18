/***************************************************************************
    qgslegendsymbologyitem.h
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
#ifndef QGSLEGENDSYMBOLOGYITEM_H
#define QGSLEGENDSYMBOLOGYITEM_H

#include "qgslegend.h"
#include <qgslegenditem.h>

/**
@author Tim Sutton
*/
class QgsLegendSymbologyItem : public QgsLegendItem
{
  public:
    QgsLegendSymbologyItem( QTreeWidgetItem* theItem, QString theString, int pixmapWidth, int pixmapHeight );
    QgsLegendSymbologyItem( int pixmapWidth, int pixmapHeight );
    ~QgsLegendSymbologyItem();
    bool isLeafNode() {return true;}
    int pixmapWidth() const {return mPixmapWidth;}
    int pixmapHeight() const {return mPixmapHeight;}
    void setLegend( QgsLegend* theLegend );
    QgsLegend* legend() const {return mLegend;}
  protected:
    int mPixmapWidth;
    int mPixmapHeight;
    /**This pointer is needed to remove the width/height values of this item
     in the legend even if this item is not in the legend anymore*/
    QgsLegend* mLegend;
};

#endif
