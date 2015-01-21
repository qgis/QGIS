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
 * \note These operations do not work using premultiplied ARGB32_Premultiplied images
 * - please make sure the images are converted to standard ARGB32 images prior to calling
 * these operations.
 *
 * \note Added in version 2.7
 */
class QgsImageOperation
{
%TypeHeaderCode
#include <qgsimageoperation.h>
%End

  public:

    /** Modes for converting a QImage to grayscale
    */
    enum GrayscaleMode
    {
      GrayscaleLightness, /*< keep the lightness of the color, drops the saturation */
      GrayscaleLuminosity, /*< grayscale by perceptual luminosity (weighted sum of color RGB components) */
      GrayscaleAverage /*< grayscale by taking average of color RGB components */
    };
    
    /** Flip operation types
    */
    enum FlipType
    {
      FlipHorizontal, /*< flip the image horizontally */
      FlipVertical /*< flip the image vertically */
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
    static void distanceTransform( QImage &image, const QgsImageOperation::DistanceTransformProperties& properties );

    /**Performs a stack blur on an image. Stack blur represents a good balance between
     * speed and blur quality.
     * @param image QImage to blur
     * @param radius blur radius in pixels, maximum value of 16
     * @param alphaOnly set to true to blur only the alpha component of the image
     */
    static void stackBlur( QImage &image, const int radius, const bool alphaOnly = false );
    
    /**Performs a gaussian blur on an image. Gaussian blur is slower but results in a high
     * quality blur.
     * @param image QImage to blur
     * @param radius blur radius in pixels
     * @returns blurred image
     */
    static QImage* gaussianBlur( QImage &image, const int radius ) /Factory/;
    
    /**Flips an image horizontally or vertically
     * @param image QImage to flip
     * @param type type of flip to perform (horizontal or vertical)
     */
    static void flipImage( QImage &image, FlipType type );
    
};
