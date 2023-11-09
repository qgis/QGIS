/***************************************************************************
    qgsmaptoolshaperegularpolygoncenterpoint.h  -  map tool for adding regular
    polygon from center and a point
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

#ifndef QGSMAPTOOLSHAPEREGULARPOLYGONCENTERPOINT_H
#define QGSMAPTOOLSHAPEREGULARPOLYGONCENTERPOINT_H

#include "qgsmaptoolshaperegularpolygonabstract.h"
#include "qgis_app.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeRegularPolygonCenterPointMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeRegularPolygonCenterPointMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeRegularPolygonCenterPoint: public QgsMapToolShapeRegularPolygonAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeRegularPolygonCenterPoint( QgsMapToolCapture *parentTool )
      : QgsMapToolShapeRegularPolygonAbstract( QgsMapToolShapeRegularPolygonCenterPointMetadata::TOOL_ID, parentTool )
    {}
    ~QgsMapToolShapeRegularPolygonCenterPoint() override;

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPEREGULARPOLYGONCENTERPOINT_H
