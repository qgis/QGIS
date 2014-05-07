#ifndef QGSLAYERTREEVIEW_H
#define QGSLAYERTREEVIEW_H

#include <QTreeView>

class QgsLayerTreeModel;
class QgsLayerTreeNode;
class QgsMapLayer;

class GUI_EXPORT QgsLayerTreeView : public QTreeView
{
  Q_OBJECT
public:
  explicit QgsLayerTreeView(QWidget *parent = 0);

  virtual void setModel(QAbstractItemModel* model);

  QgsLayerTreeModel* layerTreeModel();

  QgsMapLayer* currentLayer() const;
  void setCurrentLayer(QgsMapLayer* layer);

protected:
  void contextMenuEvent(QContextMenuEvent* event);

  QAction* create_addGroup(QObject* parent, QgsLayerTreeNode* parentNode = 0);
  QAction* create_removeGroupOrLayer(QObject* parent, QgsLayerTreeNode* parentNode);
  QAction* create_showInOverview(QObject* parent, QgsLayerTreeNode* parentNode);

  void updateExpandedStateFromNode(QgsLayerTreeNode* node);

signals:
  void currentLayerChanged(QgsMapLayer* layer);

public slots:

protected slots:
  void addGroup();
  void removeGroupOrLayer();
  void showInOverview();

  void modelRowsInserted(QModelIndex index, int start, int end);

  void updateExpandedStateToNode(QModelIndex index);

  void onCurrentChanged(QModelIndex current);

protected:
  QgsMapLayer* mCurrentLayer;
};

#endif // QGSLAYERTREEVIEW_H
