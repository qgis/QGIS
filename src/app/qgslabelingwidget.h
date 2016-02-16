#ifndef QGSLABELINGWIDGET_H
#define QGSLABELINGWIDGET_H

#include <QWidget>

#include <ui_qgslabelingwidget.h>

class QgsLabelingGui;
class QgsMapCanvas;
class QgsRuleBasedLabelingWidget;
class QgsVectorLayer;

/**
 * Master widget for configuration of labeling of a vector layer
 */
class QgsLabelingWidget : public QWidget, private Ui::QgsLabelingWidget
{
    Q_OBJECT
  public:
    QgsLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent = nullptr );

  public slots:
    //! save config to layer
    void writeSettingsToLayer();

    //! Saves the labeling configuration and immediately updates the map canvas to reflect the changes
    void apply();

    //! reload the settings shown in the dialog from the current layer
    void adaptToLayer();

  protected slots:
    void labelModeChanged( int index );
    void showEngineConfigDialog();

  protected:
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mCanvas;

    QWidget* mWidget;
};

#endif // QGSLABELINGWIDGET_H
