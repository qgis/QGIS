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
#include <QUrl>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

#include "moc_qgshighlightmaterial.cpp"

QgsHighlightMaterial::QgsHighlightMaterial( QNode *parent )
  : QgsMaterial( parent )
{
  init();
}

QgsHighlightMaterial::~QgsHighlightMaterial() = default;

void QgsHighlightMaterial::init()
{
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );

  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( QStringLiteral( "forward" ) );
  technique->addFilterKey( filterKey );

  Qt3DRender::QRenderPass *pass = new Qt3DRender::QRenderPass;

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram;
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/default.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/singlecolor.frag" ) ) ) );
  pass->setShaderProgram( shaderProgram );

  const QgsSettings settings;
  const QColor color = QColor( settings.value( QStringLiteral( "Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  Qt3DRender::QParameter *colorParam = new Qt3DRender::QParameter( QStringLiteral( "color" ), color );
  pass->addParameter( colorParam );
  const float alpha = settings.value( QStringLiteral( "Map/highlight/colorAlpha" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toFloat() / 255.f;
  Qt3DRender::QParameter *opacityParam = new Qt3DRender::QParameter( QStringLiteral( "opacity" ), alpha );
  pass->addParameter( opacityParam );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  pass->addRenderState( depthTest );

  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  pass->addRenderState( cullFace );

  technique->addRenderPass( pass );
  effect->addTechnique( technique );
  setEffect( effect );
}
