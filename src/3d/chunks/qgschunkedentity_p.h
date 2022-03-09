/***************************************************************************
  qgschunkedentity_p.h
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

#ifndef QGSCHUNKEDENTITY_P_H
#define QGSCHUNKEDENTITY_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DCore/QEntity>
#include <numeric>

#define SIP_NO_FILE

class QgsAABB;
class QgsChunkNode;
class QgsChunkList;
class QgsChunkQueueJob;
class QgsChunkLoaderFactory;
class QgsChunkBoundsEntity;
class QgsChunkQueueJobFactory;

#include <QVector3D>
#include <QMatrix4x4>

#include <QTime>

#include "qgsfeatureid.h"
#include "qgschunknode_p.h"

namespace Qt3DRender
{
  class QPickEvent;
}

/**
 * \ingroup 3d
 * \brief Implementation of entity that handles chunks of data organized in quadtree with loading data when necessary
 * based on data error and unloading of data when data are not necessary anymore
 * \since QGIS 3.0
 */
class QgsChunkedEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructs a chunked entity
    QgsChunkedEntity( float tau, QgsChunkLoaderFactory *loaderFactory, bool ownsFactory,
                      int primitivesBudget = std::numeric_limits<int>::max(),
                      Qt3DCore::QNode *parent = nullptr );
    ~QgsChunkedEntity() override;

    //! Records some bits about the scene (context for update() method)
    struct SceneState
    {
      QVector3D cameraPos;   //!< Camera position
      float cameraFov;       //!< Field of view (in degrees)
      int screenSizePx;      //!< Size of the viewport in pixels
      QMatrix4x4 viewProjectionMatrix; //!< For frustum culling
    };

    //! Called when e.g. camera changes and entity may need updated
    void update( const SceneState &state );

    //! Returns whether the entity needs update of active nodes
    bool needsUpdate() const { return mNeedsUpdate; }

    //! Determines whether bounding boxes of tiles should be shown (for debugging)
    void setShowBoundingBoxes( bool enabled );

    //! update already loaded nodes (add to the queue)
    void updateNodes( const QList<QgsChunkNode *> &nodes, QgsChunkQueueJobFactory *updateJobFactory );

    //! Returns list of active nodes - i.e. nodes that are get rendered
    QList<QgsChunkNode *> activeNodes() const { return mActiveNodes; }
    //! Returns the root node of the whole quadtree hierarchy of nodes
    QgsChunkNode *rootNode() const { return mRootNode; }

    //! Returns number of jobs pending for this entity until it is fully loaded/updated in the current view
    int pendingJobsCount() const;

    //! Enables or disables object picking on this entity. When enabled, pickedObject() signals will be emitted on mouse clicks
    void setPickingEnabled( bool enabled );
    //! Returns whether object picking is currently enabled
    bool hasPickingEnabled() const { return mPickingEnabled; }

    //! Sets whether additive strategy is enabled - see usingAditiveStrategy()
    void setUsingAdditiveStrategy( bool additive ) { mAdditiveStrategy = additive; }

    /**
     * Returns whether additive strategy is enabled.
     * With additive strategy enabled, also all parent nodes are added to active nodes.
     * This is desired when child nodes add more detailed data rather than just replace coarser data in parents.
     */
    bool usingAditiveStrategy() const { return mAdditiveStrategy; }

    /**
     * Sets the limit of the GPU memory used to render the entity
     * \since QGIS 3.26
     */
    void setGpuMemoryLimit( double gpuMemoryLimit ) { mGpuMemoryLimit = gpuMemoryLimit; }

    /**
     * Returns the limit of the GPU memory used to render the entity in megabytes
     * \since QGIS 3.26
     */
    double gpuMemoryLimit() const { return mGpuMemoryLimit; }

    static double calculateEntityGpuMemorySize( Qt3DCore::QEntity *entity );

  protected:
    //! Cancels the background job that is currently in progress
    void cancelActiveJob( QgsChunkQueueJob *job );
    void cancelActiveJobs();
    //! Sets whether the entity needs to get active nodes updated
    void setNeedsUpdate( bool needsUpdate ) { mNeedsUpdate = needsUpdate; }

  private:
    void update( QgsChunkNode *node, const SceneState &state );

    //! make sure that the chunk will be loaded soon (if not loaded yet) and not unloaded anytime soon (if loaded already)
    void requestResidency( QgsChunkNode *node );

    void startJobs();
    QgsChunkQueueJob *startJob( QgsChunkNode *node );

  private slots:
    void onActiveJobFinished();

    void onPickEvent( Qt3DRender::QPickEvent *event );

  signals:
    //! Emitted when the number of pending jobs changes (some jobs have finished or some jobs have been just created)
    void pendingJobsCountChanged();

    //! Emitted when a new 3D entity has been created. Other components can use that to do extra work
    void newEntityCreated( Qt3DCore::QEntity *entity );

    //! Emitted on mouse clicks when picking is enabled and there is a feature under the cursor
    void pickedObject( Qt3DRender::QPickEvent *pickEvent, QgsFeatureId fid );

  protected:
    //! root node of the quadtree hierarchy
    QgsChunkNode *mRootNode = nullptr;
    //! A chunk has been loaded recently? let's display it!
    bool mNeedsUpdate = false;

    /**
     * Maximum allowed screen space error (in pixels).
     * If the value is negative, it means that the screen space error
     * does not need to be taken into account. This essentially means that
     * the chunked entity will try to go deeper into the tree of chunks until
     * it reaches leafs.
     */
    float mTau;
    //! factory that creates loaders for individual chunk nodes
    QgsChunkLoaderFactory *mChunkLoaderFactory = nullptr;
    //! True if entity owns the factory
    bool mOwnsFactory = true;
    //! queue of chunks to be loaded
    QgsChunkList *mChunkLoaderQueue = nullptr;
    //! queue of chunk to be eventually replaced
    QgsChunkList *mReplacementQueue = nullptr;
    //! list of nodes that are being currently used for rendering
    QList<QgsChunkNode *> mActiveNodes;
    //! number of nodes omitted during frustum culling - for the curious ones
    int mFrustumCulled = 0;

    // TODO: max. length for loading queue

    QTime mCurrentTime;

    //! Entity that shows bounding boxes of active chunks (NULLPTR if not enabled)
    QgsChunkBoundsEntity *mBboxesEntity = nullptr;

    //! jobs that are currently being processed (asynchronously in worker threads)
    QList<QgsChunkQueueJob *> mActiveJobs;

    //! If picking is enabled, QObjectPicker objects will be assigned to chunks and pickedObject() signals fired on mouse click
    bool mPickingEnabled = false;

    /**
     * With additive strategy enabled, also all parent nodes are added to active nodes.
     * This is desired when child nodes add more detailed data rather than just replace coarser data in parents.
     */
    bool mAdditiveStrategy = false;

    bool mIsValid = true;

    int mPrimitivesBudget = std::numeric_limits<int>::max();
    double mGpuMemoryLimit = 100.0; // in megabytes
};

/// @endcond

#endif // QGSCHUNKEDENTITY_P_H
