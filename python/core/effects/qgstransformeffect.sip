/** \ingroup core
 * \class QgsTransformEffect
 * \brief A paint effect which applies transformations (such as move,
 * scale and rotate) to a picture.
 *
 * \note Added in version 2.9
 */

class QgsTransformEffect : QgsPaintEffect
{
%TypeHeaderCode
#include <qgstransformeffect.h>
%End
 
  public:

    /** Creates a new QgsTransformEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsTransformEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& ) /Factory/;

    QgsTransformEffect();
    virtual ~QgsTransformEffect();

    virtual QString type() const;
    virtual QgsStringMap properties() const;
    virtual void readProperties( const QgsStringMap& props );
    virtual QgsPaintEffect* clone() const /Factory/;

    /** Sets the transform x translation.
     * @param translateX distance to translate along the x axis
     * @see translateX
     * @see setTranslateY
     * @see setTranslateUnit
     * @see setTranslateMapUnitScale
     */
    void setTranslateX( const double translateX );

    /** Returns the transform x translation.
     * @returns X distance translated along the x axis
     * @see setTranslateX
     * @see translateY
     * @see translateUnit
     * @see translateMapUnitScale
     */
    double translateX() const;

    /** Sets the transform y translation.
     * @param translateY distance to translate along the y axis
     * @see translateY
     * @see setTranslateX
     * @see setTranslateUnit
     * @see setTranslateMapUnitScale
     */
    void setTranslateY( const double translateY );

    /** Returns the transform y translation.
     * @returns Y distance translated along the y axis
     * @see setTranslateY
     * @see translateX
     * @see translateUnit
     * @see translateMapUnitScale
     */
    double translateY() const;

    /** Sets the units used for the transform translation.
     * @param unit units for translation
     * @see translateUnit
     * @see setTranslateX
     * @see setTranslateY
     * @see setTranslateMapUnitScale
     */
    void setTranslateUnit( const QgsSymbolV2::OutputUnit unit );

    /** Returns the units used for the transform translation.
     * @returns units for translation
     * @see setTranslateUnit
     * @see translateX
     * @see translateY
     * @see translateMapUnitScale
     */
    QgsSymbolV2::OutputUnit translateUnit() const;

    /** Sets the map unit scale used for the transform translation.
     * @param scale map unit scale for translation
     * @see translateMapUnitScale
     * @see setTranslateX
     * @see setTranslateY
     * @see setTranslateUnit
     */
    void setTranslateMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the map unit scale used for the transform translation.
     * @returns map unit scale for translation
     * @see setTranslateMapUnitScale
     * @see translateX
     * @see translateY
     * @see translateUnit
     */
    const QgsMapUnitScale& translateMapUnitScale() const;

    /** Sets the x axis scaling factor.
     * @param scaleX factor to scale x axis by, where 1.0 = no scaling
     * @see scaleX
     * @see setScaleY
     */
    void setScaleX( const double scaleX );

    /** Returns the x axis scaling factor.
     * @returns x axis scaling factor, where 1.0 = no scaling
     * @see setScaleX
     * @see scaleY
     */
    double scaleX() const;

    /** Sets the x axis scaling factor.
     * @param scaleX factor to scale x axis by, where 1.0 = no scaling
     * @see scaleX
     * @see setScaleY
     */
    void setScaleY( const double scaleY );

    /** Returns the y axis scaling factor.
     * @returns y axis scaling factor, where 1.0 = no scaling
     * @see setScaleY
     * @see scaleX
     */
    double scaleY() const;

    /** Sets the transform rotation.
     * @param rotation degrees to rotate, clockwise
     * @see rotation
     */
    void setRotation( const double rotation );

    /** Returns the transform rotation.
     * @returns rotation in degrees clockwise
     * @see setRotation
     */
    double rotation() const;

    /** Sets the x axis shearing factor.
     * @param shearX x axis shearing
     * @see shearX
     * @see setShearY
     */
    void setShearX( const double shearX );

    /** Returns the x axis shearing factor.
     * @returns x axis shearing
     * @see setShearX
     * @see shearY
     */
    double shearX() const;

    /** Sets the y axis shearing factor.
     * @param shearY y axis shearing
     * @see shearY
     * @see setShearX
     */
    void setShearY( const double shearY );

    /** Returns the y axis shearing factor.
     * @returns y axis shearing
     * @see setShearY
     * @see shearX
     */
    double shearY() const;

    /** Sets whether to reflect along the x-axis
     * @param reflectX true to reflect horizontally
     * @see reflectX
     * @see setReflectY
     */
    void setReflectX( const bool reflectX );

    /** Returns whether transform will be reflected along the x-axis
     * @returns true if transform will reflect horizontally
     * @see setReflectX
     * @see reflectY
     */
    bool reflectX() const;

    /** Sets whether to reflect along the y-axis
     * @param reflectY true to reflect horizontally
     * @see reflectY
     * @see setReflectX
     */
    void setReflectY( const bool reflectY );

    /** Returns whether transform will be reflected along the y-axis
     * @returns true if transform will reflect horizontally
     * @see setReflectY
     * @see reflectX
     */
    bool reflectY() const;

  protected:

    virtual void draw( QgsRenderContext& context );
    virtual QRectF boundingRect( const QRectF& rect, const QgsRenderContext& context ) const;

};
