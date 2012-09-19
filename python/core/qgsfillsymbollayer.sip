class QgsFillSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsSimpleFillSymbolLayerV2*>(sipCpp) != NULL)
    sipClass = sipClass_QgsSimpleFillSymbolLayerV2;
  else if (dynamic_cast<QgsImageFillSymbolLayer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsImageFillSymbolLayer;
  else if (dynamic_cast<QgsCentroidFillSymbolLayerV2*>(sipCpp) != NULL)
    sipClass = sipClass_QgsCentroidFillSymbolLayerV2;  
  else
    sipClass = 0;
%End

public:
  virtual void renderPolygon(const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context) = 0;

  void drawPreviewIcon(QgsSymbolV2RenderContext& context, QSize size);

  void setAngle( double angle );
  double angle() const;

protected:
  QgsFillSymbolLayerV2(bool locked = false);
};

///////////////

class QgsCentroidFillSymbolLayerV2:QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

public:
  QgsCentroidFillSymbolLayerV2();
  ~QgsCentroidFillSymbolLayerV2();

  //! static stuff
  static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
  static QgsSymbolLayerV2* createFromSld( QDomElement &element );

  //! implemented from base classes
  QString layerType() const;
  void startRender( QgsSymbolV2RenderContext& context );
  void stopRender( QgsSymbolV2RenderContext& context );
  void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );
  QgsStringMap properties() const;
  QgsSymbolLayerV2* clone() const;
  void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;
  void setColor( const QColor& color );
  QgsSymbolV2* subSymbol();
  bool setSubSymbol( QgsSymbolV2* symbol );

};

///////////////
class QgsSimpleFillSymbolLayerV2:QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

public:
		QgsSimpleFillSymbolLayerV2(QColor color = DEFAULT_SIMPLEFILL_COLOR,
                                Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE,
                                QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                                Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE,
                                double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH);
		QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    QgsSymbolLayerV2* clone() const;
    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;
    Qt::BrushStyle brushStyle() const;
    void setBrushStyle( Qt::BrushStyle style );
    QColor borderColor() const;
    void setBorderColor( QColor borderColor );
    Qt::PenStyle borderStyle() const ;
    void setBorderStyle( Qt::PenStyle borderStyle );
    double borderWidth() const;
    void setBorderWidth( double borderWidth );
    void setOffset( QPointF offset ) ;
    QPointF offset() ;
};


class QgsImageFillSymbolLayer:QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsLinePatternFillSymbolLayer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsLinePatternFillSymbolLayer;
  else if (dynamic_cast<QgsPointPatternFillSymbolLayer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsPointPatternFillSymbolLayer;
  else if (dynamic_cast<QgsSVGFillSymbolLayer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsSVGFillSymbolLayer;  
  else
    sipClass = 0;
%End

public:
 	QgsImageFillSymbolLayer ();
	virtual 	~QgsImageFillSymbolLayer ();
	
	void renderPolygon (const QPolygonF &points, QList< QPolygonF > *rings, QgsSymbolV2RenderContext &context);
	virtual QgsSymbolV2 * subSymbol ();
	virtual bool setSubSymbol (QgsSymbolV2 *symbol);
	 	
};


///////////////

class QgsSVGFillSymbolLayer:QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

public:
  QgsSVGFillSymbolLayer( const QString& svgFilePath = "", double width = 20, double rotation = 0.0 );
  QgsSVGFillSymbolLayer( const QByteArray& svgData, double width = 20, double rotation = 0.0 );
  ~QgsSVGFillSymbolLayer();

  static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
  static QgsSymbolLayerV2* createFromSld( QDomElement &element );

  //! implemented from base classes
  QString layerType() const;
  void startRender( QgsSymbolV2RenderContext& context );
  void stopRender( QgsSymbolV2RenderContext& context );
  QgsStringMap properties() const;
  QgsSymbolLayerV2* clone() const;
  void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

  //! getters and setters
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
  
};
    
///////////////

class QgsPointPatternFillSymbolLayer:QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

	public:
    QgsPointPatternFillSymbolLayer();
    ~QgsPointPatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );
    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    QgsSymbolLayerV2* clone() const;
    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;
    //! getters and setters
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
};

///////////////

class QgsLinePatternFillSymbolLayer:QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );
    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    QgsSymbolLayerV2* clone() const;
    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;
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
};
