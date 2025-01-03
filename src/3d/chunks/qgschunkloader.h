/***************************************************************************
  qgschunkloader.h
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

#ifndef QGSCHUNKLOADER_H
#define QGSCHUNKLOADER_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgis_3d.h"
#include "qgschunkqueuejob.h"
#include "qgsbox3d.h"

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Base class for jobs that load chunks
 *
 * \note Not available in Python bindings
 *
 */
class QgsChunkLoader : public QgsChunkQueueJob
{
    Q_OBJECT
  public:
    //! Construct chunk loader for a node
    QgsChunkLoader( QgsChunkNode *node )
      : QgsChunkQueueJob( node ) {}

    /**
     * Run in main thread to use loaded data.
     * Returns entity attached to the given parent entity in disabled state
     */
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) = 0;
};


/**
 * \ingroup 3d
 * \brief Factory for chunk loaders for a particular type of entity
 */
class QgsChunkLoaderFactory : public QObject
{
    Q_OBJECT
  public:
    virtual ~QgsChunkLoaderFactory() = default;

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const = 0;

    //! Returns the primitives count for the chunk \a node
    virtual int primitivesCount( QgsChunkNode *node ) const
    {
      Q_UNUSED( node );
      return 0;
    }

    //! Creates root node of the hierarchy. Ownership of the returned object is passed to the caller.
    virtual QgsChunkNode *createRootNode() const = 0;
    //! Creates child nodes for the given node. Ownership of the returned objects is passed to the caller.
    virtual QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const = 0;

    /**
     * Returns true if createChildren() is ready to return node's children (i.e. it has
     * enough hierarchy information available). If not, createChildren() should not be called,
     * but prepareChildren() should be called first, and only after childrenPrepared() gets
     * emitted, it is safe to call createChildren().
     *
     * The default implementation returns true. This only needs to be implemented when the factory
     * would otherwise need to do blocking network requests in createChildren() to avoid GUI freeze.
     * \see prepareChildren()
     * \see createChildren()
     */
    virtual bool canCreateChildren( QgsChunkNode *node )
    {
      Q_UNUSED( node );
      return true;
    }

    /**
     * Requests that node has enough hierarchy information to create children in createChildren().
     * This function must not block, only start any requests in background. When the hierarchy
     * information is ready, the signal childrenPrepared() must be emitted.
     *
     * The default implementation does nothing. This only needs to be implemented when the factory
     * would otherwise need to do blocking network requests in createChildren() to avoid GUI freeze.
     * \see canCreateChildren()
     * \see createChildren()
     */
    virtual void prepareChildren( QgsChunkNode *node ) { Q_UNUSED( node ); }

  signals:
    //! Signal that gets emitted when a background job triggered from prepareChildren() has finished
    void childrenPrepared( QgsChunkNode *node );
};


/**
 * \ingroup 3d
 * \brief Base class for factories where the hierarchy is a quadtree where all leaves
 * are in the same depth.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsQuadtreeChunkLoaderFactory : public QgsChunkLoaderFactory
{
    Q_OBJECT
  public:
    QgsQuadtreeChunkLoaderFactory();
    virtual ~QgsQuadtreeChunkLoaderFactory();

    //! Initializes the root node setup (bounding box and error) and tree depth
    void setupQuadtree( const QgsBox3D &rootBox3D, float rootError, int maxLevel, const QgsBox3D &clippingBox3D = QgsBox3D() );

    virtual QgsChunkNode *createRootNode() const override;
    virtual QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;

  protected:
    QgsBox3D mRootBox3D;
    QgsBox3D mClippingBox3D;
    float mRootError = 0;
    //! maximum allowed depth of quad tree
    int mMaxLevel = 0;
};

/// @endcond

#endif // QGSCHUNKLOADER_H
