#include "qgslayertreeview.h"

#include "qgslayertreemodel.h"
#include "qgslayertreenode.h"
#include "qgslayertreeviewdefaultactions.h"

#include <QMenu>
#include <QContextMenuEvent>

QgsLayerTreeView::QgsLayerTreeView(QWidget *parent)
  : QTreeView(parent)
  , mCurrentLayer(0)
  , mDefaultActions(0)
  , mMenuProvider(0)
{
  setHeaderHidden(true);

  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);

  setSelectionMode(ExtendedSelection);

  connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(updateExpandedStateToNode(QModelIndex)));
  connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(updateExpandedStateToNode(QModelIndex)));
}

QgsLayerTreeView::~QgsLayerTreeView()
{
  delete mMenuProvider;
}

void QgsLayerTreeView::setModel(QAbstractItemModel* model)
{
  if (!qobject_cast<QgsLayerTreeModel*>(model))
    return;

  connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(modelRowsInserted(QModelIndex,int,int)));

  QTreeView::setModel(model);

  connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onCurrentChanged(QModelIndex)));

  updateExpandedStateFromNode(layerTreeModel()->rootGroup());
}

QgsLayerTreeModel *QgsLayerTreeView::layerTreeModel() const
{
  return qobject_cast<QgsLayerTreeModel*>(model());
}

QgsLayerTreeViewDefaultActions* QgsLayerTreeView::defaultActions()
{
  if (!mDefaultActions)
    mDefaultActions = new QgsLayerTreeViewDefaultActions(this);
  return mDefaultActions;
}

void QgsLayerTreeView::setMenuProvider(QgsLayerTreeViewMenuProvider* menuProvider)
{
  delete mMenuProvider;
  mMenuProvider = menuProvider;
}

QgsMapLayer* QgsLayerTreeView::currentLayer() const
{
  return mCurrentLayer;
}

void QgsLayerTreeView::setCurrentLayer(QgsMapLayer* layer)
{
  if (mCurrentLayer == layer)
    return;

  mCurrentLayer = layer;
  emit currentLayerChanged(mCurrentLayer);
}


void QgsLayerTreeView::contextMenuEvent(QContextMenuEvent *event)
{
  if (!mMenuProvider)
    return;

  QModelIndex idx = indexAt(event->pos());
  if (!idx.isValid())
    setCurrentIndex(QModelIndex());

  QMenu* menu = mMenuProvider->createContextMenu();
  if (menu)
    menu->exec(mapToGlobal(event->pos()));
  delete menu;
}


void QgsLayerTreeView::modelRowsInserted(QModelIndex index, int start, int end)
{
  QgsLayerTreeNode* parentNode = layerTreeModel()->index2node(index);
  if (!parentNode)
    return;

  if (parentNode->nodeType() == QgsLayerTreeNode::NodeLayer)
    return; // layers have only symbology nodes (no expanded/collapsed handling)

  for (int i = start; i <= end; ++i)
  {
    updateExpandedStateFromNode(parentNode->children()[i]);
  }
}

void QgsLayerTreeView::updateExpandedStateToNode(QModelIndex index)
{
  QgsLayerTreeNode* node = layerTreeModel()->index2node(index);
  if (!node)
    return;

  node->setExpanded(isExpanded(index));
}

void QgsLayerTreeView::onCurrentChanged(QModelIndex current)
{
  QgsLayerTreeNode* node = layerTreeModel()->index2node(current);
  if (!node)
    return; // TODO: maybe also support symbology nodes

  QgsMapLayer* layer = 0;
  if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
    layer = static_cast<QgsLayerTreeLayer*>(node)->layer();

  setCurrentLayer(layer);
}

void QgsLayerTreeView::updateExpandedStateFromNode(QgsLayerTreeNode* node)
{
  QModelIndex idx = layerTreeModel()->node2index(node);
  setExpanded(idx, node->isExpanded());

  foreach (QgsLayerTreeNode* child, node->children())
    updateExpandedStateFromNode(child);
}

QgsLayerTreeNode* QgsLayerTreeView::currentNode() const
{
  return layerTreeModel()->index2node(selectionModel()->currentIndex());
}

QgsLayerTreeGroup* QgsLayerTreeView::currentGroupNode() const
{
  // TODO: also handle if a layer / symbology is selected within a group?
  QgsLayerTreeNode* node = currentNode();
  if (node && node->nodeType() == QgsLayerTreeNode::NodeGroup)
    return static_cast<QgsLayerTreeGroup*>(node);
  return 0;
}

QList<QgsLayerTreeNode*> QgsLayerTreeView::selectedNodes(bool skipInternal) const
{
  return layerTreeModel()->indexes2nodes(selectionModel()->selectedIndexes(), skipInternal);
}

QList<QgsLayerTreeLayer*> QgsLayerTreeView::selectedLayerNodes() const
{
  QList<QgsLayerTreeLayer*> layerNodes;
  foreach (QgsLayerTreeNode* node, selectedNodes())
  {
    if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
      layerNodes << static_cast<QgsLayerTreeLayer*>(node);
  }
  return layerNodes;
}
