class QgsLineSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsLineDecorationSymbolLayerV2*>(sipCpp) != NULL)
    sipClass = sipClass_QgsLineDecorationSymbolLayerV2;
  else if (dynamic_cast<QgsMarkerLineSymbolLayerV2*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMarkerLineSymbolLayerV2;
  else if (dynamic_cast<QgsSimpleLineSymbolLayerV2*>(sipCpp) != NULL)
    sipClass = sipClass_QgsSimpleLineSymbolLayerV2;  
  else
    sipClass = 0;
%End


public:
  virtual void renderPolyline(const QPolygonF& points, QgsSymbolV2RenderContext& context) = 0;

  //! @note added in v1.7
  virtual void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

  void setWidth(double width);
  double width() const;
  	
  void drawPreviewIcon(QgsSymbolV2RenderContext& context, QSize size);

protected:
  QgsLineSymbolLayerV2(bool locked = false);

};

///////////////

class QgsLineDecorationSymbolLayerV2:QgsLineSymbolLayerV2
{
%TypeHeaderCode
#include <qgslinesymbollayerv2.h>
%End
  
public:
  QgsLineDecorationSymbolLayerV2( QColor color = DEFAULT_LINEDECORATION_COLOR,
                                  double width = DEFAULT_LINEDECORATION_WIDTH );

  ~QgsLineDecorationSymbolLayerV2();

  static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
  
  // missing implementation in .cpp
  //static QgsSymbolLayerV2* createFromSld( QDomElement &element );

  QString layerType() const;
  void startRender( QgsSymbolV2RenderContext& context );
  void stopRender( QgsSymbolV2RenderContext& context );
  void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );
  QgsStringMap properties() const;
  QgsSymbolLayerV2* clone() const;
  void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

};

///////////////

class QgsMarkerLineSymbolLayerV2:QgsLineSymbolLayerV2
{
%TypeHeaderCode
#include <qgslinesymbollayerv2.h>
%End

public:
  QgsMarkerLineSymbolLayerV2( bool rotateMarker = DEFAULT_MARKERLINE_ROTATE,
                              double interval = DEFAULT_MARKERLINE_INTERVAL );

  ~QgsMarkerLineSymbolLayerV2();

  enum Placement
  {
    Interval,
    Vertex,
    LastVertex,
    FirstVertex,
    CentralPoint
  };

  // static stuff

  static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
  static QgsSymbolLayerV2* createFromSld( QDomElement &element );

  // implemented from base classes

  QString layerType() const;
  void startRender( QgsSymbolV2RenderContext& context );
  void stopRender( QgsSymbolV2RenderContext& context );
  void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );
  QgsStringMap properties() const;
  QgsSymbolLayerV2* clone() const;
  void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;
  void setColor( const QColor& color );
  QgsSymbolV2* subSymbol();
  bool setSubSymbol( QgsSymbolV2* symbol );
  virtual void setWidth( double width );
  virtual double width() const;

  bool rotateMarker() const;
  void setRotateMarker( bool rotate );
  double interval() const;
  void setInterval( double interval );
  double offset() const;
  void setOffset( double offset );
  Placement placement() const;
  void setPlacement( Placement p );

protected:

  void renderPolylineInterval( const QPolygonF& points, QgsSymbolV2RenderContext& context );
  void renderPolylineVertex( const QPolygonF& points, QgsSymbolV2RenderContext& context );
  void renderPolylineCentral( const QPolygonF& points, QgsSymbolV2RenderContext& context );

};

///////////////

class QgsSimpleLineSymbolLayerV2:QgsLineSymbolLayerV2
{
%TypeHeaderCode
#include <qgslinesymbollayerv2.h>
%End

public:
	QgsSimpleLineSymbolLayerV2( QColor color = DEFAULT_SIMPLELINE_COLOR,
                              double width = DEFAULT_SIMPLELINE_WIDTH,
                              Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE );

  // static stuff

  static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
  static QgsSymbolLayerV2* createFromSld( QDomElement &element );

  // implemented from base classes

  QString layerType() const;
  void startRender( QgsSymbolV2RenderContext& context );
  void stopRender( QgsSymbolV2RenderContext& context );
  void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );
  QgsStringMap properties() const;
  QgsSymbolLayerV2* clone() const;
  void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

  Qt::PenStyle penStyle() const;
  void setPenStyle( Qt::PenStyle style );
  Qt::PenJoinStyle penJoinStyle() const;
  void setPenJoinStyle( Qt::PenJoinStyle style );
  Qt::PenCapStyle penCapStyle() const;
  void setPenCapStyle( Qt::PenCapStyle style );
  double offset() const;
  void setOffset( double offset );
  bool useCustomDashPattern() const;
  void setUseCustomDashPattern( bool b );
  QVector<qreal> customDashVector() const;
  void setCustomDashVector( const QVector<qreal>& vector );

};
