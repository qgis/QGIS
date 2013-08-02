typedef QMap<QString, QString> QgsStringMap;

typedef QList<QgsSymbolV2*> QgsSymbolV2List;
typedef QMap<QString, QgsSymbolV2* > QgsSymbolV2Map;

typedef QList< QPair<QString, QPixmap> > QgsLegendSymbologyList;
typedef QList< QPair<QString, QgsSymbolV2*> > QgsLegendSymbolList;

class QgsSymbolV2LevelItem
{
%TypeHeaderCode
#include <qgsrendererv2.h>
%End
  public:
    QgsSymbolV2LevelItem( QgsSymbolV2* symbol, int layer );
    QgsSymbolV2* symbol();
    int layer();
};

// every level has list of items: symbol + symbol layer num
// typedef QList< QgsSymbolV2LevelItem > QgsSymbolV2Level;

// this is a list of levels
// typedef QList< QgsSymbolV2Level > QgsSymbolV2LevelOrder;


//////////////
// renderers

class QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsrendererv2.h>
%End

%ConvertToSubClassCode
  if (sipCpp->type() == "singleSymbol")
    sipClass = sipClass_QgsSingleSymbolRendererV2;
  else if (sipCpp->type() == "categorizedSymbol")
    sipClass = sipClass_QgsCategorizedSymbolRendererV2;
  else if (sipCpp->type() == "graduatedSymbol")
    sipClass = sipClass_QgsGraduatedSymbolRendererV2;
  else if (sipCpp->type() == "RuleRenderer")
    sipClass = sipClass_QgsRuleBasedRendererV2;
  else
    sipClass = 0;
%End

  public:
    // renderer takes ownership of its symbols!

    //! return a new renderer - used by default in vector layers
    static QgsFeatureRendererV2* defaultRenderer( QGis::GeometryType geomType ) /Factory/;

    QString type() const;

    // to be overridden
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature ) = 0;

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer ) = 0;

    virtual void stopRender( QgsRenderContext& context ) = 0;

    virtual QList<QString> usedAttributes() = 0;

    virtual ~QgsFeatureRendererV2();

    virtual QgsFeatureRendererV2* clone() = 0 /Factory/;

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    //! for debugging
    virtual QString dump() const;

    enum Capabilities
    {
      SymbolLevels = 1,     // rendering with symbol levels (i.e. implements symbols(), symbolForFeature())
      RotationField = 2,    // rotate symbols by attribute value
      MoreSymbolsPerFeature = 4  // may use more than one symbol to render a feature: symbolsForFeature() will return them
      Filter         = 8,   // features may be filtered, i.e. some features may not be rendered (categorized, rule based ...)
      ScaleDependent = 16   // depends on scale if feature will be rendered (rule based )
    };

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities();

    //! for symbol levels
    virtual QgsSymbolV2List symbols() = 0;

    bool usingSymbolLevels() const;
    void setUsingSymbolLevels( bool usingSymbolLevels );

    //! create a renderer from XML element
    static QgsFeatureRendererV2* load( QDomElement& symbologyElem ) /Factory/;

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! create the SLD UserStyle element following the SLD v1.1 specs
    //! @note added in 1.9
    virtual QDomElement writeSld( QDomDocument& doc, const QgsVectorLayer &layer ) const;

    /** create a new renderer according to the information contained in
     * the UserStyle element of a SLD style document
     * @param node the node in the SLD document whose the UserStyle element
     * is a child
     * @param geomType the geometry type of the features, used to convert
     * Symbolizer elements
     * @param errorMessage it will contain the error message if something
     * went wrong
     * @return the renderer
     * @note added in 1.9
     */
    static QgsFeatureRendererV2* loadSld( const QDomNode &node, QGis::GeometryType geomType, QString &errorMessage ) /Factory/;

    //! used from subclasses to create SLD Rule elements following SLD v1.1 specs
    //! @note added in 1.9
    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    //! @note: not available in python bindings
    // virtual QgsLegendSymbolList legendSymbolItems();

    //! set type and size of editing vertex markers for subsequent rendering
    void setVertexMarkerAppearance( int type, int size );

    //! return rotation field name (or empty string if not set or not supported by renderer)
    //! @note added in 1.9
    virtual QString rotationField() const;
    //! sets rotation field of renderer (if supported by the renderer)
    //! @note added in 1.9
    virtual void setRotationField( QString fieldName );

    //! return whether the renderer will render a feature or not.
    //! Must be called between startRender() and stopRender() calls.
    //! Default implementation uses symbolForFeature().
    //! @note added in 1.9
    virtual bool willRenderFeature( QgsFeature& feat );

    //! return list of symbols used for rendering the feature.
    //! For renderers that do not support MoreSymbolsPerFeature it is more efficient
    //! to use symbolForFeature()
    //! @note added in 1.9
    virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat );

  protected:
    QgsFeatureRendererV2( QString type );

    void renderFeatureWithSymbol( QgsFeature& feature,
                                  QgsSymbolV2* symbol,
                                  QgsRenderContext& context,
                                  int layer,
                                  bool selected,
                                  bool drawVertexMarker );

    //! render editing vertex marker at specified point
    void renderVertexMarker( QPointF& pt, QgsRenderContext& context );
    //! render editing vertex marker for a polyline
    void renderVertexMarkerPolyline( QPolygonF& pts, QgsRenderContext& context );
    //! render editing vertex marker for a polygon
    void renderVertexMarkerPolygon( QPolygonF& pts, QList<QPolygonF>* rings, QgsRenderContext& context );

    static const unsigned char* _getPoint( QPointF& pt, QgsRenderContext& context, const unsigned char* wkb );
    static const unsigned char* _getLineString( QPolygonF& pts, QgsRenderContext& context, const unsigned char* wkb );
    static const unsigned char* _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, const unsigned char* wkb );

    void setScaleMethodToSymbol( QgsSymbolV2* symbol, int scaleMethod );
};
