/** \ingroup core
 * \class QgsShadowEffect
 * \brief Base class for paint effects which offset, blurred shadows
 *
 * \note Added in version 2.9
 */

class QgsShadowEffect : QgsPaintEffect
{
%TypeHeaderCode
#include <qgsshadoweffect.h>
%End

  public:

    QgsShadowEffect();
    virtual ~QgsShadowEffect();

    virtual QgsStringMap properties() const;
    virtual void readProperties( const QgsStringMap& props );

    /** Sets blur level (strength) for the shadow.
     * @param level blur level. Values between 0 and 16 are valid, with larger
     * values indicating greater blur strength.
     * @see blurLevel
     */
    void setBlurLevel( const int level );

    /** Returns the blur level (strength) for the shadow.
     * @returns blur level. Value will be between 0 and 16, with larger
     * values indicating greater blur strength.
     * @see setBlurLevel
     */
    int blurLevel() const;

    /** Sets the angle for offsetting the shadow.
     * @param angle offset angle in degrees clockwise from North
     * @see offsetAngle
     * @see setOffsetDistance
     */
    void setOffsetAngle( const int angle );

    /** Returns the angle used for offsetting the shadow.
     * @returns offset angle in degrees clockwise from North
     * @see setOffsetAngle
     * @see offsetDistance
     */
    int offsetAngle() const;

    /** Sets the distance for offsetting the shadow.
     * @param distance offset distance. Units are specified via @link setOffsetUnit @endlink
     * @see offsetDistance
     * @see setOffsetUnit
     * @see setOffsetMapUnitScale
     */
    void setOffsetDistance( const double distance );

    /** Returns the distance used for offsetting the shadow.
     * @returns offset distance. Distance units are retreived via @link offsetUnit @endlink
     * @see setOffsetDistance
     * @see offsetUnit
     * @see offsetMapUnitScale
     */
    double offsetDistance() const;

    /** Sets the units used for the shadow offset distance.
     * @param unit units for offset distance
     * @see offsetUnit
     * @see setOffsetDistance
     * @see setOffsetMapUnitScale
     */
    void setOffsetUnit( const QgsSymbolV2::OutputUnit unit );

    /** Returns the units used for the shadow offset distance.
     * @returns units for offset distance
     * @see setOffsetUnit
     * @see offsetDistance
     * @see offsetMapUnitScale
     */
    QgsSymbolV2::OutputUnit offsetUnit() const;

    /** Sets the map unit scale used for the shadow offset distance.
     * @param scale map unit scale for offset distance
     * @see offsetMapUnitScale
     * @see setOffsetDistance
     * @see setOffsetUnit
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the map unit scale used for the drop shadow offset distance.
     * @returns map unit scale for offset distance
     * @see setOffsetMapUnitScale
     * @see offsetDistance
     * @see offsetUnit
     */
    const QgsMapUnitScale& offsetMapUnitScale() const;

    /** Sets the color for the shadow.
     * @param color shadow color
     * @see color
     */
    void setColor( const QColor& color );

    /** Returns the color used for the shadow.
     * @returns shadow color
     * @see setColor
     */
    QColor color() const;

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

    virtual QRectF boundingRect( const QRectF& rect, const QgsRenderContext& context ) const;
    virtual void draw( QgsRenderContext& context );

    /** Specifies whether the shadow is drawn outside the picture or within
     * the picture.
     * @returns true if shadow is to be drawn outside the picture, or false
     * to draw shadow within the picture
     */
    virtual bool exteriorShadow() const = 0;

};


/** \ingroup core
 * \class QgsDropShadowEffect
 * \brief A paint effect which draws an offset and optionally blurred drop shadow
 *
 * \note Added in version 2.9
 */

class QgsDropShadowEffect : QgsShadowEffect
{
%TypeHeaderCode
#include <qgsshadoweffect.h>
%End

  public:

    /** Creates a new QgsDropShadowEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsDropShadowEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map ) /Factory/;

    QgsDropShadowEffect();
    virtual ~QgsDropShadowEffect();

    virtual QString type() const;
    virtual QgsPaintEffect* clone() const /Factory/;

  protected:

    virtual bool exteriorShadow() const;

};


/** \ingroup core
 * \class QgsInnerShadowEffect
 * \brief A paint effect which draws an offset and optionally blurred drop shadow
 * within a picture.
 *
 * \note Added in version 2.9
 */
class QgsInnerShadowEffect : QgsShadowEffect
{
%TypeHeaderCode
#include <qgsshadoweffect.h>
%End

  public:

    /** Creates a new QgsInnerShadowEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsInnerShadowEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map ) /Factory/;

    QgsInnerShadowEffect();
    virtual ~QgsInnerShadowEffect();

    virtual QString type() const;
    virtual QgsPaintEffect* clone() const /Factory/;

  protected:

    virtual bool exteriorShadow() const;

};
