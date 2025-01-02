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
#include "qgis_app.h"
#include "qgsfeatureid.h"
#include "qgsgeometry.h"

class QgsVertexMarker;

//! Map tool to delete vertices from line/polygon features
class APP_EXPORT QgsMapToolDeletePart : public QgsMapToolEdit
{
    Q_OBJECT

  public:
    QgsMapToolDeletePart( QgsMapCanvas *canvas );
    ~QgsMapToolDeletePart() override;

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    void canvasPressEvent( QgsMapMouseEvent *e ) override;

    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

  private:
    QgsVectorLayer *vlayer = nullptr;

    QgsGeometry partUnderPoint( QPoint p, QgsFeatureId &fid, int &partNum );

    /* Rubberband that shows the part being deleted*/
    QgsRubberBand *mRubberBand = nullptr;

    //The feature and part where the mouse cursor was pressed
    //This is used to check whether we are still in the same part at cursor release
    QgsFeatureId mPressedFid;
    int mPressedPartNum;
};

#endif
