/***************************************************************************
    qgsmaptoolshaperectanglecenter.h  -  map tool for adding rectangle
    from center and a point
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

#ifndef QGSMAPTOOLSHAPERECTANGLECENTER_H
#define QGSMAPTOOLSHAPERECTANGLECENTER_H

#include "qgsmaptoolshaperectangleabstract.h"
#include "qgis_app.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeRectangleCenterMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeRectangleCenterMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeRectangleCenter: public QgsMapToolShapeRectangleAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeRectangleCenter( QgsMapToolCapture *parentTool )
      : QgsMapToolShapeRectangleAbstract( QgsMapToolShapeRectangleCenterMetadata::TOOL_ID, parentTool )
    {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
};

#endif // QGSMAPTOOLSHAPERECTANGLECENTER_H
