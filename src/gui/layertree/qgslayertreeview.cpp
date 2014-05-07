#include "qgslayertreeview.h"

#include "qgslayertreemodel.h"
#include "qgslayertreenode.h"

#include <QMenu>
#include <QContextMenuEvent>

QgsLayerTreeView::QgsLayerTreeView(QWidget *parent)
  : QTreeView(parent)
  , mCurrentLayer(0)
{
  setHeaderHidden(true);

  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);

  setSelectionMode(ExtendedSelection);

  connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(updateExpandedStateToNode(QModelIndex)));
  connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(updateExpandedStateToNode(QModelIndex)));
  connect(this, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onCurrentChanged(QModelIndex)));
}

void QgsLayerTreeView::setModel(QAbstractItemModel* model)
{
  if (!qobject_cast<QgsLayerTreeModel*>(model))
    return;

  connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(modelRowsInserted(QModelIndex,int,int)));

  QTreeView::setModel(model);

  updateExpandedStateFromNode(layerTreeModel()->rootGroup());
}

QgsLayerTreeModel *QgsLayerTreeView::layerTreeModel()
{
  return qobject_cast<QgsLayerTreeModel*>(model());
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
  QMenu menu;

  QModelIndex idx = indexAt(event->pos());
  if (!idx.isValid())
  {
    menu.addAction(create_addGroup(&menu));
  }
  else
  {
    QgsLayerTreeNode* node = layerTreeModel()->index2node(idx);
    if (!node)
      return; // probably a symbology item

    if (node->nodeType() == QgsLayerTreeNode::NodeGroup)
      menu.addAction(create_addGroup(&menu, node));
    else if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
      menu.addAction(create_showInOverview(&menu, node)); // TODO: should be custom action

    menu.addAction(create_removeGroupOrLayer(&menu, node));
  }

  menu.exec(mapToGlobal(event->pos()));
}

QAction *QgsLayerTreeView::create_addGroup(QObject* parent, QgsLayerTreeNode* parentNode)
{
  QAction* a = new QAction(tr("Add Group"), parent);
  connect(a, SIGNAL(triggered()), this, SLOT(addGroup()));
  a->setData(QVariant::fromValue((QObject*)parentNode));
  return a;
}

QAction *QgsLayerTreeView::create_removeGroupOrLayer(QObject *parent, QgsLayerTreeNode *parentNode)
{
  QAction* a = new QAction(tr("Remove"), parent);
  connect(a, SIGNAL(triggered()), this, SLOT(removeGroupOrLayer()));
  a->setData(QVariant::fromValue((QObject*)parentNode));
  return a;
}

QAction* QgsLayerTreeView::create_showInOverview(QObject* parent, QgsLayerTreeNode* parentNode)
{
  QAction* a = new QAction(tr("Show in overview"), parent);
  connect(a, SIGNAL(triggered()), this, SLOT(showInOverview()));
  a->setData(QVariant::fromValue((QObject*)parentNode));
  a->setCheckable(true);
  a->setChecked(parentNode->customProperty("overview", 0).toInt());
  return a;
}

void QgsLayerTreeView::addGroup()
{
  QVariant v = qobject_cast<QAction*>(sender())->data();
  QgsLayerTreeGroup* group = qobject_cast<QgsLayerTreeGroup*>(v.value<QObject*>());
  if (!group)
    group = layerTreeModel()->rootGroup();

  group->addGroup("group");
}

void QgsLayerTreeView::removeGroupOrLayer()
{
  QList<QgsLayerTreeNode*> nodes = layerTreeModel()->indexes2nodes(selectionModel()->selectedIndexes(), true);
  foreach (QgsLayerTreeNode* node, nodes)
  {
    // could be more efficient if working directly with ranges instead of individual nodes
    qobject_cast<QgsLayerTreeGroup*>(node->parent())->removeChildNode(node);
  }
}

void QgsLayerTreeView::showInOverview()
{
  QVariant v = qobject_cast<QAction*>(sender())->data();
  QgsLayerTreeNode* node = qobject_cast<QgsLayerTreeNode*>(v.value<QObject*>());
  Q_ASSERT(node);

  node->setCustomProperty("overview", node->customProperty("overview", 0).toInt() ? 0 : 1);
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
