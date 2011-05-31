/***************************************************************************
    qgsmaptoolmovefeature.h  -  map tool for translating features by mouse drag
    ---------------------
    begin                : Juli 2007
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

#ifndef QGSMAPTOOLMOVEFEATURE_H
#define QGSMAPTOOLMOVEFEATURE_H

#include "qgsmaptooledit.h"
#include "qgsvectorlayer.h"

/**Map tool for translating feature position by mouse drag*/
class QgsMapToolMoveFeature: public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolMoveFeature( QgsMapCanvas* canvas );
    virtual ~QgsMapToolMoveFeature();

    virtual void canvasMoveEvent( QMouseEvent * e );

    virtual void canvasPressEvent( QMouseEvent * e );

    virtual void canvasReleaseEvent( QMouseEvent * e );

    //! called when map tool is being deactivated
    void deactivate();

  private:
    /**Start point of the move in map coordinates*/
    QgsPoint mStartPointMapCoords;

    /**Rubberband that shows the feature being moved*/
    QgsRubberBand* mRubberBand;

    /**Id of moved feature*/
    QgsFeatureIds mMovedFeatures;
};

#endif
