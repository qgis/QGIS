#ifndef SKYBOXRENDERINGSETTINGSWIDGET_H
#define SKYBOXRENDERINGSETTINGSWIDGET_H

#include "ui_skyboxrenderingsettingswidget.h"

#include "qgsskyboxsettings.h"

class QgsSkyboxRenderingSettingsWidget : public QWidget, private Ui::SkyboxRenderingSettingsWidget
{
    Q_OBJECT

  public:
    explicit QgsSkyboxRenderingSettingsWidget( QWidget *parent = nullptr );

    QgsSkyboxSettings toSkyboxSettings()
    {
      QgsSkyboxSettings settings;
      settings.setIsSkyboxEnabled( mIsSkyboxEnabled );
      settings.setSkyboxBaseName( mSkyboxBaseName );
      settings.setSkyboxExtension( mSkyboxExtension );
      return settings;
    }

  signals:
    void skyboxSettingsChanged( const QgsSkyboxSettings &skyboxSettings );

  private:
    bool mIsSkyboxEnabled = false;
    QString mSkyboxBaseName;
    QString mSkyboxExtension;
};

#endif // SKYBOXRENDERINGSETTINGSWIDGET_H
