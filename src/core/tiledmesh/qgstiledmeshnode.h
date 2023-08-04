/***************************************************************************
                         qgstiledmeshnode.h
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

#ifndef QGSTILEDMESHNODE_H
#define QGSTILEDMESHNODE_H

#include "qgis.h"

#define SIP_NO_FILE

class QgsTiledMeshTile;

/**
 * \ingroup core
 * \brief Allows representing QgsTiledMeshTiles in a hierarchical tree.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.34
 */
class QgsTiledMeshNode
{
  public:

    /**
     * Constructor for QgsTiledMeshNode, for the specified \a tile.
     *
     * Ownership of \a tile is transferred to the node.
     */
    QgsTiledMeshNode( QgsTiledMeshTile *tile SIP_TRANSFER );

    //! QgsTiledMeshNode cannot be copied
    QgsTiledMeshNode( const QgsTiledMeshNode &other ) = delete;
    //! QgsTiledMeshNode cannot be copied
    QgsTiledMeshNode &operator=( const QgsTiledMeshNode &other ) = delete;

    ~QgsTiledMeshNode();

    /**
     * Returns the tile associated with the node.
     */
    QgsTiledMeshTile *tile();

    /**
     * Adds a \a child to this node.
     *
     * Ownership of \a child is transferred to this node, and the parent for \a child will
     * automatically be set to this node.
     */
    void addChild( QgsTiledMeshNode *child SIP_TRANSFER );

    /**
     * Returns the parent of this node.
     */
    QgsTiledMeshNode *parentNode() const { return mParent; }

    /**
     * Returns this node's children.
     */
    QList< QgsTiledMeshNode * > children() const { return mChildren; }

  private:

#ifdef SIP_RUN
    QgsTiledMeshNode( const QgsTiledMeshNode &other );
#endif

    std::unique_ptr< QgsTiledMeshTile > mTile;

    QgsTiledMeshNode *mParent = nullptr;
    QList< QgsTiledMeshNode * > mChildren;

};

#endif // QGSTILEDMESHNODE_H
