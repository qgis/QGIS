/***************************************************************************
  qgsambientocclusionblurentity.cpp
  --------------------------------------
  Date                 : June 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsambientocclusionblurentity.h"

#include <Qt3DRender/QParameter>

QgsAmbientOcclusionBlurEntity::QgsAmbientOcclusionBlurEntity( Qt3DRender::QTexture2D *texture, QNode *parent )
  : QgsRenderPassQuad( parent )
{
  mAmbientOcclusionFactorTextureParameter = new Qt3DRender::QParameter( QStringLiteral( "texture" ), texture );
  mMaterial->addParameter( mAmbientOcclusionFactorTextureParameter );

  const QString vertexShaderPath = QStringLiteral( "qrc:/shaders/ssao_factor_blur.vert" );
  const QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/ssao_factor_blur.frag" );

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
}

