/***************************************************************************
                          qgsmarkersymbol.cpp  -  description
                             -------------------
    begin                : March 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#include "qgsmarkersymbol.h"
#include "qpainter.h"

void QgsMarkerSymbol::render(int x, int y, QPainter* p)
{
    if(p)
    {
	p->scale(mScaleFactor,mScaleFactor);
	p->drawPicture(x*mScaleFactor,y*mScaleFactor,mPicture);//scale the picture but keep the coordinates constant.todo: write the code such that x,y is in the midpoint of the image
	p->resetXForm();
    }
}
    

