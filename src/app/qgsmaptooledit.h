/***************************************************************************
    qgsmaptooledit.h  -  base class for editing map tools
    ---------------------
    begin                : Juli 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSMAPTOOLEDIT_H
#define QGSMAPTOOLEDIT_H

#include "qgsmaptool.h"
#include "qgsmapcanvassnapper.h"

class QgsRubberBand;
class QKeyEvent;

/**Base class for map tools that edit vector geometry*/
class QgsMapToolEdit: public QgsMapTool
{
  public:
    QgsMapToolEdit( QgsMapCanvas* canvas );
    virtual ~QgsMapToolEdit();

    virtual bool isEditTool() { return true; }

  protected:
    /**Snapper object that reads the settings from project and option
     and applies it to the map canvas*/
    QgsMapCanvasSnapper mSnapper;

    /**Inserts vertices to the snapped segments of the editing layer.
     This is useful for topological editing if snap to segment is enabled.
     @param snapResults results collected from the snapping operation
     @param editedLayer pointer to the editing layer
     @return 0 in case of success*/
    int insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults, QgsVectorLayer* editedLayer );

    /**Extracts a single snapping point from a set of snapping results.
       This is useful for snapping operations that just require a position to snap to and not all the
       snapping results. If the list is empty, the screen coordinates are transformed into map coordinates and returned
       @param snapResults results collected from the snapping operation.
       @return the snapped point in map coordinates*/
    QgsPoint snapPointFromResults( const QList<QgsSnappingResult>& snapResults, const QPoint& screenCoords );

    /**Creates a rubber band with the color/line width from
     the QGIS settings. The caller takes ownership of the
    returned object*/
    QgsRubberBand* createRubberBand( bool isPolygon = false );

    /**Returns the current vector layer of the map canvas or 0*/
    QgsVectorLayer* currentVectorLayer();

    /**Adds vertices to other features to keep topology up to date, e.g. to neighbouring polygons.
       @param geom list of points (in layer coordinate system)
       @return 0 in case of success*/
    int addTopologicalPoints( const QList<QgsPoint>& geom );
};

#endif
