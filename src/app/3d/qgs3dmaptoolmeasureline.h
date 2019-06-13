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
#include "qgsvectorlayer.h"
#include "qgsvector3d.h"
#include "qgslinestring.h"
#include "qgsfeature.h"

#include <memory>

namespace Qt3DRender
{
  class QPickEvent;
}

class Qgs3DMapToolMeasureLinePickHandler;


class Qgs3DMapToolMeasureLine : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolMeasureLine() override;

    void mousePressEvent( QMouseEvent *event ) override { Q_UNUSED( event )};
    void mouseReleaseEvent( QMouseEvent *event ) override { Q_UNUSED( event )}
    void mouseMoveEvent( QMouseEvent *event ) override {Q_UNUSED( event )}

    void activate() override;
    void deactivate() override;

  private slots:
    void onTerrainPicked( Qt3DRender::QPickEvent *event );
    void onTerrainEntityChanged();

  private:
    std::unique_ptr<Qgs3DMapToolMeasureLinePickHandler> mPickHandler;

    friend class Qgs3DMapToolMeasureLinePickHandler;

    QgsVectorLayer *mMeasurementLayer = nullptr;
    QgsFeature *mMeasurementFeature = nullptr;
    QgsLineString *mMeasurementLine = nullptr;

    void addPointToLine( QgsVector3D point3D );
    void renderMeasurementLine();
};

#endif // QGS3DMAPTOOLMEASURELINE_H
