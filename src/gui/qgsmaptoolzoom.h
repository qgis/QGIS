/***************************************************************************
    qgsmaptoolzoom.h  -  map tool for zooming
    ----------------------
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

#ifndef QGSMAPTOOLZOOM_H
#define QGSMAPTOOLZOOM_H

#include "qgsmaptool.h"
#include <QRect>


class QRubberBand;

/** \ingroup gui
 * A map tool for zooming into the map.
 * @see QgsMapTool
 */
class GUI_EXPORT QgsMapToolZoom : public QgsMapTool
{
  public:
    //! constructor
    QgsMapToolZoom( QgsMapCanvas* canvas, bool zoomOut );

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    virtual bool isTransient() { return true; }

  protected:
    //! stores actual zoom rect
    QRect mZoomRect;

    //! indicates whether we're zooming in or out
    bool mZoomOut;

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    //! TODO: to be changed to a canvas item
    QRubberBand* mRubberBand;
};

#endif
