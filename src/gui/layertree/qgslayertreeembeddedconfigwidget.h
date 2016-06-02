#ifndef QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H
#define QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H

#include "ui_qgslayertreeembeddedconfigwidget.h"

class QgsMapLayer;

class GUI_EXPORT QgsLayerTreeEmbeddedConfigWidget : public QWidget, protected Ui::QgsLayerTreeEmbeddedConfigWidget
{
    Q_OBJECT
  public:
    QgsLayerTreeEmbeddedConfigWidget( QWidget* parent = nullptr );

    void setLayer( QgsMapLayer* layer );

    void applyToLayer();

  private slots:
    void onAddClicked();
    void onRemoveClicked();

  private:
    QgsMapLayer* mLayer;
};

#endif // QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H
