/***************************************************************************
    qgsmaprender.h  -  class for rendering map layer set
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

#ifndef QGSMAPRENDER_H
#define QGSMAPRENDER_H


#include "qgis.h"
#include "qgsmaplayerset.h"

class QPainter;
class QgsScaleCalculator;
class QgsMapToPixel;
class QgsMapLayer;


/**
 * \class QgsMapRender
 * \brief Class for rendering map layer set
 *
 */

class QgsMapRender : public QObject
{
  Q_OBJECT
      
  public:
    
    //! constructor
    QgsMapRender();
    
    //! destructor
    ~QgsMapRender();

    //! starts rendering
    void render(QPainter* painter);
    
    //! sets extent and checks whether suitable (returns false if not)
    bool setExtent(const QgsRect& extent);
    
    //! returns current extent
    QgsRect extent();
    
    void setLayerSet(const QgsMapLayerSet& layers) { mLayers = layers; }
    QgsMapLayerSet& layers() { return mLayers; }
    
    QgsMapToPixel* coordXForm() { return mCoordXForm; }
    
    double scale() const { return mScale; }
    double mupp() const { return mMupp; }

    //! Recalculate the map scale
    void updateScale();

    QGis::units mapUnits() const { return mMapUnits; }
    void setMapUnits(QGis::units u);
    
    //! sets whether map image will be for overview
    void setOverview(bool isOverview = true) { mOverview = isOverview; }

    void setOutputSize(QSize size, int dpi);
    
  signals:
    
    void drawingProgress(int current, int total);
    
    void updateMap();

    //! emitted when layer's draw() returned FALSE
    void drawError(QgsMapLayer*);
    
  public slots:
    
    //! called by signal from layer current being drawn
    void onDrawingProgress(int current, int total);
  
  protected:
    
    //! adjust extent to fit the pixmap size
    void adjustExtentToSize();
    
  protected:
    
    //! indicates drawing in progress
    bool mDrawing;
    
    //! map units per pixel
    double mMupp;
    
    //! Map scale at its current zool level
    double mScale;
    
    //! map units
    QGis::units mMapUnits;
    
    //! scale calculator
    QgsScaleCalculator * mScaleCalculator;
    
    //! utility class for transformation between map and pixmap units
    QgsMapToPixel* mCoordXForm;
    
    //! layers for rendering
    QgsMapLayerSet mLayers;
    
    //! current extent to be drawn
    QgsRect mExtent;
    
    //! indicates whether it's map image for overview
    bool mOverview;
    
    QSize mSize;
};

#endif

