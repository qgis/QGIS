#ifndef QGSLAYERTREE_H
#define QGSLAYERTREE_H

#include "qgslayertreenode.h"

namespace QgsLayerTree
{
  inline bool isGroup(QgsLayerTreeNode* node)
  {
    return node && node->nodeType() == QgsLayerTreeNode::NodeGroup;
  }
  inline bool isLayer(QgsLayerTreeNode* node)
  {
    return node && node->nodeType() == QgsLayerTreeNode::NodeLayer;
  }

  inline QgsLayerTreeGroup* toGroup(QgsLayerTreeNode* node)
  {
    return static_cast<QgsLayerTreeGroup*>(node);
  }
  inline QgsLayerTreeLayer* toLayer(QgsLayerTreeNode* node)
  {
    return static_cast<QgsLayerTreeLayer*>(node);
  }

}

#endif // QGSLAYERTREE_H
