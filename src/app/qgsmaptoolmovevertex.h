/***************************************************************************
                              qgsmaptoolmovevertex.h
                              ---------------------
  begin                : June 28, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMAPTOOLMOVEVERTEX_H
#define QGSMAPTOOLMOVEVERTEX_H

#include "qgsmaptoolvertexedit.h"

class QgsRubberBand;

/**A maptool to move vertices of line or polygon fetures*/
class QgsMapToolMoveVertex: public QgsMapToolVertexEdit
{
    Q_OBJECT

  public:
    QgsMapToolMoveVertex( QgsMapCanvas* canvas );
    virtual ~QgsMapToolMoveVertex();

    void canvasMoveEvent( QMouseEvent * e );

    void canvasPressEvent( QMouseEvent * e );

    void canvasReleaseEvent( QMouseEvent * e );

    //! called when map tool is being deactivated
    void deactivate();

  private:
    QList<QgsRubberBand*> mRubberBands;
    /**Stores indices of rubber band points
     to move in the canvasMoveEvent. -1 means
    that no point should be moved*/
    QList<int> mRubberBandMovingPoints;

    /**The position of the vertex to move (in map coordinates) to exclude later from snapping*/
    QList<QgsPoint> mExcludePoint;

    /**Deletes the rubber band pointers
     and clears mRubberBands*/
    void removeRubberBands();
};

#endif
