/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgsshadoweffect.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsShadowEffect : QgsPaintEffect
{
%Docstring
 Base class for paint effects which offset, blurred shadows

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsshadoweffect.h"
%End
  public:

    QgsShadowEffect();

    virtual QgsStringMap properties() const;

    virtual void readProperties( const QgsStringMap &props );


    void setBlurLevel( const int level );
%Docstring
Sets blur level (strength) for the shadow.

:param level: blur level. Values between 0 and 16 are valid, with larger
values indicating greater blur strength.

.. seealso:: :py:func:`blurLevel`
%End

    int blurLevel() const;
%Docstring
Returns the blur level (strength) for the shadow.

:return: blur level. Value will be between 0 and 16, with larger
values indicating greater blur strength.

.. seealso:: :py:func:`setBlurLevel`
%End

    void setOffsetAngle( const int angle );
%Docstring
Sets the angle for offsetting the shadow.

:param angle: offset angle in degrees clockwise from North

.. seealso:: :py:func:`offsetAngle`

.. seealso:: :py:func:`setOffsetDistance`
%End

    int offsetAngle() const;
%Docstring
Returns the angle used for offsetting the shadow.

:return: offset angle in degrees clockwise from North

.. seealso:: :py:func:`setOffsetAngle`

.. seealso:: :py:func:`offsetDistance`
%End

    void setOffsetDistance( const double distance );
%Docstring
Sets the distance for offsetting the shadow.

:param distance: offset distance. Units are specified via setOffsetUnit()

.. seealso:: :py:func:`offsetDistance`

.. seealso:: :py:func:`setOffsetUnit`

.. seealso:: :py:func:`setOffsetMapUnitScale`
%End

    double offsetDistance() const;
%Docstring
Returns the distance used for offsetting the shadow.

:return: offset distance. Distance units are retrieved via offsetUnit()

.. seealso:: :py:func:`setOffsetDistance`

.. seealso:: :py:func:`offsetUnit`

.. seealso:: :py:func:`offsetMapUnitScale`
%End

    void setOffsetUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units used for the shadow offset distance.

:param unit: units for offset distance

.. seealso:: :py:func:`offsetUnit`

.. seealso:: :py:func:`setOffsetDistance`

.. seealso:: :py:func:`setOffsetMapUnitScale`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units used for the shadow offset distance.

:return: units for offset distance

.. seealso:: :py:func:`setOffsetUnit`

.. seealso:: :py:func:`offsetDistance`

.. seealso:: :py:func:`offsetMapUnitScale`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale used for the shadow offset distance.

:param scale: map unit scale for offset distance

.. seealso:: :py:func:`offsetMapUnitScale`

.. seealso:: :py:func:`setOffsetDistance`

.. seealso:: :py:func:`setOffsetUnit`
%End

    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
Returns the map unit scale used for the shadow offset distance.

:return: map unit scale for offset distance

.. seealso:: :py:func:`setOffsetMapUnitScale`

.. seealso:: :py:func:`offsetDistance`

.. seealso:: :py:func:`offsetUnit`
%End

    void setColor( const QColor &color );
%Docstring
Sets the color for the shadow.

:param color: shadow color

.. seealso:: :py:func:`color`
%End

    QColor color() const;
%Docstring
Returns the color used for the shadow.

:return: shadow color

.. seealso:: :py:func:`setColor`
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

  protected:

    virtual QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const;

    virtual void draw( QgsRenderContext &context );


    virtual bool exteriorShadow() const = 0;
%Docstring
Specifies whether the shadow is drawn outside the picture or within
the picture.

:return: true if shadow is to be drawn outside the picture, or false
to draw shadow within the picture
%End


};


class QgsDropShadowEffect : QgsShadowEffect
{
%Docstring
 A paint effect which draws an offset and optionally blurred drop shadow

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsshadoweffect.h"
%End
  public:

    static QgsPaintEffect *create( const QgsStringMap &map ) /Factory/;
%Docstring
Creates a new QgsDropShadowEffect effect from a properties string map.

:param map: encoded properties string map

:return: new QgsDropShadowEffect
%End

    QgsDropShadowEffect();

    virtual QString type() const;

    virtual QgsDropShadowEffect *clone() const /Factory/;


  protected:

    virtual bool exteriorShadow() const;


};

class QgsInnerShadowEffect : QgsShadowEffect
{
%Docstring
 A paint effect which draws an offset and optionally blurred drop shadow
within a picture.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsshadoweffect.h"
%End
  public:

    static QgsPaintEffect *create( const QgsStringMap &map ) /Factory/;
%Docstring
Creates a new QgsInnerShadowEffect effect from a properties string map.

:param map: encoded properties string map

:return: new QgsInnerShadowEffect
%End

    QgsInnerShadowEffect();

    virtual QString type() const;

    virtual QgsInnerShadowEffect *clone() const /Factory/;


  protected:

    virtual bool exteriorShadow() const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgsshadoweffect.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
