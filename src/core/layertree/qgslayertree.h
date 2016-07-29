/***************************************************************************
  qgslayertree.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREE_H
#define QGSLAYERTREE_H

#include "qgslayertreenode.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"

/** \ingroup core
 * Namespace with helper functions for layer tree operations.
 *
 * Only generally useful routines should be here. Miscellaneous utility functions for work
 * with the layer tree are in QgsLayerTreeUtils class.
 *
 * @note added in 2.4
 */
namespace QgsLayerTree
{
  //! Check whether the node is a valid group node
  inline bool isGroup( QgsLayerTreeNode* node )
  {
    return node && node->nodeType() == QgsLayerTreeNode::NodeGroup;
  }

  //! Check whether the node is a valid layer node
  inline bool isLayer( QgsLayerTreeNode* node )
  {
    return node && node->nodeType() == QgsLayerTreeNode::NodeLayer;
  }

  //! Cast node to a group. No type checking is done - use isGroup() to find out whether this operation is legal.
  inline QgsLayerTreeGroup* toGroup( QgsLayerTreeNode* node )
  {
    return static_cast<QgsLayerTreeGroup*>( node );
  }

  //! Cast node to a layer. No type checking is done - use isLayer() to find out whether this operation is legal.
  inline QgsLayerTreeLayer* toLayer( QgsLayerTreeNode* node )
  {
    return static_cast<QgsLayerTreeLayer*>( node );
  }

}

#endif // QGSLAYERTREE_H
