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

#include "qgs3dutils.h"

#include <QColor>
#include <QSizeF>
#include <QString>
#include <QUrl>
#include <QVector3D>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>

#include "moc_qgslinematerial_p.cpp"

using namespace Qt::StringLiterals;

/// @cond PRIVATE


QgsLineMaterial::QgsLineMaterial( LinePart part )
  : mPart( part )
  , mParameterThickness( new Qt3DRender::QParameter( "THICKNESS", 10, this ) )
  , mParameterLineColor( new Qt3DRender::QParameter( "lineColor", QVariant(), this ) )
  , mParameterUseVertexColors( new Qt3DRender::QParameter( "useVertexColors", false, this ) )
  , mParameterWindowScale( new Qt3DRender::QParameter( "WIN_SCALE", QSizeF(), this ) )
{
  addParameter( mParameterThickness );
  addParameter( mParameterLineColor );
  addParameter( mParameterUseVertexColors );
  addParameter( mParameterWindowScale );

  if ( mPart == LinePart::Join )
  {
    mParameterMiterLimit = new Qt3DRender::QParameter( "MITER_LIMIT", -1, this ); // previous implementation had this value and this always does a bevel, worth discussing/addressing in the future
    addParameter( mParameterMiterLimit );
  }

  setLineColor( QColor( 0, 255, 0 ) );

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram( this );
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( mPart == LinePart::Join ? u"qrc:/shaders/line_joins.vert"_s : u"qrc:/shaders/line_segments.vert"_s ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/lines.frag"_s ) ) );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  renderPass->setShaderProgram( shaderProgram );

  // Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest( renderPass );
  // depthTest->setDepthFunction( Qt3DRender::QDepthTest::LessOrEqual );
  // renderPass->addRenderState( depthTest );

  // Qt3DRender::QBlendEquationArguments *blendState = new Qt3DRender::QBlendEquationArguments;
  // blendState->setSourceRgb( Qt3DRender::QBlendEquationArguments::SourceAlpha );
  // blendState->setDestinationRgb( Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha );
  // renderPass->addRenderState( blendState );

  // Qt3DRender::QBlendEquation *blendEquation = new Qt3DRender::QBlendEquation;
  // blendEquation->setBlendFunction( Qt3DRender::QBlendEquation::Add );
  // renderPass->addRenderState( blendEquation );

  // without this filter the default forward renderer would not render this
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( u"renderingStyle"_s );
  filterKey->setValue( "forward" );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->addFilterKey( filterKey );
  technique->addRenderPass( renderPass );
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect( this );
  effect->addTechnique( technique );

  setEffect( effect );
}

void QgsLineMaterial::copyLineParametersTo( QgsLineMaterial *other ) const
{
  other->mParameterThickness->setValue( mParameterThickness->value() );
  other->mParameterLineColor->setValue( mParameterLineColor->value() );
  other->mParameterUseVertexColors->setValue( mParameterUseVertexColors->value() );
  other->mParameterWindowScale->setValue( mParameterWindowScale->value() );
}

void QgsLineMaterial::setLineColor( const QColor &color )
{
  mParameterLineColor->setValue( Qgs3DUtils::srgbToLinear( color ) );
}

void QgsLineMaterial::setUseVertexColors( bool enabled )
{
  mParameterUseVertexColors->setValue( enabled );
}

void QgsLineMaterial::setLineWidth( float width )
{
  mParameterThickness->setValue( width );
}

void QgsLineMaterial::setViewportSize( const QSizeF &viewportSize )
{
  mParameterWindowScale->setValue( viewportSize );
}

/// @endcond
