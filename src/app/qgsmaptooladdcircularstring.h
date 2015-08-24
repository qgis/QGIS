/***************************************************************************
    qgsmaptooladdcircularstring.h  -  map tool for adding circular strings
    ---------------------
    begin                : December 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLADDCIRCULARSTRING_H
#define QGSMAPTOOLADDCIRCULARSTRING_H

#include "qgsmaptoolcapture.h"

class QgsGeometryRubberBand;

class QgsMapToolAddCircularString: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolAddCircularString( QgsMapToolCapture* parentTool, QgsMapCanvas* canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddCircularString();

    void keyPressEvent( QKeyEvent* e );
    void keyReleaseEvent( QKeyEvent* e );

    void deactivate();

  private slots:
    void setParentTool( QgsMapTool* newTool, QgsMapTool* oldTool );

  protected:
    QgsMapToolAddCircularString( QgsMapCanvas* canvas = 0 ); //forbidden

    QgsMapToolCapture* mParentTool;
    /** Circular string points (in map coordinates)*/
    QList< QgsPointV2 > mPoints;
    QgsGeometryRubberBand* mRubberBand;

    //center point rubber band
    bool mShowCenterPointRubberBand;
    QgsGeometryRubberBand* mCenterPointRubberBand;

    void createCenterPointRubberBand();
    void updateCenterPointRubberBand( const QgsPointV2& pt );
    void removeCenterPointRubberBand();
};

#endif // QGSMAPTOOLADDCIRCULARSTRING_H
