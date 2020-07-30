#ifndef SKYBOXRENDERINGSETTINGSWIDGET_H
#define SKYBOXRENDERINGSETTINGSWIDGET_H

#include "ui_skyboxrenderingsettingswidget.h"

class Qgs3DMapSettings;

class QgsSkyboxRenderingSettingsWidget : public QWidget, private Ui::SkyboxRenderingSettingsWidget
{
    Q_OBJECT

  public:
    explicit QgsSkyboxRenderingSettingsWidget( Qgs3DMapSettings *map, QWidget *parent = nullptr );

    bool isSkyboxEnabled() const { return mIsSkyboxEnabled; }
    QString skyboxPrefix() const { return mSkyboxPrefix; }
    QString skyboxExtension() const { return mSkyboxExt; }

  signals:
    void skyboxSettingsChanged();

  private:
    Qgs3DMapSettings *mMapSettings = nullptr;
    bool mIsSkyboxEnabled = false;
    QString mSkyboxPrefix;
    QString mSkyboxExt;
};

#endif // SKYBOXRENDERINGSETTINGSWIDGET_H
