#ifndef QGSLAYERTREEVIEWDEFAULTACTIONS_H
#define QGSLAYERTREEVIEWDEFAULTACTIONS_H

#include <QObject>

class QAction;

class QgsLayerTreeGroup;
class QgsLayerTreeView;
class QgsMapCanvas;
class QgsMapLayer;

class GUI_EXPORT QgsLayerTreeViewDefaultActions : public QObject
{
  Q_OBJECT
public:
  QgsLayerTreeViewDefaultActions(QgsLayerTreeView* view);

  QAction* actionAddGroup(QObject* parent = 0);
  QAction* actionRemoveGroupOrLayer(QObject* parent = 0);
  QAction* actionShowInOverview(QObject* parent = 0);
  QAction* actionRenameGroupOrLayer(QObject* parent = 0);
  QAction* actionShowFeatureCount(QObject* parent = 0);

  QAction* actionZoomToLayer(QgsMapCanvas* canvas, QObject* parent = 0);
  QAction* actionZoomToGroup(QgsMapCanvas* canvas, QObject* parent = 0);
  // TODO: zoom to selected

  QAction* actionMakeTopLevel(QObject* parent = 0);
  QAction* actionGroupSelected(QObject* parent = 0);

public slots:
  void addGroup();
  void removeGroupOrLayer();
  void renameGroupOrLayer();
  void showInOverview();
  void showFeatureCount();
  void zoomToLayer();
  void zoomToGroup();
  void makeTopLevel();
  void groupSelected();

protected:
  void zoomToLayers(QgsMapCanvas* canvas, const QList<QgsMapLayer*>& layers);

  QString uniqueGroupName(QgsLayerTreeGroup* parentGroup);

protected:
  QgsLayerTreeView* mView;
};


#endif // QGSLAYERTREEVIEWDEFAULTACTIONS_H
