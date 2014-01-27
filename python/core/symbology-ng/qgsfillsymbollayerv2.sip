class QgsSimpleFillSymbolLayerV2 : QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
    QgsSimpleFillSymbolLayerV2( QColor color = DEFAULT_SIMPLEFILL_COLOR,
                                Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE,
                                QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                                Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE,
                                double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    Qt::BrushStyle brushStyle() const;
    void setBrushStyle( Qt::BrushStyle style );

    QColor borderColor() const;
    void setBorderColor( QColor borderColor );

    /** Get outline color.
     * @note added in 2.1 */
    QColor outlineColor() const;
    /** Set outline color.
     * @note added in 2.1 */
    void setOutlineColor( const QColor& color );

    /** Get fill color.
     * @note added in 2.1 */
    QColor fillColor() const;
    /** Set fill color.
     * @note added in 2.1 */
    void setFillColor( const QColor& color );

    Qt::PenStyle borderStyle() const;
    void setBorderStyle( Qt::PenStyle borderStyle );

    double borderWidth() const;
    void setBorderWidth( double borderWidth );

    void setOffset( QPointF offset );
    QPointF offset();

    void setBorderWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit borderWidthUnit() const;

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit offsetUnit() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    double estimateMaxBleed() const;

    double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;
    QColor dxfColor( const QgsSymbolV2RenderContext& context ) const;
    Qt::PenStyle dxfPenStyle() const;
};

class QgsGradientFillSymbolLayerV2 : QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
  
    enum GradientColorType
    {
      SimpleTwoColor,
      ColorRamp
    };

    enum GradientType
    {
      Linear,
      Radial,
      Conical
    };

    enum GradientCoordinateMode
    {
      Feature,
      Viewport
    };

    enum GradientSpread
    {
      Pad,
      Reflect,
      Repeat
    };

    QgsGradientFillSymbolLayerV2( QColor color = DEFAULT_SIMPLEFILL_COLOR,
                                  QColor color2 = Qt::white,
                                  GradientColorType gradientColorType = SimpleTwoColor,
                                  GradientType gradientType = Linear,
                                  GradientCoordinateMode coordinateMode = Feature,
                                  GradientSpread gradientSpread = Pad
                                );

    virtual ~QgsGradientFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    double estimateMaxBleed() const;

    /**Type of gradient, eg linear or radial*/
    GradientType gradientType() const;
    void setGradientType( GradientType gradientType );

    /**Gradient color mode, controls how gradient color stops are created*/
    GradientColorType gradientColorType() const;
    void setGradientColorType( GradientColorType gradientColorType );

    /**Color ramp used for the gradient fill, only used if the gradient color type is set to ColorRamp*/
    QgsVectorColorRampV2* colorRamp();
    void setColorRamp( QgsVectorColorRampV2* ramp );

    /**Color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor*/
    QColor color2() const;
    void setColor2( QColor color2 );

    /**Coordinate mode for gradient. Controls how the gradient stops are positioned.*/
    GradientCoordinateMode coordinateMode() const;
    void setCoordinateMode( GradientCoordinateMode coordinateMode );

    /**Gradient spread mode. Controls how the gradient behaves outside of the predefined stops*/
    GradientSpread gradientSpread() const;
    void setGradientSpread( GradientSpread gradientSpread );

    /**Starting point of gradient fill, in the range [0,0] - [1,1]*/
    void setReferencePoint1( QPointF referencePoint );
    QPointF referencePoint1() const;

    /**Sets the starting point of the gradient to be the feature centroid*/
    void setReferencePoint1IsCentroid( bool isCentroid );
    bool referencePoint1IsCentroid() const;

    /**End point of gradient fill, in the range [0,0] - [1,1]*/
    void setReferencePoint2( QPointF referencePoint );
    QPointF referencePoint2() const;

    /**Sets the end point of the gradient to be the feature centroid*/
    void setReferencePoint2IsCentroid( bool isCentroid );
    bool referencePoint2IsCentroid() const;

    /**Rotation angle for gradient fill. Can be used to rotate a gradient around its centre point*/
    void setAngle( double angle );
    double angle() const;

    /**Offset for gradient fill*/
    void setOffset( QPointF offset );
    QPointF offset() const;

    /**Units for gradient fill offset*/
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit offsetUnit() const;

};

/**Base class for polygon renderers generating texture images*/
class QgsImageFillSymbolLayer: QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
    QgsImageFillSymbolLayer();
    virtual ~QgsImageFillSymbolLayer();
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual QgsSymbolV2* subSymbol();
    virtual bool setSubSymbol( QgsSymbolV2* symbol );

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;

    virtual double estimateMaxBleed() const;

    virtual double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;
    virtual QColor dxfColor( const QgsSymbolV2RenderContext& context ) const;
    virtual Qt::PenStyle dxfPenStyle() const;
};

/**A class for svg fill patterns. The class automatically scales the pattern to
   the appropriate pixel dimensions of the output device*/
class QgsSVGFillSymbolLayer: QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
    QgsSVGFillSymbolLayer( const QString& svgFilePath = "", double width = 20, double rotation = 0.0 );
    QgsSVGFillSymbolLayer( const QByteArray& svgData, double width = 20, double rotation = 0.0 );
    ~QgsSVGFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    //getters and setters
    void setSvgFilePath( const QString& svgPath );
    QString svgFilePath() const;
    void setPatternWidth( double width );
    double patternWidth() const;

    void setSvgFillColor( const QColor& c );
    QColor svgFillColor() const;
    void setSvgOutlineColor( const QColor& c );
    QColor svgOutlineColor() const;
    void setSvgOutlineWidth( double w );
    double svgOutlineWidth() const;

    void setPatternWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit patternWidthUnit() const;

    void setSvgOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit svgOutlineWidthUnit() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

  protected:
    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );
};

class QgsLinePatternFillSymbolLayer: QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End
  public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    double estimateMaxBleed() const;

    QString ogrFeatureStyleWidth( double widthScaleFactor ) const;

    //getters and setters
    void setLineAngle( double a );
    double lineAngle() const;
    void setDistance( double d );
    double distance() const;
    void setLineWidth( double w );
    double lineWidth() const;
    void setColor( const QColor& c );
    QColor color() const;
    void setOffset( double offset );
    double offset() const;

    void setDistanceUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit distanceUnit() const;

    void setLineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit lineWidthUnit() const;

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit offsetUnit() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    bool setSubSymbol( QgsSymbolV2* symbol );
    QgsSymbolV2* subSymbol();

};

class QgsPointPatternFillSymbolLayer : QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End
  public:
    QgsPointPatternFillSymbolLayer();
    ~QgsPointPatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    double estimateMaxBleed() const;

    //getters and setters
    double distanceX() const;
    void setDistanceX( double d );

    double distanceY() const;
    void setDistanceY( double d );

    double displacementX() const;
    void setDisplacementX( double d );

    double displacementY() const;
    void setDisplacementY( double d );

    bool setSubSymbol( QgsSymbolV2* symbol );
    virtual QgsSymbolV2* subSymbol();

    void setDistanceXUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit distanceXUnit() const;

    void setDistanceYUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit distanceYUnit() const;

    void setDisplacementXUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit displacementXUnit() const;

    void setDisplacementYUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit displacementYUnit() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    virtual QSet<QString> usedAttributes() const;

  protected:
    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );
};

class QgsCentroidFillSymbolLayerV2 : QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
    QgsCentroidFillSymbolLayerV2();
    ~QgsCentroidFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    void setColor( const QColor& color );

    QgsSymbolV2* subSymbol();
    bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    virtual QSet<QString> usedAttributes() const;
};
