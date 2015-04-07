/** \ingroup core
 * \class QgsBlurEffect
 * \brief A paint effect which blurs a source picture, using a number of different blur
 * methods.
 *
 * \note Added in version 2.9
 */

class QgsBlurEffect : QgsPaintEffect
{
%TypeHeaderCode
#include <qgsblureffect.h>
%End

  public:

    /** Available blur methods (algorithms) */
    enum BlurMethod
    {
      StackBlur, /*< stack blur, a fast but low quality blur. Valid blur level values are between 0 - 16.*/
      GaussianBlur /*< Gaussian blur, a slower but high quality blur. Blur level values are the distance in pixels for the blur operation. */
    };

    /** Creates a new QgsBlurEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsBlurEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& ) /Factory/;

    QgsBlurEffect();
    virtual ~QgsBlurEffect();

    virtual QString type() const;
    virtual QgsStringMap properties() const;
    virtual void readProperties( const QgsStringMap& props );
    virtual QgsPaintEffect* clone() const /Factory/;

    /** Sets blur level (strength)
     * @param level blur level. Depending on the current @link blurMethod @endlink, this parameter
     * has different effects
     * @see blurLevel
     * @see blurMethod
     */
    void setBlurLevel( const int level );

    /** Returns the blur level (strength)
     * @returns blur level. Depending on the current @link blurMethod @endlink, this parameter
     * has different effects
     * @see setBlurLevel
     * @see blurMethod
     */
    int blurLevel() const;

    /** Sets the blur method (algorithm) to use for performing the blur.
     * @param method blur method
     * @see blurMethod
     */
    void setBlurMethod( const BlurMethod method );

    /** Returns the blur method (algorithm) used for performing the blur.
     * @returns blur method
     * @see setBlurMethod
     */
    BlurMethod blurMethod() const;

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
    virtual QRectF boundingRect( const QRectF& rect, const QgsRenderContext& context ) const;

};
