/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgsblureffect.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsBlurEffect : QgsPaintEffect
{
%Docstring
 A paint effect which blurs a source picture, using a number of different blur
methods.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsblureffect.h"
%End
  public:

    enum BlurMethod
    {
      StackBlur,
      GaussianBlur
    };

    static QgsPaintEffect *create( const QgsStringMap &map ) /Factory/;
%Docstring
Creates a new QgsBlurEffect effect from a properties string map.

:param map: encoded properties string map

:return: new QgsBlurEffect
%End

    QgsBlurEffect();
%Docstring
Constructor for QgsBlurEffect.
%End

    virtual QString type() const;
    virtual QgsStringMap properties() const;

    virtual void readProperties( const QgsStringMap &props );

    virtual QgsBlurEffect *clone() const /Factory/;


    void setBlurLevel( const int level );
%Docstring
Sets blur level (strength)

:param level: blur level. Depending on the current blurMethod(), this parameter
has different effects

.. seealso:: :py:func:`blurLevel`

.. seealso:: :py:func:`blurMethod`
%End

    int blurLevel() const;
%Docstring
Returns the blur level (strength)

:return: blur level. Depending on the current blurMethod(), this parameter
has different effects

.. seealso:: :py:func:`setBlurLevel`

.. seealso:: :py:func:`blurMethod`
%End

    void setBlurMethod( const BlurMethod method );
%Docstring
Sets the blur method (algorithm) to use for performing the blur.

:param method: blur method

.. seealso:: :py:func:`blurMethod`
%End

    BlurMethod blurMethod() const;
%Docstring
Returns the blur method (algorithm) used for performing the blur.

:return: blur method

.. seealso:: :py:func:`setBlurMethod`
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

    virtual void draw( QgsRenderContext &context );

    virtual QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgsblureffect.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
