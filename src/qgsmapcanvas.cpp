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
#include <memory>
#include <cassert>

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
#include "qgscoordinatetransform.h"
#include "qgsmarkersymbol.h"
#include "qgspolygonsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerinterface.h"
#include "qgsvectorlayer.h"
#include "qgsscalecalculator.h"
#include "qgsacetaterectangle.h"
#include "qgsfeature.h"

/**
  Implementation struct for QgsMapCanvas
  */
struct QgsMapCanvas::CanvasProperties
{
  CanvasProperties::CanvasProperties()
    : mapWindow( new QRect ),
  coordXForm( new QgsCoordinateTransform ),
  bgColor( Qt::white ),
  drawing( false ),
  dirty( true ),
  pmCanvas( new QPixmap),
  scaleCalculator( new QgsScaleCalculator)

  {
  }

  CanvasProperties::CanvasProperties( int width, int height )
    : mapWindow( new QRect ),
  coordXForm( new QgsCoordinateTransform ),
  bgColor( Qt::white ),
  drawing( false ),
  dirty( true ),
  pmCanvas( new QPixmap(width, height)),
  scaleCalculator( new QgsScaleCalculator)
  {
  }

  CanvasProperties::~CanvasProperties()
  {
    delete coordXForm;
    delete pmCanvas;
    delete mapWindow;
    delete scaleCalculator;
  }

  void CanvasProperties::initMetrics(QPaintDeviceMetrics *pdm){
    // set the logical dpi
    mDpi = pdm->logicalDpiX();
    scaleCalculator->setDpi(mDpi);
    // set default map units
    mMapUnits = QgsScaleCalculator::METERS;
    scaleCalculator->setMapUnits(mMapUnits);

  }
  void CanvasProperties::setMapUnits(int units)
  {
    mMapUnits = units;
    scaleCalculator->setMapUnits(mMapUnits);
  }
  int CanvasProperties::mapUnits()
  {
    return mMapUnits;
  }
  //! map containing the layers by name
  std::map < QString, QgsMapLayer * >layers;

  //! map containing the acetate objects by key (name)
  std::map <QString, QgsAcetateObject *> acetateObjects;

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

  QgsScaleCalculator *scaleCalculator;

  //! DPI of physical display
  int mDpi;

  //! Map units for the data on the canvas
  int mMapUnits;

  //! Map scale of the canvas at its current zool level
  double mScale;

}; // struct QgsMapCanvas::CanvasProperties




  QgsMapCanvas::QgsMapCanvas(QWidget * parent, const char *name)
: QWidget(parent, name), mCanvasProperties( new CanvasProperties(width(), height()) )
{
  setEraseColor(mCanvasProperties->bgColor);
  //by default we allow a user to interact with the canvas
  mUserInteractionAllowed=true;
  setMouseTracking(true);
  setFocusPolicy(QWidget::StrongFocus);
  QPaintDeviceMetrics *pdm = new QPaintDeviceMetrics(this);
  mCanvasProperties->initMetrics(pdm);
  delete pdm;
} // QgsMapCanvas ctor


QgsMapCanvas::~QgsMapCanvas()
{
  // mCanvasProperties auto-deleted via std::auto_ptr
  // CanvasProperties struct has its own dtor for freeing resources
} // dtor



void QgsMapCanvas::setLegend(QgsLegend * legend)
{
  mCanvasProperties->mapLegend = legend;
} // setLegend



QgsLegend *QgsMapCanvas::getLegend()
{
  return mCanvasProperties->mapLegend;
} // getLegend

double QgsMapCanvas::getScale()
{
  return mCanvasProperties->mScale;
} // getScale

void QgsMapCanvas::setDirty(bool _dirty)
{
  mCanvasProperties->dirty = _dirty;
} // setDirty



bool QgsMapCanvas::isDirty() const
{
  return mCanvasProperties->dirty;
} // isDirty



bool QgsMapCanvas::isDrawing()
{
  return mCanvasProperties->drawing;;
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

  mCanvasProperties->layers[lyr->getLayerID()] = lyr;

  // update extent if warranted
  if (mCanvasProperties->layers.size() == 1)
  {
    mCanvasProperties->fullExtent = lyr->extent();
    mCanvasProperties->fullExtent.scale(1.1);
    // XXX why magic number of 1.1? - TO GET SOME WHITESPACE AT THE EDGES OF THE MAP
    mCanvasProperties->currentExtent = mCanvasProperties->fullExtent;
  }
  else
  {
    updateFullExtent(lyr->extent());
  }

  mCanvasProperties->zOrder.push_back(lyr->getLayerID());

  QObject::connect(lyr, SIGNAL(visibilityChanged()), this, SLOT(layerStateChange()));
  QObject::connect(lyr, SIGNAL(repaintRequested()), this, SLOT(refresh()));

  mCanvasProperties->dirty = true;

  emit addedLayer( lyr );

} // addLayer

void QgsMapCanvas::addAcetateObject(QString key, QgsAcetateObject *obj)
{
  // since we are adding pointers, check to see if the object
  // referenced by key already exists and if so, delete it prior
  // to adding the new object with the same key
  QgsAcetateObject *oldObj = mCanvasProperties->acetateObjects[key];
  if(oldObj)
  {
    delete oldObj;
  }

  mCanvasProperties->acetateObjects[key] = obj;
}

QgsMapLayer *QgsMapCanvas::getZpos(int idx)
{
  //  iterate over the zOrder and return the layer at postion idx
  std::list < QString >::iterator zi = mCanvasProperties->zOrder.begin();
  for (int i = 0; i < idx; i++)
  {
    if (i < mCanvasProperties->zOrder.size())
    {
      zi++;
    }
  }

  QgsMapLayer *ml = mCanvasProperties->layers[*zi];
  return ml;
} // getZpos



void QgsMapCanvas::setZOrderFromLegend(QgsLegend * lv)
{
  mCanvasProperties->zOrder.clear();
  QListViewItemIterator it(lv);

  while (it.current())
  {
    QgsLegendItem *li = (QgsLegendItem *) it.current();
    QgsMapLayer *lyr = li->layer();
    mCanvasProperties->zOrder.push_front(lyr->getLayerID());
    ++it;
  }

  refresh();

} // setZOrderFromLegend



QgsMapLayer *QgsMapCanvas::layerByName(QString name)
{
  return mCanvasProperties->layers[name];
} // layerByName



void QgsMapCanvas::refresh()
{
  mCanvasProperties->dirty = true;
  render();
} // refresh



// The painter device parameter is optional - if ommitted it will default
// to the pmCanvas (ie the gui map display). The idea is that you can pass
// an alternative device such as one that will be used for printing or
// saving a map view as an image file.
void QgsMapCanvas::render(QPaintDevice * theQPaintDevice)
{
  QString msg = mCanvasProperties->frozen ? "frozen" : "thawed";
#ifdef QGISDEBUG
  std::cout << ".............................." << std::endl;
  std::cout << "...........Rendering.........." << std::endl;
  std::cout << ".............................." << std::endl;
  std::cout << "Map canvas is " << msg << std::endl;
#endif
  int myHeight=0;
  int myWidth=0;
  if (! mCanvasProperties->frozen && mCanvasProperties->dirty)
  {
    if (!mCanvasProperties->drawing)
    {
      mCanvasProperties->drawing = true;
      QPainter *paint = new QPainter();

      //default to pmCanvas if no paintdevice is supplied
      if ( ! theQPaintDevice )  //painting to mapCanvas->pixmap
      {
        mCanvasProperties->pmCanvas->fill(mCanvasProperties->bgColor);
        paint->begin(mCanvasProperties->pmCanvas);
        myHeight=height();
        myWidth=width();
      }
      else  //painting to an arbitary pixmap passed to render()
      {
        // need width/height of paint device
        QPaintDeviceMetrics myMetrics( theQPaintDevice );
        myHeight = myMetrics.height();
        myWidth = myMetrics.width();
        //fall back to widget height & width if retrieving them from passed in canvas fails
        if (myHeight==0)
        {
          myHeight=height();
        }
        if (myWidth==0)
        {
          myWidth=width();
        }
        //initialise the painter
        paint->begin(theQPaintDevice);
      }

      // calculate the translation and scaling parameters
      // mupp = map units per pixel
      double muppX, muppY;
      muppY = mCanvasProperties->currentExtent.height() / myHeight;
      muppX = mCanvasProperties->currentExtent.width() / myWidth;
      mCanvasProperties->m_mupp = muppY > muppX ? muppY : muppX;

      // calculate the actual extent of the mapCanvas
      double dxmin, dxmax, dymin, dymax, whitespace;

      if (muppY > muppX)
      {
        dymin = mCanvasProperties->currentExtent.yMin();
        dymax = mCanvasProperties->currentExtent.yMax();
        whitespace = ((myWidth * mCanvasProperties->m_mupp) - mCanvasProperties->currentExtent.width()) / 2;
        dxmin = mCanvasProperties->currentExtent.xMin() - whitespace;
        dxmax = mCanvasProperties->currentExtent.xMax() + whitespace;
      }
      else
      {
        dxmin = mCanvasProperties->currentExtent.xMin();
        dxmax = mCanvasProperties->currentExtent.xMax();
        whitespace = ((myHeight * mCanvasProperties->m_mupp) - mCanvasProperties->currentExtent.height()) / 2;
        dymin = mCanvasProperties->currentExtent.yMin() - whitespace;
        dymax = mCanvasProperties->currentExtent.yMax() + whitespace;
      }

      //update the scale shown on the statusbar
      currentScale(0);

#ifdef QGISDEBUG
      std::cout 
        << "Paint device width : " << myWidth << std::endl
        << "Paint device height : " << myHeight << std::endl
        << "Canvas current extent height : " << mCanvasProperties->currentExtent.height()  << std::endl
        << "Canvas current extent width : " << mCanvasProperties->currentExtent.width()  << std::endl
        << "muppY: " << muppY << std::endl 
        << "muppX: " << muppX << std::endl 
        << "dxmin: " << dxmin << std::endl 
        << "dxmax: " << dxmax << std::endl 
        << "dymin: " << dymin << std::endl 
        << "dymax: " << dymax << std::endl 
        << "whitespace: " << whitespace << std::endl;
#endif
      mCanvasProperties->coordXForm->setParameters(mCanvasProperties->m_mupp, dxmin, dymin, myHeight);  
      //currentExtent.xMin(),      currentExtent.yMin(), currentExtent.yMax());
      // update the currentExtent to match the device coordinates
      //GS - removed the current extent update to fix bug --
      //TODO remove the next 4 lines after we're sure this works ok

      // TS - Update : We want these 4 lines because the device coordinates may be
      // the printing device or something unrelated to the actual mapcanvas pixmap.
      // However commenting it it out causes map not to scale when QGIS  window is resized

      mCanvasProperties->currentExtent.setXmin(dxmin);
      mCanvasProperties->currentExtent.setXmax(dxmax);
      mCanvasProperties->currentExtent.setYmin(dymin);
      mCanvasProperties->currentExtent.setYmax(dymax);
      int myRenderCounter=1;
      // render all layers in the stack, starting at the base
      std::list < QString >::iterator li = mCanvasProperties->zOrder.begin();
      // std::cout << "MAP LAYER COUNT: " << layers.size() << std::endl;
      while (li != mCanvasProperties->zOrder.end())
      {
        emit setProgress(myRenderCounter++,mCanvasProperties->zOrder.size()*2);
        QgsMapLayer *ml = mCanvasProperties->layers[*li];

        if (ml)
        {
          //    QgsDatabaseLayer *dbl = (QgsDatabaseLayer *)&ml;
#ifdef QGISDEBUG
          std::cout << "Rendering " << ml->name() << std::endl;
#endif
          if (ml->visible())
          {
            ml->draw(paint, &mCanvasProperties->currentExtent, mCanvasProperties->coordXForm, this);
          }

          li++;
        }
      }
#ifdef QGISDEBUG
      std::cout << "Done rendering map layers...emitting renderComplete(paint)\n";
#endif
      
      // render all labels for vector layers in the stack, starting at the base
      li = mCanvasProperties->zOrder.begin();
      // std::cout << "MAP LAYER COUNT: " << layers.size() << std::endl;
      while (li != mCanvasProperties->zOrder.end())
      {
        emit setProgress((myRenderCounter++)*2,mCanvasProperties->zOrder.size()*2);
        QgsMapLayer *ml = mCanvasProperties->layers[*li];

        if (ml)
        {
#ifdef QGISDEBUG
          std::cout << "Rendering " << ml->name() << std::endl;
#endif
          if (ml->visible() && (ml->type() != QgsMapLayer::RASTER))
          {
            ml->drawLabels(paint, &mCanvasProperties->currentExtent, mCanvasProperties->coordXForm, this);
          }

          li++;
        }
      }

#ifdef QGISDEBUG
      std::cout << "Done rendering map labels...emitting renderComplete(paint)\n";
#endif
      emit renderComplete(paint);
      // draw the acetate layer
      std::map <QString, QgsAcetateObject *>::iterator ai = mCanvasProperties->acetateObjects.begin();
      while(ai != mCanvasProperties->acetateObjects.end())
      {
        QgsAcetateObject *acObj = ai->second;
        if(acObj)
        {
          acObj->draw(paint, mCanvasProperties->coordXForm);
        }
        ai++;
      }

      // notify any listeners that rendering is complete
      //note that pmCanvas is not draw to gui yet
      //emit renderComplete(paint);

      paint->end();
      mCanvasProperties->drawing = false;
    }
    mCanvasProperties->dirty = false;
    repaint();
  }

} // render

// return the current coordinate transform based on the extents and
// device size
QgsCoordinateTransform * QgsMapCanvas::getCoordinateTransform()
{
  return mCanvasProperties->coordXForm;
}

void QgsMapCanvas::currentScale(int thePrecision)
{
  // calculate the translation and scaling parameters
  double muppX, muppY;
  muppY = mCanvasProperties->currentExtent.height() / height();
  muppX = mCanvasProperties->currentExtent.width() / width();
  mCanvasProperties->m_mupp = muppY > muppX ? muppY : muppX;
#ifdef QGISDEBUG
  std::cout << "------------------------------------------ " << std::endl;
  std::cout << "----------   Current Scale --------------- " << std::endl;
  std::cout << "------------------------------------------ " << std::endl;

  std::cout << "Current extent is " <<
    mCanvasProperties->currentExtent.stringRep() << std::endl; std::cout << "MuppX is: " <<
    muppX << "\nMuppY is: " << muppY << std::endl;
  std::cout << "Canvas width: " << width() << ", height: " << height() << std::endl;
  std::cout << "Extent width: " << mCanvasProperties->currentExtent.width() << ", height: "
    << mCanvasProperties->currentExtent.height() << std::endl;

  QPaintDeviceMetrics pdm(this);
  std::cout << "dpiX " << pdm.logicalDpiX() << ", dpiY " <<
    pdm.logicalDpiY() << std::endl;
  std::cout << "widthMM " << pdm.widthMM() << ", heightMM "
    << pdm.heightMM() << std::endl;

#endif
  // calculate the actual extent of the mapCanvas
  double dxmin, dxmax, dymin, dymax, whitespace;
  if (muppY > muppX)
  {
    dymin = mCanvasProperties->currentExtent.yMin();
    dymax = mCanvasProperties->currentExtent.yMax();
    whitespace = ((width() * mCanvasProperties->m_mupp) - mCanvasProperties->currentExtent.width()) / 2;
    dxmin = mCanvasProperties->currentExtent.xMin() - whitespace;
    dxmax = mCanvasProperties->currentExtent.xMax() + whitespace;
  } else
  {
    dxmin = mCanvasProperties->currentExtent.xMin();
    dxmax = mCanvasProperties->currentExtent.xMax();
    whitespace = ((height() * mCanvasProperties->m_mupp) - mCanvasProperties->currentExtent.height()) / 2;
    dymin = mCanvasProperties->currentExtent.yMin() - whitespace;
    dymax = mCanvasProperties->currentExtent.yMax() + whitespace;

  }
  QgsRect paddedExtent(dxmin, dymin, dxmax, dymax);
  mCanvasProperties->mScale = mCanvasProperties->scaleCalculator->calculate(paddedExtent, width());
#ifdef QGISDEBUG
  std::cout << "Scale (assuming meters as map units) = 1:"
    << mCanvasProperties->mScale << std::endl;
#endif
  // return scale based on geographic
  //

  //@todo return a proper value
  QString myScaleString = QString("Scale 1: ")+QString::number(mCanvasProperties->mScale,'f',thePrecision);
  emit scaleChanged(myScaleString) ;
#ifdef QGISDEBUG
  std::cout << "------------------------------------------ " << std::endl;
#endif
}



//the format defaults to "PNG" if not specified
void QgsMapCanvas::saveAsImage(QString theFileName, QPixmap * theQPixmap, QString theFormat)
{
  //
  //check if the optional QPaintDevice was supplied
  //
  if (theQPixmap != NULL)
  {
    render(theQPixmap);
    theQPixmap->save(theFileName,theFormat);
  }
  else //use the map view
  {
    mCanvasProperties->pmCanvas->save(theFileName,theFormat);
  }
} // saveAsImage



void QgsMapCanvas::paintEvent(QPaintEvent * ev)
{
  if (!mCanvasProperties->dirty)
  {
    // just bit blit the image to the canvas
    bitBlt(this, ev->rect().topLeft(), mCanvasProperties->pmCanvas, ev->rect());
  } else
  {
    if (!mCanvasProperties->drawing)
    {
      render();
    }
  }
} // paintEvent



QgsRect const & QgsMapCanvas::extent() const
{
  return mCanvasProperties->currentExtent;
} // extent

QgsRect const & QgsMapCanvas::fullExtent() const
{
  return mCanvasProperties->fullExtent;
} // extent

void QgsMapCanvas::setExtent(QgsRect const & r)
{
  mCanvasProperties->currentExtent = r;
  emit extentsChanged(r);
} // setExtent


void QgsMapCanvas::clear()
{
  mCanvasProperties->dirty = true;
  erase();
} // clear



void QgsMapCanvas::zoomFullExtent()
{
  mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
  mCanvasProperties->currentExtent = mCanvasProperties->fullExtent;

  clear();
  render();
  emit extentsChanged(mCanvasProperties->currentExtent);
} // zoomFullExtent



void QgsMapCanvas::zoomPreviousExtent()
{
  if (mCanvasProperties->previousExtent.width() > 0)
  {
    QgsRect tempRect = mCanvasProperties->currentExtent;
    mCanvasProperties->currentExtent = mCanvasProperties->previousExtent;
    mCanvasProperties->previousExtent = tempRect;
    clear();
    render();
    emit extentsChanged(mCanvasProperties->currentExtent);
  }
} // zoomPreviousExtent



void QgsMapCanvas::zoomToSelected()
{
  QgsVectorLayer *lyr =
    dynamic_cast < QgsVectorLayer * >(mCanvasProperties->mapLegend->currentLayer());

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
      mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
      mCanvasProperties->currentExtent.setXmin(rect.xMin() - 25);
      mCanvasProperties->currentExtent.setYmin(rect.yMin() - 25);
      mCanvasProperties->currentExtent.setXmax(rect.xMax() + 25);
      mCanvasProperties->currentExtent.setYmax(rect.yMax() + 25);
      emit extentsChanged(mCanvasProperties->currentExtent);
      clear();
      render();
      return;
    }
    //zoom to an area
    else
    {
      mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
      mCanvasProperties->currentExtent.setXmin(rect.xMin());
      mCanvasProperties->currentExtent.setYmin(rect.yMin());
      mCanvasProperties->currentExtent.setXmax(rect.xMax());
      mCanvasProperties->currentExtent.setYmax(rect.yMax());
      emit extentsChanged(mCanvasProperties->currentExtent);
      clear();
      render();
      return;
    }
  }
} // zoomToSelected



void QgsMapCanvas::mousePressEvent(QMouseEvent * e)
{
  if (!mUserInteractionAllowed) return;
  mCanvasProperties->mouseButtonDown = true;
  mCanvasProperties->boxStartPoint = e->pos();

  switch (mCanvasProperties->mapTool)
  {
    case QGis::Select:
    case QGis::ZoomIn:
    case QGis::ZoomOut:
      mCanvasProperties->zoomBox.setRect(0, 0, 0, 0);
      break;
    case QGis::Distance:
      //              distanceEndPoint = e->pos();
      break;
  }
} // mousePressEvent


void QgsMapCanvas::mouseReleaseEvent(QMouseEvent * e)
{
  if (!mUserInteractionAllowed) return;
  QPainter paint;
  QPen     pen(Qt::gray);
  QgsPoint ll, ur;

  if (mCanvasProperties->dragging)
  {
    mCanvasProperties->dragging = false;

    switch (mCanvasProperties->mapTool)
    {
      case QGis::ZoomIn:
        // erase the rubber band box
        paint.begin(this);
        paint.setPen(pen);
        paint.setRasterOp(Qt::XorROP);
        paint.drawRect(mCanvasProperties->zoomBox);
        paint.end();
        // store the rectangle
        mCanvasProperties->zoomBox.setRight(e->pos().x());
        mCanvasProperties->zoomBox.setBottom(e->pos().y());
        // set the extent to the zoomBox

        ll = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.left(), mCanvasProperties->zoomBox.bottom());
        ur = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.right(), mCanvasProperties->zoomBox.top());
        mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
        //QgsRect newExtent(ll.x(), ll.y(), ur.x(), ur.y());
        mCanvasProperties->currentExtent.setXmin(ll.x());
        mCanvasProperties->currentExtent.setYmin(ll.y());
        mCanvasProperties->currentExtent.setXmax(ur.x());
        mCanvasProperties->currentExtent.setYmax(ur.y());
        mCanvasProperties->currentExtent.normalize();
        clear();
        render();
        emit extentsChanged(mCanvasProperties->currentExtent);

        break;
      case QGis::ZoomOut:
        {
          // erase the rubber band box
          paint.begin(this);
          paint.setPen(pen);
          paint.setRasterOp(Qt::XorROP);
          paint.drawRect(mCanvasProperties->zoomBox);
          paint.end();
          // store the rectangle
          mCanvasProperties->zoomBox.setRight(e->pos().x());
          mCanvasProperties->zoomBox.setBottom(e->pos().y());
          // scale the extent so the current view fits inside the zoomBox
          ll = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.left(), mCanvasProperties->zoomBox.bottom());
          ur = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.right(), mCanvasProperties->zoomBox.top());
          mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
          QgsRect tempRect = mCanvasProperties->currentExtent;
          mCanvasProperties->currentExtent.setXmin(ll.x());
          mCanvasProperties->currentExtent.setYmin(ll.y());
          mCanvasProperties->currentExtent.setXmax(ur.x());
          mCanvasProperties->currentExtent.setYmax(ur.y());
          mCanvasProperties->currentExtent.normalize();

          QgsPoint cer = mCanvasProperties->currentExtent.center();

          /* std::cout << "Current extent rectangle is " << tempRect << std::endl;
             std::cout << "Center of zoom out rectangle is " << cer << std::endl;
             std::cout << "Zoom out rectangle should have ll of " << ll << " and ur of " << ur << std::endl;
             std::cout << "Zoom out rectangle is " << mCanvasProperties->currentExtent << std::endl;
             */
          double sf;
          if (mCanvasProperties->zoomBox.width() > mCanvasProperties->zoomBox.height())
          {
            sf = tempRect.width() / mCanvasProperties->currentExtent.width();
          } else
          {
            sf = tempRect.height() / mCanvasProperties->currentExtent.height();
          }
          //center = new QgsPoint(zoomRect->center());
          //  delete zoomRect;
          mCanvasProperties->currentExtent.expand(sf);
#ifdef QGISDEBUG
          std::cout << "Extent scaled by " << sf << " to " << mCanvasProperties->currentExtent << std::endl;
          std::cout << "Center of currentExtent after scaling is " << mCanvasProperties->currentExtent.center() << std::endl;
#endif
          clear();
          render();
          emit extentsChanged(mCanvasProperties->currentExtent);
        }
        break;

      case QGis::Pan:
        {
          // use start and end box points to calculate the extent
          QgsPoint start = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->boxStartPoint);
          QgsPoint end = mCanvasProperties->coordXForm->toMapCoordinates(e->pos());

          double dx = fabs(end.x() - start.x());
          double dy = fabs(end.y() - start.y());

          // modify the extent
          mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;

          if (end.x() < start.x())
          {
            mCanvasProperties->currentExtent.setXmin(mCanvasProperties->currentExtent.xMin() + dx);
            mCanvasProperties->currentExtent.setXmax(mCanvasProperties->currentExtent.xMax() + dx);
          } else
          {
            mCanvasProperties->currentExtent.setXmin(mCanvasProperties->currentExtent.xMin() - dx);
            mCanvasProperties->currentExtent.setXmax(mCanvasProperties->currentExtent.xMax() - dx);
          }

          if (end.y() < start.y())
          {
            mCanvasProperties->currentExtent.setYmax(mCanvasProperties->currentExtent.yMax() + dy);
            mCanvasProperties->currentExtent.setYmin(mCanvasProperties->currentExtent.yMin() + dy);

          } else
          {
            mCanvasProperties->currentExtent.setYmax(mCanvasProperties->currentExtent.yMax() - dy);
            mCanvasProperties->currentExtent.setYmin(mCanvasProperties->currentExtent.yMin() - dy);

          }
          clear();
          render();
          emit extentsChanged(mCanvasProperties->currentExtent);
        }
        break;

      case QGis::Select:
        // erase the rubber band box
        paint.begin(this);
        paint.setPen(pen);
        paint.setRasterOp(Qt::XorROP);
        paint.drawRect(mCanvasProperties->zoomBox);
        paint.end();

        QgsMapLayer *lyr = mCanvasProperties->mapLegend->currentLayer();

        if (lyr)
        {
          QgsPoint ll, ur;

          // store the rectangle
          mCanvasProperties->zoomBox.setRight(e->pos().x());
          mCanvasProperties->zoomBox.setBottom(e->pos().y());

          ll = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.left(), mCanvasProperties->zoomBox.bottom());
          ur = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.right(), mCanvasProperties->zoomBox.top());

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
    switch (mCanvasProperties->mapTool)
    {
      case QGis::Identify:
        {
          // call identify method for selected layer
          QgsMapLayer * lyr = mCanvasProperties->mapLegend->currentLayer();

          if (lyr)
          {

            // create the search rectangle
            double searchRadius = extent().width() * calculateSearchRadiusValue();
            QgsRect * search = new QgsRect;
            // convert screen coordinates to map coordinates
            QgsPoint idPoint = mCanvasProperties->coordXForm->toMapCoordinates(e->x(), e->y());
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
        }
        break;

      case QGis::CapturePoint:
        {
          QgsPoint  idPoint = mCanvasProperties->coordXForm->toMapCoordinates(e->x(), e->y());
          emit xyClickCoordinates(idPoint);

          QgsVectorLayer* vlayer=dynamic_cast<QgsVectorLayer*>(mCanvasProperties->mapLegend->currentLayer());

          if(vlayer)
          {
            QgsFeature f(0,"WKBPoint");
            int size=5+2*sizeof(double);
            unsigned char *wkb = new unsigned char[size];
            int wkbtype=QGis::WKBPoint;
            double x=idPoint.x();
            double y=idPoint.y();
            memcpy(&wkb[1],&wkbtype, sizeof(int));
            memcpy(&wkb[5], &x, sizeof(double));
            memcpy(&wkb[5]+sizeof(double), &y, sizeof(double));
            f.setGeometry(&wkb[0],size);
            // also need to store the well known text so feature
            // can be inserted into postgis layer if applicable.
            // We set the geometry but not the SRID. The provider
            // will add the SRID when setting the WKT
            QString wkt = idPoint.wellKnownText();
            f.setWellKnownText(wkt);
            vlayer->addFeature(&f);
            refresh();
          }
          else
          {
            //not a vectorlayer
          }

          //add the feature to the active layer
#ifdef QGISDEBUG
          std::cout << "CapturePoint : " << idPoint.x() << "," << idPoint.y() << std::endl;
#endif
        }
        break;

      case QGis::CaptureLine:
      case QGis::CapturePolygon:

        mCaptureList.push_back(mCanvasProperties->coordXForm->toMapCoordinates(e->x(), e->y()));
        if(mCaptureList.size()>1)
        {
          QPainter paint(this);
          paint.setPen(QPen(QColor(255,0,0),4,Qt::DashLine));
          std::list<QgsPoint>::iterator it=mCaptureList.end();
          --it;
          --it;
          QgsPoint lastpoint = mCanvasProperties->coordXForm->transform(it->x(),it->y());
          paint.drawLine(lastpoint.x(),lastpoint.y(),e->x(),e->y());
        }
        if(e->button()==Qt::RightButton)
        {
          QgsVectorLayer* vlayer=dynamic_cast<QgsVectorLayer*>(mCanvasProperties->mapLegend->currentLayer());
          if(vlayer)
          {
            //create QgsFeature with wkb representation
            QgsFeature f(0,"WKBLineString");
            unsigned char* wkb;
            int size;
            if(mCanvasProperties->mapTool==QGis::CaptureLine)
            {
              size=1+2*sizeof(int)+2*mCaptureList.size()*sizeof(double);
              wkb= new unsigned char[size];
              int wkbtype=QGis::WKBLineString;
              int length=mCaptureList.size();
              memcpy(&wkb[1],&wkbtype, sizeof(int));
              memcpy(&wkb[5],&length, sizeof(int));
              int position=1+2*sizeof(int);
              double x,y;
              for(std::list<QgsPoint>::iterator it=mCaptureList.begin();it!=mCaptureList.end();++it)
              {
                x=it->x();
                memcpy(&wkb[position],&x,sizeof(double));
                position+=sizeof(double);
                y=it->y();
                memcpy(&wkb[position],&y,sizeof(double));
                position+=sizeof(double);
              }
            }
            else//polygon
            {
              size=1+3*sizeof(int)+2*mCaptureList.size()*sizeof(double);
              wkb= new unsigned char[size];
              int wkbtype=QGis::WKBPolygon;
              int length=mCaptureList.size();
              int numrings=1;
              memcpy(&wkb[1],&wkbtype, sizeof(int));
              memcpy(&wkb[5],&numrings,sizeof(int));
              memcpy(&wkb[9],&length, sizeof(int));
              int position=1+3*sizeof(int);
              double x,y;
              for(std::list<QgsPoint>::iterator it=mCaptureList.begin();it!=mCaptureList.end();++it)
              {
                x=it->x();
                memcpy(&wkb[position],&x,sizeof(double));
                position+=sizeof(double);
                y=it->y();
                memcpy(&wkb[position],&y,sizeof(double));
                position+=sizeof(double);
              } 
            }
            f.setGeometry(&wkb[0],size);
            vlayer->addFeature(&f);
            mCaptureList.clear();
            refresh();
          }
          else
          {
            //not a vector layer
          }
        }
        break;
    }
  }
} // mouseReleaseEvent



void QgsMapCanvas::resizeEvent(QResizeEvent * e)
{
  mCanvasProperties->dirty = true;
  mCanvasProperties->pmCanvas->resize(e->size());
  emit extentsChanged(mCanvasProperties->currentExtent);
} // resizeEvent

void QgsMapCanvas::wheelEvent(QWheelEvent *e)
{
  // Zoom the map canvas in response to a mouse wheel event. Moving the
  // wheel forward (away) from the user zooms in by a factor of 2. 
  // TODO The scale factor needs to be customizable by the user.
#ifdef QGISDEBUG
  std::cout << "Wheel event delta " << e->delta() << std::endl;
#endif
  // change extent
  double scaleFactor;
  if(e->delta() > 0){
    scaleFactor = .5;
  }else{
    scaleFactor = 2;
  }
  // transform the mouse pos to map coordinates
  QgsPoint center  = mCanvasProperties->coordXForm->toMapPoint(e->x(), e->y());
  mCanvasProperties->currentExtent.scale(scaleFactor,&center );
  clear();
  render();
  emit extentsChanged(mCanvasProperties->currentExtent);


}

void QgsMapCanvas::mouseMoveEvent(QMouseEvent * e)
{
  if (!mUserInteractionAllowed) return;
  // XXX magic numbers BAD -- 513?
  if (e->state() == Qt::LeftButton || e->state() == 513)
  {
    int dx, dy;
    QPainter paint;
    QPen pen(Qt::gray);

    // this is a drag-type operation (zoom, pan or other maptool)

    switch (mCanvasProperties->mapTool)
    {
      case QGis::Select:
      case QGis::ZoomIn:
      case QGis::ZoomOut:
        // draw the rubber band box as the user drags the mouse
        mCanvasProperties->dragging = true;

        paint.begin(this);
        paint.setPen(pen);
        paint.setRasterOp(Qt::XorROP);
        paint.drawRect(mCanvasProperties->zoomBox);

        mCanvasProperties->zoomBox.setLeft(mCanvasProperties->boxStartPoint.x());
        mCanvasProperties->zoomBox.setTop(mCanvasProperties->boxStartPoint.y());
        mCanvasProperties->zoomBox.setRight(e->pos().x());
        mCanvasProperties->zoomBox.setBottom(e->pos().y());

        paint.drawRect(mCanvasProperties->zoomBox);
        paint.end();
        break;

      case QGis::Pan:
        // show the pmCanvas as the user drags the mouse
        mCanvasProperties->dragging = true;
        // bitBlt the pixmap on the screen, offset by the
        // change in mouse coordinates
        dx = e->pos().x() - mCanvasProperties->boxStartPoint.x();
        dy = e->pos().y() - mCanvasProperties->boxStartPoint.y();

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

        bitBlt(this, dx, dy, mCanvasProperties->pmCanvas);
        break;
    }

  }

  // show x y on status bar
  QPoint xy = e->pos();
  QgsPoint coord = mCanvasProperties->coordXForm->toMapCoordinates(xy);
  emit xyCoordinates(coord);

} // mouseMoveEvent


/** Sets the map tool currently being used on the canvas */
void QgsMapCanvas::setMapTool(int tool)
{
  mCanvasProperties->mapTool = tool;
  mCaptureList.clear();
} // setMapTool


/** Write property of QColor bgColor. */
void QgsMapCanvas::setbgColor(const QColor & _newVal)
{
  mCanvasProperties->bgColor = _newVal;
  setEraseColor(_newVal);
} // setbgColor



/** Updates the full extent to include the mbr of the rectangle r */
void QgsMapCanvas::updateFullExtent(QgsRect const & r)
{
  if (r.xMin() < mCanvasProperties->fullExtent.xMin())
  {
    mCanvasProperties->fullExtent.setXmin(r.xMin());
  }

  if (r.xMax() > mCanvasProperties->fullExtent.xMax())
  {
    mCanvasProperties->fullExtent.setXmax(r.xMax());
  }

  if (r.yMin() < mCanvasProperties->fullExtent.yMin())
  {
    mCanvasProperties->fullExtent.setYmin(r.yMin());
  }

  if (r.yMax() > mCanvasProperties->fullExtent.yMax())
  {
    mCanvasProperties->fullExtent.setYmax(r.yMax());
  }

  emit extentsChanged(mCanvasProperties->currentExtent);
} // updateFullExtent



/*const std::map<QString,QgsMapLayer *> * QgsMapCanvas::mapLayers(){
  return &layers;
  }
  */
int QgsMapCanvas::layerCount() const
{
  return mCanvasProperties->layers.size();
} // layerCount



void QgsMapCanvas::layerStateChange()
{
  if (!mCanvasProperties->frozen)
  {
    clear();
    render();
  }
} // layerStateChange



void QgsMapCanvas::freeze(bool frz)
{
  mCanvasProperties->frozen = frz;
} // freeze

bool QgsMapCanvas::isFrozen()
{
  return mCanvasProperties->frozen ;
} // freeze


void QgsMapCanvas::remove(QString key)
{
  // XXX As a safety precaution, shouldn't we check to see if the 'key' is
  // XXX in 'layers'?  Theoretically it should be ok to skip this check since
  // XXX this should always be called with correct keys.

  //We no longer delete the layer her - deletiong of layers is now managed
  //by the MapLayerRegistry. All we do now is remove any local reference to this layer.
  //delete mCanvasProperties->layers[key];   // first delete the map layer itself

  mCanvasProperties->layers[key] = 0;
  mCanvasProperties->layers.erase( key );  // then erase its entry from layer table

  // XXX after removing this layer, we should probably compact the
  // XXX remaining Z values
  mCanvasProperties->zOrder.remove(key);   // ... and it's Z order entry, too

  // Since we removed a layer, we may have to adjust the map canvas'
  // over-all extent; so we update to the largest extent found in the
  // remaining layers.

  if ( mCanvasProperties->layers.empty() )
  {
    // XXX do we want to reset the extents if the last layer is deleted?
  }
  else
  {
    std::map < QString, QgsMapLayer * >::iterator mi =
      mCanvasProperties->layers.begin();

    mCanvasProperties->fullExtent = mi->second->extent();
    mCanvasProperties->fullExtent.scale(1.1); // XXX why set the scale to this magic
    // XXX number?

    ++mi;

    for ( ; mi != mCanvasProperties->layers.end(); ++mi )
    {
      updateFullExtent(mi->second->extent());
    }
  }

  mCanvasProperties->dirty = true;

  // signal that we've erased this layer
  emit removedLayer( key );
}



void QgsMapCanvas::removeAll()
{

  // Welllllll, yeah, this works, but now we have to ensure that the
  // removedLayer() signal is emitted.
  //   mCanvasProperties->layers.clear();
  //   mCanvasProperties->zOrder.clear();

  // So:
  std::map < QString, QgsMapLayer * >::iterator mi =
    mCanvasProperties->layers.begin();

  QString current_key;

  while ( mi != mCanvasProperties->layers.end() )
  {
    // save the current key
    current_key = mi->first;

    // delete it, ensuring removedLayer emitted and zOrder updated (not
    // that the zOrder ultimately matters since they're all going to go,
    // too)
    remove( current_key );

    // since mi is now invalidated because the std::map was modified in
    // remove(), reset it to the first element, if any
    mi = mCanvasProperties->layers.begin();
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
  return mCanvasProperties->pmCanvas;
} // canvasPixmap



void QgsMapCanvas::setCanvasPixmap(QPixmap * theQPixmap)
{
  mCanvasProperties->pmCanvas = theQPixmap;
} // setCanvasPixmap


void QgsMapCanvas::setZOrder(std::list <QString> theZOrder)
{
  //
  // We need to evaluate each layer in the zOrder and see
  // if it is actually a member of this mapCanvas
  // 
  std::list < QString >::iterator li = theZOrder.begin();
  mCanvasProperties->zOrder.clear(); 
  while (li != theZOrder.end())
  {
    QgsMapLayer *ml = mCanvasProperties->layers[*li];

    if (ml)
    {
#ifdef QGISDEBUG
      std::cout << "Adding  " << ml->name() << " to zOrder" << std::endl;
#endif
      mCanvasProperties->zOrder.push_back(ml->getLayerID());
    }
    else
    {
#ifdef QGISDEBUG
      std::cout << "Cant add  " << ml->name() << " to zOrder (it isnt in layers array)" << std::endl;
#endif
    }
    li++;
  }
}

std::list < QString > const & QgsMapCanvas::zOrders() const
{
  return mCanvasProperties->zOrder;
} // zOrders


std::list < QString >       & QgsMapCanvas::zOrders()
{
  return mCanvasProperties->zOrder;
} // zOrders

//! determines whether the user can interact with the overview canvas.
void QgsMapCanvas::userInteractionAllowed(bool theFlag)
{
  mUserInteractionAllowed = theFlag;
}
//! determines whether the user can interact with the overview canvas.
bool QgsMapCanvas::isUserInteractionAllowed()
{
  return mUserInteractionAllowed;
}


double QgsMapCanvas::mupp() const
{
  return mCanvasProperties->m_mupp;
} // mupp
void QgsMapCanvas::setMapUnits(int units)
{
#ifdef QGISDEBUG
  std::cerr << "Setting map units to " << units << std::endl;
#endif
  mCanvasProperties->setMapUnits(units);
}
int QgsMapCanvas::mapUnits()
{
  return mCanvasProperties->mapUnits();
}
