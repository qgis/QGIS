#include "qgslayertreemodel.h"

#include "qgslayertreenode.h"

#include <QMimeData>
#include <QTextStream>

#include "qgsdataitem.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"


QgsLayerTreeModel::QgsLayerTreeModel(QgsLayerTreeGroup* rootNode, QObject *parent)
  : QAbstractItemModel(parent)
  , mRootNode(rootNode)
  , mFlags(ShowSymbology)
{
  Q_ASSERT(mRootNode);

  // connect to all existing nodes
  connectToNode(mRootNode);
}

QgsLayerTreeModel::~QgsLayerTreeModel()
{
  foreach (QList<QgsLayerTreeModelSymbologyNode*> nodeL, mSymbologyNodes)
    qDeleteAll(nodeL);
  mSymbologyNodes.clear();
}

void QgsLayerTreeModel::connectToNode(QgsLayerTreeNode* node)
{
  connect(node, SIGNAL(willAddChildren(int,int)), this, SLOT(nodeWillAddChildren(int,int)));
  connect(node, SIGNAL(addedChildren(int,int)), this, SLOT(nodeAddedChildren(int,int)));
  connect(node, SIGNAL(willRemoveChildren(int,int)), this, SLOT(nodeWillRemoveChildren(int,int)));
  connect(node, SIGNAL(removedChildren(int,int)), this, SLOT(nodeRemovedChildren()));
  connect(node, SIGNAL(visibilityChanged(Qt::CheckState)), this, SLOT(nodeVisibilityChanded()));

  foreach (QgsLayerTreeNode* child, node->children())
    connectToNode(child);
}

QgsLayerTreeNode* QgsLayerTreeModel::index2node(const QModelIndex& index) const
{
  if (!index.isValid())
    return mRootNode;

  QObject* obj = reinterpret_cast<QObject*>(index.internalPointer());
  return qobject_cast<QgsLayerTreeNode*>(obj);
}

QgsLayerTreeModelSymbologyNode* QgsLayerTreeModel::index2symnode(const QModelIndex& index)
{
  return qobject_cast<QgsLayerTreeModelSymbologyNode*>(reinterpret_cast<QObject*>(index.internalPointer()));
}


int QgsLayerTreeModel::rowCount(const QModelIndex &parent) const
{
  if (index2symnode(parent))
  {
    return 0; // they are leaves
  }

  QgsLayerTreeNode* n = index2node(parent);

  if (parent.isValid() && parent.column() != 0)
    return 0;

  if (n->nodeType() == QgsLayerTreeNode::NodeLayer)
  {
    QgsLayerTreeLayer* nL = static_cast<QgsLayerTreeLayer*>(n);
    if (testFlag(ShowSymbology) && !mSymbologyNodes.contains(nL))
      addSymbologyToLayer(nL);

    return mSymbologyNodes[nL].count();
  }

  return n->children().count();
}

int QgsLayerTreeModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QModelIndex QgsLayerTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (column != 0 || row < 0 || row >= rowCount(parent))
    return QModelIndex();

  QgsLayerTreeNode* n = index2node(parent);

  if (!n)
  {
    QgsLayerTreeModelSymbologyNode* sym = index2symnode(parent);
    Q_ASSERT(sym);
    return QModelIndex(); // have no children
  }

  if (!n || column != 0 || row >= rowCount(parent))
    return QModelIndex();

  if (testFlag(ShowSymbology) && n->nodeType() == QgsLayerTreeNode::NodeLayer)
  {
    QgsLayerTreeLayer* nL = static_cast<QgsLayerTreeLayer*>(n);
    Q_ASSERT(mSymbologyNodes.contains(nL));
    return createIndex(row, column, static_cast<QObject*>(mSymbologyNodes[nL].at(row)));
  }

  return createIndex(row, column, static_cast<QObject*>(n->children().at(row)));
}

QModelIndex QgsLayerTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  QgsLayerTreeNode* n = index2node(child);

  QgsLayerTreeNode* parentNode = 0;

  if (!n)
  {
    QgsLayerTreeModelSymbologyNode* sym = index2symnode(child);
    Q_ASSERT(sym);
    parentNode = sym->parent();
  }
  else
    parentNode = n->parent(); // must not be null
  Q_ASSERT(parentNode);

  QgsLayerTreeNode* grandParentNode = parentNode->parent();
  if (!grandParentNode)
    return QModelIndex();  // root node -> invalid index

  int row = grandParentNode->children().indexOf(parentNode);
  Q_ASSERT(row >= 0);
  return createIndex(row, 0, static_cast<QObject*>(parentNode));
}

QVariant QgsLayerTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (QgsLayerTreeModelSymbologyNode* sym = index2symnode(index))
  {
    if (role == Qt::DisplayRole)
      return sym->name();
    else if (role == Qt::DecorationRole)
      return sym->icon();
    return QVariant();
  }

  QgsLayerTreeNode* node = index2node(index);
  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    if (node->nodeType() == QgsLayerTreeNode::NodeGroup)
      return static_cast<QgsLayerTreeGroup*>(node)->name();
    else if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
      return static_cast<QgsLayerTreeLayer*>(node)->layerName();
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 )
  {
    if (node->nodeType() == QgsLayerTreeNode::NodeGroup)
      return QgsDataCollectionItem::iconDir();
    else if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
    {
      QgsMapLayer* layer = static_cast<QgsLayerTreeLayer*>(node)->layer();
      if (!layer)
        return QVariant();
      if (layer->type() == QgsMapLayer::RasterLayer)
        return QgsLayerItem::iconRaster();
      else if (layer->type() == QgsMapLayer::VectorLayer)
      {
        QgsVectorLayer* vlayer = static_cast<QgsVectorLayer*>(layer);
        if (vlayer->geometryType() == QGis::Point)
          return QgsLayerItem::iconPoint();
        else if (vlayer->geometryType() == QGis::Line)
          return QgsLayerItem::iconLine();
        else if (vlayer->geometryType() == QGis::Polygon)
          return QgsLayerItem::iconPolygon();
        else if (vlayer->geometryType() == QGis::NoGeometry)
          return QgsLayerItem::iconTable();
      }
      return QgsLayerItem::iconDefault();
    }
  }
  else if ( role == Qt::CheckStateRole )
  {
    if (!testFlag(AllowVisibilityManagement))
      return QVariant();

    if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
    {
      QgsLayerTreeLayer* nodeLayer = static_cast<QgsLayerTreeLayer*>(node);
      return nodeLayer->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
    else if (node->nodeType() == QgsLayerTreeNode::NodeGroup)
    {
      QgsLayerTreeGroup* nodeGroup = static_cast<QgsLayerTreeGroup*>(node);
      return nodeGroup->isVisible();
    }
  }
  else if ( role == Qt::FontRole )
  {
    QFont f;
    if (node->customProperty("embedded", false).toBool())
      f.setItalic(true);
    if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
      f.setBold(true);
    return f;
  }

  return QVariant();
}

Qt::ItemFlags QgsLayerTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;

  if (index2symnode(index))
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
  QgsLayerTreeNode* node = index2node(index);
  if (testFlag(AllowVisibilityManagement) && node->nodeType() == QgsLayerTreeNode::NodeLayer)
    f |= Qt::ItemIsUserCheckable;
  else if (node->nodeType() == QgsLayerTreeNode::NodeGroup)
    f |= Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable;
  return f;
}

bool QgsLayerTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  QgsLayerTreeNode* node = index2node(index);
  if (!node)
    return QgsLayerTreeModel::setData(index, value, role);

  if (role == Qt::CheckStateRole)
  {
    if (!testFlag(AllowVisibilityManagement))
      return false;

    if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
    {
      QgsLayerTreeLayer* layer = static_cast<QgsLayerTreeLayer*>(node);
      layer->setVisible(value.toInt() == Qt::Checked);
    }
    else if (node->nodeType() == QgsLayerTreeNode::NodeGroup)
    {
      QgsLayerTreeGroup* group = static_cast<QgsLayerTreeGroup*>(node);
      group->setVisible((Qt::CheckState)value.toInt());
    }
    return true;
  }
  else if (role == Qt::EditRole)
  {
    if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
    {
      QgsLayerTreeLayer* layer = static_cast<QgsLayerTreeLayer*>(node);
      layer->setLayerName(value.toString());
      emit dataChanged(index, index);
    }
    else if (node->nodeType() == QgsLayerTreeNode::NodeGroup)
    {
      static_cast<QgsLayerTreeGroup*>(node)->setName(value.toString());
      emit dataChanged(index, index);
    }
  }

  return QAbstractItemModel::setData(index, value, role);
}

QModelIndex QgsLayerTreeModel::node2index(QgsLayerTreeNode* node)
{
  if (!node->parent())
    return QModelIndex(); // this is the only root item -> invalid index
  QModelIndex parentIndex = node2index(node->parent());

  int row = node->parent()->children().indexOf(node);
  Q_ASSERT(row >= 0);
  return index(row, 0, parentIndex);
}


static bool _isChildOfNode(QgsLayerTreeNode* child, QgsLayerTreeNode* node)
{
  if (!child->parent())
    return false;

  if (child->parent() == node)
    return true;

  return _isChildOfNode(child->parent(), node);
}

static bool _isChildOfNodes(QgsLayerTreeNode* child, QList<QgsLayerTreeNode*> nodes)
{
  foreach (QgsLayerTreeNode* n, nodes)
  {
    if (_isChildOfNode(child, n))
      return true;
  }
  return false;
}


QList<QgsLayerTreeNode*> QgsLayerTreeModel::indexes2nodes(const QModelIndexList& list, bool skipInternal) const
{
  QList<QgsLayerTreeNode*> nodes;
  foreach (QModelIndex index, list)
  {
    QgsLayerTreeNode* node = index2node(index);
    if (!node)
      continue;

    nodes << node;
  }

  if (!skipInternal)
    return nodes;

  // remove any children of nodes if both parent node and children are selected
  QList<QgsLayerTreeNode*> nodesFinal;
  foreach (QgsLayerTreeNode* node, nodes)
  {
    if (!_isChildOfNodes(node, nodes))
      nodesFinal << node;
  }

  return nodesFinal;
}

void QgsLayerTreeModel::nodeWillAddChildren(int indexFrom, int indexTo)
{
  QgsLayerTreeNode* node = qobject_cast<QgsLayerTreeNode*>(sender());
  Q_ASSERT(node);
  beginInsertRows(node2index(node), indexFrom, indexTo);
}

void QgsLayerTreeModel::nodeAddedChildren(int indexFrom, int indexTo)
{
  QgsLayerTreeNode* node = qobject_cast<QgsLayerTreeNode*>(sender());
  Q_ASSERT(node);

  endInsertRows();

  for (int i = indexFrom; i <= indexTo; ++i)
    connectToNode( node->children()[i] );
}

void QgsLayerTreeModel::nodeWillRemoveChildren(int indexFrom, int indexTo)
{
  QgsLayerTreeNode* node = qobject_cast<QgsLayerTreeNode*>(sender());
  Q_ASSERT(node);

  beginRemoveRows(node2index(node), indexFrom, indexTo);

  // cleanup - e.g. symbology
  for (int i = indexFrom; i <= indexTo; ++i)
    removeSymbologyFromSubtree(node->children()[i]);
}

void QgsLayerTreeModel::nodeRemovedChildren()
{
  endRemoveRows();
}

void QgsLayerTreeModel::nodeVisibilityChanded()
{
  QgsLayerTreeNode* node = qobject_cast<QgsLayerTreeNode*>(sender());
  Q_ASSERT(node);

  QModelIndex index = node2index(node);
  emit dataChanged(index, index);
}

void QgsLayerTreeModel::removeSymbologyFromSubtree(QgsLayerTreeNode* node)
{
  if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
  {
    QgsLayerTreeLayer* nodeLayer = static_cast<QgsLayerTreeLayer*>(node);
    if (mSymbologyNodes.contains(nodeLayer))
    {
      qDeleteAll(mSymbologyNodes[nodeLayer]);
      mSymbologyNodes.remove(nodeLayer);
    }
  }

  foreach (QgsLayerTreeNode* child, node->children())
    removeSymbologyFromSubtree(child);
}


void QgsLayerTreeModel::addSymbologyToLayer(QgsLayerTreeLayer* nodeL) const
{
  if (!nodeL->layer())
    return; // skip creation of symbology if the layer is not (yet?) loaded

  QList<QgsLayerTreeModelSymbologyNode*>& lst = mSymbologyNodes[nodeL];

  if (nodeL->layer()->type() == QgsMapLayer::VectorLayer)
  {
    QgsFeatureRendererV2* r = static_cast<QgsVectorLayer*>(nodeL->layer())->rendererV2();
    if (r)
    {
      // TODO: use legend symbol list
      //QgsLegendSymbolList syms = r->legendSymbolItems( /* double scaleDenominator = -1, QString rule = "" */ );
      QSize iconSize( 16, 16 );
      typedef QPair<QString, QPixmap> XY;
      foreach ( XY item, r->legendSymbologyItems( iconSize ))
        lst << new QgsLayerTreeModelSymbologyNode(nodeL, item.first, QIcon(item.second));
    }
  }
  else
  {
    // TODO: raster / plugin layer
  }
}


Qt::DropActions QgsLayerTreeModel::supportedDropActions() const
{
  return /*Qt::CopyAction |*/ Qt::MoveAction;
}

QStringList QgsLayerTreeModel::mimeTypes() const
{
  QStringList types;
  types << "application/qgis.layertreemodeldata";
  return types;
}


QMimeData* QgsLayerTreeModel::mimeData(const QModelIndexList& indexes) const
{
  QList<QgsLayerTreeNode*> nodesFinal = indexes2nodes(indexes, true);

  if (nodesFinal.count() == 0)
    return 0;

  QMimeData *mimeData = new QMimeData();

  QDomDocument doc;
  QDomElement rootElem = doc.createElement("layer_tree_model_data");
  foreach (QgsLayerTreeNode* node, nodesFinal)
    node->writeXML(rootElem);
  doc.appendChild(rootElem);
  QString txt = doc.toString();

  mimeData->setData("application/qgis.layertreemodeldata", txt.toUtf8());
  qDebug("%s", txt.toUtf8().data());
  return mimeData;
}

bool QgsLayerTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
  if (action == Qt::IgnoreAction)
    return true;

  if (!data->hasFormat("application/qgis.layertreemodeldata"))
    return false;

  if (column > 0)
    return false;

  QgsLayerTreeNode* nodeParent = index2node(parent);
  if (!nodeParent || nodeParent->nodeType() != QgsLayerTreeNode::NodeGroup)
    return false;

  QByteArray encodedData = data->data("application/qgis.layertreemodeldata");

  QDomDocument doc;
  if (!doc.setContent(QString::fromUtf8(encodedData)))
    return false;

  QDomElement rootElem = doc.documentElement();
  if (rootElem.tagName() != "layer_tree_model_data")
    return false;

  QList<QgsLayerTreeNode*> nodes;

  QDomElement elem = rootElem.firstChildElement();
  while (!elem.isNull())
  {
    QgsLayerTreeNode* node = QgsLayerTreeNode::readXML(elem);
    if (node)
      nodes << node;

    elem = elem.nextSiblingElement();
  }

  if (nodes.count() == 0)
    return false;

  static_cast<QgsLayerTreeGroup*>(nodeParent)->insertChildNodes(row, nodes);

  return true;
}

bool QgsLayerTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
  QgsLayerTreeNode* parentNode = index2node(parent);
  if (parentNode && parentNode->nodeType() == QgsLayerTreeNode::NodeGroup)
  {
    static_cast<QgsLayerTreeGroup*>(parentNode)->removeChildren(row, count);
    return true;
  }
  return false;
}
