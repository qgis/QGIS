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

void QgsMarkerSymbol::setPicture(const QString& svgpath)
{
    mPicture.load(svgpath,"svg");
}
    

