/***************************************************************************
    qgsmaptooladdring.h  - map tool to cut rings in polygon and multipolygon features
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

#include "qgsmaptoolcapture.h"
#include "qgsmapmouseevent.h"

/**A tool to cut holes into polygons and multipolygon features*/
class APP_EXPORT QgsMapToolAddRing: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolAddRing( QgsMapCanvas* canvas );
    virtual ~QgsMapToolAddRing();
    void canvasMapReleaseEvent( QgsMapMouseEvent * e );
};
