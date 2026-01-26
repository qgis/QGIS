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

#include "qgis.h"
#include "qgs3dmapsettings.h"

#include <QCheckBox>
#include <QLineEdit>

#include "moc_qgsskyboxrenderingsettingswidget.cpp"

QgsSkyboxRenderingSettingsWidget::QgsSkyboxRenderingSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  // To future maintainers: make sure the order of added items is the same as the order at QgsSkyboxEntity::SkyboxType
  skyboxTypeComboBox->addItem( tr( "Panoramic Texture" ) );
  skyboxTypeComboBox->addItem( tr( "Distinct Faces" ) );
  connect( skyboxTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsSkyboxRenderingSettingsWidget::showSkyboxSettings );

  showSkyboxSettings( 0 );
}

void QgsSkyboxRenderingSettingsWidget::setSkyboxSettings( const QgsSkyboxSettings &skyboxSettings )
{
  switch ( skyboxSettings.skyboxType() )
  {
    case QgsSkyboxEntity::PanoramicSkybox:
      skyboxTypeComboBox->setCurrentIndex( 0 ); // "Panoramic Texture"
      break;
    case QgsSkyboxEntity::DistinctTexturesSkybox:
      skyboxTypeComboBox->setCurrentIndex( 1 ); // "Distinct Faces"
      break;
  }

  panoramicTextureImageSource->setSource( skyboxSettings.panoramicTexturePath() );
  QMap<QString, QString> cubeMapFaces = skyboxSettings.cubeMapFacesPaths();
  posXImageSource->setSource( cubeMapFaces[u"posX"_s] );
  posYImageSource->setSource( cubeMapFaces[u"posY"_s] );
  posZImageSource->setSource( cubeMapFaces[u"posZ"_s] );
  negXImageSource->setSource( cubeMapFaces[u"negX"_s] );
  negYImageSource->setSource( cubeMapFaces[u"negY"_s] );
  negZImageSource->setSource( cubeMapFaces[u"negZ"_s] );
}

QgsSkyboxSettings QgsSkyboxRenderingSettingsWidget::toSkyboxSettings()
{
  QgsSkyboxSettings settings;
  settings.setSkyboxType( static_cast<QgsSkyboxEntity::SkyboxType>( skyboxTypeComboBox->currentIndex() ) );
  settings.setPanoramicTexturePath( panoramicTextureImageSource->source() );
  settings.setCubeMapFace( u"posX"_s, posXImageSource->source() );
  settings.setCubeMapFace( u"posY"_s, posYImageSource->source() );
  settings.setCubeMapFace( u"posZ"_s, posZImageSource->source() );
  settings.setCubeMapFace( u"negX"_s, negXImageSource->source() );
  settings.setCubeMapFace( u"negY"_s, negYImageSource->source() );
  settings.setCubeMapFace( u"negZ"_s, negZImageSource->source() );
  return settings;
}

void QgsSkyboxRenderingSettingsWidget::showSkyboxSettings( int )
{
  const QgsSkyboxEntity::SkyboxType type = static_cast<QgsSkyboxEntity::SkyboxType>( skyboxTypeComboBox->currentIndex() );
  const bool isPanoramic = type == QgsSkyboxEntity::PanoramicSkybox;
  const bool isDistinctFaces = type == QgsSkyboxEntity::DistinctTexturesSkybox;

  panoramicTextureLabel->setVisible( isPanoramic );
  panoramicTextureImageSource->setVisible( isPanoramic );

  negXImageSourceLabel->setVisible( isDistinctFaces );
  negXImageSource->setVisible( isDistinctFaces );
  negYImageSourceLabel->setVisible( isDistinctFaces );
  negYImageSource->setVisible( isDistinctFaces );
  negZImageSourceLabel->setVisible( isDistinctFaces );
  negZImageSource->setVisible( isDistinctFaces );
  posXImageSourceLabel->setVisible( isDistinctFaces );
  posXImageSource->setVisible( isDistinctFaces );
  posYImageSourceLabel->setVisible( isDistinctFaces );
  posYImageSource->setVisible( isDistinctFaces );
  posZImageSourceLabel->setVisible( isDistinctFaces );
  posZImageSource->setVisible( isDistinctFaces );
}
