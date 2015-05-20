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
#include <QColor>
#include <QtCore/qmath.h>

class QgsVectorColorRampV2;

/** \ingroup core
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
 * \note Added in version 2.7
 */
class CORE_EXPORT QgsImageOperation
{

  public:

    /** Modes for converting a QImage to grayscale
    */
    enum GrayscaleMode
    {
      GrayscaleLightness, /*!< keep the lightness of the color, drops the saturation */
      GrayscaleLuminosity, /*!< grayscale by perceptual luminosity (weighted sum of color RGB components) */
      GrayscaleAverage, /*!< grayscale by taking average of color RGB components */
      GrayscaleOff /*!< no change */
    };

    /** Flip operation types
    */
    enum FlipType
    {
      FlipHorizontal, /*!< flip the image horizontally */
      FlipVertical /*!< flip the image vertically */
    };

    /**Convert a QImage to a grayscale image. Alpha channel is preserved.
     * @param image QImage to convert
     * @param mode mode to use during grayscale conversion
    */
    static void convertToGrayscale( QImage &image, const GrayscaleMode mode = GrayscaleLuminosity );

    /**Alter the brightness or contrast of a QImage.
     * @param image QImage to alter
     * @param brightness brightness value, in the range -255 to 255. A brightness value of 0 indicates
     * no change to brightness, a negative value will darken the image, and a positive value will brighten
     * the image.
     * @param contrast contrast value. Must be a positive or zero value. A value of 1.0 indicates no change
     * to the contrast, a value of 0 represents an image with 0 contrast, and a value > 1.0 will increase the
     * contrast of the image.
     */
    static void adjustBrightnessContrast( QImage &image, const int brightness, const double contrast );

    /**Alter the hue or saturation of a QImage.
     * @param image QImage to alter
     * @param saturation double between 0 and 2 inclusive, where 0 = desaturate and 1.0 = no change
     * @param colorizeColor color to use for colorizing image. Set to an invalid QColor to disable
     * colorization.
     * @param colorizeStrength double between 0 and 1, where 0 = no colorization and 1.0 = full colorization
     */
    static void adjustHueSaturation( QImage &image, const double saturation, const QColor& colorizeColor = QColor(),
                                     const double colorizeStrength = 1.0 );

    /**Multiplies opacity of image pixel values by a factor.
     * @param image QImage to alter
     * @param factor factor to multiple pixel's opacity by
     */
    static void multiplyOpacity( QImage &image, const double factor );

    /**Overlays a color onto an image. This operation retains the alpha channel of the
     * original image, but replaces all image pixel colors with the specified color.
     * @param image QImage to alter
     * @param color color to overlay (any alpha component of the color is ignored)
     */
    static void overlayColor( QImage &image, const QColor& color );

    /**Struct for storing properties of a distance transform operation*/
    struct DistanceTransformProperties
    {
      DistanceTransformProperties()
          : shadeExterior( true )
          , useMaxDistance( true )
          , spread( 10.0 )
          , ramp( NULL )
      { }

      /**Set to true to perform the distance transform on transparent pixels
       * in the source image, set to false to perform the distance transform
       * on opaque pixels
       */
      bool shadeExterior;
      /**Set to true to automatically calculate the maximum distance in the
       * transform to use as the spread value
       */
      bool useMaxDistance;
      /**Maximum distance (in pixels) for the distance transform shading to
       * spread
       */
      double spread;
      /**Color ramp to use for shading the distance transform
       */
      QgsVectorColorRampV2* ramp;
    };

    /**Performs a distance transform on the source image and shades the result
     * using a color ramp.
     * @param image QImage to alter
     * @param properties DistanceTransformProperties object with parameters
     * for the distance transform operation
     */
    static void distanceTransform( QImage &image, const DistanceTransformProperties& properties );

    /**Performs a stack blur on an image. Stack blur represents a good balance between
     * speed and blur quality.
     * @param image QImage to blur
     * @param radius blur radius in pixels, maximum value of 16
     * @param alphaOnly set to true to blur only the alpha component of the image
     * @note for fastest operation, ensure the source image is ARGB32_Premultiplied if
     * alphaOnly is set to false, or ARGB32 if alphaOnly is true
     */
    static void stackBlur( QImage &image, const int radius, const bool alphaOnly = false );

    /**Performs a gaussian blur on an image. Gaussian blur is slower but results in a high
     * quality blur.
     * @param image QImage to blur
     * @param radius blur radius in pixels
     * @returns blurred image
     * @note for fastest operation, ensure the source image is ARGB32_Premultiplied
     */
    static QImage* gaussianBlur( QImage &image, const int radius );

    /**Flips an image horizontally or vertically
     * @param image QImage to flip
     * @param type type of flip to perform (horizontal or vertical)
     */
    static void flipImage( QImage &image, FlipType type );

    /** Calculates the non-transparent region of an image.
     * @param image source image
     * @param minSize minimum size for returned region, if desired. If the
     * non-transparent region of the image is smaller than this minimum size,
     * it will be centered in the returned rectangle.
     * @param center return rectangle will be centered on the center of the original image if set to true
     * @note added in QGIS 2.9
     * @see cropTransparent
     */
    static QRect nonTransparentImageRect( const QImage & image, const QSize& minSize = QSize(), bool center = false );

    /** Crop any transparent border from around an image.
     * @param image source image
     * @param minSize minimum size for cropped image, if desired. If the
     * cropped image is smaller than the minimum size, it will be centered
     * in the returned image.
     * @param center cropped image will be centered on the center of the original image if set to true
     * @note added in QGIS 2.9
     */
    static QImage cropTransparent( const QImage & image, const QSize& minSize = QSize(), bool center = false );

  private:

    //for blocked operations
    enum LineOperationDirection
    {
      ByRow,
      ByColumn
    };
    template <class BlockOperation> static void runBlockOperationInThreads( QImage &image, BlockOperation& operation, LineOperationDirection direction );
    struct ImageBlock
    {
      unsigned int beginLine;
      unsigned int endLine;
      unsigned int lineLength;
      QImage* image;
    };

    //for rect operations
    template <typename RectOperation> static void runRectOperation( QImage &image, RectOperation& operation );
    template <class RectOperation> static void runRectOperationOnWholeImage( QImage &image, RectOperation& operation );

    //for per pixel operations
    template <class PixelOperation> static void runPixelOperation( QImage &image, PixelOperation& operation );
    template <class PixelOperation> static void runPixelOperationOnWholeImage( QImage &image, PixelOperation& operation );
    template <class PixelOperation>
    struct ProcessBlockUsingPixelOperation
    {
      ProcessBlockUsingPixelOperation( PixelOperation& operation )
          : mOperation( operation ) { }

      typedef void result_type;

      void operator()( ImageBlock& block )
      {
        for ( unsigned int y = block.beginLine; y < block.endLine; ++y )
        {
          QRgb* ref = ( QRgb* )block.image->scanLine( y );
          for ( unsigned int x = 0; x < block.lineLength; ++x )
          {
            mOperation( ref[x], x, y );
          }
        }
      }

      PixelOperation& mOperation;
    };

    //for linear operations
    template <typename LineOperation> static void runLineOperation( QImage &image, LineOperation& operation );
    template <class LineOperation> static void runLineOperationOnWholeImage( QImage &image, LineOperation& operation );
    template <class LineOperation>
    struct ProcessBlockUsingLineOperation
    {
      ProcessBlockUsingLineOperation( LineOperation& operation )
          : mOperation( operation ) { }

      typedef void result_type;

      void operator()( ImageBlock& block )
      {
        //do something with whole lines
        int bpl = block.image->bytesPerLine();
        if ( mOperation.direction() == ByRow )
        {
          for ( unsigned int y = block.beginLine; y < block.endLine; ++y )
          {
            QRgb* ref = ( QRgb* )block.image->scanLine( y );
            mOperation( ref, block.lineLength, bpl );
          }
        }
        else
        {
          //by column
          unsigned char* ref = block.image->scanLine( 0 ) + 4 * block.beginLine;
          for ( unsigned int x = block.beginLine; x < block.endLine; ++x, ref += 4 )
          {
            mOperation(( QRgb* )ref, block.lineLength, bpl );
          }
        }
      }

      LineOperation& mOperation;
    };


    //individual operation implementations

    class GrayscalePixelOperation
    {
      public:
        GrayscalePixelOperation( const GrayscaleMode mode )
            : mMode( mode )
        {  }

        void operator()( QRgb& rgb, const int x, const int y );

      private:
        GrayscaleMode mMode;
    };
    static void grayscaleLightnessOp( QRgb& rgb );
    static void grayscaleLuminosityOp( QRgb& rgb );
    static void grayscaleAverageOp( QRgb& rgb );


    class BrightnessContrastPixelOperation
    {
      public:
        BrightnessContrastPixelOperation( const int brightness, const double contrast )
            : mBrightness( brightness )
            , mContrast( contrast )
        {  }

        void operator()( QRgb& rgb, const int x, const int y );

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

        void operator()( QRgb& rgb, const int x, const int y );

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
        MultiplyOpacityPixelOperation( const double factor )
            : mFactor( factor )
        { }

        void operator()( QRgb& rgb, const int x, const int y );

      private:
        double mFactor;
    };

    class ConvertToArrayPixelOperation
    {
      public:
        ConvertToArrayPixelOperation( const int width, double * array, const bool exterior = true )
            : mWidth( width )
            , mArray( array )
            , mExterior( exterior )
        {
        }

        void operator()( QRgb& rgb, const int x, const int y );

      private:
        int mWidth;
        double * mArray;
        bool mExterior;
    };

    class ShadeFromArrayOperation
    {
      public:
        ShadeFromArrayOperation( const int width, double* array, const double spread,
                                 const DistanceTransformProperties& properties )
            : mWidth( width )
            , mArray( array )
            , mSpread( spread )
            , mProperties( properties )
        {
          mSpreadSquared = qPow( mSpread, 2.0 );
        }

        void operator()( QRgb& rgb, const int x, const int y );

      private:
        int mWidth;
        double * mArray;
        double mSpread;
        double mSpreadSquared;
        const DistanceTransformProperties& mProperties;
    };
    static void distanceTransform2d( double *im, int width, int height );
    static void distanceTransform1d( double *f, int n, int *v, double *z, double *d );
    static double maxValueInDistanceTransformArray( const double *array, const unsigned int size );


    class StackBlurLineOperation
    {
      public:
        StackBlurLineOperation( int alpha, LineOperationDirection direction, bool forwardDirection, int i1, int i2 )
            : mAlpha( alpha )
            , mDirection( direction )
            , mForwardDirection( forwardDirection )
            , mi1( i1 )
            , mi2( i2 )
        { }

        typedef void result_type;

        LineOperationDirection direction() { return mDirection; }

        void operator()( QRgb* startRef, const int lineLength, const int bytesPerLine );

      private:
        int mAlpha;
        LineOperationDirection mDirection;
        bool mForwardDirection;
        int mi1;
        int mi2;
    };

    static double *createGaussianKernel( const int radius );

    class GaussianBlurOperation
    {
      public:
        GaussianBlurOperation( int radius, LineOperationDirection direction, QImage* destImage, double* kernel )
            : mRadius( radius )
            , mDirection( direction )
            , mDestImage( destImage )
            , mDestImageBpl( destImage->bytesPerLine() )
            , mKernel( kernel )
        {}

        typedef void result_type;

        void operator()( ImageBlock& block );

      private:
        int mRadius;
        LineOperationDirection mDirection;
        QImage* mDestImage;
        int mDestImageBpl;
        double* mKernel;

        inline QRgb gaussianBlurVertical( const int posy, unsigned char *sourceFirstLine, const int sourceBpl, const int height );
        inline QRgb gaussianBlurHorizontal( const int posx, unsigned char *sourceFirstLine, const int width );
    };

    //flip


    class FlipLineOperation
    {
      public:
        FlipLineOperation( LineOperationDirection direction )
            : mDirection( direction )
        { }

        typedef void result_type;

        LineOperationDirection direction() { return mDirection; }

        void operator()( QRgb* startRef, const int lineLength, const int bytesPerLine );

      private:
        LineOperationDirection mDirection;
    };


};

#endif // QGSIMAGEOPERATION_H

