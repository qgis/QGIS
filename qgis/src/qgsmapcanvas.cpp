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
#include <qsettings.h>
#include <qpaintdevicemetrics.h>
#include <qguardedptr.h>

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


/**
   Implementation struct for QgsMapCanvas
*/
struct QgsMapCanvas::Imp
{
    Imp::Imp()
        : mapWindow( new QRect ),
          coordXForm( new QgsCoordinateTransform ),
          bgColor( Qt::white ),
          drawing( false ),
          dirty( true ),
          pmCanvas( new QPixmap)
    {  }

    Imp::Imp( int width, int height )
        : mapWindow( new QRect ),
          coordXForm( new QgsCoordinateTransform ),
          bgColor( Qt::white ),
          drawing( false ),
          dirty( true ),
          pmCanvas( new QPixmap(width, height))
    {  }

    Imp::~Imp()
    {
        delete coordXForm;
        delete pmCanvas;
        delete mapWindow;
    }


    //! map containing the layers by name
    std::map < QString, QgsMapLayer * >layers;

    //! list containing the names of layers in zorder
    std::list < QString > zOrder;

    //! Full extent of the map canvas
    QgsRect fullExtent;

    //! Current extent
    QgsRect currentExtent;

    //! Previous view extent
    QgsRect previousExtent;

    //! Map window rectangle
    QRect * mapWindow;

    //! Pointer to the map legend
    QGuardedPtr<QgsLegend> mapLegend;

    /** Pointer to the coordinate transform object used to transform
        coordinates from real world to device coordinates 
    */
    QgsCoordinateTransform * coordXForm;

    /**
     * \brief Currently selected map tool.
     * @see QGis::MapTools enum for valid values
     */
    int mapTool;

    //!Flag to indicate status of mouse button
    bool mouseButtonDown;

    //! Map units per pixel
    double m_mupp;

    //! Rubber band box for dynamic zoom
    QRect zoomBox;

    //! Beginning point of a rubber band box
    QPoint boxStartPoint;

    //! Pixmap used for restoring the canvas.
    /** @note using QGuardedPtr causes sefault for some reason */
    //QGuardedPtr<QPixmap> pmCanvas;
    QPixmap * pmCanvas;

    //! Background color for the map canvas
    QColor bgColor;

    //! Flag to indicate a map canvas drag operation is taking place
    bool dragging;

    //! Vector containing the inital color for a layer
    std::vector < QColor > initialColor;

    //! Flag indicating a map refresh is in progress
    bool drawing;


    //! Flag indicating if the map canvas is frozen.
    bool frozen;

    /*! \brief Flag to track the state of the Map canvas.
     *
     * The canvas is
     * flagged as dirty by any operation that changes the state of
     * the layers or the view extent. If the canvas is not dirty, paint
     * events are handled by bit-blitting the stored canvas bitmap to
     * the canvas. This improves performance by not reading the data source
     * when no real change has occurred
     */
    bool dirty;


    //! Value use to calculate the search radius when identifying features
    // TODO - Do we need this?
    double radiusValue;

}; // struct QgsMapCanvas::Imp




QgsMapCanvas::QgsMapCanvas(QWidget * parent, const char *name)
    : QWidget(parent, name), imp_( new Imp(width(), height()) )
{
  setEraseColor(imp_->bgColor);
  setMouseTracking(true);
  setFocusPolicy(QWidget::StrongFocus);
} // QgsMapCanvas ctor


QgsMapCanvas::~QgsMapCanvas()
{
    // imp_ auto-deleted via std::auto_ptr
    // Imp struct has its own dtor for freeing resources
} // dtor



void QgsMapCanvas::setLegend(QgsLegend * legend)
{
  imp_->mapLegend = legend;
} // setLegend



QgsLegend *QgsMapCanvas::getLegend()
{
  return imp_->mapLegend;
} // getLegend


void QgsMapCanvas::setDirty(bool _dirty)
{
  imp_->dirty = _dirty;
} // setDirty



bool QgsMapCanvas::isDirty() const
{
  return imp_->dirty;
} // isDirty



bool QgsMapCanvas::isDrawing()
{
  return imp_->drawing;;
} // isDrawing



void QgsMapCanvas::addLayer(QgsMapLayerInterface * lyr)
{
  // add a maplayer interface to a layer type defined in a plugin
} // addlayer



void QgsMapCanvas::addLayer(QgsMapLayer * lyr)
{
#ifdef QGISDEBUG
    std::cout << "Layer name is " << lyr->name() << std::endl;
#endif

    Q_CHECK_PTR( lyr );

    if ( ! lyr ) 
    { return; }

    imp_->layers[lyr->getLayerID()] = lyr;

    // update extent if warranted
    if (imp_->layers.size() == 1)
    {
        imp_->fullExtent = lyr->extent();
        imp_->fullExtent.scale(1.1); // XXX why magic number of 1.1?
        imp_->currentExtent = imp_->fullExtent;
    }
    else
    {
        updateFullExtent(lyr->extent());
    }

    imp_->zOrder.push_back(lyr->getLayerID());

    QObject::connect(lyr, SIGNAL(visibilityChanged()), this, SLOT(layerStateChange()));
    QObject::connect(lyr, SIGNAL(repaintRequested()), this, SLOT(refresh()));

    imp_->dirty = true;

    emit addedLayer( lyr );

} // addLayer


QgsMapLayer *QgsMapCanvas::getZpos(int idx)
{
  //  iterate over the zOrder and return the layer at postion idx
  std::list < QString >::iterator zi = imp_->zOrder.begin();
  for (int i = 0; i < idx; i++)
    {
      if (i < imp_->zOrder.size())
        {
          zi++;
        }
    }

  QgsMapLayer *ml = imp_->layers[*zi];
  return ml;
} // getZpos



void QgsMapCanvas::setZOrderFromLegend(QgsLegend * lv)
{
  imp_->zOrder.clear();
  QListViewItemIterator it(lv);

  while (it.current())
    {
      QgsLegendItem *li = (QgsLegendItem *) it.current();
      QgsMapLayer *lyr = li->layer();
      imp_->zOrder.push_front(lyr->getLayerID());
      ++it;
    }

  refresh();

} // setZOrderFromLegend



QgsMapLayer *QgsMapCanvas::layerByName(QString name)
{
  return imp_->layers[name];
} // layerByName



void QgsMapCanvas::refresh()
{
  imp_->dirty = true;
  render();
} // refresh



// The painter device parameter is optional - if ommitted it will default
// to the pmCanvas (ie the gui map display). The idea is that you can pass
// an alternative device such as one that will be used for printing or 
// saving a map view as an image file.
void QgsMapCanvas::render(QPaintDevice * theQPaintDevice)
{
    QString msg = imp_->frozen ? "frozen" : "thawed";

#ifdef QGISDEBUG
  std::cout << "Map canvas is " << msg << std::endl;
#endif

  if (! imp_->frozen && imp_->dirty)
    {
      if (!imp_->drawing)
        {
//  std::cout << "IN RENDER 2" << std::endl;
          imp_->drawing = true;
          QPainter *paint = new QPainter();

          //default to pmCanvas if no paintdevice is supplied
          if ( ! theQPaintDevice )
          {
            imp_->pmCanvas->fill(imp_->bgColor);
            paint->begin(imp_->pmCanvas);
          }
          else
          {
            paint->begin(theQPaintDevice);
          }

          // calculate the translation and scaling parameters
          double muppX, muppY;
          muppY = imp_->currentExtent.height() / height();
          muppX = imp_->currentExtent.width() / width();
          imp_->m_mupp = muppY > muppX ? muppY : muppX;

          // calculate the actual extent of the mapCanvas
          double dxmin, dxmax, dymin, dymax, whitespace;

          if (muppY > muppX)
            {
              dymin = imp_->currentExtent.yMin();
              dymax = imp_->currentExtent.yMax();
              whitespace = ((width() * imp_->m_mupp) - imp_->currentExtent.width()) / 2;
              dxmin = imp_->currentExtent.xMin() - whitespace;
              dxmax = imp_->currentExtent.xMax() + whitespace;
          } 
          else
          {
              dxmin = imp_->currentExtent.xMin();
              dxmax = imp_->currentExtent.xMax();
              whitespace = ((height() * imp_->m_mupp) - imp_->currentExtent.height()) / 2;
              dymin = imp_->currentExtent.yMin() - whitespace;
              dymax = imp_->currentExtent.yMax() + whitespace;
          }

        //update the scale shown on the statusbar
	currentScale(0);

          //std::cout << "dxmin: " << dxmin << std::endl << "dymin: " << dymin << std::
          // endl << "dymax: " << dymax << std::endl << "whitespace: " << whitespace << std::endl;
          imp_->coordXForm->setParameters(imp_->m_mupp, dxmin, dymin, height());  //currentExtent.xMin(),      currentExtent.yMin(), currentExtent.yMax());
          // update the currentExtent to match the device coordinates
          //GS - removed the current extent update to fix bug --
          //TODO remove the next 4 lines after we're sure this works ok
          imp_->currentExtent.setXmin(dxmin);
          imp_->currentExtent.setXmax(dxmax);
          imp_->currentExtent.setYmin(dymin);
          imp_->currentExtent.setYmax(dymax); 
          
          // render all layers in the stack, starting at the base
          std::list < QString >::iterator li = imp_->zOrder.begin();
//      std::cout << "MAP LAYER COUNT: " << layers.size() << std::endl;
          while (li != imp_->zOrder.end())
            {
              QgsMapLayer *ml = imp_->layers[*li];

              if (ml)
                {
                  //    QgsDatabaseLayer *dbl = (QgsDatabaseLayer *)&ml;
#ifdef QGISDEBUG
                  std::cout << "Rendering " << ml->name() << std::endl;
#endif
                  if (ml->visible())
                  {
                    ml->draw(paint, &imp_->currentExtent, imp_->coordXForm,imp_->pmCanvas,this);
                  }

                  li++;
                  //  mi.draw(p, &fullExtent);
                }
            }
#ifdef QGISDEBUG
          std::cout << "Done rendering map layers\n";
#endif
          paint->end();
          imp_->drawing = false;
        }
      
      imp_->dirty = false;
      
      // notify any listeners that rendering is complete
      //note that pmCanvas is not draw to gui yet
      
      emit renderComplete();
      
      repaint();
    }

} // render



void QgsMapCanvas::currentScale(int thePrecision)
{
          // calculate the translation and scaling parameters
          double muppX, muppY;
          muppY = imp_->currentExtent.height() / height();
          muppX = imp_->currentExtent.width() / width();
          imp_->m_mupp = muppY > muppX ? muppY : muppX;

          std::cout << "Current extent is " <<
            imp_->currentExtent.stringRep() << std::endl; std::cout << "MuppX is: " <<
            muppX << "\nMuppY is: " << muppY << std::endl; 
          std::cout << "Canvas width: " << width() << ", height: " << height() << std::endl;
          std::cout << "Extent width: " << imp_->currentExtent.width() << ", height: " 
            << imp_->currentExtent.height() << std::endl; 
          
          QPaintDeviceMetrics pdm(this); 
          std::cout << "dpiX " << pdm.logicalDpiX() << ", dpiY " <<
            pdm.logicalDpiY() << std::endl; 
          std::cout << "widthMM " << pdm.widthMM() << ", heightMM " 
            << pdm.heightMM() << std::endl; 
	    

          // calculate the actual extent of the mapCanvas
          double dxmin, dxmax, dymin, dymax, whitespace;
          if (muppY > muppX)
            {
              dymin = imp_->currentExtent.yMin();
              dymax = imp_->currentExtent.yMax();
              whitespace = ((width() * imp_->m_mupp) - imp_->currentExtent.width()) / 2;
              dxmin = imp_->currentExtent.xMin() - whitespace;
              dxmax = imp_->currentExtent.xMax() + whitespace;
          } else
            {
              dxmin = imp_->currentExtent.xMin();
              dxmax = imp_->currentExtent.xMax();
              whitespace = ((height() * imp_->m_mupp) - imp_->currentExtent.height()) / 2;
              dymin = imp_->currentExtent.yMin() - whitespace;
              dymax = imp_->currentExtent.yMax() + whitespace;

            }	    
	    
          std::cout << "Scale (assuming meters as map units) = 1:" 
          << ((dxmax-dxmin) * 3.28 * 12)/(width()/pdm.logicalDpiX()) << std::endl; 

	    //@todo return a proper value
	    QString myScaleString = QString("Scale 1: ")+QString::number(((dxmax-dxmin) * 3.28 * 12)/(width()/pdm.logicalDpiX()),'f',thePrecision);
	    emit scaleChanged(myScaleString) ;
}




void QgsMapCanvas::saveAsImage(QString theFileName, QPixmap * theQPixmap)
{
  //
  //check if the optional QPaintDevice was supplied
  //
  if (theQPixmap != NULL)
  {
    render(theQPixmap);
    theQPixmap->save(theFileName,"PNG");
  }
  else //use the map view
  {
    imp_->pmCanvas->save(theFileName,"PNG");
  }
} // saveAsImage



void QgsMapCanvas::paintEvent(QPaintEvent * ev)
{
  if (!imp_->dirty)
    {
      // just bit blit the image to the canvas
      bitBlt(this, ev->rect().topLeft(), imp_->pmCanvas, ev->rect());
  } else
  {
      if (!imp_->drawing)
      {
          render();
      }
  }
} // paintEvent



QgsRect const & QgsMapCanvas::extent() const
{
  return imp_->currentExtent;
} // extent


void QgsMapCanvas::setExtent(QgsRect const & r)
{
  imp_->currentExtent = r;
  emit extentsChanged(imp_->currentExtent.stringRep(2));
} // setExtent


void QgsMapCanvas::clear()
{
  imp_->dirty = true;
  erase();
} // clear



void QgsMapCanvas::zoomFullExtent()
{
  imp_->previousExtent = imp_->currentExtent;
  imp_->currentExtent = imp_->fullExtent;

  clear();
  render();
  emit extentsChanged(imp_->currentExtent.stringRep(2));
} // zoomFullExtent



void QgsMapCanvas::zoomPreviousExtent()
{
  if (imp_->previousExtent.width() > 0)
  {
      QgsRect tempRect = imp_->currentExtent;
      imp_->currentExtent = imp_->previousExtent;
      imp_->previousExtent = tempRect;
      clear();
      render();
      emit extentsChanged(imp_->currentExtent.stringRep(2));
  }
} // zoomPreviousExtent



void QgsMapCanvas::zoomToSelected()
{
  QgsVectorLayer *lyr =
      dynamic_cast < QgsVectorLayer * >(imp_->mapLegend->currentLayer());

  if (lyr)
  {
      QgsRect rect = lyr->bBoxOfSelected();

      //no selected features
      if (rect.xMin() == DBL_MAX && 
          rect.yMin() == DBL_MAX && 
          rect.xMax() == -DBL_MAX && 
          rect.yMax() == -DBL_MAX)
      {
          return;
      }
      //zoom to one single point
      else if (rect.xMin() == rect.xMax() && 
               rect.yMin() == rect.yMax())
        {
          imp_->previousExtent = imp_->currentExtent;
          imp_->currentExtent.setXmin(rect.xMin() - 25);
          imp_->currentExtent.setYmin(rect.yMin() - 25);
          imp_->currentExtent.setXmax(rect.xMax() + 25);
          imp_->currentExtent.setYmax(rect.yMax() + 25);
          clear();
          render();
	  emit extentsChanged(imp_->currentExtent.stringRep(2));
          return;
        }
      //zoom to an area
      else
        {
          imp_->previousExtent = imp_->currentExtent;
          imp_->currentExtent.setXmin(rect.xMin());
          imp_->currentExtent.setYmin(rect.yMin());
          imp_->currentExtent.setXmax(rect.xMax());
          imp_->currentExtent.setYmax(rect.yMax());
          clear();
          render();
	  emit extentsChanged(imp_->currentExtent.stringRep(2));
          return;
        }
    }
} // zoomToSelected



void QgsMapCanvas::mousePressEvent(QMouseEvent * e)
{
  imp_->mouseButtonDown = true;
  imp_->boxStartPoint = e->pos();

  switch (imp_->mapTool)
    {
      case QGis::Select:
      case QGis::ZoomIn:
      case QGis::ZoomOut:
        imp_->zoomBox.setRect(0, 0, 0, 0);
        break;
      case QGis::Distance:
//              distanceEndPoint = e->pos();
        break;
    }
} // mousePressEvent


void QgsMapCanvas::mouseReleaseEvent(QMouseEvent * e)
{
  QPainter paint;
  QPen     pen(Qt::gray);
  QgsPoint ll, ur;

  if (imp_->dragging)
    {
      imp_->dragging = false;

      switch (imp_->mapTool)
      {
          case QGis::ZoomIn:
            // erase the rubber band box
            paint.begin(this);
            paint.setPen(pen);
            paint.setRasterOp(Qt::XorROP);
            paint.drawRect(imp_->zoomBox);
            paint.end();
            // store the rectangle
            imp_->zoomBox.setRight(e->pos().x());
            imp_->zoomBox.setBottom(e->pos().y());
            // set the extent to the zoomBox

            ll = imp_->coordXForm->toMapCoordinates(imp_->zoomBox.left(), imp_->zoomBox.bottom());
            ur = imp_->coordXForm->toMapCoordinates(imp_->zoomBox.right(), imp_->zoomBox.top());
            imp_->previousExtent = imp_->currentExtent;
            //QgsRect newExtent(ll.x(), ll.y(), ur.x(), ur.y());
            imp_->currentExtent.setXmin(ll.x());
            imp_->currentExtent.setYmin(ll.y());
            imp_->currentExtent.setXmax(ur.x());
            imp_->currentExtent.setYmax(ur.y());
            imp_->currentExtent.normalize();
	    emit extentsChanged(imp_->currentExtent.stringRep(2));
            clear();
            render();
            
            break;
          case QGis::ZoomOut:
            {
              // erase the rubber band box
              paint.begin(this);
              paint.setPen(pen);
              paint.setRasterOp(Qt::XorROP);
              paint.drawRect(imp_->zoomBox);
              paint.end();
              // store the rectangle
              imp_->zoomBox.setRight(e->pos().x());
              imp_->zoomBox.setBottom(e->pos().y());
              // scale the extent so the current view fits inside the zoomBox
              ll = imp_->coordXForm->toMapCoordinates(imp_->zoomBox.left(), imp_->zoomBox.bottom());
              ur = imp_->coordXForm->toMapCoordinates(imp_->zoomBox.right(), imp_->zoomBox.top());
              imp_->previousExtent = imp_->currentExtent;
              QgsRect tempRect = imp_->currentExtent;
              imp_->currentExtent.setXmin(ll.x());
              imp_->currentExtent.setYmin(ll.y());
              imp_->currentExtent.setXmax(ur.x());
              imp_->currentExtent.setYmax(ur.y());
              imp_->currentExtent.normalize();

              QgsPoint cer = imp_->currentExtent.center();

              /* std::cout << "Current extent rectangle is " << tempRect << std::endl;
                 std::cout << "Center of zoom out rectangle is " << cer << std::endl;
                 std::cout << "Zoom out rectangle should have ll of " << ll << " and ur of " << ur << std::endl;
                 std::cout << "Zoom out rectangle is " << imp_->currentExtent << std::endl;
               */
              double sf;
              if (imp_->zoomBox.width() > imp_->zoomBox.height())
                {
                  sf = tempRect.width() / imp_->currentExtent.width();
              } else
                {
                  sf = tempRect.height() / imp_->currentExtent.height();
                }
              //center = new QgsPoint(zoomRect->center());
              //  delete zoomRect;
              imp_->currentExtent.expand(sf);
#ifdef QGISDEBUG
              std::cout << "Extent scaled by " << sf << " to " << imp_->currentExtent << std::endl;
              std::cout << "Center of currentExtent after scaling is " << imp_->currentExtent.center() << std::endl;
#endif
              clear();	
              render();
	      emit extentsChanged(imp_->currentExtent.stringRep(2));
            }
            break;

          case QGis::Pan:
            {
              // use start and end box points to calculate the extent
              QgsPoint start = imp_->coordXForm->toMapCoordinates(imp_->boxStartPoint);
              QgsPoint end = imp_->coordXForm->toMapCoordinates(e->pos());

              double dx = fabs(end.x() - start.x());
              double dy = fabs(end.y() - start.y());

              // modify the extent
              imp_->previousExtent = imp_->currentExtent;

              if (end.x() < start.x())
                {
                  imp_->currentExtent.setXmin(imp_->currentExtent.xMin() + dx);
                  imp_->currentExtent.setXmax(imp_->currentExtent.xMax() + dx);
              } else
                {
                  imp_->currentExtent.setXmin(imp_->currentExtent.xMin() - dx);
                  imp_->currentExtent.setXmax(imp_->currentExtent.xMax() - dx);
                }

              if (end.y() < start.y())
                {
                  imp_->currentExtent.setYmax(imp_->currentExtent.yMax() + dy);
                  imp_->currentExtent.setYmin(imp_->currentExtent.yMin() + dy);

              } else
                {
                  imp_->currentExtent.setYmax(imp_->currentExtent.yMax() - dy);
                  imp_->currentExtent.setYmin(imp_->currentExtent.yMin() - dy);

                }
              clear();	
              render();
	      emit extentsChanged(imp_->currentExtent.stringRep(2));
            }
            break;

          case QGis::Select:
            // erase the rubber band box
            paint.begin(this);
            paint.setPen(pen);
            paint.setRasterOp(Qt::XorROP);
            paint.drawRect(imp_->zoomBox);
            paint.end();

            QgsMapLayer *lyr = imp_->mapLegend->currentLayer();

            if (lyr)
              {
                QgsPoint ll, ur;

                // store the rectangle
                imp_->zoomBox.setRight(e->pos().x());
                imp_->zoomBox.setBottom(e->pos().y());

                ll = imp_->coordXForm->toMapCoordinates(imp_->zoomBox.left(), imp_->zoomBox.bottom());
                ur = imp_->coordXForm->toMapCoordinates(imp_->zoomBox.right(), imp_->zoomBox.top());

                QgsRect *search = new QgsRect(ll.x(), ll.y(), ur.x(), ur.y());

                if (e->state() == 513) // XXX 513? Magic numbers bad!
                  {
                    lyr->select(search, true);
                } else
                  {
                    lyr->select(search, false);
                  }
                delete search;
            } else
              {
                QMessageBox::warning(this, 
                                     tr("No active layer"),
                                     tr("To select features, you must choose an layer active by clicking on its name in the legend"));
              }
        }
  } else
    {
      // map tools that rely on a click not a drag
      switch (imp_->mapTool)
        {
          case QGis::Identify:
            // call identify method for selected layer
            QgsMapLayer * lyr = imp_->mapLegend->currentLayer();

            if (lyr)
              {
								
                // create the search rectangle
                double searchRadius = extent().width() * calculateSearchRadiusValue();
                QGuardedPtr<QgsRect> search = new QgsRect();
                // convert screen coordinates to map coordinates
                QgsPoint idPoint = imp_->coordXForm->toMapCoordinates(e->x(), e->y());
                search->setXmin(idPoint.x() - searchRadius);
                search->setXmax(idPoint.x() + searchRadius);
                search->setYmin(idPoint.y() - searchRadius);
                search->setYmax(idPoint.y() + searchRadius);

                lyr->identify(search);

                delete search;
            } else
            {
                QMessageBox::warning(this, 
                                     tr("No active layer"),
                                     tr("To identify features, you must choose an layer active by clicking on its name in the legend"));
            }
            break;
        }
    }
} // mouseReleaseEvent



void QgsMapCanvas::resizeEvent(QResizeEvent * e)
{
  imp_->dirty = true;
  imp_->pmCanvas->resize(e->size());
  emit extentsChanged(imp_->currentExtent.stringRep(2));
} // resizeEvent



void QgsMapCanvas::mouseMoveEvent(QMouseEvent * e)
{
    // XXX magic numbers BAD -- 513?
  if (e->state() == Qt::LeftButton || e->state() == 513)
    {
      int dx, dy;
      QPainter paint;
      QPen pen(Qt::gray);

      // this is a drag-type operation (zoom, pan or other maptool)

      switch (imp_->mapTool)
        {
          case QGis::Select:
          case QGis::ZoomIn:
          case QGis::ZoomOut:
            // draw the rubber band box as the user drags the mouse
            imp_->dragging = true;

            paint.begin(this);
            paint.setPen(pen);
            paint.setRasterOp(Qt::XorROP);
            paint.drawRect(imp_->zoomBox);

            imp_->zoomBox.setLeft(imp_->boxStartPoint.x());
            imp_->zoomBox.setTop(imp_->boxStartPoint.y());
            imp_->zoomBox.setRight(e->pos().x());
            imp_->zoomBox.setBottom(e->pos().y());

            paint.drawRect(imp_->zoomBox);
            paint.end();
            break;

          case QGis::Pan:
            // show the pmCanvas as the user drags the mouse
            imp_->dragging = true;
            // bitBlt the pixmap on the screen, offset by the
            // change in mouse coordinates
            dx = e->pos().x() - imp_->boxStartPoint.x();
            dy = e->pos().y() - imp_->boxStartPoint.y();

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

            bitBlt(this, dx, dy, imp_->pmCanvas);
            break;
        }

    }

  // show x y on status bar
  QPoint xy = e->pos();
  QgsPoint coord = imp_->coordXForm->toMapCoordinates(xy);
  emit xyCoordinates(coord);

} // mouseMoveEvent


/** Sets the map tool currently being used on the canvas */
void QgsMapCanvas::setMapTool(int tool)
{
  imp_->mapTool = tool;
} // setMapTool


/** Write property of QColor bgColor. */
void QgsMapCanvas::setbgColor(const QColor & _newVal)
{
  imp_->bgColor = _newVal;
  setEraseColor(_newVal);
} // setbgColor



/** Updates the full extent to include the mbr of the rectangle r */
void QgsMapCanvas::updateFullExtent(QgsRect const & r)
{
  if (r.xMin() < imp_->fullExtent.xMin())
  {
    imp_->fullExtent.setXmin(r.xMin());
  }

  if (r.xMax() > imp_->fullExtent.xMax())
  {
    imp_->fullExtent.setXmax(r.xMax());
  }

  if (r.yMin() < imp_->fullExtent.yMin())
  {
    imp_->fullExtent.setYmin(r.yMin());
  }

  if (r.yMax() > imp_->fullExtent.yMax())
  {
    imp_->fullExtent.setYmax(r.yMax());
  }

  emit extentsChanged(imp_->currentExtent.stringRep(2));
} // updateFullExtent



/*const std::map<QString,QgsMapLayer *> * QgsMapCanvas::mapLayers(){
       return &layers;
}
*/
int QgsMapCanvas::layerCount() const
{
  return imp_->layers.size();
} // layerCount



void QgsMapCanvas::layerStateChange()
{
  if (!imp_->frozen)
    {
      clear();
      render();
    }
} // layerStateChange



void QgsMapCanvas::freeze(bool frz)
{
  imp_->frozen = frz;
} // freeze



void QgsMapCanvas::remove(QString const & key)
{
    // XXX As a safety precaution, shouldn't we check to see if the 'key' is
    // XXX in 'layers'?  Theoretically it should be ok to skip this check since
    // XXX this should always be called with correct keys.

    delete imp_->layers[key];   // first delete the map layer itself

    imp_->layers.erase( key );  // then erase its entry from layer table

    // XXX after removing this layer, we should probably compact the
    // XXX remaining Z values
    imp_->zOrder.remove(key);   // ... and it's Z order entry, too

    // Since we removed a layer, we may have to adjust the map canvas'
    // over-all extent; so we update to the largest extent found in the
    // remaining layers.

    if ( imp_->layers.empty() )
    {
        // XXX do we want to reset the extents if the last layer is deleted?
    }
    else
    {
        std::map < QString, QgsMapLayer * >::iterator mi = 
            imp_->layers.begin();

        imp_->fullExtent = mi->second->extent();
        imp_->fullExtent.scale(1.1); // XXX why set the scale to this magic
                                     // XXX number?

        ++mi;

        for ( ; mi != imp_->layers.end(); ++mi )
        {
            updateFullExtent(mi->second->extent());
        }
    }

    imp_->dirty = true;

    // signal that we've erased this layer
    emit removedLayer( key );

} // remove



void QgsMapCanvas::removeAll()
{

// Welllllll, yeah, this works, but now we have to ensure that the
// removedLayer() signal is emitted.
//   imp_->layers.clear();
//   imp_->zOrder.clear();

    // So:
    std::map < QString, QgsMapLayer * >::iterator mi = 
        imp_->layers.begin();

    QString current_key;

    while ( mi != imp_->layers.end() )
    {
        // save the current key
        current_key = mi->first;

        // delete it, ensuring removedLayer emitted and zOrder updated (not
        // that the zOrder ultimately matters since they're all going to go,
        // too)
        remove( current_key );

        // since mi is now invalidated because the std::map was modified in
        // remove(), reset it to the first element, if any
        mi = imp_->layers.begin();
    }
    
} // removeAll



/* Calculates the search radius for identifying features
* using the radius value stored in the users settings
*/
double QgsMapCanvas::calculateSearchRadiusValue()
{
  QSettings settings;

  int identifyValue = settings.readNumEntry("/qgis/map/identifyRadius", 5);

  return(identifyValue/1000.0);

} // calculateSearchRadiusValue


QPixmap * QgsMapCanvas::canvasPixmap()
{
    return imp_->pmCanvas;
} // canvasPixmap



void QgsMapCanvas::setCanvasPixmap(QPixmap * theQPixmap)
{
    imp_->pmCanvas = theQPixmap; 
} // setCanvasPixmap



std::list < QString > const & QgsMapCanvas::zOrders() const
{
    return imp_->zOrder;
} // zOrders


std::list < QString >       & QgsMapCanvas::zOrders()
{
    return imp_->zOrder;
} // zOrders


double QgsMapCanvas::mupp() const
{
    return imp_->m_mupp;
} // mupp
