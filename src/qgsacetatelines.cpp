/***************************************************************************
                              qgsacetatelines.cpp    
                   A collection of lines that can be drawn on 
                      the acetate layer of a QgsMapCanvas
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
#include "qgsline.h"
#include "qgsacetatelines.h"

QgsAcetateLines::QgsAcetateLines()
{
  mLineCollection = new std::vector<QgsLine>;
}

QgsAcetateLines::~QgsAcetateLines()
{
  delete mLineCollection;
}

void QgsAcetateLines::add(QgsLine &line)
{
  mLineCollection->push_back(line);
}
void QgsAcetateLines::draw(QPainter *painter, QgsMapToPixel *cXf)
{
  painter->setPen(QColor(255,0,0));
  painter->setBrush(Qt::NoBrush);
  // iterate through the lines in the vector and draw each
  for(int i=0; i < mLineCollection->size(); i++)
  {
    QgsLine line = mLineCollection->at(i);  
    // get the begin and end points of the line
    QgsPoint begin = line.begin();
    QgsPoint end = line.end();
    if(cXf)
    {
      // transform the points before drawing
      cXf->transform(&begin);
      cXf->transform(&end);
    }
    painter->moveTo(static_cast<int>(begin.x()), static_cast<int>(begin.y()));
    painter->lineTo(static_cast<int>(end.x()), static_cast<int>(end.y()));
  }
}
