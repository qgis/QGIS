/***************************************************************************
    qgsmaptoolcircle2tangentspoint.h  -  map tool for adding circle
    from 2 tangents and a point
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCIRCLE2TANGENTSPOINT_H
#define QGSMAPTOOLCIRCLE2TANGENTSPOINT_H

#include "qgspointlocator.h"
#include "qgsmaptooladdcircle.h"
#include "qspinbox.h"

class QSpinBox;

class QgsMapToolCircle2TangentsPoint: public QgsMapToolAddCircle
{
    Q_OBJECT

  public:
    QgsMapToolCircle2TangentsPoint( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolCircle2TangentsPoint();

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

  public slots:
    void radiusSpinBoxChanged( int radius );

  private:
    //! Return the point where segments are intersected. Method from QgsGeometryUtils doesn't work for special cases used by this tool.
    QgsPointXY intersect( QgsPointXY seg1_pt1, QgsPointXY seg1_pt2, QgsPointXY seg2_pt1, QgsPointXY seg2_pt2 );

    //! Compute 4 possible centers
    void getPossibleCenter();

    //! (re-)create the spin box to enter the radius of the circle
    void createRadiusSpinBox();
    //! delete the spin box to enter the radius of the circle, if it exists
    void deleteRadiusSpinBox();

    QSpinBox *mRadiusSpinBox = nullptr;

    int mRadius = 0;
    QVector<QgsPointXY> mCenters;
    QVector<QgsGeometryRubberBand *> mRubberBands;
};

#endif // QGSMAPTOOLCIRCLE2TANGENTSPOINT_H
