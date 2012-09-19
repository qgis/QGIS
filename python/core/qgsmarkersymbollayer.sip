class QgsMarkerSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End


%ConvertToSubClassCode
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
    sipClass = 0;
%End


public:
  virtual void renderPoint(const QPointF& point, QgsSymbolV2RenderContext& context) = 0;

  void drawPreviewIcon(QgsSymbolV2RenderContext& context, QSize size);

  void setAngle(double angle);
  double angle() const;

  void setSize(double size);
  double size() const;

protected:
  QgsMarkerSymbolLayerV2(bool locked = false);

};

///////////////

class QgsEllipseSymbolLayerV2:QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsellipsesymbollayerv2.h>
%End

  public:
    QgsEllipseSymbolLayerV2();
    ~QgsEllipseSymbolLayerV2();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsSymbolLayerV2* clone() const;
    QgsStringMap properties() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;
    void writeSldMarker( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

    void setSymbolName( const QString& name );
    QString symbolName() const;

    void setSymbolNameField( const QString& field );
    const QString& symbolNameField() const;

    void setSymbolWidth( double w );
    double symbolWidth() const;

    void setWidthField( const QString& field );
    const QString& widthField() const;

    void setSymbolHeight( double h );
    double symbolHeight() const;

    void setHeightField( const QString& field );
    const QString& heightField() const;

    void setRotationField( const QString& field );
    const QString& rotationField() const;

    void setOutlineWidth( double w );
    double outlineWidth() const;

    void setOutlineWidthField( const QString& field );
    const QString& outlineWidthField() const;

    void setFillColor( const QColor& c );
    QColor fillColor() const;

    void setFillColorField( const QString& field );
    const QString& fillColorField() const;

    void setOutlineColor( const QColor& c );
    QColor outlineColor() const;

    void setOutlineColorField( const QString& field );
    const QString& outlineColorField() const;

    QSet<QString> usedAttributes() const;

};

///////////////


class QgsFontMarkerSymbolLayerV2:QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
%End

public:
    QgsFontMarkerSymbolLayerV2( QString fontFamily = DEFAULT_FONTMARKER_FONT,
                                QChar chr = DEFAULT_FONTMARKER_CHR,
                                double pointSize = DEFAULT_FONTMARKER_SIZE,
                                QColor color = DEFAULT_FONTMARKER_COLOR,
                                double angle = DEFAULT_FONTMARKER_ANGLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    QgsSymbolLayerV2* clone() const;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;
    // new methods
    QString fontFamily() const;
    void setFontFamily( QString family );
    QChar character() const;
    void setCharacter( QChar ch );

};

///////////////


class QgsSimpleMarkerSymbolLayerV2:QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
%End

  public:
    QgsSimpleMarkerSymbolLayerV2( QString name = DEFAULT_SIMPLEMARKER_NAME,
                                  QColor color = DEFAULT_SIMPLEMARKER_COLOR,
                                  QColor borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
                                  double size = DEFAULT_SIMPLEMARKER_SIZE,
                                  double angle = DEFAULT_SIMPLEMARKER_ANGLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    QgsSymbolLayerV2* clone() const;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;
    QString name() const;
    void setName( QString name );
    QColor borderColor() const;
    void setBorderColor( QColor color );

  protected:

    void drawMarker( QPainter* p, QgsSymbolV2RenderContext& context );
    bool prepareShape();
    bool preparePath();
    void prepareCache( QgsSymbolV2RenderContext& context );

};

///////////////


class QgsSvgMarkerSymbolLayerV2:QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
%End

  public:
    QgsSvgMarkerSymbolLayerV2( QString name = DEFAULT_SVGMARKER_NAME,
                               double size = DEFAULT_SVGMARKER_SIZE,
                               double angle = DEFAULT_SVGMARKER_ANGLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    //! Return a list of all available svg files
    static QStringList listSvgFiles();

    //! Get symbol's path from its name
    static QString symbolNameToPath( QString name );

    //! Get symbols's name from its path
    static QString symbolPathToName( QString path );

    // implemented from base classes

    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    QgsSymbolLayerV2* clone() const;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;
    QString path() const;
    void setPath( QString path );

    QColor fillColor() const;
    void setFillColor( const QColor& c );
    QColor outlineColor() const;
    void setOutlineColor( const QColor& c );
    double outlineWidth() const;
    void setOutlineWidth( double w );

};

///////////////