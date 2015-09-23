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
    QgsLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent = 0 );

    //! load config from layer
    void init();
    //! save config to layer
    void writeSettingsToLayer();

  signals:

  protected slots:
    void on_mLabelModeComboBox_currentIndexChanged( int index );
    void showEngineConfigDialog();

  protected:
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mCanvas;

    QgsLabelingGui* mWidgetSimple;
    QgsRuleBasedLabelingWidget* mWidgetRules;
};

#endif // QGSLABELINGWIDGET_H
