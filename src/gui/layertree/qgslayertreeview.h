#ifndef QGSLAYERTREEVIEW_H
#define QGSLAYERTREEVIEW_H

#include <QTreeView>

class QgsLayerTreeGroup;
class QgsLayerTreeModel;
class QgsLayerTreeNode;
class QgsLayerTreeViewDefaultActions;
class QgsMapLayer;

class GUI_EXPORT QgsLayerTreeView : public QTreeView
{
  Q_OBJECT
public:
  explicit QgsLayerTreeView(QWidget *parent = 0);

  virtual void setModel(QAbstractItemModel* model);

  QgsLayerTreeModel* layerTreeModel() const;

  QgsLayerTreeViewDefaultActions* defaultActions();

  QgsMapLayer* currentLayer() const;
  void setCurrentLayer(QgsMapLayer* layer);

  QgsLayerTreeNode* currentNode() const;
  QgsLayerTreeGroup* currentGroupNode() const;

  QList<QgsLayerTreeNode*> selectedNodes(bool skipInternal = false) const;

protected:
  void contextMenuEvent(QContextMenuEvent* event);

  void updateExpandedStateFromNode(QgsLayerTreeNode* node);

signals:
  void currentLayerChanged(QgsMapLayer* layer);

public slots:

protected slots:

  void modelRowsInserted(QModelIndex index, int start, int end);

  void updateExpandedStateToNode(QModelIndex index);

  void onCurrentChanged(QModelIndex current);

protected:
  QgsMapLayer* mCurrentLayer;

  QgsLayerTreeViewDefaultActions* mDefaultActions;
};


#endif // QGSLAYERTREEVIEW_H
