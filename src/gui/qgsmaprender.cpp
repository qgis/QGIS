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

#include "qgslogger.h"
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

void QgsMapRender::updateScale()
{
  mScale = mScaleCalculator->calculate(mExtent, mSize.width());
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
  double muppY = static_cast<double>(mExtent.height()) 
               / static_cast<double>(myHeight);
  double muppX = static_cast<double>(mExtent.width())  
               / static_cast<double>(myWidth);
  mMupp = muppY > muppX ? muppY : muppX;

  // calculate the actual extent of the mapCanvas
  double dxmin, dxmax, dymin, dymax, whitespace;

  if (muppY > muppX)
  {
    dymin = mExtent.yMin();
    dymax = mExtent.yMax();
    whitespace = ((myWidth * mMupp) - mExtent.width()) * 0.5;
    dxmin = mExtent.xMin() - whitespace;
    dxmax = mExtent.xMax() + whitespace;
  }
  else
  {
    dxmin = mExtent.xMin();
    dxmax = mExtent.xMax();
    whitespace = ((myHeight * mMupp) - mExtent.height()) * 0.5;
    dymin = mExtent.yMin() - whitespace;
    dymax = mExtent.yMax() + whitespace;
  }

#ifdef QGISDEBUG
  QgsDebugMsg("========== Current Scale ==========");
  QgsDebugMsg("Current extent is " + mExtent.stringRep());
  QgsLogger::debug("MuppX", muppX, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("MuppY", muppY, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("Pixmap width", myWidth, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("Pixmap height", myHeight, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("Extent width", mExtent.width(), 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("Extent height", mExtent.height(), 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("whitespace: ", whitespace, 1, __FILE__, __FUNCTION__, __LINE__);
#endif


  // update extent
  mExtent.setXmin(dxmin);
  mExtent.setXmax(dxmax);
  mExtent.setYmin(dymin);
  mExtent.setYmax(dymax);

  // update the scale
  updateScale();

#ifdef QGISDEBUG
  QgsLogger::debug("Scale (assuming meters as map units) = 1", mScale, 1, __FILE__, __FUNCTION__, __LINE__);
#endif

  mCoordXForm->setParameters(mMupp, dxmin, dymin, myHeight);
}


void QgsMapRender::render(QPainter* painter)
{
  QgsDebugMsg("========== Rendering ==========");

  if (mExtent.isEmpty())
  {
    QgsLogger::warning("empty extent... not rendering");
    return;
  }


  if (mDrawing)
    return;
  
  mDrawing = true;
  
  int myRenderCounter = 0;
  
#ifdef QGISDEBUG
  QgsDebugMsg("QgsMapRender::render: Starting to render layer stack.");
  QTime renderTime;
  renderTime.start();
#endif
  // render all layers in the stack, starting at the base
  std::deque<QString> layers = mLayers.layerSet();
  std::deque<QString>::iterator li = layers.begin();
  
  while (li != layers.end())
  {
    QgsDebugMsg("QgsMapRender::render: at layer item '" + (*li));

    // This call is supposed to cause the progress bar to
    // advance. However, it seems that updating the progress bar is
    // incompatible with having a QPainter active (the one that is
    // passed into this function), as Qt produces a number of errors
    // when try to do so. I'm (Gavin) not sure how to fix this, but
    // added these comments and debug statement to help others...
    QgsDebugMsg("If there is a QPaintEngine error here, it is caused by an emit call");

    emit setProgress(myRenderCounter++,layers.size());
    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer(*li);

    if (!ml)
    {
      QgsLogger::warning("QgsMapRender::render: layer not found in registry!");
      li++;
      continue;
    }
        
#ifdef QGISDEBUG
		QgsDebugMsg("QgsMapRender::render: Rendering layer " + ml->name());
		QgsLogger::debug("  Layer minscale ", ml->minScale(), 1, __FILE__, __FUNCTION__, __LINE__);
		QgsLogger::debug("  Layer maxscale ", ml->maxScale(), 1, __FILE__, __FUNCTION__, __LINE__);
		QgsLogger::debug("  Scale dep. visibility enabled? ", ml->scaleBasedVisibility(), 1,\
				 __FILE__, __FUNCTION__, __LINE__);
		QgsLogger::debug("  Input extent: " + ml->extent().stringRep(), 1, __FILE__, __FUNCTION__, __LINE__);
    try
    {
      QgsDebugMsg("  Transformed extent: " + ml->coordinateTransform()->transformBoundingBox(ml->extent()).stringRep());
    }
    catch (QgsCsException &cse)
    {
      QgsLogger::warning("Transform error caught in " + QString(__FILE__) + " line " + QString(__LINE__) +\
			 QString(cse.what()));
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
	QgsDebugMsg("QgsMapRender::render: Layer not rendered because it is not within the defined \
visibility scale range")
      }
        
    } // if (ml->visible())
    
    li++;
    
  } // while (li != end)
      
    QgsDebugMsg("QgsMapRender::render: Done rendering map layers");

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
  QgsLogger::debug("QgsMapRender::render: Rendering done in (seconds)", renderTime.elapsed() / 1000.0, 1,\
		   __FILE__, __FUNCTION__, __LINE__);
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
