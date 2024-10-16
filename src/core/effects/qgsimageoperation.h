/***************************************************************************
                           qgsimageoperation.h
                           --------------------
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

#ifndef QGSIMAGEOPERATION_H
#define QGSIMAGEOPERATION_H

#include <QImage>
#include "qgis_sip.h"
#include <QColor>

#include "qgis_core.h"
#include "qgsfeedback.h"
#include <cmath>

class QgsColorRamp;
class QgsFeedback;

/**
 * \ingroup core
 * \class QgsImageOperation
 * \brief Contains operations and filters which apply to QImages
 *
 * A set of optimised pixel manipulation operations and filters which can be applied
 * to QImages. All operations only apply to ARGB32 format images, and it is left up
 * to the calling procedure to ensure that any passed images are of the correct
 * format.
 *
 * Operations are written to either modify an image in place or return a new image, depending
 * on which is faster for the particular operation.
 *
 * \since QGIS 2.7
 */
class CORE_EXPORT QgsImageOperation
{

  public:

    /**
     * Modes for converting a QImage to grayscale
     */
    enum GrayscaleMode
    {
      GrayscaleLightness, //!< Keep the lightness of the color, drops the saturation
      GrayscaleLuminosity, //!< Grayscale by perceptual luminosity (weighted sum of color RGB components)
      GrayscaleAverage, //!< Grayscale by taking average of color RGB components
      GrayscaleOff //!< No change
    };

    /**
     * Flip operation types
     */
    enum FlipType
    {
      FlipHorizontal, //!< Flip the image horizontally
      FlipVertical //!< Flip the image vertically
    };

    /**
     * Convert a QImage to a grayscale image. Alpha channel is preserved.
     * \param image QImage to convert
     * \param mode mode to use during grayscale conversion
     * \param feedback optional feedback object for responsive cancellation (since QGIS 3.22)
     */
    static void convertToGrayscale( QImage &image, GrayscaleMode mode = GrayscaleLuminosity, QgsFeedback *feedback = nullptr );

    /**
     * Alter the brightness or contrast of a QImage.
     * \param image QImage to alter
     * \param brightness brightness value, in the range -255 to 255. A brightness value of 0 indicates
     * no change to brightness, a negative value will darken the image, and a positive value will brighten
     * the image.
     * \param contrast contrast value. Must be a positive or zero value. A value of 1.0 indicates no change
     * to the contrast, a value of 0 represents an image with 0 contrast, and a value > 1.0 will increase the
     * contrast of the image.
     * \param feedback optional feedback object for responsive cancellation (since QGIS 3.22)
     */
    static void adjustBrightnessContrast( QImage &image, int brightness, double contrast, QgsFeedback *feedback = nullptr );

    /**
     * Alter the hue or saturation of a QImage.
     * \param image QImage to alter
     * \param saturation double between 0 and 2 inclusive, where 0 = desaturate and 1.0 = no change
     * \param colorizeColor color to use for colorizing image. Set to an invalid QColor to disable
     * colorization.
     * \param colorizeStrength double between 0 and 1, where 0 = no colorization and 1.0 = full colorization
     * \param feedback optional feedback object for responsive cancellation (since QGIS 3.22)
     */
    static void adjustHueSaturation( QImage &image, double saturation, const QColor &colorizeColor = QColor(),
                                     double colorizeStrength = 1.0, QgsFeedback *feedback = nullptr );

    /**
     * Multiplies opacity of image pixel values by a factor.
     * \param image QImage to alter
     * \param factor factor to multiple pixel's opacity by
     * \param feedback optional feedback object for responsive cancellation (since QGIS 3.22)
     */
    static void multiplyOpacity( QImage &image, double factor, QgsFeedback *feedback = nullptr );

    /**
     * Overlays a color onto an image. This operation retains the alpha channel of the
     * original image, but replaces all image pixel colors with the specified color.
     * \param image QImage to alter
     * \param color color to overlay (any alpha component of the color is ignored)
     */
    static void overlayColor( QImage &image, const QColor &color );

    //! Struct for storing properties of a distance transform operation
    struct DistanceTransformProperties
    {

      /**
       * Set to TRUE to perform the distance transform on transparent pixels
       * in the source image, set to FALSE to perform the distance transform
       * on opaque pixels
       */
      bool shadeExterior = true;

      /**
       * Set to TRUE to automatically calculate the maximum distance in the
       * transform to use as the spread value
       */
      bool useMaxDistance = true;

      /**
       * Maximum distance (in pixels) for the distance transform shading to
       * spread
       */
      double spread = 10.0;

      /**
       * Color ramp to use for shading the distance transform
       */
      QgsColorRamp *ramp = nullptr;
    };

    /**
     * Performs a distance transform on the source image and shades the result
     * using a color ramp.
     * \param image QImage to alter
     * \param properties DistanceTransformProperties object with parameters
     * for the distance transform operation
     * \param feedback optional feedback object for responsive cancellation (since QGIS 3.22)
     */
    static void distanceTransform( QImage &image, const QgsImageOperation::DistanceTransformProperties &properties, QgsFeedback *feedback = nullptr );

    /**
     * Performs a stack blur on an image. Stack blur represents a good balance between
     * speed and blur quality.
     * \param image QImage to blur
     * \param radius blur radius in pixels, maximum value of 16
     * \param alphaOnly set to TRUE to blur only the alpha component of the image
     * \param feedback optional feedback object for responsive cancellation (since QGIS 3.22)
     * \note for fastest operation, ensure the source image is ARGB32_Premultiplied if
     * alphaOnly is set to FALSE, or ARGB32 if alphaOnly is TRUE
     */
    static void stackBlur( QImage &image, int radius, bool alphaOnly = false, QgsFeedback *feedback = nullptr );

    /**
     * Performs a gaussian blur on an image. Gaussian blur is slower but results in a high
     * quality blur.
     * \param image QImage to blur
     * \param radius blur radius in pixels
     * \param feedback optional feedback object for responsive cancellation (since QGIS 3.22)
     * \returns blurred image
     * \note for fastest operation, ensure the source image is ARGB32_Premultiplied
     */
    static QImage *gaussianBlur( QImage &image, int radius, QgsFeedback *feedback = nullptr ) SIP_FACTORY;

    /**
     * Flips an image horizontally or vertically
     * \param image QImage to flip
     * \param type type of flip to perform (horizontal or vertical)
     */
    static void flipImage( QImage &image, FlipType type );

    /**
     * Calculates the non-transparent region of an image.
     * \param image source image
     * \param minSize minimum size for returned region, if desired. If the
     * non-transparent region of the image is smaller than this minimum size,
     * it will be centered in the returned rectangle.
     * \param center return rectangle will be centered on the center of the original image if set to TRUE
     * \see cropTransparent
     * \since QGIS 2.9
     */
    static QRect nonTransparentImageRect( const QImage &image, QSize minSize = QSize(), bool center = false );

    /**
     * Crop any transparent border from around an image.
     * \param image source image
     * \param minSize minimum size for cropped image, if desired. If the
     * cropped image is smaller than the minimum size, it will be centered
     * in the returned image.
     * \param center cropped image will be centered on the center of the original image if set to TRUE
     * \since QGIS 2.9
     */
    static QImage cropTransparent( const QImage &image, QSize minSize = QSize(), bool center = false );

  private:

    //for blocked operations
    enum LineOperationDirection
    {
      ByRow,
      ByColumn
    };
    template <class BlockOperation> static void runBlockOperationInThreads( QImage &image, BlockOperation &operation, LineOperationDirection direction );
    struct ImageBlock
    {
      unsigned int beginLine;
      unsigned int endLine;
      unsigned int lineLength;
      QImage *image = nullptr;
    };

    //for rect operations
    template <typename RectOperation> static void runRectOperation( QImage &image, RectOperation &operation );
    template <class RectOperation> static void runRectOperationOnWholeImage( QImage &image, RectOperation &operation );

    //for per pixel operations
    template <class PixelOperation> static void runPixelOperation( QImage &image, PixelOperation &operation, QgsFeedback *feedback = nullptr );
    template <class PixelOperation> static void runPixelOperationOnWholeImage( QImage &image, PixelOperation &operation, QgsFeedback *feedback = nullptr );
    template <class PixelOperation>
    struct ProcessBlockUsingPixelOperation
    {
      explicit ProcessBlockUsingPixelOperation( PixelOperation &operation, QgsFeedback *feedback )
        : mOperation( operation )
        , mFeedback( feedback )
      { }

      typedef void result_type;

      void operator()( ImageBlock &block )
      {
        for ( unsigned int y = block.beginLine; y < block.endLine; ++y )
        {
          if ( mFeedback && mFeedback->isCanceled() )
            break;

          QRgb *ref = reinterpret_cast< QRgb * >( block.image->scanLine( y ) );
          for ( unsigned int x = 0; x < block.lineLength; ++x )
          {
            mOperation( ref[x], x, y );
          }
        }
      }

      PixelOperation &mOperation;
      QgsFeedback *mFeedback = nullptr;
    };

    //for linear operations
    template <typename LineOperation> static void runLineOperation( QImage &image, LineOperation &operation, QgsFeedback *feedback = nullptr );
    template <class LineOperation> static void runLineOperationOnWholeImage( QImage &image, LineOperation &operation, QgsFeedback *feedback = nullptr );
    template <class LineOperation>
    struct ProcessBlockUsingLineOperation
    {
      explicit ProcessBlockUsingLineOperation( LineOperation &operation )
        : mOperation( operation ) { }

      typedef void result_type;

      void operator()( ImageBlock &block )
      {
        //do something with whole lines
        int bpl = block.image->bytesPerLine();
        if ( mOperation.direction() == ByRow )
        {
          for ( unsigned int y = block.beginLine; y < block.endLine; ++y )
          {
            QRgb *ref = reinterpret_cast< QRgb * >( block.image->scanLine( y ) );
            mOperation( ref, block.lineLength, bpl );
          }
        }
        else
        {
          //by column
          unsigned char *ref = block.image->scanLine( 0 ) + 4 * block.beginLine;
          for ( unsigned int x = block.beginLine; x < block.endLine; ++x, ref += 4 )
          {
            mOperation( reinterpret_cast< QRgb * >( ref ), block.lineLength, bpl );
          }
        }
      }

      LineOperation &mOperation;
    };


    //individual operation implementations

    class GrayscalePixelOperation
    {
      public:
        explicit GrayscalePixelOperation( const GrayscaleMode mode )
          : mMode( mode )
        {  }

        void operator()( QRgb &rgb, int x, int y ) const;

      private:
        GrayscaleMode mMode;
    };
    static void grayscaleLightnessOp( QRgb &rgb );
    static void grayscaleLuminosityOp( QRgb &rgb );
    static void grayscaleAverageOp( QRgb &rgb );


    class BrightnessContrastPixelOperation
    {
      public:
        BrightnessContrastPixelOperation( const int brightness, const double contrast )
          : mBrightness( brightness )
          , mContrast( contrast )
        {  }

        void operator()( QRgb &rgb, int x, int y ) const;

      private:
        int mBrightness;
        double mContrast;
    };


    class HueSaturationPixelOperation
    {
      public:
        HueSaturationPixelOperation( const double saturation, const bool colorize,
                                     const int colorizeHue, const int colorizeSaturation,
                                     const double colorizeStrength )
          : mSaturation( saturation )
          , mColorize( colorize )
          , mColorizeHue( colorizeHue )
          , mColorizeSaturation( colorizeSaturation )
          , mColorizeStrength( colorizeStrength )
        {  }

        void operator()( QRgb &rgb, int x, int y );

      private:
        double mSaturation; // [0, 2], 1 = no change
        bool mColorize;
        int mColorizeHue;
        int mColorizeSaturation;
        double mColorizeStrength; // [0,1]
    };
    static int adjustColorComponent( int colorComponent, int brightness, double contrastFactor );


    class MultiplyOpacityPixelOperation
    {
      public:
        explicit MultiplyOpacityPixelOperation( const double factor )
          : mFactor( factor )
        { }

        void operator()( QRgb &rgb, int x, int y );

      private:
        double mFactor;
    };

    class ConvertToArrayPixelOperation
    {
      public:
        ConvertToArrayPixelOperation( const int width, double *array, const bool exterior = true )
          : mWidth( width )
          , mArray( array )
          , mExterior( exterior )
        {
        }

        void operator()( QRgb &rgb, int x, int y );

      private:
        int mWidth;
        double *mArray = nullptr;
        bool mExterior;
    };

    class ShadeFromArrayOperation
    {
      public:
        ShadeFromArrayOperation( const int width, double *array, const double spread,
                                 const DistanceTransformProperties &properties )
          : mWidth( width )
          , mArray( array )
          , mSpread( spread )
          , mProperties( properties )
        {
          mSpreadSquared = std::pow( mSpread, 2.0 );
        }

        void operator()( QRgb &rgb, int x, int y );

      private:
        int mWidth;
        double *mArray = nullptr;
        double mSpread;
        double mSpreadSquared;
        const DistanceTransformProperties &mProperties;
    };
    static void distanceTransform2d( double *im, int width, int height, QgsFeedback *feedback = nullptr );
    static void distanceTransform1d( double *f, int n, int *v, double *z, double *d );
    static double maxValueInDistanceTransformArray( const double *array, unsigned int size );


    class StackBlurLineOperation
    {
      public:
        StackBlurLineOperation( int alpha, LineOperationDirection direction, bool forwardDirection, int i1, int i2, QgsFeedback *feedback )
          : mAlpha( alpha )
          , mDirection( direction )
          , mForwardDirection( forwardDirection )
          , mi1( i1 )
          , mi2( i2 )
          , mFeedback( feedback )
        { }

        typedef void result_type;

        LineOperationDirection direction() const { return mDirection; }

        void operator()( QRgb *startRef, int lineLength, int bytesPerLine )
        {
          if ( mFeedback && mFeedback->isCanceled() )
            return;

          unsigned char *p = reinterpret_cast< unsigned char * >( startRef );
          int rgba[4];
          int increment = ( mDirection == QgsImageOperation::ByRow ) ? 4 : bytesPerLine;
          if ( !mForwardDirection )
          {
            p += static_cast< std::size_t >( lineLength - 1 ) * increment;
            increment = -increment;
          }

          for ( int i = mi1; i <= mi2; ++i )
          {
            rgba[i] = p[i] << 4;
          }

          p += increment;
          for ( int j = 1; j < lineLength; ++j, p += increment )
          {
            if ( mFeedback && mFeedback->isCanceled() )
              break;

            for ( int i = mi1; i <= mi2; ++i )
            {
              p[i] = ( rgba[i] += ( ( p[i] << 4 ) - rgba[i] ) * mAlpha / 16 ) >> 4;
            }
          }
        }

      private:
        int mAlpha;
        LineOperationDirection mDirection;
        bool mForwardDirection;
        int mi1;
        int mi2;
        QgsFeedback *mFeedback = nullptr;
    };

    static double *createGaussianKernel( int radius );

    class GaussianBlurOperation
    {
      public:
        GaussianBlurOperation( int radius, LineOperationDirection direction, QImage *destImage, double *kernel, QgsFeedback *feedback )
          : mRadius( radius )
          , mDirection( direction )
          , mDestImage( destImage )
          , mDestImageBpl( destImage->bytesPerLine() )
          , mKernel( kernel )
          , mFeedback( feedback )
        {}

        typedef void result_type;

        void operator()( ImageBlock &block );

      private:
        int mRadius;
        LineOperationDirection mDirection;
        QImage *mDestImage = nullptr;
        int mDestImageBpl;
        double *mKernel = nullptr;
        QgsFeedback *mFeedback = nullptr;

        inline QRgb gaussianBlurVertical( int posy, unsigned char *sourceFirstLine, int sourceBpl, int height ) const;
        inline QRgb gaussianBlurHorizontal( int posx, unsigned char *sourceFirstLine, int width ) const;
    };

    //flip


    class FlipLineOperation
    {
      public:
        explicit FlipLineOperation( LineOperationDirection direction )
          : mDirection( direction )
        { }

        typedef void result_type;

        LineOperationDirection direction() const { return mDirection; }

        void operator()( QRgb *startRef, int lineLength, int bytesPerLine ) const;

      private:
        LineOperationDirection mDirection;
    };


};

#endif // QGSIMAGEOPERATION_H

