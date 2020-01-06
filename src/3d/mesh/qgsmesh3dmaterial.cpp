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

QgsMesh3dMaterial::QgsMesh3dMaterial( const QgsMesh3DSymbol &symbol ):
  mSymbol( symbol )
{
  Qt3DRender::QEffect *eff = new Qt3DRender::QEffect( this );

  configureColorTechnique();
  eff->addTechnique( mColorTechnique );

  configureLightsTechnique();
  eff->addTechnique( mLightTechnique );

  configureWireframeTechnique();
  eff->addTechnique( mWireFrameTechnique );

  setEffect( eff );

}

void QgsMesh3dMaterial::configureLightsTechnique()
{
  // Create and configure light technique
  mLightTechnique = new Qt3DRender::QTechnique();
  mLightTechnique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mLightTechnique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  mLightTechnique->graphicsApiFilter()->setMajorVersion( 3 );
  mLightTechnique->graphicsApiFilter()->setMinorVersion( 1 );
  Qt3DRender::QFilterKey *filterLightKey = new Qt3DRender::QFilterKey();
  filterLightKey->setName( QStringLiteral( "mesh" ) );
  filterLightKey->setValue( QStringLiteral( "renderLight" ) );
  mLightTechnique->addFilterKey( filterLightKey );

  Qt3DRender::QRenderPass *lightRenderPasses = new Qt3DRender::QRenderPass();
  Qt3DRender::QShaderProgram *lightShaderProgram = new Qt3DRender::QShaderProgram();
  //Load shader programs
  QUrl urlLightVert( QStringLiteral( "qrc:/shaders/mesh/mesh.vert" ) );
  lightShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, lightShaderProgram->loadSource( urlLightVert ) );
  QUrl urlLightGeom( QStringLiteral( "qrc:/shaders/mesh/mesh_lights.geom" ) );
  lightShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Geometry, lightShaderProgram->loadSource( urlLightGeom ) );
  QUrl urlLightFrag( QStringLiteral( "qrc:/shaders/mesh/mesh_lights.frag" ) );
  lightShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, lightShaderProgram->loadSource( urlLightFrag ) );

  lightRenderPasses->setShaderProgram( lightShaderProgram );
  mLightTechnique->addRenderPass( lightRenderPasses );

  // Parameter
  Qt3DRender::QParameter *flatTrianglesParameter = new Qt3DRender::QParameter( "flatTriangles", !mSymbol.smoothedTriangles() );
  Qt3DRender::QParameter *ambiantFactorParameter = new Qt3DRender::QParameter( "ambianceFactor", float( 0.15 ) );
  mLightTechnique->addParameter( flatTrianglesParameter );
  mLightTechnique->addParameter( ambiantFactorParameter );
}

void QgsMesh3dMaterial::configureColorTechnique()
{
  // Create and configure color technique
  mColorTechnique = new Qt3DRender::QTechnique();
  mColorTechnique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mColorTechnique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  mColorTechnique->graphicsApiFilter()->setMajorVersion( 3 );
  mColorTechnique->graphicsApiFilter()->setMinorVersion( 1 );
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey();
  filterKey->setName( QStringLiteral( "mesh" ) );
  filterKey->setValue( QStringLiteral( "renderColor" ) );
  mColorTechnique->addFilterKey( filterKey );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass();
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram( this );
  //Load shader programs
  QUrl urlVert( QStringLiteral( "qrc:/shaders/mesh/mesh.vert" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, shaderProgram->loadSource( urlVert ) );

  switch ( mSymbol.meshTextureType() )
  {
    case QgsMesh3DSymbol::uniqueColor:
    {
      QUrl urlFrag( QStringLiteral( "qrc:/shaders/mesh/mesh_colorUnique_uniform.frag" ) );
      shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, shaderProgram->loadSource( urlFrag ) );
      QColor color = mSymbol.uniqueMeshColor();
      mColorTechnique->addParameter(
        new Qt3DRender::QParameter( "uniqueColor", QVector3D( color.redF(), color.greenF(), color.blueF() ) ) );
    }
    break;
    case QgsMesh3DSymbol::colorRamp:
    {
      QgsColorRampShader colorRampShader = mSymbol.colorRampShader();

      QByteArray fragCode;
      QString strCode( "#version 330 core\n#define COLOR_RAMP_SIZE " );
      int colorCount = colorRampShader.colorRampItemList().count();
      if ( colorCount == 0 )
        colorCount = 1;
      strCode.append( QString::number( colorCount ) );
      strCode.append( "\n" );
      fragCode.append( strCode.toStdString().c_str() );

      QUrl urlFrag( QStringLiteral( "qrc:/shaders/mesh/mesh_colorRamp_uniform.frag" ) );
      fragCode.append( shaderProgram->loadSource( urlFrag ) );
      shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, fragCode );
      setColorRampParameterUniform( colorRampShader );
    }
    break;
  }

  renderPass->setShaderProgram( shaderProgram );
  mColorTechnique->addRenderPass( renderPass );

}

void QgsMesh3dMaterial::configureWireframeTechnique()
{
  // Create and configure wireframe technique
  mWireFrameTechnique = new Qt3DRender::QTechnique();
  mWireFrameTechnique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mWireFrameTechnique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  mWireFrameTechnique->graphicsApiFilter()->setMajorVersion( 3 );
  mWireFrameTechnique->graphicsApiFilter()->setMinorVersion( 1 );
  Qt3DRender::QFilterKey *filterWireframeKey = new Qt3DRender::QFilterKey();
  filterWireframeKey->setName( QStringLiteral( "mesh" ) );
  filterWireframeKey->setValue( QStringLiteral( "renderWireframe" ) );
  mWireFrameTechnique->addFilterKey( filterWireframeKey );

  Qt3DRender::QRenderPass *wireframeRenderPass = new Qt3DRender::QRenderPass();
  Qt3DRender::QShaderProgram *wireFrameShaderProgram = new Qt3DRender::QShaderProgram();

  if ( mSymbol.wireframeEnabled() )
  {
    //Load shader programs
    QUrl urlLightVert( QStringLiteral( "qrc:/shaders/mesh/mesh_wireframe.vert" ) );
    wireFrameShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, wireFrameShaderProgram->loadSource( urlLightVert ) );
    QUrl urlLightGeom( QStringLiteral( "qrc:/shaders/mesh/mesh_wireframe.geom" ) );
    wireFrameShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Geometry, wireFrameShaderProgram->loadSource( urlLightGeom ) );
    QUrl urlLightFrag( QStringLiteral( "qrc:/shaders/mesh/mesh_wireframe.frag" ) );
    wireFrameShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, wireFrameShaderProgram->loadSource( urlLightFrag ) );

    wireframeRenderPass->setShaderProgram( wireFrameShaderProgram );
    mWireFrameTechnique->addRenderPass( wireframeRenderPass );

    // Parameter
    Qt3DRender::QParameter *lineWidthParameter = new Qt3DRender::QParameter( "lineWidth", float( mSymbol.wireframeLineWidth() ) );
    QColor color = mSymbol.wireframeLineColor();
    Qt3DRender::QParameter *lineColorParameter = new Qt3DRender::QParameter( "lineColor", QVector4D( color.redF(), color.greenF(), color.blueF(), 1.0f ) );
    mWireFrameTechnique->addParameter( lineWidthParameter );
    mWireFrameTechnique->addParameter( lineColorParameter );
  }
  else
  {
    QUrl urlLightVert( QStringLiteral( "qrc:/shaders/mesh/mesh.vert" ) );
    wireFrameShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, wireFrameShaderProgram->loadSource( urlLightVert ) );
    QUrl urlLightFrag( QStringLiteral( "qrc:/shaders/mesh/mesh_wireframe_disabled.frag" ) );
    wireFrameShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, wireFrameShaderProgram->loadSource( urlLightFrag ) );

    wireframeRenderPass->setShaderProgram( wireFrameShaderProgram );
    mWireFrameTechnique->addRenderPass( wireframeRenderPass );
  }
}

void QgsMesh3dMaterial::setColorRampParameterUniform( const QgsColorRampShader &colorRampShader )
{

  QVariantList data;
  QVector<QColor> colors;
  QVector<float> values;

  QList< QgsColorRampShader::ColorRampItem > colorsValuesList = colorRampShader.colorRampItemList();

  if ( colorsValuesList.count() == 0 )
    data.append( QVector4D( 0.5, 0.5, 0.5, 0 ) );

  for ( int i = 0; i < colorsValuesList.count(); ++i )
  {
    QVector4D itemOut;
    const QgsColorRampShader::ColorRampItem &item = colorsValuesList.at( i );
    QColor color = item.color;
    float value = float( item.value );

    itemOut.setX( float( color.redF() ) );
    itemOut.setY( float( color.greenF() ) );
    itemOut.setZ( float( color.blueF() ) );
    itemOut.setW( value );

    data.append( itemOut );
  }
  mColorTechnique->addParameter( new Qt3DRender::QParameter( "colorRampValues[0]", data ) );

  int type = colorRampShader.colorRampType();
  mColorTechnique->addParameter( new Qt3DRender::QParameter( "colorRampType", type ) );
  mColorTechnique->addParameter( new Qt3DRender::QParameter( "verticaleScale", float( mSymbol.verticaleScale() ) ) );
}
