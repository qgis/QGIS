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
#include "qgsfeature.h"

/**This tool adds new point/line/polygon features to already existing vector layers*/
class QgsMapToolAddFeature : public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolAddFeature( QgsMapCanvas* canvas, CaptureMode mode );
    virtual ~QgsMapToolAddFeature();
    void canvasReleaseEvent( QMouseEvent * e );

    bool addFeature( QgsVectorLayer *vlayer, QgsFeature *f );
};
