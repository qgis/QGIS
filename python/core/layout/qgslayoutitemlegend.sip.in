/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemlegend.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLegendModel : QgsLayerTreeModel
{
%Docstring
Item model implementation based on layer tree model for layout legend.

Overrides some functionality of QgsLayerTreeModel to better fit the needs of layout legends.

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslayoutitemlegend.h"
%End
  public:
    QgsLegendModel( QgsLayerTree *rootNode, QObject *parent /TransferThis/ = 0 );
%Docstring
Construct the model based on the given layer tree
%End

    virtual QVariant data( const QModelIndex &index, int role ) const;


    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

};



class QgsLayoutItemLegend : QgsLayoutItem
{
%Docstring
A layout item subclass for map legends.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemlegend.h"
%End
  public:

    QgsLayoutItemLegend( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutItemLegend, with the specified parent ``layout``.
%End

    static QgsLayoutItemLegend *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new legend item for the specified ``layout``.

The caller takes responsibility for deleting the returned object.
%End

    virtual int type() const;

    virtual QIcon icon() const;

    virtual QString displayName() const;


    void adjustBoxSize();
%Docstring
Sets the legend's item bounds to fit the whole legend content.
%End

    void setResizeToContents( bool enabled );
%Docstring
Sets whether the legend should automatically resize to fit its contents.

:param enabled: set to false to disable automatic resizing. The legend frame will not
be expanded to fit legend items, and items may be cropped from display.

.. seealso:: :py:func:`resizeToContents`
%End

    bool resizeToContents() const;
%Docstring
Returns whether the legend should automatically resize to fit its contents.

.. seealso:: :py:func:`setResizeToContents`
%End

    QgsLegendModel *model();
%Docstring
Returns the legend model.
%End

    void setAutoUpdateModel( bool autoUpdate );
%Docstring
Sets whether the legend content should auto update to reflect changes in the project's
layer tree.

.. seealso:: :py:func:`autoUpdateModel`
%End

    bool autoUpdateModel() const;
%Docstring
Returns whether the legend content should auto update to reflect changes in the project's
layer tree.

.. seealso:: :py:func:`setAutoUpdateModel`
%End

    void setLegendFilterByMapEnabled( bool enabled );
%Docstring
Set whether legend items should be filtered to show just the ones visible in the associated map.

.. seealso:: :py:func:`legendFilterByMapEnabled`
%End

    bool legendFilterByMapEnabled() const;
%Docstring
Find out whether legend items are filtered to show just the ones visible in the associated map

.. seealso:: :py:func:`setLegendFilterByMapEnabled`
%End

    void setLegendFilterOutAtlas( bool doFilter );
%Docstring
When set to true, during an atlas rendering, it will filter out legend elements
where features are outside the current atlas feature.

.. seealso:: :py:func:`legendFilterOutAtlas`
%End

    bool legendFilterOutAtlas() const;
%Docstring
Returns whether to filter out legend elements outside of the current atlas feature.

.. seealso:: :py:func:`setLegendFilterOutAtlas`
%End

    void setTitle( const QString &title );
%Docstring
Sets the legend ``title``.

.. seealso:: :py:func:`title`
%End

    QString title() const;
%Docstring
Returns the legend title.

.. seealso:: :py:func:`setTitle`
%End

    Qt::AlignmentFlag titleAlignment() const;
%Docstring
Returns the alignment of the legend title.

.. seealso:: :py:func:`setTitleAlignment`
%End

    void setTitleAlignment( Qt::AlignmentFlag alignment );
%Docstring
Sets the ``alignment`` of the legend title.

.. seealso:: :py:func:`titleAlignment`
%End

    QgsLegendStyle &rstyle( QgsLegendStyle::Style s );
%Docstring
Returns reference to modifiable legend style.
%End

    QgsLegendStyle style( QgsLegendStyle::Style s ) const;
%Docstring
Returns legend style.
%End

    void setStyle( QgsLegendStyle::Style component, const QgsLegendStyle &style );
%Docstring
Sets the style of ``component`` to ``style`` for the legend.
%End

    QFont styleFont( QgsLegendStyle::Style component ) const;
%Docstring
Returns the font settings for a legend ``component``.

.. seealso:: :py:func:`setStyleFont`
%End

    void setStyleFont( QgsLegendStyle::Style component, const QFont &font );
%Docstring
Sets the style ``font`` for a legend ``component``.

.. seealso:: :py:func:`styleFont`
%End

    void setStyleMargin( QgsLegendStyle::Style component, double margin );
%Docstring
Set the ``margin`` for a legend ``component``.
%End

    void setStyleMargin( QgsLegendStyle::Style component, QgsLegendStyle::Side side, double margin );
%Docstring
Set the ``margin`` for a particular ``side`` of a legend ``component``.
%End

    double lineSpacing() const;
%Docstring
Returns the spacing in-between lines in layout units.

.. seealso:: :py:func:`setLineSpacing`
%End

    void setLineSpacing( double spacing );
%Docstring
Sets the ``spacing`` in-between multiple lines.

.. seealso:: :py:func:`lineSpacing`
%End

    double boxSpace() const;
%Docstring
Returns the legend box space.

.. seealso:: :py:func:`setBoxSpace`
%End

    void setBoxSpace( double space );
%Docstring
Sets the legend box ``space``.

.. seealso:: :py:func:`boxSpace`
%End

    double columnSpace() const;
%Docstring
Returns the legend column spacing.

.. seealso:: :py:func:`setColumnSpace`
%End

    void setColumnSpace( double spacing );
%Docstring
Sets the legend column ``spacing``.

.. seealso:: :py:func:`columnSpace`
%End

    QColor fontColor() const;
%Docstring
Returns the legend font color.

.. seealso:: :py:func:`setFontColor`
%End

    void setFontColor( const QColor &color );
%Docstring
Sets the legend font ``color``.

.. seealso:: :py:func:`fontColor`
%End

    double symbolWidth() const;
%Docstring
Returns the legend symbol width.

.. seealso:: :py:func:`setSymbolWidth`
%End

    void setSymbolWidth( double width );
%Docstring
Sets the legend symbol ``width``.

.. seealso:: :py:func:`symbolWidth`
%End

    double symbolHeight() const;
%Docstring
Returns the legend symbol height.

.. seealso:: :py:func:`setSymbolHeight`
%End

    void setSymbolHeight( double height );
%Docstring
Sets the legend symbol ``height``.

.. seealso:: :py:func:`symbolHeight`
%End

    double wmsLegendWidth() const;
%Docstring
Returns the WMS legend width.

.. seealso:: :py:func:`setWmsLegendWidth`
%End

    void setWmsLegendWidth( double width );
%Docstring
Sets the WMS legend ``width``.

.. seealso:: :py:func:`wmsLegendWidth`
%End

    double wmsLegendHeight() const;
%Docstring
Returns the WMS legend height.

.. seealso:: :py:func:`setWmsLegendHeight`
%End

    void setWmsLegendHeight( double height );
%Docstring
Sets the WMS legend ``height``.

.. seealso:: :py:func:`wmsLegendHeight`
%End

    void setWrapString( const QString &string );
%Docstring
Sets the legend text wrapping ``string``.

.. seealso:: :py:func:`wrapString`
%End

    QString wrapString() const;
%Docstring
Returns the legend text wrapping string.

.. seealso:: :py:func:`setWrapString`
%End

    int columnCount() const;
%Docstring
Returns the legend column count.

.. seealso:: :py:func:`setColumnCount`
%End

    void setColumnCount( int count );
%Docstring
Sets the legend column ``count``.

.. seealso:: :py:func:`columnCount`
%End

    bool splitLayer() const;
%Docstring
Returns whether the legend items from a single layer can be split
over multiple columns.

.. seealso:: :py:func:`setSplitLayer`
%End

    void setSplitLayer( bool enabled );
%Docstring
Sets whether the legend items from a single layer can be split
over multiple columns.

.. seealso:: :py:func:`splitLayer`
%End

    bool equalColumnWidth() const;
%Docstring
Returns whether column widths should be equalized.

.. seealso:: :py:func:`setEqualColumnWidth`
%End

    void setEqualColumnWidth( bool equalize );
%Docstring
Sets whether column widths should be equalized.

.. seealso:: :py:func:`equalColumnWidth`
%End

    bool drawRasterStroke() const;
%Docstring
Returns whether a stroke will be drawn around raster symbol items.

.. seealso:: :py:func:`setDrawRasterStroke`

.. seealso:: :py:func:`rasterStrokeColor`

.. seealso:: :py:func:`rasterStrokeWidth`
%End

    void setDrawRasterStroke( bool enabled );
%Docstring
Sets whether a stroke will be drawn around raster symbol items.

:param enabled: set to true to draw borders

.. seealso:: :py:func:`drawRasterStroke`

.. seealso:: :py:func:`setRasterStrokeColor`

.. seealso:: :py:func:`setRasterStrokeWidth`
%End

    QColor rasterStrokeColor() const;
%Docstring
Returns the stroke color for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

.. seealso:: :py:func:`setRasterStrokeColor`

.. seealso:: :py:func:`drawRasterStroke`

.. seealso:: :py:func:`rasterStrokeWidth`
%End

    void setRasterStrokeColor( const QColor &color );
%Docstring
Sets the stroke ``color`` for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

.. seealso:: :py:func:`rasterStrokeColor`

.. seealso:: :py:func:`setDrawRasterStroke`

.. seealso:: :py:func:`setRasterStrokeWidth`
%End

    double rasterStrokeWidth() const;
%Docstring
Returns the stroke width (in layout units) for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

.. seealso:: :py:func:`setRasterStrokeWidth`

.. seealso:: :py:func:`drawRasterStroke`

.. seealso:: :py:func:`rasterStrokeColor`
%End

    void setRasterStrokeWidth( double width );
%Docstring
Sets the stroke width for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

.. seealso:: :py:func:`rasterStrokeWidth`

.. seealso:: :py:func:`setDrawRasterStroke`

.. seealso:: :py:func:`setRasterStrokeColor`
%End

    void setLinkedMap( QgsLayoutItemMap *map );
%Docstring
Sets the ``map`` to associate with the legend.

.. seealso:: :py:func:`linkedMap`
%End

    QgsLayoutItemMap *linkedMap() const;
%Docstring
Returns the associated map.

.. seealso:: :py:func:`setLinkedMap`
%End

    void updateLegend();
%Docstring
Updates the model and all legend entries.
%End

    void updateFilterByMap( bool redraw = true );
%Docstring
Updates the legend content when filtered by map.
%End

    const QgsLegendSettings &legendSettings() const;
%Docstring
Returns the legend's renderer settings object.
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );


    virtual void finalizeRestoreFromXml();



  public slots:

    virtual void refresh();

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );


  protected:
    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemlegend.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
