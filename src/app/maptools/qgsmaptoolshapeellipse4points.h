/***************************************************************************
    qgsmaptoolshapeellipse4points.h  -  map tool for adding ellipse
    from 4 points
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPEELLIPSE4POINTS_H
#define QGSMAPTOOLSHAPEELLIPSE4POINTS_H

#include "qgis_app.h"
#include "qgsmaptoolshapeellipseabstract.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeEllipse4PointsMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeEllipse4PointsMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeEllipse4Points : public QgsMapToolShapeEllipseAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeEllipse4Points( QgsMapToolCapture *parentTool )
      : QgsMapToolShapeEllipseAbstract( QgsMapToolShapeEllipse4PointsMetadata::TOOL_ID, parentTool )
    {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPEELLIPSE4POINTS_H
