/***************************************************************************
 qgssymbol.h
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

#ifndef QGSSYMBOL_H
#define QGSSYMBOL_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgspropertycollection.h"
#include "qgsrendercontext.h"

class QgsSymbolLayer;
class QgsLegendPatchShape;
class QgsSymbolRenderContext;
class QgsLineSymbolLayer;

typedef QList<QgsSymbolLayer *> QgsSymbolLayerList;

/**
 * \ingroup core
 * \class QgsSymbolAnimationSettings
 *
 * \brief Contains settings relating to symbol animation.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsSymbolAnimationSettings
{
  public:

    /**
     * Sets whether the symbol is animated.
     *
     * This is a user-facing setting for symbols, which allows users to define whether a
     * symbol is animated, and allows for creation of animated symbols via data
     * defined properties.
     *
     * \see isAnimated()
     */
    void setIsAnimated( bool animated ) { mIsAnimated = animated; }

    /**
     * Returns TRUE if the symbol is animated.
     *
     * This is a user-facing setting for symbols, which allows users to define whether a
     * symbol is animated, and allows for creation of animated symbols via data
     * defined properties.
     *
     * \see setIsAnimated()
     */
    bool isAnimated() const { return mIsAnimated; }

    /**
     * Sets the symbol animation frame \a rate (in frames per second).
     *
     * \see frameRate()
     */
    void setFrameRate( double rate ) { mFrameRate = rate; }

    /**
     * Returns the symbol animation frame rate (in frames per second).
     *
     * \see setFrameRate()
     */
    double frameRate() const { return mFrameRate; }

  private:

    bool mIsAnimated = false;
    double mFrameRate = 10;

};

/**
 * \ingroup core
 * \class QgsSymbol
 *
 * \brief Abstract base class for all rendered symbols.
 */
class CORE_EXPORT QgsSymbol
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->type() )
    {
      case Qgis::SymbolType::Marker: sipType = sipType_QgsMarkerSymbol; break;
      case Qgis::SymbolType::Line: sipType = sipType_QgsLineSymbol; break;
      case Qgis::SymbolType::Fill: sipType = sipType_QgsFillSymbol; break;
      default: sipType = 0; break;
    }
    SIP_END
#endif

    friend class QgsFeatureRenderer;

  public:

    /**
     * Returns a translated string version of the specified symbol \a type.
     *
     * \since QGIS 3.20
     */
    static QString symbolTypeToString( Qgis::SymbolType type );

    /**
     * Returns the default symbol type required for the specified geometry \a type.
     *
     * \since QGIS 3.20
     */
    static Qgis::SymbolType symbolTypeForGeometryType( QgsWkbTypes::GeometryType type );

    /**
     * Data definable properties.
     * \since QGIS 3.18
     */
    enum Property
    {
      PropertyOpacity, //!< Opacity
    };

    /**
     * Returns the symbol property definitions.
     * \since QGIS 3.18
     */
    static const QgsPropertiesDefinition &propertyDefinitions();

    virtual ~QgsSymbol();

    /**
     * Returns a new default symbol for the specified geometry type.
     *
     * The caller takes ownership of the returned object.
     */
    static QgsSymbol *defaultSymbol( QgsWkbTypes::GeometryType geomType ) SIP_FACTORY;

    /**
     * Returns the symbol's type.
     */
    Qgis::SymbolType type() const { return mType; }

    // symbol layers handling

    /**
     * Returns the list of symbol layers contained in the symbol.
     * \returns symbol layers list
     * \see symbolLayer
     * \see symbolLayerCount
     * \since QGIS 2.7
     */
    QgsSymbolLayerList symbolLayers() { return mLayers; }

#ifndef SIP_RUN

    /**
     * Returns the symbol layer at the specified index
     * \see symbolLayers
     * \see symbolLayerCount
     * \since QGIS 2.7
     */
    QgsSymbolLayer *symbolLayer( int layer );

    /**
     * Returns the symbol layer at the specified index, const variant
     * \see symbolLayers
     * \see symbolLayerCount
     * \since QGIS 3.12
     */
    const QgsSymbolLayer *symbolLayer( int layer ) const;
#else

    /**
     * Returns the symbol layer at the specified index.
     *
     * \throws IndexError if no layer with the specified index exists.
     *
     * \see symbolLayers
     * \see symbolLayerCount
     * \since QGIS 2.7
     */
    SIP_PYOBJECT symbolLayer( int layer ) SIP_TYPEHINT( QgsSymbolLayer );
    % MethodCode
    const int count = sipCpp->symbolLayerCount();
    if ( a0 < 0 || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = sipConvertFromType( sipCpp->symbolLayer( a0 ), sipType_QgsSymbolLayer, NULL );
    }
    % End
#endif

    /**
     * Returns the total number of symbol layers contained in the symbol.
     * \returns count of symbol layers
     * \see symbolLayers
     * \see symbolLayer
     * \since QGIS 2.7
     */
    int symbolLayerCount() const { return mLayers.count(); }

#ifdef SIP_RUN

    /**
     * Returns the number of symbol layers contained in the symbol.
     */
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->symbolLayerCount();
    % End

    //! Ensures that bool(obj) returns TRUE (otherwise __len__() would be used)
    int __bool__() const;
    % MethodCode
    sipRes = true;
    % End

    /**
    * Returns the symbol layer at the specified ``index``.
    *
    * Indexes can be less than 0, in which case they correspond to layers from the end of the symbol. E.g. an index of -1
    * corresponds to the last layer in the symbol.
    *
    * \throws IndexError if no layer with the specified ``index`` exists.
    *
    * \since QGIS 3.10
    */
    SIP_PYOBJECT __getitem__( int index ) SIP_TYPEHINT( QgsSymbolLayer );
    % MethodCode
    const int count = sipCpp->symbolLayerCount();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else if ( a0 >= 0 )
    {
      return sipConvertFromType( sipCpp->symbolLayer( a0 ), sipType_QgsSymbolLayer, NULL );
    }
    else
    {
      return sipConvertFromType( sipCpp->symbolLayer( count + a0 ), sipType_QgsSymbolLayer, NULL );
    }
    % End

    /**
     * Deletes the layer at the specified ``index``.
     *
     * Indexes can be less than 0, in which case they correspond to layers from the end of the symbol. E.g. an index of -1
     * corresponds to the last layer in the symbol.
     *
     * \throws IndexError if no layer at the specified ``index`` exists
     *
     * \since QGIS 3.10
     */
    void __delitem__( int index );
    % MethodCode
    const int count = sipCpp->symbolLayerCount();
    if ( a0 >= 0 && a0 < count )
      sipCpp->deleteSymbolLayer( a0 );
    else if ( a0 < 0 && a0 >= -count )
      sipCpp->deleteSymbolLayer( count + a0 );
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End
#endif

    /**
     * Inserts a symbol \a layer to specified \a index.
     * Ownership of \a layer is transferred to the symbol.
     * \param index The index at which the layer should be added
     * \param layer The symbol layer to add
     * \returns TRUE if the layer is added, FALSE if the index or the layer is bad
     */
    bool insertSymbolLayer( int index, QgsSymbolLayer *layer SIP_TRANSFER );

    /**
     * Appends a symbol \a layer at the end of the current symbol layer list.
     * Ownership of \a layer is transferred to the symbol.
     * \returns TRUE if the layer was successfully added, FALSE if the layer is not compatible with the
     * symbol's type().
     */
    bool appendSymbolLayer( QgsSymbolLayer *layer SIP_TRANSFER );

    /**
     * Removes and deletes the symbol layer at the specified \a index.
     */
    bool deleteSymbolLayer( int index );

    /**
     * Removes a symbol layer from the list and returns a pointer to it.
     * Ownership of the layer is handed to the caller.
     * \param index The index of the layer to remove
     * \returns A pointer to the removed layer
     */
    QgsSymbolLayer *takeSymbolLayer( int index ) SIP_TRANSFERBACK;

    /**
     * Deletes the current layer at the specified \a index and replaces it with \a layer.
     * Ownership of \a layer is transferred to the symbol.
     *
     * Returns FALSE if \a layer is not compatible with the symbol's type(), or
     * TRUE if the layer was successfully replaced.
     */
    bool changeSymbolLayer( int index, QgsSymbolLayer *layer SIP_TRANSFER );

    /**
     * Begins the rendering process for the symbol. This must be called before renderFeature(),
     * and should be followed by a call to stopRender().
     * \param context render context which symbol will be drawn using
     * \param fields fields for features to be rendered (usually the associated
     * vector layer's fields). Required for correct calculation of data defined
     * overrides.
     * \see stopRender()
     */
    void startRender( QgsRenderContext &context, const QgsFields &fields = QgsFields() );

    /**
     * Ends the rendering process. This should be called after rendering all desired features.
     * \param context render context, must match the context specified when startRender()
     * was called.
     * \see startRender()
     */
    void stopRender( QgsRenderContext &context );

    /**
     * Sets the \a color for the symbol.
     *
     * Calling this method sets the color for each individual symbol layer contained
     * within the symbol to \a color.
     *
     * Locked symbol layers are skipped and are left unchanged.
     *
     * \see color()
     */
    void setColor( const QColor &color ) const;

    /**
     * Returns the symbol's color.
     *
     * For multi-layer symbols, this method returns the color of the first unlocked symbol
     * layer.
     *
     * \see setColor()
     */
    QColor color() const;

    /**
     * Draws an icon of the symbol that occupies an area given by \a size using the specified \a painter.
     *
     * Optionally a custom render context may be given in order to ensure that the preview icon exactly
     * matches the settings from that context.
     * \param painter destination painter
     * \param size size of the icon
     * \param customContext the context in which the rendering happens
     * \param selected set to TRUE to render the symbol in a selected state
     * \param expressionContext optional custom expression context
     * \param patchShape optional patch shape to use for symbol preview. If not specified a default shape will be used instead.
     *
     * \see exportImage()
     * \see asImage()
     * \note Parameter selected added in QGIS 3.10
     * \since QGIS 2.6
     */
    void drawPreviewIcon( QPainter *painter, QSize size, QgsRenderContext *customContext = nullptr, bool selected = false, const QgsExpressionContext *expressionContext = nullptr,
                          const QgsLegendPatchShape *patchShape = nullptr );

    /**
     * Export the symbol as an image format, to the specified \a path and with the given \a size.
     *
     * If \a format is "SVG" then an SVG file will be created, otherwise a raster image of the
     * specified format will be created.
     *
     * \see asImage()
     * \see drawPreviewIcon()
     */
    void exportImage( const QString &path, const QString &format, QSize size );

    /**
     * Returns an image of the symbol at the specified \a size.
     *
     * Optionally a custom render context may be given in order to ensure that the preview icon exactly
     * matches the settings from that context.
     *
     * \see exportImage()
     * \see drawPreviewIcon()
     */
    QImage asImage( QSize size, QgsRenderContext *customContext = nullptr );

    /**
     * Returns a large (roughly 100x100 pixel) preview image for the symbol.
     *
     * \param expressionContext optional expression context, for evaluation of
     * data defined symbol properties
     * \param flags optional flags to control how preview image is generated
     *
     * \see asImage()
     * \see drawPreviewIcon()
     */
    QImage bigSymbolPreviewImage( QgsExpressionContext *expressionContext = nullptr, Qgis::SymbolPreviewFlags flags = Qgis::SymbolPreviewFlag::FlagIncludeCrosshairsForMarkerSymbols ) SIP_PYNAME( bigSymbolPreviewImageV2 );

    /**
     * \deprecated use bigSymbolPreviewImageV2 instead.
     */
    Q_DECL_DEPRECATED QImage bigSymbolPreviewImage( QgsExpressionContext *expressionContext = nullptr, int flags = static_cast< int >( Qgis::SymbolPreviewFlag::FlagIncludeCrosshairsForMarkerSymbols ) ) SIP_DEPRECATED;

    /**
     * Returns a string dump of the symbol's properties.
     */
    QString dump() const;

    /**
     * Returns a deep copy of this symbol.
     *
     * Ownership is transferred to the caller.
     */
    virtual QgsSymbol *clone() const = 0 SIP_FACTORY;

    /**
     * Converts the symbol to a SLD representation.
     */
    void toSld( QDomDocument &doc, QDomElement &element, QVariantMap props ) const;

    /**
     * Returns the units to use for sizes and widths within the symbol. Individual
     * symbol layer definitions will interpret this in different ways, e.g., a marker symbol
     * may use it to specify the units for the marker size, while a line symbol
     * may use it to specify the units for the line width.
     * \returns output unit, or QgsUnitTypes::RenderUnknownUnit if the symbol contains mixed units
     * \see setOutputUnit()
     */
    QgsUnitTypes::RenderUnit outputUnit() const;

    /**
     * Returns TRUE if the symbol has any components which use map unit based sizes.
     *
     * \since QGIS 3.18
     */
    bool usesMapUnits() const;

    /**
     * Sets the units to use for sizes and widths within the symbol. Individual
     * symbol definitions will interpret this in different ways, e.g., a marker symbol
     * may use it to specify the units for the marker size, while a line symbol
     * may use it to specify the units for the line width.
     * \param unit output units
     * \see outputUnit()
     */
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) const;

    /**
     * Returns the map unit scale for the symbol.
     *
     * If the symbol consists of multiple layers, the map unit scale is only
     * returned if all layers have the same scale settings. If the settings differ,
     * a default constructed map unit scale is returned.
     *
     * \see setMapUnitScale()
     */
    QgsMapUnitScale mapUnitScale() const;

    /**
     * Sets the map unit \a scale for the symbol.
     *
     * Calling this method sets the scale for all symbol layers contained within the
     * symbol.
     *
     * \see mapUnitScale()
     */
    void setMapUnitScale( const QgsMapUnitScale &scale ) const;

    /**
     * Returns the opacity for the symbol.
     * \returns opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see setOpacity()
     */
    qreal opacity() const { return mOpacity; }

    /**
     * Sets the \a opacity for the symbol.
     * \param opacity opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see opacity()
     */
    void setOpacity( qreal opacity ) { mOpacity = opacity; }

    /**
     * Sets rendering hint flags for the symbol.
     * \see renderHints()
     */
    void setRenderHints( Qgis::SymbolRenderHints hints ) { mRenderHints = hints; }

    /**
     * Returns the rendering hint flags for the symbol.
     * \see setRenderHints()
     */
    Qgis::SymbolRenderHints renderHints() const { return mRenderHints; }

    /**
     * Sets \a flags for the symbol.
     *
     * \see flags()
     * \since QGIS 3.320
     */
    void setFlags( Qgis::SymbolFlags flags ) { mSymbolFlags = flags; }

    /**
     * Returns flags for the symbol.
     *
     * \see setFlags()
     * \since QGIS 3.20
     */
    Qgis::SymbolFlags flags() const { return mSymbolFlags; }

    /**
     * Sets whether features drawn by the symbol should be clipped to the render context's
     * extent. If this option is enabled then features which are partially outside the extent
     * will be clipped. This speeds up rendering of the feature, but may have undesirable
     * side effects for certain symbol types.
     * \param clipFeaturesToExtent set to TRUE to enable clipping (defaults to TRUE)
     * \see clipFeaturesToExtent
     * \since QGIS 2.9
     */
    void setClipFeaturesToExtent( bool clipFeaturesToExtent ) { mClipFeaturesToExtent = clipFeaturesToExtent; }

    /**
     * Returns whether features drawn by the symbol will be clipped to the render context's
     * extent. If this option is enabled then features which are partially outside the extent
     * will be clipped. This speeds up rendering of the feature, but may have undesirable
     * side effects for certain symbol types.
     * \returns TRUE if features will be clipped
     * \see setClipFeaturesToExtent
     * \since QGIS 2.9
     */
    bool clipFeaturesToExtent() const { return mClipFeaturesToExtent; }

    /**
     * Sets whether polygon features drawn by the symbol should be reoriented to follow the
     * standard right-hand-rule orientation, in which the area that is
     * bounded by the polygon is to the right of the boundary. In particular, the exterior
     * ring is oriented in a clockwise direction and the interior rings in a counter-clockwise
     * direction.
     * \see forceRHR()
     * \since QGIS 3.6
     */
    void setForceRHR( bool force ) { mForceRHR = force; }

    /**
     * Returns TRUE if polygon features drawn by the symbol will be reoriented to follow the
     * standard right-hand-rule orientation, in which the area that is
     * bounded by the polygon is to the right of the boundary. In particular, the exterior
     * ring is oriented in a clockwise direction and the interior rings in a counter-clockwise
     * direction.
     * \see setForceRHR()
     * \since QGIS 3.6
     */
    bool forceRHR() const { return mForceRHR; }

    /**
     * Returns a reference to the symbol animation settings.
     *
     * \see setAnimationSettings()
     * \since QGIS 3.26
     */
    QgsSymbolAnimationSettings &animationSettings();

    /**
     * Returns a reference to the symbol animation settings.
     *
     * \see setAnimationSettings()
     * \since QGIS 3.26
     */
    const QgsSymbolAnimationSettings &animationSettings() const SIP_SKIP;

    /**
     * Sets a the symbol animation \a settings.
     *
     * \see animationSettings()
     * \since QGIS 3.26
     */
    void setAnimationSettings( const QgsSymbolAnimationSettings &settings );

    /**
     * Returns a list of attributes required to render this feature.
     * This should include any attributes required by the symbology including
     * the ones required by expressions.
     */
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    /**
     * Sets a data defined property for the symbol. Any existing property with the same key
     * will be overwritten.
     * \see dataDefinedProperties()
     * \see Property
     * \since QGIS 3.18
     */
    void setDataDefinedProperty( Property key, const QgsProperty &property );

    /**
     * Returns a reference to the symbol's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \see Property
     * \since QGIS 3.18
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the symbol's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \since QGIS 3.18
     */
    const QgsPropertyCollection &dataDefinedProperties() const { return mDataDefinedProperties; } SIP_SKIP

    /**
     * Sets the symbol's property collection, used for data defined overrides.
     * \param collection property collection. Existing properties will be replaced.
     * \see dataDefinedProperties()
     * \since QGIS 3.18
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

    /**
     * Returns whether the symbol utilizes any data defined properties.
     * \since QGIS 2.12
     */
    bool hasDataDefinedProperties() const;

    /**
     * Returns TRUE if the symbol rendering can cause visible artifacts across a single feature
     * when the feature is rendered as a series of adjacent map tiles each containing a portion of the feature's geometry.
     *
     * Internally this calls QgsSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() for all symbol layers in the symbol
     * and returns TRUE if any of the layers returned TRUE.
     *
     * \since QGIS 3.18
     */
    bool canCauseArtifactsBetweenAdjacentTiles() const;

    /**
     * \note the layer will be NULLPTR after stopRender
     * \deprecated Will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void setLayer( const QgsVectorLayer *layer ) SIP_DEPRECATED;

    /**
     * \deprecated Will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED const QgsVectorLayer *layer() const SIP_DEPRECATED;

    /**
     * Render a feature. Before calling this the startRender() method should be called to initialize
     * the rendering process. After rendering all features stopRender() must be called.
     */
    void renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false, Qgis::VertexMarkerType currentVertexMarkerType = Qgis::VertexMarkerType::SemiTransparentCircle, double currentVertexMarkerSize = 0.0 ) SIP_THROW( QgsCsException );

    /**
     * Returns the symbol render context. Only valid between startRender and stopRender calls.
     *
     * \returns The symbol render context
     */
    QgsSymbolRenderContext *symbolRenderContext();

    /**
     * Called before symbol layers will be rendered for a particular \a feature.
     *
     * This is always followed by a call to stopFeatureRender() after the feature
     * has been completely rendered (i.e. all parts have been rendered).
     *
     * Internally, this notifies all symbol layers which will be used via a call to
     * QgsSymbolLayer::startFeatureRender().
     *
     * \since QGIS 3.20
     */
    void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context, int layer = -1 );

    /**
     * Called after symbol layers have been rendered for a particular \a feature.
     *
     * This is always preceded by a call to startFeatureRender() just before the feature
     * will be rendered.
     *
     * Internally, this notifies all symbol layers which were used via a call to
     * QgsSymbolLayer::stopFeatureRender().
     *
     * \since QGIS 3.20
     */
    void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context, int layer = -1 );

  protected:

    /**
     * Constructor for a QgsSymbol of the specified \a type.
     *
     * Ownership of \a layers will be transferred to the symbol.
     */
    QgsSymbol( Qgis::SymbolType type, const QgsSymbolLayerList &layers SIP_TRANSFER ); // can't be instantiated

    /**
     * Creates a point in screen coordinates from a QgsPoint in map coordinates
     */
    static inline QPointF _getPoint( QgsRenderContext &context, const QgsPoint &point )
    {
      QPointF pt;
      if ( context.coordinateTransform().isValid() )
      {
        double x = point.x();
        double y = point.y();
        double z = 0.0;
        context.coordinateTransform().transformInPlace( x, y, z );
        pt = QPointF( x, y );

      }
      else
        pt = point.toQPointF();

      context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );
      return pt;
    }

    /**
     * Creates a line string in screen coordinates from a QgsCurve in map coordinates
     */
    static QPolygonF _getLineString( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent = true );

    /**
     * Creates a polygon ring in screen coordinates from a QgsCurve in map coordinates.
     *
     * If \a correctRingOrientation is TRUE then the ring will be oriented to match standard ring orientation, e.g.
     * clockwise for exterior rings and counter-clockwise for interior rings.
     */
    static QPolygonF _getPolygonRing( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent, bool isExteriorRing = false, bool correctRingOrientation = false );

    /**
     * Creates a polygon in screen coordinates from a QgsPolygonXYin map coordinates
     *
     * If \a correctRingOrientation is TRUE then the ring will be oriented to match standard ring orientation, e.g.
     * clockwise for exterior rings and counter-clockwise for interior rings.
     *
     */
    static void _getPolygon( QPolygonF &pts, QVector<QPolygonF> &holes, QgsRenderContext &context, const QgsPolygon &polygon, bool clipToExtent = true, bool correctRingOrientation = false );

    /**
     * Retrieve a cloned list of all layers that make up this symbol.
     * Ownership is transferred to the caller.
     */
    QgsSymbolLayerList cloneLayers() const SIP_FACTORY;

    /**
     * Renders a context using a particular symbol layer without passing in a
     * geometry. This is used as fallback, if the symbol being rendered is not
     * compatible with the specified layer. In such a case, this method can be
     * called and will call the layer's rendering method anyway but the
     * geometry passed to the layer will be empty.
     * This is required for layers that generate their own geometry from other
     * information in the rendering context.
     *
     * Since QGIS 3.22, the optional \a geometryType, \a points and \a rings arguments can specify the original
     * geometry type, points and rings in which are being rendered by the parent symbol.
     */
    void renderUsingLayer( QgsSymbolLayer *layer, QgsSymbolRenderContext &context, QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::GeometryType::UnknownGeometry, const QPolygonF *points = nullptr, const QVector<QPolygonF> *rings = nullptr );

    /**
     * Render editing vertex marker at specified point
     * \since QGIS 2.16
     */
    void renderVertexMarker( QPointF pt, QgsRenderContext &context, Qgis::VertexMarkerType currentVertexMarkerType, double currentVertexMarkerSize );

    Qgis::SymbolType mType;
    QgsSymbolLayerList mLayers;

    //! Symbol opacity (in the range 0 - 1)
    qreal mOpacity = 1.0;

    Qgis::SymbolRenderHints mRenderHints;

    /**
     * Symbol flags.
     *
     * \since QGIS 3.20
     */
    Qgis::SymbolFlags mSymbolFlags = Qgis::SymbolFlags();

    bool mClipFeaturesToExtent = true;
    bool mForceRHR = false;

    QgsSymbolAnimationSettings mAnimationSettings;

    Q_DECL_DEPRECATED const QgsVectorLayer *mLayer = nullptr; //current vectorlayer

  private:
#ifdef SIP_RUN
    QgsSymbol( const QgsSymbol & );
#endif

    static void initPropertyDefinitions();

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

    /**
     * TRUE if render has already been started - guards against multiple calls to
     * startRender() (usually a result of not cloning a shared symbol instance before rendering).
     */
    bool mStarted = false;

    //! Initialized in startRender, destroyed in stopRender
    std::unique_ptr< QgsSymbolRenderContext > mSymbolRenderContext;

    QgsPropertyCollection mDataDefinedProperties;

    Q_DISABLE_COPY( QgsSymbol )

};

#endif

