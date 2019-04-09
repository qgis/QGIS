/***************************************************************************
  qgslinematerial_p.cpp
  --------------------------------------
  Date                 : Apr 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslinematerial_p.h"

#include <QColor>
#include <QSizeF>
#include <QUrl>
#include <QVector3D>

#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>

/// @cond PRIVATE


QgsLineMaterial::QgsLineMaterial()
  : mParameterThickness( new Qt3DRender::QParameter( "THICKNESS", 10, this ) )
  , mParameterMiterLimit( new Qt3DRender::QParameter( "MITER_LIMIT", -1, this ) )  // 0.75
  , mParameterLineColor( new Qt3DRender::QParameter( "lineColor", QColor( 0, 255, 0 ), this ) )
  , mParameterWindowScale( new Qt3DRender::QParameter( "WIN_SCALE", QSizeF(), this ) )
{
  addParameter( mParameterThickness );
  addParameter( mParameterMiterLimit );
  addParameter( mParameterLineColor );
  addParameter( mParameterWindowScale );

  //Parameter { name: "tex0"; value: txt },
  //Parameter { name: "useTex"; value: false },

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram( this );
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/lines.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/lines.frag" ) ) ) );
  shaderProgram->setGeometryShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/lines.geom" ) ) ) );

  Qt3DRender::QBlendEquation *blendEquation = new Qt3DRender::QBlendEquation( this );
  blendEquation->setBlendFunction( Qt3DRender::QBlendEquation::Add );

  Qt3DRender::QBlendEquationArguments *blendEquationArgs = new Qt3DRender::QBlendEquationArguments( this );
  blendEquationArgs->setSourceRgb( Qt3DRender::QBlendEquationArguments::SourceAlpha );
  blendEquationArgs->setDestinationRgb( Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  renderPass->setShaderProgram( shaderProgram );
  renderPass->addRenderState( blendEquation );
  renderPass->addRenderState( blendEquationArgs );

  // without this filter the default forward renderer would not render this
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( "forward" );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->addFilterKey( filterKey );
  technique->addRenderPass( renderPass );
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 1 );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect( this );
  effect->addTechnique( technique );

  setEffect( effect );
}

void QgsLineMaterial::setLineColor( const QColor &color )
{
  mParameterLineColor->setValue( color );
}

QColor QgsLineMaterial::lineColor() const
{
  return mParameterLineColor->value().value<QColor>();
}

void QgsLineMaterial::setLineWidth( float width )
{
  mParameterThickness->setValue( width );
}

float QgsLineMaterial::lineWidth() const
{
  return mParameterThickness->value().toFloat();
}

void QgsLineMaterial::setViewportSize( const QSizeF &viewportSize )
{
  mParameterWindowScale->setValue( viewportSize );
}

/// @endcond
