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
#include "qgsbox3d.h"
#include "qgschunknode.h"

#include <QEntity>
#include <QFuture>

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Result of running \ref QgsChunkLoader::loadChunk
 *
 * \note Not available in Python bindings
 *
 */
struct QgsChunkLoaderResult
{
  public:
    /**
     * Run in main thread to use loaded data.
     * Returns entity attached to the given parent entity in disabled state
     */
    std::function<Qt3DCore::QEntity *( Qt3DCore::QEntity *parent )> createEntity;

    static QgsChunkLoaderResult sEmpty;
};


/**
 * \ingroup qgis_3d
 * \brief Loader of chunks for a particular layer.
 *
 * \note Not available in Python bindings
 */
class QgsChunkLoader : public QObject
{
    Q_OBJECT
  public:
    ~QgsChunkLoader() override = default;

    //! Loads the chunk for given node.
    virtual QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) = 0;

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
    virtual bool canCreateChildren( QgsChunkNode *node );

    /**
     * Requests that node has enough hierarchy information to create children in createChildren().
     * This function must not block, only start any requests in background. When the hierarchy
     * information is ready, the signal childrenPrepared() may be emitted so that the entity gets updated.
     *
     * The default implementation does nothing. This only needs to be implemented when the factory
     * would otherwise need to do blocking network requests in createChildren() to avoid GUI freeze.
     * \see canCreateChildren()
     * \see createChildren()
     */
    virtual QFuture<void> prepareChildren( QgsChunkNode *node );
};


/**
 * \ingroup qgis_3d
 * \brief Base class for chunk loaders where the hierarchy is a quadtree where
 * all leaves are in the same depth.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsQuadtreeChunkLoader : public QgsChunkLoader
{
    Q_OBJECT
  public:
    QgsQuadtreeChunkLoader();
    ~QgsQuadtreeChunkLoader() override;

    //! Initializes the root node setup (bounding box and error) and tree depth
    void setupQuadtree( const QgsBox3D &rootBox3D, float rootError, int maxLevel = -1, const QgsBox3D &clippingBox3D = QgsBox3D() );

    QgsChunkNode *createRootNode() const override;
    QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;

  protected:
    QgsBox3D mRootBox3D;
    QgsBox3D mClippingBox3D;
    float mRootError = 0;
    //! maximum allowed depth of quad tree. -1 for no max depth.
    int mMaxLevel = -1;
};

/// @endcond

#endif // QGSCHUNKLOADER_H
