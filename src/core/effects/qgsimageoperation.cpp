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
#include "qgsvectorcolorrampv2.h"
#include "qgslogger.h"
#include <QtConcurrentMap>
#include <QColor>
#include <QPainter>
#include <qmath.h>

//determined via trial-and-error. Could possibly be optimised, or varied
//depending on the image size.
#define BLOCK_THREADS 16

#define INF 1E20

/// @cond PRIVATE

template <typename PixelOperation>
void QgsImageOperation::runPixelOperation( QImage &image, PixelOperation& operation )
{
  if ( image.height() * image.width() < 100000 )
  {
    //small image, don't multithread
    //this threshold was determined via testing various images
    runPixelOperationOnWholeImage( image, operation );
  }
  else
  {
    //large image, multithread operation
    QgsImageOperation::ProcessBlockUsingPixelOperation<PixelOperation> blockOp( operation ) ;
    runBlockOperationInThreads( image, blockOp, QgsImageOperation::ByRow );
  }
}

template <typename PixelOperation>
void QgsImageOperation::runPixelOperationOnWholeImage( QImage &image, PixelOperation& operation )
{
  int height = image.height();
  int width = image.width();
  for ( int y = 0; y < height; ++y )
  {
    QRgb* ref = reinterpret_cast< QRgb* >( image.scanLine( y ) );
    for ( int x = 0; x < width; ++x )
    {
      operation( ref[x], x, y );
    }
  }
}

//rect operations

template <typename RectOperation>
void QgsImageOperation::runRectOperation( QImage &image, RectOperation& operation )
{
  //possibly could be tweaked for rect operations
  if ( image.height() * image.width() < 100000 )
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
void QgsImageOperation::runRectOperationOnWholeImage( QImage &image, RectOperation& operation )
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
void QgsImageOperation::runLineOperation( QImage &image, LineOperation& operation )
{
  //possibly could be tweaked for rect operations
  if ( image.height() * image.width() < 100000 )
  {
    //small image, don't multithread
    //this threshold was determined via testing various images
    runLineOperationOnWholeImage( image, operation );
  }
  else
  {
    //large image, multithread operation
    QgsImageOperation::ProcessBlockUsingLineOperation<LineOperation> blockOp( operation ) ;
    runBlockOperationInThreads( image, blockOp, operation.direction() );
  }
}

template <class LineOperation>
void QgsImageOperation::runLineOperationOnWholeImage( QImage &image, LineOperation& operation )
{
  int height = image.height();
  int width = image.width();

  //do something with whole lines
  int bpl = image.bytesPerLine();
  if ( operation.direction() == ByRow )
  {
    for ( int y = 0; y < height; ++y )
    {
      QRgb* ref = reinterpret_cast< QRgb* >( image.scanLine( y ) );
      operation( ref, width, bpl );
    }
  }
  else
  {
    //by column
    unsigned char* ref = image.scanLine( 0 );
    for ( int x = 0; x < width; ++x, ref += 4 )
    {
      operation( reinterpret_cast< QRgb* >( ref ), height, bpl );
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

void QgsImageOperation::convertToGrayscale( QImage &image, const GrayscaleMode mode )
{
  if ( mode == GrayscaleOff )
  {
    return;
  }

  GrayscalePixelOperation operation( mode );
  runPixelOperation( image, operation );
}

void QgsImageOperation::GrayscalePixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  Q_UNUSED( x );
  Q_UNUSED( y );
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

  int min = qMin( qMin( red, green ), blue );
  int max = qMax( qMax( red, green ), blue );

  int lightness = qMin(( min + max ) / 2, 255 );
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

void QgsImageOperation::adjustBrightnessContrast( QImage &image, const int brightness, const double contrast )
{
  BrightnessContrastPixelOperation operation( brightness, contrast );
  runPixelOperation( image, operation );
}

void QgsImageOperation::BrightnessContrastPixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  Q_UNUSED( x );
  Q_UNUSED( y );
  int red = adjustColorComponent( qRed( rgb ), mBrightness, mContrast );
  int blue = adjustColorComponent( qBlue( rgb ), mBrightness, mContrast );
  int green = adjustColorComponent( qGreen( rgb ), mBrightness, mContrast );
  rgb = qRgba( red, green, blue, qAlpha( rgb ) );
}

int QgsImageOperation::adjustColorComponent( int colorComponent, int brightness, double contrastFactor )
{
  return qBound( 0, static_cast< int >(((((( colorComponent / 255.0 ) - 0.5 ) * contrastFactor ) + 0.5 ) * 255 ) + brightness ), 255 );
}

//hue/saturation

void QgsImageOperation::adjustHueSaturation( QImage &image, const double saturation, const QColor &colorizeColor, const double colorizeStrength )
{
  HueSaturationPixelOperation operation( saturation, colorizeColor.isValid() && colorizeStrength > 0.0,
                                         colorizeColor.hue(), colorizeColor.saturation(), colorizeStrength );
  runPixelOperation( image, operation );
}

void QgsImageOperation::HueSaturationPixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  Q_UNUSED( x );
  Q_UNUSED( y );
  QColor tmpColor( rgb );
  int h, s, l;
  tmpColor.getHsl( &h, &s, &l );

  if ( mSaturation < 1.0 )
  {
    // Lowering the saturation. Use a simple linear relationship
    s = qMin( static_cast< int >( s * mSaturation ), 255 );
  }
  else if ( mSaturation > 1.0 )
  {
    // Raising the saturation. Use a saturation curve to prevent
    // clipping at maximum saturation with ugly results.
    s = qMin( static_cast< int >( 255. * ( 1 - qPow( 1 - ( s / 255. ), qPow( mSaturation, 2 ) ) ) ), 255 );
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

void QgsImageOperation::multiplyOpacity( QImage &image, const double factor )
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
    QPainter painter( &image );
    painter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    painter.fillRect( 0, 0, image.width(), image.height(), transparentFillColor );
    painter.end();
  }
  else
  {
    //increasing opacity - run this as a pixel operation for multithreading
    MultiplyOpacityPixelOperation operation( factor );
    runPixelOperation( image, operation );
  }
}

void QgsImageOperation::MultiplyOpacityPixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  Q_UNUSED( x );
  Q_UNUSED( y );
  rgb = qRgba( qRed( rgb ), qGreen( rgb ), qBlue( rgb ), qBound( 0, qRound( mFactor * qAlpha( rgb ) ), 255 ) );
}

// overlay color

void QgsImageOperation::overlayColor( QImage &image, const QColor &color )
{
  QColor opaqueColor = color;
  opaqueColor.setAlpha( 255 );

  //use QPainter SourceIn composition mode to overlay color (fast)
  //this retains image's alpha channel but replaces color
  QPainter painter( &image );
  painter.setCompositionMode( QPainter::CompositionMode_SourceIn );
  painter.fillRect( 0, 0, image.width(), image.height(), opaqueColor );
  painter.end();
}

// distance transform

void QgsImageOperation::distanceTransform( QImage &image, const DistanceTransformProperties& properties )
{
  if ( ! properties.ramp )
  {
    QgsDebugMsg( QString( "no color ramp specified for distance transform" ) );
    return;
  }

  //first convert to 1 bit alpha mask array
  double * array = new double[ image.width() * image.height()];
  ConvertToArrayPixelOperation convertToArray( image.width(), array, properties.shadeExterior );
  runPixelOperation( image, convertToArray );

  //calculate distance transform (single threaded only)
  distanceTransform2d( array, image.width(), image.height() );

  double spread;
  if ( properties.useMaxDistance )
  {
    spread = sqrt( maxValueInDistanceTransformArray( array, image.width() * image.height() ) );
  }
  else
  {
    spread = properties.spread;
  }

  //shade distance transform
  ShadeFromArrayOperation shadeFromArray( image.width(), array, spread, properties );
  runPixelOperation( image, shadeFromArray );
  delete [] array;
}

void QgsImageOperation::ConvertToArrayPixelOperation::operator()( QRgb &rgb, const int x, const int y )
{
  int idx = y * mWidth + x;
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
    double s  = (( f[q] + q * q ) - ( f[v[k]] + ( v[k] * v[k] ) ) ) / ( 2 * q - 2 * v[k] );
    while ( s <= z[k] )
    {
      k--;
      s  = (( f[q] + q * q ) - ( f[v[k]] + ( v[k] * v[k] ) ) ) / ( 2 * q - 2 * v[k] );
    }
    k++;
    v[k] = q;
    z[k] = s;
    z[k+1] = + INF;
  }

  k = 0;
  for ( int q = 0; q <= n - 1; q++ )
  {
    while ( z[k+1] < q )
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
void QgsImageOperation::distanceTransform2d( double * im, int width, int height )
{
  int maxDimension = qMax( width, height );

  double *f = new double[ maxDimension ];
  int *v = new int[ maxDimension ];
  double *z = new double[ maxDimension + 1 ];
  double *d = new double[ maxDimension ];

  // transform along columns
  for ( int x = 0; x < width; x++ )
  {
    for ( int y = 0; y < height; y++ )
    {
      f[y] = im[ x + y * width ];
    }
    distanceTransform1d( f, height, v, z, d );
    for ( int y = 0; y < height; y++ )
    {
      im[ x + y * width ] = d[y];
    }
  }

  // transform along rows
  for ( int y = 0; y < height; y++ )
  {
    for ( int x = 0; x < width; x++ )
    {
      f[x] = im[  x + y*width ];
    }
    distanceTransform1d( f, width, v, z, d );
    for ( int x = 0; x < width; x++ )
    {
      im[  x + y*width ] = d[x];
    }
  }

  delete [] d;
  delete [] f;
  delete [] v;
  delete [] z;
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

  double distance = sqrt( squaredVal );
  double val = distance / mSpread;
  QColor rampColor = mProperties.ramp->color( val );

  if (( mProperties.shadeExterior && distance > mSpread - 1 ) )
  {
    //fade off final pixel to antialias edge
    double alphaMultiplyFactor = mSpread - distance;
    rampColor.setAlpha( rampColor.alpha() * alphaMultiplyFactor );
  }
  rgb = rampColor.rgba();
}

//stack blur

void QgsImageOperation::stackBlur( QImage &image, const int radius, const bool alphaOnly )
{
  // culled from Qt's qpixmapfilter.cpp, see: http://www.qtcentre.org/archive/index.php/t-26534.html
  int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
  int alpha = ( radius < 1 )  ? 16 : ( radius > 17 ) ? 1 : tab[radius-1];

  int i1 = 0;
  int i2 = 3;

  //ensure correct source format.
  QImage::Format originalFormat = image.format();
  QImage* pImage = &image;
  if ( !alphaOnly && originalFormat != QImage::Format_ARGB32_Premultiplied )
  {
    pImage = new QImage( image.convertToFormat( QImage::Format_ARGB32_Premultiplied ) );
  }
  else if ( alphaOnly && originalFormat != QImage::Format_ARGB32 )
  {
    pImage = new QImage( image.convertToFormat( QImage::Format_ARGB32 ) );
  }

  if ( alphaOnly )
    i1 = i2 = ( QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3 );

  StackBlurLineOperation topToBottomBlur( alpha, QgsImageOperation::ByColumn, true, i1, i2 );
  runLineOperation( *pImage, topToBottomBlur );

  StackBlurLineOperation leftToRightBlur( alpha, QgsImageOperation::ByRow, true, i1, i2 );
  runLineOperation( *pImage, leftToRightBlur );

  StackBlurLineOperation bottomToTopBlur( alpha, QgsImageOperation::ByColumn, false, i1, i2 );
  runLineOperation( *pImage, bottomToTopBlur );

  StackBlurLineOperation rightToLeftBlur( alpha, QgsImageOperation::ByRow, false, i1, i2 );
  runLineOperation( *pImage, rightToLeftBlur );

  if ( pImage->format() != originalFormat )
  {
    image = pImage->convertToFormat( originalFormat );
    delete pImage;
  }
}

void QgsImageOperation::StackBlurLineOperation::operator()( QRgb* startRef, const int lineLength, const int bytesPerLine )
{
  unsigned char* p = reinterpret_cast< unsigned char* >( startRef );
  int rgba[4];
  int increment = ( mDirection == QgsImageOperation::ByRow ) ? 4 : bytesPerLine;
  if ( !mForwardDirection )
  {
    p += ( lineLength - 1 ) * increment;
    increment = -increment;
  }

  for ( int i = mi1; i <= mi2; ++i )
  {
    rgba[i] = p[i] << 4;
  }

  p += increment;
  for ( int j = 1; j < lineLength; ++j, p += increment )
  {
    for ( int i = mi1; i <= mi2; ++i )
    {
      p[i] = ( rgba[i] += (( p[i] << 4 ) - rgba[i] ) * mAlpha / 16 ) >> 4;
    }
  }
}

//gaussian blur

QImage *QgsImageOperation::gaussianBlur( QImage &image, const int radius )
{
  int width = image.width();
  int height = image.height();

  if ( radius <= 0 )
  {
    //just make an unchanged copy
    QImage* copy = new QImage( image.copy() );
    return copy;
  }

  double* kernel = createGaussianKernel( radius );

  //ensure correct source format.
  QImage::Format originalFormat = image.format();
  QImage* pImage = &image;
  if ( originalFormat != QImage::Format_ARGB32_Premultiplied )
  {
    pImage = new QImage( image.convertToFormat( QImage::Format_ARGB32_Premultiplied ) );
  }

  //blur along rows
  QImage xBlurImage = QImage( width, height, QImage::Format_ARGB32_Premultiplied );
  GaussianBlurOperation rowBlur( radius, QgsImageOperation::ByRow, &xBlurImage, kernel );
  runRectOperation( *pImage, rowBlur );

  //blur along columns
  QImage* yBlurImage = new QImage( width, height, QImage::Format_ARGB32_Premultiplied );
  GaussianBlurOperation colBlur( radius, QgsImageOperation::ByColumn, yBlurImage, kernel );
  runRectOperation( xBlurImage, colBlur );

  delete[] kernel;

  if ( originalFormat != QImage::Format_ARGB32_Premultiplied )
  {
    QImage* convertedImage = new QImage( yBlurImage->convertToFormat( originalFormat ) );
    delete yBlurImage;
    delete pImage;
    return convertedImage;
  }

  return yBlurImage;
}

void QgsImageOperation::GaussianBlurOperation::operator()( QgsImageOperation::ImageBlock &block )
{
  int width = block.image->width();
  int height = block.image->height();
  int sourceBpl = block.image->bytesPerLine();

  unsigned char* outputLineRef = mDestImage->scanLine( block.beginLine );
  QRgb* destRef = nullptr;
  if ( mDirection == ByRow )
  {
    unsigned char* sourceFirstLine = block.image->scanLine( 0 );
    unsigned char* sourceRef;

    //blur along rows
    for ( unsigned int y = block.beginLine; y < block.endLine; ++y, outputLineRef += mDestImageBpl )
    {
      sourceRef = sourceFirstLine;
      destRef = reinterpret_cast< QRgb* >( outputLineRef );
      for ( int x = 0; x < width; ++x, ++destRef, sourceRef += 4 )
      {
        *destRef = gaussianBlurVertical( y, sourceRef, sourceBpl, height );
      }
    }
  }
  else
  {
    unsigned char* sourceRef = block.image->scanLine( block.beginLine );
    for ( unsigned int y = block.beginLine; y < block.endLine; ++y, outputLineRef += mDestImageBpl, sourceRef += sourceBpl )
    {
      destRef = reinterpret_cast< QRgb* >( outputLineRef );
      for ( int x = 0; x < width; ++x, ++destRef )
      {
        *destRef = gaussianBlurHorizontal( x, sourceRef, width );
      }
    }
  }
}

inline QRgb QgsImageOperation::GaussianBlurOperation::gaussianBlurVertical( const int posy, unsigned char *sourceFirstLine, const int sourceBpl, const int height )
{
  double r = 0;
  double b = 0;
  double g = 0;
  double a = 0;
  int y;
  unsigned char *ref;

  for ( int i = 0; i <= mRadius*2; ++i )
  {
    y = qBound( 0, posy + ( i - mRadius ), height - 1 );
    ref = sourceFirstLine + sourceBpl * y;

    QRgb* refRgb = reinterpret_cast< QRgb* >( ref );
    r += mKernel[i] * qRed( *refRgb );
    g += mKernel[i] * qGreen( *refRgb );
    b += mKernel[i] * qBlue( *refRgb );
    a += mKernel[i] * qAlpha( *refRgb );
  }

  return qRgba( r, g, b, a );
}

inline QRgb QgsImageOperation::GaussianBlurOperation::gaussianBlurHorizontal( const int posx, unsigned char *sourceFirstLine, const int width )
{
  double r = 0;
  double b = 0;
  double g = 0;
  double a = 0;
  int x;
  unsigned char *ref;

  for ( int i = 0; i <= mRadius*2; ++i )
  {
    x = qBound( 0, posx + ( i - mRadius ), width - 1 );
    ref = sourceFirstLine + x * 4;

    QRgb* refRgb = reinterpret_cast< QRgb* >( ref );
    r += mKernel[i] * qRed( *refRgb );
    g += mKernel[i] * qGreen( *refRgb );
    b += mKernel[i] * qBlue( *refRgb );
    a += mKernel[i] * qAlpha( *refRgb );
  }

  return qRgba( r, g, b, a );
}


double* QgsImageOperation::createGaussianKernel( const int radius )
{
  double* kernel = new double[ radius*2+1 ];
  double sigma = radius / 3.0;
  double twoSigmaSquared = 2 * sigma * sigma;
  double coefficient = 1.0 / sqrt( M_PI * twoSigmaSquared );
  double expCoefficient = -1.0 / twoSigmaSquared;

  double sum = 0;
  double result;
  for ( int i = 0; i <= radius; ++i )
  {
    result = coefficient * exp( i * i * expCoefficient );
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

  for ( int x = 0; x < width; ++x )
  {
    for ( int y = 0; y < height; ++y )
    {
      if ( qAlpha( image.pixel( x, y ) ) )
      {
        xmin = qMin( x, xmin );
        xmax = qMax( x, xmax );
        ymin = qMin( y, ymin );
        ymax = qMax( y, ymax );
      }
    }
  }
  if ( minSize.isValid() )
  {
    if ( xmax - xmin < minSize.width() ) // centers image on x
    {
      xmin = qMax(( xmax + xmin ) / 2 - minSize.width() / 2, 0 );
      xmax = xmin + minSize.width();
    }
    if ( ymax - ymin < minSize.height() ) // centers image on y
    {
      ymin = qMax(( ymax + ymin ) / 2 - minSize.height() / 2, 0 );
      ymax = ymin + minSize.height();
    }
  }
  if ( center )
  {
    // recompute min and max to center image
    const int dx = qMax( qAbs( xmax - width / 2 ), qAbs( xmin - width / 2 ) );
    const int dy = qMax( qAbs( ymax - height / 2 ), qAbs( ymin - height / 2 ) );
    xmin = qMax( 0, width / 2 - dx );
    xmax = qMin( width, width / 2 + dx );
    ymin = qMax( 0, height / 2 - dy );
    ymax = qMin( height, height / 2 + dy );
  }

  return QRect( xmin, ymin, xmax - xmin, ymax - ymin );
}

QImage QgsImageOperation::cropTransparent( const QImage &image, QSize minSize, bool center )
{
  return image.copy( QgsImageOperation::nonTransparentImageRect( image, minSize, center ) );
}

void QgsImageOperation::FlipLineOperation::operator()( QRgb *startRef, const int lineLength, const int bytesPerLine )
{
  int increment = ( mDirection == QgsImageOperation::ByRow ) ? 4 : bytesPerLine;

  //store temporary line
  unsigned char* p = reinterpret_cast< unsigned char* >( startRef );
  unsigned char* tempLine = new unsigned char[ lineLength * 4 ];
  for ( int i = 0; i < lineLength * 4; ++i, p += increment )
  {
    tempLine[i++] = *( p++ );
    tempLine[i++] = *( p++ );
    tempLine[i++] = *( p++ );
    tempLine[i] = *( p );
    p -= 3;
  }

  //write values back in reverse order
  p = reinterpret_cast< unsigned char* >( startRef );
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




