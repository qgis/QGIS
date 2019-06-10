/***************************************************************************
  qgs3dmaptoolmeasureline.h
  --------------------------------------
  Date                 : Jun 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPTOOLMEASURELINE_H
#define QGS3DMAPTOOLMEASURELINE_H

#include "qgs3dmaptool.h"

class Qgs3DMapToolMeasureLine : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolMeasureLine() override;

    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override { Q_UNUSED( event )}
    void mouseMoveEvent( QMouseEvent *event ) override {Q_UNUSED( event )}

    void activate() override;
    void deactivate() override;
};

#endif // QGS3DMAPTOOLMEASURELINE_H
