/***************************************************************************
                         qgstiledscenenode.h
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENENODE_H
#define QGSTILEDSCENENODE_H

#include "qgis.h"

#define SIP_NO_FILE

class QgsTiledSceneTile;

/**
 * \ingroup core
 * \brief Allows representing QgsTiledSceneTiles in a hierarchical tree.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.34
 */
class QgsTiledSceneNode
{
  public:

    /**
     * Constructor for QgsTiledSceneNode, for the specified \a tile.
     *
     * Ownership of \a tile is transferred to the node.
     */
    QgsTiledSceneNode( QgsTiledSceneTile *tile SIP_TRANSFER );

    QgsTiledSceneNode( const QgsTiledSceneNode &other ) = delete;
    QgsTiledSceneNode &operator=( const QgsTiledSceneNode &other ) = delete;

    ~QgsTiledSceneNode();

    /**
     * Returns the tile associated with the node.
     */
    QgsTiledSceneTile *tile();

    /**
     * Adds a \a child to this node.
     *
     * Ownership of \a child is transferred to this node, and the parent for \a child will
     * automatically be set to this node.
     */
    void addChild( QgsTiledSceneNode *child SIP_TRANSFER );

    /**
     * Returns the parent of this node.
     */
    QgsTiledSceneNode *parentNode() const { return mParent; }

    /**
     * Returns this node's children.
     */
    QList< QgsTiledSceneNode * > children() const { return mChildren; }

  private:

#ifdef SIP_RUN
    QgsTiledSceneNode( const QgsTiledSceneNode &other );
#endif

    std::unique_ptr< QgsTiledSceneTile > mTile;

    QgsTiledSceneNode *mParent = nullptr;
    QList< QgsTiledSceneNode * > mChildren;

};

#endif // QGSTILEDSCENENODE_H
