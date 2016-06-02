#ifndef QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H
#define QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H

#include "ui_qgslayertreeembeddedconfigwidget.h"

class QgsMapLayer;

/** \ingroup gui
 * \class QgsLayerTreeEmbeddedConfigWidget
 * A widget to configure layer tree embedded widgets for a particular map layer.
 * @note introduced in QGIS 2.16
 */
class GUI_EXPORT QgsLayerTreeEmbeddedConfigWidget : public QWidget, protected Ui::QgsLayerTreeEmbeddedConfigWidget
{
    Q_OBJECT
  public:
    QgsLayerTreeEmbeddedConfigWidget( QWidget* parent = nullptr );

    //! Initialize widget with a map layer
    void setLayer( QgsMapLayer* layer );

    //! Store changes made in the widget to the layer
    void applyToLayer();

  private slots:
    void onAddClicked();
    void onRemoveClicked();

  private:
    QgsMapLayer* mLayer;
};

#endif // QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H
