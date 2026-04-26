/***************************************************************************
  qgsgradientbackgroundentity.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgradientbackgroundentity.h"

#include <QString>
#include <QUrl>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>

#include "moc_qgsgradientbackgroundentity.cpp"

using namespace Qt::StringLiterals;

QgsGradientBackgroundEntity::QgsGradientBackgroundEntity( const QColor &topColor, const QColor &bottomColor, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mEffect( new Qt3DRender::QEffect( this ) )
  , mMaterial( new Qt3DRender::QMaterial( this ) )
  , mGl3Technique( new Qt3DRender::QTechnique( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
  , mGl3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMesh( new Qt3DExtras::QCuboidMesh( this ) )
  , mGlShader( new Qt3DRender::QShaderProgram( this ) )
  , mTopColorParameter( new Qt3DRender::QParameter( u"topColor"_s, QVector4D( topColor.redF(), topColor.greenF(), topColor.blueF(), topColor.alphaF() ) ) )
  , mBottomColorParameter( new Qt3DRender::QParameter( u"bottomColor"_s, QVector4D( bottomColor.redF(), bottomColor.greenF(), bottomColor.blueF(), bottomColor.alphaF() ) ) )
{
  mGl3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mGl3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setMinorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( mEffect );
  mFilterKey->setName( u"renderingStyle"_s );
  mFilterKey->setValue( u"forward"_s );

  mGl3Technique->addFilterKey( mFilterKey );

  Qt3DRender::QCullFace *cullFront = new Qt3DRender::QCullFace();
  cullFront->setMode( Qt3DRender::QCullFace::Front );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest();
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::LessOrEqual );
  Qt3DRender::QNoDepthMask *noDepthMask = new Qt3DRender::QNoDepthMask();

  mGl3RenderPass->addRenderState( cullFront );
  mGl3RenderPass->addRenderState( depthTest );
  mGl3RenderPass->addRenderState( noDepthMask );

  mGlShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/gradient_static.vert"_s ) ) );
  mGlShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/gradient_background.frag"_s ) ) );
  mGl3RenderPass->setShaderProgram( mGlShader );

  mGl3Technique->addRenderPass( mGl3RenderPass );
  mEffect->addTechnique( mGl3Technique );

  mMaterial->setEffect( mEffect );
  mMaterial->addParameter( mTopColorParameter );
  mMaterial->addParameter( mBottomColorParameter );

  mMesh->setXYMeshResolution( QSize( 2, 2 ) );
  mMesh->setXZMeshResolution( QSize( 2, 2 ) );
  mMesh->setYZMeshResolution( QSize( 2, 2 ) );

  addComponent( mMesh );
  addComponent( mMaterial );
}
