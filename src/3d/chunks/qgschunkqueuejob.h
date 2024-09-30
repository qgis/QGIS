/***************************************************************************
  qgschunkqueuejob.h
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

#ifndef QGSCHUNKQUEUEJOB_H
#define QGSCHUNKQUEUEJOB_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

class QgsChunkNode;

namespace Qt3DCore
{
  class QEntity;
}

#define SIP_NO_FILE

#include <QObject>

/**
 * \ingroup 3d
 * \brief  Base class for chunk queue jobs.
 *
 * Job implementations start their work when they are created
 * and all work is done asynchronously, i.e. constructor should exit as soon as possible and
 * all work should be done in a worker thread. Once the job is done, finished() signal is emitted
 * and will be processed by the parent chunked entity.
 *
 * There are currently two types of queue jobs:
 *
 * 1. chunk loaders: prepares all data needed for creation of entities (ChunkLoader sub-class)
 * 2. chunk updaters: given a chunk with already existing entity, it updates the entity (e.g. update texture or geometry)
 *
 */
class QgsChunkQueueJob : public QObject
{
    Q_OBJECT
  public:
    //! Constructs a job for given chunk node
    QgsChunkQueueJob( QgsChunkNode *node )
      : mNode( node )
    {
    }

    //! Returns chunk node of this job
    QgsChunkNode *chunk() { return mNode; }

    /**
     * Requests that the job gets canceled. The implementation should _not_ wait until
     * the asynchronous job is terminated - it should only indicate to the async code that
     * is should finish as soon as possible. It is responsibility of the object's destructor
     * to make sure that the async code has been terminated before deleting the object.
     *
     * The signal finished() will not be emitted afterwards.
     */
    virtual void cancel();

  signals:
    //! Emitted when the asynchronous job has finished. Not emitted if the job got canceled.
    void finished();

  protected:
    QgsChunkNode *mNode = nullptr;
};

/**
 * \ingroup 3d
 * \brief Base class for factories of chunk queue jobs.
 *
 * Derived classes need to implement createJob() method that will create a specific job for given chunk node.
 */
class QgsChunkQueueJobFactory
{
  public:
    virtual ~QgsChunkQueueJobFactory() = default;

    //! Creates a specific chunk queue job for the chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkQueueJob *createJob( QgsChunkNode *chunk ) = 0;
};

/// @endcond

#endif // QGSCHUNKQUEUEJOB_H
