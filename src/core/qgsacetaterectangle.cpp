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
#include <qstring.h>
#include <qpainter.h>
#include <qgsmaptopixel.h>
#include <qgspoint.h>
#include <qgsrect.h>
#include "qgsacetaterectangle.h"

  QgsAcetateRectangle::QgsAcetateRectangle(QgsPoint &origin, const QgsRect &rect) :
QgsAcetateObject(origin), mRectangle(rect)
{
}

QgsAcetateRectangle::~QgsAcetateRectangle()
{
}


void QgsAcetateRectangle::draw(QPainter *painter, QgsMapToPixel *cXf)
{
  painter->setPen(QColor(255,0,0));
  painter->setBrush(Qt::NoBrush);
  // get the lower left (ll) and upper right (ur) points of the rectangle
  QgsPoint ll(mRectangle.xMin(), mRectangle.yMin());
  QgsPoint ur(mRectangle.xMax(), mRectangle.yMax());
  if(cXf)
  {
    // transform the points before drawing
    cXf->transform(&ll);
    cXf->transform(&ur);
  }
  painter->drawRect(static_cast<int>(ll.x()), static_cast<int>(ll.y()), 
                    static_cast<int>(ur.x()) - static_cast<int>(ll.x()), 
                    static_cast<int>(ur.y()) - static_cast<int>(ll.y()));

}
