#include "qgslayertreeviewdefaultactions.h"

#include "qgsapplication.h"
#include "qgslayertreemodel.h"
#include "qgslayertreenode.h"
#include "qgslayertreeview.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"

#include <QAction>

QgsLayerTreeViewDefaultActions::QgsLayerTreeViewDefaultActions(QgsLayerTreeView* view)
  : QObject(view)
  , mView(view)
{
}

QAction* QgsLayerTreeViewDefaultActions::actionAddGroup(QObject* parent)
{
  QAction* a = new QAction(tr("&Add Group"), parent);
  connect(a, SIGNAL(triggered()), this, SLOT(addGroup()));
  return a;
}

QAction* QgsLayerTreeViewDefaultActions::actionRemoveGroupOrLayer(QObject* parent)
{
  QAction* a = new QAction(QgsApplication::getThemeIcon( "/mActionRemoveLayer.svg" ), tr("&Remove"), parent);
  connect(a, SIGNAL(triggered()), this, SLOT(removeGroupOrLayer()));
  return a;
}

QAction* QgsLayerTreeViewDefaultActions::actionShowInOverview(QObject* parent)
{
  QgsLayerTreeNode* node = mView->currentNode();
  if (!node)
    return 0;

  QAction* a = new QAction(tr("&Show in overview"), parent);
  connect(a, SIGNAL(triggered()), this, SLOT(showInOverview()));
  a->setCheckable(true);
  a->setChecked(node->customProperty("overview", 0).toInt());
  return a;
}

QAction* QgsLayerTreeViewDefaultActions::actionRenameGroupOrLayer(QObject* parent)
{
  QAction* a = new QAction(tr("Re&name"), parent);
  connect(a, SIGNAL(triggered()), this, SLOT(renameGroupOrLayer()));
  return a;
}

QAction* QgsLayerTreeViewDefaultActions::actionShowFeatureCount(QObject* parent)
{
  QgsLayerTreeNode* node = mView->currentNode();
  if (!node)
    return 0;

  QAction* a = new QAction(tr("Show Feature Count"), parent);
  connect(a, SIGNAL(triggered()), this, SLOT(showFeatureCount()));
  a->setCheckable(true);
  a->setChecked(node->customProperty("showFeatureCount", 0).toInt());
  return a;
}

QAction* QgsLayerTreeViewDefaultActions::actionZoomToLayer(QgsMapCanvas* canvas, QObject* parent)
{
  QAction* a = new QAction(QgsApplication::getThemeIcon( "/mActionZoomToLayer.svg" ),
                           tr("&Zoom to Layer"), parent);
  a->setData(QVariant::fromValue(reinterpret_cast<void*>(canvas)));
  connect(a, SIGNAL(triggered()), this, SLOT(zoomToLayer()));
  return a;
}

QAction* QgsLayerTreeViewDefaultActions::actionZoomToGroup(QgsMapCanvas* canvas, QObject* parent)
{
  QAction* a = new QAction(QgsApplication::getThemeIcon( "/mActionZoomToLayer.svg" ),
                           tr("&Zoom to Group"), parent);
  a->setData(QVariant::fromValue(reinterpret_cast<void*>(canvas)));
  connect(a, SIGNAL(triggered()), this, SLOT(zoomToGroup()));
  return a;
}

void QgsLayerTreeViewDefaultActions::addGroup()
{
  QgsLayerTreeGroup* group = mView->currentGroupNode();
  QString prefix = group == mView->layerTreeModel()->rootGroup() ? "group" : "sub-group";

  QString newName = prefix + "1";
  for ( int i = 2; group->findGroup(newName); ++i)
    newName = prefix + QString::number(i);

  QgsLayerTreeGroup* newGroup = group->addGroup(newName);
  mView->edit( mView->layerTreeModel()->node2index(newGroup) );
}

void QgsLayerTreeViewDefaultActions::removeGroupOrLayer()
{
  foreach (QgsLayerTreeNode* node, mView->selectedNodes(true))
  {
    // could be more efficient if working directly with ranges instead of individual nodes
    qobject_cast<QgsLayerTreeGroup*>(node->parent())->removeChildNode(node);
  }
}

void QgsLayerTreeViewDefaultActions::renameGroupOrLayer()
{
  mView->edit(mView->currentIndex());
}

void QgsLayerTreeViewDefaultActions::showInOverview()
{
  QgsLayerTreeNode* node = mView->currentNode();
  if (!node)
    return;

  node->setCustomProperty("overview", node->customProperty("overview", 0).toInt() ? 0 : 1);
}

void QgsLayerTreeViewDefaultActions::showFeatureCount()
{
  QgsLayerTreeNode* node = mView->currentNode();
  if (!node || node->nodeType() != QgsLayerTreeNode::NodeLayer)
    return;


  node->setCustomProperty("showFeatureCount", node->customProperty("showFeatureCount", 0).toInt() ? 0 : 1);

  mView->layerTreeModel()->refreshLayerSymbology(static_cast<QgsLayerTreeLayer*>(node));
}

void QgsLayerTreeViewDefaultActions::zoomToLayer()
{
  QAction* s = qobject_cast<QAction*>(sender());
  QgsMapCanvas* canvas = reinterpret_cast<QgsMapCanvas*>(s->data().value<void*>());

  QgsMapLayer* layer = mView->currentLayer();
  if (!layer)
    return;

  QList<QgsMapLayer*> layers;
  layers << layer;
  zoomToLayers(canvas, layers);
}

void QgsLayerTreeViewDefaultActions::zoomToGroup()
{
  QAction* s = qobject_cast<QAction*>(sender());
  QgsMapCanvas* canvas = reinterpret_cast<QgsMapCanvas*>(s->data().value<void*>());

  QList<QgsMapLayer*> layers;
  foreach (QString layerId, mView->currentGroupNode()->childLayerIds())
    layers << QgsMapLayerRegistry::instance()->mapLayer(layerId);

  zoomToLayers(canvas, layers);
}


void QgsLayerTreeViewDefaultActions::zoomToLayers(QgsMapCanvas* canvas, const QList<QgsMapLayer*>& layers)
{
  QgsRectangle extent;

  for ( int i = 0; i < layers.size(); ++i )
  {
    QgsMapLayer* layer = layers.at( i );
    QgsRectangle layerExtent = layer->extent();

    QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer*>( layer );

    if ( layerExtent.isEmpty() && layer->type() == QgsMapLayer::VectorLayer )
    {
      qobject_cast<QgsVectorLayer*>( layer )->updateExtents();
      layerExtent = vLayer->extent();
    }

    //transform extent if otf-projection is on
    if ( canvas->hasCrsTransformEnabled() )
      layerExtent = canvas->mapSettings().layerExtentToOutputExtent( layer, layerExtent );

    if ( i == 0 )
      extent = layerExtent;
    else
      extent.combineExtentWith( &layerExtent );
  }

  if ( extent.isEmpty() )
    return;

  // Increase bounding box with 5%, so that layer is a bit inside the borders
  extent.scale( 1.05 );

  //zoom to bounding box
  canvas->setExtent( extent );
  canvas->refresh();
}
