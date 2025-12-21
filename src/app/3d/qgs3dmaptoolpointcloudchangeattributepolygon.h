/***************************************************************************
    qgs3dmaptoolpointcloudchangeattributepolygon.h
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

#ifndef QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTEPOLYGON_H
#define QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTEPOLYGON_H

#include "qgs3dmaptoolpointcloudchangeattribute.h"

#include <QPointF>

class QgsRubberBand3D;
class QgsPointXY;

class Qgs3DMapToolPointCloudChangeAttributePolygon : public Qgs3DMapToolPointCloudChangeAttribute
{
    Q_OBJECT

  public:
    /**
     * Tool types used by \a Qgs3DMapToolPolygon
     */
    enum ToolType
    {
      //! Polygon defined by vertices
      Polygon,
      //! Polygon defined by 2 vertices and canvas top edge
      AboveLine,
      //! Polygon defined by 2 vertices and canvas bottom edge
      BelowLine,
    };
    Qgs3DMapToolPointCloudChangeAttributePolygon( Qgs3DMapCanvas *canvas, ToolType type );
    ~Qgs3DMapToolPointCloudChangeAttributePolygon() override;

    void activate() override;
    void deactivate() override;

  private slots:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  private:
    void run() override;
    void restart() override;

    QVector<QgsPointXY> mScreenPoints;
    std::unique_ptr<QgsRubberBand3D> mPolygonRubberBand;
    std::unique_ptr<QgsRubberBand3D> mLineRubberBand;
    QPoint mClickPoint;
    ToolType mToolType;
};


#endif //QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTEPOLYGON_H
