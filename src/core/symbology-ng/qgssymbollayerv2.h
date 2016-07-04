/***************************************************************************
 qgssymbollayerv2.h
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
#ifndef QGSSYMBOLLAYERV2_H
#define QGSSYMBOLLAYERV2_H

// MSVC compiler doesn't have defined M_PI in math.h
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#define DEG2RAD(x)    ((x)*M_PI/180)
#define DEFAULT_SCALE_METHOD              QgsSymbolV2::ScaleDiameter

#include <QColor>
#include <QMap>
#include <QPointF>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h" // QgsStringMap
#include "qgsfield.h"

class QPainter;
class QSize;
class QPolygonF;

class QgsDxfExport;
class QgsExpression;
class QgsDataDefined;
class QgsRenderContext;
class QgsPaintEffect;

/** \ingroup core
 * \class QgsSymbolLayerV2
 */
class CORE_EXPORT QgsSymbolLayerV2
{
  public:

    virtual ~QgsSymbolLayerV2();

    /**
     * The fill color.
     */
    virtual QColor color() const { return mColor; }
    /**
     * The fill color.
     */
    virtual void setColor( const QColor& color ) { mColor = color; }

    /** Set outline color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual void setOutlineColor( const QColor& color ) { Q_UNUSED( color ); }

    /** Get outline color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual QColor outlineColor() const { return QColor(); }

    /** Set fill color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual void setFillColor( const QColor& color ) { Q_UNUSED( color ); }

    /** Get fill color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual QColor fillColor() const { return QColor(); }

    /**
     * Returns a string that represents this layer type. Used for serialization.
     * Should match with the string used to register this symbol layer in the registry.
     */
    virtual QString layerType() const = 0;

    virtual void startRender( QgsSymbolV2RenderContext& context ) = 0;
    virtual void stopRender( QgsSymbolV2RenderContext& context ) = 0;

    /**
     * Shall be reimplemented by subclasses to create a deep copy of the instance.
     */
    virtual QgsSymbolLayerV2* clone() const = 0;

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
    { Q_UNUSED( props ); element.appendChild( doc.createComment( QString( "SymbolLayerV2 %1 not implemented yet" ).arg( layerType() ) ) ); }

    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const { Q_UNUSED( mmScaleFactor ); Q_UNUSED( mapUnitScaleFactor ); return QString(); }

    /**
     * Should be reimplemented by subclasses to return a string map that
     * contains the configuration information for the symbol layer. This
     * is used to serialize a symbol layer perstistently.
     */
    virtual QgsStringMap properties() const = 0;

    virtual void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) = 0;

    virtual QgsSymbolV2* subSymbol() { return nullptr; }
    //! set layer's subsymbol. takes ownership of the passed symbol
    virtual bool setSubSymbol( QgsSymbolV2* symbol ) { delete symbol; return false; }

    QgsSymbolV2::SymbolType type() const { return mType; }

    //! Returns if the layer can be used below the specified symbol
    virtual bool isCompatibleWithSymbol( QgsSymbolV2* symbol ) const;

    void setLocked( bool locked ) { mLocked = locked; }
    bool isLocked() const { return mLocked; }

    /** Returns the estimated maximum distance which the layer style will bleed outside
      the drawn shape. Eg, polygons drawn with an outline will draw half the width
      of the outline outside of the polygon. This amount is estimated, since it may
      be affected by data defined symbology rules.*/
    virtual double estimateMaxBleed() const { return 0; }

    virtual void setOutputUnit( QgsSymbolV2::OutputUnit unit ) { Q_UNUSED( unit ); }
    virtual QgsSymbolV2::OutputUnit outputUnit() const { return QgsSymbolV2::Mixed; }

    virtual void setMapUnitScale( const QgsMapUnitScale& scale ) { Q_UNUSED( scale ); }
    virtual QgsMapUnitScale mapUnitScale() const { return QgsMapUnitScale(); }

    // used only with rending with symbol levels is turned on (0 = first pass, 1 = second, ...)
    void setRenderingPass( int renderingPass ) { mRenderingPass = renderingPass; }
    int renderingPass() const { return mRenderingPass; }

    /** Returns the set of attributes referenced by the layer. This includes attributes
     * required by any data defined properties associated with the layer.
     */
    virtual QSet<QString> usedAttributes() const;

    /** Returns a data defined expression for a property, if set
     * @deprecated use getDataDefinedProperty instead
     */
    Q_DECL_DEPRECATED virtual const QgsExpression* dataDefinedProperty( const QString& property ) const;

    /** Returns a data defined expression for a property, if set
     * @deprecated use getDataDefinedProperty instead
     */
    Q_DECL_DEPRECATED virtual QString dataDefinedPropertyString( const QString& property ) const;

    /** Sets a data defined expression for a property
     * @deprecated use setDataDefinedProperty( const QString& property, QgsDataDefined* dataDefined ) instead
     */
    Q_DECL_DEPRECATED virtual void setDataDefinedProperty( const QString& property, const QString& expressionString );

    /** Returns the data defined property corresponding to the specified property key
     * @param property property key
     * @returns matching data defined property if it exists
     * @note added in QGIS 2.9
     * @see setDataDefinedProperty
     * @see hasDataDefinedProperty
     * @see evaluateDataDefinedProperty
     */
    virtual QgsDataDefined* getDataDefinedProperty( const QString& property ) const;

    /** Sets a data defined property for the layer.
     * @param property unique property key. Any existing data defined with the same key will be deleted and overridden.
     * @param dataDefined data defined object to associate with property key. Ownership is transferred to the layer.
     * @note added in QGIS 2.9
     * @see getDataDefinedProperty
     * @see removeDataDefinedProperty
     */
    virtual void setDataDefinedProperty( const QString& property, QgsDataDefined* dataDefined );

    /** Removes a data defined property from the layer.
     * @param property unique property key. If an associated QgsDataDefined object exists,
     * it will be deleted and removed from the layer.
     * @note added in QGIS 2.9
     * @see setDataDefinedProperty
     * @see removeDataDefinedProperties
     */
    virtual void removeDataDefinedProperty( const QString& property );

    /** Removes all data defined properties from the layer and deletes associated
     * objects.
     * @see removeDataDefinedProperty
     * @note added in QGIS 2.9
     */
    virtual void removeDataDefinedProperties();

    /** Checks whether the layer has any associated data defined properties.
     * @returns true if layer has data defined properties
     * @see hasDataDefinedProperty
     */
    virtual bool hasDataDefinedProperties() const;

    /** Checks whether the layer has a matching data defined property and if
     * that property is currently actived.
     * @param property property key
     * @returns true if data defined property exists and is active
     * @see hasDataDefinedProperties
     * @see evaluateDataDefinedProperty
     * @see getDataDefinedProperty
     * @note added in QGIS 2.9
     */
    virtual bool hasDataDefinedProperty( const QString& property ) const;

    /** Evaluates the matching data defined property and returns the calculated
     * value. Prior to evaluation the data defined property must be prepared
     * by calling @link prepareExpressions @endlink.
     * @param property property key
     * @param feature pointer to the feature to use during expression or field
     * evaluation
     * @param defaultVal default value to return if evaluation was not successful
     * @param ok if specified, will be set to true if evaluation was successful
     * @returns calculated value for data defined property, or default value
     * if property does not exist or is deactived.
     * @see hasDataDefinedProperty
     * @see getDataDefinedProperty
     * @note added in QGIS 2.9
     * @deprecated use variant which takes QgsSymbolV2RenderContext instead
     */
    Q_DECL_DEPRECATED virtual QVariant evaluateDataDefinedProperty( const QString& property, const QgsFeature* feature, const QVariant& defaultVal = QVariant(), bool *ok = nullptr ) const;

    /** Evaluates the matching data defined property and returns the calculated
     * value. Prior to evaluation the data defined property must be prepared
     * by calling @link prepareExpressions @endlink.
     * @param property property key
     * @param context symbol render context
     * @param defaultVal default value to return if evaluation was not successful
     * @param ok if specified, will be set to true if evaluation was successful
     * @returns calculated value for data defined property, or default value
     * if property does not exist or is deactived.
     * @see hasDataDefinedProperty
     * @see getDataDefinedProperty
     * @note added in QGIS 2.12
     */
    virtual QVariant evaluateDataDefinedProperty( const QString& property, const QgsSymbolV2RenderContext& context, const QVariant& defaultVal = QVariant(), bool *ok = nullptr ) const;

    //! write as DXF
    virtual bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolV2RenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;

    //! get line width
    virtual double dxfWidth( const QgsDxfExport& e, QgsSymbolV2RenderContext& context ) const;

    //! get offset
    virtual double dxfOffset( const QgsDxfExport& e, QgsSymbolV2RenderContext& context ) const;

    //! get color
    virtual QColor dxfColor( QgsSymbolV2RenderContext& context ) const;

    //! get angle
    virtual double dxfAngle( QgsSymbolV2RenderContext& context ) const;

    //! get dash pattern
    virtual QVector<qreal> dxfCustomDashPattern( QgsSymbolV2::OutputUnit& unit ) const;

    //! get pen style
    virtual Qt::PenStyle dxfPenStyle() const;

    //! get brush/fill color
    virtual QColor dxfBrushColor( QgsSymbolV2RenderContext& context ) const;

    //! get brush/fill style
    virtual Qt::BrushStyle dxfBrushStyle() const;

    /** Returns the current paint effect for the layer.
     * @returns paint effect
     * @note added in QGIS 2.9
     * @see setPaintEffect
     */
    QgsPaintEffect* paintEffect() const;

    /** Sets the current paint effect for the layer.
     * @param effect paint effect. Ownership is transferred to the layer.
     * @note added in QGIS 2.9
     * @see paintEffect
     */
    void setPaintEffect( QgsPaintEffect* effect );

  protected:
    QgsSymbolLayerV2( QgsSymbolV2::SymbolType type, bool locked = false );

    QgsSymbolV2::SymbolType mType;
    bool mLocked;
    QColor mColor;
    int mRenderingPass;

    QMap< QString, QgsDataDefined* > mDataDefinedProperties;
    QgsPaintEffect* mPaintEffect;
    QgsFields mFields;

    // Configuration of selected symbology implementation
    static const bool selectionIsOpaque = true;  // Selection ignores symbol alpha
    static const bool selectFillBorder = false;  // Fill symbol layer also selects border symbology
    static const bool selectFillStyle = false;   // Fill symbol uses symbol layer style..

    /** Prepares all data defined property expressions for evaluation. This should
     * be called prior to evaluating data defined properties.
     * @param fields associated layer fields
     * @param scale map scale
     * @deprecated use variant which takes QgsSymbolV2RenderContext instead
     */
    Q_DECL_DEPRECATED virtual void prepareExpressions( const QgsFields* fields, double scale = -1.0 );

    /** Prepares all data defined property expressions for evaluation. This should
     * be called prior to evaluating data defined properties.
     * @param context symbol render context
     * @note added in QGIS 2.12
     */
    virtual void prepareExpressions( const QgsSymbolV2RenderContext& context );

    /** Returns the data defined expression associated with a property
     * @deprecated use getDataDefinedProperty or evaluateDataDefinedProperty instead
     */
    Q_DECL_DEPRECATED virtual QgsExpression* expression( const QString& property ) const;

    /** Saves all data defined properties to a string map.
     * @param stringMap destination string map
     * @see restoreDataDefinedProperties
     */
    void saveDataDefinedProperties( QgsStringMap& stringMap ) const;

    /** Restores all data defined properties from string map.
     * @param stringMap source string map
     * @note added in QGIS 2.9
     * @see saveDataDefinedProperties
     */
    void restoreDataDefinedProperties( const QgsStringMap& stringMap );

    /** Copies all data defined properties of this layer to another symbol layer.
     * @param destLayer destination layer
     */
    void copyDataDefinedProperties( QgsSymbolLayerV2* destLayer ) const;

    /** Copies paint effect of this layer to another symbol layer
     * @param destLayer destination layer
     * @note added in QGIS 2.9
     */
    void copyPaintEffect( QgsSymbolLayerV2* destLayer ) const;

    static const QString EXPR_SIZE;
    static const QString EXPR_ANGLE;
    static const QString EXPR_NAME;
    static const QString EXPR_COLOR;
    static const QString EXPR_COLOR_BORDER;
    static const QString EXPR_OUTLINE_WIDTH;
    static const QString EXPR_OUTLINE_STYLE;
    static const QString EXPR_FILL;
    static const QString EXPR_OUTLINE;
    static const QString EXPR_OFFSET;
    static const QString EXPR_CHAR;
    static const QString EXPR_FILL_COLOR;
    static const QString EXPR_OUTLINE_COLOR;
    static const QString EXPR_WIDTH;
    static const QString EXPR_HEIGHT;
    static const QString EXPR_SYMBOL_NAME;
    static const QString EXPR_ROTATION;
    static const QString EXPR_FILL_STYLE;
    static const QString EXPR_WIDTH_BORDER;
    static const QString EXPR_BORDER_STYLE;
    static const QString EXPR_JOIN_STYLE;
    static const QString EXPR_BORDER_COLOR;
    static const QString EXPR_COLOR2;
    static const QString EXPR_LINEANGLE;
    static const QString EXPR_GRADIENT_TYPE;
    static const QString EXPR_COORDINATE_MODE;
    static const QString EXPR_SPREAD;
    static const QString EXPR_REFERENCE1_X;
    static const QString EXPR_REFERENCE1_Y;
    static const QString EXPR_REFERENCE2_X;
    static const QString EXPR_REFERENCE2_Y;
    static const QString EXPR_REFERENCE1_ISCENTROID;
    static const QString EXPR_REFERENCE2_ISCENTROID;
    static const QString EXPR_BLUR_RADIUS;
    static const QString EXPR_DISTANCE;
    static const QString EXPR_USE_WHOLE_SHAPE;
    static const QString EXPR_MAX_DISTANCE;
    static const QString EXPR_IGNORE_RINGS;
    static const QString EXPR_SVG_FILE;
    static const QString EXPR_SVG_FILL_COLOR;
    static const QString EXPR_SVG_OUTLINE_COLOR;
    static const QString EXPR_SVG_OUTLINE_WIDTH;
    static const QString EXPR_LINEWIDTH;
    static const QString EXPR_DISTANCE_X;
    static const QString EXPR_DISTANCE_Y;
    static const QString EXPR_DISPLACEMENT_X;
    static const QString EXPR_DISPLACEMENT_Y;
    static const QString EXPR_FILE;
    static const QString EXPR_ALPHA;
    static const QString EXPR_CUSTOMDASH;
    static const QString EXPR_LINE_STYLE;
    static const QString EXPR_JOINSTYLE; //near duplicate is required to maintain project compatibility
    static const QString EXPR_CAPSTYLE;
    static const QString EXPR_PLACEMENT;
    static const QString EXPR_INTERVAL;
    static const QString EXPR_OFFSET_ALONG_LINE;
    static const QString EXPR_HORIZONTAL_ANCHOR_POINT;
    static const QString EXPR_VERTICAL_ANCHOR_POINT;
};

//////////////////////

/** \ingroup core
 * \class QgsMarkerSymbolLayerV2
 * \brief Abstract base class for marker symbol layers.
 */
class CORE_EXPORT QgsMarkerSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:

    //! Symbol horizontal anchor points
    enum HorizontalAnchorPoint
    {
      Left, /*!< Align to left side of symbol */
      HCenter, /*!< Align to horizontal center of symbol */
      Right, /*!< Align to right side of symbol */
    };

    //! Symbol vertical anchor points
    enum VerticalAnchorPoint
    {
      Top, /*!< Align to top of symbol */
      VCenter, /*!< Align to vertical center of symbol */
      Bottom, /*!< Align to bottom of symbol */
    };

    void startRender( QgsSymbolV2RenderContext& context ) override;

    /** Renders a marker at the specified point. Derived classes must implement this to
     * handle drawing the point.
     * @param point position at which to render point, in painter units
     * @param context symbol render context
     */
    virtual void renderPoint( QPointF point, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) override;

    /** Sets the rotation angle for the marker.
     * @param angle angle in degrees clockwise from north.
     * @see angle()
     * @see setLineAngle()
     */
    void setAngle( double angle ) { mAngle = angle; }

    /** Returns the rotation angle for the marker, in degrees clockwise from north.
     * @see setAngle()
     */
    double angle() const { return mAngle; }

    /** Sets the line angle modification for the symbol's angle. This angle is added to
     * the marker's rotation and data defined rotation before rendering the symbol, and
     * is usually used for orienting symbols to match a line's angle.
     * @param lineAngle Angle in degrees clockwise from north, valid values are between 0 and 360
     * @note added in QGIS 2.9
     * @see setAngle()
     * @see angle()
     */
    void setLineAngle( double lineAngle ) { mLineAngle = lineAngle; }

    /** Sets the symbol size.
     * @param size symbol size. Units are specified by sizeUnit().
     * @see size()
     * @see setSizeUnit()
     * @see setSizeMapUnitScale()
     */
    void setSize( double size ) { mSize = size; }

    /** Returns the symbol size. Units are specified by sizeUnit().
     * @see setSize()
     * @see sizeUnit()
     * @see sizeUnitMapScale()
     */
    double size() const { return mSize; }

    /** Sets the units for the symbol's size.
     * @param unit size units
     * @see sizeUnit()
     * @see setSize()
     * @see setSizeMapUnitScale()
     */
    void setSizeUnit( QgsSymbolV2::OutputUnit unit ) { mSizeUnit = unit; }

    /** Returns the units for the symbol's size.
     * @see setSizeUnit()
     * @see size()
     * @see sizeMapUnitScale()
     */
    QgsSymbolV2::OutputUnit sizeUnit() const { return mSizeUnit; }

    /** Sets the map unit scale for the symbol's size.
     * @param scale size map unit scale
     * @see sizeMapUnitScale()
     * @see setSize()
     * @see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale& scale ) { mSizeMapUnitScale = scale; }

    /** Returns the map unit scale for the symbol's size.
     * @see setSizeMapUnitScale()
     * @see size()
     * @see sizeUnit()
     */
    const QgsMapUnitScale& sizeMapUnitScale() const { return mSizeMapUnitScale; }

    /** Sets the method to use for scaling the marker's size.
     * @param scaleMethod scale method
     * @see scaleMethod()
     */
    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod ) { mScaleMethod = scaleMethod; }

    /** Returns the method to use for scaling the marker's size.
     * @see setScaleMethod()
     */
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }

    /** Sets the marker's offset, which is the horizontal and vertical displacement which the rendered marker
     * should have from the original feature's geometry.
     * @param offset marker offset. Units are specified by offsetUnit()
     * @see offset()
     * @see setOffsetUnit()
     * @see setOffsetMapUnitScale()
     */
    void setOffset( QPointF offset ) { mOffset = offset; }

    /** Returns the marker's offset, which is the horizontal and vertical displacement which the rendered marker
     * will have from the original feature's geometry. Units are specified by offsetUnit().
     * @see setOffset()
     * @see offsetUnit()
     * @see offsetMapUnitScale()
     */
    QPointF offset() const { return mOffset; }

    /** Sets the units for the symbol's offset.
     * @param unit offset units
     * @see offsetUnit()
     * @see setOffset()
     * @see setOffsetMapUnitScale()
     */
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }

    /** Returns the units for the symbol's offset.
     * @see setOffsetUnit()
     * @see offset()
     * @see offsetMapUnitScale()
     */
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    /** Sets the map unit scale for the symbol's offset.
     * @param scale offset map unit scale
     * @see offsetMapUnitScale()
     * @see setOffset()
     * @see setOffsetUnit()
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }

    /** Returns the map unit scale for the symbol's offset.
     * @see setOffsetMapUnitScale()
     * @see offset()
     * @see offsetUnit()
     */
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    /** Sets the horizontal anchor point for positioning the symbol.
     * @param h anchor point. Symbol will be drawn so that the horizontal anchor point is aligned with
     * the marker's desired location.
     * @see horizontalAnchorPoint()
     * @see setVerticalAnchorPoint()
     */
    void setHorizontalAnchorPoint( HorizontalAnchorPoint h ) { mHorizontalAnchorPoint = h; }

    /** Returns the horizontal anchor point for positioning the symbol. The symbol will be drawn so that
     * the horizontal anchor point is aligned with the marker's desired location.
     * @see setHorizontalAnchorPoint()
     * @see verticalAnchorPoint()
     */
    HorizontalAnchorPoint horizontalAnchorPoint() const { return mHorizontalAnchorPoint; }

    /** Sets the vertical anchor point for positioning the symbol.
     * @param v anchor point. Symbol will be drawn so that the vertical anchor point is aligned with
     * the marker's desired location.
     * @see verticalAnchorPoint()
     * @see setHorizontalAnchorPoint()
     */
    void setVerticalAnchorPoint( VerticalAnchorPoint v ) { mVerticalAnchorPoint = v; }

    /** Returns the vertical anchor point for positioning the symbol. The symbol will be drawn so that
     * the vertical anchor point is aligned with the marker's desired location.
     * @see setVerticalAnchorPoint()
     * @see horizontalAnchorPoint()
     */
    VerticalAnchorPoint verticalAnchorPoint() const { return mVerticalAnchorPoint; }

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const override;

    /** Writes the symbol layer definition as a SLD XML element.
     * @param doc XML document
     * @param element parent XML element
     * @param props symbol layer definition (see properties())
     */
    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
    { Q_UNUSED( props ); element.appendChild( doc.createComment( QString( "QgsMarkerSymbolLayerV2 %1 not implemented yet" ).arg( layerType() ) ) ); }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit ) override;
    QgsSymbolV2::OutputUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale& scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    /** Returns the approximate bounding box of the marker symbol layer, taking into account
     * any data defined overrides and offsets which are set for the marker layer.
     * @returns approximate symbol bounds, in painter units
     * @note added in QGIS 2.14
     * @note this method will become pure virtual in QGIS 3.0
     */
    //TODO QGIS 3.0 - make pure virtual
    virtual QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context ) { Q_UNUSED( context ); Q_UNUSED( point ); return QRectF(); }

  protected:

    /** Constructor for QgsMarkerSymbolLayerV2.
     * @param locked set to true to lock symbol color
     */
    QgsMarkerSymbolLayerV2( bool locked = false );

    /** Calculates the required marker offset, including both the symbol offset
     * and any displacement required to align with the marker's anchor point.
     * @param context symbol render context
     * @param offsetX will be set to required horizontal offset (in painter units)
     * @param offsetY will be set to required vertical offset (in painter units)
     */
    void markerOffset( QgsSymbolV2RenderContext& context, double& offsetX, double& offsetY ) const;

    /** Calculates the required marker offset, including both the symbol offset
     * and any displacement required to align with the marker's anchor point.
     * @param context symbol render context
     * @param width marker width
     * @param height marker height
     * @param offsetX will be set to required horizontal offset (in painter units)
     * @param offsetY will be set to required vertical offset (in painter units)
     * @note available in python as markerOffsetWithWidthAndHeight
     */
    void markerOffset( QgsSymbolV2RenderContext& context, double width, double height, double& offsetX, double& offsetY ) const;

    //! @note available in python bindings as markerOffset2
    void markerOffset( QgsSymbolV2RenderContext& context, double width, double height,
                       QgsSymbolV2::OutputUnit widthUnit, QgsSymbolV2::OutputUnit heightUnit,
                       double& offsetX, double& offsetY,
                       const QgsMapUnitScale &widthMapUnitScale, const QgsMapUnitScale &heightMapUnitScale ) const;

    /** Adjusts a marker offset to account for rotation.
     * @param offset offset prior to rotation
     * @param angle rotation angle in degrees clockwise from north
     * @return adjusted offset
     */
    static QPointF _rotatedOffset( QPointF offset, double angle );

    //! Marker rotation angle, in degrees clockwise from north
    double mAngle;
    //! Line rotation angle (see setLineAngle() for details)
    double mLineAngle;
    //! Marker size
    double mSize;
    //! Marker size unit
    QgsSymbolV2::OutputUnit mSizeUnit;
    //! Marker size map unit scale
    QgsMapUnitScale mSizeMapUnitScale;
    //! Marker offset
    QPointF mOffset;
    //! Offset units
    QgsSymbolV2::OutputUnit mOffsetUnit;
    //! Offset map unit scale
    QgsMapUnitScale mOffsetMapUnitScale;
    //! Marker size scaling method
    QgsSymbolV2::ScaleMethod mScaleMethod;
    //! Horizontal anchor point
    HorizontalAnchorPoint mHorizontalAnchorPoint;
    //! Vertical anchor point
    VerticalAnchorPoint mVerticalAnchorPoint;

  private:
    static QgsMarkerSymbolLayerV2::HorizontalAnchorPoint decodeHorizontalAnchorPoint( const QString& str );
    static QgsMarkerSymbolLayerV2::VerticalAnchorPoint decodeVerticalAnchorPoint( const QString& str );
};

/** \ingroup core
 * \class QgsLineSymbolLayerV2
 */
class CORE_EXPORT QgsLineSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context ) = 0;

    virtual void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual void setWidth( double width ) { mWidth = width; }
    virtual double width() const { return mWidth; }

    double offset() const { return mOffset; }
    void setOffset( double offset ) { mOffset = offset; }

    void setWidthUnit( QgsSymbolV2::OutputUnit unit ) { mWidthUnit = unit; }
    QgsSymbolV2::OutputUnit widthUnit() const { return mWidthUnit; }

    void setWidthMapUnitScale( const QgsMapUnitScale& scale ) { mWidthMapUnitScale = scale; }
    const QgsMapUnitScale& widthMapUnitScale() const { return mWidthMapUnitScale; }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit ) override;
    QgsSymbolV2::OutputUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale& scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) override;

    virtual double dxfWidth( const QgsDxfExport& e, QgsSymbolV2RenderContext& context ) const override;

  protected:
    QgsLineSymbolLayerV2( bool locked = false );

    double mWidth;
    QgsSymbolV2::OutputUnit mWidthUnit;
    QgsMapUnitScale mWidthMapUnitScale;
    double mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;
};

/** \ingroup core
 * \class QgsFillSymbolLayerV2
 */
class CORE_EXPORT QgsFillSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) override;

    void setAngle( double angle ) { mAngle = angle; }
    double angle() const { return mAngle; }

  protected:
    QgsFillSymbolLayerV2( bool locked = false );
    /** Default method to render polygon*/
    void _renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    double mAngle;
};

class QgsSymbolLayerV2Widget;  // why does SIP fail, when this isn't here

#endif


