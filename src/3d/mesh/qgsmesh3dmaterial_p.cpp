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
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QBuffer>
#else
#include <Qt3DCore/QBuffer>
#endif

#include <QByteArray>

#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgstriangularmesh.h"

#include "qgscolorramptexture.h"

class ArrowsTextureGenerator: public Qt3DRender::QTextureImageDataGenerator
{
  public:
    ArrowsTextureGenerator( const QVector<QgsVector> &vectors, const QSize &size, bool fixedSize, double maxVectorLength ):
      mVectors( vectors ), mSize( size ), mFixedSize( fixedSize ), mMaxVectorLength( maxVectorLength )
    {}

    Qt3DRender::QTextureImageDataPtr operator()() override
    {
      Qt3DRender::QTextureImageDataPtr dataPtr = Qt3DRender::QTextureImageDataPtr::create();
      dataPtr->setFormat( QOpenGLTexture::RG32F );
      dataPtr->setTarget( QOpenGLTexture::Target2D );
      dataPtr->setPixelFormat( QOpenGLTexture::RG );
      dataPtr->setPixelType( QOpenGLTexture::Float32 );

      QByteArray data;

      dataPtr->setWidth( mSize.width() );
      dataPtr->setHeight( mSize.height() );
      dataPtr->setDepth( 1 );
      dataPtr->setFaces( 1 );
      dataPtr->setLayers( 1 );
      dataPtr->setMipLevels( 1 );

      if ( mSize.isValid() )
      {
        data.resize( 2 * mSize.width()*mSize.height()*sizeof( float ) );
        float *fptr = reinterpret_cast<float *>( data.data() );
        for ( int i = 0; i < mSize.width()*mSize.height(); ++i )
        {
          if ( mFixedSize )
            *fptr++ = 1;
          else
            *fptr++ = mVectors.at( i ).length() / mMaxVectorLength;

          *fptr++ = mVectors.at( i ).angle();
        }
      }

      dataPtr->setData( data, sizeof( float ) ); //size is the size of the type, here float
      return dataPtr;
    }

    bool operator ==( const Qt3DRender::QTextureImageDataGenerator &other ) const override
    {
      const ArrowsTextureGenerator *otherFunctor = functor_cast<ArrowsTextureGenerator>( &other );
      if ( !otherFunctor )
        return false;

      return ( otherFunctor->mVectors == mVectors &&
               otherFunctor->mSize == mSize &&
               otherFunctor->mFixedSize == mFixedSize );
    }

  private:
    const QVector<QgsVector> mVectors;
    const QSize mSize;
    const bool mFixedSize;
    const double mMaxVectorLength;

    // marked as deprecated in 5.15, but undeprecated for Qt 6.0. TODO -- remove when we require 6.0
    Q_NOWARN_DEPRECATED_PUSH
    QT3D_FUNCTOR( ArrowsTextureGenerator )
    Q_NOWARN_DEPRECATED_POP
};



QgsMesh3dMaterial::QgsMesh3dMaterial( QgsMeshLayer *layer,
                                      const QgsDateTimeRange &timeRange,
                                      const QgsVector3D &origin,
                                      const QgsMesh3DSymbol *symbol,
                                      MagnitudeType magnitudeType )
  : mSymbol( symbol->clone() )
  , mMagnitudeType( magnitudeType )
  , mOrigin( origin )
{
  Qt3DRender::QEffect *eff = new Qt3DRender::QEffect( this );

  configure();

  // this method has to be called even if there isn't arrows (terrain) because it configures the parameter of shaders
  // If all the parameters ("uniform" in shaders) are not defined in QGIS, the shaders sometimes don't work (depends on hardware?)
  configureArrows( layer, timeRange );

  eff->addTechnique( mTechnique );
  setEffect( eff );
}

void QgsMesh3dMaterial::configure()
{
  // Create the texture to pass the color ramp
  Qt3DRender::QTexture1D *colorRampTexture = nullptr;
  if ( mSymbol->colorRampShader().colorRampItemList().count() > 0 )
  {
    colorRampTexture = new Qt3DRender::QTexture1D( this );
    switch ( mMagnitudeType )
    {
      case QgsMesh3dMaterial::ZValue:
        // if the color shading is done with the Z value of vertices, the color ramp has to be adapted with vertical scale
        colorRampTexture->addTextureImage( new QgsColorRampTexture( mSymbol->colorRampShader(), mSymbol->verticalScale() ) );
        break;
      case QgsMesh3dMaterial::ScalarDataSet:
        // if the color shading is done with scalar dataset, no vertical scale to use
        colorRampTexture->addTextureImage( new QgsColorRampTexture( mSymbol->colorRampShader(), 1 ) );
        break;
    }

    colorRampTexture->setMinificationFilter( Qt3DRender::QTexture1D::Linear );
    colorRampTexture->setMagnificationFilter( Qt3DRender::QTexture1D::Linear );
  }

  // Create and configure technique
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
  const QUrl urlVert( QStringLiteral( "qrc:/shaders/mesh/mesh.vert" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( urlVert ) );
  const QUrl urlGeom( QStringLiteral( "qrc:/shaders/mesh/mesh.geom" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Geometry, Qt3DRender::QShaderProgram::loadSource( urlGeom ) );
  const QUrl urlFrag( QStringLiteral( "qrc:/shaders/mesh/mesh.frag" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, Qt3DRender::QShaderProgram::loadSource( urlFrag ) );

  renderPass->setShaderProgram( shaderProgram );
  mTechnique->addRenderPass( renderPass );

  // Parameters
  mTechnique->addParameter( new Qt3DRender::QParameter( "flatTriangles", ( !mSymbol->smoothedTriangles() ) ) );
  const QColor wireframecolor = mSymbol->wireframeLineColor();
  mTechnique->addParameter( new Qt3DRender::QParameter( "lineWidth", float( mSymbol->wireframeLineWidth() ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "lineColor", QVector4D( wireframecolor.redF(), wireframecolor.greenF(), wireframecolor.blueF(), 1.0f ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "wireframeEnabled", mSymbol->wireframeEnabled() ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "textureType", int( mSymbol->renderingStyle() ) ) );
  if ( colorRampTexture )
    mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampTexture", colorRampTexture ) ) ;
  mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampCount", mSymbol->colorRampShader().colorRampItemList().count() ) );
  const int colorRampType = mSymbol->colorRampShader().colorRampType();
  mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampType", colorRampType ) );
  const QColor meshColor = mSymbol->singleMeshColor();
  mTechnique->addParameter( new Qt3DRender::QParameter( "meshColor", QVector4D( meshColor.redF(), meshColor.greenF(), meshColor.blueF(), 1.0f ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "isScalarMagnitude", ( mMagnitudeType == QgsMesh3dMaterial::ScalarDataSet ) ) );
}

void QgsMesh3dMaterial::configureArrows( QgsMeshLayer *layer, const QgsDateTimeRange &timeRange )
{
  QgsMeshDatasetIndex datasetIndex;
  QColor arrowsColor;
  QgsMeshDatasetGroupMetadata meta;

  if ( layer )
    datasetIndex = layer->activeVectorDatasetAtTime( timeRange );

  QVector<QgsVector> vectors;
  QSize gridSize;
  QgsPointXY minCorner;
  std::unique_ptr< Qt3DRender::QParameter > arrowsEnabledParameter = std::make_unique< Qt3DRender::QParameter >( "arrowsEnabled", nullptr );
  if ( !layer || mMagnitudeType != MagnitudeType::ScalarDataSet || !mSymbol->arrowsEnabled() || meta.isScalar() || !datasetIndex.isValid() )
    arrowsEnabledParameter->setValue( false );
  else
  {
    meta = layer->datasetGroupMetadata( datasetIndex );
    arrowsColor = layer->rendererSettings().vectorSettings( datasetIndex.group() ).color();
    arrowsEnabledParameter->setValue( true );
    const int maxSize = mSymbol->maximumTextureSize();
    // construct grid
    const QgsRectangle gridExtent = layer->triangularMesh()->extent();
    gridSize = QSize( maxSize, maxSize );
    double xSpacing = mSymbol->arrowsSpacing();
    double ySpacing = mSymbol->arrowsSpacing();
    // check the size of the grid and adjust the spacing if needed
    const int desiredXSize = int( gridExtent.width() / xSpacing );
    if ( desiredXSize > maxSize )
      xSpacing = gridExtent.width() / maxSize;
    else
      gridSize.setWidth( desiredXSize );

    const int desiredYSize = int( gridExtent.height() / ySpacing );
    if ( desiredYSize > maxSize )
      ySpacing = gridExtent.height() / maxSize;
    else
      gridSize.setHeight( desiredYSize );

    const double xMin = gridExtent.xMinimum() + xSpacing / 2;
    const double yMin = gridExtent.yMinimum() + ySpacing / 2;
    minCorner = QgsPointXY( xMin, yMin );

    vectors = QgsMeshLayerUtils::griddedVectorValues(
                layer,
                datasetIndex,
                xSpacing,
                ySpacing,
                gridSize,
                minCorner );

    if ( vectors.isEmpty() )
      return;
  }

  mTechnique->addParameter( arrowsEnabledParameter.release() )  ;

  Qt3DRender::QTexture2D *arrowsGridTexture = new Qt3DRender::QTexture2D( this );
  arrowsGridTexture->addTextureImage( new ArrowsGridTexture( vectors, gridSize, mSymbol->arrowsFixedSize(), meta.maximum() ) );
  arrowsGridTexture->setMinificationFilter( Qt3DRender::QTexture2D::Nearest );
  arrowsGridTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Nearest );

  Qt3DRender::QTexture2D *arrowTexture = new Qt3DRender::QTexture2D( this );
  Qt3DRender::QTextureImage *arrowTextureImage = new Qt3DRender::QTextureImage();
  arrowTextureImage->setSource( QStringLiteral( "qrc:/textures/arrow.png" ) );
  arrowTexture->addTextureImage( arrowTextureImage );
  arrowTexture->setMinificationFilter( Qt3DRender::QTexture2D::Nearest );
  arrowTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Nearest );
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowsColor", QVector4D( arrowsColor.redF(), arrowsColor.greenF(), arrowsColor.blueF(), 1.0f ) ) ) ;
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowsSpacing", float( mSymbol->arrowsSpacing() ) ) ) ;
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowTexture", arrowTexture ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowsGridTexture", arrowsGridTexture ) ) ;
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowsMinCorner", QVector2D( minCorner.x() - mOrigin.x(), -minCorner.y() + mOrigin.y() ) ) ) ;
}

ArrowsGridTexture::ArrowsGridTexture( const QVector<QgsVector> &vectors, const QSize &size, bool fixedSize, double maxVectorLength )
  : mVectors( vectors )
  , mSize( size )
  , mFixedSize( fixedSize )
  , mMaxVectorLength( maxVectorLength )
{}

Qt3DRender::QTextureImageDataGeneratorPtr ArrowsGridTexture::dataGenerator() const
{
  return Qt3DRender::QTextureImageDataGeneratorPtr( new ArrowsTextureGenerator( mVectors, mSize, mFixedSize, mMaxVectorLength ) );
}
