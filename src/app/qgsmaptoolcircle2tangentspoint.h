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
    ~QgsMapToolCircle2TangentsPoint() override;

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

  public slots:
    void radiusSpinBoxChanged( double radius );

  private:
    //! Compute 4 possible centers
    void getPossibleCenter();

    //! (re-)create the spin box to enter the radius of the circle
    void createRadiusSpinBox();
    //! delete the spin box to enter the radius of the circle, if it exists
    void deleteRadiusSpinBox();

    QDoubleSpinBox *mRadiusSpinBox = nullptr;

    double mRadius = 0.0;
    QVector<QgsPoint> mCenters;
    QVector<QgsGeometryRubberBand *> mRubberBands;
};

#endif // QGSMAPTOOLCIRCLE2TANGENTSPOINT_H
