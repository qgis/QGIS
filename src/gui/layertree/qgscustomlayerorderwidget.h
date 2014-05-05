#ifndef QGSCUSTOMLAYERORDERWIDGET_H
#define QGSCUSTOMLAYERORDERWIDGET_H

#include <QWidget>

class CustomLayerOrderModel;
class QgsLayerTreeMapCanvasBridge;

class QCheckBox;
class QListView;

class GUI_EXPORT QgsCustomLayerOrderWidget : public QWidget
{
  Q_OBJECT
public:
  explicit QgsCustomLayerOrderWidget(QgsLayerTreeMapCanvasBridge* bridge, QWidget *parent = 0);

signals:

protected slots:
  void bridgeHasCustomLayerOrderChanged(bool override);
  void bridgeCustomLayerOrderChanged(const QStringList& order);

  void modelUpdated();

protected:
  QgsLayerTreeMapCanvasBridge* mBridge;

  QCheckBox* mChkOverride;
  CustomLayerOrderModel* mModel;
  QListView* mView;
};

#endif // QGSCUSTOMLAYERORDERWIDGET_H
