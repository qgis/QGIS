%ModuleHeaderCode
#include <qgslayertree.h>
%End

/**
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
  bool isGroup( QgsLayerTreeNode* node );

  //! Check whether the node is a valid layer node
  bool isLayer( QgsLayerTreeNode* node );

  //! Cast node to a group. No type checking is done - use isGroup() to find out whether this operation is legal.
  // PYTHON: automatic cast
  //inline QgsLayerTreeGroup* toGroup( QgsLayerTreeNode* node );

  //! Cast node to a layer. No type checking is done - use isLayer() to find out whether this operation is legal.
  // PYTHON: automatic cast
  //inline QgsLayerTreeLayer* toLayer( QgsLayerTreeNode* node );

};
