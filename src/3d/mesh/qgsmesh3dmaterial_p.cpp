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
#include <Qt3DRender/QBuffer>
#include <QByteArray>

#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgstriangularmesh.h"

class ColorRampTextureGenerator: public Qt3DRender::QTextureImageDataGenerator
{

  public:
    ColorRampTextureGenerator( const QgsColorRampShader &colorRampShader, double verticalScale = 1 ):
      mColorRampShader( colorRampShader ),
      mVerticalScale( verticalScale )
    {}

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

      dataPtr->setWidth( size );
      dataPtr->setHeight( 1 );
      dataPtr->setDepth( 1 );
      dataPtr->setFaces( 1 );
      dataPtr->setLayers( 1 );
      dataPtr->setMipLevels( 1 );

      for ( int i = 0; i < colorItemList.count(); ++i )
      {
        float mag = float( colorItemList.at( i ).value * mVerticalScale );

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
        const QColor color = colorItemList.at( i ).color;
        const QColor otherColor = otherColorItemList.at( i ).color;
        double value = colorItemList.at( i ).value;
        double otherValue = otherColorItemList.at( i ).value;
        if ( color != otherColor ||
             ( !std::isnan( value ) && !std::isnan( otherValue ) && colorItemList.at( i ).value != otherColorItemList.at( i ).value ) ||
             ( std::isnan( value ) != std::isnan( otherValue ) ) )
          return false;
      }

      return true;
    }

    QT3D_FUNCTOR( ColorRampTextureGenerator )

  private:
    QgsColorRampShader mColorRampShader;
    double mVerticalScale = 1;
};


class ColorRampTexture: public Qt3DRender::QAbstractTextureImage
{
  public:
    ColorRampTexture( const QgsColorRampShader &colorRampShader, double verticalScale = 1, Qt3DCore::QNode *parent = nullptr ):
      Qt3DRender::QAbstractTextureImage( parent ),
      mColorRampShader( colorRampShader ),
      mVerticalScale( verticalScale )
    {

    }
    // QAbstractTextureImage interface
  protected:
    Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override
    {
      return Qt3DRender::QTextureImageDataGeneratorPtr( new ColorRampTextureGenerator( mColorRampShader, mVerticalScale ) );
    }

  private:
    QgsColorRampShader mColorRampShader;
    double mVerticalScale = 1;
};


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

    QT3D_FUNCTOR( ArrowsTextureGenerator )
};


class ArrowsGridTexture: public Qt3DRender::QAbstractTextureImage
{
  public:
    ArrowsGridTexture( const QVector<QgsVector> &vectors, const QSize &size, bool fixedSize, double maxVectorLength ):
      mVectors( vectors ), mSize( size ), mFixedSize( fixedSize ), mMaxVectorLength( maxVectorLength )
    {}

  protected:
    Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override
    {
      return Qt3DRender::QTextureImageDataGeneratorPtr( new ArrowsTextureGenerator( mVectors, mSize, mFixedSize, mMaxVectorLength ) );
    }

  private:
    const QVector<QgsVector> mVectors;
    const QSize mSize;
    const bool mFixedSize;
    const double mMaxVectorLength;
};


QgsMesh3dMaterial::QgsMesh3dMaterial( QgsMeshLayer *layer,
                                      const QgsDateTimeRange &timeRange,
                                      const QgsVector3D &origin,
                                      const QgsMesh3DSymbol &symbol,
                                      MagnitudeType magnitudeType ):
  mSymbol( symbol ),
  mMagnitudeType( magnitudeType ),
  mOrigin( origin )
{
  Qt3DRender::QEffect *eff = new Qt3DRender::QEffect( this );

  configure();
  configureArrows( layer, timeRange );

  eff->addTechnique( mTechnique );
  setEffect( eff );
}

void QgsMesh3dMaterial::configure()
{
  // Create the texture to pass the color ramp
  Qt3DRender::QTexture1D *colorRampTexture = nullptr;
  if ( mSymbol.colorRampShader().colorRampItemList().count() > 0 )
  {
    colorRampTexture = new Qt3DRender::QTexture1D( this );
    switch ( mMagnitudeType )
    {
      case QgsMesh3dMaterial::ZValue:
        // if the color shading is done with the Z value of vertices, the color ramp has to be adapted with vertical scale
        colorRampTexture->addTextureImage( new ColorRampTexture( mSymbol.colorRampShader(), mSymbol.verticalScale() ) );
        break;
      case QgsMesh3dMaterial::ScalarDataSet:
        // if the color shading is done with scalar dataset, no vertical scale to use
        colorRampTexture->addTextureImage( new ColorRampTexture( mSymbol.colorRampShader(), 1 ) );
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
  QUrl urlVert( QStringLiteral( "qrc:/shaders/mesh/mesh.vert" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, shaderProgram->loadSource( urlVert ) );
  QUrl urlGeom( QStringLiteral( "qrc:/shaders/mesh/mesh.geom" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Geometry, shaderProgram->loadSource( urlGeom ) );
  QUrl urlFrag( QStringLiteral( "qrc:/shaders/mesh/mesh.frag" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, shaderProgram->loadSource( urlFrag ) );

  renderPass->setShaderProgram( shaderProgram );
  mTechnique->addRenderPass( renderPass );

  // Parameters
  mTechnique->addParameter( new Qt3DRender::QParameter( "flatTriangles", ( !mSymbol.smoothedTriangles() ) ) );
  QColor wireframecolor = mSymbol.wireframeLineColor();
  mTechnique->addParameter( new Qt3DRender::QParameter( "lineWidth", float( mSymbol.wireframeLineWidth() ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "lineColor", QVector4D( wireframecolor.redF(), wireframecolor.greenF(), wireframecolor.blueF(), 1.0f ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "wireframeEnabled", mSymbol.wireframeEnabled() ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "textureType", int( mSymbol.renderingStyle() ) ) );
  if ( colorRampTexture )
    mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampTexture", colorRampTexture ) ) ;
  mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampCount", mSymbol.colorRampShader().colorRampItemList().count() ) );
  int colorRampType = mSymbol.colorRampShader().colorRampType();
  mTechnique->addParameter( new Qt3DRender::QParameter( "colorRampType", colorRampType ) );
  QColor meshColor = mSymbol.singleMeshColor();
  mTechnique->addParameter( new Qt3DRender::QParameter( "meshColor", QVector4D( meshColor.redF(), meshColor.greenF(), meshColor.blueF(), 1.0f ) ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "isScalarMagnitude", ( mMagnitudeType == QgsMesh3dMaterial::ScalarDataSet ) ) );
}

void QgsMesh3dMaterial::configureArrows( QgsMeshLayer *layer, const QgsDateTimeRange &timeRange )
{
  if ( !layer || !layer->dataProvider() )
    return;

  QgsMeshDatasetIndex datasetIndex = layer->activeVectorDatasetAtTime( timeRange );

  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowsSpacing", float( mSymbol.arrowsSpacing() ) ) ) ;
  QColor arrowsColor = layer->rendererSettings().vectorSettings( datasetIndex.group() ).color();
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowsColor", QVector4D( arrowsColor.redF(), arrowsColor.greenF(), arrowsColor.blueF(), 1.0f ) ) ) ;

  QgsMeshDatasetGroupMetadata meta = layer->dataProvider()->datasetGroupMetadata( datasetIndex );

  QVector<QgsVector> vectors;
  QSize gridSize;
  QgsPointXY minCorner;
  std::unique_ptr< Qt3DRender::QParameter > arrowsEnabledParameter = qgis::make_unique< Qt3DRender::QParameter >( "arrowsEnabled", nullptr );
  if ( mMagnitudeType != MagnitudeType::ScalarDataSet || !mSymbol.arrowsEnabled() || meta.isScalar() || !datasetIndex.isValid() )
    arrowsEnabledParameter->setValue( false );
  else
  {
    arrowsEnabledParameter->setValue( true );
    int maxSize = mSymbol.maximumTextureSize();
    // construct grid
    QgsRectangle gridExtent = layer->triangularMesh()->extent();
    gridSize = QSize( maxSize, maxSize );
    double xSpacing = mSymbol.arrowsSpacing();
    double ySpacing = mSymbol.arrowsSpacing();
    // check the size of the grid and adjust the spacing if needed
    int desiredXSize = int( gridExtent.width() / xSpacing );
    if ( desiredXSize > maxSize )
      xSpacing = gridExtent.width() / maxSize;
    else
      gridSize.setWidth( desiredXSize );

    int desiredYSize = int( gridExtent.height() / ySpacing );
    if ( desiredYSize > maxSize )
      ySpacing = gridExtent.height() / maxSize;
    else
      gridSize.setHeight( desiredYSize );

    double xMin = gridExtent.xMinimum() + xSpacing / 2;
    double yMin = gridExtent.yMinimum() + ySpacing / 2;
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
  arrowsGridTexture->addTextureImage( new ArrowsGridTexture( vectors, gridSize, mSymbol.arrowsFixedSize(), meta.maximum() ) );
  arrowsGridTexture->setMinificationFilter( Qt3DRender::QTexture2D::Nearest );
  arrowsGridTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Nearest );

  Qt3DRender::QTexture2D *arrowTexture = new Qt3DRender::QTexture2D( this );
  Qt3DRender::QTextureImage *arrowTextureImage = new Qt3DRender::QTextureImage();
  arrowTextureImage->setSource( QStringLiteral( "qrc:/textures/arrow.png" ) );
  arrowTexture->addTextureImage( arrowTextureImage );
  arrowTexture->setMinificationFilter( Qt3DRender::QTexture2D::Nearest );
  arrowTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Nearest );
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowTexture", arrowTexture ) );
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowsGridTexture", arrowsGridTexture ) ) ;
  mTechnique->addParameter( new Qt3DRender::QParameter( "arrowsMinCorner", QVector2D( minCorner.x() - mOrigin.x(), -minCorner.y() + mOrigin.y() ) ) ) ;
}
