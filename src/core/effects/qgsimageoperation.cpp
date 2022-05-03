/***************************************************************************
                           qgsimageoperation.cpp
                           ----------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsimageoperation.h"
#include "qgis.h"
#include "qgscolorramp.h"
#include "qgslogger.h"
#include "qgsfeedback.h"
#include <QtConcurrentMap>
#include <QColor>
#include <QPainter>

//determined via trial-and-error. Could possibly be optimised, or varied
//depending on the image size.
#define BLOCK_THREADS 16

#define INF 1E20

/// @cond PRIVATE

template <typename PixelOperation>
void QgsImageOperation::runPixelOperation( QImage &image, PixelOperation &operation, QgsFeedback *feedback )
{
  if ( static_cast< qgssize >( image.height() ) * image.width() < 100000 )
  {
    //small image, don't multithread
    //this threshold was determined via testing various images
    runPixelOperationOnWholeImage( image, operation, feedback );
  }
  else
  {
    //large image, multithread operation
    QgsImageOperation::ProcessBlockUsingPixelOperation<PixelOperation> blockOp( operation, feedback );
    runBlockOperationInThreads( image, blockOp, QgsImageOperation::ByRow );
  }
}

template <typename PixelOperation>
void QgsImageOperation::runPixelOperationOnWholeImage( QImage &image, PixelOperation &operation, QgsFeedback *feedback )
{
  int height = image.height();
  int width = image.width();
  for ( int y = 0; y < height; ++y )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    QRgb *ref = reinterpret_cast< QRgb * >( image.scanLine( y ) );
    for ( int x = 0; x < width; ++x )
    {
      operation( ref[x], x, y );
    }
  }
}

//rect operations

template <typename RectOperation>
void QgsImageOperation::runRectOperation( QImage &image, RectOperation &operation )
{
  //possibly could be tweaked for rect operations
  if ( static_cast< qgssize >( image.height() ) * image.width() < 100000 )
  {
    //small image, don't multithread
    //this threshold was determined via testing various images
    runRectOperationOnWholeImage( image, operation );
  }
  else
  {
    //large image, multithread operation
    runBlockOperationInThreads( image, operation, ByRow );
  }
}

template <class RectOperation>
void QgsImageOperation::runRectOperationOnWholeImage( QImage &image, RectOperation &operation )
{
  ImageBlock fullImage;
  fullImage.beginLine = 0;
  fullImage.endLine = image.height();
  fullImage.lineLength = image.width();
  fullImage.image = &image;

  operation( fullImage );
}

//linear operations

template <typename LineOperation>
void QgsImageOperation::runLineOperation( QImage &image, LineOperation &operation, QgsFeedback *feedback )
{
  //possibly could be tweaked for rect operations
  if ( static_cast< qgssize >( image.height() ) * image.width() < 100000 )
  {
    //small image, don't multithread
    //this threshold was determined via testing various images
    runLineOperationOnWholeImage( image, operation, feedback );
  }
  else
  {
    //large image, multithread operation
    QgsImageOperation::ProcessBlockUsingLineOperation<LineOperation> blockOp( operation );
    runBlockOperationInThreads( image, blockOp, operation.direction() );
  }
}

template <class LineOperation>
void QgsImageOperation::runLineOperationOnWholeImage( QImage &image, LineOperation &operation, QgsFeedback *feedback )
{
  int height = image.height();
  int width = image.width();

  //do something with whole lines
  int bpl = image.bytesPerLine();
  if ( operation.direction() == ByRow )
  {
    for ( int y = 0; y < height; ++y )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      QRgb *ref = reinterpret_cast< QRgb * >( image.scanLine( y ) );
      operation( ref, width, bpl );
    }
  }
  else
  {
    //by column
    unsigned char *ref = image.scanLine( 0 );
    for ( int x = 0; x < width; ++x, ref += 4 )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      operation( reinterpret_cast< QRgb * >( ref ), height, bpl );
    }
  }
}


//multithreaded block processing

template <typename BlockOperation>
void QgsImageOperation::runBlockOperationInThreads( QImage &image, BlockOperation &operation, LineOperationDirection direction )
{
  QList< ImageBlock > blocks;
  unsigned int height = image.height();
  unsigned int width = image.width();

  unsigned int blockDimension1 = ( direction == QgsImageOperation::ByRow ) ? height : width;
  unsigned int blockDimension2 = ( direction == QgsImageOperation::ByRow ) ? width : height;

  //chunk image up into vertical blocks
  blocks.reserve( BLOCK_THREADS );
  unsigned int begin = 0;
  unsigned int blockLen = blockDimension1 / BLOCK_THREADS;
  for ( unsigned int block = 0; block < BLOCK_THREADS; ++block, begin += blockLen )
  {
    ImageBlock newBlock;
    newBlock.beginLine = begin;
    //make sure last block goes to end of image
    newBlock.endLine = block < ( BLOCK_THREADS - 1 ) ? begin + blockLen : blockDimension1;
    newBlock.lineLength = blockDimension2;
    newBlock.image = &image;
    blocks << newBlock;
  }

  //process blocks
  QtConcurrent::blockingMap( blocks, operation );
}


///@endcond

//
//operation specific code
//

//grayscale

void QgsImageOperation::convertToGrayscale( QImage &image, const GrayscaleMode mode, QgsFeedback *feedback )
{
  if ( mode == GrayscaleOff )
  {
    return;
  }

  image.detach();
  GrayscalePixelOperation operation( mode );
  runPixelOperation( image, operation, feedback );
}

void QgsImageOperation::GrayscalePixelOperation::operator()( QRgb &rgb, const int x, const int y ) const
{
  Q_UNUSED( x )
  Q_UNUSED( y )
  switch ( mMode )
  {
    case GrayscaleOff:
      return;
    case GrayscaleLuminosity:
      grayscaleLuminosityOp( rgb );
      return;
    case GrayscaleAverage:
      grayscaleAverageOp( rgb );
      return;
    case GrayscaleLightness:
    default:
      grayscaleLightnessOp( rgb );
      return;
  }
}

void QgsImageOperation::grayscaleLightnessOp( QRgb &rgb )
{
  int red = qRed( rgb );
  int green = qGreen( rgb );
  int blue = qBlue( rgb );

  int min = std::min( std::min( red, green ), blue );
  int max = std::max( std::max( red, green ), blue );

  int lightness = std::min( ( min + max ) / 2, 255 );
  rgb = qRgba( lightness, lightness, lightness, qAlpha( rgb ) );
}

void QgsImageOperation::grayscaleLuminosityOp( QRgb &rgb )
{
  int luminosity = 0.21 * qRed( rgb ) + 0.72 * qGreen( rgb ) + 0.07 * qBlue( rgb );
  rgb = qRgba( luminosity, luminosity, luminosity, qAlpha( rgb ) );
}

void QgsImageOperation::grayscaleAverageOp( QRgb &rgb )
{
  int average = ( qRed( rgb ) + qGreen( rgb ) + qBlue( rgb ) ) / 3;
  rgb = qRgba( average, average, average, qAlpha( rgb ) );
}


//brightness/contrast

void QgsImageOperation::adjustBrightnessContrast( QImage &image, const int brightness, const double contrast, QgsFeedback *feedback )
{
  image.detach();
  BrightnessContrastPixelOperation operation( brightness, contrast );
  runPixelOperation( image, operation, feedback );
}

void QgsImageOperation::BrightnessContrastPixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  Q_UNUSED( x )
  Q_UNUSED( y )
  int red = adjustColorComponent( qRed( rgb ), mBrightness, mContrast );
  int blue = adjustColorComponent( qBlue( rgb ), mBrightness, mContrast );
  int green = adjustColorComponent( qGreen( rgb ), mBrightness, mContrast );
  rgb = qRgba( red, green, blue, qAlpha( rgb ) );
}

int QgsImageOperation::adjustColorComponent( int colorComponent, int brightness, double contrastFactor )
{
  return std::clamp( static_cast< int >( ( ( ( ( ( colorComponent / 255.0 ) - 0.5 ) * contrastFactor ) + 0.5 ) * 255 ) + brightness ), 0, 255 );
}

//hue/saturation

void QgsImageOperation::adjustHueSaturation( QImage &image, const double saturation, const QColor &colorizeColor, const double colorizeStrength, QgsFeedback *feedback )
{
  image.detach();
  HueSaturationPixelOperation operation( saturation, colorizeColor.isValid() && colorizeStrength > 0.0,
                                         colorizeColor.hue(), colorizeColor.saturation(), colorizeStrength );
  runPixelOperation( image, operation, feedback );
}

void QgsImageOperation::HueSaturationPixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  Q_UNUSED( x )
  Q_UNUSED( y )
  QColor tmpColor( rgb );
  int h, s, l;
  tmpColor.getHsl( &h, &s, &l );

  if ( mSaturation < 1.0 )
  {
    // Lowering the saturation. Use a simple linear relationship
    s = std::min( static_cast< int >( s * mSaturation ), 255 );
  }
  else if ( mSaturation > 1.0 )
  {
    // Raising the saturation. Use a saturation curve to prevent
    // clipping at maximum saturation with ugly results.
    s = std::min( static_cast< int >( 255. * ( 1 - std::pow( 1 - ( s / 255. ), std::pow( mSaturation, 2 ) ) ) ), 255 );
  }

  if ( mColorize )
  {
    h = mColorizeHue;
    s = mColorizeSaturation;
    if ( mColorizeStrength < 1.0 )
    {
      //get rgb for colorized color
      QColor colorizedColor = QColor::fromHsl( h, s, l );
      int colorizedR, colorizedG, colorizedB;
      colorizedColor.getRgb( &colorizedR, &colorizedG, &colorizedB );

      // Now, linearly scale by colorize strength
      int r = mColorizeStrength * colorizedR + ( 1 - mColorizeStrength ) * tmpColor.red();
      int g = mColorizeStrength * colorizedG + ( 1 - mColorizeStrength ) * tmpColor.green();
      int b = mColorizeStrength * colorizedB + ( 1 - mColorizeStrength ) * tmpColor.blue();

      rgb = qRgba( r, g, b, qAlpha( rgb ) );
      return;
    }
  }

  tmpColor.setHsl( h, s, l, qAlpha( rgb ) );
  rgb = tmpColor.rgba();
}

//multiply opacity

void QgsImageOperation::multiplyOpacity( QImage &image, const double factor, QgsFeedback *feedback )
{
  if ( qgsDoubleNear( factor, 1.0 ) )
  {
    //no change
    return;
  }
  else if ( factor < 1.0 )
  {
    //decreasing opacity - we can use the faster DestinationIn composition mode
    //to reduce the alpha channel
    QColor transparentFillColor = QColor( 0, 0, 0, 255 * factor );
    image.detach();
    QPainter painter( &image );
    painter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    painter.fillRect( 0, 0, image.width(), image.height(), transparentFillColor );
    painter.end();
  }
  else
  {
    //increasing opacity - run this as a pixel operation for multithreading
    image.detach();
    MultiplyOpacityPixelOperation operation( factor );
    runPixelOperation( image, operation, feedback );
  }
}

void QgsImageOperation::MultiplyOpacityPixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  Q_UNUSED( x )
  Q_UNUSED( y )
  rgb = qRgba( qRed( rgb ), qGreen( rgb ), qBlue( rgb ), std::clamp( std::round( mFactor * qAlpha( rgb ) ), 0.0, 255.0 ) );
}

// overlay color

void QgsImageOperation::overlayColor( QImage &image, const QColor &color )
{
  QColor opaqueColor = color;
  opaqueColor.setAlpha( 255 );

  //use QPainter SourceIn composition mode to overlay color (fast)
  //this retains image's alpha channel but replaces color
  image.detach();
  QPainter painter( &image );
  painter.setCompositionMode( QPainter::CompositionMode_SourceIn );
  painter.fillRect( 0, 0, image.width(), image.height(), opaqueColor );
  painter.end();
}

// distance transform

void QgsImageOperation::distanceTransform( QImage &image, const DistanceTransformProperties &properties, QgsFeedback *feedback )
{
  if ( ! properties.ramp )
  {
    QgsDebugMsg( QStringLiteral( "no color ramp specified for distance transform" ) );
    return;
  }

  //first convert to 1 bit alpha mask array
  std::unique_ptr<double[]> array( new double[ static_cast< qgssize >( image.width() ) * image.height()] );
  if ( feedback && feedback->isCanceled() )
    return;

  image.detach();
  ConvertToArrayPixelOperation convertToArray( image.width(), array.get(), properties.shadeExterior );
  runPixelOperation( image, convertToArray, feedback );
  if ( feedback && feedback->isCanceled() )
    return;

  //calculate distance transform (single threaded only)
  distanceTransform2d( array.get(), image.width(), image.height(), feedback );
  if ( feedback && feedback->isCanceled() )
    return;

  double spread;
  if ( properties.useMaxDistance )
  {
    spread = std::sqrt( maxValueInDistanceTransformArray( array.get(), image.width() * image.height() ) );
  }
  else
  {
    spread = properties.spread;
  }

  if ( feedback && feedback->isCanceled() )
    return;

  //shade distance transform
  ShadeFromArrayOperation shadeFromArray( image.width(), array.get(), spread, properties );
  runPixelOperation( image, shadeFromArray, feedback );
}

void QgsImageOperation::ConvertToArrayPixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  qgssize idx = y * static_cast< qgssize >( mWidth ) + x;
  if ( mExterior )
  {
    if ( qAlpha( rgb ) > 0 )
    {
      //opaque pixel, so zero distance
      mArray[ idx ] = 1 - qAlpha( rgb ) / 255.0;
    }
    else
    {
      //transparent pixel, so initially set distance as infinite
      mArray[ idx ] = INF;
    }
  }
  else
  {
    //TODO - fix this for semi-transparent pixels
    if ( qAlpha( rgb ) == 255 )
    {
      mArray[ idx ] = INF;
    }
    else
    {
      mArray[idx] = 0;
    }
  }
}

//fast distance transform code, adapted from http://cs.brown.edu/~pff/dt/

/* distance transform of a 1d function using squared distance */
void QgsImageOperation::distanceTransform1d( double *f, int n, int *v, double *z, double *d )
{
  int k = 0;
  v[0] = 0;
  z[0] = -INF;
  z[1] = + INF;
  for ( int q = 1; q <= n - 1; q++ )
  {
    double s  = ( ( f[q] + q * q ) - ( f[v[k]] + ( v[k] * v[k] ) ) ) / ( 2 * q - 2 * v[k] );
    while ( s <= z[k] )
    {
      k--;
      s  = ( ( f[q] + q * q ) - ( f[v[k]] + ( v[k] * v[k] ) ) ) / ( 2 * q - 2 * v[k] );
    }
    k++;
    v[k] = q;
    z[k] = s;
    z[k + 1] = + INF;
  }

  k = 0;
  for ( int q = 0; q <= n - 1; q++ )
  {
    while ( z[k + 1] < q )
      k++;
    d[q] = ( q - v[k] ) * ( q - v[k] ) + f[v[k]];
  }
}

double QgsImageOperation::maxValueInDistanceTransformArray( const double *array, const unsigned int size )
{
  double dtMaxValue = array[0];
  for ( unsigned int i = 1; i < size; ++i )
  {
    if ( array[i] > dtMaxValue )
    {
      dtMaxValue = array[i];
    }
  }
  return dtMaxValue;
}

/* distance transform of 2d function using squared distance */
void QgsImageOperation::distanceTransform2d( double *im, int width, int height, QgsFeedback *feedback )
{
  int maxDimension = std::max( width, height );

  std::unique_ptr<double[]> f( new double[ maxDimension ] );
  std::unique_ptr<int []> v( new int[ maxDimension ] );
  std::unique_ptr<double[]>z( new double[ maxDimension + 1 ] );
  std::unique_ptr<double[]>d( new double[ maxDimension ] );

  // transform along columns
  for ( int x = 0; x < width; x++ )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    for ( int y = 0; y < height; y++ )
    {
      f[y] = im[ x + y * width ];
    }
    distanceTransform1d( f.get(), height, v.get(), z.get(), d.get() );
    for ( int y = 0; y < height; y++ )
    {
      im[ x + y * width ] = d[y];
    }
  }

  // transform along rows
  for ( int y = 0; y < height; y++ )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    for ( int x = 0; x < width; x++ )
    {
      f[x] = im[  x + y * width ];
    }
    distanceTransform1d( f.get(), width, v.get(), z.get(), d.get() );
    for ( int x = 0; x < width; x++ )
    {
      im[  x + y * width ] = d[x];
    }
  }
}

void QgsImageOperation::ShadeFromArrayOperation::operator()( QRgb &rgb, const int x, const int y )
{
  if ( ! mProperties.ramp )
    return;

  if ( qgsDoubleNear( mSpread, 0.0 ) )
  {
    rgb = mProperties.ramp->color( 1.0 ).rgba();
    return;
  }

  int idx = y * mWidth + x;

  //values are distance squared
  double squaredVal = mArray[ idx ];
  if ( squaredVal > mSpreadSquared )
  {
    rgb = Qt::transparent;
    return;
  }

  double distance = std::sqrt( squaredVal );
  double val = distance / mSpread;
  QColor rampColor = mProperties.ramp->color( val );

  if ( ( mProperties.shadeExterior && distance > mSpread - 1 ) )
  {
    //fade off final pixel to antialias edge
    double alphaMultiplyFactor = mSpread - distance;
    rampColor.setAlpha( rampColor.alpha() * alphaMultiplyFactor );
  }
  rgb = rampColor.rgba();
}

//stack blur

void QgsImageOperation::stackBlur( QImage &image, const int radius, const bool alphaOnly, QgsFeedback *feedback )
{
  // culled from Qt's qpixmapfilter.cpp, see: http://www.qtcentre.org/archive/index.php/t-26534.html
  int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
  int alpha = ( radius < 1 )  ? 16 : ( radius > 17 ) ? 1 : tab[radius - 1];

  int i1 = 0;
  int i2 = 3;

  //ensure correct source format.
  QImage::Format originalFormat = image.format();
  QImage *pImage = &image;
  std::unique_ptr< QImage> convertedImage;
  if ( !alphaOnly && originalFormat != QImage::Format_ARGB32_Premultiplied )
  {
    convertedImage = std::make_unique< QImage >( image.convertToFormat( QImage::Format_ARGB32_Premultiplied ) );
    pImage = convertedImage.get();
  }
  else if ( alphaOnly && originalFormat != QImage::Format_ARGB32 )
  {
    convertedImage = std::make_unique< QImage >( image.convertToFormat( QImage::Format_ARGB32 ) );
    pImage = convertedImage.get();
  }
  else
  {
    image.detach();
  }

  if ( feedback && feedback->isCanceled() )
    return;

  if ( alphaOnly )
    i1 = i2 = ( QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3 );

  StackBlurLineOperation topToBottomBlur( alpha, QgsImageOperation::ByColumn, true, i1, i2, feedback );
  runLineOperation( *pImage, topToBottomBlur, feedback );

  if ( feedback && feedback->isCanceled() )
    return;

  StackBlurLineOperation leftToRightBlur( alpha, QgsImageOperation::ByRow, true, i1, i2, feedback );
  runLineOperation( *pImage, leftToRightBlur, feedback );

  if ( feedback && feedback->isCanceled() )
    return;

  StackBlurLineOperation bottomToTopBlur( alpha, QgsImageOperation::ByColumn, false, i1, i2, feedback );
  runLineOperation( *pImage, bottomToTopBlur, feedback );

  if ( feedback && feedback->isCanceled() )
    return;

  StackBlurLineOperation rightToLeftBlur( alpha, QgsImageOperation::ByRow, false, i1, i2, feedback );
  runLineOperation( *pImage, rightToLeftBlur, feedback );

  if ( feedback && feedback->isCanceled() )
    return;

  if ( pImage->format() != originalFormat )
  {
    image = pImage->convertToFormat( originalFormat );
  }
}

//gaussian blur

QImage *QgsImageOperation::gaussianBlur( QImage &image, const int radius, QgsFeedback *feedback )
{
  int width = image.width();
  int height = image.height();

  if ( radius <= 0 )
  {
    //just make an unchanged copy
    QImage *copy = new QImage( image.copy() );
    return copy;
  }

  std::unique_ptr<double[]>kernel( createGaussianKernel( radius ) );
  if ( feedback && feedback->isCanceled() )
    return new QImage();

  //ensure correct source format.
  QImage::Format originalFormat = image.format();
  QImage *pImage = &image;
  std::unique_ptr< QImage> convertedImage;
  if ( originalFormat != QImage::Format_ARGB32_Premultiplied )
  {
    convertedImage = std::make_unique< QImage >( image.convertToFormat( QImage::Format_ARGB32_Premultiplied ) );
    pImage = convertedImage.get();
  }
  else
  {
    image.detach();
  }
  if ( feedback && feedback->isCanceled() )
    return new QImage();

  //blur along rows
  QImage xBlurImage = QImage( width, height, QImage::Format_ARGB32_Premultiplied );
  GaussianBlurOperation rowBlur( radius, QgsImageOperation::ByRow, &xBlurImage, kernel.get(), feedback );
  runRectOperation( *pImage, rowBlur );

  if ( feedback && feedback->isCanceled() )
    return new QImage();

  //blur along columns
  std::unique_ptr< QImage > yBlurImage = std::make_unique< QImage >( width, height, QImage::Format_ARGB32_Premultiplied );
  GaussianBlurOperation colBlur( radius, QgsImageOperation::ByColumn, yBlurImage.get(), kernel.get(), feedback );
  runRectOperation( xBlurImage, colBlur );

  if ( feedback && feedback->isCanceled() )
    return new QImage();

  kernel.reset();

  if ( originalFormat != QImage::Format_ARGB32_Premultiplied )
  {
    return new QImage( yBlurImage->convertToFormat( originalFormat ) );
  }

  return yBlurImage.release();
}

void QgsImageOperation::GaussianBlurOperation::operator()( QgsImageOperation::ImageBlock &block )
{
  if ( mFeedback && mFeedback->isCanceled() )
    return;

  int width = block.image->width();
  int height = block.image->height();
  int sourceBpl = block.image->bytesPerLine();

  unsigned char *outputLineRef = mDestImage->scanLine( block.beginLine );
  QRgb *destRef = nullptr;
  if ( mDirection == ByRow )
  {
    unsigned char *sourceFirstLine = block.image->scanLine( 0 );
    unsigned char *sourceRef;

    //blur along rows
    for ( unsigned int y = block.beginLine; y < block.endLine; ++y, outputLineRef += mDestImageBpl )
    {
      if ( mFeedback && mFeedback->isCanceled() )
        break;

      sourceRef = sourceFirstLine;
      destRef = reinterpret_cast< QRgb * >( outputLineRef );
      for ( int x = 0; x < width; ++x, ++destRef, sourceRef += 4 )
      {
        if ( mFeedback && mFeedback->isCanceled() )
          break;

        *destRef = gaussianBlurVertical( y, sourceRef, sourceBpl, height );
      }
    }
  }
  else
  {
    unsigned char *sourceRef = block.image->scanLine( block.beginLine );
    for ( unsigned int y = block.beginLine; y < block.endLine; ++y, outputLineRef += mDestImageBpl, sourceRef += sourceBpl )
    {
      if ( mFeedback && mFeedback->isCanceled() )
        break;

      destRef = reinterpret_cast< QRgb * >( outputLineRef );
      for ( int x = 0; x < width; ++x, ++destRef )
      {
        if ( mFeedback && mFeedback->isCanceled() )
          break;

        *destRef = gaussianBlurHorizontal( x, sourceRef, width );
      }
    }
  }
}

inline QRgb QgsImageOperation::GaussianBlurOperation::gaussianBlurVertical( const int posy, unsigned char *sourceFirstLine, const int sourceBpl, const int height ) const
{
  double r = 0;
  double b = 0;
  double g = 0;
  double a = 0;
  int y;
  unsigned char *ref;

  for ( int i = 0; i <= mRadius * 2; ++i )
  {
    y = std::clamp( posy + ( i - mRadius ), 0, height - 1 );
    ref = sourceFirstLine + sourceBpl * y;

    QRgb *refRgb = reinterpret_cast< QRgb * >( ref );
    r += mKernel[i] * qRed( *refRgb );
    g += mKernel[i] * qGreen( *refRgb );
    b += mKernel[i] * qBlue( *refRgb );
    a += mKernel[i] * qAlpha( *refRgb );
  }

  return qRgba( r, g, b, a );
}

inline QRgb QgsImageOperation::GaussianBlurOperation::gaussianBlurHorizontal( const int posx, unsigned char *sourceFirstLine, const int width ) const
{
  double r = 0;
  double b = 0;
  double g = 0;
  double a = 0;
  int x;
  unsigned char *ref;

  for ( int i = 0; i <= mRadius * 2; ++i )
  {
    x = std::clamp( posx + ( i - mRadius ), 0, width - 1 );
    ref = sourceFirstLine + x * 4;

    QRgb *refRgb = reinterpret_cast< QRgb * >( ref );
    r += mKernel[i] * qRed( *refRgb );
    g += mKernel[i] * qGreen( *refRgb );
    b += mKernel[i] * qBlue( *refRgb );
    a += mKernel[i] * qAlpha( *refRgb );
  }

  return qRgba( r, g, b, a );
}


double *QgsImageOperation::createGaussianKernel( const int radius )
{
  double *kernel = new double[ radius * 2 + 1 ];
  double sigma = radius / 3.0;
  double twoSigmaSquared = 2 * sigma * sigma;
  double coefficient = 1.0 / std::sqrt( M_PI * twoSigmaSquared );
  double expCoefficient = -1.0 / twoSigmaSquared;

  double sum = 0;
  double result;
  for ( int i = 0; i <= radius; ++i )
  {
    result = coefficient * std::exp( i * i * expCoefficient );
    kernel[ radius - i ] = result;
    sum += result;
    if ( i > 0 )
    {
      kernel[radius + i] = result;
      sum += result;
    }
  }
  //normalize
  for ( int i = 0; i <= radius * 2; ++i )
  {
    kernel[i] /= sum;
  }
  return kernel;
}


// flip

void QgsImageOperation::flipImage( QImage &image, QgsImageOperation::FlipType type )
{
  image.detach();
  FlipLineOperation flipOperation( type == QgsImageOperation::FlipHorizontal ? QgsImageOperation::ByRow : QgsImageOperation::ByColumn );
  runLineOperation( image, flipOperation );
}

QRect QgsImageOperation::nonTransparentImageRect( const QImage &image, QSize minSize, bool center )
{
  int width = image.width();
  int height = image.height();
  int xmin = width;
  int xmax = 0;
  int ymin = height;
  int ymax = 0;

  // scan down till we hit something
  for ( int y = 0; y < height; ++y )
  {
    bool found = false;
    const QRgb *imgScanline = reinterpret_cast< const QRgb * >( image.constScanLine( y ) );
    for ( int x = 0; x < width; ++x )
    {
      if ( qAlpha( imgScanline[x] ) )
      {
        ymin = y;
        ymax = y;
        xmin = x;
        xmax = x;
        found = true;
        break;
      }
    }
    if ( found )
      break;
  }

  //scan up till we hit something
  for ( int y = height - 1; y >= ymin; --y )
  {
    bool found = false;
    const QRgb *imgScanline = reinterpret_cast< const QRgb * >( image.constScanLine( y ) );
    for ( int x = 0; x < width; ++x )
    {
      if ( qAlpha( imgScanline[x] ) )
      {
        ymax = y;
        xmin = std::min( xmin, x );
        xmax = std::max( xmax, x );
        found = true;
        break;
      }
    }
    if ( found )
      break;
  }

  //scan left to right till we hit something, using a refined y region
  for ( int y = ymin; y <= ymax; ++y )
  {
    const QRgb *imgScanline = reinterpret_cast< const QRgb * >( image.constScanLine( y ) );
    for ( int x = 0; x < xmin; ++x )
    {
      if ( qAlpha( imgScanline[x] ) )
      {
        xmin = x;
        break;
      }
    }
  }

  //scan right to left till we hit something, using the refined y region
  for ( int y = ymin; y <= ymax; ++y )
  {
    const QRgb *imgScanline = reinterpret_cast< const QRgb * >( image.constScanLine( y ) );
    for ( int x = width - 1; x > xmax; --x )
    {
      if ( qAlpha( imgScanline[x] ) )
      {
        xmax = x;
        break;
      }
    }
  }

  if ( minSize.isValid() )
  {
    if ( xmax - xmin < minSize.width() ) // centers image on x
    {
      xmin = std::max( ( xmax + xmin ) / 2 - minSize.width() / 2, 0 );
      xmax = xmin + minSize.width();
    }
    if ( ymax - ymin < minSize.height() ) // centers image on y
    {
      ymin = std::max( ( ymax + ymin ) / 2 - minSize.height() / 2, 0 );
      ymax = ymin + minSize.height();
    }
  }
  if ( center )
  {
    // recompute min and max to center image
    const int dx = std::max( std::abs( xmax - width / 2 ), std::abs( xmin - width / 2 ) );
    const int dy = std::max( std::abs( ymax - height / 2 ), std::abs( ymin - height / 2 ) );
    xmin = std::max( 0, width / 2 - dx );
    xmax = std::min( width, width / 2 + dx );
    ymin = std::max( 0, height / 2 - dy );
    ymax = std::min( height, height / 2 + dy );
  }

  return QRect( xmin, ymin, xmax - xmin, ymax - ymin );
}

QImage QgsImageOperation::cropTransparent( const QImage &image, QSize minSize, bool center )
{
  return image.copy( QgsImageOperation::nonTransparentImageRect( image, minSize, center ) );
}

void QgsImageOperation::FlipLineOperation::operator()( QRgb *startRef, const int lineLength, const int bytesPerLine ) const
{
  int increment = ( mDirection == QgsImageOperation::ByRow ) ? 4 : bytesPerLine;

  //store temporary line
  unsigned char *p = reinterpret_cast< unsigned char * >( startRef );
  unsigned char *tempLine = new unsigned char[ lineLength * 4 ];
  for ( int i = 0; i < lineLength * 4; ++i, p += increment )
  {
    tempLine[i++] = *( p++ );
    tempLine[i++] = *( p++ );
    tempLine[i++] = *( p++ );
    tempLine[i] = *( p );
    p -= 3;
  }

  //write values back in reverse order
  p = reinterpret_cast< unsigned char * >( startRef );
  for ( int i = ( lineLength - 1 ) * 4; i >= 0; i -= 7, p += increment )
  {
    *( p++ ) = tempLine[i++];
    *( p++ ) = tempLine[i++];
    *( p++ ) = tempLine[i++];
    *( p ) = tempLine[i];
    p -= 3;
  }

  delete[] tempLine;
}
