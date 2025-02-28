/***************************************************************************
    qgs3dmaptoolpolygon.h
    ---------------------
    begin                : February 2025
    copyright            : (C) 2025 by Matej Bagar
    email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPTOOLPOLYGON_H
#define QGS3DMAPTOOLPOLYGON_H

#include "qgs3dmaptoolpointcloudchangeattribute.h"

#include <QPointF>

class QgsRubberBand3D;
class QgsPointXY;

class Qgs3DMapToolPolygon : public Qgs3DMapToolPointCloudChangeAttribute
{
    Q_OBJECT

  public:
    Qgs3DMapToolPolygon( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolPolygon() override;

    void activate() override;
    void deactivate() override;

    bool allowsCameraControls() const override { return false; }

  private slots:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  private:
    void run() override;
    void restart() override;

    QVector<QgsPointXY> mScreenPoints;
    QgsRubberBand3D *mPolygonRubberBand = nullptr;
    QPoint mClickPoint;
    bool mIsMoving = false;
};


#endif //QGS3DMAPTOOLPOLYGON_H
