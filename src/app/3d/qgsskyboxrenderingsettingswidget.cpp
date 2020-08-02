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

  layoutGroupBoxes.push_back( textureCollectionGroupBox );
  layoutGroupBoxes.push_back( hdrTextureGroupBox );
  layoutGroupBoxes.push_back( faceTexturesGroupBox );

  skyboxTypeComboBox->addItem( QStringLiteral( "Textures collection" ) );
  skyboxTypeComboBox->addItem( QStringLiteral( "HDR texture" ) );
  skyboxTypeComboBox->addItem( QStringLiteral( "Distinct Faces" ) );
  connect( skyboxTypeComboBox, &QComboBox::currentTextChanged, [&]( const QString & skyboxType )
  {
    for ( QGroupBox *groupBox : layoutGroupBoxes )
      groupBox->setVisible( false );
    if ( skyboxType == QStringLiteral( "Textures collection" ) )
      textureCollectionGroupBox->setVisible( true );
    if ( skyboxType == QStringLiteral( "HDR texture" ) )
      hdrTextureGroupBox->setVisible( true );
    if ( skyboxType == QStringLiteral( "Distinct Faces" ) )
      faceTexturesGroupBox->setVisible( true );
  } );
  skyboxTypeComboBox->setCurrentIndex( 1 );
}
