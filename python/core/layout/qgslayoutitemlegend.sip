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
 :rtype: QgsLayoutItemLegend
%End

    virtual int type() const;

    virtual QString stringType() const;

    virtual QString displayName() const;


    void adjustBoxSize();
%Docstring
 Sets the legend's item bounds to fit the whole legend content.
%End

    void setResizeToContents( bool enabled );
%Docstring
 Sets whether the legend should automatically resize to fit its contents.
 \param enabled set to false to disable automatic resizing. The legend frame will not
 be expanded to fit legend items, and items may be cropped from display.
.. seealso:: resizeToContents()
%End

    bool resizeToContents() const;
%Docstring
 Returns whether the legend should automatically resize to fit its contents.
.. seealso:: setResizeToContents()
 :rtype: bool
%End

    QgsLegendModel *model();
%Docstring
 Returns the legend model.
 :rtype: QgsLegendModel
%End

    void setAutoUpdateModel( bool autoUpdate );
%Docstring
 Sets whether the legend content should auto update to reflect changes in the project's
 layer tree.
.. seealso:: autoUpdateModel()
%End

    bool autoUpdateModel() const;
%Docstring
 Returns whether the legend content should auto update to reflect changes in the project's
 layer tree.
.. seealso:: setAutoUpdateModel()
 :rtype: bool
%End

    void setLegendFilterByMapEnabled( bool enabled );
%Docstring
 Set whether legend items should be filtered to show just the ones visible in the associated map.
.. seealso:: legendFilterByMapEnabled()
%End

    bool legendFilterByMapEnabled() const;
%Docstring
 Find out whether legend items are filtered to show just the ones visible in the associated map
.. seealso:: setLegendFilterByMapEnabled()
 :rtype: bool
%End


    void setLegendFilterOutAtlas( bool doFilter );
%Docstring
 When set to true, during an atlas rendering, it will filter out legend elements
 where features are outside the current atlas feature.
.. seealso:: legendFilterOutAtlas()
%End

    bool legendFilterOutAtlas() const;
%Docstring
 Returns whether to filter out legend elements outside of the current atlas feature.
.. seealso:: setLegendFilterOutAtlas()
 :rtype: bool
%End

    void setTitle( const QString &title );
%Docstring
 Sets the legend ``title``.
.. seealso:: title()
%End

    QString title() const;
%Docstring
 Returns the legend title.
.. seealso:: setTitle()
 :rtype: str
%End

    Qt::AlignmentFlag titleAlignment() const;
%Docstring
 Returns the alignment of the legend title.
.. seealso:: setTitleAlignment()
 :rtype: Qt.AlignmentFlag
%End

    void setTitleAlignment( Qt::AlignmentFlag alignment );
%Docstring
 Sets the ``alignment`` of the legend title.
.. seealso:: titleAlignment()
%End

    QgsLegendStyle &rstyle( QgsLegendStyle::Style s );
%Docstring
 Returns reference to modifiable legend style.
 :rtype: QgsLegendStyle
%End

    QgsLegendStyle style( QgsLegendStyle::Style s ) const;
%Docstring
 Returns legend style.
 :rtype: QgsLegendStyle
%End

    void setStyle( QgsLegendStyle::Style component, const QgsLegendStyle &style );
%Docstring
 Sets the style of ``component`` to ``style`` for the legend.
%End

    QFont styleFont( QgsLegendStyle::Style component ) const;
%Docstring
 Returns the font settings for a legend ``component``.
.. seealso:: setStyleFont()
 :rtype: QFont
%End

    void setStyleFont( QgsLegendStyle::Style component, const QFont &font );
%Docstring
 Sets the style ``font`` for a legend ``component``.
.. seealso:: styleFont()
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
.. seealso:: setLineSpacing()
 :rtype: float
%End

    void setLineSpacing( double spacing );
%Docstring
 Sets the ``spacing`` in-between multiple lines.
.. seealso:: lineSpacing()
%End

    double boxSpace() const;
%Docstring
 Returns the legend box space.
.. seealso:: setBoxSpace()
 :rtype: float
%End

    void setBoxSpace( double space );
%Docstring
 Sets the legend box ``space``.
.. seealso:: boxSpace()
%End

    double columnSpace() const;
%Docstring
 Returns the legend column spacing.
.. seealso:: setColumnSpace()
 :rtype: float
%End

    void setColumnSpace( double spacing );
%Docstring
 Sets the legend column ``spacing``.
.. seealso:: columnSpace()
%End

    QColor fontColor() const;
%Docstring
 Returns the legend font color.
.. seealso:: setFontColor()
 :rtype: QColor
%End

    void setFontColor( const QColor &color );
%Docstring
 Sets the legend font ``color``.
.. seealso:: fontColor()
%End

    double symbolWidth() const;
%Docstring
 Returns the legend symbol width.
.. seealso:: setSymbolWidth()
 :rtype: float
%End

    void setSymbolWidth( double width );
%Docstring
 Sets the legend symbol ``width``.
.. seealso:: symbolWidth()
%End

    double symbolHeight() const;
%Docstring
 Returns the legend symbol height.
.. seealso:: setSymbolHeight()
 :rtype: float
%End

    void setSymbolHeight( double height );
%Docstring
 Sets the legend symbol ``height``.
.. seealso:: symbolHeight()
%End

    double wmsLegendWidth() const;
%Docstring
 Returns the WMS legend width.
.. seealso:: setWmsLegendWidth()
 :rtype: float
%End

    void setWmsLegendWidth( double width );
%Docstring
 Sets the WMS legend ``width``.
.. seealso:: wmsLegendWidth()
%End

    double wmsLegendHeight() const;
%Docstring
 Returns the WMS legend height.
.. seealso:: setWmsLegendHeight()
 :rtype: float
%End

    void setWmsLegendHeight( double height );
%Docstring
 Sets the WMS legend ``height``.
.. seealso:: wmsLegendHeight()
%End

    void setWrapString( const QString &string );
%Docstring
 Sets the legend text wrapping ``string``.
.. seealso:: wrapString()
%End

    QString wrapString() const;
%Docstring
 Returns the legend text wrapping string.
.. seealso:: setWrapString()
 :rtype: str
%End

    int columnCount() const;
%Docstring
 Returns the legend column count.
.. seealso:: setColumnCount()
 :rtype: int
%End

    void setColumnCount( int count );
%Docstring
 Sets the legend column ``count``.
.. seealso:: columnCount()
%End

    bool splitLayer() const;
%Docstring
 Returns whether the legend items from a single layer can be split
 over multiple columns.
.. seealso:: setSplitLayer()
 :rtype: bool
%End

    void setSplitLayer( bool enabled );
%Docstring
 Sets whether the legend items from a single layer can be split
 over multiple columns.
.. seealso:: splitLayer()
%End

    bool equalColumnWidth() const;
%Docstring
 Returns whether column widths should be equalized.
.. seealso:: setEqualColumnWidth()
 :rtype: bool
%End

    void setEqualColumnWidth( bool equalize );
%Docstring
 Sets whether column widths should be equalized.
.. seealso:: equalColumnWidth()
%End

    bool drawRasterStroke() const;
%Docstring
 Returns whether a stroke will be drawn around raster symbol items.
.. seealso:: setDrawRasterStroke()
.. seealso:: rasterStrokeColor()
.. seealso:: rasterStrokeWidth()
 :rtype: bool
%End

    void setDrawRasterStroke( bool enabled );
%Docstring
 Sets whether a stroke will be drawn around raster symbol items.
 \param enabled set to true to draw borders
.. seealso:: drawRasterStroke()
.. seealso:: setRasterStrokeColor()
.. seealso:: setRasterStrokeWidth()
%End

    QColor rasterStrokeColor() const;
%Docstring
 Returns the stroke color for the stroke drawn around raster symbol items. The stroke is
 only drawn if drawRasterStroke() is true.
.. seealso:: setRasterStrokeColor()
.. seealso:: drawRasterStroke()
.. seealso:: rasterStrokeWidth()
 :rtype: QColor
%End

    void setRasterStrokeColor( const QColor &color );
%Docstring
 Sets the stroke ``color`` for the stroke drawn around raster symbol items. The stroke is
 only drawn if drawRasterStroke() is true.
.. seealso:: rasterStrokeColor()
.. seealso:: setDrawRasterStroke()
.. seealso:: setRasterStrokeWidth()
%End

    double rasterStrokeWidth() const;
%Docstring
 Returns the stroke width (in layout units) for the stroke drawn around raster symbol items. The stroke is
 only drawn if drawRasterStroke() is true.
.. seealso:: setRasterStrokeWidth()
.. seealso:: drawRasterStroke()
.. seealso:: rasterStrokeColor()
 :rtype: float
%End

    void setRasterStrokeWidth( double width );
%Docstring
 Sets the stroke width for the stroke drawn around raster symbol items. The stroke is
 only drawn if drawRasterStroke() is true.
.. seealso:: rasterStrokeWidth()
.. seealso:: setDrawRasterStroke()
.. seealso:: setRasterStrokeColor()
%End

    void setMap( QgsLayoutItemMap *map );
%Docstring
 Sets the ``map`` to associate with the legend.
.. seealso:: map()
%End

    QgsLayoutItemMap *map() const;
%Docstring
 Returns the associated map.
.. seealso:: setMap()
 :rtype: QgsLayoutItemMap
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
 :rtype: QgsLegendSettings
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );



  public slots:

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
