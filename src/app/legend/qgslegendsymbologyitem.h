/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   aps02ts@macbuntu                                                      *
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
