/***************************************************************************
    qgsmaptoolidentify.h  -  map tool for identifying features
    ---------------------
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

#ifndef QGSMAPTOOLIDENTIFY_H
#define QGSMAPTOOLIDENTIFY_H

#include "qgsmaptool.h"
#include "qgspoint.h"

class QgsIdentifyResults;
class QgsMessageViewer;
class QgsMapLayer;
class QgsRasterLayer;
class QgsVectorLayer;

/**
  \brief Map tool for identifying features in current layer

  after selecting a point shows dialog with identification results
  - for raster layers shows value of underlying pixel
  - for vector layers shows feature attributes within search radius
    (allows to edit values when vector layer is in editing mode)
*/
class QgsMapToolIdentify : public QgsMapTool
{
  public:
    QgsMapToolIdentify(QgsMapCanvas* canvas);

    ~QgsMapToolIdentify();

    //! Overridden mouse move event
    virtual void canvasMoveEvent(QMouseEvent * e);

    //! Overridden mouse press event
    virtual void canvasPressEvent(QMouseEvent * e);

    //! Overridden mouse release event
    virtual void canvasReleaseEvent(QMouseEvent * e);


  private:

    /**
     * \brief function for identifying pixel values at a coordinate in a non-OGC-WMS raster layer
     *
     * \param point[in]  The coordinate (as the CRS of the raster layer)
     */
    void identifyRasterLayer(QgsRasterLayer* layer, const QgsPoint& point);

    /**
     * \brief function for identifying a pixel in a OGC WMS raster layer
     *
     * \param point[in]  The pixel coordinate (as it was displayed locally on screen)
     *
     * \note WMS Servers prefer to receive coordinates in image space not CRS space, therefore
     *       this special variant of identifyRasterLayer.
     */
    void identifyRasterWmsLayer(QgsRasterLayer* layer, const QgsPoint& point);

    /**
     * \brief function for identifying features at a coordinate in a vector layer
     *
     * \param point[in]  The coordinate (as the CRS of the vector layer)
     */
    void identifyVectorLayer(QgsVectorLayer* layer, const QgsPoint& point);

    //! show whatever error is exposed by the QgsMapLayer.
    void showError(QgsMapLayer * mapLayer);


    //! Pointer to the identify results dialog for name/value pairs
    QgsIdentifyResults *mResults;

    //! Pointer to the identify results dialog for WMS XML files
    QgsMessageViewer * mViewer;

};

#endif
