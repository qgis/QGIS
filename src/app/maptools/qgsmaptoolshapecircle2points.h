/***************************************************************************
    qgsmaptoolshapecircle2points.h  -  map tool for adding circle
    from 2 points
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPECIRCLE2POINTS_H
#define QGSMAPTOOLSHAPECIRCLE2POINTS_H

#include "qgsmaptoolshapecircleabstract.h"
#include "qgis_app.h"
#include "qgsmaptoolshaperegistry.h"


class APP_EXPORT QgsMapToolShapeCircle2PointsMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeCircle2PointsMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeCircle2Points : public QgsMapToolShapeCircleAbstract
{
  public:
    QgsMapToolShapeCircle2Points( QgsMapToolCapture *parentTool )
      : QgsMapToolShapeCircleAbstract( QgsMapToolShapeCircle2PointsMetadata::TOOL_ID, parentTool )
    {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPECIRCLE2POINTS_H
