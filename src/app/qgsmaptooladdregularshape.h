/***************************************************************************
    qgsmaptooladdregularshape.h  -  map tool for adding regular shape except
    circle or ellipse
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPTOOLADDREGULARSHAPE_H
#define QGSMAPTOOLADDREGULARSHAPE_H

#include "qgslinestring.h"
#include "qgsregularpolygon.h"
#include "qgsmaptoolcapture.h"

class QgsMapToolAddRegularShape: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolAddRegularShape( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddRegularShape();

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void deactivate() override;

    void activate() override;

  private slots:
    void setParentTool( QgsMapTool *newTool, QgsMapTool *oldTool );

  protected:
    explicit QgsMapToolAddRegularShape( QgsMapCanvas *canvas ); //forbidden

    /** The parent map tool, e.g. the add feature tool.
     *  Completed regular shape will be added to this tool by calling its addCurve() method.
     * */
    QgsMapToolCapture *mParentTool = nullptr;
    //! Regular Shape points (in map coordinates)
    QgsPointSequence mPoints;
    //! The rubberband to show the Regular Shape currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;
    //! Regular shape as a linestring
    QgsLineString *mRegularShape = nullptr;

};

#endif // QGSMAPTOOLADDREGULARSHAPE_H
