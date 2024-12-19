/***************************************************************************
    qgmaptooltrimextendfeature.h  -  map tool to trim or extend feature
    ---------------------
    begin                : October 2018
    copyright            : (C) 2018 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLTRIMEXTENDFEATURE_H
#define QGSMAPTOOLTRIMEXTENDFEATURE_H

#include "qgsmaptooledit.h"
#include "qgis_app.h"
#include "qgsrubberband.h"
#include "qgssnappingconfig.h"

class APP_EXPORT QgsMapToolTrimExtendFeature : public QgsMapToolEdit
{
  public:
    Q_OBJECT

  public:
    QgsMapToolTrimExtendFeature( QgsMapCanvas *canvas );
    ~QgsMapToolTrimExtendFeature() override = default;

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    void keyPressEvent( QKeyEvent *e ) override;

    //! called when map tool is being activated
    void activate() override;

    //! called when map tool is being deactivated
    void deactivate() override;

  private slots:
    // Recompute the extended limit
    void extendLimit();
    void reset();

  private:
    //!  Rubberband that highlights the limit segment
    std::unique_ptr<QgsRubberBand> mRubberBandLimit;
    //!  Rubberband that extends the limit segment
    std::unique_ptr<QgsRubberBand> mRubberBandLimitExtend;
    //! Rubberband that shows the feature being extended
    std::unique_ptr<QgsRubberBand> mRubberBandExtend;
    //!  Rubberband that shows the intersection point
    std::unique_ptr<QgsRubberBand> mRubberBandIntersection;
    //!  Points for the limit
    QgsPoint pLimit1, pLimit2;
    //!  Points for extend
    QgsPoint pExtend1, pExtend2;
    //!  intersection point between the projection of [pExtend1 - pExtend2] on [pLimit1 - pLimit2]
    QgsPoint mIntersection;
    //!  map point used to determine which edges will be used for trim the feature
    QgsPointXY mMapPoint;
    //! geometry that will be returned
    QgsGeometry mGeom;
    //! Limit layer which will be snapped
    QgsVectorLayer *mLimitLayer = nullptr;
    //! Current layer which will be modified
    QgsVectorLayer *mVlayer = nullptr;
    //! Keep information about the state of the intersection
    bool mIsIntersection = false;
    //! Keep information of the first layer snapped is 3D or not
    bool mIs3DLayer = false;
    //! if feature is modified
    bool mIsModified = false;
    //! if the segments are intersected = trim
    bool mSegmentIntersects = false;
    enum Step
    {
      StepLimit,
      StepExtend,
    };
    //! The first step (0): choose the limit. The second step (1): choose the segment to trim/extend
    Step mStep = StepLimit;

    //! Snapping config that will be restored on deactivation
    QgsSnappingConfig mOriginalSnappingConfig;
};

#endif // QGSMAPTOOLTRIMEXTENDFEATURE_H
