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

namespace Qt3DRender
{
  class QPickEvent;
}


class Qgs3DMapToolIdentify : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolIdentify( Qgs3DMapCanvas *canvas );

    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override { Q_UNUSED( event );}
    void mouseMoveEvent( QMouseEvent *event ) override {Q_UNUSED( event );}

    void activate() override;
    void deactivate() override;

  private slots:
    void onTerrainPicked( Qt3DRender::QPickEvent *event );
    void onTerrainEntityChanged();

  private:

};

#endif // QGS3DMAPTOOLIDENTIFY_H
