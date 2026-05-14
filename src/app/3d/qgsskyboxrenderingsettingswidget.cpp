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

#include <QCheckBox>
#include <QLineEdit>
#include <QString>

#include "moc_qgsskyboxrenderingsettingswidget.cpp"

// this is broken for z-up coordinate system
#define ENABLE_PANORAMIC_SKYBOX 0

using namespace Qt::StringLiterals;

QgsSkyboxRenderingSettingsWidget::QgsSkyboxRenderingSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
#if ENABLE_PANORAMIC_SKYBOX
  skyboxTypeComboBox->addItem( tr( "Panoramic Texture" ), QVariant::fromValue( Qgis::Map3DBackgroundType::NoBackground ) ); // this will have to fixed when panoramic skybox is fixed
#endif
  skyboxTypeComboBox->addItem( tr( "Distinct Faces" ), QVariant::fromValue( Qgis::Map3DBackgroundType::DistinctTextureSkybox ) );
  connect( skyboxTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsSkyboxRenderingSettingsWidget::showSkyboxSettings );

  mMappingComboBox->addItem( tr( "Native (Z-Up)" ), QVariant::fromValue( Qgis::SkyboxCubeMapping::NativeZUp ) );
  mMappingComboBox->addItem( tr( "OpenGL / WebGL (Y-Up)" ), QVariant::fromValue( Qgis::SkyboxCubeMapping::OpenGLYUp ) );
  mMappingComboBox->addItem( tr( "Godot Engine (Y-Up)" ), QVariant::fromValue( Qgis::SkyboxCubeMapping::GodotYUp ) );
  mMappingComboBox->addItem( tr( "Unreal Engine (Z-Up)" ), QVariant::fromValue( Qgis::SkyboxCubeMapping::UnrealEngineZUp ) );
  mMappingComboBox->addItem( tr( "Unity Engine / Left-Handed (Y-Up)" ), QVariant::fromValue( Qgis::SkyboxCubeMapping::LeftHandedYUpMirrored ) );
  showSkyboxSettings( 0 );
}

void QgsSkyboxRenderingSettingsWidget::setSkyboxSettings( const QgsSkyboxSettings &skyboxSettings )
{
  skyboxTypeComboBox->setCurrentIndex( skyboxTypeComboBox->findData( QVariant::fromValue( skyboxSettings.type() ) ) );

#if ENABLE_PANORAMIC_SKYBOX
  panoramicTextureImageSource->setSource( skyboxSettings.panoramicTexturePath() );
#endif
  QMap<QString, QString> cubeMapFaces = skyboxSettings.cubeMapFacesPaths();
  posXImageSource->setSource( cubeMapFaces[u"posX"_s] );
  posYImageSource->setSource( cubeMapFaces[u"posY"_s] );
  posZImageSource->setSource( cubeMapFaces[u"posZ"_s] );
  negXImageSource->setSource( cubeMapFaces[u"negX"_s] );
  negYImageSource->setSource( cubeMapFaces[u"negY"_s] );
  negZImageSource->setSource( cubeMapFaces[u"negZ"_s] );

  mMappingComboBox->setCurrentIndex( mMappingComboBox->findData( QVariant::fromValue( skyboxSettings.cubeMapping() ) ) );
}

QgsSkyboxSettings QgsSkyboxRenderingSettingsWidget::toSkyboxSettings()
{
  QgsSkyboxSettings settings;
#if ENABLE_PANORAMIC_SKYBOX
  settings.setPanoramicTexturePath( panoramicTextureImageSource->source() );
#endif
  settings.setCubeMapFace( u"posX"_s, posXImageSource->source() );
  settings.setCubeMapFace( u"posY"_s, posYImageSource->source() );
  settings.setCubeMapFace( u"posZ"_s, posZImageSource->source() );
  settings.setCubeMapFace( u"negX"_s, negXImageSource->source() );
  settings.setCubeMapFace( u"negY"_s, negYImageSource->source() );
  settings.setCubeMapFace( u"negZ"_s, negZImageSource->source() );
  settings.setCubeMapping( mMappingComboBox->currentData().value<Qgis::SkyboxCubeMapping>() );
  return settings;
}

void QgsSkyboxRenderingSettingsWidget::showSkyboxSettings( int )
{
  const Qgis::Map3DBackgroundType type = skyboxTypeComboBox->currentData().value<Qgis::Map3DBackgroundType>();
  bool showPanoramicWidgets = false;
  bool showDistinctFacesWidgets = false;


  switch ( type )
  {
#if ENABLE_PANORAMIC_SKYBOX
    case Qgis::Map3DBackgroundType::NoBackground: // this needs fixing when panoramic skybox is fixed
      showPanoramicWidgets = true;
      break;
#endif
    case Qgis::Map3DBackgroundType::DistinctTextureSkybox:
      showDistinctFacesWidgets = true;
      break;
    case Qgis::Map3DBackgroundType::NoBackground:
    case Qgis::Map3DBackgroundType::FixedGradientBackground:
      break;
  }

  mPanoramicWidget->setVisible( showPanoramicWidgets );
  mCubeMapWidget->setVisible( showDistinctFacesWidgets );
}
