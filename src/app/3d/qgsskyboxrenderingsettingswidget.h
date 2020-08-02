/***************************************************************************
  qgsskyboxrenderingsettingswidget.h
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb uderscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SKYBOXRENDERINGSETTINGSWIDGET_H
#define SKYBOXRENDERINGSETTINGSWIDGET_H

#include "ui_skyboxrenderingsettingswidget.h"

#include "qgsskyboxsettings.h"

class QgsSkyboxRenderingSettingsWidget : public QWidget, private Ui::SkyboxRenderingSettingsWidget
{
    Q_OBJECT

  public:
    explicit QgsSkyboxRenderingSettingsWidget( QWidget *parent = nullptr );

    void setSkyboxSettings( const QgsSkyboxSettings &skyboxSettings )
    {
      skyboxEnabledCheckBox->setCheckState( skyboxSettings.isSkyboxEnabled() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
      skyboxTypeComboBox->setCurrentText( skyboxSettings.skyboxType() );
      skyboxBaseNameLineEdit->setText( skyboxSettings.skyboxBaseName() );
      skyboxExtensionLineEdit->setText( skyboxSettings.skyboxExtension() );
      hdrTextureImageSource->setSource( skyboxSettings.hdrTexturePath() );
      QMap<QString, QString> cubeMapFaces = skyboxSettings.cubeMapFacesPaths();
      posXImageSource->setSource( cubeMapFaces[ QStringLiteral( "posX" ) ] );
      posYImageSource->setSource( cubeMapFaces[ QStringLiteral( "posY" ) ] );
      posZImageSource->setSource( cubeMapFaces[ QStringLiteral( "posZ" ) ] );
      negXImageSource->setSource( cubeMapFaces[ QStringLiteral( "negX" ) ] );
      negYImageSource->setSource( cubeMapFaces[ QStringLiteral( "negY" ) ] );
      negZImageSource->setSource( cubeMapFaces[ QStringLiteral( "negZ" ) ] );
    }

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
