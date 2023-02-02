#include "qgssettingstree.h"

QgsSettingsTreeNode *QgsSettingsTree::treeRoot()
{
  // this must be defined in cpp code so we are sure only one instance is around
  static QgsSettingsTreeNode *sTreeRoot = QgsSettingsTreeNode::createRootNode();
  return sTreeRoot;
}


QgsSettingsTreeNode *QgsSettingsTree::createPluginTreeNode( const QString &pluginName )
{
  QgsSettingsTreeNode *te = sTreePlugins->childNode( pluginName );
  if ( te )
    return te;
  else
    return sTreePlugins->createChildNode( pluginName );
}

void QgsSettingsTree::unregisterPluginTreeNode( const QString &pluginName )
{
  QgsSettingsTreeNode *pluginTreeNode = sTreePlugins->childNode( pluginName );
  delete pluginTreeNode;
}
