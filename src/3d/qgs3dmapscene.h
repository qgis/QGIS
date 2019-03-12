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
  class QPickEvent;
}

namespace Qt3DLogic
{
  class QFrameAction;
}

namespace Qt3DExtras
{
  class QForwardRenderer;
}

class QgsAbstract3DEngine;
class QgsMapLayer;
class QgsCameraController;
class Qgs3DMapScenePickHandler;
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
    Qgs3DMapScene( const Qgs3DMapSettings &map, QgsAbstract3DEngine *engine );

    //! Returns camera controller
    QgsCameraController *cameraController() { return mCameraController; }
    //! Returns terrain entity (may be temporarily null)
    QgsTerrainEntity *terrainEntity() { return mTerrain; }

    //! Resets camera view to show the whole scene (top view)
    void viewZoomFull();

    //! Returns number of pending jobs of the terrain entity
    int terrainPendingJobsCount() const;

    //! Enumeration of possible states of the 3D scene
    enum SceneState
    {
      Ready,     //!< The scene is fully loaded/updated
      Updating,  //!< The scene is still being loaded/updated
    };

    //! Returns the current state of the scene
    SceneState sceneState() const { return mSceneState; }

    //! Registers an object that will get results of pick events on 3D entities. Does not take ownership of the pick handler. Adds object picker components to 3D entities.
    void registerPickHandler( Qgs3DMapScenePickHandler *pickHandler );
    //! Unregisters previously registered pick handler. Pick handler is not deleted. Also removes object picker components from 3D entities.
    void unregisterPickHandler( Qgs3DMapScenePickHandler *pickHandler );

    /**
     * Given screen error (in pixels) and distance from camera (in 3D world coordinates), this function
     * estimates the error in world space. Takes into account camera's field of view and the screen (3D view) size.
     */
    float worldSpaceError( float epsilon, float distance );

  signals:
    //! Emitted when the current terrain entity is replaced by a new one
    void terrainEntityChanged();
    //! Emitted when the number of terrain's pending jobs changes
    void terrainPendingJobsCountChanged();
    //! Emitted when the scene's state has changed
    void sceneStateChanged();

  private slots:
    void onCameraChanged();
    void onFrameTriggered( float dt );
    void createTerrain();
    void onLayerRenderer3DChanged();
    void onLayersChanged();
    void createTerrainDeferred();
    void onBackgroundColorChanged();
    void onLayerEntityPickEvent( Qt3DRender::QPickEvent *event );
    void updateLights();
    void updateCameraLens();
  private:
    void addLayerEntity( QgsMapLayer *layer );
    void removeLayerEntity( QgsMapLayer *layer );
    void addCameraViewCenterEntity( Qt3DRender::QCamera *camera );
    void setSceneState( SceneState state );
    void updateSceneState();
    void updateScene();
    bool updateCameraNearFarPlanes();

  private:
    const Qgs3DMapSettings &mMap;
    QgsAbstract3DEngine *mEngine = nullptr;
    //! Provides a way to have a synchronous function executed each frame
    Qt3DLogic::QFrameAction *mFrameAction = nullptr;
    QgsCameraController *mCameraController = nullptr;
    QgsTerrainEntity *mTerrain = nullptr;
    QList<QgsChunkedEntity *> mChunkEntities;
    //! Entity that shows view center - useful for debugging camera issues
    Qt3DCore::QEntity *mEntityCameraViewCenter = nullptr;
    //! Keeps track of entities that belong to a particular layer
    QMap<QgsMapLayer *, Qt3DCore::QEntity *> mLayerEntities;
    bool mTerrainUpdateScheduled = false;
    SceneState mSceneState = Ready;
    //! List of currently registered pick handlers (used by identify tool)
    QList<Qgs3DMapScenePickHandler *> mPickHandlers;
    //! List of lights in the scene
    QList<Qt3DCore::QEntity *> mLightEntities;
};

#endif // QGS3DMAPSCENE_H
