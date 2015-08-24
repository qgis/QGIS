/***************************************************************************
    qgsmaptoolcircularstringradius.h  -  map tool for adding circular strings
    by two points and radius
    ---------------------
    begin                : Feb 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCIRCULARSTRINGRADIUS_H
#define QGSMAPTOOLCIRCULARSTRINGRADIUS_H

#include "qgsmaptooladdcircularstring.h"
#include "qgspointv2.h"

class QDoubleSpinBox;

class QgsMapToolCircularStringRadius: public QgsMapToolAddCircularString
{
    Q_OBJECT
  public:
    QgsMapToolCircularStringRadius( QgsMapToolCapture* parentTool, QgsMapCanvas* canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolCircularStringRadius();

    virtual void canvasMapReleaseEvent( QgsMapMouseEvent* e ) override;
    virtual void canvasMapMoveEvent( QgsMapMouseEvent* e ) override;

  private slots:
    void updateRadiusFromSpinBox( double radius );

  private:
    double mTemporaryEndPointX;
    double mTemporaryEndPointY;
    bool mRadiusMode;
    double mRadius;
    QgsPointV2 mLastMouseMapPos;
    QDoubleSpinBox* mRadiusSpinBox;

    //recalculate circular string and rubber band depending on mRadius/mLeft and endpoints
    void recalculateCircularString();
    void createRadiusSpinBox();
    void deleteRadiusSpinBox();
};

#endif // QGSMAPTOOLCIRCULARSTRINGRADIUS_H
