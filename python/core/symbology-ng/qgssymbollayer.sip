class QgsSymbolLayer
{
%TypeHeaderCode
#include <qgssymbollayer.h>
#include <qgslinesymbollayer.h>
%End

%ConvertToSubClassCode
  switch (sipCpp->type())
  {
    case QgsSymbol::Marker:
      if (dynamic_cast<QgsEllipseSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsEllipseSymbolLayer;
      else if (dynamic_cast<QgsFontMarkerSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsFontMarkerSymbolLayer;
      else if (dynamic_cast<QgsSimpleMarkerSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsSimpleMarkerSymbolLayer;
      else if (dynamic_cast<QgsFilledMarkerSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsFilledMarkerSymbolLayer;
      else if (dynamic_cast<QgsSvgMarkerSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsSvgMarkerSymbolLayer;
      else if (dynamic_cast<QgsVectorFieldSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsVectorFieldSymbolLayer;
      else
	sipType = sipType_QgsMarkerSymbolLayer;
      break;

    case QgsSymbol::Line:
      if (dynamic_cast<QgsMarkerLineSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsMarkerLineSymbolLayer;
      else if (dynamic_cast<QgsSimpleLineSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsSimpleLineSymbolLayer;
      else if (dynamic_cast<QgsArrowSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsArrowSymbolLayer;
      else
	sipType = sipType_QgsLineSymbolLayer;
      break;

    case QgsSymbol::Fill:
      if (dynamic_cast<QgsSimpleFillSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsSimpleFillSymbolLayer;
      else if (dynamic_cast<QgsImageFillSymbolLayer*>(sipCpp) != NULL)
      {
	if (dynamic_cast<QgsLinePatternFillSymbolLayer*>(sipCpp) != NULL)
	  sipType = sipType_QgsLinePatternFillSymbolLayer;
	else if (dynamic_cast<QgsPointPatternFillSymbolLayer*>(sipCpp) != NULL)
	  sipType = sipType_QgsPointPatternFillSymbolLayer;
	else if (dynamic_cast<QgsSVGFillSymbolLayer*>(sipCpp) != NULL)
	  sipType = sipType_QgsSVGFillSymbolLayer;
	else if (dynamic_cast<QgsRasterFillSymbolLayer*>(sipCpp) != NULL)
	  sipType = sipType_QgsRasterFillSymbolLayer;
	else
	  sipType = sipType_QgsImageFillSymbolLayer;
      }
      else if (dynamic_cast<QgsCentroidFillSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsCentroidFillSymbolLayer;
      else if (dynamic_cast<QgsGradientFillSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsGradientFillSymbolLayer;
      else if (dynamic_cast<QgsShapeburstFillSymbolLayer*>(sipCpp) != NULL)
	sipType = sipType_QgsShapeburstFillSymbolLayer;
      else
	sipType = sipType_QgsFillSymbolLayer;
      break;

    case QgsSymbol::Hybrid:
      sipType = sipType_QgsGeometryGeneratorSymbolLayer;
      break;
  }
%End

  public:

    virtual ~QgsSymbolLayer();

    /**
     * The fill color.
     */
    virtual QColor color() const;
    /**
     * The fill color.
     */
    virtual void setColor( const QColor& color );

    /** Set outline color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual void setOutlineColor( const QColor& color );

    /** Get outline color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual QColor outlineColor() const;

    /** Set fill color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual void setFillColor( const QColor& color );

    /** Get fill color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual QColor fillColor() const;

    /**
     * Returns a string that represents this layer type. Used for serialization.
     * Should match with the string used to register this symbol layer in the registry.
     */
    virtual QString layerType() const = 0;

    virtual void startRender( QgsSymbolRenderContext& context ) = 0;
    virtual void stopRender( QgsSymbolRenderContext& context ) = 0;

    /**
     * Shall be reimplemented by subclasses to create a deep copy of the instance.
     */
    virtual QgsSymbolLayer* clone() const = 0 /Factory/;

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    /**
     * Should be reimplemented by subclasses to return a string map that
     * contains the configuration information for the symbol layer. This
     * is used to serialize a symbol layer perstistently.
     */
    virtual QgsStringMap properties() const = 0;

    virtual void drawPreviewIcon( QgsSymbolRenderContext& context, QSize size ) = 0;

    virtual QgsSymbol* subSymbol();
    //! set layer's subsymbol. takes ownership of the passed symbol
    virtual bool setSubSymbol( QgsSymbol* symbol /Transfer/ );

    QgsSymbol::SymbolType type() const;

    //! Returns if the layer can be used below the specified symbol
    virtual bool isCompatibleWithSymbol( QgsSymbol* symbol ) const;

    void setLocked( bool locked );
    bool isLocked() const;

    /** Returns the estimated maximum distance which the layer style will bleed outside
      the drawn shape. Eg, polygons drawn with an outline will draw half the width
      of the outline outside of the polygon. This amount is estimated, since it may
      be affected by data defined symbology rules.*/
    virtual double estimateMaxBleed() const;

    /** Sets the units to use for sizes and widths within the symbol layer. Individual
     * symbol layer subclasses will interpret this in different ways, eg a marker symbol
     * layer may use it to specify the units for the marker size, while a line symbol
     * layer may use it to specify the units for the line width.
     * @param unit output units
     * @see outputUnit()
     */
    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units to use for sizes and widths within the symbol layer. Individual
     * symbol layer subclasses will interpret this in different ways, eg a marker symbol
     * layer may use it to specify the units for the marker size, while a line symbol
     * layer may use it to specify the units for the line width.
     * @returns output unit, or QgsUnitTypes::RenderUnknownUnit if the symbol layer contains mixed units
     * @see setOutputUnit()
     */
    virtual QgsUnitTypes::RenderUnit outputUnit() const;

    virtual void setMapUnitScale( const QgsMapUnitScale& scale );
    virtual QgsMapUnitScale mapUnitScale() const;

    // used only with rending with symbol levels is turned on (0 = first pass, 1 = second, ...)
    void setRenderingPass( int renderingPass );
    int renderingPass() const;

    /** Returns the set of attributes referenced by the layer. This includes attributes
     * required by any data defined properties associated with the layer.
     */
    virtual QSet<QString> usedAttributes() const;

    /** Returns a data defined expression for a property, if set
     * @deprecated use getDataDefinedProperty instead
     */
    virtual const QgsExpression* dataDefinedProperty( const QString& property ) const /Deprecated/;

    /** Returns a data defined expression for a property, if set
     * @deprecated use getDataDefinedProperty instead
     */
    virtual QString dataDefinedPropertyString( const QString& property ) const /Deprecated/;

    /** Sets a data defined expression for a property
     * @deprecated use setDataDefinedProperty( const QString& property, QgsDataDefined* dataDefined ) instead
     */
    virtual void setDataDefinedProperty( const QString& property, const QString& expressionString ) /Deprecated/;

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
    virtual void setDataDefinedProperty( const QString& property, QgsDataDefined* dataDefined /Transfer/ );

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
     * @deprecated use variant which takes QgsSymbolRenderContext instead
     */
    virtual QVariant evaluateDataDefinedProperty( const QString& property, const QgsFeature* feature ) const /Deprecated/;

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
    virtual QVariant evaluateDataDefinedProperty( const QString& property, const QgsSymbolRenderContext& context, const QVariant& defaultVal = QVariant(), bool *ok = 0 ) const;

    virtual bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;

    virtual double dxfWidth( const QgsDxfExport& e, QgsSymbolRenderContext& context ) const;
    virtual double dxfOffset( const QgsDxfExport& e, QgsSymbolRenderContext& context ) const;

    virtual QColor dxfColor( QgsSymbolRenderContext& context ) const;
    virtual double dxfAngle( QgsSymbolRenderContext& context ) const;

    virtual QVector<qreal> dxfCustomDashPattern( QgsUnitTypes::RenderUnit& unit ) const;
    virtual Qt::PenStyle dxfPenStyle() const;
    virtual QColor dxfBrushColor( QgsSymbolRenderContext& context ) const;
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
    void setPaintEffect( QgsPaintEffect* effect /Transfer/);

  protected:
    QgsSymbolLayer( QgsSymbol::SymbolType type, bool locked = false );

    /** Prepares all data defined property expressions for evaluation. This should
     * be called prior to evaluating data defined properties.
     * @param context symbol render context
     * @note added in QGIS 2.12
     */
    virtual void prepareExpressions( const QgsSymbolRenderContext& context );

    /** Returns the data defined expression associated with a property
     * @deprecated use getDataDefinedProperty or evaluateDataDefinedProperty instead
     */
    virtual QgsExpression* expression( const QString& property ) const /Deprecated/;

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
    void copyDataDefinedProperties( QgsSymbolLayer* destLayer ) const;

    /** Copies paint effect of this layer to another symbol layer
     * @param destLayer destination layer
     * @note added in QGIS 2.9
     */
    void copyPaintEffect( QgsSymbolLayer* destLayer ) const;

};

//////////////////////

/** \ingroup core
 * \class QgsMarkerSymbolLayer
 * \brief Abstract base class for marker symbol layers.
 */
class QgsMarkerSymbolLayer : QgsSymbolLayer
{
%TypeHeaderCode
#include <qgssymbollayer.h>
%End
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

    void startRender( QgsSymbolRenderContext& context );

    /** Renders a marker at the specified point. Derived classes must implement this to
     * handle drawing the point.
     * @param point position at which to render point, in painter units
     * @param context symbol render context
     */
    virtual void renderPoint( QPointF point, QgsSymbolRenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolRenderContext& context, QSize size );

    /** Sets the rotation angle for the marker.
     * @param angle angle in degrees clockwise from north.
     * @see angle()
     * @see setLineAngle()
     */
    void setAngle( double angle );

    /** Returns the rotation angle for the marker, in degrees clockwise from north.
     * @see setAngle()
     */
    double angle() const;

    /** Sets the line angle modification for the symbol's angle. This angle is added to
     * the marker's rotation and data defined rotation before rendering the symbol, and
     * is usually used for orienting symbols to match a line's angle.
     * @param lineAngle Angle in degrees clockwise from north, valid values are between 0 and 360
     * @note added in QGIS 2.9
     * @see setAngle()
     * @see angle()
     */
    void setLineAngle( double lineAngle );

    /** Sets the symbol size.
     * @param size symbol size. Units are specified by sizeUnit().
     * @see size()
     * @see setSizeUnit()
     * @see setSizeMapUnitScale()
     */
    void setSize( double size );

    /** Returns the symbol size. Units are specified by sizeUnit().
     * @see setSize()
     * @see sizeUnit()
     * @see sizeUnitMapScale()
     */
    double size() const;

    /** Sets the units for the symbol's size.
     * @param unit size units
     * @see sizeUnit()
     * @see setSize()
     * @see setSizeMapUnitScale()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the symbol's size.
     * @see setSizeUnit()
     * @see size()
     * @see sizeMapUnitScale()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /** Sets the map unit scale for the symbol's size.
     * @param scale size map unit scale
     * @see sizeMapUnitScale()
     * @see setSize()
     * @see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale& scale);

    /** Returns the map unit scale for the symbol's size.
     * @see setSizeMapUnitScale()
     * @see size()
     * @see sizeUnit()
     */
    const QgsMapUnitScale& sizeMapUnitScale() const;

    /** Sets the method to use for scaling the marker's size.
     * @param scaleMethod scale method
     * @see scaleMethod()
     */
    void setScaleMethod( QgsSymbol::ScaleMethod scaleMethod );

    /** Returns the method to use for scaling the marker's size.
     * @see setScaleMethod()
     */
    QgsSymbol::ScaleMethod scaleMethod() const;

    /** Sets the marker's offset, which is the horizontal and vertical displacement which the rendered marker
     * should have from the original feature's geometry.
     * @param offset marker offset. Units are specified by offsetUnit()
     * @see offset()
     * @see setOffsetUnit()
     * @see setOffsetMapUnitScale()
     */
    void setOffset( QPointF offset );

    /** Returns the marker's offset, which is the horizontal and vertical displacement which the rendered marker
     * will have from the original feature's geometry. Units are specified by offsetUnit().
     * @see setOffset()
     * @see offsetUnit()
     * @see offsetMapUnitScale()
     */
    QPointF offset();

    /** Sets the units for the symbol's offset.
     * @param unit offset units
     * @see offsetUnit()
     * @see setOffset()
     * @see setOffsetMapUnitScale()
     */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the symbol's offset.
     * @see setOffsetUnit()
     * @see offset()
     * @see offsetMapUnitScale()
     */
    QgsUnitTypes::RenderUnit offsetUnit() const;

   /** Sets the map unit scale for the symbol's offset.
     * @param scale offset map unit scale
     * @see offsetMapUnitScale()
     * @see setOffset()
     * @see setOffsetUnit()
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale& scale);

    /** Returns the map unit scale for the symbol's offset.
     * @see setOffsetMapUnitScale()
     * @see offset()
     * @see offsetUnit()
     */
    const QgsMapUnitScale& offsetMapUnitScale() const;

    /** Sets the horizontal anchor point for positioning the symbol.
     * @param h anchor point. Symbol will be drawn so that the horizontal anchor point is aligned with
     * the marker's desired location.
     * @see horizontalAnchorPoint()
     * @see setVerticalAnchorPoint()
     */
    void setHorizontalAnchorPoint( HorizontalAnchorPoint h );

    /** Returns the horizontal anchor point for positioning the symbol. The symbol will be drawn so that
     * the horizontal anchor point is aligned with the marker's desired location.
     * @see setHorizontalAnchorPoint()
     * @see verticalAnchorPoint()
     */
    HorizontalAnchorPoint horizontalAnchorPoint() const;

    /** Sets the vertical anchor point for positioning the symbol.
     * @param v anchor point. Symbol will be drawn so that the vertical anchor point is aligned with
     * the marker's desired location.
     * @see verticalAnchorPoint()
     * @see setHorizontalAnchorPoint()
     */
    void setVerticalAnchorPoint( VerticalAnchorPoint v );

    /** Returns the vertical anchor point for positioning the symbol. The symbol will be drawn so that
     * the vertical anchor point is aligned with the marker's desired location.
     * @see setVerticalAnchorPoint()
     * @see horizontalAnchorPoint()
     */
    VerticalAnchorPoint verticalAnchorPoint() const;

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    /** Writes the symbol layer definition as a SLD XML element.
     * @param doc XML document
     * @param element parent XML element
     * @param props symbol layer definition (see properties())
     */
    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    void setOutputUnit( QgsUnitTypes::RenderUnit unit );
    QgsUnitTypes::RenderUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    /** Returns the approximate bounding box of the marker symbol layer, taking into account
     * any data defined overrides and offsets which are set for the marker layer.
     * @returns approximate symbol bounds, in painter units
     * @note added in QGIS 2.14
     * @note this method will become pure virtual in QGIS 3.0
     */
    //TODO QGIS 3.0 - make pure virtual
    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext& context );

  protected:

    /** Constructor for QgsMarkerSymbolLayer.
     * @param locked set to true to lock symbol color
     */
    QgsMarkerSymbolLayer( bool locked = false );

    /** Calculates the required marker offset, including both the symbol offset
     * and any displacement required to align with the marker's anchor point.
     * @param context symbol render context
     * @param offsetX will be set to required horizontal offset (in painter units)
     * @param offsetY will be set to required vertical offset (in painter units)
     */
    void markerOffset( QgsSymbolRenderContext& context, double& offsetX, double& offsetY ) const;

    /** Calculates the required marker offset, including both the symbol offset
     * and any displacement required to align with the marker's anchor point.
     * @param context symbol render context
     * @param width marker width
     * @param height marker height
     * @param offsetX will be set to required horizontal offset (in painter units)
     * @param offsetY will be set to required vertical offset (in painter units)
     * @note available in python as markerOffsetWithWidthAndHeight
     */
    void markerOffset( QgsSymbolRenderContext& context, double width, double height, double& offsetX, double& offsetY ) const /PyName=markerOffsetWithWidthAndHeight/;

    //! @note available in python bindings as markerOffset2
    void markerOffset( QgsSymbolRenderContext& context, double width, double height,
                       QgsUnitTypes::RenderUnit widthUnit, QgsUnitTypes::RenderUnit heightUnit,
                       double& offsetX, double& offsetY,
                       const QgsMapUnitScale &widthMapUnitScale, const QgsMapUnitScale &heightMapUnitScale ) const /PyName=markerOffset2/;

   /** Adjusts a marker offset to account for rotation.
     * @param offset offset prior to rotation
     * @param angle rotation angle in degrees clockwise from north
     * @return adjusted offset
     */
    static QPointF _rotatedOffset( QPointF offset, double angle );
};

class QgsLineSymbolLayer : QgsSymbolLayer
{
%TypeHeaderCode
#include <qgssymbollayer.h>
%End

  public:
    virtual void renderPolyline( const QPolygonF& points, QgsSymbolRenderContext& context ) = 0;

    virtual void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolRenderContext& context );

    virtual void setWidth( double width );
    virtual double width() const;

    double offset() const;
    void setOffset( double offset );

    /** Sets the units for the line's width.
     * @param unit width units
     * @see widthUnit()
    */
    void setWidthUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the line's width.
     * @see setWidthUnit()
    */
    QgsUnitTypes::RenderUnit widthUnit() const;

    void setWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& widthMapUnitScale() const;

    /** Sets the units for the line's offset.
     * @param unit offset units
     * @see offsetUnit()
    */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the line's offset.
     * @see setOffsetUnit()
    */
    QgsUnitTypes::RenderUnit offsetUnit() const;

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale );
    const QgsMapUnitScale& offsetMapUnitScale() const;

    void setOutputUnit( QgsUnitTypes::RenderUnit unit );
    QgsUnitTypes::RenderUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    void drawPreviewIcon( QgsSymbolRenderContext& context, QSize size );

    virtual double dxfWidth( const QgsDxfExport& e, QgsSymbolRenderContext& context ) const;
    virtual double dxfOffset( const QgsDxfExport& e, QgsSymbolRenderContext& context ) const;

  protected:
    QgsLineSymbolLayer( bool locked = false );
};

class QgsFillSymbolLayer : QgsSymbolLayer
{
%TypeHeaderCode
#include <qgssymbollayer.h>
%End

  public:
    virtual void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolRenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolRenderContext& context, QSize size );

    void setAngle( double angle );
    double angle() const;

  protected:
    QgsFillSymbolLayer( bool locked = false );
    /** Default method to render polygon*/
    void _renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings, QgsSymbolRenderContext& context );
};
