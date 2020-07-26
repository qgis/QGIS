#ifndef SKYBOXRENDERINGSETTINGSWIDGET_H
#define SKYBOXRENDERINGSETTINGSWIDGET_H

#include "ui_skyboxrenderingsettingswidget.h"

class QgsSkyboxRenderingSettingsWidget : public QWidget, private Ui::SkyboxRenderingSettingsWidget
{
    Q_OBJECT

  public:
    explicit QgsSkyboxRenderingSettingsWidget(QWidget *parent = nullptr);
};

#endif // SKYBOXRENDERINGSETTINGSWIDGET_H
