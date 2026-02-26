/***************************************************************************
    qgshighlightmaterial.cpp
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshighlightmaterial.h"

#include "qgssettings.h"

#include <QColor>
#include <QString>
#include <QUrl>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

#include "moc_qgshighlightmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QgsHighlightMaterial::QgsHighlightMaterial( QgsMaterialSettingsRenderingTechnique technique, QNode *parent )
  : QgsMaterial( parent )
{
  init( technique );
}

QgsHighlightMaterial::~QgsHighlightMaterial() = default;

void QgsHighlightMaterial::init( QgsMaterialSettingsRenderingTechnique renderingTechnique )
{
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );

  Qt3DRender::QRenderPass *pass = new Qt3DRender::QRenderPass;

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram;
  switch ( renderingTechnique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
    {
      shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) ) );
      break;
    }
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    {
      shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/instanced.vert"_s ) ) );
      break;
    }
    case QgsMaterialSettingsRenderingTechnique::Lines:
    case QgsMaterialSettingsRenderingTechnique::Points:
    {
      // Lines are single color and do not need the highlight material
      // Billboards are not supported yet
      break;
    }
  }

  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/singlecolor.frag"_s ) ) );
  pass->setShaderProgram( shaderProgram );

  const QgsSettings settings;
  const float alpha = settings.value( u"Map/highlight/colorAlpha"_s, Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toFloat() / 255.f;
  QColor color = QColor( settings.value( u"Map/highlight/color"_s, Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  color.setAlphaF( alpha );
  Qt3DRender::QParameter *colorParam = new Qt3DRender::QParameter( u"color"_s, color );
  pass->addParameter( colorParam );

  technique->addRenderPass( pass );
  effect->addTechnique( technique );
  setEffect( effect );
}

///@endcond PRIVATE
