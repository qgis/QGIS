/***************************************************************************
                          qgsmapcanvas.cpp  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Gary E.Sherman
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
#include <iostream>
#include <cfloat>
#include <cmath>
#include <qstring.h>
#include <qpainter.h>
#include <qrect.h>
#include <qevent.h>
#include <qlistview.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include "qgsrect.h"
#include "qgis.h"

#include "qgsmaplayer.h"
#include "qgslegend.h"
#include "qgslegenditem.h"
#include "qgsdatabaselayer.h"
#include "qgscoordinatetransform.h"
#include "qgsmarkersymbol.h"
#include "qgspolygonsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerinterface.h"
#include "qgsvectorlayer.h"

QgsMapCanvas::QgsMapCanvas(QWidget * parent, const char *name):QWidget(parent, name)
{
  mapWindow = new QRect();
  coordXForm = new QgsCoordinateTransform();
  bgColor = QColor(Qt::white);
  setEraseColor(bgColor);
  setMouseTracking(true);
  drawing = false;
  dirty = true;
  pmCanvas = new QPixmap(width(), height());
  setFocusPolicy(QWidget::StrongFocus);
}

QgsMapCanvas::~QgsMapCanvas()
{
  delete coordXForm;
  delete pmCanvas;
  delete mapWindow;
}

void QgsMapCanvas::setLegend(QgsLegend * legend)
{
  mapLegend = legend;
}

QgsLegend *QgsMapCanvas::getLegend()
{
  return mapLegend;
}

void QgsMapCanvas::setDirty(bool _dirty)
{
  dirty = _dirty;
}

bool QgsMapCanvas::isDirty()
{
  return dirty;
}

void QgsMapCanvas::addLayer(QgsMapLayerInterface * lyr)
{
  // add a maplayer interface to a layer type defined in a plugin

}
void QgsMapCanvas::addLayer(QgsMapLayer * lyr)
{
  if (lyr->type() != QgsMapLayer::RASTER)
    {
#ifdef QGISDEBUG
      std::cout << "Layer name is " << lyr->name() << std::endl;
#endif
      // give the layer a default symbol
      QgsSymbol *sym;
      QColor *fill;
      int red, green, blue;
      switch (lyr->featureType())
        {
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
            // temporary hack to allow addition of raster layers
            // raster layers don't have symbology. should this symbol
            // related code be in qgsshapefilelayer.cpp?
          default:
            sym = new QgsMarkerSymbol();
        }
      red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
      green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
      blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));

      sym->setColor(QColor(red, green, blue));
      sym->setLineWidth(1);
      lyr->setSymbol(sym);
    }
  layers[lyr->getLayerID()] = lyr;
  // update extent if warranted
  if (layers.size() == 1)
    {
      fullExtent = lyr->extent();
      fullExtent.scale(1.1);
      currentExtent = fullExtent;

    }

  updateFullExtent(lyr->extent());
  zOrder.push_back(lyr->getLayerID());
  QObject::connect(lyr, SIGNAL(visibilityChanged()), this, SLOT(layerStateChange()));
  QObject::connect(lyr, SIGNAL(repaintRequested()), this, SLOT(refresh()));
  dirty = true;
}

QgsMapLayer *QgsMapCanvas::getZpos(int idx)
{
  //  iterate over the zOrder and return the layer at postion idx
  std::list < QString >::iterator zi = zOrder.begin();
  for (int i = 0; i < idx; i++)
    {
      if (i < zOrder.size())
        {
          zi++;
        }
    }

  QgsMapLayer *ml = layers[*zi];
  return ml;
}

void QgsMapCanvas::setZOrderFromLegend(QgsLegend * lv)
{
  zOrder.clear();
  QListViewItemIterator it(lv);
  while (it.current())
    {
      QgsLegendItem *li = (QgsLegendItem *) it.current();
      QgsMapLayer *lyr = li->layer();
      zOrder.push_front(lyr->getLayerID());
      ++it;
    }
  refresh();
}

QgsMapLayer *QgsMapCanvas::layerByName(QString name)
{
  return layers[name];

}

void QgsMapCanvas::refresh()
{
  dirty = true;
  render2();
}

void QgsMapCanvas::render2()
{
  QString msg = frozen ? "frozen" : "thawed";
//std::cout << "Map canvas is " << msg << std::endl;
  if (!frozen && dirty)
    {
      if (!drawing)
        {
//  std::cout << "IN RENDER 2" << std::endl;
          drawing = true;
          QPainter *paint = new QPainter();
          pmCanvas->fill(bgColor);
          paint->begin(pmCanvas);
          // calculate the translation and scaling parameters
          double muppX, muppY;
          muppY = currentExtent.height() / height();
          muppX = currentExtent.width() / width();
//      std::cout << "MuppX is: " << muppX << "\nMuppY is: " << muppY << std::endl;
          m_mupp = muppY > muppX ? muppY : muppX;
          // calculate the actual extent of the mapCanvas
          double dxmin, dxmax, dymin, dymax, whitespace;
          if (muppY > muppX)
            {
              dymin = currentExtent.yMin();
              dymax = currentExtent.yMax();
              whitespace = ((width() * m_mupp) - currentExtent.width()) / 2;
              dxmin = currentExtent.xMin() - whitespace;
              dxmax = currentExtent.xMax() + whitespace;
          } else
            {
              dxmin = currentExtent.xMin();
              dxmax = currentExtent.xMax();
              whitespace = ((height() * m_mupp) - currentExtent.height()) / 2;
              dymin = currentExtent.yMin() - whitespace;
              dymax = currentExtent.yMax() + whitespace;

            }
//      std::cout << "dxmin: " << dxmin << std::endl << "dymin: " << dymin << std::
//        endl << "dymax: " << dymax << std::endl << "whitespace: " << whitespace << std::endl;
          coordXForm->setParameters(m_mupp, dxmin, dymin, height());  //currentExtent.xMin(),      currentExtent.yMin(), currentExtent.yMax());
          // update the currentExtent to match the device coordinates
          currentExtent.setXmin(dxmin);
          currentExtent.setXmax(dxmax);
          currentExtent.setYmin(dymin);
          currentExtent.setYmax(dymax);
          // render all layers in the stack, starting at the base
          std::list < QString >::iterator li = zOrder.begin();
//      std::cout << "MAP LAYER COUNT: " << layers.size() << std::endl;
          while (li != zOrder.end())
            {
              QgsMapLayer *ml = layers[*li];
              if (ml)
                {
                  //    QgsDatabaseLayer *dbl = (QgsDatabaseLayer *)&ml;
#ifdef QGISDEBUG
                  std::cout << "Rendering " << ml->name() << std::endl;
#endif
                  if (ml->visible())
                    ml->draw(paint, &currentExtent, coordXForm);
                  li++;
                  //  mi.draw(p, &fullExtent);
                }
            }
#ifdef QGISDEBUG
          std::cout << "Done rendering map layers\n";
#endif
          paint->end();
          drawing = false;
        }
      
      dirty = false;
      repaint();
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

void QgsMapCanvas::paintEvent(QPaintEvent * ev)
{
  if (!dirty)
    {
      // just bit blit the image to the canvas
      bitBlt(this, ev->rect().topLeft(), pmCanvas, ev->rect());
  } else
    {
      if (!drawing)
        render2();

    }
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
  dirty = true;
  erase();
}

void QgsMapCanvas::zoomFullExtent()
{
  previousExtent = currentExtent;
  currentExtent = fullExtent;
  clear();
  render2();
}

void QgsMapCanvas::zoomPreviousExtent()
{
  if (previousExtent.width() > 0)
    {
      QgsRect tempRect = currentExtent;
      currentExtent = previousExtent;
      previousExtent = tempRect;
      clear();
      render2();
    }
}

void QgsMapCanvas::zoomToSelected()
{
  QgsVectorLayer *lyr = dynamic_cast < QgsVectorLayer * >(mapLegend->currentLayer());

  if (lyr)
    {
      QgsRect rect = lyr->bBoxOfSelected();

      //no selected features
      if (rect.xMin() == DBL_MAX && rect.yMin() == DBL_MAX && rect.xMax() == -DBL_MAX && rect.yMax() == -DBL_MAX)
        {
          return;
        }
      //zoom to one single point
      else if (rect.xMin() == rect.xMax() && rect.yMin() == rect.yMax())
        {
          previousExtent = currentExtent;
          currentExtent.setXmin(rect.xMin() - 25);
          currentExtent.setYmin(rect.yMin() - 25);
          currentExtent.setXmax(rect.xMax() + 25);
          currentExtent.setYmax(rect.yMax() + 25);
          clear();
          render2();
          return;
        }
      //zoom to an area
      else
        {
          previousExtent = currentExtent;
          currentExtent.setXmin(rect.xMin());
          currentExtent.setYmin(rect.yMin());
          currentExtent.setXmax(rect.xMax());
          currentExtent.setYmax(rect.yMax());
          clear();
          render2();
          return;
        }
    }
}

void QgsMapCanvas::mousePressEvent(QMouseEvent * e)
{
  mouseButtonDown = true;
  boxStartPoint = e->pos();
  switch (mapTool)
    {
      case QGis::Select:
      case QGis::ZoomIn:
      case QGis::ZoomOut:
        zoomBox.setRect(0, 0, 0, 0);
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
  if (dragging)
    {
      dragging = false;
      switch (mapTool)
        {
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
              /* std::cout << "Current extent rectangle is " << tempRect << std::endl;
                 std::cout << "Center of zoom out rectangle is " << cer << std::endl;
                 std::cout << "Zoom out rectangle should have ll of " << ll << " and ur of " << ur << std::endl;
                 std::cout << "Zoom out rectangle is " << currentExtent << std::endl;
               */
              double sf;
              if (zoomBox.width() > zoomBox.height())
                {
                  sf = tempRect.width() / currentExtent.width();
              } else
                {
                  sf = tempRect.height() / currentExtent.height();
                }
              //center = new QgsPoint(zoomRect->center());
              //  delete zoomRect;
              currentExtent.expand(sf);
#ifdef QGISDEBUG
              std::cout << "Extent scaled by " << sf << " to " << currentExtent << std::endl;
              std::cout << "Center of currentExtent after scaling is " << currentExtent.center() << std::endl;
#endif
              clear();	
              render2();
            }
            break;

          case QGis::Pan:
            {
              // use start and end box points to calculate the extent
              QgsPoint start = coordXForm->toMapCoordinates(boxStartPoint);
              QgsPoint end = coordXForm->toMapCoordinates(e->pos());
              double dx = fabs(end.x() - start.x());
              double dy = fabs(end.y() - start.y());
              // modify the extent
              previousExtent = currentExtent;
              if (end.x() < start.x())
                {
                  currentExtent.setXmin(currentExtent.xMin() + dx);
                  currentExtent.setXmax(currentExtent.xMax() + dx);
              } else
                {
                  currentExtent.setXmin(currentExtent.xMin() - dx);
                  currentExtent.setXmax(currentExtent.xMax() - dx);
                }

              if (end.y() < start.y())
                {
                  currentExtent.setYmax(currentExtent.yMax() + dy);
                  currentExtent.setYmin(currentExtent.yMin() + dy);

              } else
                {
                  currentExtent.setYmax(currentExtent.yMax() - dy);
                  currentExtent.setYmin(currentExtent.yMin() - dy);

                }
              clear();	
              render2();
            }
            break;

          case QGis::Select:
            // erase the rubber band box
            paint.begin(this);
            paint.setPen(pen);
            paint.setRasterOp(Qt::XorROP);
            paint.drawRect(zoomBox);
            paint.end();

            QgsMapLayer *lyr = mapLegend->currentLayer();
            if (lyr)
              {
                QgsPoint ll, ur;

                // store the rectangle
                zoomBox.setRight(e->pos().x());
                zoomBox.setBottom(e->pos().y());

                ll = coordXForm->toMapCoordinates(zoomBox.left(), zoomBox.bottom());
                ur = coordXForm->toMapCoordinates(zoomBox.right(), zoomBox.top());

                QgsRect *search = new QgsRect(ll.x(), ll.y(), ur.x(), ur.y());
                if (e->state() == 513)
                  {
                    lyr->select(search, true);
                } else
                  {
                    lyr->select(search, false);
                  }
                delete search;
            } else
              {
                QMessageBox::warning(this, tr("No active layer"),
                                     tr("To select features, you must choose an layer active by clicking on its name in the legend"));
              }
        }
  } else
    {
      // map tools that rely on a click not a drag
      switch (mapTool)
        {
          case QGis::Identify:
            // call identify method for selected layer
            QgsMapLayer * lyr = mapLegend->currentLayer();
            if (lyr)
              {
                // create the search rectangle
                double searchRadius = extent().width() * .005;
                QgsRect *search = new QgsRect();
                // convert screen coordinates to map coordinates
                QgsPoint idPoint = coordXForm->toMapCoordinates(e->x(), e->y());
                search->setXmin(idPoint.x() - searchRadius);
                search->setXmax(idPoint.x() + searchRadius);
                search->setYmin(idPoint.y() - searchRadius);
                search->setYmax(idPoint.y() + searchRadius);
                lyr->identify(search);
                delete search;
            } else
              {
                QMessageBox::warning(this, tr("No active layer"),
                                     tr
                                     ("To identify features, you must choose an layer active by clicking on its name in the legend"));
              }
            break;


        }
    }
}
void QgsMapCanvas::resizeEvent(QResizeEvent * e)
{
  dirty = true;
  pmCanvas->resize(e->size());
}

void QgsMapCanvas::mouseMoveEvent(QMouseEvent * e)
{
  if (e->state() == Qt::LeftButton || e->state() == 513)
    {
      int dx, dy;
      QPainter paint;
      QPen pen(Qt::gray);
      // this is a drag-type operation (zoom, pan or other maptool)

      switch (mapTool)
        {
          case QGis::Select:
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
            // show the pmCanvas as the user drags the mouse
            dragging = true;
            // bitBlt the pixmap on the screen, offset by the
            // change in mouse coordinates
            dx = e->pos().x() - boxStartPoint.x();
            dy = e->pos().y() - boxStartPoint.y();

            //erase only the necessary parts to avoid flickering
            if (dx > 0)
              {
                erase(0, 0, dx, height());
            } else
              {
                erase(width() + dx, 0, -dx, height());
              }
            if (dy > 0)
              {
                erase(0, 0, width(), dy);
            } else
              {
                erase(0, height() + dy, width(), -dy);
              }

            bitBlt(this, dx, dy, pmCanvas);
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
  setEraseColor(_newVal);

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
  if (!frozen)
    {
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
  while (mi != layers.end())
    {
      QgsMapLayer *ml = (*mi).second;
      if (ml->getLayerID() != key)
        {
          newLayers[ml->getLayerID()] = ml;

          // recalculate full extent
          if (newLayers.size() == 1)
            {
              fullExtent = ml->extent();
              fullExtent.scale(1.1);
            }
          updateFullExtent(ml->extent());
        }

      mi++;

    }



  QgsMapLayer *l = layers[key];


  layers = newLayers;
  delete l;
  zOrder.remove(key);
  dirty = true;
}

void QgsMapCanvas::removeAll()
{
  layers.clear();
  zOrder.clear();
}
