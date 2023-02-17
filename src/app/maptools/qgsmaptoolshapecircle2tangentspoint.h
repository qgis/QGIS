/***************************************************************************
    qgsmaptoolshapecircle2tangentspoint.h  -  map tool for adding circle
    from 2 tangents and a point
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPECIRCLE2TANGENTSPOINT_H
#define QGSMAPTOOLSHAPECIRCLE2TANGENTSPOINT_H

#include "qgspointlocator.h"
#include "qgsmaptoolshapecircleabstract.h"
#include "qspinbox.h"
#include "qgsmaptoolshaperegistry.h"


class QSpinBox;

class APP_EXPORT QgsMapToolShapeCircle2TangentsPointMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeCircle2TangentsPointMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class QgsMapToolShapeCircle2TangentsPoint: public QgsMapToolShapeCircleAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeCircle2TangentsPoint( QgsMapToolCapture *parentTool ) : QgsMapToolShapeCircleAbstract( QgsMapToolShapeCircle2TangentsPointMetadata::TOOL_ID, parentTool ) {}
    ~QgsMapToolShapeCircle2TangentsPoint() override;

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;

    void clean() override;


  public slots:
    void radiusSpinBoxChanged( double radius );

  private:
    //! Compute 4 possible centers
    void getPossibleCenter();

    //! (re-)create the spin box to enter the radius of the circle
    void createRadiusSpinBox();
    //! delete the spin box to enter the radius of the circle, if it exists
    void deleteRadiusSpinBox();

    QDoubleSpinBox *mRadiusSpinBox = nullptr;

    double mRadius = 0.0;
    QVector<QgsPoint> mCenters;
    QVector<QgsGeometryRubberBand *> mRubberBands;
};

#endif // QGSMAPTOOLSHAPECIRCLE2TANGENTSPOINT_H
