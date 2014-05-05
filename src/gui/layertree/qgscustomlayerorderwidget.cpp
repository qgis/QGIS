#include "qgscustomlayerorderwidget.h"

#include <QCheckBox>
#include <QListView>
#include <QMimeData>
#include <QVBoxLayout>

#include "qgslayertreemapcanvasbridge.h"

#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"


class CustomLayerOrderModel : public QAbstractListModel
{
public:
  CustomLayerOrderModel(QgsLayerTreeMapCanvasBridge* bridge, QObject* parent = 0)
    : QAbstractListModel(parent), mBridge(bridge)
  {
  }

  int rowCount(const QModelIndex & ) const
  {
    return mOrder.count();
  }

  QVariant data(const QModelIndex &index, int role) const
  {
    if (role == Qt::DisplayRole)
    {
      QString id = mOrder.at(index.row());
      QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer(id);
      if (layer)
        return layer->name();
    }

    if (role == Qt::UserRole+1)
    {
      QString id = mOrder.at(index.row());
      QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer(id);
      if (layer)
        return layer->id();
    }

    if (role == Qt::CheckStateRole)
    {
      // TODO: layer visibility
      return Qt::Checked;
    }

    return QVariant();
  }

  Qt::ItemFlags flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled; // | Qt::ItemIsUserCheckable;
  }

  Qt::DropActions supportedDropActions() const
  {
    return Qt::MoveAction;
  }

  QStringList mimeTypes() const
  {
    QStringList types;
    types << "application/qgis.layerorderdata";
    return types;
  }

  QMimeData* mimeData(const QModelIndexList& indexes) const
  {
    QStringList lst;
    foreach (QModelIndex index, indexes)
      lst << data(index, Qt::UserRole+1).toString();

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("application/qgis.layerorderdata", lst.join("\n").toUtf8());
    return mimeData;
  }

  bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
  {
    Q_UNUSED(parent);
    Q_UNUSED(column);

    if (action == Qt::IgnoreAction)
      return true;

    if (!data->hasFormat("application/qgis.layerorderdata"))
      return false;

    QByteArray encodedData = data->data("application/qgis.layerorderdata");
    QStringList lst = QString::fromUtf8(encodedData).split("\n");

    if (row < 0)
      row = mOrder.count();

    beginInsertRows(QModelIndex(), row, row+lst.count()-1);
    for (int i = 0; i < lst.count(); ++i)
      mOrder.insert(row+i, lst[i]);
    endInsertRows();

    return true;
  }

  bool removeRows(int row, int count, const QModelIndex& parent)
  {
    Q_UNUSED(parent);
    if (count <= 0)
      return false;

    beginRemoveRows(QModelIndex(), row, row+count-1);
    while (--count >= 0)
      mOrder.removeAt(row);
    endRemoveRows();
    return true;
  }

  void refreshModel(const QStringList& order)
  {
    beginResetModel();
    mOrder = order;
    endResetModel();
  }

  QStringList order() const { return mOrder; }

protected:
  QgsLayerTreeMapCanvasBridge* mBridge;
  QStringList mOrder;
};


QgsCustomLayerOrderWidget::QgsCustomLayerOrderWidget(QgsLayerTreeMapCanvasBridge* bridge, QWidget* parent)
  : QWidget(parent)
  , mBridge(bridge)
{
  mModel = new CustomLayerOrderModel(bridge, this);

  mView = new QListView(this);
  mView->setDragEnabled(true);
  mView->setAcceptDrops(true);
  mView->setDropIndicatorShown(true);
  mView->setDragDropMode(QAbstractItemView::InternalMove);
  mView->setSelectionMode(QAbstractItemView::ExtendedSelection);

  mView->setModel(mModel);

  mChkOverride = new QCheckBox( tr( "Control rendering order" ) );
  bridgeHasCustomLayerOrderChanged( bridge->hasCustomLayerOrder() );
  connect( mChkOverride, SIGNAL( toggled( bool ) ), bridge, SLOT( setHasCustomLayerOrder(bool) ) );

  connect(bridge, SIGNAL(hasCustomLayerOrderChanged(bool)), this, SLOT(bridgeHasCustomLayerOrderChanged(bool)));
  connect(bridge, SIGNAL(customLayerOrderChanged(QStringList)), this, SLOT(bridgeCustomLayerOrderChanged(QStringList)));

  connect(mModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(modelUpdated()));
  connect(mModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(modelUpdated()));

  QVBoxLayout* l = new QVBoxLayout;
  l->setMargin( 0 );
  l->addWidget( mView );
  l->addWidget( mChkOverride );
  setLayout( l );
}

void QgsCustomLayerOrderWidget::bridgeHasCustomLayerOrderChanged(bool override)
{
  mChkOverride->setChecked(override);
  mModel->refreshModel( mBridge->hasCustomLayerOrder() ? mBridge->customLayerOrder() : mBridge->defaultLayerOrder() );
  mView->setEnabled(override);
}

void QgsCustomLayerOrderWidget::bridgeCustomLayerOrderChanged(const QStringList& order)
{
  Q_UNUSED(order);
  mModel->refreshModel(mBridge->hasCustomLayerOrder() ? mBridge->customLayerOrder() : mBridge->defaultLayerOrder());
}

void QgsCustomLayerOrderWidget::modelUpdated()
{
  mBridge->setCustomLayerOrder(mModel->order());
}
