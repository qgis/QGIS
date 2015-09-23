#ifndef QGSRULEBASEDLABELINGWIDGET_H
#define QGSRULEBASEDLABELINGWIDGET_H

#include <QWidget>

#include <ui_qgsrulebasedlabelingwidget.h>

class QgsMapCanvas;
class QgsVectorLayer;

class QgsRuleBasedLabelingWidget : public QWidget, private Ui::QgsRuleBasedLabelingWidget
{
    Q_OBJECT
  public:
    QgsRuleBasedLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent = 0 );

    //! load config from layer
    void init();
    //! save config to layer
    void writeSettingsToLayer();

  signals:

  public slots:

  protected:
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mCanvas;
};

#endif // QGSRULEBASEDLABELINGWIDGET_H
