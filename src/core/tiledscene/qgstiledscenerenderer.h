/***************************************************************************
                         qgstiledscenerenderer.h
                         --------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#ifndef QGSTILEDSCENERENDERER_H
#define QGSTILEDSCENERENDERER_H

#include "qgsrendercontext.h"

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsLayerTreeLayer;
class QgsLayerTreeModelLegendNode;

/**
 * \ingroup core
 * \class QgsTiledSceneRenderContext
 *
 * \brief Encapsulates the render context for a 2D tiled scene rendering operation.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneRenderContext
{
  public:

    /**
     * Constructor for QgsTiledSceneRenderContext.
     */
    QgsTiledSceneRenderContext( QgsRenderContext &context, QgsFeedback *feedback = nullptr );

    QgsTiledSceneRenderContext( const QgsTiledSceneRenderContext &rh ) = delete;
    QgsTiledSceneRenderContext &operator=( const QgsTiledSceneRenderContext & ) = delete;

    /**
     * Returns a reference to the context's render context.
     */
    QgsRenderContext &renderContext() { return mRenderContext; }

    /**
     * Returns a reference to the context's render context.
     * \note Not available in Python bindings.
     */
    const QgsRenderContext &renderContext() const SIP_SKIP { return mRenderContext; }

    /**
     * Returns the feedback object used to cancel rendering
     *
     * \since QGIS 3.20
     */
    QgsFeedback *feedback() const { return mFeedback; }

    /**
     * Returns the current texture image.
     *
     * \see setTextureImage()
     */
    QImage textureImage() const;

    /**
     * Sets the current texture \a image.
     *
     * \see textureImage()
     */
    void setTextureImage( const QImage &image );

    /**
     * Sets the current texture coordinates.
     *
     * \see textureCoordinates()
     */
    void setTextureCoordinates(
      float textureX1, float textureY1,
      float textureX2, float textureY2,
      float textureX3, float textureY3
    );

    /**
     * Returns the current texture coordinates.
     *
     * \see setTextureCoordinates()
     */
    void textureCoordinates( float &textureX1 SIP_OUT, float &textureY1 SIP_OUT, float &textureX2 SIP_OUT, float &textureY2 SIP_OUT, float &textureX3 SIP_OUT, float &textureY3 SIP_OUT ) const;

  private:
#ifdef SIP_RUN
    QgsTiledSceneRenderContext( const QgsTiledSceneRenderContext &rh );
#endif

    QgsRenderContext &mRenderContext;
    QgsFeedback *mFeedback = nullptr;
    QImage mTextureImage;
    float mTextureCoordinates[6] { 0, 0, 0, 0, 0, 0 };

};

/**
 * \ingroup core
 * \class QgsTiledSceneRenderer
 *
 * \brief Abstract base class for 2d tiled scene renderers.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneRenderer
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE

    const QString type = sipCpp->type();
    if ( type == QLatin1String( "texture" ) )
      sipType = sipType_QgsTiledSceneTextureRenderer;
    else if ( type == QLatin1String( "wireframe" ) )
      sipType = sipType_QgsTiledSceneWireframeRenderer;
    else
      sipType = 0;

    SIP_END
#endif

  public:

    QgsTiledSceneRenderer() = default;

    virtual ~QgsTiledSceneRenderer() = default;

    /**
     * Returns the identifier of the renderer type.
     */
    virtual QString type() const = 0;

    /**
     * Create a deep copy of this renderer. Should be implemented by all subclasses
     * and generate a proper subclass.
     */
    virtual QgsTiledSceneRenderer *clone() const = 0 SIP_FACTORY;

    //! QgsTiledSceneRenderer cannot be copied -- use clone() instead
    QgsTiledSceneRenderer( const QgsTiledSceneRenderer &other ) = delete;

    //! QgsTiledSceneRenderer cannot be copied -- use clone() instead
    QgsTiledSceneRenderer &operator=( const QgsTiledSceneRenderer &other ) = delete;

    /**
     * Returns flags which control how the renderer behaves.
     */
    virtual Qgis::TiledSceneRendererFlags flags() const;

    /**
     * Creates a renderer from an XML \a element.
     *
     * Caller takes ownership of the returned renderer.
     *
     * \see save()
     */
    static QgsTiledSceneRenderer *load( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Saves the renderer configuration to an XML element.
     * \see load()
     */
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;

    /**
     * Returns the maximum screen error allowed when rendering the tiled scene.
     *
     * Larger values result in a faster render with less detailed features rendered.
     *
     * Units are retrieved via maximumScreenErrorUnit().
     *
     * \see setMaximumScreenError()
     * \see maximumScreenErrorUnit()
     */
    double maximumScreenError() const;

    /**
     * Sets the maximum screen \a error allowed when rendering the tiled scene.
     *
     * Larger values result in a faster render with less detailed features rendered.
     *
     * Units are set via setMaximumScreenErrorUnit().
     *
     * \see maximumScreenError()
     * \see setMaximumScreenErrorUnit()
     */
    void setMaximumScreenError( double error );

    /**
     * Returns the unit for the maximum screen error allowed when rendering the tiled scene.
     *
     * \see maximumScreenError()
     * \see setMaximumScreenErrorUnit()
     */
    Qgis::RenderUnit maximumScreenErrorUnit() const;

    /**
     * Sets the \a unit for the maximum screen error allowed when rendering the tiled scene.
     *
     * \see setMaximumScreenError()
     * \see maximumScreenErrorUnit()
     */
    void setMaximumScreenErrorUnit( Qgis::RenderUnit unit );

    /**
     * Sets whether to render the borders of tiles.
     *
     * \see isTileBorderRenderingEnabled()
     */
    void setTileBorderRenderingEnabled( bool enabled ) { mTileBorderRendering = enabled; }

    /**
     * Returns whether to render also borders of tiles.
     *
     * see setTileBorderRenderingEnabled()
     */
    bool isTileBorderRenderingEnabled() const { return mTileBorderRendering; }

    /**
     * Must be called when a new render cycle is started. A call to startRender() must always
     * be followed by a corresponding call to stopRender() after all features have been rendered.
     *
     * \see stopRender()
     *
     * \warning This method is not thread safe. Before calling startRender() in a non-main thread,
     * the renderer should instead be cloned and startRender()/stopRender() called on the clone.
     */
    virtual void startRender( QgsTiledSceneRenderContext &context );

    /**
     * Must be called when a render cycle has finished, to allow the renderer to clean up.
     *
     * Calls to stopRender() must always be preceded by a call to startRender().
     *
     * \warning This method is not thread safe. Before calling startRender() in a non-main thread,
     * the renderer should instead be cloned and startRender()/stopRender() called on the clone.
     *
     * \see startRender()
     */
    virtual void stopRender( QgsTiledSceneRenderContext &context );

    /**
     * Creates a set of legend nodes representing the renderer.
     */
    virtual QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY;

    /**
     * Returns a list of all rule keys for legend nodes created by the renderer.
     */
    virtual QStringList legendRuleKeys() const;

    /**
     * Renders a \a triangle.
     */
    virtual void renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle ) = 0;

    /**
     * Renders a \a line.
     */
    virtual void renderLine( QgsTiledSceneRenderContext &context, const QPolygonF &line ) = 0;

  protected:

    /**
     * Copies common tiled scene renderer properties (such as screen error) to the \a destination renderer.
     */
    void copyCommonProperties( QgsTiledSceneRenderer *destination ) const;

    /**
     * Restores common renderer properties (such as screen error) from the
     * specified DOM \a element.
     *
     * \see saveCommonProperties()
     */
    void restoreCommonProperties( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Saves common renderer properties (such as point size and screen error) to the
     * specified DOM \a element.
     *
     * \see restoreCommonProperties()
     */
    void saveCommonProperties( QDomElement &element, const QgsReadWriteContext &context ) const;

  private:
#ifdef SIP_RUN
    QgsTiledSceneRenderer( const QgsTiledSceneRenderer &other );
#endif

#ifdef QGISDEBUG
    //! Pointer to thread in which startRender was first called
    QThread *mThread = nullptr;
#endif

    double mMaximumScreenError = 3;
    Qgis::RenderUnit mMaximumScreenErrorUnit = Qgis::RenderUnit::Millimeters;
    bool mTileBorderRendering = false;

};

#endif // QGSTILEDSCENERENDERER_H
