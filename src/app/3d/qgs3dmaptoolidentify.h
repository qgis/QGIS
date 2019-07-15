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

namespace Qt3DRender
{
  class QPickEvent;
}

class Qgs3DMapToolIdentifyPickHandler;


class Qgs3DMapToolIdentify : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolIdentify( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolIdentify() override;

    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override { Q_UNUSED( event )}
    void mouseMoveEvent( QMouseEvent *event ) override {Q_UNUSED( event )}

    void activate() override;
    void deactivate() override;

    QCursor cursor() const override;

  private slots:
    void onTerrainPicked( Qt3DRender::QPickEvent *event );
    void onTerrainEntityChanged();

  private:
    std::unique_ptr<Qgs3DMapToolIdentifyPickHandler> mPickHandler;

    friend class Qgs3DMapToolIdentifyPickHandler;
};

#endif // QGS3DMAPTOOLIDENTIFY_H
