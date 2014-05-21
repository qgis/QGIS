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

namespace QgsLayerTree
{
  inline bool isGroup( QgsLayerTreeNode* node )
  {
    return node && node->nodeType() == QgsLayerTreeNode::NodeGroup;
  }
  inline bool isLayer( QgsLayerTreeNode* node )
  {
    return node && node->nodeType() == QgsLayerTreeNode::NodeLayer;
  }

  inline QgsLayerTreeGroup* toGroup( QgsLayerTreeNode* node )
  {
    return static_cast<QgsLayerTreeGroup*>( node );
  }
  inline QgsLayerTreeLayer* toLayer( QgsLayerTreeNode* node )
  {
    return static_cast<QgsLayerTreeLayer*>( node );
  }

}

#endif // QGSLAYERTREE_H
