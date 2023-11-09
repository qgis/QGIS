/***************************************************************************
    qgsmaptoolshapecircle3points.h  -  map tool for adding circle
    from 3 points
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

#ifndef QGSMAPTOOLSHAPECIRCLE3POINTS_H
#define QGSMAPTOOLSHAPECIRCLE3POINTS_H

#include "qgsmaptoolshapecircleabstract.h"
#include "qgis_app.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeCircle3PointsMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeCircle3PointsMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeCircle3Points: public QgsMapToolShapeCircleAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeCircle3Points( QgsMapToolCapture *parentTool ) : QgsMapToolShapeCircleAbstract( QgsMapToolShapeCircle3PointsMetadata::TOOL_ID, parentTool ) {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPECIRCLE3POINTS_H
