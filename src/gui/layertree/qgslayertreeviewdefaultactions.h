#ifndef QGSLAYERTREEVIEWDEFAULTACTIONS_H
#define QGSLAYERTREEVIEWDEFAULTACTIONS_H

#include <QObject>

class QAction;

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

  QAction* actionZoomToLayer(QgsMapCanvas* canvas, QObject* parent = 0);
  QAction* actionZoomToGroup(QgsMapCanvas* canvas, QObject* parent = 0);
  // TODO: zoom to selected

protected slots:
  void addGroup();
  void removeGroupOrLayer();
  void renameGroupOrLayer();
  void showInOverview();
  void zoomToLayer();
  void zoomToGroup();

protected:
  void zoomToLayers(QgsMapCanvas* canvas, const QList<QgsMapLayer*>& layers);

protected:
  QgsLayerTreeView* mView;
};


#endif // QGSLAYERTREEVIEWDEFAULTACTIONS_H
