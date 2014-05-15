#ifndef QGSLAYERNODE_H
#define QGSLAYERNODE_H

#include <QObject>

#include "qgsmaplayer.h"

/**
 * One node of the layer tree - group or layer.
 *
 * Once added to the tree, nodes are owned by their parent.
 */
class CORE_EXPORT QgsLayerTreeNode : public QObject
{
  Q_OBJECT
public:
  enum NodeType
  {
    NodeGroup,
    NodeLayer
  };

  ~QgsLayerTreeNode() { qDeleteAll(mChildren); }

  NodeType nodeType() { return mNodeType; }
  QgsLayerTreeNode* parent() { return mParent; }
  QList<QgsLayerTreeNode*> children() { return mChildren; }

  static QgsLayerTreeNode* readXML(QDomElement& element);
  virtual void writeXML(QDomElement& parentElement) = 0;

  virtual QString dump() const = 0;

  virtual QgsLayerTreeNode* clone() const = 0;

  bool isExpanded() const { return mExpanded; }
  void setExpanded(bool expanded) { mExpanded = expanded; }

  /** Set a custom property for the node. Properties are stored in a map and saved in project file. */
  void setCustomProperty( const QString& key, const QVariant& value );
  /** Read a custom property from layer. Properties are stored in a map and saved in project file. */
  QVariant customProperty( const QString& key, const QVariant& defaultValue = QVariant() ) const;
  /** Remove a custom property from layer. Properties are stored in a map and saved in project file. */
  void removeCustomProperty( const QString& key );
  /** Return list of keys stored in custom properties */
  QStringList customProperties() const;

signals:

  // low-level signals (mainly for the model)

  void willAddChildren(int indexFrom, int indexTo);
  void addedChildren(int indexFrom, int indexTo);

  void willRemoveChildren(int indexFrom, int indexTo);
  void removedChildren(int indexFrom, int indexTo);

  void visibilityChanged(Qt::CheckState state);

protected:

  QgsLayerTreeNode(NodeType t);
  QgsLayerTreeNode(const QgsLayerTreeNode& other);

  // low-level utility functions

  void readCommonXML(QDomElement& element);
  void writeCommonXML(QDomElement& element);

  // the child must not be in any tree yet!
  void addChild(QgsLayerTreeNode* node);
  void insertChildren(int index, QList<QgsLayerTreeNode*> nodes);
  void insertChild(int index, QgsLayerTreeNode* node);
  void removeChildAt(int i);
  void removeChildrenRange(int from, int count);


protected:
  NodeType mNodeType;
  QgsLayerTreeNode* mParent;
  QList<QgsLayerTreeNode*> mChildren;
  bool mExpanded;
  QgsObjectCustomProperties mProperties;
};

/**
 * Layer Node
 *
 * It is expected that the layer is registered in QgsMapLayerRegistry.
 *
 * One layer is supposed to be present in one layer tree just once. It is possible that temporarily a layer
 * temporarily exists in one tree more than once, e.g. while reordering items.
 *
 * Can exist also without a valid instance of a layer (just ID),
 * so that referenced layer does not need to be loaded in order to use it in layer tree.
 */
class QgsLayerTreeLayer : public QgsLayerTreeNode
{
  Q_OBJECT
public:
  explicit QgsLayerTreeLayer(QgsMapLayer* layer);
  QgsLayerTreeLayer(const QgsLayerTreeLayer& other);

  explicit QgsLayerTreeLayer(QString layerId, QString name = QString());

  QString layerId() const { return mLayerId; }

  QgsMapLayer* layer() const { return mLayer; }

  QString layerName() const { return mLayer ? mLayer->name() : mLayerName; }
  void setLayerName(const QString& n) { if (mLayer) mLayer->setLayerName(n); else mLayerName = n; }

  bool isVisible() const { return mVisible; }
  void setVisible(bool state);

  static QgsLayerTreeLayer* readXML(QDomElement& element);
  virtual void writeXML(QDomElement& parentElement);

  virtual QString dump() const;

  virtual QgsLayerTreeNode* clone() const;

protected slots:
  void registryLayersAdded(QList<QgsMapLayer*> layers);

signals:
  //! emitted when a previously unavailable layer got loaded
  void layerLoaded();

protected:
  void attachToLayer();

  QString mLayerId;
  QString mLayerName; // only used if layer does not exist
  QgsMapLayer* mLayer; // not owned! may be null
  bool mVisible;
};


class QgsLayerTreeGroup : public QgsLayerTreeNode
{
  Q_OBJECT
public:
  QgsLayerTreeGroup(const QString& name = QString(), Qt::CheckState checked = Qt::Checked);
  QgsLayerTreeGroup(const QgsLayerTreeGroup& other);

  QString name() const { return mName; }
  void setName(const QString& n) { mName = n; }

  QgsLayerTreeGroup* addGroup(const QString& name);
  QgsLayerTreeLayer* addLayer(QgsMapLayer* layer);

  void insertChildNodes(int index, QList<QgsLayerTreeNode*> nodes);
  void insertChildNode(int index, QgsLayerTreeNode* node);
  void addChildNode(QgsLayerTreeNode* node);

  void removeChildNode(QgsLayerTreeNode* node);

  void removeLayer(QgsMapLayer* layer);

  void removeChildren(int from, int count);

  void removeAllChildren();

  QgsLayerTreeLayer* findLayer(const QString& layerId);
  QgsLayerTreeGroup* findGroup(const QString& name);

  static QgsLayerTreeGroup* readXML(QDomElement& element);
  virtual void writeXML(QDomElement& parentElement);

  void readChildrenFromXML(QDomElement& element);

  virtual QString dump() const;

  virtual QgsLayerTreeNode* clone() const;

  Qt::CheckState isVisible() const { return mChecked; }
  void setVisible(Qt::CheckState state);

  QStringList childLayerIds() const;

protected slots:
  void layerDestroyed();
  void updateVisibilityFromChildren();

protected:
  void connectToChildNode(QgsLayerTreeNode* node);

protected:
  QString mName;
  Qt::CheckState mChecked;

  bool mChangingChildVisibility;
};



#endif // QGSLAYERNODE_H
