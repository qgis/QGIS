/***************************************************************************
    qgsmaptoolshapecircularstringradius.h  -  map tool for adding circular strings
    by two points and radius
    ---------------------
    begin                : Feb 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPECIRCULARSTRINGRADIUS_H
#define QGSMAPTOOLSHAPECIRCULARSTRINGRADIUS_H

#include "qgsmaptoolshapecircularstringabstract.h"
#include "qgspoint.h"
#include "qgis_app.h"
#include "qgsmaptoolshaperegistry.h"

class QDoubleSpinBox;

class APP_EXPORT QgsMapToolShapeCircularStringRadiusMetadata : public QgsMapToolShapeMetadata
{
  public:
    QgsMapToolShapeCircularStringRadiusMetadata()
      : QgsMapToolShapeMetadata()
    {}

    static const QString TOOL_ID;

    QString id() const override;
    QString name() const override;
    QIcon icon() const override;
    QgsMapToolShapeAbstract::ShapeCategory category() const override;
    QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentTool ) const override;
};

class APP_EXPORT QgsMapToolShapeCircularStringRadius: public QgsMapToolShapeCircularStringAbstract
{
    Q_OBJECT
  public:
    QgsMapToolShapeCircularStringRadius( QgsMapToolCapture *parentTool )
      : QgsMapToolShapeCircularStringAbstract( QgsMapToolShapeCircularStringRadiusMetadata::TOOL_ID, parentTool )
      , mTemporaryEndPoint( QgsPoint() )
      , mRadius( 0.0 )
    {}

    bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) override;
    void deactivate() override;

  private slots:
    void updateRadiusFromSpinBox( double radius );

  private:
    QgsPoint mTemporaryEndPoint;
    double mRadius;
    QDoubleSpinBox *mRadiusSpinBox = nullptr;
    QgsMapToolCapture::CaptureMode mCaptureMode = QgsMapToolCapture::CaptureMode::CaptureLine;

    //! recalculate the rubberband
    void recalculateRubberBand();
    //! recalculate the temporary rubberband using the given mouse position
    void recalculateTempRubberBand( const QgsPointXY &mousePosition );
    //! (re-)create the spin box to enter the radius
    void createRadiusSpinBox();
    //! delete the spin box to enter the radius, if it exists
    void deleteRadiusSpinBox();
};

#endif // QGSMAPTOOLSHAPECIRCULARSTRINGRADIUS_H
