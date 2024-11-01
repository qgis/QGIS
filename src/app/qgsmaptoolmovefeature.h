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

#include "qgsmaptooladvanceddigitizing.h"
#include "qgis_app.h"
#include "qgspointxy.h"
#include "qgsfeatureid.h"

class QgsSnapIndicator;

//! Map tool for translating feature position by mouse drag
class APP_EXPORT QgsMapToolMoveFeature : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:
    //! Mode for moving features
    enum MoveMode
    {
      Move,    //!< Move feature
      CopyMove //!< Copy and move feature
    };

    QgsMapToolMoveFeature( QgsMapCanvas *canvas, MoveMode mode = Move );
    ~QgsMapToolMoveFeature() override;

    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    void deactivate() override;

    //! catch escape when active to action
    void keyReleaseEvent( QKeyEvent *e ) override;

  private:
    void deleteRubberband();

    //! Start point of the move in map coordinates
    QgsPointXY mStartPointMapCoords;

    //! Rubberband that shows the feature being moved
    QgsRubberBand *mRubberBand = nullptr;

    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    //! Id of moved features
    QgsFeatureIds mMovedFeatures;

    QPoint mPressPos;

    MoveMode mMode;

    // MultiGeometry of the moved features
    QgsGeometry mGeom;
};

#endif
