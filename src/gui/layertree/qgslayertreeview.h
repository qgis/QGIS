#ifndef QGSLAYERTREEVIEW_H
#define QGSLAYERTREEVIEW_H

#include <QTreeView>

class QgsLayerTreeModel;
class QgsLayerTreeNode;

class GUI_EXPORT QgsLayerTreeView : public QTreeView
{
  Q_OBJECT
public:
  explicit QgsLayerTreeView(QWidget *parent = 0);

  virtual void setModel(QAbstractItemModel* model);

  QgsLayerTreeModel* layerTreeModel();

protected:
  void contextMenuEvent(QContextMenuEvent* event);

  QAction* create_addGroup(QObject* parent, QgsLayerTreeNode* parentNode = 0);
  QAction* create_removeGroupOrLayer(QObject* parent, QgsLayerTreeNode* parentNode);
  QAction* create_showInOverview(QObject* parent, QgsLayerTreeNode* parentNode);

  void updateExpandedStateFromNode(QgsLayerTreeNode* node);

signals:

public slots:

protected slots:
  void addGroup();
  void removeGroupOrLayer();
  void showInOverview();

  void modelRowsInserted(QModelIndex index, int start, int end);

  void updateExpandedStateToNode(QModelIndex index);

};

#endif // QGSLAYERTREEVIEW_H
