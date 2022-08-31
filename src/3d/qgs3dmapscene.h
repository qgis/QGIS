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

#include "qgsfeatureid.h"
#include "qgsshadowrenderingframegraph.h"
#include "qgsray3d.h"
#include "qgscameracontroller.h"

namespace Qt3DRender
{
  class QRenderSettings;
  class QCamera;
  class QPickEvent;
  class QObjectPicker;
}

namespace Qt3DLogic
{
  class QFrameAction;
}

namespace Qt3DExtras
{
  class QForwardRenderer;
  class QSkyboxEntity;
}

class Qgs3DAxis;
class QgsAbstract3DEngine;
class QgsAbstract3DRenderer;
class QgsMapLayer;
class Qgs3DMapScenePickHandler;
class Qgs3DMapSettings;
class QgsTerrainEntity;
class QgsChunkedEntity;
class QgsSkyboxEntity;
class QgsSkyboxSettings;
class Qgs3DMapExportSettings;
class QgsShadowRenderingFrameGraph;
class QgsPostprocessingEntity;
class QgsChunkNode;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Entity that encapsulates our 3D scene - contains all other entities (such as terrain) as children.
 * \note Not available in Python bindings
 * \since QGIS 3.0
 */
class _3D_EXPORT Qgs3DMapScene : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructs a 3D scene based on map settings and Qt 3D renderer configuration
    Qgs3DMapScene( Qgs3DMapSettings &map, QgsAbstract3DEngine *engine );

    //! Returns camera controller
    QgsCameraController *cameraController() { return mCameraController; }
    //! Returns terrain entity (may be temporarily NULLPTR)
    QgsTerrainEntity *terrainEntity() { return mTerrain; }

    //! Resets camera view to show the whole scene (top view)
    void viewZoomFull();

    /**
     * Resets camera view to show the extent \a extent (top view)
     *
     * \since QGIS 3.26
     */
    void setViewFrom2DExtent( const QgsRectangle &extent );

    /**
     * Calculates the 2D extent viewed by the 3D camera as the vertices of the viewed trapezoid
     *
     * \since QGIS 3.26
     */
    QVector<QgsPointXY> viewFrustum2DExtent();

    //! Returns number of pending jobs of the terrain entity
    int terrainPendingJobsCount() const;

    /**
     * Returns number of pending jobs for all chunked entities
     * \since QGIS 3.12
     */
    int totalPendingJobsCount() const;

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

    //! Exports the scene according to the scene export settings
    void exportScene( const Qgs3DMapExportSettings &exportSettings );

    /**
     * Returns the active chunk nodes of \a layer
     *
     * \since QGIS 3.18
     */
    QVector<const QgsChunkNode *> getLayerActiveChunkNodes( QgsMapLayer *layer ) SIP_SKIP;

    /**
     * Returns the scene extent in the map's CRS
     *
     * \since QGIS 3.20
     */
    QgsRectangle sceneExtent();

    /**
     * Returns the 3D axis object
     *
     * \since QGIS 3.26
     */
    Qgs3DAxis *get3DAxis() { return m3DAxis; }

    /**
     * Returns the abstract 3D engine
     *
     * \since QGIS 3.26
     */
    QgsAbstract3DEngine *engine() { return mEngine; }

  signals:
    //! Emitted when the current terrain entity is replaced by a new one
    void terrainEntityChanged();
    //! Emitted when the number of terrain's pending jobs changes
    void terrainPendingJobsCountChanged();

    /**
     * Emitted when the total number of pending jobs changes
     * \since QGIS 3.12
     */
    void totalPendingJobsCountChanged();
    //! Emitted when the scene's state has changed
    void sceneStateChanged();

    //! Emitted when the FPS count changes
    void fpsCountChanged( float fpsCount );
    //! Emitted when the FPS counter is activated or deactivated
    void fpsCounterEnabledChanged( bool fpsCounterEnabled );

    /**
     * Emitted when the viewed 2D extent seen by the 3D camera has changed
     *
     * \since QGIS 3.26
     */
    void viewed2DExtentFrom3DChanged( QVector<QgsPointXY> extent );

  public slots:
    //! Updates the temporale entities
    void updateTemporal();

  private slots:
    void onCameraChanged();
    void onFrameTriggered( float dt );
    void createTerrain();
    void onLayerRenderer3DChanged();
    void onLayersChanged();
    void createTerrainDeferred();
    void onBackgroundColorChanged();
    void onLayerEntityPickedObject( Qt3DRender::QPickEvent *pickEvent, QgsFeatureId fid );
    void updateLights();
    void updateCameraLens();
    void onRenderersChanged();
    void onSkyboxSettingsChanged();
    void onShadowSettingsChanged();
    void onAmbientOcclusionSettingsChanged();
    void onEyeDomeShadingSettingsChanged();
    void onDebugShadowMapSettingsChanged();
    void onDebugDepthMapSettingsChanged();
    void onCameraMovementSpeedChanged();
    void onCameraNavigationModeChanged();
    void onDebugOverlayEnabledChanged();

    void on3DAxisSettingsChanged();

    bool updateCameraNearFarPlanes();

  private:
    void addLayerEntity( QgsMapLayer *layer );
    void removeLayerEntity( QgsMapLayer *layer );
    void addCameraViewCenterEntity( Qt3DRender::QCamera *camera );
    void addCameraRotationCenterEntity( QgsCameraController *controller );
    void setSceneState( SceneState state );
    void updateSceneState();
    void updateScene();
    void finalizeNewEntity( Qt3DCore::QEntity *newEntity );
    int maximumTextureSize() const;

  private:
    Qgs3DMapSettings &mMap;
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
    QMap<const QgsAbstract3DRenderer *, Qt3DCore::QEntity *> mRenderersEntities;
    bool mTerrainUpdateScheduled = false;
    SceneState mSceneState = Ready;
    //! List of currently registered pick handlers (used by identify tool)
    QList<Qgs3DMapScenePickHandler *> mPickHandlers;
    //! List of lights in the scene
    QList<Qt3DCore::QEntity *> mLightEntities;
    QList<QgsMapLayer *> mModelVectorLayers;
    QgsSkyboxEntity *mSkybox = nullptr;
    //! Entity that shows rotation center = useful for debugging camera issues
    Qt3DCore::QEntity *mEntityRotationCenter = nullptr;

    //! 3d axis visualization
    Qgs3DAxis *m3DAxis = nullptr;

};

#endif // QGS3DMAPSCENE_H
