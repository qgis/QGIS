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
#include <iostream>
#include <cmath>
#include <qstring.h>
#include <qpainter.h>
#include <qrect.h>
#include <qevent.h>
#include <qpixmap.h>
#include "qgsrect.h"
#include "qgis.h"

#include "qgsmaplayer.h"
#include "qgslegend.h"
#include "qgsdatabaselayer.h"
#include "qgscoordinatetransform.h"
#include "qgsmarkersymbol.h"
#include "qgspolygonsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmapcanvas.h"

QgsMapCanvas::QgsMapCanvas(QWidget * parent, const char *name):QWidget(parent, name)
{
	mapWindow = new QRect();
	coordXForm = new QgsCoordinateTransform();
	bgColor = QColor(Qt::white);
	setMouseTracking(true);
	drawing = false;
}

QgsMapCanvas::~QgsMapCanvas()
{
	delete coordXForm;
	delete mapWindow;
}

void QgsMapCanvas::addLayer(QgsMapLayer * lyr)
{
// give the layer a default symbol
	QgsSymbol *sym;
	QColor *fill;
	int red, green, blue;
	switch (lyr->featureType()) {
	  case QGis::WKBPoint:
	  case QGis::WKBMultiPoint:
		  sym = new QgsMarkerSymbol();
		  break;
	  case QGis::WKBLineString:
	  case QGis::WKBMultiLineString:
		  sym = new QgsLineSymbol();
		  break;
	  case QGis::WKBPolygon:
	  case QGis::WKBMultiPolygon:
		  sym = new QgsPolygonSymbol();
		  red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
		  green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
		  blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
		  fill = new QColor(red, green, blue);
		  sym->setFillColor(*fill);
		  break;

	}
	red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));

	sym->setColor(QColor(red, green, blue));
	sym->setLineWidth(1);
	lyr->setSymbol(sym);
	layers[lyr->name()] = lyr;
	// update extent if warranted
	if (layers.size() == 1) {
		fullExtent = lyr->extent();
		fullExtent.scale(1.1);
		currentExtent = fullExtent;

	}

	updateFullExtent(lyr->extent());
	// increment zpos for all layers in the map
	incrementZpos();
	lyr->setZ(layers.size() - 1);
	updateZpos();
	zOrder.push_back(lyr->name());
	connect(lyr, SIGNAL(visibilityChanged()), this, SLOT(layerStateChange()));
	//lyr->zpos = 0;
}

void QgsMapCanvas::incrementZpos()
{
}
void QgsMapCanvas::updateZpos()
{
}
QgsMapLayer *QgsMapCanvas::getZpos(int)
{
//  QString name = zOrder[index];
//  return layers[name];
	return 0;
}

QgsMapLayer *QgsMapCanvas::layerByName(QString name)
{
	return layers[name];

}

void QgsMapCanvas::render2()
{
	QString msg = frozen ? "frozen" : "thawed";
//std::cout << "Map canvas is " << msg << std::endl;
	if (!frozen) {
		if (!drawing) {
//  std::cout << "IN RENDER 2" << std::endl;
			drawing = true;
			QPainter *paint = new QPainter();
			paint->begin(this);

			// calculate the translation and scaling parameters
			double muppX, muppY;
			muppY = currentExtent.height() / height();
			muppX = currentExtent.width() / width();
//      std::cout << "MuppX is: " << muppX << "\nMuppY is: " << muppY << std::endl;
			m_mupp = muppY > muppX ? muppY : muppX;
			// calculate the actual extent of the mapCanvas
			double dxmin, dxmax, dymin, dymax, whitespace;
			if (muppY > muppX) {
				dymin = currentExtent.yMin();
				dymax = currentExtent.yMax();
				whitespace = ((width() * m_mupp) - currentExtent.width()) / 2;
				dxmin = currentExtent.xMin() - whitespace;
				dxmax = currentExtent.xMax() + whitespace;
			} else {
				dxmin = currentExtent.xMin();
				dxmax = currentExtent.xMax();
				whitespace = ((height() * m_mupp) - currentExtent.height()) / 2;
				dymin = currentExtent.yMin() - whitespace;
				dymax = currentExtent.yMax() + whitespace;

			}
//      std::cout << "dxmin: " << dxmin << std::endl << "dymin: " << dymin << std::
//        endl << "dymax: " << dymax << std::endl << "whitespace: " << whitespace << std::endl;
			coordXForm->setParameters(m_mupp, dxmin, dymin, height());	//currentExtent.xMin(),      currentExtent.yMin(), currentExtent.yMax());
			// update the currentExtent to match the device coordinates
			currentExtent.setXmin(dxmin);
			currentExtent.setXmax(dxmax);
			currentExtent.setYmin(dymin);
			currentExtent.setYmax(dymax);
			// render all layers in the stack, starting at the base
			std::map < QString, QgsMapLayer * >::iterator mi = layers.begin();
//      std::cout << "MAP LAYER COUNT: " << layers.size() << std::endl;
			while (mi != layers.end()) {
				QgsMapLayer *ml = (*mi).second;
				if (ml) {
					//    QgsDatabaseLayer *dbl = (QgsDatabaseLayer *)&ml;
//          std::cout << "Rendering " << ml->name() << std::endl;
					if (ml->visible())
						ml->draw(paint, &currentExtent, coordXForm);
					mi++;
					//  mi.draw(p, &fullExtent);
				}
			}

			paint->end();
			drawing = false;
		}
	}
}

void QgsMapCanvas::render()
{
/*  QPainter *paint = new QPainter();
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
  */
}
void QgsMapCanvas::paintEvent(QPaintEvent *)
{
	if (!drawing)
		render2();
//  else
//      std::cout << "Can't paint in paint event -- drawing = true" << std::endl;
}

QgsRect QgsMapCanvas::extent()
{
	return currentExtent;
}

void QgsMapCanvas::setExtent(QgsRect r)
{
	currentExtent = r;
}

void QgsMapCanvas::clear()
{
	QPainter *p = new QPainter();
	p->begin(this);
	p->eraseRect(this->rect());
	p->end();

}

void QgsMapCanvas::zoomFullExtent()
{
   previousExtent = currentExtent;
	currentExtent = fullExtent;
	clear();
	render2();
}

void QgsMapCanvas::zoomPreviousExtent(){
  if(previousExtent.width() > 0){
      QgsRect tempRect = currentExtent;
      currentExtent = previousExtent;
      previousExtent = tempRect;
      clear();
      render2();
    }
  }
void QgsMapCanvas::mousePressEvent(QMouseEvent * e)
{
	mouseButtonDown = true;
	boxStartPoint = e->pos();
	switch (mapTool) {
	  case QGis::ZoomIn:
	  case QGis::ZoomOut:
		  zoomBox.setRect(0, 0, 0, 0);
		  break;

	  case QGis::Pan:
		  // create a pixmap to use in panning the map canvas
		  tempPanImage = new QPixmap();

		  //*tempPanImage = QPixmap::grabWidget(this);
		  *tempPanImage = QPixmap::grabWindow(winId());


		  backgroundFill = new QPixmap(tempPanImage->width(), tempPanImage->height());
		  backgroundFill->fill(bgColor);
		  break;
	  case QGis::Distance:
//              distanceEndPoint = e->pos();
		  break;
	}
}
void QgsMapCanvas::mouseReleaseEvent(QMouseEvent * e)
{
	QPainter paint;
	QPen pen(Qt::gray);
	QgsPoint ll, ur;
	if (dragging) {
		dragging = false;
		switch (mapTool) {
		  case QGis::ZoomIn:
			  // erase the rubber band box
			  paint.begin(this);
			  paint.setPen(pen);
			  paint.setRasterOp(Qt::XorROP);
			  paint.drawRect(zoomBox);
			  paint.end();
			  // store the rectangle
			  zoomBox.setRight(e->pos().x());
			  zoomBox.setBottom(e->pos().y());
			  // set the extent to the zoomBox

			  ll = coordXForm->toMapCoordinates(zoomBox.left(), zoomBox.bottom());
			  ur = coordXForm->toMapCoordinates(zoomBox.right(), zoomBox.top());
            previousExtent = currentExtent;
			  //QgsRect newExtent(ll.x(), ll.y(), ur.x(), ur.y());
			  currentExtent.setXmin(ll.x());
			  currentExtent.setYmin(ll.y());
			  currentExtent.setXmax(ur.x());
			  currentExtent.setYmax(ur.y());
			  currentExtent.normalize();
			  clear();
			  render2();
			  break;
		  case QGis::ZoomOut:
      {
			  // erase the rubber band box
			  paint.begin(this);
			  paint.setPen(pen);
			  paint.setRasterOp(Qt::XorROP);
			  paint.drawRect(zoomBox);
			  paint.end();
			  // store the rectangle
			  zoomBox.setRight(e->pos().x());
			  zoomBox.setBottom(e->pos().y());
			  // scale the extent so the current view fits inside the zoomBox
			  ll = coordXForm->toMapCoordinates(zoomBox.left(), zoomBox.bottom());
			  ur = coordXForm->toMapCoordinates(zoomBox.right(), zoomBox.top());
            previousExtent = currentExtent;
              QgsRect tempRect = currentExtent;
              currentExtent.setXmin(ll.x());
			   currentExtent.setYmin(ll.y());
			   currentExtent.setXmax(ur.x());
			   currentExtent.setYmax(ur.y());
			   currentExtent.normalize();
          QgsPoint cer = currentExtent.center();
          std::cout << "Current extent rectangle is " << tempRect << std::endl;
            std::cout << "Center of zoom out rectangle is " << cer << std::endl;
            std::cout << "Zoom out rectangle should have ll of " << ll << " and ur of " << ur << std::endl;
            std::cout << "Zoom out rectangle is " << currentExtent << std::endl;
              double sf;
			  if (zoomBox.width() > zoomBox.height()) {
              sf = tempRect.width()/currentExtent.width();
			  } else {
              sf = tempRect.height()/currentExtent.height();
			  }
          //center = new QgsPoint(zoomRect->center());
        //  delete zoomRect;
			  currentExtent.expand(sf);
        std::cout << "Extent scaled by " << sf << " to " << currentExtent << std::endl;
        std::cout << "Center of currentExtent after scaling is " << currentExtent.center() << std::endl;
			  clear();
			  render2();
        }
			  break;

		  case QGis::Pan:
			  // new extent based on offset
			  delete tempPanImage;
			  delete backgroundFill;

			  // use start and end box points to calculate the extent
			  QgsPoint start = coordXForm->toMapCoordinates(boxStartPoint);
			  QgsPoint end = coordXForm->toMapCoordinates(e->pos());
			  double dx = fabs(end.x() - start.x());
			  double dy = fabs(end.y() - start.y());
			  // modify the extent
             previousExtent = currentExtent;
			  if (end.x() < start.x()) {
				  currentExtent.setXmin(currentExtent.xMin() + dx);
				  currentExtent.setXmax(currentExtent.xMax() + dx);
			  } else {
				  currentExtent.setXmin(currentExtent.xMin() - dx);
				  currentExtent.setXmax(currentExtent.xMax() - dx);
			  }

			  if (end.y() < start.y()) {
				  currentExtent.setYmax(currentExtent.yMax() + dy);
				  currentExtent.setYmin(currentExtent.yMin() + dy);

			  } else {
				  currentExtent.setYmax(currentExtent.yMax() - dy);
				  currentExtent.setYmin(currentExtent.yMin() - dy);

			  }
			  clear();
			  render2();
			  break;
		}
	}
}
void QgsMapCanvas::mouseMoveEvent(QMouseEvent * e)
{
	if (e->state() == Qt::LeftButton) {
		int dx, dy;
		QPainter paint;
		QPen pen(Qt::gray);
		// this is a drag-type operation (zoom, pan or other maptool)

		switch (mapTool) {
		  case QGis::ZoomIn:
		  case QGis::ZoomOut:
			  // draw the rubber band box as the user drags the mouse
			  dragging = true;

			  paint.begin(this);
			  paint.setPen(pen);
			  paint.setRasterOp(Qt::XorROP);
			  paint.drawRect(zoomBox);

			  zoomBox.setLeft(boxStartPoint.x());
			  zoomBox.setTop(boxStartPoint.y());
			  zoomBox.setRight(e->pos().x());
			  zoomBox.setBottom(e->pos().y());

			  paint.drawRect(zoomBox);
			  paint.end();
			  break;
		  case QGis::Pan:
			  // show the temporary image as the user drags the mouse
			  dragging = true;
			  // bitBlt the pixmap on the screen, offset by the
			  // change in mouse coordinates
			  dx = e->pos().x() - boxStartPoint.x();
			  dy = e->pos().y() - boxStartPoint.y();
			  bitBlt(this, 0, 0, backgroundFill);
			  bitBlt(this, dx, dy, tempPanImage);
			  break;
		}

	}
	// show x y on status bar
	QPoint xy = e->pos();
	QgsPoint coord = coordXForm->toMapCoordinates(xy);
	emit xyCoordinates(coord);
}

/** Sets the map tool currently being used on the canvas */
void QgsMapCanvas::setMapTool(int tool)
{
	mapTool = tool;
}

/** Write property of QColor bgColor. */
void QgsMapCanvas::setbgColor(const QColor & _newVal)
{
	bgColor = _newVal;
}

/** Updates the full extent to include the mbr of the rectangle r */
void QgsMapCanvas::updateFullExtent(QgsRect r)
{
	if (r.xMin() < fullExtent.xMin())
		fullExtent.setXmin(r.xMin());
	if (r.xMax() > fullExtent.xMax())
		fullExtent.setXmax(r.xMax());
	if (r.yMin() < fullExtent.yMin())
		fullExtent.setYmin(r.yMin());
	if (r.yMax() > fullExtent.yMax())
		fullExtent.setYmax(r.yMax());
}

/*const std::map<QString,QgsMapLayer *> * QgsMapCanvas::mapLayers(){
       return &layers;
}
*/
int QgsMapCanvas::layerCount()
{
	return layers.size();
}

void QgsMapCanvas::layerStateChange()
{
	if (!frozen) {
		clear();
		render2();
	}
}

void QgsMapCanvas::freeze(bool frz)
{
	frozen = frz;
}

void QgsMapCanvas::remove(QString key)
{
	std::map < QString, QgsMapLayer * >newLayers;

	std::map < QString, QgsMapLayer * >::iterator mi = layers.begin();
	while (mi != layers.end()) {
		QgsMapLayer *ml = (*mi).second;
		if (ml->name() != key)
			newLayers[ml->name()] = ml;

		mi++;

	}



	QgsMapLayer *l = layers[key];


	layers = newLayers;
	delete l;
	zOrder.remove(key);
}
