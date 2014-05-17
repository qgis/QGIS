#ifndef QGSLAYERTREEREGISTRYBRIDGE_H
#define QGSLAYERTREEREGISTRYBRIDGE_H

#include <QObject>
#include <QStringList>

class QgsLayerTreeGroup;

class QgsMapLayer;

/**
 * Listens to the updates in map layer registry and does changes in layer tree
 */
class CORE_EXPORT QgsLayerTreeRegistryBridge : public QObject
{
  Q_OBJECT
public:
  explicit QgsLayerTreeRegistryBridge(QgsLayerTreeGroup* root, QObject *parent = 0);

  void setEnabled(bool enabled) { mEnabled = enabled; }
  bool isEnabled() const { return mEnabled; }

  //! Set where the new layers should be inserted - can be used to follow current selection.
  //! By default it is root group with zero index.
  void setLayerInsertionPoint(QgsLayerTreeGroup* parentGroup, int index);

signals:

protected slots:
  void layersAdded(QList<QgsMapLayer*> layers);
  void layersWillBeRemoved(QStringList layerIds);

  void groupAddedChildren(int indexFrom, int indexTo);
  void groupWillRemoveChildren(int indexFrom, int indexTo);
  void groupRemovedChildren();

protected:
  void connectToGroup(QgsLayerTreeGroup* group);

protected:
  QgsLayerTreeGroup* mRoot;
  QStringList mLayerIdsForRemoval;
  bool mEnabled;

  QgsLayerTreeGroup* mInsertionPointGroup;
  int mInsertionPointIndex;
};

#endif // QGSLAYERTREEREGISTRYBRIDGE_H
