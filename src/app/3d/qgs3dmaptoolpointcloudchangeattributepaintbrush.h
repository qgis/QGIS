/***************************************************************************
  qgs3dmaptoolpointcloudchangeattributepaintbrush.h
  --------------------------------------
  Date                 : January 2025
  Copyright            : (C) 2025 by Matej Bagar
  Email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTEPAINTBRUSH_H
#define QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTEPAINTBRUSH_H
#include "qgs3dmaptool.h"
#include "qgspoint.h"
#include "qgs3dmaptoolpointcloudchangeattribute.h"

class QgsRubberBand3D;
class Qgs3DMapToolPointCloudChangeAttributePaintbrush : public Qgs3DMapToolPointCloudChangeAttribute
{
    Q_OBJECT

  public:
    explicit Qgs3DMapToolPointCloudChangeAttributePaintbrush( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolPointCloudChangeAttributePaintbrush() override;

    void activate() override;

    void deactivate() override;

    QCursor cursor() const override;

    void restart() override;

  private slots:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseWheelEvent( QWheelEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  private:
    void run() override;
    void generateHighlightArea();

    std::unique_ptr<QgsRubberBand3D> mSelectionRubberBand;
    std::unique_ptr<QgsRubberBand3D> mHighlighterRubberBand;
    QVector<QgsPointXY> mDragPositions = QVector<QgsPointXY>();
    //! Check if mouse was moved between mousePress and mouseRelease
    bool mIsClicked = false;
    //! Check if the tool is selected
    bool mIsActive = false;
    //! Check if the tool or movement is active
    bool mIsMoving = false;
};


#endif //QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTEPAINTBRUSH_H
