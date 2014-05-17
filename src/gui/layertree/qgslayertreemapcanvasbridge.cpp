#include "qgslayertreemapcanvasbridge.h"

#include "qgslayertree.h"

#include "qgsmapcanvas.h"

QgsLayerTreeMapCanvasBridge::QgsLayerTreeMapCanvasBridge(QgsLayerTreeGroup *root, QgsMapCanvas *canvas)
  : mRoot(root)
  , mCanvas(canvas)
  , mPendingCanvasUpdate(false)
  , mHasCustomLayerOrder(false)
{
  connect(root, SIGNAL(addedChildren(QgsLayerTreeNode*,int,int)), this, SLOT(nodeAddedChildren(QgsLayerTreeNode*,int,int)));
  connect(root, SIGNAL(customPropertyChanged(QgsLayerTreeNode*,QString)), this, SLOT(nodeCustomPropertyChanged(QgsLayerTreeNode*,QString)));
  connect(root, SIGNAL(removedChildren(QgsLayerTreeNode*,int,int)), this, SLOT(nodeRemovedChildren()));
  connect(root, SIGNAL(visibilityChanged(QgsLayerTreeNode*,Qt::CheckState)), this, SLOT(nodeVisibilityChanged()));

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
  if (QgsLayerTree::isLayer(node))
  {
    QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer(node);
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

void QgsLayerTreeMapCanvasBridge::setCanvasLayers()
{
  QList<QgsMapCanvasLayer> layers;

  if (mHasCustomLayerOrder)
  {
    foreach (QString layerId, mCustomLayerOrder)
    {
      QgsLayerTreeLayer* nodeLayer = mRoot->findLayer(layerId);
      if (nodeLayer)
        layers << QgsMapCanvasLayer(nodeLayer->layer(), nodeLayer->isVisible() == Qt::Checked, nodeLayer->customProperty("overview", 0).toInt());
    }
  }
  else
    setCanvasLayers(mRoot, layers);

  mCanvas->setLayerSet(layers);

  mPendingCanvasUpdate = false;
}

void QgsLayerTreeMapCanvasBridge::setCanvasLayers(QgsLayerTreeNode *node, QList<QgsMapCanvasLayer> &layers)
{
  if (QgsLayerTree::isLayer(node))
  {
    QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer(node);
    layers << QgsMapCanvasLayer(nodeLayer->layer(), nodeLayer->isVisible() == Qt::Checked, nodeLayer->customProperty("overview", 0).toInt());
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

void QgsLayerTreeMapCanvasBridge::nodeAddedChildren(QgsLayerTreeNode* node, int indexFrom, int indexTo)
{
  Q_ASSERT(node);

  // collect layer IDs that have been added in order to put them into custom layer order
  QStringList layerIds;
  QList<QgsLayerTreeNode*> children = node->children();
  for (int i = indexFrom; i <= indexTo; ++i)
  {
    QgsLayerTreeNode* child = children.at(i);
    if (QgsLayerTree::isLayer(child))
    {
      layerIds << QgsLayerTree::toLayer(child)->layerId();
    }
    else if (QgsLayerTree::isGroup(child))
    {
      foreach (QgsLayerTreeLayer* nodeL, QgsLayerTree::toGroup(child)->findLayers())
        layerIds << nodeL->layerId();
    }
  }

  foreach (QString layerId, layerIds)
  {
    if (!mCustomLayerOrder.contains(layerId))
      mCustomLayerOrder.append(layerId);
  }

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

void QgsLayerTreeMapCanvasBridge::nodeCustomPropertyChanged(QgsLayerTreeNode* node, QString key)
{
  Q_UNUSED(node);
  if (key == "overview")
    deferredSetCanvasLayers();
}

