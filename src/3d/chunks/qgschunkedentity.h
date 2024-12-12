/***************************************************************************
  qgschunkedentity.h
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

#ifndef QGSCHUNKEDENTITY_H
#define QGSCHUNKEDENTITY_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgs3dmapsceneentity.h"
#include <numeric>

#define SIP_NO_FILE

class QgsAABB;
class QgsChunkNode;
class QgsChunkList;
class QgsChunkQueueJob;
class QgsChunkLoaderFactory;
class QgsChunkBoundsEntity;
class QgsChunkQueueJobFactory;

namespace QgsRayCastingUtils
{
  class Ray3D;
  struct RayCastContext;
  struct RayHit;
} // namespace QgsRayCastingUtils

#include <QVector3D>
#include <QMatrix4x4>

#include <QTime>

#include "qgschunknode.h"


/**
 * \ingroup 3d
 * \brief Implementation of entity that handles chunks of data organized in quadtree with loading data when necessary
 * based on data error and unloading of data when data are not necessary anymore
 */
class QgsChunkedEntity : public Qgs3DMapSceneEntity
{
    Q_OBJECT
  public:
    //! Constructs a chunked entity
    QgsChunkedEntity( Qgs3DMapSettings *mapSettings, float tau, QgsChunkLoaderFactory *loaderFactory, bool ownsFactory, int primitivesBudget = std::numeric_limits<int>::max(), Qt3DCore::QNode *parent = nullptr );
    ~QgsChunkedEntity() override;

    //! Called when e.g. camera changes and entity may need updated
    void handleSceneUpdate( const SceneContext &sceneContext ) override;

    //! Returns number of jobs pending for this entity until it is fully loaded/updated in the current view
    int pendingJobsCount() const override;

    //! Returns whether the entity needs update of active nodes
    bool needsUpdate() const override { return mNeedsUpdate; }

    QgsRange<float> getNearFarPlaneRange( const QMatrix4x4 &viewMatrix ) const override;

    //! Determines whether bounding boxes of tiles should be shown (for debugging)
    void setShowBoundingBoxes( bool enabled );

    //! update already loaded nodes (add to the queue)
    void updateNodes( const QList<QgsChunkNode *> &nodes, QgsChunkQueueJobFactory *updateJobFactory );

    //! Returns list of active nodes - i.e. nodes that are get rendered
    QList<QgsChunkNode *> activeNodes() const { return mActiveNodes; }
    //! Returns the root node of the whole quadtree hierarchy of nodes
    QgsChunkNode *rootNode() const { return mRootNode; }

    /**
     * Checks if \a ray intersects the entity by using the specified parameters in \a context and returns information about the hits.
     * This method is typically used by map tools that need to identify the exact location on a 3d entity that the mouse cursor points at,
     * as well as properties of the intersected entity (fid for vector layers, point cloud attributes for point cloud layers etc). The camera position
     * is used as the ray's origin in that case.
     * The number of successful hits returned depends on the entity's implementation (eg. point cloud entities use a tolerance and return multiple 'near' hits
     * \note The ray uses World coordinates.
     * \since QGIS 3.32
     */
    virtual QVector<QgsRayCastingUtils::RayHit> rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QgsRayCastingUtils::RayCastContext &context ) const;

  protected:
    //! Cancels the background job that is currently in progress
    void cancelActiveJob( QgsChunkQueueJob *job );
    void cancelActiveJobs();
    //! Sets whether the entity needs to get active nodes updated
    void setNeedsUpdate( bool needsUpdate ) { mNeedsUpdate = needsUpdate; }

  private:
    void update( QgsChunkNode *node, const SceneContext &sceneContext );

    //! Removes chunks for loading queue that are currently not needed
    void pruneLoaderQueue( const SceneContext &sceneContext );

    //! make sure that the chunk will be loaded soon (if not loaded yet) and not unloaded anytime soon (if loaded already)
    void requestResidency( QgsChunkNode *node );

    void startJobs();
    QgsChunkQueueJob *startJob( QgsChunkNode *node );

    int unloadNodes();

  private slots:
    void onActiveJobFinished();

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

    bool mIsValid = true;

    int mPrimitivesBudget = std::numeric_limits<int>::max();
};

/// @endcond

#endif // QGSCHUNKEDENTITY_H
