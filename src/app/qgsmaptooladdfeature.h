/***************************************************************************
    qgsmaptooladdfeature.h  -  map tool for adding point/line/polygon features
    ---------------------
    begin                : April 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsmaptoolcapture.h"

/**This tool adds new point/line/polygon features to already existing vector layers*/
class QgsMapToolAddFeature: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolAddFeature( QgsMapCanvas* canvas, enum CaptureMode tool );
    virtual ~QgsMapToolAddFeature();
    void canvasReleaseEvent( QMouseEvent * e );
    
    /**Modifies geometry to avoid intersections with the layers specified in project properties
    @return 0 in case of success, 
    @return 1 if geometry is not of polygon type, 
    @return 2 if avoid intersection would change the geometry type, \
    3 other error during intersection removal
    @note Consider moving this into analysis lib since it is now used by QgsGpsInformation too. */
    static int avoidIntersections( QgsGeometry* g );
};
