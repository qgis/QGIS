/***************************************************************************
  qgsmaterial.cpp
  --------------------------------------
  Date                 : July 2024
  Copyright            : (C) 2024 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaterial.h"
#include "moc_qgsmaterial.cpp"
#include "qgs3dutils.h"

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QRenderPass>

const QString QgsMaterial::CLIP_PLANE_ARRAY_PARAMETER_NAME = QStringLiteral( "clipPlane[0]" );
const QString QgsMaterial::CLIP_PLANE_MAX_PLANE_PARAMETER_NAME = QStringLiteral( "max_plane_real" );
const QString QgsMaterial::CLIP_PLANE_DEFINE = QStringLiteral( "CLIPPING" );


QgsMaterial::QgsMaterial( QNode *parent )
  : QMaterial( parent )
{
}

QgsMaterial::~QgsMaterial() = default;

void QgsMaterial::enableClipping( const QList<QVector4D> &clipPlanesEquations )
{
  Qt3DRender::QEffect *materialEffect = effect();
  if ( !materialEffect )
    return;

  // First, disable possible existing clipping parameters
  disableClipping();
  if ( clipPlanesEquations.isEmpty() )
  {
    return;
  }

  // Add #define CLIPPING to the relevant shaders
  for ( Qt3DRender::QTechnique *technique : materialEffect->techniques() )
  {
    for ( Qt3DRender::QRenderPass *renderPass : technique->renderPasses() )
    {
      Qt3DRender::QShaderProgram *shaderProgram = renderPass->shaderProgram();
      const QByteArray geomCode = shaderProgram->geometryShaderCode();
      if ( !geomCode.isEmpty() )
      {
        const QByteArray newGeomCode = Qgs3DUtils::addDefinesToShaderCode( geomCode, QStringList( QgsMaterial::CLIP_PLANE_DEFINE ) );
        shaderProgram->setGeometryShaderCode( newGeomCode );
      }

      const QByteArray vertexCode = shaderProgram->vertexShaderCode();
      if ( !vertexCode.isEmpty() )
      {
        const QByteArray newVertexCode = Qgs3DUtils::addDefinesToShaderCode( vertexCode, QStringList( QgsMaterial::CLIP_PLANE_DEFINE ) );
        shaderProgram->setVertexShaderCode( newVertexCode );
      }
    }
  }

  // Add the clipping parameters
  const int nrClipPlanes = clipPlanesEquations.size();
  QVariantList clipPlanesEquationsVariant = QVariantList();
  for ( int i = 0; i < nrClipPlanes; ++i )
  {
    clipPlanesEquationsVariant << clipPlanesEquations[i];
  }
  Qt3DRender::QParameter *clipPlane = new Qt3DRender::QParameter( QgsMaterial::CLIP_PLANE_ARRAY_PARAMETER_NAME, clipPlanesEquationsVariant );
  Qt3DRender::QParameter *clipPlaneNumber = new Qt3DRender::QParameter( QgsMaterial::CLIP_PLANE_MAX_PLANE_PARAMETER_NAME, nrClipPlanes );

  materialEffect->addParameter( clipPlane );
  materialEffect->addParameter( clipPlaneNumber );

  mClippingEnabled = true;
}

void QgsMaterial::disableClipping()
{
  Qt3DRender::QEffect *materialEffect = effect();
  if ( !materialEffect || !mClippingEnabled )
    return;

  // Remove #define CLIPPING from the shaders
  for ( Qt3DRender::QTechnique *technique : materialEffect->techniques() )
  {
    for ( Qt3DRender::QRenderPass *renderPass : technique->renderPasses() )
    {
      Qt3DRender::QShaderProgram *shaderProgram = renderPass->shaderProgram();
      const QByteArray geomCode = shaderProgram->geometryShaderCode();
      if ( !geomCode.isEmpty() )
      {
        const QByteArray newGeomCode = Qgs3DUtils::removeDefinesFromShaderCode( geomCode, QStringList( QgsMaterial::CLIP_PLANE_DEFINE ) );
        shaderProgram->setGeometryShaderCode( newGeomCode );
      }

      const QByteArray vertexCode = shaderProgram->vertexShaderCode();
      if ( !vertexCode.isEmpty() )
      {
        const QByteArray newVertexCode = Qgs3DUtils::removeDefinesFromShaderCode( vertexCode, QStringList( QgsMaterial::CLIP_PLANE_DEFINE ) );
        shaderProgram->setVertexShaderCode( newVertexCode );
      }
    }
  }

  // Remove the parameters
  for ( Qt3DRender::QParameter *parameter : materialEffect->parameters() )
  {
    const QString parameterName = parameter->name();
    if ( parameterName == QgsMaterial::CLIP_PLANE_ARRAY_PARAMETER_NAME || parameterName == QgsMaterial::CLIP_PLANE_MAX_PLANE_PARAMETER_NAME )
    {
      materialEffect->removeParameter( parameter );
      break;
    }
  }

  mClippingEnabled = false;
}
