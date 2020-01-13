/***************************************************************************
                         qgsmesh3dmaterial.cpp
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dmaterial.h"

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <QUrl>
#include <QVector3D>
#include <QVector4D>
#include <Qt3DRender/QBuffer>
#include <QByteArray>

QgsMesh3dMaterial::QgsMesh3dMaterial( MagnitudeType magnitudeType, const QgsMesh3DSymbol &symbol ):
  mMagnitudeType( magnitudeType ),
  mSymbol( symbol )
{
  Qt3DRender::QEffect *eff = new Qt3DRender::QEffect( this );

  configure();
  eff->addTechnique( mTechnique );
  setEffect( eff );
}

void QgsMesh3dMaterial::configure()
{
  // Create and configure wireframe technique
  mTechnique = new Qt3DRender::QTechnique();
  mTechnique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mTechnique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  mTechnique->graphicsApiFilter()->setMajorVersion( 3 );
  mTechnique->graphicsApiFilter()->setMinorVersion( 1 );
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey();
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( QStringLiteral( "forward" ) );
  mTechnique->addFilterKey( filterKey );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass();
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram();

  //Load shader programs
  QUrl urlVert( QStringLiteral( "qrc:/shaders/mesh/mesh.vert" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, shaderProgram->loadSource( urlVert ) );
  QUrl urlGeom( QStringLiteral( "qrc:/shaders/mesh/mesh.geom" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Geometry, shaderProgram->loadSource( urlGeom ) );
  QUrl urlFrag( QStringLiteral( "qrc:/shaders/mesh/mesh.frag" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, shaderProgram->loadSource( urlFrag ) );

  renderPass->setShaderProgram( shaderProgram );
  mTechnique->addRenderPass( renderPass );

  // Parameter
  Qt3DRender::QParameter *lineWidthParameter = new Qt3DRender::QParameter( "lineWidth", float( mSymbol.wireframeLineWidth() ) );
  QColor color = mSymbol.wireframeLineColor();
  Qt3DRender::QParameter *lineColorParameter = new Qt3DRender::QParameter( "lineColor", QVector4D( color.redF(), color.greenF(), color.blueF(), 1.0f ) );
  mTechnique->addParameter( lineWidthParameter );
  mTechnique->addParameter( lineColorParameter );
}

