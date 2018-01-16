/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgspainteffect.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsPaintEffect
{
%Docstring
 Base class for visual effects which can be applied to QPicture drawings

QgsPaintEffect objects can be used to modify QPicture drawings prior to rendering
them with a QPainter operation. There are two methods for drawing using an effect,
either drawing a picture directly, or by intercepting drawing operations to a
render context.

To directly draw a picture, use the render() method with a source
QPicture and destination render context.

Intercepting drawing operations to a render context is achieved by first calling
the begin() method, passing a render context. Any drawing operations
performed on the render context will not directly affect the context's paint
device. When the drawing operations have been completed, call the end()
method. This will perform the paint effect on the intercepted drawing operations
and render the result to the render context's paint device.

.. seealso:: :py:class:`QgsPaintEffectRegistry`

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgspainteffect.h"
%End
%ConvertToSubClassCode
    if ( sipCpp->type() == "drawSource" && dynamic_cast<QgsDrawSourceEffect *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsDrawSourceEffect;
    }
    else if ( sipCpp->type() == "effectStack" && dynamic_cast<QgsEffectStack *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsEffectStack;
    }
    else if ( sipCpp->type() == "blur" && dynamic_cast<QgsBlurEffect *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsBlurEffect;
    }
    else if ( sipCpp->type() == "dropShadow" && dynamic_cast<QgsDropShadowEffect *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsDropShadowEffect;
    }
    else if ( sipCpp->type() == "outerGlow" && dynamic_cast<QgsOuterGlowEffect *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsOuterGlowEffect;
    }
    else if ( sipCpp->type() == "innerGlow" && dynamic_cast<QgsInnerGlowEffect *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsInnerGlowEffect;
    }
    else if ( sipCpp->type() == "transform" && dynamic_cast<QgsTransformEffect *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsTransformEffect;
    }
    else if ( sipCpp->type() == "color" && dynamic_cast<QgsColorEffect *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsColorEffect;
    }
    else
    {
      sipType = 0;
    }
%End
  public:

    enum DrawMode
    {
      Modifier,
      Render,
      ModifyAndRender
    };

    QgsPaintEffect();
%Docstring
Constructor for QgsPaintEffect.
%End

    QgsPaintEffect( const QgsPaintEffect &other );
    virtual ~QgsPaintEffect();

    virtual QString type() const = 0;
%Docstring
Returns the effect type.

:return: unique string representation of the effect type
%End

    virtual QgsPaintEffect *clone() const = 0 /Factory/;
%Docstring
Duplicates an effect by creating a deep copy of the effect

:return: clone of paint effect
%End

    virtual QgsStringMap properties() const = 0;
%Docstring
Returns the properties describing the paint effect encoded in a
string format.

:return: string map of properties, in the form property key, value

.. seealso:: :py:func:`readProperties`

.. seealso:: :py:func:`saveProperties`
%End

    virtual void readProperties( const QgsStringMap &props ) = 0;
%Docstring
Reads a string map of an effect's properties and restores the effect
to the state described by the properties map.

:param props: effect properties encoded in a string map

.. seealso:: :py:func:`properties`
%End

    virtual bool saveProperties( QDomDocument &doc, QDomElement &element ) const;
%Docstring
Saves the current state of the effect to a DOM element. The default
behavior is to save the properties string map returned by
properties().

:param doc: destination DOM document
:param element: destination DOM element

:return: true if save was successful

.. seealso:: :py:func:`readProperties`
%End

    virtual bool readProperties( const QDomElement &element );
%Docstring
Restores the effect to the state described by a DOM element.

:param element: DOM element describing an effect's state

:return: true if read was successful

.. seealso:: :py:func:`saveProperties`
%End

    virtual void render( QPicture &picture, QgsRenderContext &context );
%Docstring
Renders a picture using the effect.

:param picture: source QPicture to render
:param context: destination render context

.. seealso:: :py:func:`begin`
%End

    virtual void begin( QgsRenderContext &context );
%Docstring
Begins intercepting paint operations to a render context. When the corresponding
end() member is called all intercepted paint operations will be
drawn to the render context after being modified by the effect.

:param context: destination render context

.. seealso:: :py:func:`end`

.. seealso:: :py:func:`render`
%End

    virtual void end( QgsRenderContext &context );
%Docstring
Ends interception of paint operations to a render context, and draws the result
to the render context after being modified by the effect.

:param context: destination render context

.. seealso:: :py:func:`begin`
%End

    bool enabled() const;
%Docstring
Returns whether the effect is enabled

:return: true if effect is enabled

.. seealso:: :py:func:`setEnabled`
%End

    void setEnabled( const bool enabled );
%Docstring
Sets whether the effect is enabled

:param enabled: set to false to disable the effect

.. seealso:: :py:func:`enabled`
%End

    DrawMode drawMode() const;
%Docstring
Returns the draw mode for the effect. This property only has an
effect if the paint effect is used in a :py:class:`QgsEffectStack`.

:return: draw mode for effect

.. seealso:: :py:func:`setDrawMode`
%End

    void setDrawMode( const DrawMode drawMode );
%Docstring
Sets the draw mode for the effect. This property only has an
effect if the paint effect is used in a :py:class:`QgsEffectStack`.

:param drawMode: draw mode for effect

.. seealso:: :py:func:`drawMode`
%End

  protected:


    virtual void draw( QgsRenderContext &context ) = 0;
%Docstring
Handles drawing of the effect's result on to the specified render context.
Derived classes must reimplement this method to apply any transformations to
the source QPicture and draw the result using the context's painter.

:param context: destination render context

.. seealso:: :py:func:`drawSource`
%End

    void drawSource( QPainter &painter );
%Docstring
Draws the source QPicture onto the specified painter. Handles scaling of the picture
to account for the destination painter's DPI.

:param painter: destination painter

.. seealso:: :py:func:`source`

.. seealso:: :py:func:`sourceAsImage`
%End

    const QPicture *source() const;
%Docstring
Returns the source QPicture. The draw() member can utilize this when
drawing the effect.

:return: source QPicture

.. seealso:: :py:func:`drawSource`

.. seealso:: :py:func:`sourceAsImage`
%End

    QImage *sourceAsImage( QgsRenderContext &context );
%Docstring
Returns the source QPicture rendered to a new QImage. The draw() member can
utilize this when drawing the effect. The image will be padded or cropped from the original
source QPicture by the results of the boundingRect() method.
The result is cached to speed up subsequent calls to sourceAsImage.

:return: source QPicture rendered to an image

.. seealso:: :py:func:`drawSource`

.. seealso:: :py:func:`source`

.. seealso:: :py:func:`imageOffset`

.. seealso:: :py:func:`boundingRect`
%End

    QPointF imageOffset( const QgsRenderContext &context ) const;
%Docstring
Returns the offset which should be used when drawing the source image on to a destination
render context.

:param context: destination render context

:return: point offset for image top left corner

.. seealso:: :py:func:`sourceAsImage`
%End

    virtual QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const;
%Docstring
Returns the bounding rect required for drawing the effect. This method can be used
to expand the bounding rect of a source picture to account for offset or blurring
effects.

:param rect: original source bounding rect
:param context: destination render context

:return: modified bounding rect

.. seealso:: :py:func:`sourceAsImage`
%End

    void fixQPictureDpi( QPainter *painter ) const;
%Docstring
Applies a workaround to a QPainter to avoid an issue with incorrect scaling
when drawing QPictures. This may need to be called by derived classes prior
to rendering results onto a painter.

:param painter: destination painter
%End

};


class QgsDrawSourceEffect : QgsPaintEffect
{
%Docstring
 A paint effect which draws the source picture with minor or no alterations

The draw source effect can be used to draw an unaltered copy of the original source
picture. Minor changes like lowering the opacity and applying a blend mode are
supported, however these changes will force the resultant output to be rasterized.
If no alterations are performed then the original picture will be rendered as a vector.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgspainteffect.h"
%End
  public:

    QgsDrawSourceEffect();
%Docstring
Constructor for QgsDrawSourceEffect
%End

    static QgsPaintEffect *create( const QgsStringMap &map ) /Factory/;
%Docstring
Creates a new QgsDrawSource effect from a properties string map.

:param map: encoded properties string map

:return: new QgsDrawSourceEffect
%End

    virtual QString type() const;
    virtual QgsDrawSourceEffect *clone() const /Factory/;

    virtual QgsStringMap properties() const;

    virtual void readProperties( const QgsStringMap &props );


    void setOpacity( const double opacity );
%Docstring
Sets the ``opacity`` for the effect.

:param opacity: double between 0 and 1 inclusive, where 0 is fully transparent
and 1 is fully opaque

.. seealso:: :py:func:`opacity`
%End

    double opacity() const;
%Docstring
Returns the opacity for the effect

:return: opacity value between 0 and 1 inclusive, where 0 is fully transparent
and 1 is fully opaque

.. seealso:: :py:func:`setOpacity`
%End

    void setBlendMode( const QPainter::CompositionMode mode );
%Docstring
Sets the blend mode for the effect

:param mode: blend mode used for drawing the source on to a destination
paint device

.. seealso:: :py:func:`blendMode`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Returns the blend mode for the effect

:return: blend mode used for drawing the source on to a destination
paint device

.. seealso:: :py:func:`setBlendMode`
%End

  protected:

    virtual void draw( QgsRenderContext &context );


};

class QgsEffectPainter
{
%Docstring
 A class to manager painter saving and restoring required for effect drawing

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspainteffect.h"
%End
  public:

    QgsEffectPainter( QgsRenderContext &renderContext );
%Docstring
QgsEffectPainter constructor

:param renderContext: the QgsRenderContext object

.. versionadded:: 3.0
%End

    QgsEffectPainter( QgsRenderContext &renderContext, QgsPaintEffect *effect );
%Docstring
QgsEffectPainter constructor alternative if no painter translation is needed

:param renderContext: the QgsRenderContext object
:param effect: the QgsPaintEffect object

.. versionadded:: 3.0
%End
    ~QgsEffectPainter();

    void setEffect( QgsPaintEffect *effect );
%Docstring
Sets the effect to be painted

:param effect: the QgsPaintEffect object
%End



};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgspainteffect.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
