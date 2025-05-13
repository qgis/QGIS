/***************************************************************************
  qgsdepthentity.cpp
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdepthentity.h"
#include "moc_qgsdepthentity.cpp"

#include <Qt3DRender/QParameter>
#include <QUrl>

#include "qgsframegraph.h"
#include "qgsabstractrenderview.h"

QgsDepthEntity::QgsDepthEntity( Qt3DRender::QTexture2D *texture, Qt3DRender::QLayer *layer, QNode *parent )
  : QgsRenderPassQuad( layer, parent )
{
  setObjectName( "depthRenderQuad" );

  // construct material
  Qt3DRender::QParameter *textureParameter = new Qt3DRender::QParameter( "depthTexture", texture );
  mMaterial->addParameter( textureParameter );

  const QString vertexShaderPath = QStringLiteral( "qrc:/shaders/depth_render.vert" );
  const QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/depth_render.frag" );

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
}
