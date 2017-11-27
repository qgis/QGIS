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
 \param level blur level. Values between 0 and 16 are valid, with larger
 values indicating greater blur strength.
.. seealso:: blurLevel
%End

    int blurLevel() const;
%Docstring
 Returns the blur level (strength) for the shadow.
 :return: blur level. Value will be between 0 and 16, with larger
 values indicating greater blur strength.
.. seealso:: setBlurLevel
 :rtype: int
%End

    void setOffsetAngle( const int angle );
%Docstring
 Sets the angle for offsetting the shadow.
 \param angle offset angle in degrees clockwise from North
.. seealso:: offsetAngle
.. seealso:: setOffsetDistance
%End

    int offsetAngle() const;
%Docstring
 Returns the angle used for offsetting the shadow.
 :return: offset angle in degrees clockwise from North
.. seealso:: setOffsetAngle
.. seealso:: offsetDistance
 :rtype: int
%End

    void setOffsetDistance( const double distance );
%Docstring
 Sets the distance for offsetting the shadow.
 \param distance offset distance. Units are specified via setOffsetUnit()
.. seealso:: offsetDistance
.. seealso:: setOffsetUnit
.. seealso:: setOffsetMapUnitScale
%End

    double offsetDistance() const;
%Docstring
 Returns the distance used for offsetting the shadow.
 :return: offset distance. Distance units are retrieved via offsetUnit()
.. seealso:: setOffsetDistance
.. seealso:: offsetUnit
.. seealso:: offsetMapUnitScale
 :rtype: float
%End

    void setOffsetUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units used for the shadow offset distance.
 \param unit units for offset distance
.. seealso:: offsetUnit
.. seealso:: setOffsetDistance
.. seealso:: setOffsetMapUnitScale
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
 Returns the units used for the shadow offset distance.
 :return: units for offset distance
.. seealso:: setOffsetUnit
.. seealso:: offsetDistance
.. seealso:: offsetMapUnitScale
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale used for the shadow offset distance.
 \param scale map unit scale for offset distance
.. seealso:: offsetMapUnitScale
.. seealso:: setOffsetDistance
.. seealso:: setOffsetUnit
%End

    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
 Returns the map unit scale used for the shadow offset distance.
 :return: map unit scale for offset distance
.. seealso:: setOffsetMapUnitScale
.. seealso:: offsetDistance
.. seealso:: offsetUnit
 :rtype: QgsMapUnitScale
%End

    void setColor( const QColor &color );
%Docstring
 Sets the color for the shadow.
 \param color shadow color
.. seealso:: color
%End

    QColor color() const;
%Docstring
 Returns the color used for the shadow.
 :return: shadow color
.. seealso:: setColor
 :rtype: QColor
%End

    void setOpacity( const double opacity );
%Docstring
 Sets the ``opacity`` for the effect.
 \param opacity double between 0 and 1 inclusive, where 0 is fully transparent
 and 1 is fully opaque
.. seealso:: opacity()
%End

    double opacity() const;
%Docstring
 Returns the opacity for the effect.
 :return: opacity value between 0 and 1 inclusive, where 0 is fully transparent
 and 1 is fully opaque
.. seealso:: setOpacity()
 :rtype: float
%End

    void setBlendMode( const QPainter::CompositionMode mode );
%Docstring
 Sets the blend mode for the effect
 \param mode blend mode used for drawing the effect on to a destination
 paint device
.. seealso:: blendMode
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
 Returns the blend mode for the effect
 :return: blend mode used for drawing the effect on to a destination
 paint device
.. seealso:: setBlendMode
 :rtype: QPainter.CompositionMode
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
 :rtype: bool
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
 \param map encoded properties string map
 :return: new QgsDropShadowEffect
 :rtype: QgsPaintEffect
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
 \param map encoded properties string map
 :return: new QgsInnerShadowEffect
 :rtype: QgsPaintEffect
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
