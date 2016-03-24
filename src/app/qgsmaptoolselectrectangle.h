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


class APP_EXPORT QgsMapToolSelectFeatures : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectFeatures( QgsMapCanvas* canvas );

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

  private:

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    //! stores actual select rect
    QRect mSelectRect;

    QgsRubberBand* mRubberBand;

    QColor mFillColor;

    QColor mBorderColour;
};

#endif
