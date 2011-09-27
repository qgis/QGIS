/***************************************************************************
    qgsmaptooldeletepart.h  - delete a part from multipart geometry
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

#ifndef QGSMAPTOOLDELETEPART_H
#define QGSMAPTOOLDELETEPART_H

#include "qgsmaptoolvertexedit.h"

class QgsVertexMarker;

/**Map tool to delete vertices from line/polygon features*/
class QgsMapToolDeletePart: public QgsMapToolVertexEdit
{
    Q_OBJECT

  public:
    QgsMapToolDeletePart( QgsMapCanvas* canvas );
    virtual ~QgsMapToolDeletePart();

    void canvasMoveEvent( QMouseEvent * e );

    void canvasPressEvent( QMouseEvent * e );

    void canvasReleaseEvent( QMouseEvent * e );

    //! called when map tool is being deactivated
    void deactivate();

  private:
    QgsVertexMarker* mCross;

    //! delete part of a geometry
    void deletePart( QgsFeatureId fId, int beforeVertexNr, QgsVectorLayer* vlayer );

    //! find out part number of geometry given the snapped vertex number
    int partNumberOfVertex( QgsGeometry* g, int beforeVertexNr );

};

#endif
