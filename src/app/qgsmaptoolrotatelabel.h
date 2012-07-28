/***************************************************************************
                          qgsmaptoolrotatelabel.h
                          -----------------------
    begin                : 2010-11-09
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLROTATELABEL_H
#define QGSMAPTOOLROTATELABEL_H

#include "qgsmaptoollabel.h"
class QgsPointRotationItem;

class QgsMapToolRotateLabel: public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolRotateLabel( QgsMapCanvas* canvas );
    ~QgsMapToolRotateLabel();

    virtual void canvasPressEvent( QMouseEvent * e );
    virtual void canvasMoveEvent( QMouseEvent * e );
    virtual void canvasReleaseEvent( QMouseEvent * e );

  protected:

    static int roundTo15Degrees( double n );
    /**Converts azimuth value to counterclockwise 0 - 360*/
    static double azimuthToCCW( double a );

    QgsRubberBand* createRotationPreviewBox();
    void setRotationPreviewBox( double rotation );

    /**Rotates input point counterclockwise around centerPoint*/
    QgsPoint rotatePointCounterClockwise( const QgsPoint& input, const QgsPoint& centerPoint, double degrees );

    double mStartRotation; //rotation value prior to start rotating
    double mCurrentRotation;
    double mCurrentMouseAzimuth;
    QgsPoint mRotationPoint;
    QgsPointRotationItem* mRotationItem;
    QgsRubberBand* mRotationPreviewBox;

    /**True if ctrl was pressed during the last mouse move event*/
    bool mCtrlPressed;
};

#endif // QGSMAPTOOLROTATELABEL_H
