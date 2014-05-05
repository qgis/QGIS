#include "qgslayertreemapcanvasbridge.h"

#include "qgslayertreenode.h"

#include "qgsmapcanvas.h"

QgsLayerTreeMapCanvasBridge::QgsLayerTreeMapCanvasBridge(QgsLayerTreeGroup *root, QgsMapCanvas *canvas)
  : mRoot(root)
  , mCanvas(canvas)
  , mPendingCanvasUpdate(false)
  , mHasCustomLayerOrder(false)
{
  connectToNode(root);

  setCanvasLayers();
}

QStringList QgsLayerTreeMapCanvasBridge::defaultLayerOrder() const
{
  QStringList order;
  defaultLayerOrder(mRoot, order);
  return order;
}

void QgsLayerTreeMapCanvasBridge::defaultLayerOrder(QgsLayerTreeNode* node, QStringList& order) const
{
  if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
  {
    QgsLayerTreeLayer* nodeLayer = static_cast<QgsLayerTreeLayer*>(node);
    order << nodeLayer->layerId();
  }

  foreach (QgsLayerTreeNode* child, node->children())
    defaultLayerOrder(child, order);
}


void QgsLayerTreeMapCanvasBridge::setHasCustomLayerOrder(bool override)
{
  if (mHasCustomLayerOrder == override)
    return;

  mHasCustomLayerOrder = override;
  emit hasCustomLayerOrderChanged(mHasCustomLayerOrder);

  deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::setCustomLayerOrder(const QStringList& order)
{
  if (mCustomLayerOrder == order)
    return;

  // verify that the new order is correct
  QStringList defOrder = defaultLayerOrder();
  QStringList sortedNewOrder = order;
  qSort(defOrder);
  qSort(sortedNewOrder);
  if (defOrder != sortedNewOrder)
    return; // must be permutation of the default order

  mCustomLayerOrder = order;
  emit customLayerOrderChanged(mCustomLayerOrder);

  if (mHasCustomLayerOrder)
    deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::connectToNode(QgsLayerTreeNode *node)
{
  connect(node, SIGNAL(addedChildren(int,int)), this, SLOT(nodeAddedChildren(int,int)));
  connect(node, SIGNAL(removedChildren(int,int)), this, SLOT(nodeRemovedChildren()));
  connect(node, SIGNAL(visibilityChanged(Qt::CheckState)), this, SLOT(nodeVisibilityChanged()));

  if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
  {
    QString layerId = static_cast<QgsLayerTreeLayer*>(node)->layerId();
    if (!mCustomLayerOrder.contains(layerId))
      mCustomLayerOrder.append(layerId);
  }

  foreach (QgsLayerTreeNode* child, node->children())
    connectToNode(child);
}

void QgsLayerTreeMapCanvasBridge::setCanvasLayers()
{
  QList<QgsMapCanvasLayer> layers;

  if (mHasCustomLayerOrder)
  {
    foreach (QString layerId, mCustomLayerOrder)
    {
      QgsLayerTreeLayer* nodeLayer = mRoot->findLayer(layerId);
      if (nodeLayer)
        layers << QgsMapCanvasLayer(nodeLayer->layer(), nodeLayer->isVisible(), nodeLayer->customProperty("overview", 0).toInt());
    }
  }
  else
    setCanvasLayers(mRoot, layers);

  mCanvas->setLayerSet(layers);

  mPendingCanvasUpdate = false;
}

void QgsLayerTreeMapCanvasBridge::setCanvasLayers(QgsLayerTreeNode *node, QList<QgsMapCanvasLayer> &layers)
{
  if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
  {
    QgsLayerTreeLayer* nodeLayer = static_cast<QgsLayerTreeLayer*>(node);
    layers << QgsMapCanvasLayer(nodeLayer->layer(), nodeLayer->isVisible(), nodeLayer->customProperty("overview", 0).toInt());
  }

  foreach (QgsLayerTreeNode* child, node->children())
    setCanvasLayers(child, layers);
}

void QgsLayerTreeMapCanvasBridge::deferredSetCanvasLayers()
{
  if (mPendingCanvasUpdate)
    return;

  mPendingCanvasUpdate = true;
  QMetaObject::invokeMethod(this, "setCanvasLayers", Qt::QueuedConnection);
}

void QgsLayerTreeMapCanvasBridge::nodeAddedChildren(int indexFrom, int indexTo)
{
  // connect to new children
  Q_ASSERT(sender() && qobject_cast<QgsLayerTreeNode*>(sender()));
  QgsLayerTreeNode* node = qobject_cast<QgsLayerTreeNode*>(sender());
  for (int i = indexFrom; i <= indexTo; ++i)
    connectToNode(node->children()[i]);

  emit customLayerOrderChanged(mCustomLayerOrder);

  deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::nodeRemovedChildren()
{
  // no need to disconnect from removed nodes as they are deleted

  // check whether the layers are still there, if not, remove them from the layer order!
  QList<int> toRemove;
  for (int i = 0; i < mCustomLayerOrder.count(); ++i)
  {
    QgsLayerTreeLayer* node = mRoot->findLayer(mCustomLayerOrder[i]);
    if (!node)
      toRemove << i;
  }
  for (int i = toRemove.count()-1; i >= 0; --i)
    mCustomLayerOrder.removeAt(toRemove[i]);
  emit customLayerOrderChanged(mCustomLayerOrder);

  deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::nodeVisibilityChanged()
{
  deferredSetCanvasLayers();
}

