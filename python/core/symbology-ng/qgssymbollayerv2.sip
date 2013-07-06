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
      if (dynamic_cast<QgsLineDecorationSymbolLayerV2*>(sipCpp) != NULL)
	sipClass = sipClass_QgsLineDecorationSymbolLayerV2;
      else if (dynamic_cast<QgsMarkerLineSymbolLayerV2*>(sipCpp) != NULL)
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
    virtual void setColor( const QColor& color );
    virtual QColor color() const;

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

    virtual void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    virtual QgsSymbolV2::OutputUnit outputUnit() const;

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

  protected:
    QgsSymbolLayerV2( QgsSymbolV2::SymbolType type, bool locked = false );

    virtual void prepareExpressions( const QgsVectorLayer* vl );
    virtual QgsExpression* expression( const QString& property );
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

    void setSizeUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit sizeUnit() const;

    virtual void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    virtual QgsSymbolV2::OutputUnit outputUnit() const;

  protected:
    QgsMarkerSymbolLayerV2( bool locked = false );
    void markerOffset( QgsSymbolV2RenderContext& context, double& offsetX, double& offsetY );
    static QPointF _rotatedOffset( const QPointF& offset, double angle );
};

class QgsLineSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

  public:
    virtual void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context ) = 0;

    //! @note added in v1.7
    virtual void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual void setWidth( double width );
    virtual double width() const;

    void setWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit widthUnit() const;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

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
    void _renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings );
};
