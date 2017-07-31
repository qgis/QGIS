/***************************************************************************
    qgsmaptooladdellipse.h  -  map tool for adding ellipse
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
#ifndef QGSMAPTOOLADDELLIPSE_H
#define QGSMAPTOOLADDELLIPSE_H

#include "qgsmaptoolcapture.h"
#include "qgsellipse.h"

class QgsGeometryRubberBand;

class QgsMapToolAddEllipse: public QgsMapToolCapture
{
    Q_OBJECT
    void clean();
  public:
    QgsMapToolAddEllipse( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddEllipse();

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void deactivate() override;

    void activate() override;

  private slots:
    void setParentTool( QgsMapTool *newTool, QgsMapTool *oldTool );

  protected:
    explicit QgsMapToolAddEllipse( QgsMapCanvas *canvas ); //forbidden

    /** The parent map tool, e.g. the add feature tool.
     *  Completed ellipse will be added to this tool by calling its toLineString() method.
     * */
    QgsMapToolCapture *mParentTool = nullptr;
    //! Ellipse points (in map coordinates)
    QgsPointSequence mPoints;
    //! The rubberband to show the ellipse currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;
    //! Ellipse
    QgsEllipse mEllipse;

};

#endif // QGSMAPTOOLADDELLIPSE_H
