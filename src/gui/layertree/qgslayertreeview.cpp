#include "qgslayertreeview.h"

#include "qgslayertreemodel.h"
#include "qgslayertreenode.h"
#include "qgslayertreeviewdefaultactions.h"

#include <QMenu>
#include <QContextMenuEvent>

QgsLayerTreeView::QgsLayerTreeView(QWidget *parent)
  : QTreeView(parent)
  , mDefaultActions(0)
  , mMenuProvider(0)
{
  setHeaderHidden(true);

  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);
  setEditTriggers(EditKeyPressed | SelectedClicked);

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

  connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onCurrentChanged(QModelIndex,QModelIndex)));

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
  return layerForIndex( currentIndex() );
}

void QgsLayerTreeView::setCurrentLayer(QgsMapLayer* layer)
{
  QgsLayerTreeLayer* nodeLayer = layerTreeModel()->rootGroup()->findLayer(layer->id());
  if (!nodeLayer)
    return;

  setCurrentIndex( layerTreeModel()->node2index(nodeLayer) );
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

void QgsLayerTreeView::onCurrentChanged(QModelIndex current, QModelIndex previous)
{
  QgsMapLayer* layerPrevious = layerForIndex(previous);
  QgsMapLayer* layerCurrent = layerForIndex(current);

  if (layerPrevious == layerCurrent)
    return;

  qDebug("current layer changed!");
  emit currentLayerChanged(layerCurrent);
}

void QgsLayerTreeView::updateExpandedStateFromNode(QgsLayerTreeNode* node)
{
  QModelIndex idx = layerTreeModel()->node2index(node);
  setExpanded(idx, node->isExpanded());

  foreach (QgsLayerTreeNode* child, node->children())
    updateExpandedStateFromNode(child);
}

QgsMapLayer* QgsLayerTreeView::layerForIndex(const QModelIndex& index) const
{
  QgsLayerTreeNode* node = layerTreeModel()->index2node(index);
  if (node)
  {
    if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
      return static_cast<QgsLayerTreeLayer*>(node)->layer();
  }
  else
  {
    // possibly a symbology node
    QgsLayerTreeModelSymbologyNode* symnode = layerTreeModel()->index2symnode(index);
    if (symnode)
      return symnode->parent()->layer();
  }

  return 0;
}

QgsLayerTreeNode* QgsLayerTreeView::currentNode() const
{
  return layerTreeModel()->index2node(selectionModel()->currentIndex());
}

QgsLayerTreeGroup* QgsLayerTreeView::currentGroupNode() const
{
  QgsLayerTreeNode* node = currentNode();
  if (node && node->nodeType() == QgsLayerTreeNode::NodeGroup)
    return static_cast<QgsLayerTreeGroup*>(node);
  else if (node && node->nodeType() == QgsLayerTreeNode::NodeLayer)
  {
    QgsLayerTreeNode* parent = node->parent();
    if (parent && parent->nodeType() == QgsLayerTreeNode::NodeGroup)
      return static_cast<QgsLayerTreeGroup*>(node);
  }
  // TODO: also handle if symbology is selected?

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

QList<QgsMapLayer*> QgsLayerTreeView::selectedLayers() const
{
  QList<QgsMapLayer*> list;
  foreach (QgsLayerTreeLayer* node, selectedLayerNodes())
  {
    if (node->layer())
      list << node->layer();
  }
  return list;
}
