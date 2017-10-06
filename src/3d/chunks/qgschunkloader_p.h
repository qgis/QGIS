/***************************************************************************
  qgschunkloader_p.h
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

#ifndef QGSCHUNKLOADER_P_H
#define QGSCHUNKLOADER_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgschunkqueuejob_p.h"

/**
 * \ingroup 3d
 * Base class for jobs that load chunks
 * \since QGIS 3.0
 */
class QgsChunkLoader : public QgsChunkQueueJob
{
    Q_OBJECT
  public:
    //! Construct chunk loader for a node
    QgsChunkLoader( QgsChunkNode *node )
      : QgsChunkQueueJob( node )
    {
    }

    virtual ~QgsChunkLoader() = default;

    /**
     * Run in main thread to use loaded data.
     * Returns entity attached to the given parent entity in disabled state
     */
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) = 0;

};


/**
 * \ingroup 3d
 * Factory for chunk loaders for a particular type of entity
 * \since QGIS 3.0
 */
class QgsChunkLoaderFactory
{
  public:
    virtual ~QgsChunkLoaderFactory() = default;

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const = 0;
};

/// @endcond

#endif // QGSCHUNKLOADER_P_H
