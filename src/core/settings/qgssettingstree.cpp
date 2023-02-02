#include "qgssettingstree.h"


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
