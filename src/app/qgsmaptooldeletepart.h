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

#include "qgsmaptooledit.h"

class QgsVertexMarker;

/**Map tool to delete vertices from line/polygon features*/
class APP_EXPORT QgsMapToolDeletePart: public QgsMapToolEdit
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
    QgsVectorLayer* vlayer;
    QList<QgsSnappingResult> mRecentSnappingResults;

    //! find out the part number of geometry given the vertex number
    int partNumberOfVertex( QgsGeometry* g, int beforeVertexNr );

    //! find out the part number of geometry including the point
    int partNumberOfPoint( QgsGeometry* g, QgsPoint point );

    //! find which feature is under the point position (different from snapping as we allow the whole polygon surface)
    QgsFeature featureUnderPoint(QgsPoint p);

    void notifySinglePart();

};

#endif
