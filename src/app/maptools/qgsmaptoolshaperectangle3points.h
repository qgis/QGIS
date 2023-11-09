/***************************************************************************
   qgsmaptoolshaperectangle3points.h  -  map tool for adding rectangle
   from 3 points
   ---------------------
   begin                : September 2017
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

#ifndef QGSMAPTOOLSHAPERECTANGLE3POINTS_H
#define QGSMAPTOOLSHAPERECTANGLE3POINTS_H

#include "qgsmaptoolshaperectangleabstract.h"
#include "qgis_app.h"
#include "qgsmaptoolshaperegistry.h"

class APP_EXPORT QgsMapToolShapeRectangle3PointsMetadata : public QgsMapToolShapeMetadata
{
    Q_GADGET
  public:
    enum class CreateMode
    {
      Distance,
      Projected,
    };
    Q_ENUM( CreateMode )

    QgsMapToolShapeRectangle3PointsMetadata( CreateMode createMode )
      : QgsMapToolShapeMetadata()
      , mCreateMode( createMode )
    {}

    static const QString TOOL_ID_PROJECTED;
    static const QString TOOL_ID_DISTANCE;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;

  private:
    CreateMode mCreateMode;

};

class APP_EXPORT QgsMapToolShapeRectangle3Points: public QgsMapToolShapeRectangleAbstract
{
    Q_OBJECT

  public:

    QgsMapToolShapeRectangle3Points( const QString &id, QgsMapToolShapeRectangle3PointsMetadata::CreateMode createMode, QgsMapToolCapture *parentTool );

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;

  private:
    QgsMapToolShapeRectangle3PointsMetadata::CreateMode mCreateMode;
};

#endif // QGSMAPTOOLSHAPERECTANGLE3POINTS_H
