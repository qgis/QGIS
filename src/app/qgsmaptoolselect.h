/***************************************************************************
    qgsmaptoolselect.h  -  map tool for selecting features
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

#ifndef QGSMAPTOOLSELECT_H
#define QGSMAPTOOLSELECT_H

#include "qgsmaptool.h"
#include <QRect>


class QRubberBand;
class QgsMapCanvas;


class QgsMapToolSelect : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelect( QgsMapCanvas* canvas );

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );


  protected:

    //! stores actual select rect
    QRect mSelectRect;

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    //! rubber band for select rect
    QRubberBand* mRubberBand;
};

#endif
