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

    virtual void cadCanvasReleaseEvent( QgsMapMouseEvent* e ) override;
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent* e ) override;
    virtual void deactivate() override;

  private slots:
    void updateRadiusFromSpinBox( double radius );

  private:
    QgsPointV2 mTemporaryEndPoint;
    double mRadius;
    QDoubleSpinBox* mRadiusSpinBox;

    //! recalculate the rubberband
    void recalculateRubberBand();
    //! recalculate the temporary rubberband using the given mouse position
    void recalculateTempRubberBand( const QgsPoint& mousePosition );
    //! (re-)create the spin box to enter the radius
    void createRadiusSpinBox();
    //! delete the spin box to enter the radius, if it exists
    void deleteRadiusSpinBox();
};

#endif // QGSMAPTOOLCIRCULARSTRINGRADIUS_H
