/***************************************************************************
    qgsmaptoolreverseline.h  - reverse a line geometry
    ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLREVERSELINE_H
#define QGSMAPTOOLREVERSELINE_H

#include "qgsmaptooledit.h"
#include "qgis_app.h"
#include "qgsgeometry.h"
#include "qgsfeatureid.h"


class QgsVertexMarker;

//! Map tool to delete vertices from line/polygon features
class APP_EXPORT QgsMapToolReverseLine : public QgsMapToolEdit
{
    Q_OBJECT

  public:
    QgsMapToolReverseLine( QgsMapCanvas *canvas );
    ~QgsMapToolReverseLine() override;

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    void canvasPressEvent( QgsMapMouseEvent *e ) override;

    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

  private:
    QgsVectorLayer *vlayer = nullptr;

    QgsGeometry partUnderPoint( QPoint p, QgsFeatureId &fid, int &partNum );

    /* Rubberband that shows the part being reversed*/
    std::unique_ptr<QgsRubberBand> mRubberBand;

    //The feature and part where the mouse cursor was pressed
    //This is used to check whether we are still in the same part at cursor release
    QgsFeatureId mPressedFid = 0;
    int mPressedPartNum = 0;
};

#endif // QGSMAPTOOLREVERSELINE_H
