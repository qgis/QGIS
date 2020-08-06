/***************************************************************************
  qgsskyboxrenderingsettingswidget.cpp
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

#include "qgsskyboxrenderingsettingswidget.h"

#include <QCheckBox>
#include <QLineEdit>
#include "qgs3dmapsettings.h"

QgsSkyboxRenderingSettingsWidget::QgsSkyboxRenderingSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  layoutGroupBoxes.push_back( panoramicTextureGroupBox );
  layoutGroupBoxes.push_back( faceTexturesGroupBox );

  skyboxTypeComboBox->addItem( tr( "Panoramic texture" ) );
  skyboxTypeComboBox->addItem( tr( "Distinct Faces" ) );
  connect( skyboxTypeComboBox, &QComboBox::currentTextChanged, [&]( const QString & skyboxType )
  {
    for ( QGroupBox *groupBox : layoutGroupBoxes )
      groupBox->setVisible( false );
    if ( skyboxType == tr( "Panoramic texture" ) )
      panoramicTextureGroupBox->setVisible( true );
    if ( skyboxType == tr( "Distinct Faces" ) )
      faceTexturesGroupBox->setVisible( true );
  } );
  skyboxTypeComboBox->setCurrentIndex( 1 );
}

void QgsSkyboxRenderingSettingsWidget::setSkyboxSettings( const QgsSkyboxSettings &skyboxSettings )
{
  skyboxEnabledCheckBox->setCheckState( skyboxSettings.isSkyboxEnabled() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

  switch ( skyboxSettings.skyboxType() )
  {
    case QgsSkyboxEntity::PanoramicSkybox:
      skyboxTypeComboBox->setCurrentText( tr( "Panoramic texture" ) );
      break;
    case QgsSkyboxEntity::DistinctTexturesSkybox:
      skyboxTypeComboBox->setCurrentText( tr( "Distinct Faces" ) );
      break;
  }

  panoramicTextureImageSource->setSource( skyboxSettings.panoramicTexturePath() );
  QMap<QString, QString> cubeMapFaces = skyboxSettings.cubeMapFacesPaths();
  posXImageSource->setSource( cubeMapFaces[ QStringLiteral( "posX" ) ] );
  posYImageSource->setSource( cubeMapFaces[ QStringLiteral( "posY" ) ] );
  posZImageSource->setSource( cubeMapFaces[ QStringLiteral( "posZ" ) ] );
  negXImageSource->setSource( cubeMapFaces[ QStringLiteral( "negX" ) ] );
  negYImageSource->setSource( cubeMapFaces[ QStringLiteral( "negY" ) ] );
  negZImageSource->setSource( cubeMapFaces[ QStringLiteral( "negZ" ) ] );
}

QgsSkyboxSettings QgsSkyboxRenderingSettingsWidget::toSkyboxSettings()
{
  QgsSkyboxSettings settings;
  settings.setIsSkyboxEnabled( skyboxEnabledCheckBox->checkState() == Qt::CheckState::Checked );

  if ( skyboxTypeComboBox->currentText() == tr( "Panoramic texture" ) )
    settings.setSkyboxType( QgsSkyboxEntity::PanoramicSkybox );
  else if ( skyboxTypeComboBox->currentText() == tr( "Distinct Faces" ) )
    settings.setSkyboxType( QgsSkyboxEntity::DistinctTexturesSkybox );

  settings.setPanoramicTexturePath( panoramicTextureImageSource->source() );
  settings.setCubeMapFace( QStringLiteral( "posX" ), posXImageSource->source() );
  settings.setCubeMapFace( QStringLiteral( "posY" ), posYImageSource->source() );
  settings.setCubeMapFace( QStringLiteral( "posZ" ), posZImageSource->source() );
  settings.setCubeMapFace( QStringLiteral( "negX" ), negXImageSource->source() );
  settings.setCubeMapFace( QStringLiteral( "negY" ), negYImageSource->source() );
  settings.setCubeMapFace( QStringLiteral( "negZ" ), negZImageSource->source() );
  return settings;
}
