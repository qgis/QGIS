/***************************************************************************
                             qgspainteffect.h
                             ----------------
    begin                : December 2014
    copyright            : (C) 2014 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPAINTEFFECT_H
#define QGSPAINTEFFECT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QPainter>
#include <QDomDocument>
#include <QDomElement>

class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsPaintEffect
 * \brief Base class for visual effects which can be applied to QPicture drawings
 *
 * QgsPaintEffect objects can be used to modify QPicture drawings prior to rendering
 * them with a QPainter operation. There are two methods for drawing using an effect,
 * either drawing a picture directly, or by intercepting drawing operations to a
 * render context.
 *
 * To directly draw a picture, use the render() method with a source
 * QPicture and destination render context.
 *
 * Intercepting drawing operations to a render context is achieved by first calling
 * the begin() method, passing a render context. Any drawing operations
 * performed on the render context will not directly affect the context's paint
 * device. When the drawing operations have been completed, call the end()
 * method. This will perform the paint effect on the intercepted drawing operations
 * and render the result to the render context's paint device.
 *
 * \see QgsPaintEffectRegistry
 */

class CORE_EXPORT QgsPaintEffect SIP_NODEFAULTCTORS
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
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
    SIP_END
#endif

  public:

    /**
     * Drawing modes for effects. These modes are used only when effects are
     * drawn as part of an effects stack
     * \see QgsEffectStack
     */
    enum DrawMode
    {
      Modifier, //!< The result of the effect is not rendered, but is passed on to following effects in the stack
      Render, //!< The result of the effect is rendered on the destination, but does not affect subsequent effects in the stack
      ModifyAndRender //!< The result of the effect is both rendered and passed on to subsequent effects in the stack
    };

    QgsPaintEffect() = default;

    QgsPaintEffect( const QgsPaintEffect &other );
    virtual ~QgsPaintEffect();

    /**
     * Returns the effect type.
     * \returns unique string representation of the effect type
     */
    virtual QString type() const = 0;

    /**
     * Duplicates an effect by creating a deep copy of the effect
     * \returns clone of paint effect
     */
    virtual QgsPaintEffect *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the properties describing the paint effect encoded in a
     * string format.
     * \returns string map of properties, in the form property key, value
     * \see readProperties
     * \see saveProperties
     */
    virtual QVariantMap properties() const = 0;

    /**
     * Reads a string map of an effect's properties and restores the effect
     * to the state described by the properties map.
     * \param props effect properties encoded in a string map
     * \see properties
     */
    virtual void readProperties( const QVariantMap &props ) = 0;

    /**
     * Saves the current state of the effect to a DOM element. The default
     * behavior is to save the properties string map returned by
     * properties().
     * \param doc destination DOM document
     * \param element destination DOM element
     * \returns TRUE if save was successful
     * \see readProperties
     */
    virtual bool saveProperties( QDomDocument &doc, QDomElement &element ) const;

    /**
     * Restores the effect to the state described by a DOM element.
     * \param element DOM element describing an effect's state
     * \returns TRUE if read was successful
     * \see saveProperties
     */
    virtual bool readProperties( const QDomElement &element );

    /**
     * Renders a picture using the effect.
     * \param picture source QPicture to render
     * \param context destination render context
     * \see begin
     */
    virtual void render( QPicture &picture, QgsRenderContext &context );

    /**
     * Begins intercepting paint operations to a render context. When the corresponding
     * end() member is called all intercepted paint operations will be
     * drawn to the render context after being modified by the effect.
     * \param context destination render context
     * \see end
     * \see render
     */
    virtual void begin( QgsRenderContext &context );

    /**
     * Ends interception of paint operations to a render context, and draws the result
     * to the render context after being modified by the effect.
     * \param context destination render context
     * \see begin
     */
    virtual void end( QgsRenderContext &context );

    /**
     * Returns whether the effect is enabled
     * \returns TRUE if effect is enabled
     * \see setEnabled
     */
    bool enabled() const { return mEnabled; }

    /**
     * Sets whether the effect is enabled
     * \param enabled set to FALSE to disable the effect
     * \see enabled
     */
    void setEnabled( bool enabled );

    /**
     * Returns the draw mode for the effect. This property only has an
     * effect if the paint effect is used in a QgsEffectStack.
     * \returns draw mode for effect
     * \see setDrawMode
     */
    DrawMode drawMode() const { return mDrawMode; }

    /**
     * Sets the draw mode for the effect. This property only has an
     * effect if the paint effect is used in a QgsEffectStack.
     * \param drawMode draw mode for effect
     * \see drawMode
     */
    void setDrawMode( DrawMode drawMode );

  protected:

    bool mEnabled = true;
    DrawMode mDrawMode = ModifyAndRender;
    bool requiresQPainterDpiFix = true;

    /**
     * Handles drawing of the effect's result on to the specified render context.
     * Derived classes must reimplement this method to apply any transformations to
     * the source QPicture and draw the result using the context's painter.
     * \param context destination render context
     * \see drawSource
     */
    virtual void draw( QgsRenderContext &context ) = 0;

    /**
     * Draws the source QPicture onto the specified painter. Handles scaling of the picture
     * to account for the destination painter's DPI.
     * \param painter destination painter
     * \see source
     * \see sourceAsImage
     */
    void drawSource( QPainter &painter );

    /**
     * Returns the source QPicture. The draw() member can utilize this when
     * drawing the effect.
     * \returns source QPicture
     * \see drawSource
     * \see sourceAsImage
     */
    const QPicture *source() const { return mPicture; }

    /**
     * Returns the source QPicture rendered to a new QImage. The draw() member can
     * utilize this when drawing the effect. The image will be padded or cropped from the original
     * source QPicture by the results of the boundingRect() method.
     * The result is cached to speed up subsequent calls to sourceAsImage.
     * \returns source QPicture rendered to an image
     * \see drawSource
     * \see source
     * \see imageOffset
     * \see boundingRect
     */
    QImage *sourceAsImage( QgsRenderContext &context );

    /**
     * Returns the offset which should be used when drawing the source image on to a destination
     * render context.
     * \param context destination render context
     * \returns point offset for image top left corner
     * \see sourceAsImage
     */
    QPointF imageOffset( const QgsRenderContext &context ) const;

    /**
     * Returns the bounding rect required for drawing the effect. This method can be used
     * to expand the bounding rect of a source picture to account for offset or blurring
     * effects.
     * \param rect original source bounding rect
     * \param context destination render context
     * \returns modified bounding rect
     * \see sourceAsImage
     */
    virtual QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const;

    /**
     * Applies a workaround to a QPainter to avoid an issue with incorrect scaling
     * when drawing QPictures. This may need to be called by derived classes prior
     * to rendering results onto a painter.
     * \param painter destination painter
     *
     * \deprecated QGIS 3.40, use QgsPainting::drawPicture() or QgsPainting::applyScaleFixForQPictureDpi() instead.
     */
    Q_DECL_DEPRECATED void fixQPictureDpi( QPainter *painter ) const SIP_DEPRECATED;

  private:

    const QPicture *mPicture = nullptr;
    QImage *mSourceImage = nullptr;
    bool mOwnsImage = false;

    QPainter *mPrevPainter = nullptr;
    QPainter *mEffectPainter = nullptr;
    QPicture *mTempPicture = nullptr;

    QRectF imageBoundingRect( const QgsRenderContext &context ) const;

    friend class QgsEffectStack;

    QgsPaintEffect &operator= ( const QgsPaintEffect & ) = delete;

};

/**
 * \ingroup core
 * \class QgsDrawSourceEffect
 * \brief A paint effect which draws the source picture with minor or no alterations
 *
 * The draw source effect can be used to draw an unaltered copy of the original source
 * picture. Minor changes like lowering the opacity and applying a blend mode are
 * supported, however these changes will force the resultant output to be rasterized.
 * If no alterations are performed then the original picture will be rendered as a vector.
 *
 */

class CORE_EXPORT QgsDrawSourceEffect : public QgsPaintEffect SIP_NODEFAULTCTORS
{
  public:

    QgsDrawSourceEffect() = default;

    /**
     * Creates a new QgsDrawSource effect from a properties string map.
     * \param map encoded properties string map
     * \returns new QgsDrawSourceEffect
     */
    static QgsPaintEffect *create( const QVariantMap &map ) SIP_FACTORY;

    QString type() const override { return QStringLiteral( "drawSource" ); }
    QgsDrawSourceEffect *clone() const override SIP_FACTORY;
    QVariantMap properties() const override;
    void readProperties( const QVariantMap &props ) override;

    /**
     * Sets the \a opacity for the effect.
     * \param opacity double between 0 and 1 inclusive, where 0 is fully transparent
     * and 1 is fully opaque
     * \see opacity()
     */
    void setOpacity( const double opacity ) { mOpacity = opacity; }

    /**
     * Returns the opacity for the effect
     * \returns opacity value between 0 and 1 inclusive, where 0 is fully transparent
     * and 1 is fully opaque
     * \see setOpacity()
     */
    double opacity() const { return mOpacity; }

    /**
     * Sets the blend mode for the effect
     * \param mode blend mode used for drawing the source on to a destination
     * paint device
     * \see blendMode
     */
    void setBlendMode( const QPainter::CompositionMode mode ) { mBlendMode = mode; }

    /**
     * Returns the blend mode for the effect
     * \returns blend mode used for drawing the source on to a destination
     * paint device
     * \see setBlendMode
     */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

  protected:

    void draw( QgsRenderContext &context ) override;

  private:

    double mOpacity = 1.0;
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;
};

/**
 * \ingroup core
 * \class QgsEffectPainter
 * \brief A class to manager painter saving and restoring required for effect drawing
 *
 */
class CORE_EXPORT QgsEffectPainter
{
  public:

    /**
     * QgsEffectPainter constructor
     *
     * \param renderContext the QgsRenderContext object
     */
    QgsEffectPainter( QgsRenderContext &renderContext );

    /**
     * QgsEffectPainter constructor alternative if no painter translation is needed
     *
     * \param renderContext the QgsRenderContext object
     * \param effect the QgsPaintEffect object
     */
    QgsEffectPainter( QgsRenderContext &renderContext, QgsPaintEffect *effect );
    ~QgsEffectPainter();

    /**
     * Sets the effect to be painted
     *
     * \param effect the QgsPaintEffect object
     */
    void setEffect( QgsPaintEffect *effect );

    ///@cond PRIVATE

    /**
     * Access to the painter object
     *
     */
    QPainter *operator->() { return mPainter; }
    ///@endcond

  private:
#ifdef SIP_RUN
    const QgsEffectPainter &operator=( const QgsEffectPainter & );
#endif

    QgsRenderContext &mRenderContext;
    QPainter *mPainter = nullptr;
    QgsPaintEffect *mEffect = nullptr;
};

#endif // QGSPAINTEFFECT_H

