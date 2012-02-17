/***************************************************************************
                              qgsmaptooloffsetcurve.h
    ------------------------------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLOFFSETCURVE_H
#define QGSMAPTOOLOFFSETCURVE_H

#include "qgsmaptooledit.h"
#include "qgsgeometry.h"
#include "qgssnapper.h"

class QgsMapToolOffsetCurve: public QgsMapToolEdit
{
  public:
    QgsMapToolOffsetCurve( QgsMapCanvas* canvas );
    ~QgsMapToolOffsetCurve();

    void canvasPressEvent( QMouseEvent * e );
    void canvasReleaseEvent( QMouseEvent * e );
    void canvasMoveEvent( QMouseEvent * e );

  private:

    /**Rubberband that shows the position of the offset curve*/
    QgsRubberBand* mRubberBand;
    /**Geometry to manipulate*/
    QgsGeometry* mOriginalGeometry;
    /**Geometry after manipulation*/
    QgsGeometry mModifiedGeometry;
    /**ID of manipulated feature*/
    QgsFeatureId mModifiedFeature;
    /**Layer ID of source layer*/
    QString mSourceLayerId;
    /**Internal flag to distinguish move from click*/
    bool mGeometryModified;


    void deleteRubberBandAndGeometry();
    QgsGeometry* createOriginGeometry( QgsVectorLayer* vl, const QgsSnappingResult& sr, QgsFeature& snappedFeature );
};

#endif // QGSMAPTOOLOFFSETCURVE_H
