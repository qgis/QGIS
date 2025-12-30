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

#include "moc_qgsambientocclusionblurentity.cpp"

using namespace Qt::StringLiterals;

QgsAmbientOcclusionBlurEntity::QgsAmbientOcclusionBlurEntity( Qt3DRender::QTexture2D *texture, Qt3DRender::QLayer *layer, QNode *parent )
  : QgsRenderPassQuad( layer, parent )
{
  mAmbientOcclusionFactorTextureParameter = new Qt3DRender::QParameter( u"texture"_s, texture );
  mMaterial->addParameter( mAmbientOcclusionFactorTextureParameter );

  const QString vertexShaderPath = u"qrc:/shaders/ssao_factor_blur.vert"_s;
  const QString fragmentShaderPath = u"qrc:/shaders/ssao_factor_blur.frag"_s;

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
}
