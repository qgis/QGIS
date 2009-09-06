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

    virtual void activate();

    virtual void deactivate();

  private:
    bool identifyLayer( QgsMapLayer *layer, int x, int y );
    bool identifyRasterLayer( QgsRasterLayer *layer, int x, int y );
    bool identifyVectorLayer( QgsVectorLayer *layer, int x, int y );

    //! Pointer to the identify results dialog for name/value pairs
    QgsIdentifyResults *mResults;

    //! Private helper
    void convertMeasurement( QgsDistanceArea &calc, double &measure, QGis::UnitType &u, bool isArea );

    void addFeature( QgsMapLayer *layer, int fid,
                     QString displayField, QString displayValue,
                     const QMap< QString, QString > &attributes,
                     const QMap< QString, QString > &derivedAttributes );

  private slots:
    // Let us know when the QgsIdentifyResults dialog box has been closed
    void resultsDialogGone();
};

#endif
