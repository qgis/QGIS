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
#include <QList>
#include <QMap>
#include "qgsmapunitscale.h"
#include "qgsfields.h"
#include "qgsrendercontext.h"
#include "qgsproperty.h"

class QColor;
class QImage;
class QPainter;
class QSize;
class QPointF;
class QPolygonF;
class QDomDocument;
class QDomElement;

class QgsFields;
class QgsSymbolLayer;
class QgsRenderContext;
class QgsVectorLayer;
class QgsPaintEffect;
class QgsMarkerSymbolLayer;
class QgsLineSymbolLayer;
class QgsFillSymbolLayer;
class QgsSymbolRenderContext;
class QgsFeature;
class QgsFeatureRenderer;
class QgsCurve;
class QgsPolygon;
class QgsExpressionContext;
class QgsPoint;

typedef QList<QgsSymbolLayer *> QgsSymbolLayerList;

/**
 * \ingroup core
 * \class QgsSymbol
 *
 * Abstract base class for all rendered symbols.
 */
class CORE_EXPORT QgsSymbol
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->type() )
    {
      case QgsSymbol::Marker: sipType = sipType_QgsMarkerSymbol; break;
      case QgsSymbol::Line: sipType = sipType_QgsLineSymbol; break;
      case QgsSymbol::Fill: sipType = sipType_QgsFillSymbol; break;
      default: sipType = 0; break;
    }
    SIP_END
#endif

    friend class QgsFeatureRenderer;

  public:

    /**
     * Type of the symbol
     */
    enum SymbolType
    {
      Marker, //!< Marker symbol
      Line,   //!< Line symbol
      Fill,   //!< Fill symbol
      Hybrid  //!< Hybrid symbol
    };

    /**
     * Scale method
     */
    enum ScaleMethod
    {
      ScaleArea,     //!< Calculate scale by the area
      ScaleDiameter  //!< Calculate scale by the diameter
    };


    //! Flags controlling behavior of symbols during rendering
    enum RenderHint
    {
      DynamicRotation = 2, //!< Rotation of symbol may be changed during rendering and symbol should not be cached
    };
    Q_DECLARE_FLAGS( RenderHints, RenderHint )

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
    SymbolType type() const { return mType; }

    // symbol layers handling

    /**
     * Returns the list of symbol layers contained in the symbol.
     * \returns symbol layers list
     * \see symbolLayer
     * \see symbolLayerCount
     * \since QGIS 2.7
     */
    QgsSymbolLayerList symbolLayers() { return mLayers; }

    /**
     * Returns a specific symbol layer contained in the symbol.
     * \param layer layer number
     * \returns corresponding symbol layer
     * \see symbolLayers
     * \see symbolLayerCount
     * \since QGIS 2.7
     */
    QgsSymbolLayer *symbolLayer( int layer );

    /**
     * Returns the total number of symbol layers contained in the symbol.
     * \returns count of symbol layers
     * \see symbolLayers
     * \see symbolLayer
     * \since QGIS 2.7
     */
    int symbolLayerCount() const { return mLayers.count(); }

    /**
     * Inserts a symbol \a layer to specified \a index.
     * Ownership of \a layer is transferred to the symbol.
     * \param index The index at which the layer should be added
     * \param layer The symbol layer to add
     * \returns True if the layer is added, False if the index or the layer is bad
     */
    bool insertSymbolLayer( int index, QgsSymbolLayer *layer SIP_TRANSFER );

    /**
     * Appends a symbol \a layer at the end of the current symbol layer list.
     * Ownership of \a layer is transferred to the symbol.
     * \returns true if the layer was successfully added, false if the layer is not compatible with the
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
     * Returns false if \a layer is not compatible with the symbol's type(), or
     * true if the layer was successfully replaced.
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
    void setColor( const QColor &color );

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
     *
     * \see exportImage()
     * \see asImage()
     * \since QGIS 2.6
     */
    void drawPreviewIcon( QPainter *painter, QSize size, QgsRenderContext *customContext = nullptr );

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
     * \param expressionContext optional expression context, for evaluation of
     * data defined symbol properties
     *
     * \see asImage()
     * \see drawPreviewIcon()
     */
    QImage bigSymbolPreviewImage( QgsExpressionContext *expressionContext = nullptr );

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
    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

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
     * Sets the units to use for sizes and widths within the symbol. Individual
     * symbol definitions will interpret this in different ways, e.g., a marker symbol
     * may use it to specify the units for the marker size, while a line symbol
     * may use it to specify the units for the line width.
     * \param unit output units
     * \see outputUnit()
     */
    void setOutputUnit( QgsUnitTypes::RenderUnit unit );

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
    void setMapUnitScale( const QgsMapUnitScale &scale );

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
    void setRenderHints( RenderHints hints ) { mRenderHints = hints; }

    /**
     * Returns the rendering hint flags for the symbol.
     * \see setRenderHints()
     */
    RenderHints renderHints() const { return mRenderHints; }

    /**
     * Sets whether features drawn by the symbol should be clipped to the render context's
     * extent. If this option is enabled then features which are partially outside the extent
     * will be clipped. This speeds up rendering of the feature, but may have undesirable
     * side effects for certain symbol types.
     * \param clipFeaturesToExtent set to true to enable clipping (defaults to true)
     * \see clipFeaturesToExtent
     * \since QGIS 2.9
     */
    void setClipFeaturesToExtent( bool clipFeaturesToExtent ) { mClipFeaturesToExtent = clipFeaturesToExtent; }

    /**
     * Returns whether features drawn by the symbol will be clipped to the render context's
     * extent. If this option is enabled then features which are partially outside the extent
     * will be clipped. This speeds up rendering of the feature, but may have undesirable
     * side effects for certain symbol types.
     * \returns true if features will be clipped
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
     * Returns true if polygon features drawn by the symbol will be reoriented to follow the
     * standard right-hand-rule orientation, in which the area that is
     * bounded by the polygon is to the right of the boundary. In particular, the exterior
     * ring is oriented in a clockwise direction and the interior rings in a counter-clockwise
     * direction.
     * \see setForceRHR()
     * \since QGIS 3.6
     */
    bool forceRHR() const { return mForceRHR; }

    /**
     * Returns a list of attributes required to render this feature.
     * This should include any attributes required by the symbology including
     * the ones required by expressions.
     */
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    /**
     * Returns whether the symbol utilizes any data defined properties.
     * \since QGIS 2.12
     */
    bool hasDataDefinedProperties() const;

    /**
     * \note the layer will be NULL after stopRender
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
    void renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false, int currentVertexMarkerType = 0, double currentVertexMarkerSize = 0.0 );

    /**
     * Returns the symbol render context. Only valid between startRender and stopRender calls.
     *
     * \returns The symbol render context
     */
    QgsSymbolRenderContext *symbolRenderContext();

  protected:
    QgsSymbol( SymbolType type, const QgsSymbolLayerList &layers SIP_TRANSFER ); // can't be instantiated

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
     * If \a correctRingOrientation is true then the ring will be oriented to match standard ring orientation, e.g.
     * clockwise for exterior rings and counter-clockwise for interior rings.
     */
    static QPolygonF _getPolygonRing( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent, bool isExteriorRing = false, bool correctRingOrientation = false );

    /**
     * Creates a polygon in screen coordinates from a QgsPolygonXYin map coordinates
     *
     * If \a correctRingOrientation is true then the ring will be oriented to match standard ring orientation, e.g.
     * clockwise for exterior rings and counter-clockwise for interior rings.
     *
     */
    static void _getPolygon( QPolygonF &pts, QList<QPolygonF> &holes, QgsRenderContext &context, const QgsPolygon &polygon, bool clipToExtent = true, bool correctRingOrientation = false );

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
     */
    void renderUsingLayer( QgsSymbolLayer *layer, QgsSymbolRenderContext &context );

    /**
     * Render editing vertex marker at specified point
     * \since QGIS 2.16
     */
    void renderVertexMarker( QPointF pt, QgsRenderContext &context, int currentVertexMarkerType, double currentVertexMarkerSize );

    SymbolType mType;
    QgsSymbolLayerList mLayers;

    //! Symbol opacity (in the range 0 - 1)
    qreal mOpacity = 1.0;

    RenderHints mRenderHints = nullptr;
    bool mClipFeaturesToExtent = true;
    bool mForceRHR = false;

    Q_DECL_DEPRECATED const QgsVectorLayer *mLayer = nullptr; //current vectorlayer

  private:
#ifdef SIP_RUN
    QgsSymbol( const QgsSymbol & );
#endif

    /**
     * True if render has already been started - guards against multiple calls to
     * startRender() (usually a result of not cloning a shared symbol instance before rendering).
     */
    bool mStarted = false;

    //! Initialized in startRender, destroyed in stopRender
    std::unique_ptr< QgsSymbolRenderContext > mSymbolRenderContext;

    Q_DISABLE_COPY( QgsSymbol )

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsSymbol::RenderHints )

///////////////////////

/**
 * \ingroup core
 * \class QgsSymbolRenderContext
 */
class CORE_EXPORT QgsSymbolRenderContext
{
  public:

    /**
     * Constructor for QgsSymbolRenderContext
     * \param c
     * \param u
     * \param opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \param selected set to true if symbol should be drawn in a "selected" state
     * \param renderHints flags controlling rendering behavior
     * \param f
     * \param fields
     * \param mapUnitScale
     */
    QgsSymbolRenderContext( QgsRenderContext &c, QgsUnitTypes::RenderUnit u, qreal opacity = 1.0, bool selected = false, QgsSymbol::RenderHints renderHints = nullptr, const QgsFeature *f = nullptr, const QgsFields &fields = QgsFields(), const QgsMapUnitScale &mapUnitScale = QgsMapUnitScale() );

    //! QgsSymbolRenderContext cannot be copied.
    QgsSymbolRenderContext( const QgsSymbolRenderContext &rh ) = delete;

    /**
     * Returns a reference to the context's render context.
     */
    QgsRenderContext &renderContext() { return mRenderContext; }

    /**
     * Returns a reference to the context's render context.
     * \note Not available in Python bindings.
     */
    const QgsRenderContext &renderContext() const { return mRenderContext; } SIP_SKIP

    /**
     * Sets the original value variable value for data defined symbology
     * \param value value for original value variable. This usually represents the symbol property value
     * before any data defined overrides have been applied.
     * \since QGIS 2.12
     */
    void setOriginalValueVariable( const QVariant &value );

    //! Returns the output unit for the context
    QgsUnitTypes::RenderUnit outputUnit() const { return mOutputUnit; }

    //! Sets the output unit for the context
    void setOutputUnit( QgsUnitTypes::RenderUnit u ) { mOutputUnit = u; }

    QgsMapUnitScale mapUnitScale() const { return mMapUnitScale; }
    void setMapUnitScale( const QgsMapUnitScale &scale ) { mMapUnitScale = scale; }

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

    bool selected() const { return mSelected; }
    void setSelected( bool selected ) { mSelected = selected; }

    /**
     * Returns the rendering hint flags for the symbol.
     * \see setRenderHints()
     */
    QgsSymbol::RenderHints renderHints() const { return mRenderHints; }

    /**
     * Sets rendering hint flags for the symbol.
     * \see renderHints()
     */
    void setRenderHints( QgsSymbol::RenderHints hints ) { mRenderHints = hints; }

    void setFeature( const QgsFeature *f ) { mFeature = f; }
    //! Current feature being rendered - may be null
    const QgsFeature *feature() const { return mFeature; }

    /**
     * Sets the geometry type for the original feature geometry being rendered.
     * \see originalGeometryType()
     * \since QGIS 3.0
     */
    void setOriginalGeometryType( QgsWkbTypes::GeometryType type ) { mOriginalGeometryType = type; }

    /**
     * Returns the geometry type for the original feature geometry being rendered. This can be
     * useful if symbol layers alter their appearance based on geometry type - eg offsetting a
     * simple line style will look different if the simple line is rendering a polygon feature
     * (a closed buffer) vs a line feature (an unclosed offset line).
     * \see originalGeometryType()
     * \since QGIS 3.0
     */
    QgsWkbTypes::GeometryType originalGeometryType() const { return mOriginalGeometryType; }

    /**
     * Fields of the layer. Currently only available in startRender() calls
     * to allow symbols with data-defined properties prepare the expressions
     * (other times fields() returns null)
     * \since QGIS 2.4
     */
    QgsFields fields() const { return mFields; }

    /**
     * Part count of current geometry
     * \since QGIS 2.16
     */
    int geometryPartCount() const { return mGeometryPartCount; }

    /**
     * Sets the part count of current geometry
     * \since QGIS 2.16
     */
    void setGeometryPartCount( int count ) { mGeometryPartCount = count; }

    /**
     * Part number of current geometry
     * \since QGIS 2.16
     */
    int geometryPartNum() const { return mGeometryPartNum; }

    /**
     * Sets the part number of current geometry
     * \since QGIS 2.16
     */
    void setGeometryPartNum( int num ) { mGeometryPartNum = num; }

    double outputLineWidth( double width ) const;
    double outputPixelSize( double size ) const;

    // workaround for sip 4.7. Don't use assignment - will fail with assertion error
    QgsSymbolRenderContext &operator=( const QgsSymbolRenderContext & );

    /**
     * This scope is always available when a symbol of this type is being rendered.
     *
     * \returns An expression scope for details about this symbol
     */
    QgsExpressionContextScope *expressionContextScope();

    /**
     * Set an expression scope for this symbol.
     *
     * Will take ownership.
     *
     * \param contextScope An expression scope for details about this symbol
     */
    void setExpressionContextScope( QgsExpressionContextScope *contextScope SIP_TRANSFER );

  private:

#ifdef SIP_RUN
    QgsSymbolRenderContext( const QgsSymbolRenderContext &rh ) SIP_FORCE;
#endif

    QgsRenderContext &mRenderContext;
    std::unique_ptr< QgsExpressionContextScope > mExpressionContextScope;
    QgsUnitTypes::RenderUnit mOutputUnit;
    QgsMapUnitScale mMapUnitScale;
    qreal mOpacity = 1.0;
    bool mSelected;
    QgsSymbol::RenderHints mRenderHints;
    const QgsFeature *mFeature; //current feature
    QgsFields mFields;
    int mGeometryPartCount;
    int mGeometryPartNum;
    QgsWkbTypes::GeometryType mOriginalGeometryType = QgsWkbTypes::UnknownGeometry;
};



//////////////////////


/**
 * \ingroup core
 * \class QgsMarkerSymbol
 *
 * A marker symbol type, for rendering Point and MultiPoint geometries.
 */
class CORE_EXPORT QgsMarkerSymbol : public QgsSymbol
{
  public:

    /**
     * Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
     * This is a convenience method for easier creation of marker symbols.
     */
    static QgsMarkerSymbol *createSimple( const QgsStringMap &properties ) SIP_FACTORY;

    /**
     * Constructor for QgsMarkerSymbol, with the specified list of initial symbol \a layers.
     *
     * Ownership of the \a layers are transferred to the symbol.
     */
    QgsMarkerSymbol( const QgsSymbolLayerList &layers SIP_TRANSFER = QgsSymbolLayerList() );

    /**
     * Sets the angle for the whole symbol. Individual symbol layer sizes
     * will be rotated to maintain their current relative angle to the whole symbol angle.
     * \param symbolAngle new symbol angle
     * \see angle()
     */
    void setAngle( double symbolAngle );

    /**
     * Returns the marker angle for the whole symbol. Note that for symbols with
     * multiple symbol layers, this will correspond just to the angle of
     * the first symbol layer.
     * \see setAngle()
     * \since QGIS 2.16
     */
    double angle() const;

    /**
     * Set data defined angle for whole symbol (including all symbol layers).
     * \see dataDefinedAngle()
     * \since QGIS 3.0
     */
    void setDataDefinedAngle( const QgsProperty &property );

    /**
     * Returns data defined angle for whole symbol (including all symbol layers).
     * \returns data defined angle, or invalid property if angle is not set
     * at the marker level.
     * \see setDataDefinedAngle()
     * \since QGIS 3.0
     */
    QgsProperty dataDefinedAngle() const;

    /**
     * Sets the line angle modification for the symbol's angle. This angle is added to
     * the marker's rotation and data defined rotation before rendering the symbol, and
     * is usually used for orienting symbols to match a line's angle.
     * \param lineAngle Angle in degrees, valid values are between 0 and 360
     * \since QGIS 2.9
     */
    void setLineAngle( double lineAngle );

    /**
     * Sets the size for the whole symbol. Individual symbol layer sizes
     * will be scaled to maintain their current relative size to the whole symbol size.
     * \param size new symbol size
     * \see size()
     * \see setSizeUnit()
     * \see setSizeMapUnitScale()
     */
    void setSize( double size );

    /**
     * Returns the estimated size for the whole symbol, which is the maximum size of
     * all marker symbol layers in the symbol.
     *
     * \warning This returned value is inaccurate if the symbol consists of multiple
     * symbol layers with different size units. Use the overload accepting a QgsRenderContext
     * argument instead for accurate sizes in this case.
     *
     * \see setSize()
     * \see sizeUnit()
     * \see sizeMapUnitScale()
     */
    double size() const;

    /**
     * Returns the symbol size, in painter units. This is the maximum size of
     * all marker symbol layers in the symbol.
     *
     * This method returns an accurate size by calculating the actual rendered
     * size of each symbol layer using the provided render \a context.
     *
     * \see setSize()
     * \see sizeUnit()
     * \see sizeMapUnitScale()
     *
     * \since QGIS 3.4.5
     */
    double size( const QgsRenderContext &context ) const;

    /**
     * Sets the size units for the whole symbol (including all symbol layers).
     * \param unit size units
     * \see sizeUnit()
     * \see setSizeMapUnitScale()
     * \see setSize()
     * \since QGIS 2.16
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns the size units for the whole symbol (including all symbol layers).
     * \returns size units, or mixed units if symbol layers have different units
     * \see setSizeUnit()
     * \see sizeMapUnitScale()
     * \see size()
     * \since QGIS 2.16
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /**
     * Sets the size map unit scale for the whole symbol (including all symbol layers).
     * \param scale map unit scale
     * \see sizeMapUnitScale()
     * \see setSizeUnit()
     * \see setSize()
     * \since QGIS 2.16
     */
    void setSizeMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the size map unit scale for the whole symbol. Note that for symbols with
     * multiple symbol layers, this will correspond just to the map unit scale
     * for the first symbol layer.
     * \see setSizeMapUnitScale()
     * \see sizeUnit()
     * \see size()
     * \since QGIS 2.16
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /**
     * Set data defined size for whole symbol (including all symbol layers).
     * \see dataDefinedSize()
     * \since QGIS 3.0
     */
    void setDataDefinedSize( const QgsProperty &property );

    /**
     * Returns data defined size for whole symbol (including all symbol layers).
     * \returns data defined size, or invalid property if size is not set
     * at the marker level.
     * \see setDataDefinedSize
     * \since QGIS 3.0
     */
    QgsProperty dataDefinedSize() const;

    void setScaleMethod( QgsSymbol::ScaleMethod scaleMethod );
    ScaleMethod scaleMethod();

    void renderPoint( QPointF point, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    /**
     * Returns the approximate bounding box of the marker symbol, which includes the bounding box
     * of all symbol layers for the symbol. It is recommended to use this method only between startRender()
     * and stopRender() calls, or data defined rotation and offset will not be correctly calculated.
     * \param point location of rendered point in painter units
     * \param context render context
     * \param feature feature being rendered at point (optional). If not specified, the bounds calculation will not
     * include data defined parameters such as offset and rotation
     * \returns approximate symbol bounds, in painter units
     * \since QGIS 2.14
    */
    QRectF bounds( QPointF point, QgsRenderContext &context, const QgsFeature &feature = QgsFeature() ) const;

    QgsMarkerSymbol *clone() const override SIP_FACTORY;

  private:

    void renderPointUsingLayer( QgsMarkerSymbolLayer *layer, QPointF point, QgsSymbolRenderContext &context );

};


/**
 * \ingroup core
 * \class QgsLineSymbol
 *
 * A line symbol type, for rendering LineString and MultiLineString geometries.
 */
class CORE_EXPORT QgsLineSymbol : public QgsSymbol
{
  public:

    /**
     * Create a line symbol with one symbol layer: SimpleLine with specified properties.
     * This is a convenience method for easier creation of line symbols.
     */
    static QgsLineSymbol *createSimple( const QgsStringMap &properties ) SIP_FACTORY;

    /**
     * Constructor for QgsLineSymbol, with the specified list of initial symbol \a layers.
     *
     * Ownership of the \a layers are transferred to the symbol.
     */
    QgsLineSymbol( const QgsSymbolLayerList &layers SIP_TRANSFER = QgsSymbolLayerList() );

    /**
     * Sets the \a width for the whole line symbol. Individual symbol layer sizes
     * will be scaled to maintain their current relative size to the whole symbol size.
     *
     * \see width()
     */
    void setWidth( double width );

    /**
     * Returns the estimated width for the whole symbol, which is the maximum width of
     * all marker symbol layers in the symbol.
     *
     * \warning This returned value is inaccurate if the symbol consists of multiple
     * symbol layers with different width units. Use the overload accepting a QgsRenderContext
     * argument instead for accurate sizes in this case.
     *
     * \see setWidth()
     */
    double width() const;

    /**
     * Returns the symbol width, in painter units. This is the maximum width of
     * all marker symbol layers in the symbol.
     *
     * This method returns an accurate width by calculating the actual rendered
     * width of each symbol layer using the provided render \a context.
     *
     * \see setWidth()
     *
     * \since QGIS 3.4.5
     */
    double width( const QgsRenderContext &context ) const;

    /**
     * Set data defined width for whole symbol (including all symbol layers).
     * \see dataDefinedWidth()
     * \since QGIS 3.0
     */
    void setDataDefinedWidth( const QgsProperty &property );

    /**
     * Returns data defined width for whole symbol (including all symbol layers).
     * \returns data defined width, or invalid property if size is not set
     * at the line level. Caller takes responsibility for deleting the returned object.
     * \see setDataDefinedWidth
     * \since QGIS 3.0
     */
    QgsProperty dataDefinedWidth() const;

    void renderPolyline( const QPolygonF &points, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    QgsLineSymbol *clone() const override SIP_FACTORY;

  private:

    void renderPolylineUsingLayer( QgsLineSymbolLayer *layer, const QPolygonF &points, QgsSymbolRenderContext &context );

};


/**
 * \ingroup core
 * \class QgsFillSymbol
 *
 * A fill symbol type, for rendering Polygon and MultiPolygon geometries.
 */
class CORE_EXPORT QgsFillSymbol : public QgsSymbol
{
  public:

    /**
     * Create a fill symbol with one symbol layer: SimpleFill with specified properties.
     * This is a convenience method for easier creation of fill symbols.
     */
    static QgsFillSymbol *createSimple( const QgsStringMap &properties ) SIP_FACTORY;

    /**
     * Constructor for QgsFillSymbol, with the specified list of initial symbol \a layers.
     *
     * Ownership of the \a layers are transferred to the symbol.
     */
    QgsFillSymbol( const QgsSymbolLayerList &layers SIP_TRANSFER = QgsSymbolLayerList() );
    void setAngle( double angle );
    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    QgsFillSymbol *clone() const override SIP_FACTORY;

  private:

    void renderPolygonUsingLayer( QgsSymbolLayer *layer, const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );
    //! Calculates the bounds of a polygon including rings
    QRectF polygonBounds( const QPolygonF &points, const QList<QPolygonF> *rings ) const;
    //! Translates the rings in a polygon by a set distance
    QList<QPolygonF> *translateRings( const QList<QPolygonF> *rings, double dx, double dy ) const;
};

#endif

