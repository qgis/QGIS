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

#include "qgsmesh3dmaterial_p.h"

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

#include <QUrl>
#include <QVector3D>
#include <QVector4D>
#include <Qt3DRender/QBuffer>
#include <QByteArray>

class ColorRampTextureGenerator: public Qt3DRender::QTextureImageDataGenerator
{

  public:
    ColorRampTextureGenerator( const QgsColorRampShader &colorRampShader ): mColorRampShader( colorRampShader )
    {

    }

    // QTextureImageDataGenerator interface
  public:
    Qt3DRender::QTextureImageDataPtr operator()() override
    {
      Qt3DRender::QTextureImageDataPtr dataPtr = Qt3DRender::QTextureImageDataPtr::create();
      dataPtr->setFormat( QOpenGLTexture::RGBA32F );
      dataPtr->setTarget( QOpenGLTexture::Target1D );
      dataPtr->setPixelFormat( QOpenGLTexture::RGBA );
      dataPtr->setPixelType( QOpenGLTexture::Float32 );

      QByteArray data;
      QList<QgsColorRampShader::ColorRampItem> colorItemList = mColorRampShader.colorRampItemList();
      int size = colorItemList.count() ;

      //dataPtr->setWidth( colorItemList.count() );
      dataPtr->setWidth( size );
      dataPtr->setHeight( 1 );
      dataPtr->setDepth( 1 );
      dataPtr->setFaces( 1 );
      dataPtr->setLayers( 1 );
      dataPtr->setMipLevels( 1 );

      for ( int i = 0; i < colorItemList.count(); ++i )
      {
        float mag = float( colorItemList.at( i ).value );

        QColor color = colorItemList.at( i ).color;
        float rf = float( color.redF() );
        float gf = float( color.greenF() );
        float bf = float( color.blueF() );

        data.append( reinterpret_cast<const char *>( &mag ), sizeof( float ) );
        data.append( reinterpret_cast<const char *>( &rf ), sizeof( float ) );
        data.append( reinterpret_cast<const char *>( &gf ), sizeof( float ) );
        data.append( reinterpret_cast<const char *>( &bf ), sizeof( float ) );

      }

      dataPtr->setData( data, sizeof( float ) ); //size is the size of the type, here float

      return dataPtr;
    }


    bool operator ==( const Qt3DRender::QTextureImageDataGenerator &other ) const override
    {
      const ColorRampTextureGenerator *otherFunctor = functor_cast<ColorRampTextureGenerator>( &other );
      if ( !otherFunctor )
        return false;

      QgsColorRampShader otherColorRampShader = otherFunctor->mColorRampShader;

      if ( mColorRampShader.colorRampItemList().count() != otherColorRampShader.colorRampItemList().count() ||
           mColorRampShader.classificationMode() != otherColorRampShader.classificationMode() ||
           mColorRampShader.colorRampType() != otherColorRampShader.colorRampType() )
      {
        return false;
      }

      QList<QgsColorRampShader::ColorRampItem> colorItemList = mColorRampShader.colorRampItemList();
      QList<QgsColorRampShader::ColorRampItem> otherColorItemList = otherColorRampShader.colorRampItemList();
      for ( int i = 0; i < colorItemList.count(); ++i )
      {
        if ( colorItemList.at( i ).color != otherColorItemList.at( i ).color ||
             colorItemList.at( i ).value != otherColorItemList.at( i ).value )
          return false;
      }

      return true;
    }

    QT3D_FUNCTOR( ColorRampTextureGenerator )

  private:
    QgsColorRampShader mColorRampShader;

};

class ColorRampTexture: public Qt3DRender::QAbstractTextureImage
{
  public:
    ColorRampTexture( const QgsColorRampShader &colorRampShader, Qt3DCore::QNode *parent = nullptr ):
      Qt3DRender::QAbstractTextureImage( parent ),
      mColorRampShader( colorRampShader )
    {

    }
    // QAbstractTextureImage interface
  protected:
    Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override
    {
      return Qt3DRender::QTextureImageDataGeneratorPtr( new ColorRampTextureGenerator( mColorRampShader ) );
    }

  private:
    QgsColorRampShader mColorRampShader;
};


QgsMesh3dMaterial::QgsMesh3dMaterial( const QgsMesh3DSymbol &symbol ):
  mSymbol( symbol )
{
  Qt3DRender::QEffect *eff = new Qt3DRender::QEffect( this );

  configure();
  eff->addTechnique( mTechnique );
  setEffect( eff );
}

void QgsMesh3dMaterial::configure()
{
  Qt3DRender::QTexture1D *colorRampTexture = new Qt3DRender::QTexture1D;
  colorRampTexture->addTextureImage( new ColorRampTexture( mSymbol.colorRampShader() ) );
  colorRampTexture->setMinificationFilter( Qt3DRender::QTexture1D::Linear );
  colorRampTexture->setMagnificationFilter( Qt3DRender::QTexture1D::Linear );

  // Create and configure wireframe technique
  mTechnique = new Qt3DRender::QTechnique();
  mTechnique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mTechnique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  mTechnique->graphicsApiFilter()->setMajorVersion( 3 );
  mTechnique->graphicsApiFilter()->setMinorVersion( 3 );
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
  mTechnique->addParameter( new Qt3DRender::QParameter( "flatTriangles", ( !mSymbol.smoothedTriangles() ) ) );
  QColor wireframecolor = mSymbol.wireframeLineColor();
  mTechnique->addParameter( new Qt3DRender::QParameter( "lineWidth", float( mSymbol.wireframeLineWidth() ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "lineColor", QVector4D( wireframecolor.redF(), wireframecolor.greenF(), wireframecolor.blueF(), 1.0f ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "wireframeEnabled", mSymbol.wireframeEnabled() ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "textureType", int( mSymbol.renderingStyle() ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampTexture", colorRampTexture ) ) ;
  mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampCount", mSymbol.colorRampShader().colorRampItemList().count() ) );
  int colorRampType = mSymbol.colorRampShader().colorRampType();
  mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampType", colorRampType ) );
  QColor meshColor = mSymbol.singleMeshColor();
  mTechnique->addParameter( new Qt3DRender::QParameter( "meshColor", QVector4D( meshColor.redF(), meshColor.greenF(), meshColor.blueF(), 1.0f ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "verticaleScale", float( mSymbol.verticaleScale() ) ) );
}

