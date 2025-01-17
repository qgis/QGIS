/***************************************************************************
  qgs3dmaptoolpaintbrush.h
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

#ifndef QGS3DMAPTOOLPAINTBRUSH_H
#define QGS3DMAPTOOLPAINTBRUSH_H
#include "qgs3dmaptool.h"

#include <QPoint>

class QgsRubberBand3D;

class Qgs3DMapToolPaintBrush : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    explicit Qgs3DMapToolPaintBrush( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolPaintBrush() override;

    // Add all the points intersecting the selection rubberband
    void addSelection();
    // Remove all the points intersecting the selection rubberband
    void removeSelection();
    // Resize the selection rubberband
    void resizeSelector();

    void activate() override;

    void deactivate() override;

    QCursor cursor() const override;

    //TODO: looks like this is useless, as camera still responds to input
    bool allowsCameraControls() const override { return false; }

  private slots:
    void handleClick( const QPoint &screenPos );
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseWheelEvent( QWheelEvent *event ) override;

  private:
    std::unique_ptr<QgsRubberBand3D> mRubberBand;
    QPoint mMouseClickPos;
    //! Check if mouse was moved between mousePress and mouseRelease
    bool mMouseHasMoved = false;
    //! Check if the tool is selected
    bool mIsActive = false;
};


#endif //QGS3DMAPTOOLPAINTBRUSH_H
