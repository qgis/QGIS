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

  void willAddChildren(QgsLayerTreeNode* node, int indexFrom, int indexTo);
  void addedChildren(QgsLayerTreeNode* node, int indexFrom, int indexTo);

  void willRemoveChildren(QgsLayerTreeNode* node, int indexFrom, int indexTo);
  void removedChildren(QgsLayerTreeNode* node, int indexFrom, int indexTo);

  void visibilityChanged(Qt::CheckState state);

  void customPropertyChanged(QgsLayerTreeNode* node, QString key);

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




#endif // QGSLAYERNODE_H
