/***************************************************************************
  qgsbloomupsampleentity.cpp
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbloomupsampleentity.h"

#include <QString>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTexture>

#include "moc_qgsbloomupsampleentity.cpp"

using namespace Qt::StringLiterals;

QgsBloomUpsampleEntity::QgsBloomUpsampleEntity( Qt3DRender::QTexture2D *texture, Qt3DRender::QLayer *layer, QNode *parent )
  : QgsRenderPassQuad( layer, parent )
{
  mSourceTextureParameter = new Qt3DRender::QParameter( u"srcTexture"_s, texture );
  mMaterial->addParameter( mSourceTextureParameter );

  const QString vertexShaderPath = u"qrc:/shaders/postprocess.vert"_s;
  const QString fragmentShaderPath = u"qrc:/shaders/bloom_upsample.frag"_s;

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
}

void QgsBloomUpsampleEntity::addParameters( const QVector<Qt3DRender::QParameter *> &parameters )
{
  for ( Qt3DRender::QParameter *parameter : parameters )
  {
    mMaterial->addParameter( parameter );
  }
}
