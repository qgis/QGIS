/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsrenderer.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






typedef QList<QgsSymbol *> QgsSymbolList;





class QgsSymbolLevelItem
{

%TypeHeaderCode
#include "qgsrenderer.h"
%End
  public:
    QgsSymbolLevelItem( QgsSymbol *symbol, int layer );
    QgsSymbol *symbol();
    int layer();
  protected:
};

typedef QList< QgsSymbolLevelItem > QgsSymbolLevel;

typedef QList< QList< QgsSymbolLevelItem > > QgsSymbolLevelOrder;



class QgsFeatureRenderer
{

%TypeHeaderCode
#include "qgsrenderer.h"
%End
%ConvertToSubClassCode
    if ( sipCpp->type() == "singleSymbol" )
      sipType = sipType_QgsSingleSymbolRenderer;
    else if ( sipCpp->type() == "categorizedSymbol" )
      sipType = sipType_QgsCategorizedSymbolRenderer;
    else if ( sipCpp->type() == "graduatedSymbol" )
      sipType = sipType_QgsGraduatedSymbolRenderer;
    else if ( sipCpp->type() == "RuleRenderer" )
      sipType = sipType_QgsRuleBasedRenderer;
    else if ( sipCpp->type() == "heatmapRenderer" )
      sipType = sipType_QgsHeatmapRenderer;
    else if ( sipCpp->type() == "invertedPolygonRenderer" )
      sipType = sipType_QgsInvertedPolygonRenderer;
    else if ( sipCpp->type() == "pointCluster" )
      sipType = sipType_QgsPointClusterRenderer;
    else if ( sipCpp->type() == "pointDisplacement" )
      sipType = sipType_QgsPointDisplacementRenderer;
    else if ( sipCpp->type() == "25dRenderer" )
      sipType = sipType_Qgs25DRenderer;
    else if ( sipCpp->type() == "nullSymbol" )
      sipType = sipType_QgsNullSymbolRenderer;
    else
      sipType = 0;
%End
  public:

    static QgsFeatureRenderer *defaultRenderer( QgsWkbTypes::GeometryType geomType ) /Factory/;
%Docstring
return a new renderer - used by default in vector layers
%End

    QString type() const;

    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context ) = 0;
%Docstring
To be overridden

Must be called between startRender() and stopRender() calls.

:param feature: feature
:param context: render context

:return: returns pointer to symbol or 0 if symbol was not found

.. versionadded:: 2.12
%End

    virtual QgsSymbol *originalSymbolForFeature( QgsFeature &feature, QgsRenderContext &context );
%Docstring
Return symbol for feature. The difference compared to symbolForFeature() is that it returns original
symbol which can be used as an identifier for renderer's rule - the former may return a temporary replacement
of a symbol for use in rendering.

.. versionadded:: 2.12
%End

    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context );
%Docstring
Return legend keys matching a specified feature.

.. versionadded:: 2.14
%End

    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );
%Docstring
Must be called when a new render cycle is started. A call to startRender() must always
be followed by a corresponding call to stopRender() after all features have been rendered.

:param context:  Additional information passed to the renderer about the job which will be rendered
:param fields:   The fields available for rendering

.. seealso:: :py:func:`stopRender`


.. warning::

   This method is not thread safe. Before calling startRender() in a non-main thread,
the renderer should instead be cloned and startRender()/stopRender() called on the clone.
%End

    virtual void stopRender( QgsRenderContext &context );
%Docstring
Must be called when a render cycle has finished, to allow the renderer to clean up.

Calls to stopRender() must always be preceded by a call to startRender().

.. warning::

   This method is not thread safe. Before calling startRender() in a non-main thread,
the renderer should instead be cloned and startRender()/stopRender() called on the clone.

.. seealso:: :py:func:`startRender`
%End

    virtual QString filter( const QgsFields &fields = QgsFields() );
%Docstring
If a renderer does not require all the features this method may be overridden
and return an expression used as where clause.
This will be called once after startRender() and before the first call
to renderFeature().
By default this returns a null string and all features will be requested.
You do not need to specify the extent in here, this is taken care of separately and
will be combined with a filter returned from this method.

:return: An expression used as where clause
%End

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const = 0;
%Docstring
Return a list of attributes required by this renderer. Attributes not listed in here may
not have been requested from the provider at rendering time.

:return: A set of attributes
%End

    virtual bool filterNeedsGeometry() const;
%Docstring
Returns true if this renderer requires the geometry to apply the filter.
%End

    virtual ~QgsFeatureRenderer();

    virtual QgsFeatureRenderer *clone() const = 0 /Factory/;
%Docstring
Create a deep copy of this renderer. Should be implemented by all subclasses
and generate a proper subclass.

:return: A copy of this renderer
%End

    virtual bool renderFeature( QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false );
%Docstring
Render a feature using this renderer in the given context.
Must be called between startRender() and stopRender() calls.
Default implementation renders a symbol as determined by symbolForFeature() call.
Returns true if the feature has been returned (this is used for example
to determine whether the feature may be labelled).

If layer is not -1, the renderer should draw only a particula layer from symbols
(in order to support symbol level rendering).

.. seealso:: :py:func:`startRender`

.. seealso:: :py:func:`stopRender`
%End

    virtual QString dump() const;
%Docstring
Returns debug information about this renderer
%End

    enum Capability
    {
      SymbolLevels,
      MoreSymbolsPerFeature,
      Filter,
      ScaleDependent
    };

    typedef QFlags<QgsFeatureRenderer::Capability> Capabilities;


    virtual QgsFeatureRenderer::Capabilities capabilities();
%Docstring
Returns details about internals of this renderer.

E.g. if you only want to deal with visible features:

.. code-block:: python

     if not renderer.capabilities().testFlag(QgsFeatureRenderer.Filter) or renderer.willRenderFeature(feature, context):
         deal_with_my_feature()
     else:
         skip_the_curren_feature()
%End

    virtual QgsSymbolList symbols( QgsRenderContext &context );
%Docstring
Returns list of symbols used by the renderer.

:param context: render context

.. versionadded:: 2.12
%End

    bool usingSymbolLevels() const;
    void setUsingSymbolLevels( bool usingSymbolLevels );

    static QgsFeatureRenderer *load( QDomElement &symbologyElem, const QgsReadWriteContext &context ) /Factory/;
%Docstring
create a renderer from XML element
%End

    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );
%Docstring
store renderer info to XML element
%End

    virtual QDomElement writeSld( QDomDocument &doc, const QString &styleName, const QgsStringMap &props = QgsStringMap() ) const;
%Docstring
create the SLD UserStyle element following the SLD v1.1 specs with the given name

.. versionadded:: 2.8
%End

    static QgsFeatureRenderer *loadSld( const QDomNode &node, QgsWkbTypes::GeometryType geomType, QString &errorMessage ) /Factory/;
%Docstring
Create a new renderer according to the information contained in
the UserStyle element of a SLD style document

:param node: the node in the SLD document whose the UserStyle element
is a child
:param geomType: the geometry type of the features, used to convert
Symbolizer elements
:param errorMessage: it will contain the error message if something
went wrong

:return: the renderer
%End

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const;
%Docstring
used from subclasses to create SLD Rule elements following SLD v1.1 specs
%End

    virtual bool legendSymbolItemsCheckable() const;
%Docstring
items of symbology items in legend should be checkable

.. versionadded:: 2.5
%End

    virtual bool legendSymbolItemChecked( const QString &key );
%Docstring
items of symbology items in legend is checked

.. versionadded:: 2.5
%End

    virtual void checkLegendSymbolItem( const QString &key, bool state = true );
%Docstring
item in symbology was checked

.. versionadded:: 2.5
%End

    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol /Transfer/ );
%Docstring
Sets the symbol to be used for a legend symbol item.

:param key: rule key for legend symbol
:param symbol: new symbol for legend item. Ownership is transferred to renderer.

.. versionadded:: 2.14
%End

    virtual QgsLegendSymbolList legendSymbolItems() const;
%Docstring
Returns a list of symbology items for the legend

.. versionadded:: 2.6
%End

    virtual QString legendClassificationAttribute() const;
%Docstring
If supported by the renderer, return classification attribute for the use in legend

.. versionadded:: 2.6
%End

    void setVertexMarkerAppearance( int type, int size );
%Docstring
set type and size of editing vertex markers for subsequent rendering
%End

    virtual bool willRenderFeature( QgsFeature &feat, QgsRenderContext &context );
%Docstring
Returns whether the renderer will render a feature or not.
Must be called between startRender() and stopRender() calls.
Default implementation uses symbolForFeature().

.. versionadded:: 2.12
%End

    virtual QgsSymbolList symbolsForFeature( QgsFeature &feat, QgsRenderContext &context );
%Docstring
Returns list of symbols used for rendering the feature.
For renderers that do not support MoreSymbolsPerFeature it is more efficient
to use symbolForFeature()

.. versionadded:: 2.12
%End

    virtual QgsSymbolList originalSymbolsForFeature( QgsFeature &feat, QgsRenderContext &context );
%Docstring
Equivalent of originalSymbolsForFeature() call
extended to support renderers that may use more symbols per feature - similar to symbolsForFeature()

.. versionadded:: 2.12
%End

    virtual void modifyRequestExtent( QgsRectangle &extent, QgsRenderContext &context );
%Docstring
Allows for a renderer to modify the extent of a feature request prior to rendering

:param extent: reference to request's filter extent. Modify extent to change the
extent of feature request
:param context: render context

.. versionadded:: 2.7
%End

    QgsPaintEffect *paintEffect() const;
%Docstring
Returns the current paint effect for the renderer.

:return: paint effect

.. versionadded:: 2.9

.. seealso:: :py:func:`setPaintEffect`
%End

    void setPaintEffect( QgsPaintEffect *effect );
%Docstring
Sets the current paint effect for the renderer.

:param effect: paint effect. Ownership is transferred to the renderer.

.. versionadded:: 2.9

.. seealso:: :py:func:`paintEffect`
%End

    bool forceRasterRender() const;
%Docstring
Returns whether the renderer must render as a raster.

.. versionadded:: 2.12

.. seealso:: :py:func:`setForceRasterRender`
%End

    void setForceRasterRender( bool forceRaster );
%Docstring
Sets whether the renderer should be rendered to a raster destination.

:param forceRaster: set to true if renderer must be drawn on a raster surface.
This may be desirable for highly detailed layers where rendering as a vector
would result in a large, complex vector output.

.. seealso:: :py:func:`forceRasterRender`

.. versionadded:: 2.12
%End

    QgsFeatureRequest::OrderBy orderBy() const;
%Docstring
Get the order in which features shall be processed by this renderer.

.. versionadded:: 2.14

.. note::

   this property has no effect if orderByEnabled() is false

.. seealso:: :py:func:`orderByEnabled`
%End

    void setOrderBy( const QgsFeatureRequest::OrderBy &orderBy );
%Docstring
Define the order in which features shall be processed by this renderer.

.. note::

   this property has no effect if orderByEnabled() is false

.. versionadded:: 2.14

.. seealso:: :py:func:`setOrderByEnabled`
%End

    bool orderByEnabled() const;
%Docstring
Returns whether custom ordering will be applied before features are processed by this renderer.

.. versionadded:: 2.14

.. seealso:: :py:func:`orderBy`

.. seealso:: :py:func:`setOrderByEnabled`
%End

    void setOrderByEnabled( bool enabled );
%Docstring
Sets whether custom ordering should be applied before features are processed by this renderer.

:param enabled: set to true to enable custom feature ordering

.. versionadded:: 2.14

.. seealso:: :py:func:`setOrderBy`

.. seealso:: :py:func:`orderByEnabled`
%End

    virtual void setEmbeddedRenderer( QgsFeatureRenderer *subRenderer /Transfer/ );
%Docstring
Sets an embedded renderer (subrenderer) for this feature renderer. The base class implementation
does nothing with subrenderers, but individual derived classes can use these to modify their behavior.

:param subRenderer: the embedded renderer. Ownership will be transferred.

.. seealso:: :py:func:`embeddedRenderer`

.. versionadded:: 2.16
%End

    virtual const QgsFeatureRenderer *embeddedRenderer() const;
%Docstring
Returns the current embedded renderer (subrenderer) for this feature renderer. The base class
implementation does not use subrenderers and will always return null.

.. seealso:: :py:func:`setEmbeddedRenderer`

.. versionadded:: 2.16
%End

  protected:
    QgsFeatureRenderer( const QString &type );

    void renderFeatureWithSymbol( QgsFeature &feature,
                                  QgsSymbol *symbol,
                                  QgsRenderContext &context,
                                  int layer,
                                  bool selected,
                                  bool drawVertexMarker );

    void renderVertexMarker( QPointF pt, QgsRenderContext &context );
%Docstring
render editing vertex marker at specified point
%End
    void renderVertexMarkerPolyline( QPolygonF &pts, QgsRenderContext &context );
%Docstring
render editing vertex marker for a polyline
%End
    void renderVertexMarkerPolygon( QPolygonF &pts, QList<QPolygonF> *rings, QgsRenderContext &context );
%Docstring
render editing vertex marker for a polygon
%End

    static QPointF _getPoint( QgsRenderContext &context, const QgsPoint &point );
%Docstring
Creates a point in screen coordinates from a wkb string in map
coordinates
%End

    void copyRendererData( QgsFeatureRenderer *destRenderer ) const;
%Docstring
Clones generic renderer data to another renderer.
Currently clones
* Order By
* Paint Effect

:param destRenderer: destination renderer for copied effect
%End






    static void convertSymbolSizeScale( QgsSymbol *symbol, QgsSymbol::ScaleMethod method, const QString &field );
%Docstring

.. note::

   this function is used to convert old sizeScale expressions to symbol
level DataDefined size
%End

    static void convertSymbolRotation( QgsSymbol *symbol, const QString &field );
%Docstring

.. note::

   this function is used to convert old rotations expressions to symbol
level DataDefined angle
%End



  private:
    QgsFeatureRenderer( const QgsFeatureRenderer & );
    QgsFeatureRenderer &operator=( const QgsFeatureRenderer & );
};

QFlags<QgsFeatureRenderer::Capability> operator|(QgsFeatureRenderer::Capability f1, QFlags<QgsFeatureRenderer::Capability> f2);



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsrenderer.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
