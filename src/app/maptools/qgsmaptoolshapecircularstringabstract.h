/***************************************************************************
    qgsmaptoolshapecircularstringabstract.h  -  map tool for adding circular strings
    ---------------------
    begin                : December 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPECIRCULARSTRINGABSTRACT_H
#define QGSMAPTOOLSHAPECIRCULARSTRINGABSTRACT_H

#include "qgsmaptoolshapeabstract.h"
#include "qgis_app.h"

class QgsGeometryRubberBand;


class APP_EXPORT QgsMapToolShapeCircularStringAbstract: public QgsMapToolShapeAbstract
{
    Q_OBJECT
  public:
    QgsMapToolShapeCircularStringAbstract( const QString &id, QgsMapToolCapture *parentTool );
    ~QgsMapToolShapeCircularStringAbstract() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void activate( QgsMapToolCapture::CaptureMode mode, const QgsPoint &lastCapturedMapPoint ) override;

    void clean() override;

    void undo() override;

  protected:

    void addCurveToParentTool();

    //! The rubberband to show the already completed circular strings
    QgsGeometryRubberBand *mRubberBand = nullptr;
    //! The rubberband to show the circular string currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;

    //center point rubber band
    bool mShowCenterPointRubberBand;
    QgsGeometryRubberBand *mCenterPointRubberBand = nullptr;

    void createCenterPointRubberBand();
    void updateCenterPointRubberBand( const QgsPoint &pt );
    void removeCenterPointRubberBand();
};

#endif // QGSMAPTOOLSHAPECIRCULARSTRINGABSTRACT_H
