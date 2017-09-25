#ifndef CHUNKEDENTITY_H
#define CHUNKEDENTITY_H

#include <Qt3DCore/QEntity>

class AABB;
class ChunkNode;
class ChunkList;
class ChunkQueueJob;
class ChunkLoaderFactory;
class ChunkBoundsEntity;
class ChunkQueueJobFactory;

#include <QVector3D>
#include <QMatrix4x4>

/** \ingroup 3d
 * Records some bits about the scene
 * \since QGIS 3.0
 */
class SceneState
{
  public:
    QVector3D cameraPos;
    float cameraFov;
    int screenSizePx;

    QMatrix4x4 viewProjectionMatrix; //!< For frustum culling
};

#include <QTime>

/** \ingroup 3d
 * Implementation of entity that handles chunks of data organized in quadtree with loading data when necessary
 * based on data error and unloading of data when data are not necessary anymore
 * \since QGIS 3.0
 */
class ChunkedEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructs a chunked entity
    ChunkedEntity( const AABB &rootBbox, float rootError, float tau, int maxLevel, ChunkLoaderFactory *loaderFactory, Qt3DCore::QNode *parent = nullptr );
    ~ChunkedEntity();

    //! Called when e.g. camera changes and entity may need updated
    void update( const SceneState &state );

    bool needsUpdate; //!< A chunk has been loaded recently - let's display it!

    //! Determines whether bounding boxes of tiles should be shown (for debugging)
    void setShowBoundingBoxes( bool enabled );

    //! update already loaded nodes (add to the queue)
    void updateNodes( const QList<ChunkNode *> &nodes, ChunkQueueJobFactory *updateJobFactory );

    //! Returns list of active nodes - i.e. nodes that are get rendered
    QList<ChunkNode *> getActiveNodes() const { return activeNodes; }
    //! Returns the root node of the whole quadtree hierarchy of nodes
    ChunkNode *getRootNode() const { return rootNode; }

  protected:
    //! Cancels the background job that is currently in progress
    void cancelActiveJob();

  private:
    void update( ChunkNode *node, const SceneState &state );

    //! make sure that the chunk will be loaded soon (if not loaded yet) and not unloaded anytime soon (if loaded already)
    void requestResidency( ChunkNode *node );

    void startJob();

  private slots:
    void onActiveJobFinished();

  protected:
    //! root node of the quadtree hierarchy
    ChunkNode *rootNode;
    //! max. allowed screen space error
    float tau;
    //! maximum allowed depth of quad tree
    int maxLevel;
    //! factory that creates loaders for individual chunk nodes
    ChunkLoaderFactory *chunkLoaderFactory;
    //! queue of chunks to be loaded
    ChunkList *chunkLoaderQueue;
    //! queue of chunk to be eventually replaced
    ChunkList *replacementQueue;

    QList<ChunkNode *> activeNodes;
    int frustumCulled;

    // TODO: max. length for loading queue

    QTime currentTime;

    //! max. length for replacement queue
    int maxLoadedChunks;

    ChunkBoundsEntity *bboxesEntity;

    //! job that is currently being processed (asynchronously in a worker thread)
    ChunkQueueJob *activeJob = nullptr;
};


#endif // CHUNKEDENTITY_H
