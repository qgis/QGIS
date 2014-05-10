#ifndef QGSLAYERTREEMODEL_H
#define QGSLAYERTREEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

class QgsLayerTreeNode;
class QgsLayerTreeGroup;
class QgsLayerTreeLayer;

/** internal class, not in public API */
class QgsLayerTreeModelSymbologyNode : public QObject
{
  Q_OBJECT
public:
  QgsLayerTreeModelSymbologyNode(QgsLayerTreeLayer* parent, const QString& name, const QIcon& icon = QIcon())
    : mParent(parent), mName(name), mIcon(icon) {}

  QgsLayerTreeLayer* parent() const { return mParent; }
  QString name() const { return mName; }
  QIcon icon() const { return mIcon; }

  // TODO: ref to renderer

protected:
  QgsLayerTreeLayer* mParent;
  QString mName;
  QIcon mIcon;
};



class GUI_EXPORT QgsLayerTreeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit QgsLayerTreeModel(QgsLayerTreeGroup* rootNode, QObject *parent = 0);
  ~QgsLayerTreeModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
  Qt::DropActions supportedDropActions() const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

  bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

  enum Flag
  {
    AllowTreeManagement,
    ShowSymbology,
    AllowVisibilityManagement,
    ShowFeatureCounts, // TODO: this is per-layer
  };
  Q_DECLARE_FLAGS(Flags, Flag)

  void setFlags(Flags f) { mFlags = f; }
  void setFlag(Flag f, bool on = true) { if (on) mFlags |= f; else mFlags &= ~f; }
  Flags flags() const { return mFlags; }
  bool testFlag(Flag f) const { return mFlags.testFlag(f); }

  // conversion functions used by views

  QgsLayerTreeNode* index2node(const QModelIndex& index) const;
  static QgsLayerTreeModelSymbologyNode* index2symnode(const QModelIndex& index);
  QModelIndex node2index(QgsLayerTreeNode* node);

  QList<QgsLayerTreeNode*> indexes2nodes(const QModelIndexList& list, bool skipInternal = false) const;

  QgsLayerTreeGroup* rootGroup() { return mRootNode; }

signals:

protected slots:
  void nodeWillAddChildren(int indexFrom, int indexTo);
  void nodeAddedChildren(int indexFrom, int indexTo);
  void nodeWillRemoveChildren(int indexFrom, int indexTo);
  void nodeRemovedChildren();

  void nodeVisibilityChanded();

protected:
  void connectToNode(QgsLayerTreeNode* node);
  void removeSymbologyFromSubtree(QgsLayerTreeNode* node);
  void addSymbologyToLayer(QgsLayerTreeLayer* nodeL) const;

protected:
  QgsLayerTreeGroup* mRootNode; // not owned!
  Flags mFlags;

  mutable QMap<QgsLayerTreeLayer*, QList<QgsLayerTreeModelSymbologyNode*> > mSymbologyNodes;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QgsLayerTreeModel::Flags)

#endif // QGSLAYERTREEMODEL_H
