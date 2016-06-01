#ifndef QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H
#define QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H

#include "ui_qgslayertreeembeddedconfigwidget.h"

class QgsLayerTreeLayer;

class GUI_EXPORT QgsLayerTreeEmbeddedConfigWidget : public QWidget, protected Ui::QgsLayerTreeEmbeddedConfigWidget
{
    Q_OBJECT
  public:
    QgsLayerTreeEmbeddedConfigWidget( QgsLayerTreeLayer* nodeLayer, QWidget* parent = nullptr );

  private slots:
    void onAddClicked();
    void onRemoveClicked();

  private:
    void updateCustomProperties();

  private:
    QgsLayerTreeLayer* mNodeLayer;
};

#endif // QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H
