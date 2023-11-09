/***************************************************************************
  qgs3dmaptoolidentify.h
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPTOOLIDENTIFY_H
#define QGS3DMAPTOOLIDENTIFY_H

#include "qgs3dmaptool.h"

#include <memory>
#include <QDebug>
#include <QPoint>

class Qgs3DMapToolIdentify : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolIdentify( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolIdentify() override;

    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;

    void activate() override;
    void deactivate() override;

    QCursor cursor() const override;
  signals:
    void mouseReleased( QMouseEvent *event );

  private slots:

  private:
    bool mIsActive = false;

    //! Check if mouse was moved between mousePress and mouseRelease
    bool mMouseHasMoved = false;
    QPoint mMouseClickPos;
};

#endif // QGS3DMAPTOOLIDENTIFY_H
