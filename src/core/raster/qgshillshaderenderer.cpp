/***************************************************************************
                         qgshillshaderenderer.cpp
                         ---------------------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QColor>

#include "qgshillshaderenderer.h"
#include "qgsrastertransparency.h"
#include "qgsrasterinterface.h"
#include "qgsrasterblock.h"
#include "qgsrectangle.h"
#include "qgsmessagelog.h"
#include <memory>

#ifdef HAVE_OPENCL
#ifdef QGISDEBUG
#include <chrono>
#include "qgssettings.h"
#endif
#include "qgsexception.h"
#include "qgsopenclutils.h"
#endif

QgsHillshadeRenderer::QgsHillshadeRenderer( QgsRasterInterface *input, int band, double lightAzimuth, double lightAngle ):
  QgsRasterRenderer( input, QStringLiteral( "hillshade" ) )
  , mBand( band )
  , mLightAngle( lightAngle )
  , mLightAzimuth( lightAzimuth )
{

}

QgsHillshadeRenderer *QgsHillshadeRenderer::clone() const
{
  QgsHillshadeRenderer *r = new QgsHillshadeRenderer( nullptr, mBand, mLightAzimuth, mLightAngle );
  r->copyCommonProperties( this );

  r->setZFactor( mZFactor );
  r->setMultiDirectional( mMultiDirectional );
  return r;
}

Qgis::RasterRendererFlags QgsHillshadeRenderer::flags() const
{
  return Qgis::RasterRendererFlag::InternalLayerOpacityHandling;
}

QgsRasterRenderer *QgsHillshadeRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  int band = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "0" ) ).toInt();
  double azimuth = elem.attribute( QStringLiteral( "azimuth" ), QStringLiteral( "315" ) ).toDouble();
  double angle = elem.attribute( QStringLiteral( "angle" ), QStringLiteral( "45" ) ).toDouble();
  double zFactor = elem.attribute( QStringLiteral( "zfactor" ), QStringLiteral( "1" ) ).toDouble();
  bool multiDirectional = elem.attribute( QStringLiteral( "multidirection" ), QStringLiteral( "0" ) ).toInt();
  QgsHillshadeRenderer *r = new QgsHillshadeRenderer( input, band, azimuth, angle );
  r->readXml( elem );

  r->setZFactor( zFactor );
  r->setMultiDirectional( multiDirectional );
  return r;
}

void QgsHillshadeRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "band" ), mBand );
  rasterRendererElem.setAttribute( QStringLiteral( "azimuth" ), QString::number( mLightAzimuth ) );
  rasterRendererElem.setAttribute( QStringLiteral( "angle" ), QString::number( mLightAngle ) );
  rasterRendererElem.setAttribute( QStringLiteral( "zfactor" ), QString::number( mZFactor ) );
  rasterRendererElem.setAttribute( QStringLiteral( "multidirection" ), QString::number( mMultiDirectional ) );
  parentElem.appendChild( rasterRendererElem );
}

QgsRasterBlock *QgsHillshadeRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )
  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
  {
    QgsDebugError( QStringLiteral( "No input raster!" ) );
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( mBand, extent, width, height, feedback ) );

  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugError( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > alphaBlock;

  if ( mAlphaBand > 0 && mBand != mAlphaBand )
  {
    alphaBlock.reset( mInput->block( mAlphaBand, extent, width, height, feedback ) );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      // TODO: better to render without alpha
      return outputBlock.release();
    }
  }
  else if ( mAlphaBand > 0 )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  if ( width == 0 || height == 0 )
    return outputBlock.release();

  // Starting the computation

  // Common pre-calculated values
  float cellXSize = static_cast<float>( extent.width() ) / width;
  float cellYSize = static_cast<float>( extent.height() ) / height;
  float zenithRad = static_cast<float>( std::max( 0.0, 90 - mLightAngle ) * M_PI / 180.0 );
  float azimuthRad = static_cast<float>( -1 * mLightAzimuth * M_PI / 180.0 );
  float cosZenithRad = std::cos( zenithRad );
  float sinZenithRad = std::sin( zenithRad );

  // For fast formula from GDAL DEM
  float cos_alt_mul_z = cosZenithRad * static_cast<float>( mZFactor );
  float cos_az_mul_cos_alt_mul_z = std::cos( azimuthRad ) * cos_alt_mul_z;
  float sin_az_mul_cos_alt_mul_z = std::sin( azimuthRad ) * cos_alt_mul_z;
  float cos_az_mul_cos_alt_mul_z_mul_254 = 254.0f * cos_az_mul_cos_alt_mul_z;
  float sin_az_mul_cos_alt_mul_z_mul_254 = 254.0f * sin_az_mul_cos_alt_mul_z;
  float square_z = static_cast<float>( mZFactor * mZFactor );
  float sin_altRadians_mul_254 = 254.0f * sinZenithRad;

  // For multi directional
  float sin_altRadians_mul_127 = 127.0f * sinZenithRad;
  // 127.0 * std::cos(225.0 *  M_PI / 180.0) = -32.87001872802012
  float cos225_az_mul_cos_alt_mul_z_mul_127 = -32.87001872802012f * cos_alt_mul_z;
  float cos_alt_mul_z_mul_127 = 127.0f * cos_alt_mul_z;

  const QRgb defaultNodataColor = renderColorForNodataPixel();

#ifdef HAVE_OPENCL

  // Use OpenCL? For now OpenCL is enabled in the default configuration only
  bool useOpenCL( QgsOpenClUtils::enabled()
                  && QgsOpenClUtils::available()
                  && ( ! mRasterTransparency || mRasterTransparency->isEmpty() )
                  && mAlphaBand <= 0
                  && inputBlock->dataTypeSize() <= 4 );
  // Check for sources
  QString source;
  if ( useOpenCL )
  {
    source = QgsOpenClUtils::sourceFromBaseName( QStringLiteral( "hillshade_renderer" ) );
    if ( source.isEmpty() )
    {
      useOpenCL = false;
      QgsMessageLog::logMessage( QObject::tr( "Error loading OpenCL program source from path %1" ).arg( QgsOpenClUtils::sourcePath() ), QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
    }
  }

#ifdef QGISDEBUG
  std::chrono::time_point<std::chrono::system_clock> startTime( std::chrono::system_clock::now() );
#endif

  if ( useOpenCL )
  {

    try
    {
      std::size_t inputDataTypeSize = inputBlock->dataTypeSize();
      std::size_t outputDataTypeSize = outputBlock->dataTypeSize();
      // Buffer scanline, 1px height, 2px wider
      QString typeName;
      switch ( inputBlock->dataType() )
      {
        case Qgis::DataType::Byte:
          typeName = QStringLiteral( "unsigned char" );
          break;
        case Qgis::DataType::UInt16:
          typeName = QStringLiteral( "unsigned int" );
          break;
        case Qgis::DataType::Int16:
          typeName = QStringLiteral( "short" );
          break;
        case Qgis::DataType::UInt32:
          typeName = QStringLiteral( "unsigned int" );
          break;
        case Qgis::DataType::Int32:
          typeName = QStringLiteral( "int" );
          break;
        case Qgis::DataType::Float32:
          typeName = QStringLiteral( "float" );
          break;
        default:
          throw QgsException( QStringLiteral( "Unsupported data type for OpenCL processing." ) );
      }

      if ( inputBlock->dataType() != Qgis::DataType::Float32 )
      {
        source.replace( QLatin1String( "__global float *scanLine" ), QStringLiteral( "__global %1 *scanLine" ).arg( typeName ) );
      }

      // Data type for input is Float32 (4 bytes)
      std::size_t scanLineWidth( inputBlock->width() + 2 );
      std::size_t inputSize( inputDataTypeSize * inputBlock->width() );

      // CL buffers are also 2px wider for nodata initial and final columns
      std::size_t bufferWidth( width + 2 );
      std::size_t bufferSize( inputDataTypeSize * bufferWidth );

      // Buffer scanlines, 1px height, 2px wider
      // Data type for input is Float32 (4 bytes)
      // keep only three scanlines in memory at a time, make room for initial and final nodata
      std::unique_ptr<QgsRasterBlock> scanLine = std::make_unique<QgsRasterBlock>( inputBlock->dataType(), scanLineWidth, 1 );
      // Note: output block is not 2px wider and it is an image
      // Prepare context and queue
      cl::Context ctx = QgsOpenClUtils::context();
      cl::CommandQueue queue = QgsOpenClUtils::commandQueue();

      // Cast to float (because double just crashes on some GPUs)
      std::vector<float> rasterParams;
      rasterParams.push_back( inputBlock->noDataValue() );
      rasterParams.push_back( outputBlock->noDataValue() );
      rasterParams.push_back( mZFactor );
      rasterParams.push_back( cellXSize );
      rasterParams.push_back( cellYSize );
      rasterParams.push_back( static_cast<float>( mOpacity ) ); // 5

      // For fast formula from GDAL DEM
      rasterParams.push_back( cos_az_mul_cos_alt_mul_z_mul_254 ); // 6
      rasterParams.push_back( sin_az_mul_cos_alt_mul_z_mul_254 ); // 7
      rasterParams.push_back( square_z ); // 8
      rasterParams.push_back( sin_altRadians_mul_254 ); // 9

      // For multidirectional fast formula
      rasterParams.push_back( sin_altRadians_mul_127 ); // 10
      rasterParams.push_back( cos225_az_mul_cos_alt_mul_z_mul_127 ); // 11
      rasterParams.push_back( cos_alt_mul_z_mul_127 ); // 12

      // Default color for nodata (BGR components)
      rasterParams.push_back( static_cast<float>( qBlue( defaultNodataColor ) ) ); // 13
      rasterParams.push_back( static_cast<float>( qGreen( defaultNodataColor ) ) ); // 14
      rasterParams.push_back( static_cast<float>( qRed( defaultNodataColor ) ) ); // 15
      rasterParams.push_back( static_cast<float>( qAlpha( defaultNodataColor ) ) / 255.0f ); // 16

      // Whether use multidirectional
      rasterParams.push_back( static_cast<float>( mMultiDirectional ) ); // 17

      cl::Buffer rasterParamsBuffer( queue, rasterParams.begin(), rasterParams.end(), true, false, nullptr );
      cl::Buffer scanLine1Buffer( ctx, CL_MEM_READ_ONLY, bufferSize, nullptr, nullptr );
      cl::Buffer scanLine2Buffer( ctx, CL_MEM_READ_ONLY, bufferSize, nullptr, nullptr );
      cl::Buffer scanLine3Buffer( ctx, CL_MEM_READ_ONLY, bufferSize, nullptr, nullptr );
      cl::Buffer *scanLineBuffer[3] = {&scanLine1Buffer, &scanLine2Buffer, &scanLine3Buffer};
      // Note that result buffer is an image
      cl::Buffer resultLineBuffer( ctx, CL_MEM_WRITE_ONLY, outputDataTypeSize * width, nullptr, nullptr );

      static std::map<Qgis::DataType, cl::Program> programCache;
      cl::Program program = programCache[inputBlock->dataType()];
      if ( ! program.get() )
      {
        // Create a program from the kernel source
        programCache[inputBlock->dataType()] = QgsOpenClUtils::buildProgram( source, QgsOpenClUtils::ExceptionBehavior::Throw );
        program = programCache[inputBlock->dataType()];
      }

      // Disable program cache when developing and testing cl program
      // program = QgsOpenClUtils::buildProgram( ctx, source, QgsOpenClUtils::ExceptionBehavior::Throw );

      // Create the OpenCL kernel
      auto kernel =  cl::KernelFunctor <
                     cl::Buffer &,
                     cl::Buffer &,
                     cl::Buffer &,
                     cl::Buffer &,
                     cl::Buffer &
                     > ( program, "processNineCellWindow" );


      // Rotating buffer index
      std::vector<int> rowIndex = {0, 1, 2};

      for ( int i = 0; i < height; i++ )
      {
        if ( feedback && feedback->isCanceled() )
        {
          break;
        }

        if ( feedback )
        {
          feedback->setProgress( 100.0 * static_cast< double >( i ) / height );
        }

        if ( i == 0 )
        {
          // Fill scanline 1 with (input) nodata for the values above the first row and feed scanline2 with the first row
          scanLine->resetNoDataValue();
          queue.enqueueWriteBuffer( scanLine1Buffer, CL_TRUE, 0, bufferSize, scanLine->bits( ) );
          // Read first row
          memcpy( scanLine->bits( 0, 1 ), inputBlock->bits( i, 0 ), inputSize );
          queue.enqueueWriteBuffer( scanLine2Buffer, CL_TRUE, 0, bufferSize, scanLine->bits( ) ); // row 0
          // Second row
          memcpy( scanLine->bits( 0, 1 ), inputBlock->bits( i + 1, 0 ), inputSize );
          queue.enqueueWriteBuffer( scanLine3Buffer, CL_TRUE, 0, bufferSize, scanLine->bits( ) ); //
        }
        else
        {
          // Normally fetch only scanLine3 and move forward one row
          // Read scanline 3, fill the last row with nodata values if it's the last iteration
          if ( i == inputBlock->height() - 1 )
          {
            scanLine->resetNoDataValue();
            queue.enqueueWriteBuffer( *scanLineBuffer[rowIndex[2]], CL_TRUE, 0, bufferSize, scanLine->bits( ) );
          }
          else // Overwrite from input, skip first and last
          {
            queue.enqueueWriteBuffer( *scanLineBuffer[rowIndex[2]], CL_TRUE, inputDataTypeSize * 1 /* offset 1 */, inputSize, inputBlock->bits( i + 1, 0 ) );
          }
        }

        kernel( cl::EnqueueArgs(
                  queue,
                  cl::NDRange( width )
                ),
                *scanLineBuffer[rowIndex[0]],
                *scanLineBuffer[rowIndex[1]],
                *scanLineBuffer[rowIndex[2]],
                resultLineBuffer,
                rasterParamsBuffer
              );

        queue.enqueueReadBuffer( resultLineBuffer, CL_TRUE, 0, outputDataTypeSize * outputBlock->width( ), outputBlock->bits( i, 0 ) );
        std::rotate( rowIndex.begin(), rowIndex.begin() + 1, rowIndex.end() );
      }
    }
    catch ( cl::Error &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Error running OpenCL program: %1 - %2" ).arg( e.what( ) ).arg( QgsOpenClUtils::errorText( e.err( ) ) ),
                                 QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
      QgsOpenClUtils::setEnabled( false );
      QgsMessageLog::logMessage( QObject::tr( "OpenCL has been disabled, you can re-enable it in the options dialog." ),
                                 QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
    }

  } // End of OpenCL processing path
  else  // Use the CPU and the original algorithm
  {

#endif

    double pixelValues[9] {0, 0, 0, 0, 0, 0, 0, 0, 0};
    bool isNoData[9] {false, false, false, false, false, false, false, false, false};

    for ( int row = 0; row < height; row++ )
    {
      for ( int col = 0; col < width; col++ )
      {
        int iUp = row - 1;
        int iDown = row + 1;
        if ( row == 0 )
        {
          iUp = row;
        }
        else if ( row == height - 1 )
        {
          iDown = row;
        }

        if ( col == 0 )
        {
          // seed the matrix with the values from the first column
          pixelValues[ 0 ] = inputBlock->valueAndNoData( iUp, 0, isNoData[0] );
          pixelValues[ 1 ] = pixelValues[0];
          isNoData[1] = isNoData[0];
          pixelValues[ 2 ] = pixelValues[0];
          isNoData[2] = isNoData[0];

          pixelValues[ 3 ] = inputBlock->valueAndNoData( row, 0, isNoData[3] );
          pixelValues[ 4 ] = pixelValues[3];
          isNoData[4] = isNoData[3];
          pixelValues[ 5 ] = pixelValues[3];
          isNoData[5] = isNoData[3];

          pixelValues[ 6 ] = inputBlock->valueAndNoData( iDown, 0, isNoData[6] );
          pixelValues[ 7 ] = pixelValues[6];
          isNoData[7] = isNoData[6];
          pixelValues[ 8 ] = pixelValues[6];
          isNoData[8] = isNoData[6];
        }
        else
        {
          // shift matrices left
          pixelValues[ 0 ] = pixelValues[1];
          pixelValues[ 1 ] = pixelValues[2];
          pixelValues[ 3 ] = pixelValues[4];
          pixelValues[ 4 ] = pixelValues[5];
          pixelValues[ 6 ] = pixelValues[7];
          pixelValues[ 7 ] = pixelValues[8];
          isNoData[ 0 ] = isNoData[1];
          isNoData[ 1 ] = isNoData[2];
          isNoData[ 3 ] = isNoData[4];
          isNoData[ 4 ] = isNoData[5];
          isNoData[ 6 ] = isNoData[7];
          isNoData[ 7 ] = isNoData[8];
        }

        // calculate new values
        if ( col < width - 1 )
        {
          pixelValues[2] = inputBlock->valueAndNoData( iUp, col + 1, isNoData[2] );
          pixelValues[5] = inputBlock->valueAndNoData( row, col + 1, isNoData[5] );
          pixelValues[8] = inputBlock->valueAndNoData( iDown, col + 1, isNoData[8] );
        }

        if ( isNoData[ 4 ] )
        {
          outputBlock->setColor( row, col, defaultNodataColor );
          continue;
        }

        // This is center cell. Use this in place of nodata neighbors
        const double x22 = pixelValues[4];

        const double x11 = isNoData[0] ? x22 : pixelValues[0];
        const double x21 = isNoData[3] ? x22 : pixelValues[3];
        const double x31 = isNoData[6] ? x22 : pixelValues[6];
        const double x12 = isNoData[1] ? x22 : pixelValues[1];
        // x22
        const double x32 = isNoData[7] ? x22 : pixelValues[7];
        const double x13 = isNoData[2] ? x22 : pixelValues[2];
        const double x23 = isNoData[5] ? x22 : pixelValues[5];
        const double x33 = isNoData[8] ? x22 : pixelValues[8];

        // Calculates the first order derivative in x-direction according to Horn (1981)
        const double derX = ( ( x13 + x23 + x23 + x33 ) - ( x11 + x21 + x21 + x31 ) ) / ( 8 * cellXSize );
        const double derY = ( ( x31 + x32 + x32 + x33 ) - ( x11 + x12 + x12 + x13 ) ) / ( 8 * -cellYSize );

        // Fast formula

        double grayValue;
        if ( !mMultiDirectional )
        {
          // Standard single direction hillshade
          grayValue = std::clamp( ( sin_altRadians_mul_254 -
                                    ( derY * cos_az_mul_cos_alt_mul_z_mul_254 -
                                      derX * sin_az_mul_cos_alt_mul_z_mul_254 ) ) /
                                  std::sqrt( 1 + square_z * ( derX * derX + derY * derY ) ),
                                  0.0, 255.0 );
        }
        else
        {
          // Weighted multi direction as in http://pubs.usgs.gov/of/1992/of92-422/of92-422.pdf
          // Fast formula from GDAL DEM
          const float xx = derX * derX;
          const float yy = derY * derY;
          const float xx_plus_yy = xx + yy;
          // Flat?
          if ( xx_plus_yy == 0.0 )
          {
            grayValue = std::clamp( static_cast<float>( 1.0 + sin_altRadians_mul_254 ), 0.0f, 255.0f );
          }
          else
          {
            // ... then the shade value from different azimuth
            float val225_mul_127 = sin_altRadians_mul_127 +
                                   ( derX - derY ) * cos225_az_mul_cos_alt_mul_z_mul_127;
            val225_mul_127 = ( val225_mul_127 <= 0.0 ) ? 0.0 : val225_mul_127;
            float val270_mul_127 = sin_altRadians_mul_127 -
                                   derX * cos_alt_mul_z_mul_127;
            val270_mul_127 = ( val270_mul_127 <= 0.0 ) ? 0.0 : val270_mul_127;
            float val315_mul_127 = sin_altRadians_mul_127 +
                                   ( derX + derY ) * cos225_az_mul_cos_alt_mul_z_mul_127;
            val315_mul_127 = ( val315_mul_127 <= 0.0 ) ? 0.0 : val315_mul_127;
            float val360_mul_127 = sin_altRadians_mul_127 -
                                   derY * cos_alt_mul_z_mul_127;
            val360_mul_127 = ( val360_mul_127 <= 0.0 ) ? 0.0 : val360_mul_127;

            // ... then the weighted shading
            const float weight_225 = 0.5 * xx_plus_yy - derX * derY;
            const float weight_270 = xx;
            const float weight_315 = xx_plus_yy - weight_225;
            const float weight_360 = yy;
            const float cang_mul_127 = (
                                         ( weight_225 * val225_mul_127 +
                                           weight_270 * val270_mul_127 +
                                           weight_315 * val315_mul_127 +
                                           weight_360 * val360_mul_127 ) / xx_plus_yy ) /
                                       ( 1 + square_z * xx_plus_yy );

            grayValue = std::clamp( 1.0f + cang_mul_127, 0.0f, 255.0f );
          }
        }

        double currentAlpha = mOpacity;
        if ( mRasterTransparency )
        {
          currentAlpha *= mRasterTransparency->opacityForValue( x22 );
        }
        if ( mAlphaBand > 0 )
        {
          currentAlpha *= alphaBlock->value( row ) / 255.0;
        }

        if ( qgsDoubleNear( currentAlpha, 1.0 ) )
        {
          outputBlock->setColor( row, col, qRgba( grayValue, grayValue, grayValue, 255 ) );
        }
        else
        {
          outputBlock->setColor( row, col, qRgba( currentAlpha * grayValue, currentAlpha * grayValue, currentAlpha * grayValue, currentAlpha * 255 ) );
        }
      }
    }

#ifdef HAVE_OPENCL
  } // End of switch in case OpenCL is not available or enabled

#ifdef QGISDEBUG
  if ( QgsSettings().value( QStringLiteral( "Map/logCanvasRefreshEvent" ), false ).toBool() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "%1 processing time for hillshade (%2 x %3 ): %4 ms" )
                               .arg( useOpenCL ? QStringLiteral( "OpenCL" ) : QStringLiteral( "CPU" ) )
                               .arg( width )
                               .arg( height )
                               .arg( std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - startTime ).count() ),
                               tr( "Rendering" ) );
  }
#endif
#endif

  return outputBlock.release();
}

QList<int> QgsHillshadeRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mBand != -1 )
  {
    bandList << mBand;
  }
  return bandList;

}

int QgsHillshadeRenderer::inputBand() const
{
  return mBand;
}

void QgsHillshadeRenderer::setBand( int bandNo )
{
  setInputBand( bandNo );
}

bool QgsHillshadeRenderer::setInputBand( int band )
{
  if ( !mInput )
  {
    mBand = band;
    return true;
  }
  else if ( band > 0 && band <= mInput->bandCount() )
  {
    mBand = band;
    return true;
  }
  return false;
}

void QgsHillshadeRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // create base structure
  QgsRasterRenderer::toSld( doc, element, props );

  // look for RasterSymbolizer tag
  QDomNodeList elements = element.elementsByTagName( QStringLiteral( "sld:RasterSymbolizer" ) );
  if ( elements.size() == 0 )
    return;

  // there SHOULD be only one
  QDomElement rasterSymbolizerElem = elements.at( 0 ).toElement();

  // add Channel Selection tags (if band is not default 1)
  // Need to insert channelSelection in the correct sequence as in SLD standard e.g.
  // after opacity or geometry or as first element after sld:RasterSymbolizer
  if ( mBand != 1 )
  {
    QDomElement channelSelectionElem = doc.createElement( QStringLiteral( "sld:ChannelSelection" ) );
    elements = rasterSymbolizerElem.elementsByTagName( QStringLiteral( "sld:Opacity" ) );
    if ( elements.size() != 0 )
    {
      rasterSymbolizerElem.insertAfter( channelSelectionElem, elements.at( 0 ) );
    }
    else
    {
      elements = rasterSymbolizerElem.elementsByTagName( QStringLiteral( "sld:Geometry" ) );
      if ( elements.size() != 0 )
      {
        rasterSymbolizerElem.insertAfter( channelSelectionElem, elements.at( 0 ) );
      }
      else
      {
        rasterSymbolizerElem.insertBefore( channelSelectionElem, rasterSymbolizerElem.firstChild() );
      }
    }

    // for gray band
    QDomElement channelElem = doc.createElement( QStringLiteral( "sld:GrayChannel" ) );
    channelSelectionElem.appendChild( channelElem );

    // set band
    QDomElement sourceChannelNameElem = doc.createElement( QStringLiteral( "sld:SourceChannelName" ) );
    sourceChannelNameElem.appendChild( doc.createTextNode( QString::number( mBand ) ) );
    channelElem.appendChild( sourceChannelNameElem );
  }

  // add ShadedRelief tag
  QDomElement shadedReliefElem = doc.createElement( QStringLiteral( "sld:ShadedRelief" ) );
  rasterSymbolizerElem.appendChild( shadedReliefElem );

  // brightnessOnly tag
  QDomElement brightnessOnlyElem = doc.createElement( QStringLiteral( "sld:BrightnessOnly" ) );
  brightnessOnlyElem.appendChild( doc.createTextNode( QStringLiteral( "true" ) ) );
  shadedReliefElem.appendChild( brightnessOnlyElem );

  // ReliefFactor tag
  QDomElement reliefFactorElem = doc.createElement( QStringLiteral( "sld:ReliefFactor" ) );
  reliefFactorElem.appendChild( doc.createTextNode( QString::number( zFactor() ) ) );
  shadedReliefElem.appendChild( reliefFactorElem );

  // altitude VendorOption tag
  QDomElement altitudeVendorOptionElem = doc.createElement( QStringLiteral( "sld:VendorOption" ) );
  altitudeVendorOptionElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "altitude" ) );
  altitudeVendorOptionElem.appendChild( doc.createTextNode( QString::number( altitude() ) ) );
  shadedReliefElem.appendChild( altitudeVendorOptionElem );

  // azimuth VendorOption tag
  QDomElement azimutVendorOptionElem = doc.createElement( QStringLiteral( "sld:VendorOption" ) );
  azimutVendorOptionElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "azimuth" ) );
  azimutVendorOptionElem.appendChild( doc.createTextNode( QString::number( azimuth() ) ) );
  shadedReliefElem.appendChild( azimutVendorOptionElem );

  // multidirectional VendorOption tag
  QDomElement multidirectionalVendorOptionElem = doc.createElement( QStringLiteral( "sld:VendorOption" ) );
  multidirectionalVendorOptionElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "multidirectional" ) );
  multidirectionalVendorOptionElem.appendChild( doc.createTextNode( QString::number( multiDirectional() ) ) );
  shadedReliefElem.appendChild( multidirectionalVendorOptionElem );
}
