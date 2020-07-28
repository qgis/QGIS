/***************************************************************************
    qgsannotationitem.cpp
    ------------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationitem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"


QgsAnnotationItem::QgsAnnotationItem( const QgsCoordinateReferenceSystem &crs )
  : mCrs( crs )
{

}

void QgsAnnotationItem::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}
