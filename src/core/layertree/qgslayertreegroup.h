#ifndef QGSLAYERTREEGROUP_H
#define QGSLAYERTREEGROUP_H

#include "qgslayertreenode.h"

class QgsLayerTreeLayer;

class QgsLayerTreeGroup : public QgsLayerTreeNode
{
  Q_OBJECT
public:
  QgsLayerTreeGroup(const QString& name = QString(), Qt::CheckState checked = Qt::Checked);
  QgsLayerTreeGroup(const QgsLayerTreeGroup& other);

  QString name() const { return mName; }
  void setName(const QString& n) { mName = n; }

  QgsLayerTreeGroup* addGroup(const QString& name);
  QgsLayerTreeLayer* insertLayer(int index, QgsMapLayer* layer);
  QgsLayerTreeLayer* addLayer(QgsMapLayer* layer);

  void insertChildNodes(int index, QList<QgsLayerTreeNode*> nodes);
  void insertChildNode(int index, QgsLayerTreeNode* node);
  void addChildNode(QgsLayerTreeNode* node);

  void removeChildNode(QgsLayerTreeNode* node);

  void removeLayer(QgsMapLayer* layer);

  void removeChildren(int from, int count);

  void removeAllChildren();

  QgsLayerTreeLayer* findLayer(const QString& layerId);
  QList<QgsLayerTreeLayer*> findLayers() const;
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


#endif // QGSLAYERTREEGROUP_H
