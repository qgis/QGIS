#include "qgslayertreenode.h"

#include "qgslayertree.h"
#include "qgslayertreeutils.h"


QgsLayerTreeNode::QgsLayerTreeNode(QgsLayerTreeNode::NodeType t)
  : mNodeType(t)
  , mParent(0)
  , mExpanded(true)
{
}

QgsLayerTreeNode::QgsLayerTreeNode(const QgsLayerTreeNode& other)
  : QObject()
  , mNodeType(other.mNodeType)
  , mParent(0)
  , mExpanded(other.mExpanded)
  , mProperties(other.mProperties)
{
  QList<QgsLayerTreeNode*> clonedChildren;
  foreach (QgsLayerTreeNode* child, other.mChildren)
    clonedChildren << child->clone();
  insertChildren(-1, clonedChildren);
}

QgsLayerTreeNode* QgsLayerTreeNode::readXML(QDomElement& element)
{
  QgsLayerTreeNode* node = 0;
  if (element.tagName() == "layer-tree-group")
    node = QgsLayerTreeGroup::readXML(element);
  else if (element.tagName() == "layer-tree-layer")
    node = QgsLayerTreeLayer::readXML(element);

  return node;
}

void QgsLayerTreeNode::setCustomProperty(const QString &key, const QVariant &value)
{
  mProperties.setValue(key, value);
  emit customPropertyChanged(this, key);
}

QVariant QgsLayerTreeNode::customProperty(const QString &key, const QVariant &defaultValue) const
{
  return mProperties.value(key, defaultValue);
}

void QgsLayerTreeNode::removeCustomProperty(const QString &key)
{
  mProperties.remove(key);
  emit customPropertyChanged(this, key);
}

QStringList QgsLayerTreeNode::customProperties() const
{
  return mProperties.keys();
}

void QgsLayerTreeNode::readCommonXML(QDomElement& element)
{
  mProperties.readXml(element);
}

void QgsLayerTreeNode::writeCommonXML(QDomElement& element)
{
  QDomDocument doc(element.ownerDocument());
  mProperties.writeXml(element, doc);
}

void QgsLayerTreeNode::addChild(QgsLayerTreeNode *node)
{
  insertChild(-1, node);
}

void QgsLayerTreeNode::insertChild(int index, QgsLayerTreeNode *node)
{
  QList<QgsLayerTreeNode*> nodes;
  nodes << node;
  insertChildren(index, nodes);
}

void QgsLayerTreeNode::insertChildren(int index, QList<QgsLayerTreeNode*> nodes)
{
  foreach (QgsLayerTreeNode* node, nodes)
  {
    Q_ASSERT(node->mParent == 0);
    node->mParent = this;
  }

  if (index < 0 || index >= mChildren.count())
    index = mChildren.count();

  int indexTo = index+nodes.count()-1;
  emit willAddChildren(index, indexTo);
  for (int i = 0; i < nodes.count(); ++i)
    mChildren.insert(index+i, nodes[i]);
  emit addedChildren(index, indexTo);
}

void QgsLayerTreeNode::removeChildAt(int i)
{
  removeChildrenRange(i, 1);
}

void QgsLayerTreeNode::removeChildrenRange(int from, int count)
{
  if (count <= 0)
    return;

  int to = from+count-1;
  emit willRemoveChildren(from, to);
  while (--count >= 0)
  {
    QgsLayerTreeNode* node = mChildren.takeAt(from);
    node->mParent = 0;
    delete node;
  }
  emit removedChildren(from, to);
}
