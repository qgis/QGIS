/***************************************************************************
                          qgsmapcanvas.cpp  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qstring.h>
#include <qpainter.h>
#include <qrect.h>
#include "qgsrect.h"
#include "qgsmaplayer.h"
#include "qgsdatabaselayer.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"

QgsMapCanvas::QgsMapCanvas(QWidget *parent, const char *name ) : QWidget(parent,name) {
  mapWindow = new QRect();
  coordXForm = new QgsCoordinateTransform();
}
QgsMapCanvas::~QgsMapCanvas(){
  delete coordXForm;
  delete mapWindow;
}
void QgsMapCanvas::addLayer(QgsMapLayer *lyr){
  layers[lyr->name()] = lyr;
  // update extent if warranted
  if(layers.size() == 1){
    fullExtent = lyr->extent();
    currentExtent = fullExtent;
  }

  // set zpos to something...
  //lyr->zpos = 0;
}
void QgsMapCanvas::render2(){
 QPainter *paint = new QPainter();
  paint->begin(this);
  //currentExtent = fullExtent;
  QRect v = rect();// paint->viewport();
  // calculate the translation and scaling parameters
  double muppX, muppY;
      muppY = currentExtent.height()/height();
      muppX = currentExtent.width()/width();
      cout << "MuppX is: " << muppX << "\nMuppY is: " << muppY << endl;
      m_mupp = muppY > muppX?muppY:muppX;
      m_mupp *= 1.20;
  coordXForm->setParameters(m_mupp, currentExtent.xMin(), 
			    currentExtent.yMin(), currentExtent.yMax());
  // render all layers in the stack, starting at the base
  map<QString,QgsMapLayer *>::iterator mi = layers.begin();
  while(mi != layers.end()){
    QgsMapLayer *ml = (*mi).second;
    //    QgsDatabaseLayer *dbl = (QgsDatabaseLayer *)&ml;
    ml->draw(paint, &currentExtent, coordXForm);
    mi++;
    //  mi.draw(p, &fullExtent);
  }
  paint->end();

}
void QgsMapCanvas::render(){
  QPainter *paint = new QPainter();
  paint->begin(this);
  currentExtent = fullExtent;
  mapWindow->setLeft(currentExtent.xMin());
  mapWindow->setBottom(currentExtent.yMin());
  
    // determine the dominate direction for the mapcanvas
      if (width () > height ())
	{
	  mapWindow->setWidth(currentExtent.width());
	  mapWindow->setHeight(currentExtent.width());
	}
      else
	{
	  mapWindow->setWidth(currentExtent.height());
	  mapWindow->setHeight(currentExtent.height());
	}
     
      paint->setWindow(*mapWindow);
     
 QRect v = paint->viewport ();
      int d = QMIN (v.width (), v.height ());
      int dm = QMAX(v.width(), v.height());
           paint->setViewport (v.left () + (v.width () - d) / 2,
       		 v.top () + (v.height () - d) / 2, d, d);
      
  // render all layers in the stack, starting at the base
  map<QString,QgsMapLayer *>::iterator mi = layers.begin();
  int yTransform =  currentExtent.yMax();//mapWindow->bottom() -  abs(mapWindow->height() - currentExtent.height())/2;
  while(mi != layers.end()){
    QgsMapLayer *ml = (*mi).second;
    //    QgsDatabaseLayer *dbl = (QgsDatabaseLayer *)&ml;
    ml->draw(paint, &currentExtent, yTransform);
    mi++;
    //  mi.draw(p, &fullExtent);
  }
  paint->end();
}
void QgsMapCanvas::paintEvent(QPaintEvent *pe){
  render2();
}
QgsRect QgsMapCanvas::extent(){
  return currentExtent;
}
void QgsMapCanvas::setExtent(QgsRect r){
  currentExtent = r;
}
void QgsMapCanvas::clear(){
  QPainter *p = new QPainter();
  p->begin(this);
  p->eraseRect(this->rect());
  p->end();
  
}
void QgsMapCanvas::zoomFullExtent(){
  currentExtent = fullExtent;
  clear();
  render2();
}
