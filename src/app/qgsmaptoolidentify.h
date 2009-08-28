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

#include "qgis.h"
#include "qgsmaptool.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include "qgsdistancearea.h"

#include <QObject>

class QgsIdentifyResults;
class QgsMapLayer;
class QgsRasterLayer;
class QgsRubberBand;
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
    Q_OBJECT

  public:
    QgsMapToolIdentify( QgsMapCanvas* canvas );

    ~QgsMapToolIdentify();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    //! called when map tool is being deactivated
    virtual void deactivate();

  public slots:
    //! creates rubberband on top of the feature to highlight it
    void highlightFeature( int featureId );

    //! edit a feature
    void editFeature( int featureId );

  private:

    /**
     * \brief function for identifying pixel values at a coordinate in a non-OGC-WMS raster layer
     *
     * \param point[in]  The coordinate (as the CRS of the raster layer)
     */
    void identifyRasterLayer( const QgsPoint& point );

    /**
     * \brief function for identifying a pixel in a OGC WMS raster layer
     *
     * \param point[in]  The pixel coordinate (as it was displayed locally on screen)
     *
     * \note WMS Servers prefer to receive coordinates in image space not CRS space, therefore
     *       this special variant of identifyRasterLayer.
     */
    void identifyRasterWmsLayer( const QgsPoint& point );

    /**
     * \brief function for identifying features at a coordinate in a vector layer
     *
     * \param point[in]  The coordinate (as the CRS of the vector layer)
     */
    void identifyVectorLayer( const QgsPoint& point );

    //! show whatever error is exposed by the QgsMapLayer.
    void showError();

    //! edit a feature
    void editFeature( QgsFeature &f );

    //! Pointer to the identify results dialog for name/value pairs
    QgsIdentifyResults *mResults;

    //! Rubber band for highlighting identified feature
    QgsRubberBand* mRubberBand;

    QgsMapLayer *mLayer;

    //! list of identified features
    QgsFeatureList mFeatureList;

    //! Private helper
    void convertMeasurement( QgsDistanceArea &calc, double &measure, QGis::UnitType &u, bool isArea );

  private slots:
    // Let us know when the QgsIdentifyResults dialog box has been closed
    void resultsDialogGone();

    // layer was destroyed
    void layerDestroyed();
};

#endif
