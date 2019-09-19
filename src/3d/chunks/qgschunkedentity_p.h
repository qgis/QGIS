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

/**
 * \ingroup 3d
 * Implementation of entity that handles chunks of data organized in quadtree with loading data when necessary
 * based on data error and unloading of data when data are not necessary anymore
 * \since QGIS 3.0
 */
class QgsChunkedEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructs a chunked entity
    QgsChunkedEntity( const QgsAABB &rootBbox, float rootError, float mTau, int mMaxLevel, QgsChunkLoaderFactory *loaderFactory, Qt3DCore::QNode *parent = nullptr );
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

  protected:
    //! Cancels the background job that is currently in progress
    void cancelActiveJob();
    //! Sets whether the entity needs to get active nodes updated
    void setNeedsUpdate( bool needsUpdate ) { mNeedsUpdate = needsUpdate; }

  private:
    void update( QgsChunkNode *node, const SceneState &state );

    //! make sure that the chunk will be loaded soon (if not loaded yet) and not unloaded anytime soon (if loaded already)
    void requestResidency( QgsChunkNode *node );

    void startJob();

  private slots:
    void onActiveJobFinished();

  signals:
    //! Emitted when the number of pending jobs changes (some jobs have finished or some jobs have been just created)
    void pendingJobsCountChanged();

  protected:
    //! root node of the quadtree hierarchy
    QgsChunkNode *mRootNode = nullptr;
    //! A chunk has been loaded recently? let's display it!
    bool mNeedsUpdate = false;
    //! max. allowed screen space error
    float mTau;
    //! maximum allowed depth of quad tree
    int mMaxLevel;
    //! factory that creates loaders for individual chunk nodes
    QgsChunkLoaderFactory *mChunkLoaderFactory = nullptr;
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

    //! max. length for replacement queue
    int mMaxLoadedChunks = 512;

    //! Entity that shows bounding boxes of active chunks (NULLPTR if not enabled)
    QgsChunkBoundsEntity *mBboxesEntity = nullptr;

    //! job that is currently being processed (asynchronously in a worker thread)
    QgsChunkQueueJob *mActiveJob = nullptr;
};

/// @endcond

#endif // QGSCHUNKEDENTITY_P_H
