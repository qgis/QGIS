/***************************************************************************
    qgsmaprender.cpp  -  class for rendering map layer set
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <cmath>

#include "qgsmaprender.h"
#include "qgsscalecalculator.h"
#include "qgsmaptopixel.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"

#include <QPixmap>
#include <QPainter>
#include <Q3PaintDeviceMetrics>
#include <QTime>


QgsMapRender::QgsMapRender()
{
  mCoordXForm = new QgsMapToPixel;
  mScaleCalculator = new QgsScaleCalculator;
    
  mDrawing = false;
  mOverview = false;
  
  // set default map units
  mMapUnits = QGis::METERS;
  mScaleCalculator->setMapUnits(mMapUnits);

  mSize = QSize(0,0);
}

QgsMapRender::~QgsMapRender()
{
  delete mCoordXForm;
  delete mScaleCalculator;
}


QgsRect QgsMapRender::extent()
{
  return mExtent;
}


bool QgsMapRender::setExtent(const QgsRect& extent)
{

  // Don't allow zooms where the current extent is so small that it
  // can't be accurately represented using a double (which is what
  // currentExtent uses). Excluding 0 avoids a divide by zero and an
  // infinite loop when rendering to a new canvas. Excluding extents
  // greater than 1 avoids doing unnecessary calculations.

  // The scheme is to compare the width against the mean x coordinate
  // (and height against mean y coordinate) and only allow zooms where
  // the ratio indicates that there is more than about 12 significant
  // figures (there are about 16 significant figures in a double).

  if (extent.width()  > 0 && 
      extent.height() > 0 && 
      extent.width()  < 1 &&
      extent.height() < 1)
  {
    // Use abs() on the extent to avoid the case where the extent is
    // symmetrical about 0.
    double xMean = (fabs(extent.xMin()) + fabs(extent.xMax())) * 0.5;
    double yMean = (fabs(extent.yMin()) + fabs(extent.yMax())) * 0.5;

    double xRange = extent.width() / xMean;
    double yRange = extent.height() / yMean;

    static const double minProportion = 1e-12;
    if (xRange < minProportion || yRange < minProportion)
      return false;
  }

  mExtent = extent;
  if (!mExtent.isEmpty())
    adjustExtentToSize();
  return true;
}



void QgsMapRender::setOutputSize(QSize size, int dpi)
{
  mSize = size;
  mScaleCalculator->setDpi(dpi);
  adjustExtentToSize();
}

void QgsMapRender::adjustExtentToSize()
{
  int myHeight = mSize.height();
  int myWidth = mSize.width();
  
  if (!myWidth || !myHeight)
  {
    mScale = 1;
    mCoordXForm->setParameters(0, 0, 0, 0);
    return;
  }

  // calculate the translation and scaling parameters
  // mupp = map units per pixel
  double muppY = mExtent.height() / myHeight;
  double muppX = mExtent.width() / myWidth;
  mMupp = muppY > muppX ? muppY : muppX;

  // calculate the actual extent of the mapCanvas
  double dxmin, dxmax, dymin, dymax, whitespace;

  if (muppY > muppX)
  {
    dymin = mExtent.yMin();
    dymax = mExtent.yMax();
    whitespace = ((myWidth * mMupp) - mExtent.width()) / 2;
    dxmin = mExtent.xMin() - whitespace;
    dxmax = mExtent.xMax() + whitespace;
  }
  else
  {
    dxmin = mExtent.xMin();
    dxmax = mExtent.xMax();
    whitespace = ((myHeight * mMupp) - mExtent.height()) / 2;
    dymin = mExtent.yMin() - whitespace;
    dymax = mExtent.yMax() + whitespace;
  }

#ifdef QGISDEBUG
  std::cout << "========== Current Scale ==========" << std::endl;
  std::cout << "Current extent is " << mExtent.stringRep().toLocal8Bit().data() << std::endl;
  std::cout << "MuppX is: " << muppX << "\n" << "MuppY is: " << muppY << std::endl;
  std::cout << "Pixmap width: " << myWidth << ", height: " << myHeight << std::endl;
  std::cout << "Extent width: " << mExtent.width() << ", height: " << mExtent.height() << std::endl;
  std::cout << "whitespace: " << whitespace << std::endl;
#endif

  // update extent
  mExtent.setXmin(dxmin);
  mExtent.setXmax(dxmax);
  mExtent.setYmin(dymin);
  mExtent.setYmax(dymax);
  
  // update the scale
  mScale = mScaleCalculator->calculate(mExtent, myWidth);

#ifdef QGISDEBUG
  std::cout << "Scale (assuming meters as map units) = 1:" << mScale << std::endl;
  std::cout << "------------------------------------------ " << std::endl;
#endif

  mCoordXForm->setParameters(mMupp, dxmin, dymin, myHeight);
}


void QgsMapRender::render(QPainter* painter)
{

#ifdef QGISDEBUG
  std::cout << "========== Rendering ==========" << std::endl;
#endif

  if (mExtent.isEmpty())
  {
    std::cout << "empty extent... not rendering" << endl;
    return;
  }


  if (mDrawing)
    return;
  
  mDrawing = true;
  
  int myRenderCounter = 0;
  
#ifdef QGISDEBUG
  std::cout << "QgsMapRender::render: Starting to render layer stack." << std::endl;
  QTime renderTime;
  renderTime.start();
#endif
  // render all layers in the stack, starting at the base
  std::deque<QString> layers = mLayers.layerSet();
  std::deque<QString>::iterator li = layers.begin();
  
  while (li != layers.end())
  {
#ifdef QGISDEBUG
    std::cout << "QgsMapRender::render: at layer item '" << (*li).toLocal8Bit().data() << "'." << std::endl;
#endif

    emit setProgress(myRenderCounter++,layers.size());
    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer(*li);

    if (!ml)
    {
#ifdef QGISDEBUG
      std::cout << "QgsMapRender::render: layer not found in registry!" << std::endl;
#endif
      li++;
      continue;
    }
        
#ifdef QGISDEBUG
    std::cout << "QgsMapRender::render: Rendering layer " << ml->name().toLocal8Bit().data() << '\n'
      << "  Layer minscale " << ml->minScale() 
      << ", maxscale " << ml->maxScale() << '\n' 
      << "  Scale dep. visibility enabled? " 
      << ml->scaleBasedVisibility() << '\n'
      << "  Input extent: " << ml->extent().stringRep().toLocal8Bit().data() 
      << std::endl;
    try
    {
      std::cout << "  Transformed extent: " 
          << ml->coordinateTransform()->transformBoundingBox(ml->extent()).stringRep().toLocal8Bit().data() 
          << std::endl;
    }
    catch (QgsCsException &cse)
    {
      qDebug( "Transform error caught in %s line %d:\n%s", __FILE__, __LINE__, cse.what());
    }
#endif

    if ((!mOverview && ml->visible()) || (mOverview && ml->showInOverviewStatus()))
    {
      if ((ml->scaleBasedVisibility() && ml->minScale() < mScale && ml->maxScale() > mScale)
          || (!ml->scaleBasedVisibility()))
      {
        connect(ml, SIGNAL(drawingProgress(int,int)), this, SLOT(onDrawingProgress(int,int)));        
        
        QgsRect r1 = mExtent, r2;
        bool split = ml->projectExtent(r1, r2);
        //
                    // Now do the call to the layer that actually does
                    // the rendering work!
        //
        if (!ml->draw(painter, &r1, mCoordXForm))
          emit drawError(ml);
        
        if (split)
        {
          if (!ml->draw(painter, &r2, mCoordXForm))
            emit drawError(ml);
        }
        
        disconnect(ml, SIGNAL(drawingProgress(int,int)), this, SLOT(onDrawingProgress(int,int)));
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "QgsMapRender::render: Layer not rendered because it is not within "
            << "the defined visibility scale range" << std::endl;
#endif
      }
        
    } // if (ml->visible())
    
    li++;
    
  } // while (li != end)
      
#ifdef QGISDEBUG
  std::cout << "QgsMapRender::render: Done rendering map layers" << std::endl;
#endif

  if (!mOverview)
  {
    // render all labels for vector layers in the stack, starting at the base
    li = layers.begin();
    while (li != layers.end())
    {
      // TODO: emit setProgress((myRenderCounter++),zOrder.size());
      QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer(*li);
  
      if (ml && ml->visible() && (ml->type() != QgsMapLayer::RASTER))
      {
        // only make labels if the layer is visible
        // after scale dep viewing settings are checked
        if ((ml->scaleBasedVisibility() && ml->minScale() < mScale  && ml->maxScale() > mScale)
            || (!ml->scaleBasedVisibility()))
        {
          QgsRect r1 = mExtent, r2;
          bool split = ml->projectExtent(r1, r2);
      
          ml->drawLabels(painter, &r1, mCoordXForm);
          if (split)
            ml->drawLabels(painter, &r2, mCoordXForm);
        }
      }
      li++;
    }
  } // if (!mOverview)

  // make sure progress bar arrives at 100%!
  emit setProgress(1,1);      
      
#ifdef QGISDEBUG
  std::cout << "QgsMapRender::render: Rendering done in " <<
               renderTime.elapsed() / 1000.0 << " seconds" << std::endl;
#endif

  mDrawing = false;

}

void QgsMapRender::setMapUnits(QGis::units u)
{
  mMapUnits = u;
  mScaleCalculator->setMapUnits(mMapUnits);
  QgsProject::instance()->mapUnits(u); // TODO: sort out
}

void QgsMapRender::onDrawingProgress(int current, int total)
{
  // TODO: emit signal with progress
  //std::cout << "onDrawingProgress: " << current << " / " << total << std::endl;
  emit updateMap();
}
