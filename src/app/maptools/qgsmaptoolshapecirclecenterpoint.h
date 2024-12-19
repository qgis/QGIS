/***************************************************************************
    qgmaptoolshapecirclecenterpoint.h  -  map tool for adding circle
    from center and a point
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

#ifndef QGSMAPTOOLSHAPECIRCLECENTERPOINT_H
#define QGSMAPTOOLSHAPECIRCLECENTERPOINT_H

#include "qgsmaptoolshapecircleabstract.h"
#include "qgis_app.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeCircleCenterPointMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeCircleCenterPointMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeCircleCenterPoint: public QgsMapToolShapeCircleAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeCircleCenterPoint( QgsMapToolCapture *parentTool ) : QgsMapToolShapeCircleAbstract( QgsMapToolShapeCircleCenterPointMetadata::TOOL_ID, parentTool ) {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPECIRCLECENTERPOINT_H
