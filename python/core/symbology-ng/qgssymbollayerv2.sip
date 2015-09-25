class QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
#include <qgslinesymbollayerv2.h>
%End

%ConvertToSubClassCode
  switch (sipCpp->type())
  {
    case QgsSymbolV2::Marker:
      if (dynamic_cast<QgsEllipseSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsEllipseSymbolLayerV2;
      else if (dynamic_cast<QgsFontMarkerSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsFontMarkerSymbolLayerV2;
      else if (dynamic_cast<QgsSimpleMarkerSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsSimpleMarkerSymbolLayerV2;
      else if (dynamic_cast<QgsSvgMarkerSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsSvgMarkerSymbolLayerV2;
      else if (dynamic_cast<QgsVectorFieldSymbolLayer*>(sipCpp) != NULL)
	sipClass = sipClass_QgsVectorFieldSymbolLayer;
      else
	sipClass = sipClass_QgsMarkerSymbolLayerV2;
      break;

    case QgsSymbolV2::Line:
      if (dynamic_cast<QgsMarkerLineSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsMarkerLineSymbolLayerV2;
      else if (dynamic_cast<QgsSimpleLineSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsSimpleLineSymbolLayerV2;
      else
	sipClass = sipClass_QgsLineSymbolLayerV2;
      break;

    case QgsSymbolV2::Fill:
      if (dynamic_cast<QgsSimpleFillSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsSimpleFillSymbolLayerV2;
      else if (dynamic_cast<QgsImageFillSymbolLayer*>(sipCpp) != NULL)
      {
	if (dynamic_cast<QgsLinePatternFillSymbolLayer*>(sipCpp) != NULL)
	  sipClass = sipClass_QgsLinePatternFillSymbolLayer;
	else if (dynamic_cast<QgsPointPatternFillSymbolLayer*>(sipCpp) != NULL)
	  sipClass = sipClass_QgsPointPatternFillSymbolLayer;
	else if (dynamic_cast<QgsSVGFillSymbolLayer*>(sipCpp) != NULL)
	  sipClass = sipClass_QgsSVGFillSymbolLayer;
	else if (dynamic_cast<QgsRasterFillSymbolLayer*>(sipCpp) != NULL)
	  sipClass = sipClass_QgsRasterFillSymbolLayer;
	else
	  sipClass = sipClass_QgsImageFillSymbolLayer;
      }
      else if (dynamic_cast<QgsCentroidFillSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsCentroidFillSymbolLayerV2;
      else if (dynamic_cast<QgsGradientFillSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsGradientFillSymbolLayerV2;
      else if (dynamic_cast<QgsShapeburstFillSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsShapeburstFillSymbolLayerV2;
      else
	sipClass = sipClass_QgsFillSymbolLayerV2;
      break;

    default:
      sipClass = 0;
      break;
  }
%End

  public:

    virtual ~QgsSymbolLayerV2();

    // not necessarily supported by all symbol layers...
    virtual QColor color() const;
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

    virtual QString layerType() const = 0;

    virtual void startRender( QgsSymbolV2RenderContext& context ) = 0;
    virtual void stopRender( QgsSymbolV2RenderContext& context ) = 0;

    virtual QgsSymbolLayerV2* clone() const = 0 /Factory/;

    virtual void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    virtual QgsStringMap properties() const = 0;

    virtual void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) = 0;

    virtual QgsSymbolV2* subSymbol();
    // set layer's subsymbol. takes ownership of the passed symbol
    virtual bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );

    QgsSymbolV2::SymbolType type() const;

    void setLocked( bool locked );
    bool isLocked() const;

    /** Returns the estimated maximum distance which the layer style will bleed outside
      the drawn shape. Eg, polygons drawn with an outline will draw half the width
      of the outline outside of the polygon. This amount is estimated, since it may
      be affected by data defined symbology rules.*/
    virtual double estimateMaxBleed() const;

    virtual void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    virtual QgsSymbolV2::OutputUnit outputUnit() const;

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
     * @param property unique property key. Any existing data defined with the same
     * key will be deleted and overriden.
     * @param dataDefined data defined object to associate with property key. Ownership
     * is transferred to the layer.
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
    virtual QVariant evaluateDataDefinedProperty( const QString& property, const QgsSymbolV2RenderContext& context, const QVariant& defaultVal = QVariant(), bool *ok = 0 ) const;

    virtual bool writeDxf( QgsDxfExport& e,
                           double mmMapUnitScaleFactor,
                           const QString& layerName,
                           const QgsSymbolV2RenderContext* context,
                           const QgsFeature* f,
                           const QPointF& shift = QPointF( 0.0, 0.0 ) ) const;

    virtual double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;
    virtual double dxfOffset( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;

    virtual QColor dxfColor( const QgsSymbolV2RenderContext& context ) const;

    virtual QVector<qreal> dxfCustomDashPattern( QgsSymbolV2::OutputUnit& unit ) const;
    virtual Qt::PenStyle dxfPenStyle() const;
    virtual QColor dxfBrushColor( const QgsSymbolV2RenderContext& context ) const;
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
    QgsSymbolLayerV2( QgsSymbolV2::SymbolType type, bool locked = false );

   /** Prepares all data defined property expressions for evaluation. This should
     * be called prior to evaluating data defined properties.
     * @param fields associated layer fields
     * @param scale map scale
     */
    virtual void prepareExpressions( const QgsFields* fields, double scale = -1.0 ) /Deprecated/;

    /** Prepares all data defined property expressions for evaluation. This should
     * be called prior to evaluating data defined properties.
     * @param context symbol render context
     * @note added in QGIS 2.12
     */
    virtual void prepareExpressions( const QgsSymbolV2RenderContext& context );

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
    void copyDataDefinedProperties( QgsSymbolLayerV2* destLayer ) const;

    /** Copies paint effect of this layer to another symbol layer
     * @param destLayer destination layer
     * @note added in QGIS 2.9
     */
    void copyPaintEffect( QgsSymbolLayerV2* destLayer ) const;

};

//////////////////////

class QgsMarkerSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End
  public:

    enum HorizontalAnchorPoint
    {
      Left,
      HCenter,
      Right
    };

    enum VerticalAnchorPoint
    {
      Top,
      VCenter,
      Bottom
    };

    void startRender( QgsSymbolV2RenderContext& context );

    virtual void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    void setAngle( double angle );
    double angle() const;

    /** Sets the line angle modification for the symbol's angle. This angle is added to
     * the marker's rotation and data defined rotation before rendering the symbol, and
     * is usually used for orienting symbols to match a line's angle.
     * @param lineangle Angle in degrees, valid values are between 0 and 360
     * @note added in QGIS 2.9
    */
    void setLineAngle( double lineAngle );

    void setSize( double size );
    double size() const;

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    QgsSymbolV2::ScaleMethod scaleMethod() const;

    void setOffset( QPointF offset );
    QPointF offset();

    virtual void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit offsetUnit() const;

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& offsetMapUnitScale() const;

    void setSizeUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit sizeUnit() const;

    void setSizeMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& sizeMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    void setHorizontalAnchorPoint( HorizontalAnchorPoint h );
    HorizontalAnchorPoint horizontalAnchorPoint() const;

    void setVerticalAnchorPoint( VerticalAnchorPoint v );
    VerticalAnchorPoint verticalAnchorPoint() const;

  protected:
    QgsMarkerSymbolLayerV2( bool locked = false );

    //handles marker offset and anchor point shift together
    void markerOffset( const QgsSymbolV2RenderContext& context, double& offsetX, double& offsetY ) const;

    void markerOffset( const QgsSymbolV2RenderContext& context, double width, double height, double& offsetX, double& offsetY ) const /PyName=markerOffsetWithWidthAndHeight/;

    //! @note available in python bindings as markerOffset2
    void markerOffset( const QgsSymbolV2RenderContext& context, double width, double height,
                       QgsSymbolV2::OutputUnit widthUnit, QgsSymbolV2::OutputUnit heightUnit,
                       double& offsetX, double& offsetY,
                       const QgsMapUnitScale &widthMapUnitScale, const QgsMapUnitScale &heightMapUnitScale ) const /PyName=markerOffset2/;

    static QPointF _rotatedOffset( const QPointF& offset, double angle );
};

class QgsLineSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

  public:
    virtual void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context ) = 0;

    virtual void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual void setWidth( double width );
    virtual double width() const;

    double offset() const;
    void setOffset( double offset );

    void setWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit widthUnit() const;

    void setWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& widthMapUnitScale() const;

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit offsetUnit() const;

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale );
    const QgsMapUnitScale& offsetMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    virtual double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;
    virtual double dxfOffset( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;

  protected:
    QgsLineSymbolLayerV2( bool locked = false );
};

class QgsFillSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

  public:
    virtual void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    void setAngle( double angle );
    double angle() const;

  protected:
    QgsFillSymbolLayerV2( bool locked = false );
    /** Default method to render polygon*/
    void _renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );
};
