/***************************************************************************
    qgsmaptoolselectrectangle.h  -  map tool for selecting features by
                                 rectangle
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

#ifndef QGSMAPTOOLRECTANGLE_H
#define QGSMAPTOOLRECTANGLE_H

#include <QRect>
#include "qgsmaptool.h"

class QPoint;
class QMouseEvent;
class QgsMapCanvas;
class QgsVectorLayer;
class QgsGeometry;
class QgsRubberBand;


class QgsMapToolSelectRectangle : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectRectangle( QgsMapCanvas* canvas );

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

  private:

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    //! stores actual select rect
    QRect mSelectRect;

    QgsRubberBand* mRubberBand;
};

#endif
