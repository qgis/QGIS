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

#include "qgsmaptoolvertexedit.h"

class QgsVertexMarker;
/**Map tool to delete vertices from line/polygon features*/

class QgsMapToolDeleteRing : public QgsMapToolVertexEdit
{
  Q_OBJECT

  public:
    QgsMapToolDeleteRing( QgsMapCanvas* canvas );
    virtual ~QgsMapToolDeleteRing();

    void canvasMoveEvent( QMouseEvent * e );

    void canvasPressEvent( QMouseEvent * e );

    void canvasReleaseEvent( QMouseEvent * e );

    //! called when map tool is being deactivated
    void deactivate();

  private:
    QgsVertexMarker* mCross;

    //! delete inner ring from the geometry
    void deleteRing( int fId, int beforeVertexNr, QgsVectorLayer* vlayer );

    //! return ring number in polygon
    int ringNumInPolygon( QgsGeometry* g, int vertexNr );

    //! return ring number in multipolygon and set parNum to index of the part
    int ringNumInMultiPolygon( QgsGeometry* g, int vertexNr, int& partNum );
};

#endif
