#include "qgslayertreenode.h"

#include "qgsmaplayerregistry.h"

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
}

QVariant QgsLayerTreeNode::customProperty(const QString &key, const QVariant &defaultValue) const
{
  return mProperties.value(key, defaultValue);
}

void QgsLayerTreeNode::removeCustomProperty(const QString &key)
{
  mProperties.remove(key);
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

// -----


QgsLayerTreeGroup::QgsLayerTreeGroup(const QString& name, Qt::CheckState checked)
  : QgsLayerTreeNode(NodeGroup)
  , mName(name)
  , mChecked(checked)
  , mChangingChildVisibility(false)
{
}

QgsLayerTreeGroup::QgsLayerTreeGroup(const QgsLayerTreeGroup& other)
  : QgsLayerTreeNode(other)
  , mName(other.mName)
  , mChecked(other.mChecked)
  , mChangingChildVisibility(false)
{
}


QgsLayerTreeGroup* QgsLayerTreeGroup::addGroup(const QString &name)
{
  QgsLayerTreeGroup* grp = new QgsLayerTreeGroup(name);
  addChildNode( grp );
  return grp;
}

QgsLayerTreeLayer* QgsLayerTreeGroup::addLayer(QgsMapLayer* layer)
{
  QgsLayerTreeLayer* ll = new QgsLayerTreeLayer(layer);
  addChildNode( ll );
  return ll;
}

void QgsLayerTreeGroup::connectToChildNode(QgsLayerTreeNode* node)
{
  if (node->nodeType() == QgsLayerTreeNode::NodeLayer)
  {
    // TODO: this could be handled directly by LayerTreeLayer by listening to QgsMapLayerRegistry...
    //QgsLayerTreeLayer* nodeLayer = static_cast<QgsLayerTreeLayer*>(node);
    //connect(nodeLayer->layer(), SIGNAL(destroyed()), this, SLOT(layerDestroyed()));
  }

  connect(node, SIGNAL(visibilityChanged(Qt::CheckState)), this, SLOT(updateVisibilityFromChildren()));
}

void QgsLayerTreeGroup::insertChildNode(int index, QgsLayerTreeNode* node)
{
  QList<QgsLayerTreeNode*> nodes;
  nodes << node;
  insertChildNodes(index, nodes);
}

void QgsLayerTreeGroup::insertChildNodes(int index, QList<QgsLayerTreeNode*> nodes)
{
  // low-level insert
  insertChildren(index, nodes);

  foreach (QgsLayerTreeNode* node, nodes)
    connectToChildNode(node);

  updateVisibilityFromChildren();
}

void QgsLayerTreeGroup::addChildNode(QgsLayerTreeNode* node)
{
  insertChildNode(-1, node);
}

void QgsLayerTreeGroup::removeChildNode(QgsLayerTreeNode *node)
{
  int i = mChildren.indexOf(node);
  if (i >= 0)
    removeChildAt(i);
}

void QgsLayerTreeGroup::removeLayer(QgsMapLayer* layer)
{
  foreach (QgsLayerTreeNode* child, mChildren)
  {
    if (child->nodeType() == QgsLayerTreeNode::NodeLayer)
    {
      QgsLayerTreeLayer* childLayer = static_cast<QgsLayerTreeLayer*>(child);
      if (childLayer->layer() == layer)
      {
        removeChildAt(mChildren.indexOf(child));
        break;
      }
    }
  }
}

void QgsLayerTreeGroup::removeChildren(int from, int count)
{
  removeChildrenRange(from, count);

  updateVisibilityFromChildren();
}

void QgsLayerTreeGroup::removeAllChildren()
{
  removeChildren(0, mChildren.count());
}

QgsLayerTreeLayer *QgsLayerTreeGroup::findLayer(const QString& layerId)
{
  foreach (QgsLayerTreeNode* child, mChildren)
  {
    if (child->nodeType() == QgsLayerTreeNode::NodeLayer)
    {
      QgsLayerTreeLayer* childLayer = static_cast<QgsLayerTreeLayer*>(child);
      if (childLayer->layerId() == layerId)
        return childLayer;
    }
    else if (child->nodeType() == QgsLayerTreeNode::NodeGroup)
    {
      QgsLayerTreeLayer* res = static_cast<QgsLayerTreeGroup*>(child)->findLayer(layerId);
      if (res)
        return res;
    }
  }
  return 0;
}

QgsLayerTreeGroup* QgsLayerTreeGroup::findGroup(const QString& name)
{
  foreach (QgsLayerTreeNode* child, mChildren)
  {
    if (child->nodeType() == QgsLayerTreeNode::NodeGroup)
    {
      QgsLayerTreeGroup* childGroup = static_cast<QgsLayerTreeGroup*>(child);
      if (childGroup->name() == name)
        return childGroup;
      else
      {
        QgsLayerTreeGroup* grp = childGroup->findGroup(name);
        if (grp)
          return grp;
      }
    }
  }
  return 0;
}

QgsLayerTreeGroup* QgsLayerTreeGroup::readXML(QDomElement& element)
{
  if (element.tagName() != "layer-tree-group")
    return 0;

  QString name = element.attribute("name");
  bool isExpanded = (element.attribute("expanded", "1") == "1");
  Qt::CheckState checked = QgsLayerTreeUtils::checkStateFromXml(element.attribute("checked"));

  QgsLayerTreeGroup* groupNode = new QgsLayerTreeGroup(name, checked);
  groupNode->setExpanded(isExpanded);

  groupNode->readCommonXML(element);

  groupNode->readChildrenFromXML(element);

  return groupNode;
}

void QgsLayerTreeGroup::writeXML(QDomElement& parentElement)
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement("layer-tree-group");
  elem.setAttribute("name", mName);
  elem.setAttribute("expanded", mExpanded ? "1" : "0");
  elem.setAttribute("checked", QgsLayerTreeUtils::checkStateToXml(mChecked));

  writeCommonXML(elem);

  foreach (QgsLayerTreeNode* node, mChildren)
    node->writeXML(elem);

  parentElement.appendChild(elem);
}

void QgsLayerTreeGroup::readChildrenFromXML(QDomElement& element)
{
  QDomElement childElem = element.firstChildElement();
  while (!childElem.isNull())
  {
    QgsLayerTreeNode* newNode = QgsLayerTreeNode::readXML(childElem);
    if (newNode)
      addChildNode(newNode);

    childElem = childElem.nextSiblingElement();
  }
}

QString QgsLayerTreeGroup::dump() const
{
  QString header = QString( "GROUP: %1 visible=%2 expanded=%3\n" ).arg( name() ).arg( mChecked ).arg( mExpanded );
  QStringList childrenDump;
  foreach (QgsLayerTreeNode* node, mChildren)
    childrenDump << node->dump().split( "\n" );
  for (int i = 0; i < childrenDump.count(); ++i)
    childrenDump[i].prepend( "  " );
  return header + childrenDump.join( "\n" );
}

QgsLayerTreeNode* QgsLayerTreeGroup::clone() const
{
  return new QgsLayerTreeGroup(*this);
}

void QgsLayerTreeGroup::setVisible(Qt::CheckState state)
{
  if (mChecked == state)
    return;

  mChecked = state;
  emit visibilityChanged(state);

  if (mChecked == Qt::Unchecked || mChecked == Qt::Checked)
  {
    mChangingChildVisibility = true; // guard against running again setVisible() triggered from children

    // update children to have the correct visibility
    foreach (QgsLayerTreeNode* child, mChildren)
    {
      if (child->nodeType() == NodeGroup)
        static_cast<QgsLayerTreeGroup*>(child)->setVisible(mChecked);
      else if (child->nodeType() == NodeLayer)
        static_cast<QgsLayerTreeLayer*>(child)->setVisible(mChecked == Qt::Checked);
    }

    mChangingChildVisibility = false;
  }
}

QStringList QgsLayerTreeGroup::childLayerIds() const
{
  QStringList lst;
  foreach (QgsLayerTreeNode* child, mChildren)
  {
    if (child->nodeType() == NodeGroup)
      lst << static_cast<QgsLayerTreeGroup*>(child)->childLayerIds();
    else if (child->nodeType() == NodeLayer)
      lst << static_cast<QgsLayerTreeLayer*>(child)->layerId();
  }
  return lst;
}


void QgsLayerTreeGroup::layerDestroyed()
{
  QgsMapLayer* layer = static_cast<QgsMapLayer*>(sender());
  removeLayer(layer);
}

void QgsLayerTreeGroup::updateVisibilityFromChildren()
{
  if (mChangingChildVisibility)
    return;

  if (mChildren.count() == 0)
    return;

  bool hasVisible = false, hasHidden = false;

  foreach (QgsLayerTreeNode* child, mChildren)
  {
    if (child->nodeType() == QgsLayerTreeNode::NodeLayer)
    {
      bool layerVisible = static_cast<QgsLayerTreeLayer*>(child)->isVisible();
      if (layerVisible) hasVisible = true;
      if (!layerVisible) hasHidden = true;
    }
    else if (child->nodeType() == QgsLayerTreeNode::NodeGroup)
    {
      Qt::CheckState state = static_cast<QgsLayerTreeGroup*>(child)->isVisible();
      if (state == Qt::Checked || state == Qt::PartiallyChecked) hasVisible = true;
      if (state == Qt::Unchecked || state == Qt::PartiallyChecked) hasHidden = true;
    }
  }

  Qt::CheckState newState;
  if (hasVisible && !hasHidden)
    newState = Qt::Checked;
  else if (hasHidden && !hasVisible)
    newState = Qt::Unchecked;
  else
    newState = Qt::PartiallyChecked;

  setVisible(newState);
}


// ----------


QgsLayerTreeLayer::QgsLayerTreeLayer(QgsMapLayer *layer)
  : QgsLayerTreeNode(NodeLayer), mLayerId(layer->id()), mLayer(layer), mVisible(true)
{
  Q_ASSERT( QgsMapLayerRegistry::instance()->mapLayer(mLayerId) == layer );
}

QgsLayerTreeLayer::QgsLayerTreeLayer(QString layerId, QString name)
  : QgsLayerTreeNode(NodeLayer)
  , mLayerId(layerId)
  , mLayerName(name)
  , mLayer(0)
  , mVisible(true)
{
  attachToLayer();
}

QgsLayerTreeLayer::QgsLayerTreeLayer(const QgsLayerTreeLayer& other)
  : QgsLayerTreeNode(other)
  , mLayerId(other.mLayerId)
  , mLayerName(other.mLayerName)
  , mLayer(0)
  , mVisible(other.mVisible)
{
  attachToLayer();
}

void QgsLayerTreeLayer::attachToLayer()
{
  // layer is not necessarily already loaded
  QgsMapLayer* l = QgsMapLayerRegistry::instance()->mapLayer(mLayerId);
  if (l)
  {
    mLayer = l;
    mLayerName = l->name();
  }
  else
  {
    if (mLayerName.isEmpty())
      mLayerName = "(?)";
    // wait for the layer to be eventually loaded
    connect(QgsMapLayerRegistry::instance(), SIGNAL(layersAdded(QList<QgsMapLayer*>)), this, SLOT(registryLayersAdded(QList<QgsMapLayer*>)));
  }
}

void QgsLayerTreeLayer::setVisible(bool state)
{
  if (mVisible == state)
    return;

  mVisible = state;
  emit visibilityChanged(state ? Qt::Checked : Qt::Unchecked);
}

QgsLayerTreeLayer* QgsLayerTreeLayer::readXML(QDomElement& element)
{
  if (element.tagName() != "layer-tree-layer")
    return 0;

  QString layerID = element.attribute("id");
  QString layerName = element.attribute("name");
  bool visible = element.attribute("visible").toInt();
  bool isExpanded = (element.attribute("expanded", "1") == "1");

  QgsLayerTreeLayer* nodeLayer = 0;

  // TODO: maybe allow other sources of layers than just registry singleton?
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer(layerID);

  if (layer)
    nodeLayer = new QgsLayerTreeLayer(layer);
  else
    nodeLayer = new QgsLayerTreeLayer(layerID, layerName);

  nodeLayer->readCommonXML(element);

  nodeLayer->setVisible(visible);
  nodeLayer->setExpanded(isExpanded);
  return nodeLayer;
}

void QgsLayerTreeLayer::writeXML(QDomElement& parentElement)
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement("layer-tree-layer");
  elem.setAttribute("id", mLayerId);
  elem.setAttribute("name", layerName());
  elem.setAttribute("visible", mVisible ? "1" : "0");
  elem.setAttribute("expanded", mExpanded ? "1" : "0");

  writeCommonXML(elem);

  parentElement.appendChild(elem);
}

QString QgsLayerTreeLayer::dump() const
{
  return QString( "LAYER: %1 visible=%2 expanded=%3 id=%4\n" ).arg( layerName() ).arg( mVisible ).arg( mExpanded ).arg( layerId() );
}

QgsLayerTreeNode* QgsLayerTreeLayer::clone() const
{
  return new QgsLayerTreeLayer(*this);
}

void QgsLayerTreeLayer::registryLayersAdded(QList<QgsMapLayer*> layers)
{
  foreach (QgsMapLayer* l, layers)
  {
    if (l->id() == mLayerId)
    {
      mLayer = l;
      disconnect(QgsMapLayerRegistry::instance(), SIGNAL(layersAdded(QList<QgsMapLayer*>)), this, SLOT(registryLayersAdded(QList<QgsMapLayer*>)));
      break;
    }
  }
}
