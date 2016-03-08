/** \ingroup core
 * \class QgsGlowEffect
 * \brief Base class for paint effect which draw a glow inside or outside a
 * picture.
 *
 * \note Added in version 2.9
 */

class QgsGlowEffect : QgsPaintEffect
{
%TypeHeaderCode
#include <qgsgloweffect.h>
%End

  public:

    /** Color sources for the glow */
    enum GlowColorType
    {
      SingleColor, /*!< use a single color and fade the color to totally transparent */
      ColorRamp /*!< use colors from a color ramp */
    };

    QgsGlowEffect();
    QgsGlowEffect( const QgsGlowEffect& other );
    virtual ~QgsGlowEffect();

    virtual QgsStringMap properties() const;
    virtual void readProperties( const QgsStringMap& props );

    /** Sets the spread distance for drawing the glow effect.
     * @param spread spread distance. Units are specified via @link setSpreadUnit @endlink
     * @see spread
     * @see setSpreadUnit
     * @see setSpreadMapUnitScale
     */
    void setSpread( const double spread );

    /** Returns the spread distance used for drawing the glow effect.
     * @returns spread distance. Units are retrieved via @link spreadUnit @endlink
     * @see setSpread
     * @see spreadUnit
     * @see spreadMapUnitScale
     */
    double spread() const;

    /** Sets the units used for the glow spread distance.
     * @param unit units for spread distance
     * @see spreadUnit
     * @see setSpread
     * @see setSpreadMapUnitScale
     */
    void setSpreadUnit( const QgsSymbolV2::OutputUnit unit );

    /** Returns the units used for the glow spread distance.
     * @returns units for spread distance
     * @see setSpreadUnit
     * @see spread
     * @see spreadMapUnitScale
     */
    QgsSymbolV2::OutputUnit spreadUnit() const;

    /** Sets the map unit scale used for the spread distance.
     * @param scale map unit scale for spread distance
     * @see spreadMapUnitScale
     * @see setSpread
     * @see setSpreadUnit
     */
    void setSpreadMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the map unit scale used for the spread distance.
     * @returns map unit scale for spread distance
     * @see setSpreadMapUnitScale
     * @see spread
     * @see spreadUnit
     */
    const QgsMapUnitScale& spreadMapUnitScale() const;

    /** Sets blur level (strength) for the glow. This can be used to smooth the
     * output from the glow effect.
     * @param level blur level. Values between 0 and 16 are valid, with larger
     * values indicating greater blur strength.
     * @see blurLevel
     */
    void setBlurLevel( const int level );

    /** Returns the blur level (strength) for the glow.
     * @returns blur level. Value will be between 0 and 16, with larger
     * values indicating greater blur strength.
     * @see setBlurLevel
     */
    int blurLevel() const;

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

    /** Sets the color for the glow. This only applies if the @link colorType @endlink
     * is set to SingleColor. The glow will fade between the specified color and
     * a totally transparent version of the color.
     * @param color glow color
     * @see color
     * @see setColorType
     */
    void setColor( const QColor& color );

    /** Returns the color for the glow. This only applies if the @link colorType @endlink
     * is set to SingleColor. The glow will fade between the specified color and
     * a totally transparent version of the color.
     * @returns glow color
     * @see setColor
     * @see colorType
     */
    QColor color() const;

    /** Sets the color ramp for the glow. This only applies if the @link colorType @endlink
     * is set to ColorRamp. The glow will utilise colors from the ramp.
     * @param ramp color ramp for glow. Ownership of the ramp is transferred to the effect.
     * @see ramp
     * @see setColorType
     */
    void setRamp( QgsVectorColorRampV2* ramp /Transfer/ );

    /** Returns the color ramp used for the glow. This only applies if the @link colorType @endlink
     * is set to ColorRamp. The glow will utilise colors from the ramp.
     * @returns color ramp for glow
     * @see setRamp
     * @see colorType
     */
    QgsVectorColorRampV2* ramp() const;

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

    /** Sets the color mode to use for the glow. The glow can either be drawn using a QgsVectorColorRampV2
     * color ramp or by simply specificing a single color. setColorType is used to specify which mode to use
     * for the glow.
     * @param colorType color type to use for glow
     * @see colorType
     * @see setColor
     * @see setRamp
     */
    void setColorType( GlowColorType colorType );

    /** Returns the color mode used for the glow. The glow can either be drawn using a QgsVectorColorRampV2
     * color ramp or by specificing a single color.
     * @returns current color mode used for the glow
     * @see setColorType
     * @see color
     * @see ramp
     */
    GlowColorType colorType() const;

  protected:

    virtual QRectF boundingRect( const QRectF& rect, const QgsRenderContext& context ) const;
    virtual void draw( QgsRenderContext& context );

    /** Specifies whether the glow is drawn outside the picture or within
     * the picture.
     * @returns true if glow is to be drawn outside the picture, or false
     * to draw glow within the picture
     */
    virtual bool shadeExterior() const = 0;

};


/** \ingroup core
 * \class QgsOuterGlowEffect
 * \brief A paint effect which draws a glow outside of a picture.
 *
 * \note Added in version 2.9
 */

class QgsOuterGlowEffect : QgsGlowEffect
{
%TypeHeaderCode
#include <qgsgloweffect.h>
%End
  public:

    /** Creates a new QgsOuterGlowEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsOuterGlowEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map ) /Factory/;

    QgsOuterGlowEffect();
    virtual ~QgsOuterGlowEffect();

    virtual QString type() const;
    virtual QgsOuterGlowEffect* clone() const /Factory/;

  protected:

    virtual bool shadeExterior() const;

};


/** \ingroup core
 * \class QgsInnerGlowEffect
 * \brief A paint effect which draws a glow within a picture.
 *
 * \note Added in version 2.9
 */

class QgsInnerGlowEffect : QgsGlowEffect
{
%TypeHeaderCode
#include <qgsgloweffect.h>
%End
  public:

    /** Creates a new QgsInnerGlowEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsInnerGlowEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map ) /Factory/;

    QgsInnerGlowEffect();
    virtual ~QgsInnerGlowEffect();

    virtual QString type() const;
    virtual QgsInnerGlowEffect* clone() const /Factory/;

  protected:

    virtual bool shadeExterior() const;

};

