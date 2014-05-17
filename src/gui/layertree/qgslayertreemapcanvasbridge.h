#ifndef QGSLAYERTREEMAPCANVASBRIDGE_H
#define QGSLAYERTREEMAPCANVASBRIDGE_H

#include <QObject>
#include <QStringList>

class QgsMapCanvas;
class QgsMapCanvasLayer;
class QgsLayerTreeGroup;
class QgsLayerTreeNode;

class GUI_EXPORT QgsLayerTreeMapCanvasBridge : public QObject
{
  Q_OBJECT
public:
  QgsLayerTreeMapCanvasBridge(QgsLayerTreeGroup* root, QgsMapCanvas* canvas);

  bool hasCustomLayerOrder() const { return mHasCustomLayerOrder; }
  QStringList customLayerOrder() const { return mCustomLayerOrder; }

  QStringList defaultLayerOrder() const;

public slots:
  void setHasCustomLayerOrder(bool override);
  void setCustomLayerOrder(const QStringList& order);

signals:
  void hasCustomLayerOrderChanged(bool);
  void customLayerOrderChanged(const QStringList& order);

protected:
  void connectToNode(QgsLayerTreeNode* node);

  // TODO: disconnectFromNode

  void defaultLayerOrder(QgsLayerTreeNode* node, QStringList& order) const;

  void setCanvasLayers(QgsLayerTreeNode* node, QList<QgsMapCanvasLayer>& layers);

  void deferredSetCanvasLayers();

protected slots:
  void nodeAddedChildren(int indexFrom, int indexTo);
  void nodeRemovedChildren();
  void nodeVisibilityChanged();
  void nodeCustomPropertyChanged(QgsLayerTreeNode* node, QString key);

  void setCanvasLayers();

protected:
  QgsLayerTreeGroup* mRoot;
  QgsMapCanvas* mCanvas;

  bool mPendingCanvasUpdate;

  bool mHasCustomLayerOrder;
  QStringList mCustomLayerOrder;
};

#endif // QGSLAYERTREEMAPCANVASBRIDGE_H
