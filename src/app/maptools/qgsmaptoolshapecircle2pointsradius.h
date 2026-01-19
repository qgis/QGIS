/***************************************************************************
    qgsmaptoolshapecircle2pointsradius.h  -  map tool for adding circle
    from 2 points and a radius
    ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Loïc Bartoletti
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPECIRCLE2POINTSRADIUS_H
#define QGSMAPTOOLSHAPECIRCLE2POINTSRADIUS_H

#include "qgis_app.h"
#include "qgsmaptoolshapecircleabstract.h"
#include "qgsmaptoolshaperegistry.h"

#include <QString>

using namespace Qt::StringLiterals;

class QDoubleSpinBox;
class QgsGeometryRubberBand;

class APP_EXPORT QgsMapToolShapeCircle2PointsRadiusMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeCircle2PointsRadiusMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const inline QString TOOL_ID = u"circle-from-2-points-radius"_s;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeCircle2PointsRadius : public QgsMapToolShapeCircleAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeCircle2PointsRadius( QgsMapToolCapture *parentTool );
    ~QgsMapToolShapeCircle2PointsRadius() override;

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;

    void clean() override;
    void undo() override;

  public slots:
    void radiusSpinBoxChanged( double radius );

  private:
    //! (re-)create the spin box to enter the radius of the circle
    void createRadiusSpinBox();
    //! delete the spin box to enter the radius of the circle, if it exists
    void deleteRadiusSpinBox();

    //! Update the possible centers and rubber bands
    void updateCenters( const QgsPoint &pt1, const QgsPoint &pt2 );
    //! Clear center rubber bands
    void clearCenterRubberBands();

    //! Update the segment rubber band between two points
    void updateSegmentRubberBand( const QgsPoint &pt1, const QgsPoint &pt2 );
    //! Delete the segment rubber band
    void deleteSegmentRubberBand();

    QDoubleSpinBox *mRadiusSpinBox = nullptr;

    double mRadius = 0.0;

    //! Possible centers for the circle
    QVector<QgsPoint> mCenters;

    //! Rubber bands to display possible centers
    QVector<QgsGeometryRubberBand *> mCenterRubberBands;

    //! Rubber band to display segment between two points
    QgsGeometryRubberBand *mSegmentRubberBand = nullptr;
};

#endif // QGSMAPTOOLSHAPECIRCLE2POINTSRADIUS_H
