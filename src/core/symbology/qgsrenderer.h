/***************************************************************************
    qgsrenderer.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERER_H
#define QGSRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgssymbol.h"
#include "qgsfields.h"
#include "qgsfeaturerequest.h"

#include <QList>
#include <QString>
#include <QVariant>
#include <QPair>
#include <QPixmap>
#include <QDomDocument>
#include <QDomElement>

class QgsFeature;
class QgsVectorLayer;
class QgsPaintEffect;
class QgsReadWriteContext;

typedef QMap<QString, QString> QgsStringMap SIP_SKIP;

typedef QList<QgsSymbol *> QgsSymbolList;
typedef QMap<QString, QgsSymbol * > QgsSymbolMap SIP_SKIP;

#include "qgslegendsymbolitem.h"


#define RENDERER_TAG_NAME   "renderer-v2"

////////
// symbol levels

/**
 * \ingroup core
 * \class QgsSymbolLevelItem
 */
class CORE_EXPORT QgsSymbolLevelItem
{
  public:
    QgsSymbolLevelItem( QgsSymbol *symbol, int layer )
      : mSymbol( symbol )
      , mLayer( layer )
    {}
    QgsSymbol *symbol() { return mSymbol; }
    int layer() { return mLayer; }
  protected:
    QgsSymbol *mSymbol = nullptr;
    int mLayer;
};

// every level has list of items: symbol + symbol layer num
typedef QList< QgsSymbolLevelItem > QgsSymbolLevel;

// this is a list of levels
#ifndef SIP_RUN
typedef QList< QgsSymbolLevel > QgsSymbolLevelOrder;
#else
typedef QList< QList< QgsSymbolLevelItem > > QgsSymbolLevelOrder;
#endif


//////////////
// renderers

/**
 * \ingroup core
 * \class QgsFeatureRenderer
 */
class CORE_EXPORT QgsFeatureRenderer
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE

    const QString type = sipCpp->type();

    if ( type == QStringLiteral( "singleSymbol" ) )
      sipType = sipType_QgsSingleSymbolRenderer;
    else if ( type == QStringLiteral( "categorizedSymbol" ) )
      sipType = sipType_QgsCategorizedSymbolRenderer;
    else if ( type == QStringLiteral( "graduatedSymbol" ) )
      sipType = sipType_QgsGraduatedSymbolRenderer;
    else if ( type == QStringLiteral( "RuleRenderer" ) )
      sipType = sipType_QgsRuleBasedRenderer;
    else if ( type == QStringLiteral( "heatmapRenderer" ) )
      sipType = sipType_QgsHeatmapRenderer;
    else if ( type == QStringLiteral( "invertedPolygonRenderer" ) )
      sipType = sipType_QgsInvertedPolygonRenderer;
    else if ( type == QStringLiteral( "pointCluster" ) )
      sipType = sipType_QgsPointClusterRenderer;
    else if ( type == QStringLiteral( "pointDisplacement" ) )
      sipType = sipType_QgsPointDisplacementRenderer;
    else if ( type == QStringLiteral( "25dRenderer" ) )
      sipType = sipType_Qgs25DRenderer;
    else if ( type == QStringLiteral( "nullSymbol" ) )
      sipType = sipType_QgsNullSymbolRenderer;
    else
      sipType = 0;
    SIP_END
#endif

  public:
    // renderer takes ownership of its symbols!

    //! Returns a new renderer - used by default in vector layers
    static QgsFeatureRenderer *defaultRenderer( QgsWkbTypes::GeometryType geomType ) SIP_FACTORY;

    QString type() const { return mType; }

    /**
     * To be overridden
     *
     * Must be called between startRender() and stopRender() calls.
     * \param feature feature
     * \param context render context
     * \returns returns pointer to symbol or 0 if symbol was not found
     * \since QGIS 2.12
     */
    virtual QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const = 0;

    /**
     * Returns symbol for feature. The difference compared to symbolForFeature() is that it returns original
     * symbol which can be used as an identifier for renderer's rule - the former may return a temporary replacement
     * of a symbol for use in rendering.
     * \since QGIS 2.12
     */
    virtual QgsSymbol *originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const;

    /**
     * Returns legend keys matching a specified feature.
     * \since QGIS 2.14
     */
    virtual QSet< QString > legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const;

    /**
     * Must be called when a new render cycle is started. A call to startRender() must always
     * be followed by a corresponding call to stopRender() after all features have been rendered.
     *
     * \param context  Additional information passed to the renderer about the job which will be rendered
     * \param fields   The fields available for rendering
     *
     * \see stopRender()
     *
     * \warning This method is not thread safe. Before calling startRender() in a non-main thread,
     * the renderer should instead be cloned and startRender()/stopRender() called on the clone.
     */
    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );

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
    virtual void stopRender( QgsRenderContext &context );

    /**
     * If a renderer does not require all the features this method may be overridden
     * and return an expression used as where clause.
     * This will be called once after startRender() and before the first call
     * to renderFeature().
     * By default this returns a null string and all features will be requested.
     * You do not need to specify the extent in here, this is taken care of separately and
     * will be combined with a filter returned from this method.
     *
     * \returns An expression used as where clause
     */
    virtual QString filter( const QgsFields &fields = QgsFields() ) { Q_UNUSED( fields ); return QString(); }

    /**
     * Returns a list of attributes required by this renderer. Attributes not listed in here may
     * not have been requested from the provider at rendering time.
     *
     * \returns A set of attributes
     */
    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const = 0;

    /**
     * Returns TRUE if this renderer requires the geometry to apply the filter.
     */
    virtual bool filterNeedsGeometry() const;

    virtual ~QgsFeatureRenderer();

    /**
     * Create a deep copy of this renderer. Should be implemented by all subclasses
     * and generate a proper subclass.
     *
     * \returns A copy of this renderer
     */
    virtual QgsFeatureRenderer *clone() const = 0 SIP_FACTORY;

    /**
     * Render a feature using this renderer in the given context.
     * Must be called between startRender() and stopRender() calls.
     * Default implementation renders a symbol as determined by symbolForFeature() call.
     * Returns TRUE if the feature has been returned (this is used for example
     * to determine whether the feature may be labelled).
     *
     * If layer is not -1, the renderer should draw only a particula layer from symbols
     * (in order to support symbol level rendering).
     *
     * \see startRender()
     * \see stopRender()
     */
    virtual bool renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    //! Returns debug information about this renderer
    virtual QString dump() const;

    /**
     * Used to specify details about a renderer.
     * Is returned from the capabilities() method.
     */
    enum Capability
    {
      SymbolLevels          = 1,      //!< Rendering with symbol levels (i.e. implements symbols(), symbolForFeature())
      MoreSymbolsPerFeature = 1 << 2, //!< May use more than one symbol to render a feature: symbolsForFeature() will return them
      Filter                = 1 << 3, //!< Features may be filtered, i.e. some features may not be rendered (categorized, rule based ...)
      ScaleDependent        = 1 << 4  //!< Depends on scale if feature will be rendered (rule based )
    };

    Q_DECLARE_FLAGS( Capabilities, Capability )

    /**
     * Returns details about internals of this renderer.
     *
     * E.g. if you only want to deal with visible features:
     *
     * \code{.py}
     * if not renderer.capabilities().testFlag(QgsFeatureRenderer.Filter) or renderer.willRenderFeature(feature, context):
     *     deal_with_my_feature()
     * else:
     *     skip_the_curren_feature()
     * \endcode
     */
    virtual QgsFeatureRenderer::Capabilities capabilities() { return nullptr; }

    /**
     * Returns list of symbols used by the renderer.
     * \param context render context
     * \since QGIS 2.12
     */
    virtual QgsSymbolList symbols( QgsRenderContext &context ) const;

    bool usingSymbolLevels() const { return mUsingSymbolLevels; }
    void setUsingSymbolLevels( bool usingSymbolLevels ) { mUsingSymbolLevels = usingSymbolLevels; }

    //! create a renderer from XML element
    static QgsFeatureRenderer *load( QDomElement &symbologyElem, const QgsReadWriteContext &context ) SIP_FACTORY;

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );

    /**
     * create the SLD UserStyle element following the SLD v1.1 specs with the given name
     * \since QGIS 2.8
     */
    virtual QDomElement writeSld( QDomDocument &doc, const QString &styleName, const QgsStringMap &props = QgsStringMap() ) const;

    /**
     * Create a new renderer according to the information contained in
     * the UserStyle element of a SLD style document
     * \param node the node in the SLD document whose the UserStyle element
     * is a child
     * \param geomType the geometry type of the features, used to convert
     * Symbolizer elements
     * \param errorMessage it will contain the error message if something
     * went wrong
     * \returns the renderer
     */
    static QgsFeatureRenderer *loadSld( const QDomNode &node, QgsWkbTypes::GeometryType geomType, QString &errorMessage ) SIP_FACTORY;

    //! used from subclasses to create SLD Rule elements following SLD v1.1 specs
    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const
    {
      element.appendChild( doc.createComment( QStringLiteral( "FeatureRenderer %1 not implemented yet" ).arg( type() ) ) );
      ( void ) props; // warning avoidance
    }

    /**
     * items of symbology items in legend should be checkable
     * \since QGIS 2.5
     */
    virtual bool legendSymbolItemsCheckable() const;

    /**
     * items of symbology items in legend is checked
     * \since QGIS 2.5
     */
    virtual bool legendSymbolItemChecked( const QString &key );

    /**
     * item in symbology was checked
     * \since QGIS 2.5
     */
    virtual void checkLegendSymbolItem( const QString &key, bool state = true );

    /**
     * Sets the symbol to be used for a legend symbol item.
     * \param key rule key for legend symbol
     * \param symbol new symbol for legend item. Ownership is transferred to renderer.
     * \since QGIS 2.14
     */
    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Returns a list of symbology items for the legend
     * \since QGIS 2.6
     */
    virtual QgsLegendSymbolList legendSymbolItems() const;

    /**
     * If supported by the renderer, return classification attribute for the use in legend
     * \since QGIS 2.6
     */
    virtual QString legendClassificationAttribute() const { return QString(); }

    //! Sets type and size of editing vertex markers for subsequent rendering
    void setVertexMarkerAppearance( int type, double size );

    /**
     * Returns whether the renderer will render a feature or not.
     * Must be called between startRender() and stopRender() calls.
     * Default implementation uses symbolForFeature().
     * \since QGIS 2.12
     */
    virtual bool willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const;

    /**
     * Returns list of symbols used for rendering the feature.
     * For renderers that do not support MoreSymbolsPerFeature it is more efficient
     * to use symbolForFeature()
     * \since QGIS 2.12
     */
    virtual QgsSymbolList symbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const;

    /**
     * Equivalent of originalSymbolsForFeature() call
     * extended to support renderers that may use more symbols per feature - similar to symbolsForFeature()
     * \since QGIS 2.12
     */
    virtual QgsSymbolList originalSymbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const;

    /**
     * Allows for a renderer to modify the extent of a feature request prior to rendering
     * \param extent reference to request's filter extent. Modify extent to change the
     * extent of feature request
     * \param context render context
     * \since QGIS 2.7
     */
    virtual void modifyRequestExtent( QgsRectangle &extent, QgsRenderContext &context );

    /**
     * Returns the current paint effect for the renderer.
     * \returns paint effect
     * \see setPaintEffect
     * \since QGIS 2.9
     */
    QgsPaintEffect *paintEffect() const;

    /**
     * Sets the current paint effect for the renderer.
     * \param effect paint effect. Ownership is transferred to the renderer.
     * \see paintEffect
     * \since QGIS 2.9
     */
    void setPaintEffect( QgsPaintEffect *effect );

    /**
     * Returns whether the renderer must render as a raster.
     * \see setForceRasterRender
     * \since QGIS 2.12
     */
    bool forceRasterRender() const { return mForceRaster; }

    /**
     * Sets whether the renderer should be rendered to a raster destination.
     * \param forceRaster set to TRUE if renderer must be drawn on a raster surface.
     * This may be desirable for highly detailed layers where rendering as a vector
     * would result in a large, complex vector output.
     * \see forceRasterRender
     * \since QGIS 2.12
     */
    void setForceRasterRender( bool forceRaster ) { mForceRaster = forceRaster; }

    /**
     * Gets the order in which features shall be processed by this renderer.
     * \note this property has no effect if orderByEnabled() is FALSE
     * \see orderByEnabled()
     * \since QGIS 2.14
     */
    QgsFeatureRequest::OrderBy orderBy() const;

    /**
     * Define the order in which features shall be processed by this renderer.
     * \note this property has no effect if orderByEnabled() is FALSE
     * \see setOrderByEnabled()
     * \since QGIS 2.14
     */
    void setOrderBy( const QgsFeatureRequest::OrderBy &orderBy );

    /**
     * Returns whether custom ordering will be applied before features are processed by this renderer.
     * \see orderBy()
     * \see setOrderByEnabled()
     * \since QGIS 2.14
     */
    bool orderByEnabled() const;

    /**
     * Sets whether custom ordering should be applied before features are processed by this renderer.
     * \param enabled set to TRUE to enable custom feature ordering
     * \see setOrderBy()
     * \see orderByEnabled()
     * \since QGIS 2.14
     */
    void setOrderByEnabled( bool enabled );

    /**
     * Sets an embedded renderer (subrenderer) for this feature renderer. The base class implementation
     * does nothing with subrenderers, but individual derived classes can use these to modify their behavior.
     * \param subRenderer the embedded renderer. Ownership will be transferred.
     * \see embeddedRenderer()
     * \since QGIS 2.16
     */
    virtual void setEmbeddedRenderer( QgsFeatureRenderer *subRenderer SIP_TRANSFER );

    /**
     * Returns the current embedded renderer (subrenderer) for this feature renderer. The base class
     * implementation does not use subrenderers and will always return NULLPTR.
     * \see setEmbeddedRenderer()
     * \since QGIS 2.16
     */
    virtual const QgsFeatureRenderer *embeddedRenderer() const;

  protected:
    QgsFeatureRenderer( const QString &type );

    /**
     * Render the \a feature with the \a symbol using \a context.
     * Use \a layer to specify the symbol layer, \a selected to
     * specify if it should be rendered as selected and \a drawVertexMarker
     * to specify if vertex markers should be rendered.
     */
    void renderFeatureWithSymbol( const QgsFeature &feature,
                                  QgsSymbol *symbol,
                                  QgsRenderContext &context,
                                  int layer,
                                  bool selected,
                                  bool drawVertexMarker );

    //! render editing vertex marker at specified point
    void renderVertexMarker( QPointF pt, QgsRenderContext &context );
    //! render editing vertex marker for a polyline
    void renderVertexMarkerPolyline( QPolygonF &pts, QgsRenderContext &context );
    //! render editing vertex marker for a polygon
    void renderVertexMarkerPolygon( QPolygonF &pts, QList<QPolygonF> *rings, QgsRenderContext &context );

    /**
     * Creates a point in screen coordinates from a wkb string in map
     * coordinates
     */
    static QPointF _getPoint( QgsRenderContext &context, const QgsPoint &point );

    /**
     * Clones generic renderer data to another renderer.
     * Currently clones
     *  * Order By
     *  * Paint Effect
     *
     * \param destRenderer destination renderer for copied effect
     */
    void copyRendererData( QgsFeatureRenderer *destRenderer ) const;

    QString mType;

    bool mUsingSymbolLevels;

    //! The current type of editing marker
    int mCurrentVertexMarkerType;
    //! The current size of editing marker
    double mCurrentVertexMarkerSize;

    QgsPaintEffect *mPaintEffect = nullptr;

    bool mForceRaster;

    /**
     * \note this function is used to convert old sizeScale expressions to symbol
     * level DataDefined size
     */
    static void convertSymbolSizeScale( QgsSymbol *symbol, QgsSymbol::ScaleMethod method, const QString &field );

    /**
     * \note this function is used to convert old rotations expressions to symbol
     * level DataDefined angle
     */
    static void convertSymbolRotation( QgsSymbol *symbol, const QString &field );

    QgsFeatureRequest::OrderBy mOrderBy;

    bool mOrderByEnabled;

  private:
#ifdef SIP_RUN
    QgsFeatureRenderer( const QgsFeatureRenderer & );
    QgsFeatureRenderer &operator=( const QgsFeatureRenderer & );
#endif

#ifdef QGISDEBUG
    //! Pointer to thread in which startRender was first called
    QThread *mThread = nullptr;
#endif

    Q_DISABLE_COPY( QgsFeatureRenderer )
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFeatureRenderer::Capabilities )

// for some reason SIP compilation fails if these lines are not included:
class QgsRendererWidget;
class QgsPaintEffectWidget;

#endif // QGSRENDERER_H
