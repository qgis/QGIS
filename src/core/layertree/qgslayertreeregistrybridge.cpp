#include "qgslayertreeregistrybridge.h"

#include "qgsmaplayerregistry.h"

#include "qgslayertreenode.h"

#include "qgsproject.h"

QgsLayerTreeRegistryBridge::QgsLayerTreeRegistryBridge(QgsLayerTreeGroup *root, QObject *parent)
  : QObject(parent)
  , mRoot(root)
  , mEnabled(true)
{
  connect(QgsMapLayerRegistry::instance(), SIGNAL(layersAdded(QList<QgsMapLayer*>)), this, SLOT(layersAdded(QList<QgsMapLayer*>)));
  connect(QgsMapLayerRegistry::instance(), SIGNAL(layersWillBeRemoved(QStringList)), this, SLOT(layersWillBeRemoved(QStringList)));

  connectToGroup(mRoot);
}

void QgsLayerTreeRegistryBridge::layersAdded(QList<QgsMapLayer*> layers)
{
  if (!mEnabled)
    return;

  foreach (QgsMapLayer* layer, layers)
  {
    QgsLayerTreeLayer* nodeLayer = mRoot->addLayer(layer);

    // check whether the layer is marked as embedded
    QString projectFile = QgsProject::instance()->layerIsEmbedded(nodeLayer->layerId());
    if (!projectFile.isEmpty())
    {
      nodeLayer->setCustomProperty("embedded", true);
      nodeLayer->setCustomProperty("embedded_project", projectFile);
    }
  }
}

void QgsLayerTreeRegistryBridge::layersWillBeRemoved(QStringList layerIds)
{
  if (!mEnabled)
    return;

  foreach (QString layerId, layerIds)
  {
    QgsLayerTreeLayer* nodeLayer = mRoot->findLayer(layerId);
    if (nodeLayer)
      qobject_cast<QgsLayerTreeGroup*>(nodeLayer->parent())->removeChildNode(nodeLayer);
  }
}


void QgsLayerTreeRegistryBridge::groupAddedChildren(int indexFrom, int indexTo)
{
  QgsLayerTreeGroup* group = qobject_cast<QgsLayerTreeGroup*>(sender());
  Q_ASSERT(group);

  for (int i = indexFrom; i <= indexTo; ++i)
  {
    QgsLayerTreeNode* child = group->children()[i];
    if (child->nodeType() == QgsLayerTreeNode::NodeGroup)
      connectToGroup(static_cast<QgsLayerTreeGroup*>(child));
  }
}

static void _collectLayerIdsInGroup(QgsLayerTreeGroup* group, int indexFrom, int indexTo, QStringList& lst)
{
  for (int i = indexFrom; i <= indexTo; ++i)
  {
    QgsLayerTreeNode* child = group->children()[i];
    if (child->nodeType() == QgsLayerTreeNode::NodeLayer)
    {
      lst << static_cast<QgsLayerTreeLayer*>(child)->layerId();
    }
    else if (child->nodeType() == QgsLayerTreeNode::NodeGroup)
    {
      _collectLayerIdsInGroup(static_cast<QgsLayerTreeGroup*>(child), 0, child->children().count()-1, lst);
    }
  }
}

void QgsLayerTreeRegistryBridge::groupWillRemoveChildren(int indexFrom, int indexTo)
{
  Q_ASSERT(mLayerIdsForRemoval.isEmpty());

  QgsLayerTreeGroup* group = qobject_cast<QgsLayerTreeGroup*>(sender());
  Q_ASSERT(group);

  _collectLayerIdsInGroup(group, indexFrom, indexTo, mLayerIdsForRemoval);
}

void QgsLayerTreeRegistryBridge::groupRemovedChildren()
{
  // remove only those that really do not exist in the tree
  // (ignores layers that were dragged'n'dropped: 1. drop new 2. remove old)
  QStringList toRemove;
  foreach (QString layerId, mLayerIdsForRemoval)
    if (!mRoot->findLayer(layerId))
      toRemove << layerId;
  mLayerIdsForRemoval.clear();

  QgsMapLayerRegistry::instance()->removeMapLayers(toRemove);
}

void QgsLayerTreeRegistryBridge::connectToGroup(QgsLayerTreeGroup* group)
{
  connect(group, SIGNAL(addedChildren(int,int)), this, SLOT(groupAddedChildren(int,int)));
  connect(group, SIGNAL(willRemoveChildren(int,int)), this, SLOT(groupWillRemoveChildren(int,int)));
  connect(group, SIGNAL(removedChildren(int,int)), this, SLOT(groupRemovedChildren()));

  foreach (QgsLayerTreeNode* child, group->children())
  {
    if (child->nodeType() == QgsLayerTreeNode::NodeGroup)
      connectToGroup(static_cast<QgsLayerTreeGroup*>(child));
  }
}

