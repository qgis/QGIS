/***************************************************************************
    qgsmaptooldeletering.h  - delete a ring from polygon
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLDELETERING_H
#define QGSMAPTOOLDELETERING_H

#include "qgsmaptooledit.h"
#include "qgsrubberband.h"

class QgsVertexMarker;
/**Map tool to delete vertices from line/polygon features*/

class APP_EXPORT QgsMapToolDeleteRing : public QgsMapToolEdit
{
    Q_OBJECT

  public:
    QgsMapToolDeleteRing( QgsMapCanvas* canvas );
    virtual ~QgsMapToolDeleteRing();

    void canvasMoveEvent( QMouseEvent * e ) override;

    void canvasPressEvent( QMouseEvent * e ) override;

    void canvasReleaseEvent( QMouseEvent * e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

  private:
    QgsVectorLayer* vlayer;

    //! delete inner ring from the geometry
    void deleteRing( QgsFeatureId fId, int beforeVertexNr, QgsVectorLayer* vlayer );

    //! return ring number in polygon
    int ringNumInPolygon( QgsGeometry* g, int vertexNr );

    //! return ring number in multipolygon and set parNum to index of the part
    int ringNumInMultiPolygon( QgsGeometry* g, int vertexNr, int& partNum );

    /*! return the geometry of the ring under the point p and sets fid to the feature id,
     * partNum to the part number in the feature and ringNum to the ring number in the part
     */
    QgsGeometry* ringUnderPoint( QgsPoint p, QgsFeatureId& fid, int& partNum, int& ringNum );

    /* Rubberband that shows the ring being deleted*/
    QgsRubberBand* mRubberBand;

    //The feature, part and ring the mouse was pressed in, to  check we are still in the same ring at release
    QgsFeatureId mPressedFid;
    int mPressedPartNum;
    int mPressedRingNum;
};

#endif
