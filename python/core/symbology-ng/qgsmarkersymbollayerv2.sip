class QgsSimpleMarkerSymbolLayerV2 : QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

  public:
    QgsSimpleMarkerSymbolLayerV2( QString name = DEFAULT_SIMPLEMARKER_NAME,
                                  QColor color = DEFAULT_SIMPLEMARKER_COLOR,
                                  QColor borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
                                  double size = DEFAULT_SIMPLEMARKER_SIZE,
                                  double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                  QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    QString name() const;
    void setName( QString name );

    QColor borderColor() const;
    void setBorderColor( QColor color );

    Qt::PenStyle outlineStyle() const;
    void setOutlineStyle( Qt::PenStyle outlineStyle );

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

    double outlineWidth() const;
    void setOutlineWidth( double w );

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit u );
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& outlineWidthMapUnitScale() const;

    bool writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, const QgsSymbolV2RenderContext* context, const QgsFeature* f, const QPointF& shift = QPointF( 0.0, 0.0 ) ) const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

  protected:
    void drawMarker( QPainter* p, QgsSymbolV2RenderContext& context );

    bool prepareShape( QString name = QString() );
    bool preparePath( QString name = QString() );

    /**Prepares cache image
    @return true in case of success, false if cache image size too large*/
    bool prepareCache( QgsSymbolV2RenderContext& context );
};

class QgsSvgMarkerSymbolLayerV2 : QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
%End

  public:
    QgsSvgMarkerSymbolLayerV2( QString name = DEFAULT_SVGMARKER_NAME,
                               double size = DEFAULT_SVGMARKER_SIZE,
                               double angle = DEFAULT_SVGMARKER_ANGLE,
                               QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QString path() const;
    void setPath( QString path );

    QColor fillColor() const;
    void setFillColor( const QColor& c );

    QColor outlineColor() const;
    void setOutlineColor( const QColor& c );

    double outlineWidth() const;
    void setOutlineWidth( double w );

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& outlineWidthMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    bool writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, const QgsSymbolV2RenderContext* context, const QgsFeature* f, const QPointF& shift = QPointF( 0.0, 0.0 ) ) const;
};

class QgsFontMarkerSymbolLayerV2 : QgsMarkerSymbolLayerV2
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

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    // new methods

    QString fontFamily() const;
    void setFontFamily( QString family );

    QChar character() const;
    void setCharacter( QChar ch );
};
