/***************************************************************************
    qgsmaptoolrotatefeature.h  -  map tool for rotating features by mouse drag
    ---------------------
    begin                : January 2013
    copyright            : (C) 2013 by Vinayan Parameswaran
    email                : vinayan123 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLROTATEFEATURE_H
#define QGSMAPTOOLROTATEFEATURE_H

#include "qgsmaptooledit.h"
#include "qgsvectorlayer.h"

class QgsVertexMarker;

/**Map tool for translating feature position by mouse drag*/
class APP_EXPORT QgsMapToolRotateFeature: public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolRotateFeature( QgsMapCanvas* canvas );
    virtual ~QgsMapToolRotateFeature();

    virtual void canvasMoveEvent( QMouseEvent * e );

    virtual void canvasPressEvent( QMouseEvent * e );

    virtual void canvasReleaseEvent( QMouseEvent * e );

    void keyPressEvent( QKeyEvent* e );

    void keyReleaseEvent( QKeyEvent* e );


    //! to reset the rotation anchor to selectionbound center
    void resetAnchor();
    //! called when map tool is being deactivated
    void deactivate();

    void activate();


  private:

    QgsGeometry rotateGeometry( QgsGeometry geom, QgsPoint point, double angle );
    QgsPoint rotatePoint( QgsPoint point, double angle );

    /**Start point of the move in map coordinates*/
    QgsPoint mStartPointMapCoords;
    QPointF mInitialPos;

    /**Rubberband that shows the feature being moved*/
    QgsRubberBand* mRubberBand;

    /**Id of moved feature*/
    QgsFeatureIds mRotatedFeatures;
    double mRotation;

    QPoint mStPoint;
    QgsVertexMarker* mAnchorPoint;

    /** flag if crtl is pressed */
    bool mCtrl;
};

#endif
