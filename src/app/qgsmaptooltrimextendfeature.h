/***************************************************************************
    qgmaptoolextendfeature.h  -  map tool for extending feature
    ---------------------
    begin                : October 2018
    copyright            : (C) 2018 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLTRIMEXTENDFEATURE_H
#define QGSMAPTOOLTRIMEXTENDFEATURE_H

#include "qgsmaptooledit.h"
#include "qgis_app.h"
#include "qgspointlocator.h"


class APP_EXPORT QgsMapToolTrimExtendFeature : public QgsMapToolEdit
{
  public:
    Q_OBJECT

  public:
    QgsMapToolTrimExtendFeature( QgsMapCanvas *canvas );
    ~QgsMapToolTrimExtendFeature() override;

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    void keyPressEvent( QKeyEvent *e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

  private:
    //!  Rubberband that shows the limit
    std::unique_ptr<QgsRubberBand>mRubberBandLimit;
    //! Rubberband that shows the feature being extended
    std::unique_ptr<QgsRubberBand>mRubberBandExtend;
    //!  Rubberband that shows the intersection point
    std::unique_ptr<QgsRubberBand>mRubberBandIntersection;
    //!  Points for the limit
    QgsPoint pLimit1, pLimit2;
    //!  Points for extend
    QgsPoint pExtend1, pExtend2;
    //!  intersection point between the projection of [pExtend1 - pExtend2] on [pLimit1 - pLimit2]
    QgsPoint intersection;
    //!  map point used to determine which edges will be used for trim the feature
    QgsPointXY mapPoint;
    //! geometry that will be returned
    QgsGeometry geom;
    //! Current layer which will be modified
    QgsVectorLayer *vlayer = nullptr;
    //! Keep information about the state of the intersection
    bool isIntersection = false;
    //! Keep information of the first layer snapped is 3D or not
    bool is3DLayer = false;
    //! if feature is modified
    bool isModified = false;
    //! if the segments are intersected = trim
    bool segmentIntersects = false;
    //! The first step (0): choose the limit. The second step (1): choose the segment to trim/extend
    int step = 0;
};

#endif // QGSMAPTOOLTRIMEXTENDFEATURE_H
