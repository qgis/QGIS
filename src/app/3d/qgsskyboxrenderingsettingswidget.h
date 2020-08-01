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
      settings.setIsSkyboxEnabled( skyboxEnabledCheckBox->checkState() == Qt::CheckState::Checked );
      settings.setSkyboxType( skyboxTypeComboBox->currentText() );
      settings.setSkyboxBaseName( skyboxBaseNameLineEdit->text() );
      settings.setSkyboxExtension( skyboxExtensionLineEdit->text() );
      settings.setHDRTexturePath( hdrTextureImageSource->source() );
      settings.setCubeMapFace( QStringLiteral( "posX" ), posXImageSource->source() );
      settings.setCubeMapFace( QStringLiteral( "posY" ), posYImageSource->source() );
      settings.setCubeMapFace( QStringLiteral( "posZ" ), posZImageSource->source() );
      settings.setCubeMapFace( QStringLiteral( "negX" ), negXImageSource->source() );
      settings.setCubeMapFace( QStringLiteral( "negY" ), negYImageSource->source() );
      settings.setCubeMapFace( QStringLiteral( "negZ" ), negZImageSource->source() );
      return settings;
    }

  private:
    QVector<QGroupBox *> layoutGroupBoxes;
};

#endif // SKYBOXRENDERINGSETTINGSWIDGET_H
