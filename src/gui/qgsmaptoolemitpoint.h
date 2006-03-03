/***************************************************************************
    qgsmaptoolemitpoint.h  -  map tool for signaling click on map canvas
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

#ifndef QGSMAPTOOLEMITPOINT_H
#define QGSMAPTOOLEMITPOINT_H

#include "qgsmaptool.h"

#define MapTool_EmitPoint  "emit point"

class QgsMapCanvas;


class QgsMapToolEmitPoint : public QgsMapTool
{
  public:
    QgsMapToolEmitPoint(QgsMapCanvas* canvas);
    
    //! Overridden mouse move event
    virtual void canvasMoveEvent(QMouseEvent * e);
  
    //! Overridden mouse press event
    virtual void canvasPressEvent(QMouseEvent * e);
  
    //! Overridden mouse release event
    virtual void canvasReleaseEvent(QMouseEvent * e);

    virtual QString toolName() { return MapTool_EmitPoint; }
};

#endif
