/***************************************************************************
  qgsacetateobject.cpp    
  An object that can be drawn on the acetate layer of a QgsMapCanvas
            -------------------
  begin                : June 10, 2004
  copyright            : (C) 2004 by Gary E.Sherman
  email                : sherman at mrcc.com
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
#include <qgspoint.h>
#include <qpainter.h>
#include <qgscoordinatetransform.h>
#include "qgsacetateobject.h"

QgsAcetateObject::QgsAcetateObject(QgsPoint &origin) :
	mOrigin(origin)
{
}

QgsAcetateObject::~QgsAcetateObject()
{
}
/*void QgsAcetateObject::draw(QPainter * painter, QgsCoordinateTransform * cXf)
{
}
*/
QgsPoint QgsAcetateObject::origin ( ) {
 return mOrigin;
}

void QgsAcetateObject::setOrigin (QgsPoint value ) {
				mOrigin = value;
}

