/***************************************************************************
                          qgslegend.cpp  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
               Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <map>
#include <qstring.h>
#include <qpainter.h>
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgslegend.h"

QgsLegend::QgsLegend (QWidget * parent, const char *name):QScrollView (parent,
	     name)
{
//legendContainer = new QVBox(viewport());
// addChild(legendContainer);

}

QgsLegend::~QgsLegend ()
{
}

void QgsLegend::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph){
   // Calculate the coordinates...
    int x1 = 0;
    int y1 = 0;
    int x2 = width();
    int y2 = height();//map->layerCount() * 35;

    // Clip the coordinates so X/Windows will not have problems...
    if (x1 < clipx) x1=clipx;
    if (y1 < clipy) y1=clipy;
    if (x2 > clipx+clipw-1) x2=clipx+clipw-1;
    if (y2 > clipy+cliph-1) y2=clipy+cliph-1;

    // Paint using the small coordinates...
    if ( x2 >= x1 && y2 >= y1 )
        p->fillRect(x1, y1, x2-x1+1, y2-y1+1, red);
  
  
}
void QgsLegend::setMapCanvas(QgsMapCanvas *canvas){
	map = canvas;
	}
	
void QgsLegend::update(){
resizeContents(width(),height());
QPainter *p = new QPainter(this);
drawContents(p, 0,0,width(),height());
enableClipper(true);

// clear the legend


// Get the list of layers in order from the
// map canvas and add legenditems to the legend

for(int idx=0; idx < map->layerCount(); idx++){
	QgsMapLayer *lyr = map->getZpos(idx);
	QgsLegendItem *li = new QgsLegendItem(lyr, this);
	addChild(li,0,idx*60);
	repaint();
}
}

