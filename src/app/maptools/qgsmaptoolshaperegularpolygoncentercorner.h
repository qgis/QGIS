/***************************************************************************
    qgsmaptoolshaperegularpolygoncentercorner.h  -  map tool for adding regular
    polygon from center and a corner
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

#ifndef QGSMAPTOOLSHAPEREGULARPOLYGONCENTERCORNER_H
#define QGSMAPTOOLSHAPEREGULARPOLYGONCENTERCORNER_H

#include "qgsmaptoolshaperegularpolygonabstract.h"
#include "qgis_app.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeRegularPolygonCenterCornerMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeRegularPolygonCenterCornerMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeRegularPolygonCenterCorner: public QgsMapToolShapeRegularPolygonAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeRegularPolygonCenterCorner( QgsMapToolCapture *parentTool )
      : QgsMapToolShapeRegularPolygonAbstract( QgsMapToolShapeRegularPolygonCenterCornerMetadata::TOOL_ID, parentTool )
    {}
    ~QgsMapToolShapeRegularPolygonCenterCorner() override;

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPEREGULARPOLYGONCENTERCORNER_H
