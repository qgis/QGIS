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
	else
	  sipClass = sipClass_QgsImageFillSymbolLayer;
      }
      else if (dynamic_cast<QgsCentroidFillSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsCentroidFillSymbolLayerV2;
      else
	sipClass = sipClass_QgsFillSymbolLayerV2;
      break;

    default:
      sipClass = 0;
      break;
  }
%End

  public:

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

    virtual ~QgsSymbolLayerV2();

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

    /**Returns the estimated maximum distance which the layer style will bleed outside
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

    // symbol layers normally only use additional attributes to provide data defined settings
    virtual QSet<QString> usedAttributes() const;

    virtual const QgsExpression* dataDefinedProperty( const QString& property ) const;
    virtual QString dataDefinedPropertyString( const QString& property ) const;
    virtual void setDataDefinedProperty( const QString& property, const QString& expressionString );
    virtual void removeDataDefinedProperty( const QString& property );
    virtual void removeDataDefinedProperties();
    bool hasDataDefinedProperties() const;

    virtual bool writeDxf( QgsDxfExport& e,
                           double mmMapUnitScaleFactor,
                           const QString& layerName,
                           const QgsSymbolV2RenderContext* context,
                           const QgsFeature* f,
                           const QPointF& shift = QPointF( 0.0, 0.0 ) ) const;

    virtual double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;

    virtual QColor dxfColor( const QgsSymbolV2RenderContext& context ) const;

    virtual QVector<qreal> dxfCustomDashPattern( QgsSymbolV2::OutputUnit& unit ) const;
    virtual Qt::PenStyle dxfPenStyle() const;
    virtual QColor dxfBrushColor( const QgsSymbolV2RenderContext& context ) const;
    virtual Qt::BrushStyle dxfBrushStyle() const;

  protected:
    QgsSymbolLayerV2( QgsSymbolV2::SymbolType type, bool locked = false );

    virtual void prepareExpressions( const QgsFields* fields, double scale = -1.0 );
    virtual QgsExpression* expression( const QString& property ) const;
    /**Saves data defined properties to string map*/
    void saveDataDefinedProperties( QgsStringMap& stringMap ) const;
    /**Copies data defined properties of this layer to another symbol layer*/
    void copyDataDefinedProperties( QgsSymbolLayerV2* destLayer ) const;
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

    void setWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit widthUnit() const;

    void setWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& widthMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    virtual double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;

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
    /**Default method to render polygon*/
    void _renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );
};
