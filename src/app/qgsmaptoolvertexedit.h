/***************************************************************************
    qgsmaptoolvertexedit.h  - tool for adding, moving, deleting vertices
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

#ifndef QGSMAPTOOLVERTEXEDIT_H
#define QGSMAPTOOLVERTEXEDIT_H

#include "qgsmaptool.h"
#include "qgsgeometry.h"
#include "qgsgeometryvertexindex.h"

class QgsRubberBand;
class QgsVertexMarker;

class QgsMapToolVertexEdit : public QgsMapTool
{
  public:
  
    enum Tool
    {
      AddVertex,
      MoveVertex,
      DeleteVertex
    };
  
    //! constructor
    QgsMapToolVertexEdit(QgsMapCanvas* canvas, enum Tool tool);

    virtual ~QgsMapToolVertexEdit();
    
    //! Overridden mouse move event
    virtual void canvasMoveEvent(QMouseEvent * e);
  
    //! Overridden mouse press event
    virtual void canvasPressEvent(QMouseEvent * e);
  
    //! Overridden mouse release event
    virtual void canvasReleaseEvent(QMouseEvent * e);

    //! called when map tool is being deactivated
    virtual void deactivate();

  private:
    
    //! create instance of rubber band - done when the mouse is pressed down
    void createRubberBand();

    //! delete instance of rubber band - done when the mouse is released
    void deleteRubberBand();

    //! returns tolerance in map coordinates
    double tolerance();

    //! current vertex edit tool
    enum Tool mTool;

    //! Displays a warning about the snap tolerance settings
    void displaySnapToleranceWarning();
    
    /**Searches the closest point within the project tolerance and setx  mSnappedAtFeatureId and mSnappedAtVertex*/
    bool snapVertexWithContext(QgsPoint& point);

    /** Searches the closest vertex of "geometry" to the given "point" 

        This is useful when selecting a vertex to move, as the selection process is a two step process:

        1. Select the closest line segment to the mouse pointer
             (snapSegmentWithContext()).
        2. Select the closest vertex on (1)
             (snpVertexOfSnappedSegment()).

        Step 1 is needed to disambiguate between two geometries sharing the same vertex.
     */
    bool snapVertexOfSnappedSegment(QgsPoint& point);

    /**Snaps a point (without setting mSnappedAtFeatureId and mSnappedAtVertex). Does not snap to the specified vertex,
     because during dragging, a vertex should not be snapped to itself*/
    void snapVertex(QgsPoint& point, int exclFeatureId, int exclVertexNr);


    bool snapSegmentWithContext(QgsPoint& point);


    //! The snapped-to segment before this vertex number (identifying the vertex that is being moved)
    QgsGeometryVertexIndex mSnappedAtVertex;

    /**The index of the first rubber band point (to connect to mSnappedatVertex). -1 if no such point exists*/
    int mRubberBandIndex1;

    /**The index of the second rubber band point (to connect to mSnappedatVertex). -1 if no such point exists*/
    int mRubberBandIndex2;

    //! The snapped-to segment before this vertex number (identifying the segment that a new vertex is being added to)
    QgsGeometryVertexIndex mSnappedBeforeVertex;

    //! The snapped-to feature ID
    int mSnappedAtFeatureId;

    //! The snapped-to geometry
    QgsGeometry mSnappedAtGeometry;
    
    QgsRubberBand* mRubberBand;
    
    bool mStartPointValid;
    bool mStopPointValid;
    
    QgsVertexMarker* mCross;

};

#endif
