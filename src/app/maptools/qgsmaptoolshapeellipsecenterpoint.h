/***************************************************************************
    qgmaptoolellipsecenterpoint.h  -  map tool for adding ellipse
    from center and a point
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPEELLIPSECENTERPOINT_H
#define QGSMAPTOOLSHAPEELLIPSECENTERPOINT_H

#include "qgis_app.h"
#include "qgsmaptoolshapeellipseabstract.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeEllipseCenterPointMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeEllipseCenterPointMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeEllipseCenterPoint : public QgsMapToolShapeEllipseAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeEllipseCenterPoint( QgsMapToolCapture *parentTool )
      : QgsMapToolShapeEllipseAbstract( QgsMapToolShapeEllipseCenterPointMetadata::TOOL_ID, parentTool ) {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPEELLIPSECENTERPOINT_H
