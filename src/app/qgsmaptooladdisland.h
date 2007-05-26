/***************************************************************************
    qgsmaptooladdisland.h  - map tool to add new polygons to multipolygon features
    ---------------------
    begin                : Mai 2007
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

class QgsMapToolAddIsland: public QgsMapToolCapture
{
 public:
  QgsMapToolAddIsland(QgsMapCanvas* canvas);
  ~QgsMapToolAddIsland();
  void canvasReleaseEvent(QMouseEvent * e);
};
