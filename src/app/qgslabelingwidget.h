#ifndef QGSLABELINGWIDGET_H
#define QGSLABELINGWIDGET_H

#include <QWidget>

#include <ui_qgslabelingwidget.h>
#include <qgspallabeling.h>
#include "qgsvectorlayerlabeling.h"

class QgsLabelingGui;
class QgsMapCanvas;
class QgsRuleBasedLabelingWidget;
class QgsVectorLayer;
class QgsMapLayer;

/**
 * Master widget for configuration of labeling of a vector layer
 */
class QgsLabelingWidget : public QWidget, private Ui::QgsLabelingWidget
{
    Q_OBJECT
  public:
    QgsLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent = nullptr );

  public slots:
    void setLayer( QgsMapLayer *layer );
    void setDockMode( bool enabled );
    //! save config to layer
    void writeSettingsToLayer();

    //! Saves the labeling configuration and immediately updates the map canvas to reflect the changes
    void apply();

    //! reload the settings shown in the dialog from the current layer
    void adaptToLayer();

    void resetSettings();

  signals:
    void widgetChanged();

  protected slots:
    void labelModeChanged( int index );
    void showEngineConfigDialog();

  protected:
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mCanvas;

    bool mDockMode;

    QWidget* mWidget;
    QgsLabelingGui* mLabelGui;
    QScopedPointer< QgsAbstractVectorLayerLabeling > mOldSettings;
    QgsPalLayerSettings mOldPalSettings;
};

#endif // QGSLABELINGWIDGET_H
