/***************************************************************************
    qgsmaptoolcapture.h  -  map tool for capturing points, lines, polygons
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSMAPTOOLCAPTURE_H
#define QGSMAPTOOLCAPTURE_H

#include "qgsmaptool.h"
#include "qgspoint.h"

#define MapTool_Capture  "capture"

class QgsRubberBand;

#include <QPoint>
#include <list>

class QgsMapToolCapture : public QgsMapTool
{
  public:
  
    enum CaptureTool
    {
      CapturePoint,
      CaptureLine,
      CapturePolygon
    };

    //! constructor
    QgsMapToolCapture(QgsMapCanvas* canvas, enum CaptureTool tool);

    //! destructor
    virtual ~QgsMapToolCapture();

    //! Overridden mouse move event
    virtual void canvasMoveEvent(QMouseEvent * e);
  
    //! Overridden mouse press event
    virtual void canvasPressEvent(QMouseEvent * e);
  
    //! Overridden mouse release event
    virtual void canvasReleaseEvent(QMouseEvent * e);    
    
    //! Resize rubber band
    virtual void renderComplete();
    
    virtual QString toolName() { return MapTool_Capture; }

    virtual void deactivate();
  
  protected:
    
    /** Helper function to inverse project a point if projections
        are enabled. Failsafe, returns the sent point if anything fails.
        @whenmsg is a part fo the error message. */
    QgsPoint maybeInversePoint(QgsPoint point, const char whenmsg[]);

    /** which capturing tool is being used */
    enum CaptureTool mTool;
    
    /** Flag to indicate a map canvas capture operation is taking place */
    bool mCapturing;
    
    /** rubber band for polylines and polygons */
    QgsRubberBand* mRubberBand;

    /** List to store the points of digitised lines and polygons */
    std::list<QgsPoint> mCaptureList;

};

#endif
