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

#include <QStringList>

#include "qgis.h"
#include "qgsrect.h"

class QDomDocument;
class QDomNode;
class QPainter;

class QgsMapToPixel;
class QgsMapLayer;
class QgsScaleCalculator;
class QgsSpatialRefSys;
class QgsDistanceArea;

/**
 * \class QgsMapRender
 * \brief Class for rendering map layer set
 *
 */

class CORE_EXPORT QgsMapRender : public QObject
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
    
    QgsMapToPixel* coordXForm() { return mCoordXForm; }
    
    double scale() const { return mScale; }
    double mupp() const { return mMupp; }

    int width() const { return mSize.width(); };
    int height() const { return mSize.height(); };

    //! Recalculate the map scale
    void updateScale();

    //! Return the measuring object
    QgsDistanceArea* distArea() { return mDistArea; }
    QGis::units mapUnits() const;
    void setMapUnits(QGis::units u);
    
    //! sets whether map image will be for overview
    void setOverview(bool isOverview = true) { mOverview = isOverview; }

    void setOutputSize(QSize size, int dpi);

    //!accessor for output dpi
    int outputDpi();
    //!accessor for output size
    QSize outputSize();
    
    //! transform extent in layer's SRS to extent in output SRS
    QgsRect layerExtentToOutputExtent(QgsMapLayer* theLayer, QgsRect extent);
    
    //! transform coordinates from layer's SRS to output SRS
    QgsPoint layerCoordsToOutputCoords(QgsMapLayer* theLayer, QgsPoint point);
    
    //! transform coordinates from output SRS to layer's SRS
    QgsPoint outputCoordsToLayerCoords(QgsMapLayer* theLayer, QgsPoint point);

    //! transform rect's coordinates from output SRS to layer's SRS
    QgsRect outputCoordsToLayerCoords(QgsMapLayer* theLayer, QgsRect rect);
    
    //! sets whether to use projections for this layer set
    void setProjectionsEnabled(bool enabled);
    
    //! returns true if projections are enabled for this layer set
    bool projectionsEnabled();
    
    //! sets destination spatial reference system
    void setDestinationSrs(const QgsSpatialRefSys& srs);
    
    //! returns SRS ID of destination spatial reference system
    const QgsSpatialRefSys& destinationSrs();

    //! returns current extent of layer set
    QgsRect fullExtent();
    
    //! returns current layer set
    QStringList& layerSet();
    
    //! change current layer set
    void setLayerSet(const QStringList& layers);

    //! updates extent of the layer set
    void updateFullExtent();

    //! read settings
    bool readXML(QDomNode & theNode);

    //! write settings
    bool writeXML(QDomNode & theNode, QDomDocument & theDoc);

  signals:
    
    void drawingProgress(int current, int total);
    
    void projectionsEnabled(bool flag);
    
    void destinationSrsChanged();
    
    void updateMap();
    
    void mapUnitsChanged();

    //! emitted when layer's draw() returned FALSE
    void drawError(QgsMapLayer*);

  public slots:
    
    //! called by signal from layer current being drawn
    void onDrawingProgress(int current, int total);

  protected:
    
    //! adjust extent to fit the pixmap size
    void adjustExtentToSize();
    
    /** Convenience function to project an extent into the layer source
     * SRS, but also split it into two extents if it crosses
     * the +/- 180 degree line. Modifies the given extent to be in the
     * source SRS coordinates, and if it was split, returns true, and
     * also sets the contents of the r2 parameter
     */
    bool splitLayersExtent(QgsMapLayer* layer, QgsRect& extent, QgsRect& r2);

  protected:
    
    //! indicates drawing in progress
    bool mDrawing;
    
    //! map units per pixel
    double mMupp;
    
    //! Map scale at its current zool level
    double mScale;
    
    //! scale calculator
    QgsScaleCalculator * mScaleCalculator;
    
    //! utility class for transformation between map and pixmap units
    QgsMapToPixel* mCoordXForm;
    
    //! current extent to be drawn
    QgsRect mExtent;
    
    //! indicates whether it's map image for overview
    bool mOverview;
    
    QSize mSize;

    //! detemines whether on the fly projection support is enabled
    bool mProjectionsEnabled;
    
    //! destination spatial reference system of the projection
    QgsSpatialRefSys* mDestSRS;

    //! stores array of layers to be rendered (identified by string)
    QStringList mLayerSet;
    
    //! full extent of the layer set
    QgsRect mFullExtent;

    //! tool for measuring 
    QgsDistanceArea* mDistArea;
};

#endif

