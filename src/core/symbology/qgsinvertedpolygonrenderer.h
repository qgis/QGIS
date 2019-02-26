/***************************************************************************
    qgsinvertedpolygonrenderer.h
    ---------------------
    begin                : April 2014
    copyright            : (C) 2014 Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINVERTEDPOLYGONRENDERER_H
#define QGSINVERTEDPOLYGONRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsrenderer.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"

/**
 * \ingroup core
 * QgsInvertedPolygonRenderer is a polygon-only feature renderer used to
 * display features inverted, where the exterior is turned to an interior
 * and where the exterior theoretically spans the entire plane, allowing
 * to mask the surroundings of some features.
 *
 * It is designed on top of another feature renderer, which is called "embedded"
 * Most of the methods are then only proxies to the embedded renderer.
 *
 * Features are collected to form one "inverted" polygon
 * during renderFeature() and rendered on stopRender().
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsInvertedPolygonRenderer : public QgsFeatureRenderer
{
  public:

    /**
     * Constructor
     * \param embeddedRenderer optional embeddedRenderer. If NULLPTR, a default one will be assigned.
     * Ownership will be transferred.
     */
    QgsInvertedPolygonRenderer( QgsFeatureRenderer *embeddedRenderer SIP_TRANSFER = nullptr );

    //! Direct copies are forbidden. Use clone() instead.
    QgsInvertedPolygonRenderer( const QgsInvertedPolygonRenderer & ) = delete;
    //! Direct copies are forbidden. Use clone() instead.
    QgsInvertedPolygonRenderer &operator=( const QgsInvertedPolygonRenderer & ) = delete;

    QgsInvertedPolygonRenderer *clone() const override SIP_FACTORY;
    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;

    /**
     * Renders a given feature.
     * This will here collect features. The actual rendering will be postponed to stopRender()
     * \param feature the feature to render
     * \param context the rendering context
     * \param layer the symbol layer to render, if that makes sense
     * \param selected whether this feature has been selected (this will add decorations)
     * \param drawVertexMarker whether this feature has vertex markers (in edit mode usually)
     * \returns TRUE if the rendering was OK
     */
    bool renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) override;

    /**
     * The actual rendering will take place here.
     * Features collected during renderFeature() are rendered using the embedded feature renderer
     */
    void stopRender( QgsRenderContext &context ) override;

    QString dump() const override;

    //! Proxy that will call this method on the embedded renderer.
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool filterNeedsGeometry() const override;
    //! Proxy that will call this method on the embedded renderer.
    QgsFeatureRenderer::Capabilities capabilities() override;

    /**
     * Proxy that will call this method on the embedded renderer.
     */
    QgsSymbolList symbols( QgsRenderContext &context ) const override;

    /**
     * Proxy that will call this method on the embedded renderer.
     */
    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    /**
     * Proxy that will call this method on the embedded renderer.
     */
    QgsSymbol *originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    /**
     * Proxy that will call this method on the embedded renderer.
     */
    QgsSymbolList symbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    /**
     * Proxy that will call this method on the embedded renderer.
     */
    QgsSymbolList originalSymbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    /**
     * Proxy that will call this method on the embedded renderer.
     */
    QgsLegendSymbolList legendSymbolItems() const override;

    /**
     * Proxy that will call this method on the embedded renderer.
     */
    bool willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    //! Creates a renderer out of an XML, for loading
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;

    void setEmbeddedRenderer( QgsFeatureRenderer *subRenderer SIP_TRANSFER ) override;
    const QgsFeatureRenderer *embeddedRenderer() const override;

    void setLegendSymbolItem( const QString &key, QgsSymbol *symbol ) override;

    bool legendSymbolItemsCheckable() const override;
    bool legendSymbolItemChecked( const QString &key ) override;
    void checkLegendSymbolItem( const QString &key, bool state = true ) override;

    //! \returns TRUE if the geometries are to be preprocessed (merged with an union) before rendering.
    bool preprocessingEnabled() const { return mPreprocessingEnabled; }

    /**
     * \param enabled enables or disables the preprocessing.
     * When enabled, geometries will be merged with an union before being rendered.
     * It allows fixing some rendering artifacts (when rendering overlapping polygons for instance).
     * This will involve some CPU-demanding computations and is thus disabled by default.
     */
    void setPreprocessingEnabled( bool enabled ) { mPreprocessingEnabled = enabled; }

    /**
     * Creates a QgsInvertedPolygonRenderer by a conversion from an existing renderer.
     * \returns a new renderer if the conversion was possible, otherwise 0.
     * \since QGIS 2.5
     */
    static QgsInvertedPolygonRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

  private:

    //! Embedded renderer
    std::unique_ptr<QgsFeatureRenderer> mSubRenderer;

    //! Structure where the reversed geometry is built during renderFeature
    struct CombinedFeature
    {
      QVector<QgsGeometry> geometries; //< list of geometries
      QgsFeature feature;             //< one feature (for attriute-based rendering)
    };
    typedef QVector<CombinedFeature> FeatureCategoryVector;
    //! Where features are stored, based on the index of their symbol category \see mSymbolCategories
    FeatureCategoryVector mFeaturesCategories;

    //! Maps a category to an index
    QMap<QByteArray, int> mSymbolCategories;

    //! The polygon used as exterior ring that covers the current extent
    QgsPolygonXY mExtentPolygon;

    //! The context used for rendering
    QgsRenderContext mContext;

    //! Fields of each feature
    QgsFields mFields;

    /**
     * Class used to represent features that must be rendered
     *  with decorations (selection, vertex markers)
     */
    struct FeatureDecoration
    {
      QgsFeature feature;
      bool selected;
      bool drawMarkers;
      int layer;
      FeatureDecoration( const QgsFeature &a_feature, bool a_selected, bool a_drawMarkers, int a_layer )
        : feature( a_feature )
        , selected( a_selected )
        , drawMarkers( a_drawMarkers )
        , layer( a_layer )
      {}
    };
    QList<FeatureDecoration> mFeatureDecorations;

    //! Whether to preprocess (merge) geometries before rendering
    bool mPreprocessingEnabled = false;
};


#endif // QGSINVERTEDPOLYGONRENDERER_H
