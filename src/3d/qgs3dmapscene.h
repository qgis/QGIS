/***************************************************************************
  qgs3dmapscene.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPSCENE_H
#define QGS3DMAPSCENE_H

#include "qgis_3d.h"

#include <Qt3DCore/QEntity>

namespace Qt3DRender
{
  class QRenderSettings;
  class QCamera;
}

namespace Qt3DLogic
{
  class QFrameAction;
}

namespace Qt3DExtras
{
  class QForwardRenderer;
}

class QgsMapLayer;
class QgsCameraController;
class Qgs3DMapSettings;
class QgsTerrainEntity;
class QgsChunkedEntity;

/**
 * \ingroup 3d
 * Entity that encapsulates our 3D scene - contains all other entities (such as terrain) as children.
 * \since QGIS 3.0
 */
class _3D_EXPORT Qgs3DMapScene : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructs a 3D scene based on map settings and Qt 3D renderer configuration
    Qgs3DMapScene( const Qgs3DMapSettings &map, Qt3DExtras::QForwardRenderer *defaultFrameGraph, Qt3DRender::QRenderSettings *renderSettings, Qt3DRender::QCamera *camera, const QRect &viewportRect, Qt3DCore::QNode *parent = nullptr );

    //! Returns camera controller
    QgsCameraController *cameraController() { return mCameraController; }
    //! Returns terrain entity
    QgsTerrainEntity *terrain() { return mTerrain; }

    //! Resets camera view to show the whole scene (top view)
    void viewZoomFull();

  private slots:
    void onCameraChanged();
    void onFrameTriggered( float dt );
    void createTerrain();
    void onLayerRenderer3DChanged();
    void onLayersChanged();
    void createTerrainDeferred();
    void onBackgroundColorChanged();

  private:
    void addLayerEntity( QgsMapLayer *layer );
    void removeLayerEntity( QgsMapLayer *layer );

  private:
    const Qgs3DMapSettings &mMap;
    //! Provides a way to have a synchronous function executed each frame
    Qt3DLogic::QFrameAction *mFrameAction = nullptr;
    QgsCameraController *mCameraController = nullptr;
    QgsTerrainEntity *mTerrain = nullptr;
    //! Forward renderer provided by 3D window
    Qt3DExtras::QForwardRenderer *mForwardRenderer = nullptr;
    QList<QgsChunkedEntity *> mChunkEntities;
    //! Keeps track of entities that belong to a particular layer
    QMap<QgsMapLayer *, Qt3DCore::QEntity *> mLayerEntities;
    bool mTerrainUpdateScheduled = false;
};

#endif // QGS3DMAPSCENE_H
