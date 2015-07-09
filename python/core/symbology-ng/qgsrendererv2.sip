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
  else if (sipCpp->type() == "heatmapRenderer")
    sipClass = sipClass_QgsHeatmapRenderer;
  else if (sipCpp->type() == "invertedPolygonRenderer")
    sipClass = sipClass_QgsInvertedPolygonRenderer;
  else if (sipCpp->type() == "pointDisplacement")
    sipClass = sipClass_QgsPointDisplacementRenderer;
  else
    sipClass = 0;
%End

  public:
    // renderer takes ownership of its symbols!

    //! return a new renderer - used by default in vector layers
    static QgsFeatureRendererV2* defaultRenderer( QGis::GeometryType geomType ) /Factory/;

    QString type() const;

    /** to be overridden
     * @param feature feature
     * @return returns pointer to symbol or 0 if symbol was not found
     */
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature ) = 0;

    /**
     * Return symbol for feature. The difference compared to symbolForFeature() is that it returns original
     * symbol which can be used as an identifier for renderer's rule - the former may return a temporary replacement
     * of a symbol for use in rendering.
     * @note added in 2.6
     */
    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) = 0;

    //! @deprecated since 2.4 - not using QgsVectorLayer directly anymore
    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer ) /Deprecated/;

    virtual void stopRender( QgsRenderContext& context ) = 0;

    virtual QList<QString> usedAttributes() = 0;

    virtual ~QgsFeatureRendererV2();

    virtual QgsFeatureRendererV2* clone() const = 0 /Factory/;

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    //! for debugging
    virtual QString dump() const;

    enum Capabilities
    {
      SymbolLevels,           // rendering with symbol levels (i.e. implements symbols(), symbolForFeature())
      RotationField,          // rotate symbols by attribute value
      MoreSymbolsPerFeature,  // may use more than one symbol to render a feature: symbolsForFeature() will return them
      Filter,                 // features may be filtered, i.e. some features may not be rendered (categorized, rule based ...)
      ScaleDependent          // depends on scale if feature will be rendered (rule based )
    };

    //! returns bitwise OR-ed capabilities of the renderer
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
    //! @deprecated since 2.8 - use the other override with styleName
    virtual QDomElement writeSld( QDomDocument& doc, const QgsVectorLayer &layer ) const /Deprecated/;
    //! create the SLD UserStyle element following the SLD v1.1 specs with the given name
    //! @note added in 2.8
    virtual QDomElement writeSld( QDomDocument& doc, const QString& styleName ) const;

    /** create a new renderer according to the information contained in
     * the UserStyle element of a SLD style document
     * @param node the node in the SLD document whose the UserStyle element
     * is a child
     * @param geomType the geometry type of the features, used to convert
     * Symbolizer elements
     * @param errorMessage it will contain the error message if something
     * went wrong
     * @return the renderer
     */
    static QgsFeatureRendererV2* loadSld( const QDomNode &node, QGis::GeometryType geomType, QString &errorMessage ) /Factory/;

    //! used from subclasses to create SLD Rule elements following SLD v1.1 specs
    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! items of symbology items in legend is checked
    //! @note added in 2.5
    virtual bool legendSymbolItemChecked( QString key );

    //! item in symbology was checked
    //! @note added in 2.5
    virtual void checkLegendSymbolItem( QString key, bool state = true );

    //! return a list of item text / symbol
    //! @note not available in python bindings
    // virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = "" );

    //! Return a list of symbology items for the legend. Better choice than legendSymbolItems().
    //! Default fallback implementation just uses legendSymbolItems() implementation
    //! @note added in 2.6
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const;

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute() const;

    //! set type and size of editing vertex markers for subsequent rendering
    void setVertexMarkerAppearance( int type, int size );

    //! return rotation field name (or empty string if not set or not supported by renderer)
    virtual QString rotationField() const;
    //! sets rotation field of renderer (if supported by the renderer)
    virtual void setRotationField( QString fieldName );

    //! return whether the renderer will render a feature or not.
    //! Must be called between startRender() and stopRender() calls.
    //! Default implementation uses symbolForFeature().
    virtual bool willRenderFeature( QgsFeature& feat );

    //! return list of symbols used for rendering the feature.
    //! For renderers that do not support MoreSymbolsPerFeature it is more efficient
    //! to use symbolForFeature()
    virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat );

    //! Equivalent of originalSymbolsForFeature() call
    //! extended to support renderers that may use more symbols per feature - similar to symbolsForFeature()
    //! @note added in 2.6
    virtual QgsSymbolV2List originalSymbolsForFeature( QgsFeature& feat );

    /**Allows for a renderer to modify the extent of a feature request prior to rendering
     * @param extent reference to request's filter extent. Modify extent to change the
     * extent of feature request
     * @param context render context
     * @note added in QGIS 2.7
     */
    virtual void modifyRequestExtent( QgsRectangle& extent, QgsRenderContext& context );

    /** Returns the current paint effect for the renderer.
     * @returns paint effect
     * @note added in QGIS 2.9
     * @see setPaintEffect
     */
    QgsPaintEffect* paintEffect() const;

    /** Sets the current paint effect for the renderer.
     * @param effect paint effect. Ownership is transferred to the renderer.
     * @note added in QGIS 2.9
     * @see paintEffect
     */
    void setPaintEffect( QgsPaintEffect* effect /Transfer/);

    /** Returns whether the renderer must render as a raster.
     * @note added in QGIS 2.12
     * @see setForceRasterRender
     */
    bool forceRasterRender() const;

    /** Sets whether the renderer should be rendered to a raster destination.
     * @param forceRaster set to true if renderer must be drawn on a raster surface.
     * This may be desirable for highly detailed layers where rendering as a vector
     * would result in a large, complex vector output.
     * @see forceRasterRender
     * @note added in QGIS 2.12
     */
    void setForceRasterRender( bool forceRaster );

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
    static const unsigned char* _getLineString( QPolygonF& pts, QgsRenderContext& context, const unsigned char* wkb, bool clipToExtent = true );
    static const unsigned char* _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, const unsigned char* wkb, bool clipToExtent = true );

    void setScaleMethodToSymbol( QgsSymbolV2* symbol, int scaleMethod );

    /** Copies paint effect of this renderer to another renderer
     * @param destRenderer destination renderer for copied effect
     */
    void copyPaintEffect( QgsFeatureRendererV2 *destRenderer ) const;

  private:
    QgsFeatureRendererV2( const QgsFeatureRendererV2 & );
    QgsFeatureRendererV2 & operator=( const QgsFeatureRendererV2 & );

};
