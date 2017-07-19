#ifndef SCENE_H
#define SCENE_H

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

class CameraController;
class Map3D;
class Terrain;
class ChunkedEntity;

/**
 * Entity that encapsulates our 3D scene - contains all other entities (such as terrain) as children.
 */
class Scene : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    Scene( const Map3D &map, Qt3DExtras::QForwardRenderer *defaultFrameGraph, Qt3DRender::QRenderSettings *renderSettings, Qt3DRender::QCamera *camera, const QRect &viewportRect, Qt3DCore::QNode *parent = nullptr );

    CameraController *cameraController() { return mCameraController; }
    Terrain *terrain() { return mTerrain; }

  private slots:
    void onCameraChanged();
    void onFrameTriggered( float dt );

  private:
    //! Provides a way to have a synchronous function executed each frame
    Qt3DLogic::QFrameAction *mFrameAction;
    CameraController *mCameraController;
    Terrain *mTerrain;
    QList<ChunkedEntity *> chunkEntities;
};

#endif // SCENE_H
