/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgsgloweffect.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGlowEffect : QgsPaintEffect
{
%Docstring
 Base class for paint effect which draw a glow inside or outside a
picture.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsgloweffect.h"
%End
  public:

    enum GlowColorType
    {
      SingleColor,
      ColorRamp
    };

    QgsGlowEffect();
    QgsGlowEffect( const QgsGlowEffect &other );
    ~QgsGlowEffect();

    virtual QgsStringMap properties() const;

    virtual void readProperties( const QgsStringMap &props );


    void setSpread( const double spread );
%Docstring
Sets the spread distance for drawing the glow effect.

:param spread: spread distance. Units are specified via setSpreadUnit()

.. seealso:: :py:func:`spread`

.. seealso:: :py:func:`setSpreadUnit`

.. seealso:: :py:func:`setSpreadMapUnitScale`
%End

    double spread() const;
%Docstring
Returns the spread distance used for drawing the glow effect.

:return: spread distance. Units are retrieved via spreadUnit()

.. seealso:: :py:func:`setSpread`

.. seealso:: :py:func:`spreadUnit`

.. seealso:: :py:func:`spreadMapUnitScale`
%End

    void setSpreadUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units used for the glow spread distance.

:param unit: units for spread distance

.. seealso:: :py:func:`spreadUnit`

.. seealso:: :py:func:`setSpread`

.. seealso:: :py:func:`setSpreadMapUnitScale`
%End

    QgsUnitTypes::RenderUnit spreadUnit() const;
%Docstring
Returns the units used for the glow spread distance.

:return: units for spread distance

.. seealso:: :py:func:`setSpreadUnit`

.. seealso:: :py:func:`spread`

.. seealso:: :py:func:`spreadMapUnitScale`
%End

    void setSpreadMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale used for the spread distance.

:param scale: map unit scale for spread distance

.. seealso:: :py:func:`spreadMapUnitScale`

.. seealso:: :py:func:`setSpread`

.. seealso:: :py:func:`setSpreadUnit`
%End

    const QgsMapUnitScale &spreadMapUnitScale() const;
%Docstring
Returns the map unit scale used for the spread distance.

:return: map unit scale for spread distance

.. seealso:: :py:func:`setSpreadMapUnitScale`

.. seealso:: :py:func:`spread`

.. seealso:: :py:func:`spreadUnit`
%End

    void setBlurLevel( const int level );
%Docstring
Sets blur level (strength) for the glow. This can be used to smooth the
output from the glow effect.

:param level: blur level. Values between 0 and 16 are valid, with larger
values indicating greater blur strength.

.. seealso:: :py:func:`blurLevel`
%End

    int blurLevel() const;
%Docstring
Returns the blur level (strength) for the glow.

:return: blur level. Value will be between 0 and 16, with larger
values indicating greater blur strength.

.. seealso:: :py:func:`setBlurLevel`
%End

    void setOpacity( const double opacity );
%Docstring
Sets the ``opacity`` for the effect.

:param opacity: double between 0 and 1 inclusive, where 0 is fully transparent
and 1 is fully opaque

.. seealso:: :py:func:`opacity`
%End

    double opacity() const;
%Docstring
Returns the opacity for the effect.

:return: opacity value between 0 and 1 inclusive, where 0 is fully transparent
and 1 is fully opaque

.. seealso:: :py:func:`setOpacity`
%End

    void setColor( const QColor &color );
%Docstring
Sets the color for the glow. This only applies if the colorType()
is set to SingleColor. The glow will fade between the specified color and
a totally transparent version of the color.

:param color: glow color

.. seealso:: :py:func:`color`

.. seealso:: :py:func:`setColorType`
%End

    QColor color() const;
%Docstring
Returns the color for the glow. This only applies if the colorType()
is set to SingleColor. The glow will fade between the specified color and
a totally transparent version of the color.

:return: glow color

.. seealso:: :py:func:`setColor`

.. seealso:: :py:func:`colorType`
%End

    void setRamp( QgsColorRamp *ramp /Transfer/ );
%Docstring
Sets the color ramp for the glow. This only applies if the colorType()
is set to ColorRamp. The glow will utilize colors from the ramp.

:param ramp: color ramp for glow. Ownership of the ramp is transferred to the effect.

.. seealso:: :py:func:`ramp`

.. seealso:: :py:func:`setColorType`
%End

    QgsColorRamp *ramp() const;
%Docstring
Returns the color ramp used for the glow. This only applies if the colorType()
is set to ColorRamp. The glow will utilize colors from the ramp.

:return: color ramp for glow

.. seealso:: :py:func:`setRamp`

.. seealso:: :py:func:`colorType`
%End

    void setBlendMode( const QPainter::CompositionMode mode );
%Docstring
Sets the blend mode for the effect

:param mode: blend mode used for drawing the effect on to a destination
paint device

.. seealso:: :py:func:`blendMode`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Returns the blend mode for the effect

:return: blend mode used for drawing the effect on to a destination
paint device

.. seealso:: :py:func:`setBlendMode`
%End

    void setColorType( GlowColorType colorType );
%Docstring
Sets the color mode to use for the glow. The glow can either be drawn using a :py:class:`QgsColorRamp`
color ramp or by simply specificing a single color. setColorType is used to specify which mode to use
for the glow.

:param colorType: color type to use for glow

.. seealso:: :py:func:`colorType`

.. seealso:: :py:func:`setColor`

.. seealso:: :py:func:`setRamp`
%End

    GlowColorType colorType() const;
%Docstring
Returns the color mode used for the glow. The glow can either be drawn using a :py:class:`QgsColorRamp`
color ramp or by specificing a single color.

:return: current color mode used for the glow

.. seealso:: :py:func:`setColorType`

.. seealso:: :py:func:`color`

.. seealso:: :py:func:`ramp`
%End


  protected:

    virtual QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const;

    virtual void draw( QgsRenderContext &context );


    virtual bool shadeExterior() const = 0;
%Docstring
Specifies whether the glow is drawn outside the picture or within
the picture.

:return: true if glow is to be drawn outside the picture, or false
to draw glow within the picture
%End


};



class QgsOuterGlowEffect : QgsGlowEffect
{
%Docstring
 A paint effect which draws a glow outside of a picture.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsgloweffect.h"
%End
  public:

    static QgsPaintEffect *create( const QgsStringMap &map ) /Factory/;
%Docstring
Creates a new QgsOuterGlowEffect effect from a properties string map.

:param map: encoded properties string map

:return: new QgsOuterGlowEffect
%End

    QgsOuterGlowEffect();

    virtual QString type() const;
    virtual QgsOuterGlowEffect *clone() const /Factory/;


  protected:

    virtual bool shadeExterior() const;

};



class QgsInnerGlowEffect : QgsGlowEffect
{
%Docstring
 A paint effect which draws a glow within a picture.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsgloweffect.h"
%End
  public:

    static QgsPaintEffect *create( const QgsStringMap &map ) /Factory/;
%Docstring
Creates a new QgsInnerGlowEffect effect from a properties string map.

:param map: encoded properties string map

:return: new QgsInnerGlowEffect
%End

    QgsInnerGlowEffect();

    virtual QString type() const;
    virtual QgsInnerGlowEffect *clone() const /Factory/;


  protected:

    virtual bool shadeExterior() const;

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgsgloweffect.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
