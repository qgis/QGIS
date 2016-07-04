/***************************************************************************
 qgssymbolv2.h
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

#ifndef QGSSYMBOLV2_H
#define QGSSYMBOLV2_H

#include "qgis.h"
#include <QList>
#include <QMap>
#include "qgsmapunitscale.h"
#include "qgsgeometry.h"
#include "qgspointv2.h"

class QColor;
class QImage;
class QPainter;
class QSize;
class QPointF;
class QPolygonF;

class QDomDocument;
class QDomElement;
//class

class QgsFeature;
class QgsFields;
class QgsSymbolLayerV2;
class QgsRenderContext;
class QgsVectorLayer;
class QgsPaintEffect;
class QgsMarkerSymbolLayerV2;
class QgsLineSymbolLayerV2;
class QgsFillSymbolLayerV2;
class QgsDataDefined;
class QgsSymbolV2RenderContext;
class QgsFeatureRendererV2;

typedef QList<QgsSymbolLayerV2*> QgsSymbolLayerV2List;

/** \ingroup core
 * \class QgsSymbolV2
 */
class CORE_EXPORT QgsSymbolV2
{
    friend class QgsFeatureRendererV2;

  public:

    /**
     * The unit of the output
     */
    //TODO QGIS 3.0 - move to QgsUnitTypes and rename to SymbolUnit
    enum OutputUnit
    {
      MM = 0,     //!< The output shall be in millimeters
      MapUnit,    //!< The output shall be in map unitx
      Mixed,      //!< Mixed units in symbol layers
      Pixel,      //!< The output shall be in pixels
      Percentage,  //!< The ouput shall be a percentage of another measurement (eg canvas size, feature size)
    };

    typedef QList<OutputUnit> OutputUnitList;

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

    enum RenderHint
    {
      DataDefinedSizeScale = 1,
      DataDefinedRotation = 2
    };

    virtual ~QgsSymbolV2();

    //! return new default symbol for specified geometry type
    static QgsSymbolV2* defaultSymbol( QGis::GeometryType geomType );

    SymbolType type() const { return mType; }

    // symbol layers handling

    /** Returns list of symbol layers contained in the symbol.
     * @returns symbol layers list
     * @note added in QGIS 2.7
     * @see symbolLayer
     * @see symbolLayerCount
     */
    QgsSymbolLayerV2List symbolLayers() { return mLayers; }

    /** Returns a specific symbol layers contained in the symbol.
     * @param layer layer number
     * @returns corresponding symbol layer
     * @note added in QGIS 2.7
     * @see symbolLayers
     * @see symbolLayerCount
     */
    QgsSymbolLayerV2* symbolLayer( int layer );

    /** Returns total number of symbol layers contained in the symbol.
     * @returns count of symbol layers
     * @note added in QGIS 2.7
     * @see symbolLayers
     * @see symbolLayer
     */
    int symbolLayerCount() { return mLayers.count(); }

    /**
     * Insert symbol layer to specified index
     * Ownership will be transferred.
     * @param index The index at which the layer should be added
     * @param layer The symbol layer to add
     * @return True if the layer is added, False if the index or the layer is bad
     */
    bool insertSymbolLayer( int index, QgsSymbolLayerV2* layer );

    /**
     * Append symbol layer at the end of the list
     * Ownership will be transferred.
     * @param layer The layer to add
     * @return True if the layer is added, False if the layer is bad
     */
    bool appendSymbolLayer( QgsSymbolLayerV2* layer );

    //! delete symbol layer at specified index
    bool deleteSymbolLayer( int index );

    /**
     * Remove symbol layer from the list and return pointer to it.
     * Ownership is handed to the caller.
     * @param index The index of the layer to remove
     * @return A pointer to the removed layer
     */
    QgsSymbolLayerV2* takeSymbolLayer( int index );

    //! delete layer at specified index and set a new one
    bool changeSymbolLayer( int index, QgsSymbolLayerV2 *layer );

    void startRender( QgsRenderContext& context, const QgsFields* fields = nullptr );
    void stopRender( QgsRenderContext& context );

    void setColor( const QColor& color );
    QColor color() const;

    //! Draw icon of the symbol that occupyies area given by size using the painter.
    //! Optionally custom context may be given in order to get rendering of symbols that use map units right.
    //! @note customContext parameter added in 2.6
    void drawPreviewIcon( QPainter* painter, QSize size, QgsRenderContext* customContext = nullptr );

    //! export symbol as image format. PNG and SVG supported
    void exportImage( const QString& path, const QString& format, QSize size );

    //! Generate symbol as image
    QImage asImage( QSize size, QgsRenderContext* customContext = nullptr );

    /** Returns a large (roughly 100x100 pixel) preview image for the symbol.
     * @param expressionContext optional expression context, for evaluation of
     * data defined symbol properties
     */
    QImage bigSymbolPreviewImage( QgsExpressionContext* expressionContext = nullptr );

    QString dump() const;

    virtual QgsSymbolV2* clone() const = 0;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QgsSymbolV2::OutputUnit outputUnit() const;
    void setOutputUnit( QgsSymbolV2::OutputUnit u );

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale& scale );

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const { return mAlpha; }
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha ) { mAlpha = alpha; }

    void setRenderHints( int hints ) { mRenderHints = hints; }
    int renderHints() const { return mRenderHints; }

    /** Sets whether features drawn by the symbol should be clipped to the render context's
     * extent. If this option is enabled then features which are partially outside the extent
     * will be clipped. This speeds up rendering of the feature, but may have undesirable
     * side effects for certain symbol types.
     * @param clipFeaturesToExtent set to true to enable clipping (defaults to true)
     * @note added in QGIS 2.9
     * @see clipFeaturesToExtent
     */
    void setClipFeaturesToExtent( bool clipFeaturesToExtent ) { mClipFeaturesToExtent = clipFeaturesToExtent; }

    /** Returns whether features drawn by the symbol will be clipped to the render context's
     * extent. If this option is enabled then features which are partially outside the extent
     * will be clipped. This speeds up rendering of the feature, but may have undesirable
     * side effects for certain symbol types.
     * @returns true if features will be clipped
     * @note added in QGIS 2.9
     * @see setClipFeaturesToExtent
     */
    bool clipFeaturesToExtent() const { return mClipFeaturesToExtent; }

    /**
     * Return a list of attributes required to render this feature.
     * This should include any attributes required by the symbology including
     * the ones required by expressions.
     */
    QSet<QString> usedAttributes() const;

    /** Returns whether the symbol utilises any data defined properties.
     * @note added in QGIS 2.12
     */
    bool hasDataDefinedProperties() const;

    //! @note the layer will be NULL after stopRender
    void setLayer( const QgsVectorLayer* layer ) { mLayer = layer; }
    const QgsVectorLayer* layer() const { return mLayer; }

    /**
     * Render a feature.
     */
    void renderFeature( const QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false, int currentVertexMarkerType = 0, int currentVertexMarkerSize = 0 );

    /**
     * Returns the symbol render context. Only valid between startRender and stopRender calls.
     *
     * @return The symbol render context
     */
    QgsSymbolV2RenderContext* symbolRenderContext();

  protected:
    QgsSymbolV2( SymbolType type, const QgsSymbolLayerV2List& layers ); // can't be instantiated

    /**
     * Creates a point in screen coordinates from a QgsPointV2 in map coordinates
     */
    static inline void _getPoint( QPointF& pt, QgsRenderContext& context, const QgsPointV2* point )
    {
      if ( context.coordinateTransform() )
      {
        double x = point->x();
        double y = point->y();
        double z = 0.0;
        context.coordinateTransform()->transformInPlace( x, y, z );
        pt = QPointF( x, y );

      }
      else
        pt = point->toQPointF();

      context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );
    }

    /**
     * Creates a point in screen coordinates from a wkb string in map
     * coordinates
     */
    static QgsConstWkbPtr _getPoint( QPointF& pt, QgsRenderContext& context, QgsConstWkbPtr& wkb );

    /**
     * Creates a line string in screen coordinates from a wkb string in map
     * coordinates
     */
    static QgsConstWkbPtr _getLineString( QPolygonF& pts, QgsRenderContext& context, QgsConstWkbPtr& wkb, bool clipToExtent = true );

    /**
     * Creates a polygon in screen coordinates from a wkb string in map
     * coordinates
     */
    static QgsConstWkbPtr _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, QgsConstWkbPtr& wkb, bool clipToExtent = true );

    /**
     * Retrieve a cloned list of all layers that make up this symbol.
     * Ownership is transferred to the caller.
     */
    QgsSymbolLayerV2List cloneLayers() const;

    /**
     * Renders a context using a particular symbol layer without passing in a
     * geometry. This is used as fallback, if the symbol being rendered is not
     * compatible with the specified layer. In such a case, this method can be
     * called and will call the layer's rendering method anyway but the
     * geometry passed to the layer will be empty.
     * This is required for layers that generate their own geometry from other
     * information in the rendering context.
     */
    void renderUsingLayer( QgsSymbolLayerV2* layer, QgsSymbolV2RenderContext& context );

    //! check whether a symbol layer type can be used within the symbol
    //! (marker-marker, line-line, fill-fill/line)
    //! @deprecated since 2.14, use QgsSymbolLayerV2::isCompatibleWithSymbol instead
    Q_DECL_DEPRECATED bool isSymbolLayerCompatible( SymbolType layerType );

    //! Render editing vertex marker at specified point
    //! @note added in QGIS 2.16
    void renderVertexMarker( QPointF pt, QgsRenderContext& context, int currentVertexMarkerType, int currentVertexMarkerSize );

    SymbolType mType;
    QgsSymbolLayerV2List mLayers;

    /** Symbol opacity (in the range 0 - 1)*/
    qreal mAlpha;

    int mRenderHints;
    bool mClipFeaturesToExtent;

    const QgsVectorLayer* mLayer; //current vectorlayer

  private:
    //! Initialized in startRender, destroyed in stopRender
    QgsSymbolV2RenderContext* mSymbolRenderContext;

    Q_DISABLE_COPY( QgsSymbolV2 )

};

///////////////////////

/** \ingroup core
 * \class QgsSymbolV2RenderContext
 */
class CORE_EXPORT QgsSymbolV2RenderContext
{
  public:
    QgsSymbolV2RenderContext( QgsRenderContext& c, QgsSymbolV2::OutputUnit u, qreal alpha = 1.0, bool selected = false, int renderHints = 0, const QgsFeature* f = nullptr, const QgsFields* fields = nullptr, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() );
    ~QgsSymbolV2RenderContext();

    QgsRenderContext& renderContext() { return mRenderContext; }
    const QgsRenderContext& renderContext() const { return mRenderContext; }

    /** Sets the original value variable value for data defined symbology
     * @param value value for original value variable. This usually represents the symbol property value
     * before any data defined overrides have been applied.
     * @note added in QGIS 2.12
     */
    void setOriginalValueVariable( const QVariant& value );

    QgsSymbolV2::OutputUnit outputUnit() const { return mOutputUnit; }
    void setOutputUnit( QgsSymbolV2::OutputUnit u ) { mOutputUnit = u; }

    QgsMapUnitScale mapUnitScale() const { return mMapUnitScale; }
    void setMapUnitScale( const QgsMapUnitScale& scale ) { mMapUnitScale = scale; }

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const { return mAlpha; }
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha ) { mAlpha = alpha; }

    bool selected() const { return mSelected; }
    void setSelected( bool selected ) { mSelected = selected; }

    int renderHints() const { return mRenderHints; }
    void setRenderHints( int hints ) { mRenderHints = hints; }

    void setFeature( const QgsFeature* f ) { mFeature = f; }
    //! Current feature being rendered - may be null
    const QgsFeature* feature() const { return mFeature; }

    //! Fields of the layer. Currently only available in startRender() calls
    //! to allow symbols with data-defined properties prepare the expressions
    //! (other times fields() returns null)
    //! @note added in 2.4
    const QgsFields* fields() const { return mFields; }

    /** Part count of current geometry
     * @note added in QGIS 2.16
     */
    int geometryPartCount() const { return mGeometryPartCount; }
    /** Sets the part count of current geometry
     * @note added in QGIS 2.16
     */
    void setGeometryPartCount( int count ) { mGeometryPartCount = count; }

    /** Part number of current geometry
     * @note added in QGIS 2.16
     */
    int geometryPartNum() const { return mGeometryPartNum; }
    /** Sets the part number of current geometry
     * @note added in QGIS 2.16
     */
    void setGeometryPartNum( int num ) { mGeometryPartNum = num; }

    double outputLineWidth( double width ) const;
    double outputPixelSize( double size ) const;

    // workaround for sip 4.7. Don't use assignment - will fail with assertion error
    QgsSymbolV2RenderContext& operator=( const QgsSymbolV2RenderContext& );

    /**
     * This scope is always available when a symbol of this type is being rendered.
     *
     * @return An expression scope for details about this symbol
     */
    QgsExpressionContextScope* expressionContextScope();
    /**
     * Set an expression scope for this symbol.
     *
     * Will take ownership.
     *
     * @param contextScope An expression scope for details about this symbol
     */
    void setExpressionContextScope( QgsExpressionContextScope* contextScope );

  private:
    QgsRenderContext& mRenderContext;
    QgsExpressionContextScope* mExpressionContextScope;
    QgsSymbolV2::OutputUnit mOutputUnit;
    QgsMapUnitScale mMapUnitScale;
    qreal mAlpha;
    bool mSelected;
    int mRenderHints;
    const QgsFeature* mFeature; //current feature
    const QgsFields* mFields;
    int mGeometryPartCount;
    int mGeometryPartNum;


    QgsSymbolV2RenderContext( const QgsSymbolV2RenderContext& rh );
};



//////////////////////


/** \ingroup core
 * \class QgsMarkerSymbolV2
 */
class CORE_EXPORT QgsMarkerSymbolV2 : public QgsSymbolV2
{
  public:

    /** Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
     * This is a convenience method for easier creation of marker symbols.
     */
    static QgsMarkerSymbolV2* createSimple( const QgsStringMap& properties );

    QgsMarkerSymbolV2( const QgsSymbolLayerV2List& layers = QgsSymbolLayerV2List() );

    /** Sets the angle for the whole symbol. Individual symbol layer sizes
     * will be rotated to maintain their current relative angle to the whole symbol angle.
     * @param angle new symbol angle
     * @see angle()
     */
    void setAngle( double angle );

    /** Returns the marker angle for the whole symbol. Note that for symbols with
     * multiple symbol layers, this will correspond just to the angle of
     * the first symbol layer.
     * @note added in QGIS 2.16
     * @see setAngle()
     */
    double angle() const;

    /** Set data defined angle for whole symbol (including all symbol layers).
     * @param dd data defined angle
     * @note added in QGIS 2.9
     * @see dataDefinedAngle
     */
    void setDataDefinedAngle( const QgsDataDefined& dd );

    /** Returns data defined angle for whole symbol (including all symbol layers).
     * @returns data defined angle, or empty data defined if angle is not set
     * at the marker level
     * @note added in QGIS 2.9
     * @see setDataDefinedAngle
     */
    QgsDataDefined dataDefinedAngle() const;

    /** Sets the line angle modification for the symbol's angle. This angle is added to
     * the marker's rotation and data defined rotation before rendering the symbol, and
     * is usually used for orienting symbols to match a line's angle.
     * @param lineAngle Angle in degrees, valid values are between 0 and 360
     * @note added in QGIS 2.9
     */
    void setLineAngle( double lineAngle );

    /** Sets the size for the whole symbol. Individual symbol layer sizes
     * will be scaled to maintain their current relative size to the whole symbol size.
     * @param size new symbol size
     * @see size()
     * @see setSizeUnit()
     * @see setSizeMapUnitScale()
     */
    void setSize( double size );

    /** Returns the size for the whole symbol, which is the maximum size of
     * all marker symbol layers in the symbol.
     * @see setSize()
     * @see sizeUnit()
     * @see sizeMapUnitScale()
     */
    double size() const;

    /** Sets the size units for the whole symbol (including all symbol layers).
     * @param unit size units
     * @note added in QGIS 2.16
     * @see sizeUnit()
     * @see setSizeMapUnitScale()
     * @see setSize()
     */
    void setSizeUnit( OutputUnit unit );

    /** Returns the size units for the whole symbol (including all symbol layers).
     * @returns size units, or mixed units if symbol layers have different units
     * @note added in QGIS 2.16
     * @see setSizeUnit()
     * @see sizeMapUnitScale()
     * @see size()
     */
    OutputUnit sizeUnit() const;

    /** Sets the size map unit scale for the whole symbol (including all symbol layers).
     * @param scale map unit scale
     * @note added in QGIS 2.16
     * @see sizeMapUnitScale()
     * @see setSizeUnit()
     * @see setSize()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the size map unit scale for the whole symbol. Note that for symbols with
     * multiple symbol layers, this will correspond just to the map unit scale
     * for the first symbol layer.
     * @note added in QGIS 2.16
     * @see setSizeMapUnitScale()
     * @see sizeUnit()
     * @see size()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /** Set data defined size for whole symbol (including all symbol layers).
     * @param dd data defined size
     * @note added in QGIS 2.9
     * @see dataDefinedSize
     */
    void setDataDefinedSize( const QgsDataDefined& dd );

    /** Returns data defined size for whole symbol (including all symbol layers).
     * @returns data defined size, or empty data defined if size is not set
     * at the marker level
     * @note added in QGIS 2.9
     * @see setDataDefinedSize
     */
    QgsDataDefined dataDefinedSize() const;

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    ScaleMethod scaleMethod();

    void renderPoint( QPointF point, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    /** Returns the approximate bounding box of the marker symbol, which includes the bounding box
     * of all symbol layers for the symbol. It is recommended to use this method only between startRender()
     * and stopRender() calls, or data defined rotation and offset will not be correctly calculated.
     * @param point location of rendered point in painter units
     * @param context render context
     * @param feature feature being rendered at point (optional). If not specified, the bounds calculation will not
     * include data defined parameters such as offset and rotation
     * @returns approximate symbol bounds, in painter units
     * @note added in QGIS 2.14
    */
    QRectF bounds( QPointF point, QgsRenderContext& context, const QgsFeature &feature = QgsFeature() ) const;

    virtual QgsMarkerSymbolV2* clone() const override;

  private:

    void renderPointUsingLayer( QgsMarkerSymbolLayerV2* layer, QPointF point, QgsSymbolV2RenderContext& context );

};


/** \ingroup core
 * \class QgsLineSymbolV2
 */
class CORE_EXPORT QgsLineSymbolV2 : public QgsSymbolV2
{
  public:
    /** Create a line symbol with one symbol layer: SimpleLine with specified properties.
     * This is a convenience method for easier creation of line symbols.
     */
    static QgsLineSymbolV2* createSimple( const QgsStringMap& properties );

    QgsLineSymbolV2( const QgsSymbolLayerV2List& layers = QgsSymbolLayerV2List() );

    void setWidth( double width );
    double width() const;

    /** Set data defined width for whole symbol (including all symbol layers).
     * @param dd data defined width
     * @note added in QGIS 2.9
     * @see dataDefinedWidth
     */
    void setDataDefinedWidth( const QgsDataDefined& dd );

    /** Returns data defined size for whole symbol (including all symbol layers).
     * @returns data defined size, or empty data defined if size is not set
     * at the line level
     * @note added in QGIS 2.9
     * @see setDataDefinedWidth
     */
    QgsDataDefined dataDefinedWidth() const;

    void renderPolyline( const QPolygonF& points, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsLineSymbolV2* clone() const override;

  private:

    void renderPolylineUsingLayer( QgsLineSymbolLayerV2* layer, const QPolygonF& points, QgsSymbolV2RenderContext& context );

};


/** \ingroup core
 * \class QgsFillSymbolV2
 */
class CORE_EXPORT QgsFillSymbolV2 : public QgsSymbolV2
{
  public:
    /** Create a fill symbol with one symbol layer: SimpleFill with specified properties.
     * This is a convenience method for easier creation of fill symbols.
     */
    static QgsFillSymbolV2* createSimple( const QgsStringMap& properties );

    QgsFillSymbolV2( const QgsSymbolLayerV2List& layers = QgsSymbolLayerV2List() );
    void setAngle( double angle );
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsFillSymbolV2* clone() const override;

  private:

    void renderPolygonUsingLayer( QgsSymbolLayerV2* layer, const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolV2RenderContext &context );
    /** Calculates the bounds of a polygon including rings*/
    QRectF polygonBounds( const QPolygonF &points, const QList<QPolygonF> *rings ) const;
    /** Translates the rings in a polygon by a set distance*/
    QList<QPolygonF>* translateRings( const QList<QPolygonF> *rings, double dx, double dy ) const;
};

#endif


/*

QgsSymbolV2* ps = new QgsPointSymbol();

// ----

sl = QgsSymbolLayerV2Registry::instance()->createSymbolLayer("SimpleLine", { "color", "..." })

// (or)

sl = QgsSymbolLayerV2Registry::defaultSymbolLayer(QgsSymbolV2::Line)

// (or)

QgsSymbolLayerV2* sl = new QgsSimpleLineSymbolLayer(x,y,z);
QgsLineSymbol* s = new LineSymbol( [ sl ] );

// ----

rend = QgsSingleSymbolRenderer( new LineSymbol() );
*/


