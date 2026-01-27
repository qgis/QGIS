/***************************************************************************
    qgsmaptoolshapeellipsecenter3points.h  -  map tool for adding ellipse
    from center and 3 points
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

#ifndef QGSMAPTOOLSHAPEELLIPSECENTER3POINTS_H
#define QGSMAPTOOLSHAPEELLIPSECENTER3POINTS_H

#include "qgis_app.h"
#include "qgsmaptoolshapeellipseabstract.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeEllipseCenter3PointsMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeEllipseCenter3PointsMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeEllipseCenter3Points : public QgsMapToolShapeEllipseAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeEllipseCenter3Points( QgsMapToolCapture *parentTool )
      : QgsMapToolShapeEllipseAbstract( QgsMapToolShapeEllipseCenter3PointsMetadata::TOOL_ID, parentTool )
    {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPEELLIPSECENTER3POINTS_H
