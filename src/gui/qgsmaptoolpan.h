/***************************************************************************
    qgsmaptoolpan.h  -  map tool for panning in map canvas
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

#ifndef QGSMAPTOOLPAN_H
#define QGSMAPTOOLPAN_H

#include "qgsmaptool.h"
class QgsMapCanvas;


/** \ingroup gui
 * A map tool for panning the map.
 * @see QgsMapTool
 */
class GUI_EXPORT QgsMapToolPan : public QgsMapTool
{
  public:
    //! constructor
    QgsMapToolPan( QgsMapCanvas* canvas );

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    virtual bool isTransient() { return true; }

  private:

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

};

#endif
