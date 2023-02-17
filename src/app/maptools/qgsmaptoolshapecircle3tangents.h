/***************************************************************************
    qgsmaptoolshapecircle3tangents.h  -  map tool for adding circle
    from 3 tangents
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

#ifndef QGSMAPTOOLSHAPECIRCLE3TANGENTS_H
#define QGSMAPTOOLSHAPECIRCLE3TANGENTS_H

#include "qgspointlocator.h"
#include "qgsmaptoolshapecircleabstract.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeCircle3TangentsMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeCircle3TangentsMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class QgsMapToolShapeCircle3Tangents: public QgsMapToolShapeCircleAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeCircle3Tangents( QgsMapToolCapture *parentTool ) : QgsMapToolShapeCircleAbstract( QgsMapToolShapeCircle3TangentsMetadata::TOOL_ID, parentTool ) {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void clean() override;

  private:
    //! Snapped points on the segments. Useful to determine which circle to choose in case of there are two parallels
    QVector<QgsPoint> mPosPoints;
};

#endif // QGSMAPTOOLSHAPECIRCLE3TANGENTS_H
