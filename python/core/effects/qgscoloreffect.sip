/** \ingroup core
 * \class QgsColorEffect
 * \brief A paint effect which alters the colors (eg brightness, contrast) in a
 * source picture.
 *
 * \note Added in version 2.9
 */

class QgsColorEffect : QgsPaintEffect
{
%TypeHeaderCode
#include <qgscoloreffect.h>
%End

  public:

    /** Creates a new QgsColorEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsColorEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map ) /Factory/;

    QgsColorEffect();
    virtual ~QgsColorEffect();

    virtual QString type() const;
    virtual QgsStringMap properties() const;
    virtual void readProperties( const QgsStringMap& props );
    virtual QgsColorEffect* clone() const /Factory/;

    /** Sets the brightness modification for the effect.
     * @param brightness Valid values are between -255 and 255, where 0 represents
     * no change, negative values indicate darkening and positive values indicate
     * lightening
     * @see setBrightness
     */
    void setBrightness( int brightness );

    /** Returns the brightness modification for the effect.
     * @returns brightness value. Values are between -255 and 255, where 0 represents
     * no change, negative values indicate darkening and positive values indicate
     * lightening
     * @see setBrightness
     */
    int brightness() const;

    /** Sets the contrast modification for the effect.
     * @param contrast Valid values are between -100 and 100, where 0 represents
     * no change, negative values indicate less contrast and positive values indicate
     * greater contrast
     * @see setContrast
     */
    void setContrast( int contrast );

    /** Returns the contrast modification for the effect.
     * @returns contrast value. Values are between -100 and 100, where 0 represents
     * no change, negative values indicate less contrast and positive values indicate
     * greater contrast
     * @see setContrast
     */
    int contrast() const;

    /** Sets the saturation modification for the effect.
     * @param saturation Valid values are between 0 and 2.0, where 1.0 represents
     * no change, 0.0 represents totally desaturated (grayscale), and positive values indicate
     * greater saturation
     * @see saturation
     */
    void setSaturation( double saturation );

    /** Returns the saturation modification for the effect.
     * @returns saturation value. Values are between 0 and 2.0, where 1.0 represents
     * no change, 0.0 represents totally desaturated (grayscale), and positive values indicate
     * greater saturation
     * @see setSaturation
     */
    double saturation() const;

    /** Sets whether the effect should convert a picture to grayscale.
     * @param grayscaleMode method for grayscale conversion
     * @see grayscaleMode
     */
    void setGrayscaleMode( QgsImageOperation::GrayscaleMode grayscaleMode );

    /** Returns whether the effect will convert a picture to grayscale.
     * @returns method for grayscale conversion
     * @see setGrayscaleMode
     */
    QgsImageOperation::GrayscaleMode grayscaleMode() const;

    /** Sets whether the effect should colorize a picture.
     * @param colorizeOn set to true to enable colorization
     * @see colorizeOn
     * @see setColorizeColor
     * @see setColorizeStrength
     */
    void setColorizeOn( bool colorizeOn );

    /** Returns whether the effect will colorize a picture.
     * @returns true if colorization is enableds
     * @see setColorizeOn
     * @see colorizeColor
     * @see colorizeStrength
     */
    bool colorizeOn() const;

    /** Sets the color used for colorizing a picture. This is only used if
     * @link setColorizeOn @endlink is set to true.
     * @param colorizeColor colorization color
     * @see colorizeColor
     * @see setColorizeOn
     * @see setColorizeStrength
     */
    void setColorizeColor( const QColor& colorizeColor );

    /** Returns the color used for colorizing a picture. This is only used if
     * @link colorizeOn @endlink is set to true.
     * @returns colorization color
     * @see setColorizeColor
     * @see colorizeOn
     * @see colorizeStrength
     */
    QColor colorizeColor() const;

    /** Sets the strength for colorizing a picture. This is only used if
     * @link setColorizeOn @endlink is set to true.
     * @param colorizeStrength colorization strength, between 0 and 100
     * @see colorizeStrength
     * @see setColorizeOn
     * @see setColorizeColor
     */
    void setColorizeStrength( int colorizeStrength );

    /** Returns the strength used for colorizing a picture. This is only used if
     * @link setColorizeOn @endlink is set to true.
     * @returns colorization strength, between 0 and 100
     * @see setColorizeStrengths
     * @see colorizeOn
     * @see colorizeColor
     */
    int colorizeStrength() const;

    /** Sets the transparency for the effect
     * @param transparency double between 0 and 1 inclusive, where 0 is fully opaque
     * and 1 is fully transparent
     * @see transparency
     */
    void setTransparency( const double transparency );

    /** Returns the transparency for the effect
     * @returns transparency value between 0 and 1 inclusive, where 0 is fully opaque
     * and 1 is fully transparent
     * @see setTransparency
     */
    double transparency() const;

    /** Sets the blend mode for the effect
     * @param mode blend mode used for drawing the effect on to a destination
     * paint device
     * @see blendMode
     */
    void setBlendMode( const QPainter::CompositionMode mode );

    /** Returns the blend mode for the effect
     * @returns blend mode used for drawing the effect on to a destination
     * paint device
     * @see setBlendMode
     */
    QPainter::CompositionMode blendMode() const;

  protected:

    virtual void draw( QgsRenderContext& context );
};
